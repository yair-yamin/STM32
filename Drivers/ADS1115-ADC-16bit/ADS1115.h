#ifndef ADS1115_H
#define ADS1115_H
#include "main.h"
/*------------------- Regester Address Defines ---------------------------*/
#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG     0x01
#define ADS1115_REG_LO_THRESH  0x02
#define ADS1115_REG_HI_THRESH  0x03

/************************ Config Reg Bit Mask defines ********************************/
#define ADS1115_OS_MASK        0x8000 // begin single-shot conversion start only in power-down mode

#define ADS1115_MUX_A0_A3_MASK 0x1000 //AINp = AIN0 and AINn = AIN3 
#define ADS1115_MUX_A1_A3_MASK 0x2000 //AINp = AIN1 and AINn = AIN3
#define ADS1115_MUX_A2_A3_MASK 0x3000 //AINp = AIN2 and AINn = AIN3
#define ADS1115_MUX_A0_GND_MASK 0x4000 //AINp = AIN0 and AINn = GND
#define ADS1115_MUX_A1_GND_MASK 0x5000 //AINp = AIN1 and AINn = GND
#define ADS1115_MUX_A2_GND_MASK 0x6000 //AINp = AIN2 and AINn = GND
#define ADS1115_MUX_A3_GND_MASK 0x7000 //AINp = AIN3 and AINn = GND

#define ADS1115_PGA_6_144V_MASK 0x0000 // +/-6.144V range = Gain 2/3
#define ADS1115_PGA_4_096V_MASK 0x0200 // +/-4.096V range = Gain 1
#define ADS1115_PGA_2_048V_MASK 0x0400 // +/-2.048V range = Gain 2 (default)
#define ADS1115_PGA_1_024V_MASK 0x0600 // +/-1.024V range = Gain 4
#define ADS1115_PGA_0_512V_MASK 0x0800 // +/-0.512V range = Gain 8
#define ADS1115_PGA_0_256V_MASK 0x0A00 // +/-0.256V range = Gain 16

#define ADS1115_MODE_CONTINUOUS_MASK 0x0000 // Continuous conversion mode
#define ADS1115_MODE_SINGLESHOT_MASK 0x0100 // Power-down single-shot mode

#define ADS1115_DR_8SPS_MASK  0x0000 // 8 samples per second
#define ADS1115_DR_16SPS_MASK 0x0020 // 16 samples per second
#define ADS1115_DR_32SPS_MASK 0x0040 // 32 samples per second
#define ADS1115_DR_64SPS_MASK 0x0060 // 64 samples per second
#define ADS1115_DR_128SPS_MASK 0x0080 // 128 samples per second (default)
#define ADS1115_DR_250SPS_MASK 0x00A0 // 250 samples per second
#define ADS1115_DR_475SPS_MASK 0x00C0 // 475 samples per second
#define ADS1115_DR_860SPS_MASK 0x00E0 // 860 samples per second

#define ADS1115_COMP_MODE_TRAD_MASK 0x0000 // Traditional comparator with hyster
#define ADS1115_COMP_MODE_WINDOW_MASK 0x0010 // Window comparator

#define ADS1115_COMP_POL_ACTIVE_LOW_MASK 0x0000 // ALERT/RDY pin is low when active (default)
#define ADS1115_COMP_POL_ACTIVE_HIGH_MASK 0x0008 // ALERT/RDY pin is high when active

#define ADS1115_COMP_LAT_NON_LATCHING_MASK 0x0000 // Non-latching comparator (default)
#define ADS1115_COMP_LAT_LATCHING_MASK 0x0004 // Latching comparator

#define ADS1115_COMP_QUE_1_MASK 0x0000 // Assert ALERT/RDY after one conversion
#define ADS1115_COMP_QUE_2_MASK 0x0001 // Assert ALERT/RDY after two conversions
#define ADS1115_COMP_QUE_4_MASK 0x0002 // Assert ALERT/RDY after four conversions
#define ADS1115_COMP_QUE_DISABLE_MASK 0x0003 // Disable the comparator and put ALERT/RDY in high state (default)

/************************ Driver Structs ********************************/
typedef enum {
    DIF_A0_A3 = 1,
    DIF_A1_A3 = 2,
    DIF_A2_A3 = 3,
    AIN0 = 4,
    AIN1= 5,
    AIN2= 6,
    Ain3= 7
}sChannel_t;

typedef enum {
    SPS_8= 0,
    SPS_16 = 1,
    SPS_32 = 2,
    SPS_64 = 3,
    SPS_128 = 4,
    SPS_250 = 5,
    SPS_475 = 6,
    SPS_860 = 7
    
}sSampleRate_t;



typedef struct {
    I2C_HandleTypeDef* i2c_handle;
    uint8_t I2C_address;
    sChannel_t channel;
    uint8_t ptr_reg;
    uint16_t Reg[4]; // Register buffer
} ADS1115_Handle_t;

/*------------------- Function Declarations ---------------------------*/
HAL_StatusTypeDef ADS1115_Init(ADS1115_Handle_t* hads1115,uint16_t mode,sChannel_t channel, uint16_t pga, uint16_t sampleRate);
HAL_StatusTypeDef ADS1115_ReadConfigReg(ADS1115_Handle_t* hads1115);
HAL_StatusTypeDef ADS1115_ReadConversionReg(ADS1115_Handle_t* hads1115);
HAL_StatusTypeDef ADS1115_SetChannel(ADS1115_Handle_t* hads1115, sChannel_t channel);
HAL_StatusTypeDef ADS1115_SetSampleRate(ADS1115_Handle_t* hads1115, sSampleRate_t rate);
HAL_StatusTypeDef ADS1115_StartSSConv(ADS1115_Handle_t* hads1115);
HAL_StatusTypeDef ADS1115_SetSSMode(ADS1115_Handle_t* hads1115);
HAL_StatusTypeDef ADS1115_SetThresholds(ADS1115_Handle_t* hads1115, uint16_t lo_thresh, uint16_t hi_thresh);
HAL_StatusTypeDef ADS1115_Comp_Init(ADS1115_Handle_t* hads1115, uint16_t mode, uint16_t pol, uint16_t lat, uint16_t que);
HAL_StatusTypeDef ADS1115_Comp_SetMode(ADS1115_Handle_t* hads1115, uint16_t mode);
HAL_StatusTypeDef ADS1115_Comp_SetPol(ADS1115_Handle_t* hads1115, uint16_t pol);
HAL_StatusTypeDef ADS1115_Comp_SetLat(ADS1115_Handle_t* hads1115, uint16_t lat);
HAL_StatusTypeDef ADS1115_Comp_SetQue(ADS1115_Handle_t* hads1115, uint16_t que);


#endif // ADS1115_H