#include "BME680.h"

#include "string.h"

//uint8_t resetCmd = 0xB6;
//I2C_WriteData(0x77, 0xE0, &resetCmd, 1);

BME680Instance g_Instance = {
    .m_Initialized = false
};

bool BME680_Initialize(BME680Config* config)
{
    if(g_Instance.m_Initialized)
    {
        return true;
    }

    if(config == NULL)
    {
        return false;
    }

    // Store config settings
    memcpy(&g_Instance.m_Config, config, sizeof(BME680Config));

    // Quick test the device    
    uint8_t chipID = 0;
    g_Instance.m_Config.m_I2CRead(g_Instance.m_Config.m_Addr, 0xD0, &chipID, 1);
    if(chipID != 0x61)
    {
        return false;
    }

    return true;
}