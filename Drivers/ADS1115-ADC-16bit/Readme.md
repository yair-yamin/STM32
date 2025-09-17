# ADS1115 16-bit ADC Driver for STM32 HAL

A comprehensive C driver for the Texas Instruments ADS1115 16-bit analog-to-digital converter, designed for STM32 microcontrollers using the HAL library with DMA support.

## Overview

The ADS1115 is a precision, low-power, 16-bit analog-to-digital converter that features an onboard reference and oscillator. This driver provides a complete interface for all ADS1115 features including single-ended and differential measurements, programmable gain amplifier, and built-in comparator functionality.

## Features

- **16-bit Resolution**: High-precision analog measurements
- **4 Input Channels**: Configurable as single-ended or differential inputs
- **Programmable Gain Amplifier (PGA)**: Input ranges from ±0.256V to ±6.144V
- **Variable Data Rates**: 8 to 860 samples per second
- **Operating Modes**: Single-shot and continuous conversion
- **Built-in Comparator**: Programmable thresholds with interrupt capability
- **DMA Support**: Non-blocking I2C operations for improved performance
- **Low Power**: Single-shot mode for battery-powered applications

## Hardware Requirements

- STM32 microcontroller with I2C peripheral
- ADS1115 breakout board or module
- Pull-up resistors on I2C lines (typically 4.7kΩ)

### Wiring

| ADS1115 Pin | STM32 Pin | Description |
|-------------|-----------|-------------|
| VDD | 3.3V | Power supply |
| GND | GND | Ground |
| SCL | I2C SCL | Serial clock |
| SDA | I2C SDA | Serial data |
| ADDR | GND/VDD | I2C address select |
| ALERT/RDY | GPIO (optional) | Alert/Ready pin |

## Installation

1. Copy `ADS1115.h` and `ADS1115.c` to your STM32 project
2. Include the header in your main application:
```c
#include "ADS1115.h"
```
3. Ensure I2C and DMA are properly configured in STM32CubeIDE

## Quick Start

### Basic Setup

```c
#include "ADS1115.h"

// Declare handle
ADS1115_Handle_t hads1115;

int main(void) {
    // HAL initialization code here...
    
    // Configure ADS1115 handle
    hads1115.i2c_handle = &hi2c1;        // Your I2C handle
    hads1115.I2C_address = 0x90;         // 0x48 << 1 (7-bit addr shifted)
    hads1115.channel = AIN0;             // Start with channel A0
    
    // Initialize ADS1115
    ADS1115_Init(&hads1115, 
                 ADS1115_MODE_SINGLESHOT_MASK,  // Single-shot mode
                 AIN0,                          // Channel A0
                 ADS1115_PGA_4_096V_MASK,       // ±4.096V range
                 ADS1115_DR_128SPS_MASK);       // 128 SPS
    
    while(1) {
        // Start conversion
        ADS1115_StartSSConv(&hads1115);
        HAL_Delay(10);  // Wait for conversion
        
        // Read result
        ADS1115_ReadConversionReg(&hads1115);
        int16_t raw_value = hads1115.Reg[ADS1115_REG_CONVERSION];
        
        // Convert to voltage (for ±4.096V range)
        float voltage = (raw_value * 4.096) / 32768.0;
        
        HAL_Delay(1000);
    }
}
```

## API Reference
### Core Functions

#### ADS1115_Init()
Initialize the ADS1115 with specified configuration.
```c
HAL_StatusTypeDef ADS1115_Init(ADS1115_Handle_t* hads1115, 
                               uint16_t mode, 
                               sChannel_t channel, 
                               uint16_t pga, 
                               uint16_t sampleRate);
```

#### ADS1115_ReadConversionReg()
Read the latest conversion result.
```c
HAL_StatusTypeDef ADS1115_ReadConversionReg(ADS1115_Handle_t* hads1115);
```

#### ADS1115_SetChannel()
Change the active input channel.
```c
HAL_StatusTypeDef ADS1115_SetChannel(ADS1115_Handle_t* hads1115, 
                                     sChannel_t channel);
```

#### ADS1115_StartSSConv()
Start a single-shot conversion.
```c
HAL_StatusTypeDef ADS1115_StartSSConv(ADS1115_Handle_t* hads1115);
```

### PGA Settings

| Mask | Range | LSB Size |
|------|-------|----------|
| `ADS1115_PGA_6_144V_MASK` | ±6.144V | 187.5μV |
| `ADS1115_PGA_4_096V_MASK` | ±4.096V | 125μV |
| `ADS1115_PGA_2_048V_MASK` | ±2.048V | 62.5μV |
| `ADS1115_PGA_1_024V_MASK` | ±1.024V | 31.25μV |
| `ADS1115_PGA_0_512V_MASK` | ±0.512V | 15.625μV |
| `ADS1115_PGA_0_256V_MASK` | ±0.256V | 7.8125μV |

## Usage Examples

### Single-Shot Mode (Battery Powered)
```c
// Configure for single-shot mode
ADS1115_Init(&hads1115, ADS1115_MODE_SINGLESHOT_MASK, 
             AIN0, ADS1115_PGA_4_096V_MASK, ADS1115_DR_128SPS_MASK);

// Take measurement
ADS1115_StartSSConv(&hads1115);
HAL_Delay(10);  // Wait for conversion
ADS1115_ReadConversionReg(&hads1115);
```

### Continuous Mode
```c
// Configure for continuous mode
ADS1115_Init(&hads1115, ADS1115_MODE_CONTINUOUS_MASK, 
             AIN0, ADS1115_PGA_4_096V_MASK, ADS1115_DR_860SPS_MASK);

// Continuously read data
while(1) {
    ADS1115_ReadConversionReg(&hads1115);
    // Process data
    HAL_Delay(10);
}
```

### Multi-Channel Reading
```c
float read_channel(ADS1115_Handle_t* hads, sChannel_t channel) {
    ADS1115_SetChannel(hads, channel);
    ADS1115_StartSSConv(hads);
    HAL_Delay(10);
    ADS1115_ReadConversionReg(hads);
    
    int16_t raw = hads->Reg[ADS1115_REG_CONVERSION];
    return (raw * 4.096) / 32768.0;  // Convert to voltage
}

// Read all channels
float ch0 = read_channel(&hads1115, AIN0);
float ch1 = read_channel(&hads1115, AIN1);
float ch2 = read_channel(&hads1115, AIN2);
float ch3 = read_channel(&hads1115, Ain3);
```

### Comparator Setup
```c
// Set up comparator with thresholds
ADS1115_SetThresholds(&hads1115, 0x8000, 0x7FFF);  // Low and high thresholds
ADS1115_Comp_Init(&hads1115, 
                  ADS1115_COMP_MODE_TRAD_MASK,      // Traditional mode
                  ADS1115_COMP_POL_ACTIVE_LOW_MASK,  // Active low
                  ADS1115_COMP_LAT_NON_LATCHING_MASK, // Non-latching
                  ADS1115_COMP_QUE_1_MASK);          // Assert after 1 conversion
```

## I2C Addresses

The ADS1115 supports four I2C addresses based on the ADDR pin connection:

| ADDR Pin | 7-bit Address | 8-bit Address (for HAL) |
|----------|---------------|-------------------------|
| GND | 0x48 | 0x90 |
| VDD | 0x49 | 0x92 |
| SDA | 0x4A | 0x94 |
| SCL | 0x4B | 0x96 |

## Troubleshooting

### Common Issues

1. **No I2C Communication**
   - Check wiring and pull-up resistors
   - Verify I2C address matches ADDR pin configuration
   - Ensure I2C peripheral is properly initialized

2. **Incorrect Readings**
   - Verify PGA setting matches input voltage range
   - Check for proper grounding
   - Ensure adequate conversion time in single-shot mode

3. **DMA Issues**
   - Confirm DMA is configured for I2C peripheral
   - Check DMA interrupt priorities
   - Ensure proper callback handling

### Debugging Tips

- Use an oscilloscope to verify I2C signals
- Check return values from all function calls
- Monitor the ALERT/RDY pin for conversion completion
- Use continuous mode for faster debugging

## Author

**Yair Yamin**  
Date: 17.09.25

## References

- [ADS1115 Datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf)
- [STM32 HAL Documentation](https://www.st.com/en/embedded-software/stm32cube-mcu-mpu-packages.html)

## License

This driver is provided as-is for educational and commercial use. Please refer to your project's license terms.