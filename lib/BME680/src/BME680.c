#include "BME680.h"
#include "Core.h"

#include "bme68x_defs.h"
#include "bme68x.h"

#include "string.h"

typedef struct
{
    BME680InitSettings m_Config;
    bool m_Initialized;
    struct bme68x_dev* m_Impl;
    uint32_t m_SampleDelay;
} BME680Instance;

static BME680Instance g_Instance = {
    .m_Initialized = false
};

static BME680SamplingSettings g_DefaultSampling = {
    .m_HeaterEnabled = true,
    .m_HeaterDuration = 100,
    .m_HeaterTemperature = 300,
    .m_PressureOS = OS_4X,
    .m_TemperatureOS = OS_4X,
    .m_HumidityOS = OS_4X,
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

bool BME680_Initialize(BME680InitSettings* config)
{
    if(g_Instance.m_Initialized)
    {
        return true;
    }
    
    // Init the instance
    g_Instance.m_Impl = malloc(sizeof(struct bme68x_dev));
    memcpy(&g_Instance.m_Config, config, sizeof(BME680InitSettings));

    // Prepare the BME implementation settings
    g_Instance.m_Impl->read = &BMERead;
    g_Instance.m_Impl->write = &BMEWrite;
    g_Instance.m_Impl->intf = BME68X_I2C_INTF;
    g_Instance.m_Impl->delay_us = &BMEDelay;
    g_Instance.m_Impl->amb_temp = 21;
    g_Instance.m_Impl->intf_ptr = &g_Instance;

    if(bme68x_init(g_Instance.m_Impl) != BME68X_OK)
    {
        return false;
    }

    g_Instance.m_Initialized = true;
    
    // Finally, configure it with some sensible defaults
    return BME680_SetSamplingSettings(&g_DefaultSampling);
}

bool BME680_SelfTest()
{
    if(!g_Instance.m_Initialized)
    {
        return false;
    }
    return bme68x_selftest_check(g_Instance.m_Impl) != BME68X_OK;
}

bool BME680_SetSamplingSettings(const BME680SamplingSettings* settings)
{
    if(!g_Instance.m_Initialized)
    {
        return false;
    }

    // This will configure TPH sampling settings
    struct bme68x_conf conf = {
        .filter = BME68X_FILTER_OFF,
        .odr = BME68X_ODR_NONE,
        .os_hum = settings->m_HumidityOS,
        .os_pres = settings->m_PressureOS,
        .os_temp = settings->m_TemperatureOS,
    };
    bme68x_set_conf(&conf, g_Instance.m_Impl);
    
    // Configure gas heater
    struct bme68x_heatr_conf heatr_conf = {
        .enable = settings->m_HeaterEnabled,
        .heatr_temp = settings->m_HeaterTemperature,
        .heatr_dur = settings->m_HeaterDuration,
    };
    bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, g_Instance.m_Impl);
    
    // Cache the delay we will need once we sample the data (accounting for THP and heater)
    g_Instance.m_SampleDelay = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, g_Instance.m_Impl) + (heatr_conf.heatr_dur * 1000);

    // Keep in mind that the sensor is in Sleep mode all the time. We will wake it up (FORCED_MODE) right before
    // sampling the data
    return true;
}

bool BME680_Sample(BME680Data* output)
{
    // Reset all values to an invalid state
    output->m_Humidity = 0.0f;
    output->m_Pressure = 0.0f;
    output->m_Temperature = 0.0f;
 
    if(!g_Instance.m_Initialized)
    {
        return false;
    }
    
    // This will wake up the device and start a sampling routine
    bme68x_set_op_mode(BME68X_FORCED_MODE, g_Instance.m_Impl);

    // Lets wait to make sure the results are ready
    DelayUS(g_Instance.m_SampleDelay);

    struct bme68x_data sampleData;
    uint8_t n_fields;
    bme68x_get_data(BME68X_FORCED_MODE, &sampleData, &n_fields, g_Instance.m_Impl);

    if(n_fields)
    {
        output->m_Humidity = sampleData.humidity;
        output->m_Temperature = sampleData.temperature;
        output->m_Pressure = sampleData.pressure;
    }

    return n_fields != 0;
}