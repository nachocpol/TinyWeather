#include "driver/gpio.h"
#include "esp_log.h"

#include "Core.h"
#include "I2C.h"
#include "BME680.h"
#include "Wifi.h"
#include "Util.h"

#include "string.h"

static const char* k_LogTag = "Firmware";
static const char* k_FirmwareVersion = "0.1";

void Initialize();
void Update();

void app_main() 
{
    // Flush log and allow Monitor to catch-up
    ESP_LOGI(k_LogTag, "\n \n"); 
    DelayMS(5000);

    Initialize();
    while(true)
    {
        Update();
    }
}

void Initialize()
{
    ESP_LOGI(k_LogTag, "Initializing firmware. Current version is: %s", k_FirmwareVersion);

    InitializeSubSystems();

    I2C_Initialize(NULL);

    if(Wifi_Initialize())
    {
        ESP_LOGI(k_LogTag, "Scanning nearby WiFi access points...");
        uint16_t maxAps = 5;
        AccessPointInfo aps[maxAps];
        Wifi_ScanAPs(aps, &maxAps);
        ESP_LOGI(k_LogTag, "Found: %i APs", maxAps);
        for(uint16_t i = 0; i < maxAps; ++i)
        {
            ESP_LOGI(k_LogTag, "\t %s RSSI:%i  Authm:%i", aps[i].m_SSID, aps[i].m_RSSI, aps[i].m_AuthMode);
        }

        ESP_LOGI(k_LogTag, "Connecting to AP...");
        AccessPointInfo apInfo = {
            .m_AuthMode = WPA2_PSK
        };
        const char* ssid = "VM2930766";
        memcpy(apInfo.m_SSID, ssid, strlen(ssid));
        Wifi_Connect(&apInfo, "yy6nnSmcdgvj");
    }
    
    // Led pin
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);


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