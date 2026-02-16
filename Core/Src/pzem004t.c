/* PZEM004T v3.0 Library Implementation for STM32 HAL */
#include "pzem004t.h"
#include <string.h>
#include <math.h>

#define RESPONSE_SIZE 25
#define READ_TIMEOUT 200  // Fast timeout (PZEM responds in ~30ms at 9600 baud)
#define PZEM_DEBUG 0      // Set to 1 to enable TX/RX frame debugging, 0 for max speed

// CRC16 Modbus calculation
static uint16_t calculateCRC(uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Send command and receive response using direct register polling
static bool sendReceive(PZEM_t *pzem, uint8_t *sendBuffer, uint8_t sendSize,
                        uint8_t *recvBuffer, uint8_t recvSize) {
#if PZEM_DEBUG
    extern int printf(const char *format, ...);
#endif
    
    // Clear RX buffer by reading any stale data
    while (pzem->huart->Instance->SR & USART_SR_RXNE) {
        volatile uint8_t dummy = pzem->huart->Instance->DR;
        (void)dummy;
    }
    
#if PZEM_DEBUG
    // Show TX frame
    printf("TX: ");
    for(uint8_t i = 0; i < sendSize; i++) {
        printf("%02X ", sendBuffer[i]);
    }
    printf("\r\n");
#endif
    
    // Send command
    if (HAL_UART_Transmit(pzem->huart, sendBuffer, sendSize, pzem->timeout) != HAL_OK) {
#if PZEM_DEBUG
        printf("TX FAILED\r\n");
#endif
        return false;
    }
    
    // Raw register polling receive (proven to work!)
    uint32_t startTime = HAL_GetTick();
    uint8_t bytesReceived = 0;
    uint32_t lastByteTime = startTime;
    
    while (bytesReceived < recvSize) {
        // Check if data ready
        if (pzem->huart->Instance->SR & USART_SR_RXNE) {
            recvBuffer[bytesReceived] = pzem->huart->Instance->DR;
            bytesReceived++;
            lastByteTime = HAL_GetTick();
        }
        
        // Timeout: either total time exceeded or no new byte for 50ms
        if ((HAL_GetTick() - startTime) > pzem->timeout || 
            (bytesReceived > 0 && (HAL_GetTick() - lastByteTime) > 50)) {
            break;
        }
    }
    
#if PZEM_DEBUG
    // Show RX frame
    printf("RX: ");
#endif
    if (bytesReceived == 0) {
#if PZEM_DEBUG
        printf("FAILED (no data)\r\n");
#endif
        return false;
    }
    
#if PZEM_DEBUG
    for(uint8_t i = 0; i < bytesReceived; i++) {
        printf("%02X ", recvBuffer[i]);
    }
    printf("\r\n");
#endif
    
    if (bytesReceived < recvSize) {
#if PZEM_DEBUG
        printf("PARTIAL (%d/%d bytes)\r\n", bytesReceived, recvSize);
#endif
        return false;
    }
    
    // Verify CRC
    uint16_t receivedCRC = (recvBuffer[recvSize - 1] << 8) | recvBuffer[recvSize - 2];
    uint16_t calculatedCRC = calculateCRC(recvBuffer, recvSize - 2);
    
    if (receivedCRC != calculatedCRC) {
#if PZEM_DEBUG
        printf("CRC ERROR: Got %04X, Expected %04X\r\n", receivedCRC, calculatedCRC);
#endif
        return false;
    }
    
#if PZEM_DEBUG
    printf("CRC OK\r\n");
#endif
    return true;
}

// Initialize PZEM structure
void PZEM_Init(PZEM_t *pzem, UART_HandleTypeDef *huart, uint8_t addr) {
    pzem->huart = huart;
    pzem->addr = addr;
    pzem->timeout = READ_TIMEOUT;
    memset(pzem->lastResponse, 0, sizeof(pzem->lastResponse));
}

// Read all registers at once
static bool readRegisters(PZEM_t *pzem, uint8_t *response) {
    uint8_t command[8];
    command[0] = pzem->addr;         // Device address
    command[1] = PZEM_READ_COMMAND;  // Read command
    command[2] = 0x00;               // Start register high byte
    command[3] = 0x00;               // Start register low byte
    command[4] = 0x00;               // Number of registers high byte
    command[5] = 0x0A;               // Number of registers low byte (10 registers)
    
    uint16_t crc = calculateCRC(command, 6);
    command[6] = crc & 0xFF;         // CRC low byte
    command[7] = (crc >> 8) & 0xFF;  // CRC high byte
    
    return sendReceive(pzem, command, 8, response, RESPONSE_SIZE);
}

// Read all values at once and cache them
bool PZEM_ReadAll(PZEM_t *pzem) {
    return readRegisters(pzem, pzem->lastResponse);
}

// Get voltage from cached data
float PZEM_Voltage(PZEM_t *pzem) {
    uint32_t value = (pzem->lastResponse[3] << 8) | pzem->lastResponse[4];
    return value / 10.0f;
}

// Get current from cached data
float PZEM_Current(PZEM_t *pzem) {
    // Current uses 2 registers: LOW word first, HIGH word second
    uint16_t low_word = (pzem->lastResponse[5] << 8) | pzem->lastResponse[6];
    uint16_t high_word = (pzem->lastResponse[7] << 8) | pzem->lastResponse[8];
    uint32_t value = ((uint32_t)high_word << 16) | low_word;
    return value / 1000.0f;
}

// Get power from cached data
float PZEM_Power(PZEM_t *pzem) {
    // Power uses 2 registers: LOW word first, HIGH word second
    uint16_t low_word = (pzem->lastResponse[9] << 8) | pzem->lastResponse[10];
    uint16_t high_word = (pzem->lastResponse[11] << 8) | pzem->lastResponse[12];
    uint32_t value = ((uint32_t)high_word << 16) | low_word;
    return value / 10.0f;
}

// Get energy from cached data
float PZEM_Energy(PZEM_t *pzem) {
    // Energy uses 2 registers: LOW word first, HIGH word second
    uint16_t low_word = (pzem->lastResponse[13] << 8) | pzem->lastResponse[14];
    uint16_t high_word = (pzem->lastResponse[15] << 8) | pzem->lastResponse[16];
    uint32_t value = ((uint32_t)high_word << 16) | low_word;
    return value / 1000.0f;
}

// Get frequency from cached data
float PZEM_Frequency(PZEM_t *pzem) {
    uint32_t value = (pzem->lastResponse[17] << 8) | pzem->lastResponse[18];
    return value / 10.0f;
}

// Get power factor from cached data
float PZEM_PowerFactor(PZEM_t *pzem) {
    uint32_t value = (pzem->lastResponse[19] << 8) | pzem->lastResponse[20];
    return value / 100.0f;
}

// Reset energy counter
bool PZEM_ResetEnergy(PZEM_t *pzem) {
    uint8_t command[4];
    command[0] = pzem->addr;
    command[1] = PZEM_RESET_ENERGY;
    
    uint16_t crc = calculateCRC(command, 2);
    command[2] = crc & 0xFF;
    command[3] = (crc >> 8) & 0xFF;
    
    uint8_t response[5];
    return sendReceive(pzem, command, 4, response, 5);
}
