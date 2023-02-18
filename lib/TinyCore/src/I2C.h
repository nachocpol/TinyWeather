#ifndef I2C_H
#define I2C_H

#include "stdint.h"
#include "stdbool.h"

typedef struct
{
    bool m_Initialized;
} I2C;

typedef struct 
{
    uint8_t m_SDAPin;
    uint8_t m_SCLPin;
    uint32_t m_Speed;
} I2CConfig;

bool I2C_Initialize(I2CConfig* config);

bool I2C_ReadData(uint8_t slaveAddr, uint8_t slaveRegister, uint8_t* data, uint8_t dataSize);

bool I2C_WriteData(uint8_t slaveAddr, uint8_t slaveRegister, uint8_t* data, uint8_t dataSize);

#endif