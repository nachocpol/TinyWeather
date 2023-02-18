#include "BME680.h"
#include "bme68x_defs.h"
#include "bme68x.h"

#include "Core.h"

#include "string.h"

BME680Instance g_Instance = {
    .m_Initialized = false
};

BME68X_INTF_RET_TYPE BMERead(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void* intf_ptr)
{
    BME680Instance* instance = (BME680Instance*)intf_ptr;
    bool result = instance->m_Config.m_I2CRead(instance->m_Config.m_Addr, reg_addr, reg_data, length);
    return result ? 0 : 1;
}

BME68X_INTF_RET_TYPE BMEWrite(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr)
{
    BME680Instance* instance = (BME680Instance*)intf_ptr;
    bool result = instance->m_Config.m_I2CWrite(instance->m_Config.m_Addr, reg_addr, reg_data, length);
    return result ? 0 : 1;
}

void BMEDelay(uint32_t period, void *intf_ptr)
{
    DelayUS(period);
}

bool BME680_Initialize(BME680Config* config)
{
    if(g_Instance.m_Initialized)
    {
        return true;
    }
    
    // Init the instance
    g_Instance.m_Impl = malloc(sizeof(struct bme68x_dev));
    memcpy(&g_Instance.m_Config, config, sizeof(BME680Config));

    // Prepare the BME implementation settings
    struct bme68x_dev* bme = (struct bme68x_dev*)g_Instance.m_Impl;
    bme->read = &BMERead;
    bme->write = &BMEWrite;
    bme->intf = BME68X_I2C_INTF;
    bme->delay_us = &BMEDelay;
    bme->amb_temp = 21;
    bme->intf_ptr = &g_Instance;

    if(bme68x_init(bme) != BME68X_OK)
    {
        return false;
    }

    //if(bme68x_selftest_check(bme) != BME68X_OK)
    //{
    //    return false;
    //}

    return true;
}

bool BME680_Sample(BME680Data* output)
{
    if(!g_Instance.m_Initialized)
    {
        return false;
    }

    struct bme68x_dev* bme = (struct bme68x_dev*)g_Instance.m_Impl;

    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    struct bme68x_data data;
    uint8_t n_fields;

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    bme68x_set_conf(&conf, &bme);

    heatr_conf.enable = BME68X_DISABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
    
    bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);

    uint32_t measDuration = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, bme);

    DelayUS(measDuration);

    bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);

    if(n_fields)
    {
        output->m_Humidity = data.humidity;
        output->m_Temperature = data.temperature;
        output->m_Pressure = data.pressure;
    }

    return true;
}