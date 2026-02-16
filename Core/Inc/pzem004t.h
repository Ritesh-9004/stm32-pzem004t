/* PZEM004T v3.0 Library for STM32 HAL */
#ifndef PZEM004T_H
#define PZEM004T_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

// PZEM004T Commands
#define PZEM_READ_COMMAND     0x04
#define PZEM_WRITE_COMMAND    0x06
#define PZEM_RESET_ENERGY     0x42

// Register addresses
#define REG_VOLTAGE       0x0000
#define REG_CURRENT_L     0x0001
#define REG_CURRENT_H     0x0002
#define REG_POWER_L       0x0003
#define REG_POWER_H       0x0004
#define REG_ENERGY_L      0x0005
#define REG_ENERGY_H      0x0006
#define REG_FREQUENCY     0x0007
#define REG_PF            0x0008
#define REG_ALARM         0x0009

typedef struct {
    UART_HandleTypeDef *huart;
    uint8_t addr;
    uint32_t timeout;
    uint8_t lastResponse[25];  // Cache for last Modbus response
} PZEM_t;

// Initialize PZEM
void PZEM_Init(PZEM_t *pzem, UART_HandleTypeDef *huart, uint8_t addr);

// Read all values from PZEM at once
bool PZEM_ReadAll(PZEM_t *pzem);

// Get cached values (call PZEM_ReadAll first)
float PZEM_Voltage(PZEM_t *pzem);
float PZEM_Current(PZEM_t *pzem);
float PZEM_Power(PZEM_t *pzem);
float PZEM_Energy(PZEM_t *pzem);
float PZEM_Frequency(PZEM_t *pzem);
float PZEM_PowerFactor(PZEM_t *pzem);

// Reset energy
bool PZEM_ResetEnergy(PZEM_t *pzem);

#endif // PZEM004T_H
