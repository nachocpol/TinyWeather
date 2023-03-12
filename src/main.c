#include "driver/gpio.h"
#include "esp_log.h"

#include "esp_http_client.h"

#include "Core.h"
#include "I2C.h"
#include "BME680.h"
#include "Wifi.h"
#include "Util.h"

#include "Packets.h"
#include "JSONHelper.h"

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

        while(!Wifi_Connected())
        {
            ESP_LOGI(k_LogTag, "Waiting to stablish wifi connection...");
            DelayMS(250);
        }
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

void SendData(BME680Data* sourceData)
{
    if(sourceData == NULL)
    {
        return;
    }

    JSONObject* json = JSON_CreateObject(300);

    ESP_LOGI(k_LogTag, "%i", (int)json->m_DataSize);

    JSON_BeginObject(json);
    {
        JSON_AddProperty_U8(json, "magic", k_Magic);
        JSON_AddProperty_U8(json, "version", k_DataPacketVersion);
        JSON_AddProperty_Float(json, "temperature", sourceData->m_Temperature);
        JSON_AddProperty_Float(json, "pressure", sourceData->m_Pressure);
        JSON_AddProperty_Float(json, "humidity", sourceData->m_Humidity);
    }
    JSON_EndObject(json);
 
    ESP_LOGI(k_LogTag, "%s", json->m_RawData);

    json->m_RawData[json->m_DataSize] = 0;

    const char* port = "3000";
    const char* ip = "192.168.0.30";

    char url[128];
    sprintf(url, "http://%s:%s/data", ip, port);

    esp_http_client_config_t clientConfig = {
        .url = url
    };
    esp_http_client_handle_t  client = esp_http_client_init(&clientConfig);
    if(client != NULL)
    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        if(esp_http_client_open(client, json->m_DataPosition + 1) == ESP_OK)
        {
            esp_http_client_write(client, json->m_RawData, json->m_DataPosition + 1);
            esp_http_client_close(client);
        }
        else
        {
            ESP_LOGE(k_LogTag, "Failed to open http client");
        }
        esp_http_client_cleanup(client);
    }
    else
    {
        ESP_LOGE(k_LogTag, "Could not initialize the http client");
    }
}

void Update()
{
    gpio_set_level(GPIO_NUM_18, 1);
    DelayMS(1000);
    gpio_set_level(GPIO_NUM_18, 0);
    DelayMS(1000);

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
        SendData(&bmeData);
    }
}