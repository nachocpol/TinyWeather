#ifndef BME680_H
#define BME680_H

#include "stdbool.h"
#include "stdint.h"

typedef bool (*BME_I2C_Read)(uint8_t addr, uint8_t registerAddr, uint8_t* data, uint8_t dataLen);
typedef bool (*BME_I2C_Write)(uint8_t addr, uint8_t registerAddr, uint8_t* data, uint8_t dataLen);

typedef struct 
{
    uint8_t m_Addr;
    BME_I2C_Read m_I2CRead;
    BME_I2C_Write m_I2CWrite;
} BME680InitSettings;

typedef struct 
{
    float m_Temperature;
    float m_Humidity;
    float m_Pressure;   
} BME680Data;

enum BME680OSMode
{
    OS_0X = 0,
    OS_1X,
    OS_2X,
    OS_4X,
    OS_8X,
    OS_16X
};

typedef struct
{
    enum BME680OSMode m_TemperatureOS;
    enum BME680OSMode m_PressureOS;
    enum BME680OSMode m_HumidityOS;
    bool m_HeaterEnabled;
    uint32_t m_HeaterDuration;
    uint32_t m_HeaterTemperature;
} BME680SamplingSettings;


// Initializes the BME driver. Checks the chipID and retrieves the calibration
// parameters. Sampling settings are left as default.
bool BME680_Initialize(BME680InitSettings* config);

// Performs a self test routine. Returns true if the test passed. False otherwise.
bool BME680_SelfTest();

bool BME680_SetSamplingSettings(const BME680SamplingSettings* settings);

bool BME680_Sample(BME680Data* output);

#endif