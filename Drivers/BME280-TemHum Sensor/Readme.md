# BME280 STM32 HAL Driver


Driver for the **Bosch BME280** environmental sensor (temperature, pressure, humidity) implemented on **STM32 HAL** with **DMA-based I²C communication**.

---

## 🌍 Overview
The **BME280** integrates:
- 🌡️ Temperature sensor (±1 °C accuracy)  
- ⬇️ Pressure sensor (±1 hPa accuracy, 300–1100 hPa range)  
- 💧 Humidity sensor (±3% RH accuracy)  

This driver supports:
- Initialization and device ID check  
- Automatic loading of **calibration parameters**  
- Oversampling & IIR filtering  
- DMA-based data transfers for efficiency  
- Full floating-point compensated values  

---

## 📂 File Structure
```
├── BME280.c   # Driver implementation
├── BME280.h   # Register definitions, structs, prototypes
```

---

## ⚡ Installation

1. Copy `BME280.c` and `BME280.h` into your STM32 project `Core/Src` and `Core/Inc` folders.
2. Enable **I²C with DMA** in STM32CubeMX (or configure manually).  
3. Include the driver in your code:
   ```c
   #include "BME280.h"
   ```

---

## 🔧 Dependencies
- STM32 HAL (`stm32fxxx_hal.h`, `stm32fxxx_hal_i2c.h`)  
- DMA support enabled for I²C  
- `main.h`  

---

## 🛠️ Usage Example
```c
#include "BME280.h"

BME280_Handle_t hbme280;

void BME280_UserInit(void) {
    hbme280.i2c_handle = &hi2c1;     // Use configured I²C handle
    hbme280.I2C_address = 0x76 << 1; // Use 0x76 or 0x77 depending on wiring

    if (BME280_Init(&hbme280) == HAL_OK) {
        BME280_SetOSVals(&hbme280, BME280_MODE_NORMAL,
                         BME280_OS_TEMP_x2,
                         BME280_OS_PRESS_x16,
                         BME280_OS_HUM_x1);

        BME280_SetConfig(&hbme280, BME280_STANDBY_125MS, BME280_FILTER_x4);
    }
}

void BME280_ReadData(void) {
    BME280_GetTemp(&hbme280);
    BME280_GetPress(&hbme280);
    BME280_GetHum(&hbme280);

    printf("Temp: %.2f °C, Press: %.2f Pa, Hum: %.2f %%\n",
           hbme280.temperature, hbme280.pressure, hbme280.humidity);
}
```

---

## 📖 API Reference

### Initialization
- `HAL_StatusTypeDef BME280_Init(BME280_Handle_t* hbme280)`  
  Initializes sensor, checks device ID, and loads calibration data.  

### Measurement
- `HAL_StatusTypeDef BME280_GetTemp(BME280_Handle_t* hbme280)`  
- `HAL_StatusTypeDef BME280_GetPress(BME280_Handle_t* hbme280)`  
- `HAL_StatusTypeDef BME280_GetHum(BME280_Handle_t* hbme280)`  

### Configuration
- `HAL_StatusTypeDef BME280_SetOSVals(BME280_Handle_t* hbme280, uint8_t mode, uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h)`  
- `HAL_StatusTypeDef BME280_SetConfig(BME280_Handle_t* hbme280, uint8_t t_sb, uint8_t filter)`  

### Compensation (internal use, but exposed)
- `BME280_S32_t  BME280_compensate_T_int32(...)`  
- `BME280_U32_t  BME280_compensate_P_int64(...)`  
- `BME280_U32_t  BME280_compensate_H_int32(...)`  


---

## 🚀 Future Work
- [ ] Add **SPI interface** support  
- [ ] Blocking and interrupt-based I²C options  
- [ ] Auto measurement cycle with polling  
- [ ] Example projects for Nucleo/Discovery boards  

---
## References

- [BME280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)
- [STM32 HAL Documentation](https://www.st.com/en/embedded-software/stm32cube-mcu-mpu-packages.html)

---
## Author

**Yair Yamin**  
Date: 17.09.25

---

## 📜 License
Released under the **MIT License**. See [LICENSE](LICENSE) for details.
