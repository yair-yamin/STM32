#include "DS3231.h"

/**
 ******************************************************************************
 * @file    DS3231.c
 * @author  Yair Yamin
 * @date    21-08-2025
 * @brief   Driver for the DS3231 Real-Time Clock (RTC) module.
 * @see     https://www.analog.com/media/en/technical-documentation/data-sheets/ds3231.pdf
 * @details This driver, designed for STM32 microcontrollers using the HAL
 * library, provides a comprehensive set of functions to manage the
 * time, date, alarms, and other features of the DS3231 RTC.
 *
 * Key Features:
 * - Time and Date Management: Allows you to easily set and retrieve
 * the current time (hours, minutes, seconds), date (day, month,
 * year), and day of the week. It correctly handles the
 * Binary-Coded Decimal (BCD) format required by the DS3231.
 *
 * - Configurable Alarms: A standout feature is its support for both
 * Alarm 1 and Alarm 2. You can configure these alarms with various
 * trigger modes, including once per second, or matching by minute,
 * hour, date, or day.
 *
 * - Square Wave/PWM Output: The driver includes a function to
 * configure the SQW pin. This allows the DS3231 to output a
 * square wave at one of several fixed frequencies (1Hz, 1.024kHz,
 * 4.096kHz, or 8.192kHz).
 *
 * - Temperature Sensor: It also includes a function to read the
 * on-chip temperature sensor, providing ambient temperature
 * readings with a resolution of 0.25°C.
 ******************************************************************************
 */

/* =============================== Global Variables =============================== */


/* ========================== Function Definitions ============================ */

HAL_StatusTypeDef DS3231_Init(DS3231_Handle_t *handle) 
{

    VALID(DS3231_SetTime(handle));
    VALID(DS3231_SetDate(handle));
    VALID(DS3231_SetDOW(handle));
    return HAL_OK;
}

/** Functionality: Set operations for time, day, date, and alarms (Alarm1, Alarm2) **/


HAL_StatusTypeDef DS3231_SetTime( DS3231_Handle_t *handle)
{
    HAL_StatusTypeDef status;
    ds3231_time_t *time = &handle->time;
    uint8_t *DS3231_Reg = handle->Reg;

    // Validate the input time values
    if(time->hours > 23 || time->minutes > 59 || time->seconds > 59) {
        return HAL_ERROR ;
    }
    
    // Convert BCD format
    DS3231_Reg[DS3231_REG_SECONDS] = decToBCD(time->seconds);
    DS3231_Reg[DS3231_REG_MINUTES] = decToBCD(time->minutes);
    DS3231_Reg[DS3231_REG_HOURS] = decToBCD(time->hours);
    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_SECONDS, I2C_MEMADD_SIZE_8BIT, DS3231_Reg, 3, HAL_MAX_DELAY);

    return status;
}


HAL_StatusTypeDef DS3231_SetDate( DS3231_Handle_t *handle) {
    HAL_StatusTypeDef status;
    ds3231_data_t *date = &handle->date;
    uint8_t *DS3231_Reg = handle->Reg;
    // Validate the input date values
    if(date->date > 31 || date->month < 1 || date->month > 12 || date->year > 99) {
        return HAL_ERROR;
    }
    
    // Convert BCD format
    DS3231_Reg[DS3231_REG_DATE] = decToBCD(date->date);
    DS3231_Reg[DS3231_REG_MONTH] =  decToBCD(date->month);
    DS3231_Reg[DS3231_REG_YEAR] =decToBCD(date->year);

    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_DATE, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_DATE, 3, HAL_MAX_DELAY);
    return status;
}


HAL_StatusTypeDef DS3231_SetDOW(DS3231_Handle_t *handle)
{
    HAL_StatusTypeDef status;
    uint8_t *DS3231_Reg = handle->Reg;
    DS3231_Reg[DS3231_REG_DAY] = handle->dayOfWeek;
    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_DAY, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_DAY, 1, HAL_MAX_DELAY);
    return status;

}


HAL_StatusTypeDef DS3231_SetAlarm1(sMode_t mode,DS3231_Handle_t *handle) {

    uint8_t A1M1, A1M2, A1M3, A1M4;
    uint8_t temp = 0;
    HAL_StatusTypeDef status;
    sAlram_t *alarm = &handle->alarm1;
    uint8_t *DS3231_Reg = handle->Reg;
    // Validate the input alarm values
    if(alarm->hours > 23 || alarm->minutes > 59 || alarm->seconds > 59 || (alarm->dayOfWeek < Sunday || alarm->dayOfWeek > Saturday) || alarm->date < 1 || alarm->date > 31) {
        return HAL_ERROR;
    }

    switch (mode)
    {

    case EveryMinute:
        A1M1 = 0b00000000;
        A1M2 = 0b10000000;  
        A1M3 = 0b10000000;
        A1M4 = 0b10000000;

        DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] = A1M4 ;

    break;
    case EveryHour:
        A1M1 = 0b00000000;
        A1M2 = 0b00000000;
        A1M3 = 0b10000000;
        A1M4 = 0b10000000;
        DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] = A1M4 ;
    break;
    case EveryDay:
        A1M1 = 0b00000000;
        A1M2 = 0b00000000;  
        A1M3 = 0b00000000;
        A1M4 = 0b10000000;

        DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] = A1M4 ;
    break;
    case Once:
        A1M1 = 0b00000000;
        A1M2 = 0b00000000;
        A1M3 = 0b00000000;
        A1M4 = 0b00000000;
        temp = (alarm->date / 10) << 4;
        DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] = A1M4 | temp | (alarm->date % 10); // Set the alarm to trigger on a specific date
    break;
    case EveryWeek:
        A1M1 = 0b00000000;
        A1M2 = 0b00000000;  
        A1M3 = 0b00000000;
        A1M4 = 0b00000000;
        DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] = A1M4 | ALRAM_DAY_MASK | alarm->dayOfWeek; //Set the Alram to trigger on a specific day of the week
    break;
    default:
        break;
    }

    // Convert BCD format
    DS3231_Reg[DS3231_REG_ALARM1_SECONDS] = A1M1 | decToBCD(alarm->seconds);
    DS3231_Reg[DS3231_REG_ALARM1_MINUTES] = A1M2 | decToBCD(alarm->minutes);
    DS3231_Reg[DS3231_REG_ALARM1_HOURS] = A1M3 | decToBCD(alarm->hours);

    // Clear the A1F (Alarm 1 Flag) before setting the alarm
    DS3231_CLearAlarmsFlags(handle);
    
    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_ALARM1_SECONDS, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_ALARM1_SECONDS, 4, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    DS3231_Reg[DS3231_REG_CONTROL] =  INTR_MODE_MASK |ALARM1_MASK ; // Enable Alarm 1 Interrupt

    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_CONTROL, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_CONTROL, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    return HAL_OK;

}


HAL_StatusTypeDef DS3231_SetAlarm2(sMode_t mode ,DS3231_Handle_t *handle) 
{
    uint8_t  A2M2, A2M3, A2M4;
    uint8_t temp = 0;
    HAL_StatusTypeDef status;
    sAlram_t *alarm = &handle->alarm2;
    uint8_t *DS3231_Reg = handle->Reg;
    // Validate the input alarm values
    if(alarm->hours > 23 || alarm->minutes > 59 || (alarm->dayOfWeek < Sunday || alarm->dayOfWeek > Saturday) || alarm->date < 1 || alarm->date > 31) {
        return HAL_ERROR;
    }
    switch (mode)
    {

    case EveryMinute:
       
        A2M2 = 0b10000000;
        A2M3 = 0b10000000;
        A2M4 = 0b10000000;
        DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] = A2M4;
    break;
    case EveryHour:
        A2M2 = 0b00000000;
        A2M3 = 0b10000000;
        A2M4 = 0b10000000;
        DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] = A2M4;
    break;
    case EveryDay:
        
        A2M2 = 0b00000000;
        A2M3 = 0b00000000;
        A2M4 = 0b10000000;
        DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] = A2M4;
    break;
    case Once:
       
        A2M2 = 0b00000000;
        A2M3 = 0b00000000;
        A2M4 = 0b00000000;
        temp = (alarm->date / 10) << 4;
        DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] = A2M4 | temp | (alarm->date % 10);
    break;
    case EveryWeek:
        
        A2M2 = 0b00000000;
        A2M3 = 0b00000000;
        A2M4 = 0b00000000;
        DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] = A2M4 | ALRAM_DAY_MASK | alarm->dayOfWeek; //Set the Alram to trigger on a specific day of the week
    break;
    default:
        return HAL_ERROR;
    }

    // Convert BCD format
    DS3231_Reg[DS3231_REG_ALARM2_MINUTES] = A2M2 | decToBCD(alarm->minutes);
    DS3231_Reg[DS3231_REG_ALARM2_HOURS] = A2M3 | decToBCD(alarm->hours) ;

    // Clear the A2F (Alarm 2 Flag) before setting the alarm
    DS3231_CLearAlarmsFlags(handle);
    
    DS3231_Reg[DS3231_REG_CONTROL] |= INTR_MODE_MASK | ALARM2_MASK; // Enable Alarm 2 & Interrupt mode
    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_CONTROL, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_CONTROL, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }

    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_ALARM2_MINUTES, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_ALARM2_MINUTES, 3, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    return HAL_OK;
}


/** Functionality: Get operations for time, day, date, and alarms (Alarm1, Alarm2) **/


HAL_StatusTypeDef DS3231_GetTime(DS3231_Handle_t *handle) {
    HAL_StatusTypeDef status;
    ds3231_time_t *time = &handle->time;
    uint8_t *DS3231_Reg = handle->Reg;
    status = HAL_I2C_Mem_Read(handle->i2c_handle,handle->I2C_address, DS3231_REG_SECONDS, I2C_MEMADD_SIZE_8BIT, DS3231_Reg, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        time->seconds = BCDToDec(DS3231_Reg[DS3231_REG_SECONDS]);
        time ->minutes = BCDToDec(DS3231_Reg[DS3231_REG_MINUTES]);
        time ->hours = BCDToDec(DS3231_Reg[DS3231_REG_HOURS]);
        return HAL_OK;
    }
    return status;
}


HAL_StatusTypeDef DS3231_GetDate(DS3231_Handle_t *handle){
    HAL_StatusTypeDef status;
    ds3231_data_t *date = &handle->date;
    uint8_t *DS3231_Reg = handle->Reg;
    status = HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_DATE, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_DATE, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        date->date = BCDToDec(DS3231_Reg[DS3231_REG_DATE]);
        date->month = BCDToDec(DS3231_Reg[DS3231_REG_MONTH]);
        date->year = BCDToDec(DS3231_Reg[DS3231_REG_YEAR]);
        return HAL_OK;
    }
    return status;
}


HAL_StatusTypeDef DS3231_GetControlRegister(DS3231_Handle_t *handle) {
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_CONTROL, I2C_MEMADD_SIZE_8BIT, &handle->Reg[DS3231_REG_CONTROL], 1, HAL_MAX_DELAY);
    return status;
}


HAL_StatusTypeDef DS3231_GetDOW( DS3231_Handle_t *handle) {
    HAL_StatusTypeDef status;
    DOW_t *dayOfWeek = &handle->dayOfWeek;
    uint8_t *DS3231_Reg = handle->Reg;
    status = HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_DAY, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_DAY, 1, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        *dayOfWeek = (DOW_t)(DS3231_Reg[DS3231_REG_DAY]);
        return HAL_OK;
    }
    return status;
}


HAL_StatusTypeDef DS3231_GetTemp(DS3231_Handle_t *handle)
{
    HAL_StatusTypeDef status;
    float *temperature = &handle->temp;
    uint8_t *DS3231_Reg = handle->Reg;
    int16_t raw_value = 0;
    status = HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_TEMP_MSB, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_TEMP_MSB, 2, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        raw_value = (int16_t)(DS3231_Reg[DS3231_REG_TEMP_MSB] << 2) | (DS3231_Reg[DS3231_REG_TEMP_LSB] >> 6);
        *temperature = raw_value / 4.0f;
        return HAL_OK;
    }
    return status;
}


HAL_StatusTypeDef DS3231_GetAlarm1(DS3231_Handle_t *handle) 
{
    HAL_StatusTypeDef status;
    sAlram_t *alarm = &handle->alarm1;
    uint8_t *DS3231_Reg = handle->Reg;
    status = HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_ALARM1_SECONDS, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_ALARM1_SECONDS, 4, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        alarm->seconds = (DS3231_Reg[DS3231_REG_ALARM1_SECONDS] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM1_SECONDS] >> 4) * 10);
        alarm->minutes = (DS3231_Reg[DS3231_REG_ALARM1_MINUTES] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM1_MINUTES] >> 4) * 10);
        alarm->hours   = (DS3231_Reg[DS3231_REG_ALARM1_HOURS] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM1_HOURS] >> 4) * 10);
        alarm->dayOfWeek = (DOW_t)(DS3231_Reg[DS3231_REG_ALARM1_DAYDATE]);
        alarm->date = (DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM1_DAYDATE] >> 4) * 10);
    }
    return status;
}


HAL_StatusTypeDef DS3231_GetAlarm2(DS3231_Handle_t *handle) {
    HAL_StatusTypeDef status;
    sAlram_t *alarm = &handle->alarm2;
    uint8_t *DS3231_Reg = handle->Reg;
    status = HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_ALARM2_MINUTES, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_ALARM2_MINUTES, 3, HAL_MAX_DELAY);
    if (status == HAL_OK) {
        alarm->minutes = (DS3231_Reg[DS3231_REG_ALARM2_MINUTES] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM2_MINUTES] >> 4) * 10);
        alarm->hours   = (DS3231_Reg[DS3231_REG_ALARM2_HOURS] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM2_HOURS] >> 4) * 10);
        alarm->dayOfWeek = (DOW_t)(DS3231_Reg[DS3231_REG_ALARM2_DAYDATE]);
        alarm->date = (DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] & 0x0F) + ((DS3231_Reg[DS3231_REG_ALARM2_DAYDATE] >> 4) * 10);
        alarm->seconds = 0; // Alarm B does not use seconds, so set it to 0
        return HAL_OK;
    }
    return status;
}

/** Functionality: Read and write operations for status register and clear alarm flags **/

HAL_StatusTypeDef DS3231_ReadStatus(DS3231_Handle_t *handle) {

    return HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_STATUS,
                            I2C_MEMADD_SIZE_8BIT, &handle->Reg[DS3231_REG_STATUS], 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef DS3231_WriteStatus(DS3231_Handle_t *handle) {
    
    return HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_STATUS,
                             I2C_MEMADD_SIZE_8BIT, &handle->Reg[DS3231_REG_STATUS], 1, HAL_MAX_DELAY);
}


HAL_StatusTypeDef DS3231_CLearAlarmsFlags(DS3231_Handle_t *handle)
{
    uint8_t *st = &handle->Reg[DS3231_REG_STATUS];
        if (HAL_I2C_Mem_Read(handle->i2c_handle, handle->I2C_address, DS3231_REG_STATUS,I2C_MEMADD_SIZE_8BIT, st, 1, HAL_MAX_DELAY) == HAL_OK) {
            *st &= ~(1u<<0);  // clear A1F
            *st &= ~(1u<<1);  // clear A2F
            HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_STATUS,I2C_MEMADD_SIZE_8BIT, st, 1, HAL_MAX_DELAY);
        return HAL_OK;
        }
    return HAL_ERROR;
}


/** Functionality: Configure the square wave output or PWM mode **/
HAL_StatusTypeDef DS3231_OutputPWM( DS3231_Handle_t *handle,uint8_t RS2,uint8_t RS1) 
{
    /******************************************************************************
    * Square-Wave Output Frequency Options (when INTCN bit is 0):
    * ----------------------------------------------------
    * | RS2 | RS1 |   Output Frequency   |
    * |-----|-----|----------------------|
    * |  0  |  0  |         1 Hz         |
    * |  0  |  1  |       1.024 kHz      |
    * |  1  |  0  |       4.096 kHz      |
    * |  1  |  1  |       8.192 kHz      |
    * ----------------------------------------------------
    ******************************************************************************/
    
    HAL_StatusTypeDef status;
    uint8_t *DS3231_Reg = handle->Reg;
    // Validate the input values
    if(RS1 != 0 && RS1 != 1 && RS2 != 0 && RS2 != 1) {
        return HAL_ERROR;
    }

    // Set the control register for PWM output
    DS3231_Reg[DS3231_REG_CONTROL] &= ~(PWM_MODE_MASK); // Clear INTCN bit
    DS3231_Reg[DS3231_REG_CONTROL] = (RS1 << 3) | (RS2 << 4); // Set RS1 and RS2 bits

    status = HAL_I2C_Mem_Write(handle->i2c_handle, handle->I2C_address, DS3231_REG_CONTROL, I2C_MEMADD_SIZE_8BIT, DS3231_Reg + DS3231_REG_CONTROL, 1, HAL_MAX_DELAY);
    return status;
}
