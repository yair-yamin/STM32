# DS3231 STM32 HAL Driver

## Overview
This repository provides a driver implementation for the **DS3231 Real-Time Clock (RTC)** module, designed for **STM32 microcontrollers** using the **HAL library**.  
The driver supports complete time and date management, alarms, square wave output, and temperature sensing.

Reference Datasheet: [DS3231 RTC Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf)

---

## Features
- **Time and Date Management**
  - Set and get current time (hours, minutes, seconds)
  - Set and get date (day, month, year)
  - Day of week management

- **Alarms**
  - Configure **Alarm 1** and **Alarm 2** with multiple modes:
    - Once per second
    - Every minute / hour / day / week
  - Read current alarm configurations
  - Clear alarm flags

- **Square Wave / PWM Output**
  - Configure SQW pin to output fixed frequency signals:
    - 1 Hz, 1.024 kHz, 4.096 kHz, 8.192 kHz

- **Temperature Sensor**
  - Read on-chip temperature sensor
  - Resolution: **0.25 °C**

---

## File Structure
- **DS3231.h** – Header file with register definitions, structures, enums, and API prototypes【8†source】
- **DS3231.c** – Driver implementation for STM32 HAL【9†source】

---

## Usage

### 1. Include the Driver
Copy `DS3231.c` and `DS3231.h` into your STM32 project and include in your code:
```c
#include "DS3231.h"
```

### 2. Initialize Handle
```c
DS3231_Handle_t rtc;
rtc.i2c_handle = &hi2c1;       // Your I2C handle
rtc.I2C_address = 0xD0;        // DS3231 I2C address (shifted)

```

### 3. Set Time and Date
```c
rtc.time.hours = 12;
rtc.time.minutes = 30;
rtc.time.seconds = 0;

rtc.date.date = 21;
rtc.date.month = 8;
rtc.date.year = 25;  // 2025

rtc.dayofweek = Sunday;

DS3231_Init(&rtc);
```

### 4. Get Time and Date
```c
DS3231_GetTime(&rtc);
DS3231_GetDate(&rtc);
printf("Time: %02d:%02d:%02d\n", rtc.time.hours, rtc.time.minutes, rtc.time.seconds);
printf("Date: %02d/%02d/20%02d\n", rtc.date.date, rtc.date.month, rtc.date.year);
```

### 5. Configure Alarm
```c
rtc.alarm1.hours = 6;
rtc.alarm1.minutes = 30;
rtc.alarm1.seconds = 0;
rtc.alarm1.date = 22;
DS3231_SetAlarm1(Once, &rtc);
```

### 6. Read Temperature
```c
DS3231_GetTemp(&rtc);
printf("Temperature: %.2f C\n", rtc.temp);
```

---

## Dependencies
- STM32 HAL Library (I2C)
- Standard `main.h` project header

---

## Author
- **Yair Yamin** – Initial Implementation (21-08-2025)

---

## License
This project is provided as-is without any warranty.  
You are free to use and modify it in your projects.
