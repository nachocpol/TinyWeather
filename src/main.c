#include "driver/gpio.h"
#include "esp_log.h"

#include "Core.h"
#include "I2C.h"
#include "BME680.h"

static const char* k_LogTag = "Firmware";
static const char* k_FirmwareVersion = "0.1";

void Initialize();
void Update();

void app_main() 
{
    Initialize();
    while(true)
    {
        Update();
    }
}

void Initialize()
{
    // Flush log
    ESP_LOGI(k_LogTag, "\n \n"); 

    DelayMS(5000);
    
    ESP_LOGI(k_LogTag, "Initializing firmware. Current version is: %s", k_FirmwareVersion);
    
    // Led pin
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);

    I2C_Initialize(NULL);

    // Setup the BME
    BME680InitSettings bmeConfig = {
        .m_Addr = 0x77,
        .m_I2CRead = &I2C_ReadData,
        .m_I2CWrite = &I2C_WriteData
    };

    if(!BME680_Initialize(&bmeConfig))
    {
        while(true)
        {
            ESP_LOGI(k_LogTag, "Failed to initialize the BME680");
            DelayMS(2000);
        }
    }
}

void Update()
{
    gpio_set_level(GPIO_NUM_18, 1);
    DelayMS(500);
    gpio_set_level(GPIO_NUM_18, 0);
    DelayMS(500);

    BME680Data bmeData;
    if(!BME680_Sample(&bmeData))
    {
        ESP_LOGI(k_LogTag, "Failed to sample the BME...");
    }
    else
    {
        ESP_LOGI(k_LogTag, 
                "Temperature: %f (C) Humidity: %f Pressure: %f (Pa)",
                bmeData.m_Temperature,bmeData.m_Humidity, bmeData.m_Pressure
        );
    }
}