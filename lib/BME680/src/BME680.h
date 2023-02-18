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
} BME680Config;

typedef struct
{
    BME680Config m_Config;
    bool m_Initialized;
    void* m_Impl;
} BME680Instance;

typedef struct 
{
    float m_Temperature;
    float m_Humidity;
    float m_Pressure;   
} BME680Data;

bool BME680_Initialize(BME680Config* config);

bool BME680_Sample(BME680Data* output);

#endif