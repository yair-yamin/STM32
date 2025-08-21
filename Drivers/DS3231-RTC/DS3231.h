#ifndef DS3231_H
#define DS3231_H
#include "main.h"



/*------------------- Regester Address Defines ---------------------------*/
#define DS3231_REG_SECONDS 0x00
#define DS3231_REG_MINUTES 0x01
#define DS3231_REG_HOURS   0x02
#define DS3231_REG_DAY     0x03
#define DS3231_REG_DATE    0x04
#define DS3231_REG_MONTH   0x05
#define DS3231_REG_YEAR    0x06
#define DS3231_REG_ALARM1_SECONDS 0x07
#define DS3231_REG_ALARM1_MINUTES 0x08
#define DS3231_REG_ALARM1_HOURS   0x09
#define DS3231_REG_ALARM1_DAYDATE 0x0A
#define DS3231_REG_ALARM2_MINUTES 0x0B
#define DS3231_REG_ALARM2_HOURS   0x0C
#define DS3231_REG_ALARM2_DAYDATE 0x0D
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS 0x0F
#define DS3231_REG_AGING 0x10
#define DS3231_REG_TEMP_MSB 0x11
#define DS3231_REG_TEMP_LSB 0x12

/************************ Bit Mask defines ********************************/
#define ALRAM_DAY_MASK 0b01000000 // Mask for day of the week in Alarm registers
#define INTR_MODE_MASK 0b00000100 // Interrupt mode for SQW pin  
#define ALARM1_MASK 0b00000001 // Alarm 1 Interrupt enable 
#define ALARM2_MASK 0b00000010 // Alarm 2 Interrupt enable
#define PWM_MODE_MASK 0b00000100 // PWM mode for SQW pin

/************************ Driver Structs ********************************/
typedef struct {
    uint8_t hours;    // 0-23
    uint8_t minutes;  // 0-59
    uint8_t seconds;  // 0-59
}ds3231_time_t;

typedef struct {
    uint8_t date;  // 1-31
    uint8_t month; // 1-12
    uint8_t year;  // 0-99 (0=2000, 1=2001, ..., 99=2099)
}ds3231_data_t;

typedef enum {
    Sunday = 1,
    Monday = 2,
    Tuesday = 3,
    Wednesday = 4,
    Thursday = 5,
    Friday = 6,
    Saturday = 7
}DOW_t;

typedef struct {
    uint8_t hours;    // 0-23
    uint8_t minutes;  // 0-59
    uint8_t seconds;  // 0-59
    DOW_t dayOfWeek;  // Day of the week
    uint8_t date;     // Date
}sAlram_t;

typedef enum {
    EveryMinute = 1,
    EveryHour = 2,
    EveryDay = 3,
    Once = 4,
    EveryWeek = 5,
}sMode_t;

typedef struct {
    I2C_HandleTypeDef* i2c_handle;
    uint8_t I2C_address;
    ds3231_time_t time;
    ds3231_data_t date;
    DOW_t dayOfWeek;
    uint8_t Reg[19]; // Register buffer
    sAlram_t alarm1;
    sAlram_t alarm2;
    float temp;
} DS3231_Handle_t;

/*------------------- Function Prototypes ---------------------------*/
HAL_StatusTypeDef DS3231_Init(DS3231_Handle_t *handle) ;
HAL_StatusTypeDef DS3231_SetTime( DS3231_Handle_t *handle); 
HAL_StatusTypeDef DS3231_GetTime(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_SetDOW(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_GetDOW( DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_SetDate( DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_GetDate(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_SetAlarm1(sMode_t mode,DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_SetAlarm2(sMode_t mode ,DS3231_Handle_t *handle) ;
HAL_StatusTypeDef DS3231_GetAlarm1(DS3231_Handle_t *handle) ;
HAL_StatusTypeDef DS3231_GetAlarm2(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_GetTemp(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_GetControlRegister(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_ReadStatus(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_WriteStatus(DS3231_Handle_t *handle);
HAL_StatusTypeDef DS3231_OutputPWM( DS3231_Handle_t *handle,uint8_t RS2,uint8_t RS1) ;
HAL_StatusTypeDef DS3231_CLearAlarmsFlags(DS3231_Handle_t *handle);

/*------------------- Macros ---------------------------*/
#define VALID(x) if((x) != HAL_OK) { return HAL_ERROR; }
#define decToBCD(value) (((value / 10) << 4) | (value % 10))
#define BCDToDec(value) (((value >> 4) * 10) + (value & 0x0F))

#endif