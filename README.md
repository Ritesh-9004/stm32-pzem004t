<div align="center">

# STM32 PZEM-004T v4.0 Library

![Platform](https://img.shields.io/badge/Platform-STM32-blue?logo=stmicroelectronics)
![Language](https://img.shields.io/badge/Language-C-00599C?logo=c)
![HAL](https://img.shields.io/badge/HAL-Driver-green)
![Protocol](https://img.shields.io/badge/Protocol-Modbus_RTU-orange)
![Module](https://img.shields.io/badge/Module-PZEM--004T_v3.0-red)


</div>

---
A complete STM32 HAL library for interfacing with the PZEM-004T v4.0 AC power/energy monitoring module using Modbus RTU protocol over UART.

## Features

- ✅ Read AC voltage (V)
- ✅ Read current (A)
- ✅ Read active power (W)
- ✅ Read energy consumption (kWh)
- ✅ Read frequency (Hz)
- ✅ Read power factor
- ✅ Reset energy counter
- ✅ Modbus RTU protocol with CRC16 verification
- ✅ Efficient register polling for reliable communication
- ✅ Non-blocking operation with configurable poll intervals
- ✅ Debug output via UART
- ✅ Clean and simple API

## Hardware Requirements

- **Microcontroller:** STM32F103C8T6 (Blue Pill) or any STM32 with HAL support
- **Module:** PZEM-004T v4.0 (TTL UART version)
- **AC Load:** Connected to PZEM (required for valid readings)
- **USB-to-TTL adapter:** For debug output (optional)

## Wiring Diagram

### STM32F103C8T6 ↔ PZEM-004T

```
STM32 (USART3)         PZEM-004T
─────────────────────────────────
PB10 (TX)      →       RX
PB11 (RX)      ←       TX
GND            ─       GND
                       5V (from PZEM, not STM32!)
```

### Debug Output (Optional)



```
STM32 (USART1)         USB-to-TTL
─────────────────────────────────
PA9  (TX)      →       RX
PA10 (RX)      ←       TX
GND            ─       GND
```



**Baud Rates:**
- PZEM communication (USART3): **9600 baud, 8N1**
- Debug output (USART1): **115200 baud**

### PZEM-004T Power Connections

```
PZEM-004T Terminals
──────────────────────
L  ─  AC Live (220V)
N  ─  AC Neutral
```

⚠️ **Warning:** Connect PZEM to mains AC voltage with proper safety precautions!

## Circuit Diagram

![Circuit Diagram](docs/stm32-pzem004t.svg)

*Complete circuit schematic showing STM32, PZEM-004T connections, and power supply.*

## Software Requirements

- **STM32CubeIDE** or **STM32CubeMX** + any ARM toolchain
- **STM32 HAL Library** for your specific MCU
- **UART peripherals configured:**
  - USART3: 9600 baud, 8N1 (for PZEM)
  - USART1: 115200 baud (for debug) - optional

## Getting Started

### 1. Clone or Download

```bash
git clone https://github.com/Ritesh-9004/stm32-pzem004t.git
```

### 2. Include Library Files

Add these files to your project:
- `Core/Inc/pzem004t.h`
- `Core/Src/pzem004t.c`

### 3. Configure UART

In STM32CubeMX or your initialization code:

```c
// USART3 for PZEM (9600 baud, 8N1)
huart3.Instance = USART3;
huart3.Init.BaudRate = 9600;
huart3.Init.WordLength = UART_WORDLENGTH_8B;
huart3.Init.StopBits = UART_STOPBITS_1;
huart3.Init.Parity = UART_PARITY_NONE;
huart3.Init.Mode = UART_MODE_TX_RX;
```

### 4. Basic Usage Example

```c
#include "pzem004t.h"
#include <stdio.h>

PZEM_t pzem;
UART_HandleTypeDef huart3;  // UART connected to PZEM

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_USART3_UART_Init();
    
    // Initialize PZEM (address 0x01)
    PZEM_Init(&pzem, &huart3, 0x01);
    
    while (1) {
        // Read all parameters from PZEM
        if (PZEM_ReadAll(&pzem)) {
            float voltage = PZEM_Voltage(&pzem);
            float current = PZEM_Current(&pzem);
            float power = PZEM_Power(&pzem);
            float energy = PZEM_Energy(&pzem);
            float frequency = PZEM_Frequency(&pzem);
            float pf = PZEM_PowerFactor(&pzem);
            
            printf("V: %.1fV | I: %.3fA | P: %.1fW | E: %.3fkWh | F: %.1fHz | PF: %.2f\n",
                   voltage, current, power, energy, frequency, pf);
        } else {
            printf("PZEM read failed!\n");
        }
        
        HAL_Delay(1000);  // Read every 1 second
    }
}
```

## API Reference

### Initialization

```c
void PZEM_Init(PZEM_t *pzem, UART_HandleTypeDef *huart, uint8_t addr);
```
- `pzem`: Pointer to PZEM structure
- `huart`: Pointer to UART handle (configured for 9600 baud)
- `addr`: PZEM device address (default: `0x01`)

### Reading Data

```c
bool PZEM_ReadAll(PZEM_t *pzem);
```
Reads all registers from PZEM in a single transaction. Returns `true` on success.

**Always call this before getting individual values!**

### Get Individual Values

```c
float PZEM_Voltage(PZEM_t *pzem);      // Returns voltage in V
float PZEM_Current(PZEM_t *pzem);      // Returns current in A
float PZEM_Power(PZEM_t *pzem);        // Returns power in W
float PZEM_Energy(PZEM_t *pzem);       // Returns energy in kWh
float PZEM_Frequency(PZEM_t *pzem);    // Returns frequency in Hz
float PZEM_PowerFactor(PZEM_t *pzem);  // Returns power factor (0.00-1.00)
```

These functions extract values from the last successful `PZEM_ReadAll()` call.

### Reset Energy Counter

```c
bool PZEM_ResetEnergy(PZEM_t *pzem);
```
Resets the accumulated energy counter to zero.

## Example Output

```
============================================
=== STM32 PZEM004T Energy Monitor ===
============================================
USART3: PB10(TX) PB11(RX) @ 9600 8N1
USART1: PA9(TX) @ 115200 (Debug)
PZEM Address: 0x01
Using raw register polling (proven method)
============================================

PZEM initialized
Polling every 500 ms...

[1023 ms] Voltage: 230.2V | Current: 0.421A | Power: 95.3W | Energy: 0.127kWh | Freq: 50.0Hz | PF: 0.98
[1523 ms] Voltage: 230.1V | Current: 0.419A | Power: 94.8W | Energy: 0.127kWh | Freq: 50.0Hz | PF: 0.98
[2023 ms] Voltage: 230.3V | Current: 0.422A | Power: 95.5W | Energy: 0.127kWh | Freq: 50.0Hz | PF: 0.98
```

## Technical Details

### Modbus RTU Protocol

The PZEM-004T v3.0 uses Modbus RTU over UART:
- **Baud rate:** 9600
- **Data format:** 8N1 (8 data bits, no parity, 1 stop bit)
- **CRC:** CRC16-Modbus (polynomial 0xA001)

### Register Map

| Register | Parameter       | Units  | Format   |
|----------|----------------|--------|----------|
| 0x0000   | Voltage        | 0.1V   | uint16   |
| 0x0001-2 | Current        | 0.001A | uint32   |
| 0x0003-4 | Power          | 0.1W   | uint32   |
| 0x0005-6 | Energy         | 1Wh    | uint32   |
| 0x0007   | Frequency      | 0.1Hz  | uint16   |
| 0x0008   | Power Factor   | 0.01   | uint16   |
| 0x0009   | Alarm Status   | -      | uint16   |

### Communication Details

- **Read command:** Function code `0x04` (Read Input Registers)
- **Registers read:** 10 (0x0000 to 0x0009)
- **Response time:** ~30ms @ 9600 baud
- **Timeout:** 200ms
- **Raw register polling:** Direct UART register access for reliability

## Troubleshooting

### No Response from PZEM

1. Check wiring (TX↔RX crossover)
2. Verify PZEM has AC power (LED should be on)
3. Ensure baud rate is 9600
4. Check PZEM address (default 0x01)
5. Connect AC load to PZEM for valid readings

### CRC Errors

- Check for loose connections
- Verify GND is common between STM32 and PZEM
- Reduce cable length between STM32 and PZEM

### Zero/Invalid Readings

- PZEM needs an AC load connected to measure properly
- Wait 1-2 seconds after power-up for PZEM to stabilize

## Project Structure

```
stm32-pzem004t/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── pzem004t.h          ← Library header
│   │   └── stm32f1xx_hal_conf.h
│   └── Src/
│       ├── main.c               ← Example application
│       ├── pzem004t.c           ← Library implementation
│       └── stm32f1xx_hal_msp.c
├── Drivers/                     ← STM32 HAL drivers
├── README.md
└── stm32-pzem.ioc              ← STM32CubeMX project
```

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## References

- [PZEM-004T v3.0 Datasheet](https://innovatorsguru.com/wp-content/uploads/2019/06/PZEM-004T-V3.0-Datasheet-User-Manual.pdf)
- [Modbus Protocol Specification](https://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)
- [STM32F103 HAL Documentation](https://www.st.com/resource/en/user_manual/um1850-description-of-stm32f1-hal-and-lowlayer-drivers-stmicroelectronics.pdf)


---

**⚠️ Safety Warning:** This project involves working with mains AC voltage. Only qualified personnel should work with high voltage. Always disconnect power before making connections. Use proper isolation and safety equipment.
