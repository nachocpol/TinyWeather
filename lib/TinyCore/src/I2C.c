#include "I2C.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

I2C g_I2C = {
    .m_Initialized = false
};

I2CConfig g_I2CDefaultConfig = {
    .m_SDAPin = 7,
    .m_SCLPin = 6,
    .m_Speed = 400000
};

bool I2C_Initialize(I2CConfig* config)
{
    if(g_I2C.m_Initialized)
    {
        return true;
    }

    if(config == NULL)
    {
        config = &g_I2CDefaultConfig;
    }

    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .scl_pullup_en = true,
        .sda_pullup_en = true,
        .scl_io_num = config->m_SCLPin,
        .sda_io_num = config->m_SDAPin,
        .master.clk_speed = config->m_Speed,
        .clk_flags = 0
    };
    esp_err_t result = i2c_param_config(0, &cfg);
    ESP_ERROR_CHECK(result);
    result = i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0);
    ESP_ERROR_CHECK(result);

    g_I2C.m_Initialized = true;

    return true;
}

// Encode if we are reading/writing
uint8_t FormatRW(uint8_t value, bool write)
{
    return (value << 1) | (write ? I2C_MASTER_WRITE : I2C_MASTER_READ);
}

bool I2C_ReadData(uint8_t slaveAddr, uint8_t slaveRegister, uint8_t* data, uint8_t dataSize)
{
    if(data == NULL || dataSize == 0)
    {
        return false;
    }

    esp_err_t result = 0;
    
    i2c_cmd_handle_t cmdList = i2c_cmd_link_create();
    // Preamble
    {
        result = i2c_master_start(cmdList);
        result = i2c_master_write_byte(cmdList, FormatRW(slaveAddr, true), I2C_MASTER_ACK);
        result = i2c_master_write_byte(cmdList, slaveRegister, I2C_MASTER_ACK);
    }
    // Body
    {
        result = i2c_master_start(cmdList);
        result = i2c_master_write_byte(cmdList, FormatRW(slaveAddr, false), I2C_MASTER_ACK);
        for(uint8_t i = 0; i < dataSize; ++i)
        {
            const bool last = i == (dataSize - 1);
            result = i2c_master_read_byte(cmdList, &data[i], last ? I2C_MASTER_NACK : I2C_MASTER_ACK);
        }
        result = i2c_master_stop(cmdList);
    }
    result = i2c_master_cmd_begin(0, cmdList, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmdList);

    return true;
}

bool I2C_WriteData(uint8_t slaveAddr, uint8_t slaveRegister, uint8_t* data, uint8_t dataSize)
{
    if(data == NULL || dataSize == 0)
    {
        return false;
    }

    esp_err_t result = 0;
    
    i2c_cmd_handle_t cmdList = i2c_cmd_link_create();
    // Preamble
    {
        result = i2c_master_start(cmdList);
        result = i2c_master_write_byte(cmdList, FormatRW(slaveAddr, true), I2C_MASTER_ACK);
    }
    // Body
    {
        result = i2c_master_write_byte(cmdList, slaveRegister, I2C_MASTER_ACK); // First part to write
        for(uint8_t i = 0; i < dataSize; ++i)
        {
            result = i2c_master_write_byte(cmdList, data[i], I2C_MASTER_ACK); // Write all the rest 
        }
        result = i2c_master_stop(cmdList);
    }
    result = i2c_master_cmd_begin(0, cmdList, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmdList);

    return true;
}