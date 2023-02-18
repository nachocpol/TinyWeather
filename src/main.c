#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

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
    vTaskDelay(3500 / portTICK_PERIOD_MS);
    ESP_LOGI(k_LogTag, "Initializing firmware. Current version is: %s", k_FirmwareVersion);
    
    // Led pin
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);

    I2C_Initialize(NULL);

    // Setup the BME
    BME680Config bmeConfig = {
        .m_Addr = 0x77,
        .m_I2CRead = &I2C_ReadData,
        .m_I2CWrite = &I2C_WriteData
    };
    if(!BME680_Initialize(&bmeConfig))
    {
        ESP_LOGI(k_LogTag, "Failed to initialize the BME680");
    }
}

void Update()
{
    gpio_set_level(GPIO_NUM_18, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_18, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    uint8_t chipId = 0;
    if(I2C_ReadData(0x77, 0xD0, &chipId, 1))
    {
        ESP_LOGI(k_LogTag, "Got bm id");
    }
    else
    {
        ESP_LOGI(k_LogTag, "Failed to get bm id");
    }
}