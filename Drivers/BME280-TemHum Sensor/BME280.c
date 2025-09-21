#include "BME280.h"
/**
 ******************************************************************************
 * @file    BME280.h
 * @author  Yair Yamin
 * @date    17.09.25
 * @brief   Header file for BME280 driver for STM32 HAL
 * @see     https://www.ti.com/lit/ds/symlink/ads1115.pdf
 * @details This header file provides definitions, structures, and function
 *          declarations for interfacing with the BME280 temperature, humidity, and
 *          pressure sensor and reading data from the
 *          I2C using STM32 HAL library with DMA support.
 *
 * Key Features:
 * - Temperature, pressure, and humidity sensing in a single package
 * - Temperature: ±1°C accuracy, pressure: ±1 hPa absolute accuracy
 * - Humidity: ±3% relative humidity accuracy
 * - Pressure range: 300-1100 hPa
 * - Multiple power modes and oversampling settings
 * - I2C and SPI digital interfaces (I2C implementation in this driver)
 *
 ******************************************************************************
 */

/* ========================== Global Variables ============================ */
static BME280_S32_t t_fine;

 /* ========================== Macros ============================ */
static inline uint16_t u16(uint8_t lsb, uint8_t msb) { return (uint16_t)lsb | ((uint16_t)msb << 8); }
static inline  int16_t s16(uint8_t lsb, uint8_t msb) { return (int16_t) u16(lsb, msb); }

 /* ========================== Function Definitions ============================ */

/**
 * @brief Calculate raw temperature reading using calibration parameters
 * @param adc_T Raw temperature reading from sensor
 * @param comp Structure containing compensation/calibration parameters
 * @return BME280_S32_t Raw temperature value in DegC with resolution of 0.01 DegC
 */
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T,BME280_Compensations_t comp)
{
    BME280_S32_t var1, var2, T;
    var1  = ((((adc_T >> 3) - ((BME280_S32_t)comp.dig_T1 << 1))) * ((BME280_S32_t)comp.dig_T2)) >> 11;
    var2  = (((((adc_T >> 4) - ((BME280_S32_t)comp.dig_T1)) *
              ((adc_T >> 4) - ((BME280_S32_t)comp.dig_T1))) >> 12) *
              ((BME280_S32_t)comp.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T  = (t_fine * 5 + 128) >> 8;
    return T;
}

/**
 * @brief Calculate raw pressure reading using calibration parameters
 * @param adc_P Raw pressure reading from sensor
 * @param comp Structure containing compensation/calibration parameters
 * @return BME280_U32_t Raw pressure value in Pa with resolution of 1/256 Pa
 */
BME280_U32_t BME280_compensate_P_int64(BME280_S32_t adc_P,BME280_Compensations_t comp)
{
    BME280_S64_t var1, var2, p;
    var1 = ((BME280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BME280_S64_t)comp.dig_P6;
    var2 = var2 + ((var1 * (BME280_S64_t)comp.dig_P5) << 17);
    var2 = var2 + (((BME280_S64_t)comp.dig_P4) << 35);
    var1 = ((var1 * var1 * (BME280_S64_t)comp.dig_P3) >> 8) + ((var1 * (BME280_S64_t)comp.dig_P2) << 12);
    var1 = (((((BME280_S64_t)1) << 47) + var1)) * ((BME280_S64_t)comp.dig_P1) >> 33;
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((BME280_S64_t)comp.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((BME280_S64_t)comp.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)comp.dig_P7) << 4);
    return (BME280_U32_t)p;
}

/**
 * @brief Calculate raw humidity reading using calibration parameters
 * @param adc_H Raw humidity reading from sensor
 * @param comp Structure containing compensation/calibration parameters
 * @return BME280_U32_t Raw humidity value in %RH with resolution of 1/1024 %RH
 */
BME280_U32_t BME280_compensate_H_int32(BME280_S32_t adc_H,BME280_Compensations_t comp)
{
    BME280_S32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((BME280_S32_t)76800));

    v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)comp.dig_H4) << 20) -
                    (((BME280_S32_t)comp.dig_H5) * v_x1_u32r)) + 
                   ((BME280_S32_t)16384)) >> 15) *
                 (((((((v_x1_u32r * ((BME280_S32_t)comp.dig_H6)) >> 10) *
                      (((v_x1_u32r * ((BME280_S32_t)comp.dig_H3)) >> 11) +
                       ((BME280_S32_t)32768))) >> 10) +
                    ((BME280_S32_t)2097152)) * ((BME280_S32_t)comp.dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r -
                 (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                   ((BME280_S32_t)comp.dig_H1)) >> 4));

    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (BME280_U32_t)(v_x1_u32r >> 12);
}

/**
 * @brief Initialize the BME280 sensor
 * @param hbme280 Pointer to BME280 handle structure
 * @return HAL_StatusTypeDef HAL_OK if initialization successful, HAL_ERROR if device ID mismatch
 */
HAL_StatusTypeDef BME280_Init(BME280_Handle_t* hbme280)
{
    HAL_I2C_Mem_Read_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_ID_REG,I2C_MEMADD_SIZE_8BIT,&hbme280->Reg.id_reg,1);
    if(hbme280->Reg.id_reg != 0x60) return HAL_ERROR;

    return BME280_CalCompensationParams(hbme280);
}

/**
 * @brief Calculate compensation parameters from sensor calibration data
 * @param hbme280 Pointer to BME280 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef BME280_CalCompensationParams(BME280_Handle_t* hbme280)
{
    HAL_StatusTypeDef status;
    uint8_t  calibA[26] = {0};
    uint8_t  calibB[7] = {0};
    

    status = HAL_I2C_Mem_Read_DMA(hbme280->i2c_handle,hbme280->I2C_address,0x88,I2C_MEMADD_SIZE_8BIT,calibA,26);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Mem_Read_DMA(hbme280->i2c_handle,hbme280->I2C_address,0xE1,I2C_MEMADD_SIZE_8BIT,calibB,7);
    if(status != HAL_OK) return status;
    
    /* Temperature coefficients (Table 16) */
    uint16_t dig_T1 = u16(calibA[0],  calibA[1]);   // 0x88 / 0x89  (unsigned)
    int16_t dig_T2 = s16(calibA[2],  calibA[3]);   // 0x8A / 0x8B  (signed)
    int16_t dig_T3 = s16(calibA[4],  calibA[5]);   // 0x8C / 0x8D  (signed)

    /* Pressure coefficients (Table 16) */
    hbme280->Comp.dig_P1 = u16(calibA[6],  calibA[7]);   // 0x8E / 0x8F  (unsigned)
    hbme280->Comp.dig_P2 = s16(calibA[8],  calibA[9]);   // 0x90 / 0x91  (signed)
    hbme280->Comp.dig_P3 = s16(calibA[10], calibA[11]);  // 0x92 / 0x93  (signed)
    hbme280->Comp.dig_P4 = s16(calibA[12], calibA[13]);  // 0x94 / 0x95  (signed)
    hbme280->Comp.dig_P5 = s16(calibA[14], calibA[15]);  // 0x96 / 0x97  (signed)
    hbme280->Comp.dig_P6 = s16(calibA[16], calibA[17]);  // 0x98 / 0x99  (signed)
    hbme280->Comp.dig_P7 = s16(calibA[18], calibA[19]);  // 0x9A / 0x9B  (signed)
    hbme280->Comp.dig_P8 = s16(calibA[20], calibA[21]);  // 0x9C / 0x9D  (signed)
    hbme280->Comp.dig_P9 = s16(calibA[22], calibA[23]);  // 0x9E / 0x9F  (signed)

    /* Humidity coefficients (Table 16 + notes on H4/H5 packing) */
    hbme280->Comp.dig_H1 = calibA[25];                   // 0xA1 (unsigned 8-bit)
    hbme280->Comp.dig_H2 = s16(calibB[0],  calibB[1]);   // 0xE1 / 0xE2  (signed)
    hbme280->Comp.dig_H3 = calibB[2];                    // 0xE3 (unsigned 8-bit)

    /* H4: [11:4] in 0xE4, [3:0] in 0xE5[3:0]  → sign-extended 12-bit => int16_t */
    hbme280->Comp.dig_H4 = (int16_t)(( (int16_t)calibB[3] << 4) | (calibB[4] & 0x0F));
    /* H5: [3:0] in 0xE5[7:4], [11:4] in 0xE6   → sign-extended 12-bit => int16_t */
    hbme280->Comp.dig_H5 = (int16_t)(( (int16_t)calibB[5] << 4) | (calibB[4] >> 4));
    hbme280->Comp.dig_H6 = (int8_t)calibB[6];           // 0xE7 (signed 8-bit)

    return HAL_OK;
}


/**
 * @brief Read and calculate the temperature from BME280
 * @param hbme280 Pointer to BME280 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef BME280_GetTemp(BME280_Handle_t* hbme280)
{
    HAL_StatusTypeDef status;
    BME280_S32_t adc_T;

    status = HAL_I2C_Mem_Read_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_TEMP_MSB_REG,I2C_MEMADD_SIZE_8BIT,hbme280->Reg.temp_msb_reg,3);
    if(status != HAL_OK) return status;

    adc_T = (BME280_S32_t)(((uint32_t)(hbme280->Reg.temp_msb_reg) << 12) | ((uint32_t)(hbme280->Reg.temp_lsb_reg) << 4) | ((uint32_t)hbme280->Reg.temp_xlsb_reg >> 4));
    hbme280->temperature = (float)BME280_compensate_T_int32(adc_T,hbme280->Comp) / 100.0f;
    return HAL_OK;
}

/**
 * @brief Read and calculate the pressure from BME280
 * @param hbme280 Pointer to BME280 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef BME280_GetPress(BME280_Handle_t* hbme280)
{
    HAL_StatusTypeDef status;
    BME280_S32_t adc_P;

    status = HAL_I2C_Mem_Read_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_PRESS_MSB_REG,I2C_MEMADD_SIZE_8BIT,hbme280->Reg.press_msb_reg,3);
    if(status != HAL_OK) return status;

    adc_P = (BME280_S32_t)(((uint32_t)(hbme280->Reg.press_msb_reg) << 12) | ((uint32_t)(hbme280->Reg.press_lsb_reg) << 4) | ((uint32_t)hbme280->Reg.press_xlsb_reg >> 4));
    hbme280->pressure = (float)BME280_compensate_P_int64(adc_P,hbme280->Comp) / 256.0f;
    return HAL_OK;
}

/**
 * @brief Read and calculate the compensated humidity from BME280
 * @param hbme280 Pointer to BME280 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef BME280_GetHum(BME280_Handle_t* hbme280)
{
    HAL_StatusTypeDef status;
    BME280_S32_t adc_H;

    status = HAL_I2C_Mem_Read_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_HUM_MSB_REG,I2C_MEMADD_SIZE_8BIT,hbme280->Reg.hum_msb_reg,2);
    if(status != HAL_OK) return status;

    adc_H = (BME280_S32_t)(((uint32_t)(hbme280->Reg.hum_msb_reg) << 8) | ((uint32_t)hbme280->Reg.hum_lsb_reg));
    hbme280->humidity = (float)BME280_compensate_H_int32(adc_H,hbme280->Comp) / 1024.0f;
    return HAL_OK;
}

/**
 * @brief Set oversampling values and mode for BME280
 * @param hbme280 Pointer to BME280 handle structure
 * @param mode Operating mode (sleep, forced, or normal)
 * @param osrs_t Temperature oversampling setting
 * @param osrs_p Pressure oversampling setting
 * @param osrs_h Humidity oversampling setting
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef BME280_SetOSVals(BME280_Handle_t* hbme280,uint8_t mode ,uint8_t osrs_t,uint8_t osrs_p,uint8_t osrs_h)
{
    HAL_StatusTypeDef status;
    hbme280->Reg.ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
    hbme280->Reg.ctrl_hum_reg = osrs_h;

    status = HAL_I2C_Mem_Write_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_CTRL_HUM_REG,I2C_MEMADD_SIZE_8BIT,&hbme280->Reg.ctrl_hum_reg,1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Mem_Write_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_CTRL_MEAS_REG,I2C_MEMADD_SIZE_8BIT,&hbme280->Reg.ctrl_meas_reg,1);
    if(status != HAL_OK) return status;

    return HAL_OK;
}

/**
 * @brief Configure BME280 standby time and filter settings
 * @param hbme280 Pointer to BME280 handle structure
 * @param t_sb Standby time between measurements in normal mode
 * @param filter IIR filter coefficient
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef BME280_SetConfig(BME280_Handle_t* hbme280,uint8_t t_sb,uint8_t filter)
{
    HAL_StatusTypeDef status;
    hbme280->Reg.config_reg = (t_sb << 5) | (filter << 2);

    status = HAL_I2C_Mem_Write_DMA(hbme280->i2c_handle,hbme280->I2C_address,BME280_CONFIG_REG,I2C_MEMADD_SIZE_8BIT,&hbme280->Reg.config_reg,1);
    if(status != HAL_OK) return status;

    return HAL_OK;
}

