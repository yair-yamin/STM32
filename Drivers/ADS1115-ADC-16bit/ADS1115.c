#include "ADS1115.h"
/**
 ******************************************************************************
 * @file    ADS1115.h
 * @author  Yair Yamin
 * @date    17.09.25
 * @brief   Header file for ADS1115 16-bit ADC driver for STM32 HAL
 * @see     https://www.ti.com/lit/ds/symlink/ads1115.pdf
 * @details This header file provides definitions, structures, and function
 *          declarations for interfacing with the ADS1115 16-bit analog-to-digital
 *          converter via I2C using STM32 HAL library with DMA support.
 *
 * Key Features:
 * - 16-bit resolution ADC with programmable gain amplifier (PGA)
 * - 4 single-ended or 2 differential input channels
 * - Programmable data rate from 8 to 860 samples per second
 * - Single-shot and continuous conversion modes
 * - Built-in programmable comparator with interrupt capability
 *
 ******************************************************************************
 */

 /* ========================== Function Definitions ============================ */

/**
 * @brief Initialize the ADS1115 with specified configuration
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param mode Operating mode (continuous or single-shot)
 * @param channel Input channel selection
 * @param pga Programmable gain amplifier setting
 * @param sampleRate Data rate configuration
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 */
HAL_StatusTypeDef ADS1115_Init(ADS1115_Handle_t* hads1115,uint16_t mode,sChannel_t channel, uint16_t pga, uint16_t sampleRate)
{     
    HAL_StatusTypeDef status;
    memset(hads1115->Reg,0,sizeof(hads1115->Reg)); // Clear register buffer
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    uint16_t channel_config = 0;
    channel_config = (uint16_t)channel << 12;
    hads1115->Reg[ADS1115_REG_CONFIG] = channel_config | pga | mode | sampleRate | ADS1115_COMP_QUE_DISABLE_MASK;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;   
}

/**
 * @brief Read the configuration register of the ADS1115
 * @param hads1115 Pointer to ADS1115 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details This function reads the current configuration of the ADS1115's config register using DMA
 */
HAL_StatusTypeDef ADS1115_ReadConfigReg(ADS1115_Handle_t* hads1115)
{
    HAL_StatusTypeDef status;
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Receive_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Read the conversion register containing ADC result
 * @param hads1115 Pointer to ADS1115 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Reads the latest conversion result from the ADC's conversion register using DMA
 */
HAL_StatusTypeDef ADS1115_ReadConversionReg(ADS1115_Handle_t* hads1115)
{
    HAL_StatusTypeDef status;
    hads1115->ptr_reg = ADS1115_REG_CONVERSION;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Receive_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONVERSION], 2);
    return status;
}

/**
 * @brief Configure the input channel multiplexer
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param channel Input channel selection from sChannel_t enum
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Changes the active input channel and waits for conversion to complete
 *          Includes a delay to ensure conversion is complete at slowest sample rate
 */
HAL_StatusTypeDef ADS1115_SetChannel(ADS1115_Handle_t* hads1115, sChannel_t channel)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x7000; // Clear MUX bits
    hads1115->Reg[ADS1115_REG_CONFIG] |= (uint16_t)channel << 12; // Set new channel
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    if(status != HAL_OK) return status;
    
    // Add delay for the slowest possible conversion (8 SPS = 125ms)
    HAL_Delay(125); // Wait for one complete conversion at 8 SPS rate
    return status;
}

/**
 * @brief Set the data rate for ADC conversions
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param rate Sample rate selection from sSampleRate_t enum
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures the data rate at which the ADC performs conversions
 *          Rates range from 8 SPS to 860 SPS
 */
HAL_StatusTypeDef ADS1115_SetSampleRate(ADS1115_Handle_t* hads1115, sSampleRate_t rate)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x00E0; // Clear sample rate bits
    hads1115->Reg[ADS1115_REG_CONFIG] |= (uint16_t)rate << 5; // Set new sample rate
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Set the ADS1115 to single-shot conversion mode
 * @param hads1115 Pointer to ADS1115 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures the ADC to perform one conversion and then enter a low-power standby mode
 */
HAL_StatusTypeDef ADS1115_SetSSMode(ADS1115_Handle_t* hads1115)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] |= 0x0100; // Set to single-shot mode
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Start a single-shot conversion
 * @param hads1115 Pointer to ADS1115 handle structure
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Initiates a single ADC conversion by setting the OS bit in the config register
 */
HAL_StatusTypeDef ADS1115_StartSSConv(ADS1115_Handle_t* hads1115)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] |= ADS1115_OS_MASK; // Start single-shot conversion
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Set the high and low threshold registers for the comparator
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param lo_thresh Low threshold value
 * @param hi_thresh High threshold value
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Programs both the high and low threshold registers used by the 
 *          ADS1115's built-in comparator function
 */
HAL_StatusTypeDef ADS1115_SetThresholds(ADS1115_Handle_t* hads1115, uint16_t lo_thresh, uint16_t hi_thresh)
{
    HAL_StatusTypeDef status;
    hads1115->Reg[ADS1115_REG_LO_THRESH] = lo_thresh;
    hads1115->Reg[ADS1115_REG_HI_THRESH] = hi_thresh;
    hads1115->ptr_reg = ADS1115_REG_LO_THRESH;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_LO_THRESH], 2);
    if(status != HAL_OK) return status;
    hads1115->ptr_reg = ADS1115_REG_HI_THRESH;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_HI_THRESH], 2);
    return status;
}

/**
 * @brief Initialize the comparator functionality
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param mode Comparator mode (traditional or window)
 * @param pol Comparator polarity (active low or high)
 * @param lat Latching mode enable/disable
 * @param que Number of conversions before asserting alert
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures all comparator-related settings including mode, polarity,
 *          latching behavior, and number of conversions that must exceed thresholds
 */
HAL_StatusTypeDef ADS1115_Comp_Init(ADS1115_Handle_t* hads1115, uint16_t mode, uint16_t pol, uint16_t lat, uint16_t que)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x001F; // Clear comparator bits
    hads1115->Reg[ADS1115_REG_CONFIG] |= mode | pol | lat | que; // Set comparator configuration
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Set the comparator operating mode
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param mode Comparator mode (traditional or window)
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures the comparator to operate in either traditional mode
 *          (compare to fixed thresholds) or window mode (alert within range)
 */
HAL_StatusTypeDef ADS1115_Comp_SetMode(ADS1115_Handle_t* hads1115, uint16_t mode)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x0010; // Clear mode bit
    hads1115->Reg[ADS1115_REG_CONFIG] |= mode; // Set new mode
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Set the comparator alert pin polarity
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param pol Polarity setting (active low or high)
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures whether the ALERT/RDY pin is active-low or active-high
 *          when asserting an alert condition
 */
HAL_StatusTypeDef ADS1115_Comp_SetPol(ADS1115_Handle_t* hads1115, uint16_t pol)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x0008; // Clear polarity bit
    hads1115->Reg[ADS1115_REG_CONFIG] |= pol; // Set new polarity
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Set the comparator latching mode
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param lat Latching mode setting (enabled or disabled)
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures whether the ALERT/RDY pin latches after being asserted
 *          When latched, the pin remains active until conversion data is read
 */
HAL_StatusTypeDef ADS1115_Comp_SetLat(ADS1115_Handle_t* hads1115, uint16_t lat)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x0004; // Clear latching bit
    hads1115->Reg[ADS1115_REG_CONFIG] |= lat; // Set new latching mode
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

/**
 * @brief Set the number of conversions before asserting alert
 * @param hads1115 Pointer to ADS1115 handle structure
 * @param que Number of conversions that must exceed thresholds
 * @return HAL_StatusTypeDef HAL_OK on success, error code otherwise
 * @details Configures how many consecutive conversions must exceed the threshold
 *          values before the ALERT/RDY pin is asserted
 */
HAL_StatusTypeDef ADS1115_Comp_SetQue(ADS1115_Handle_t* hads1115, uint16_t que)
{
    HAL_StatusTypeDef status;
    ADS1115_ReadConfigReg(hads1115);
    if(status != HAL_OK) return status;
    hads1115->Reg[ADS1115_REG_CONFIG] &= ~0x0003; // Clear queue bits
    hads1115->Reg[ADS1115_REG_CONFIG] |= que; // Set new queue configuration
    hads1115->ptr_reg = ADS1115_REG_CONFIG;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, &hads1115->ptr_reg, 1);
    if(status != HAL_OK) return status;
    status = HAL_I2C_Master_Transmit_DMA(hads1115->i2c_handle, hads1115->I2C_address, (uint8_t*)&hads1115->Reg[ADS1115_REG_CONFIG], 2);
    return status;
}

