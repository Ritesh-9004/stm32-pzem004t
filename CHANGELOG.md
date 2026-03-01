# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-03-01

### Added
- Initial release of STM32 PZEM-004T v4.0 library
- Complete Modbus RTU protocol implementation with CRC16 verification
- Read AC voltage measurement (0.1V resolution)
- Read current measurement (0.001A resolution)
- Read active power measurement (0.1W resolution)
- Read energy consumption (kWh)
- Read frequency measurement (0.1Hz resolution)
- Read power factor (0.01 resolution)
- Energy counter reset function
- Efficient register polling using direct UART access
- Non-blocking operation with configurable poll intervals
- Support for STM32F103C8T6 (Blue Pill) and other STM32 devices
- Full HAL library compatibility
- Debug output via secondary UART
- Comprehensive API documentation
- Example application with complete usage guide
- Circuit diagram and wiring documentation
- Troubleshooting guide
- Register map documentation
- MIT License

### Features
- Single transaction read for all parameters
- 200ms timeout for reliable communication
- Proven raw register polling method
- Clean and simple C API
- No external dependencies beyond STM32 HAL
- Support for any STM32 with HAL support

### Documentation
- Complete README with setup instructions
- Hardware requirements and wiring diagrams
- Software configuration guide
- API reference with examples
- Technical details and register map
- Safety warnings for AC voltage handling
