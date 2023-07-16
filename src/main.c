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

extern const char certStart[] asm("_binary_influxCert_pem_start");
extern const char certEnd[]   asm("_binary_influxCert_pem_end");

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

#if 0

    // Setup the JSON raw data we will be sending the server
    JSONWriter* json = JSON_CreateWriter(300);
    JSON_BeginObject(json);
    {
        JSON_AddProperty_U8(json, "magic", k_Magic);
        JSON_AddProperty_U8(json, "version", k_DataPacketVersion);
        JSON_AddProperty_Float(json, "temperature", sourceData->m_Temperature);
        JSON_AddProperty_Float(json, "pressure", sourceData->m_Pressure);
        JSON_AddProperty_Float(json, "humidity", sourceData->m_Humidity);
    }
    JSON_EndObject(json);
    
    const uint32_t numBytesToWrite = json->m_DataPosition;

    // Build URL
    char url[128];
    const char* ip = "192.168.0.30";
    const char* port = "3000";
    sprintf(url, "http://%s:%s/data", ip, port);

    esp_http_client_config_t clientConfig = {
        .url = url
    };
    
    esp_http_client_handle_t  client = esp_http_client_init(&clientConfig);
    if(client != NULL)
    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        if(esp_http_client_open(client, numBytesToWrite) == ESP_OK)
        {
            esp_http_client_write(client, json->m_RawData, numBytesToWrite);
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

    JSON_ReleaseWriter(json);
#else

    const char* influxToken = "fIxFBU3Stsi6cz0VpcgMAH6xyRE97lpk5NLWtKqurBeaXFCgWY2WFpHjrBy1H5JHhGxTuCWCazcLKuKequQrFw==";
    const char* influxBucket = "WeatherDB";
    const char* influxOrg = "TinyWeather";
    const char* influxEndpoint = "https://eu-central-1-1.aws.cloud2.influxdata.com";

    char data[100] = {};
    sprintf(
        data, 
        "weatherData temperature=%.2f,humidity=%.2f,pressure=%.2f", 
        sourceData->m_Temperature, sourceData->m_Humidity, sourceData->m_Pressure
    );

    ESP_LOGI(k_LogTag, "Sending data:%s", data);

    char url[150] = {};
    sprintf(
        url, 
        "%s/api/v2/write?org=%s&bucket=%s&precision=s",
        influxEndpoint, influxOrg, influxBucket
    );


    esp_http_client_config_t  config = {
        .url = url,
        .cert_pem = certStart,
    };

    /*
    curl --request POST \
    "http://localhost:8086/api/v2/write?org=YOUR_ORG&bucket=YOUR_BUCKET&precision=ns" \
        --header "Authorization: Token YOUR_API_TOKEN" \
        --header "Content-Type: text/plain; charset=utf-8" \
        --header "Accept: application/json" \
        --data-binary '
            airSensors,sensor_id=TLM0201 temperature=73.97038159354763,humidity=35.23103248356096,co=0.48445310567793615 1630424257000000000
            airSensors,sensor_id=TLM0202 temperature=75.30007505999716,humidity=35.651929918691714,co=0.5141876544505826 1630424257000000000
        '
    */
   char tokenStr[180] = {};
   sprintf(tokenStr, "Token %s", influxToken);

    esp_http_client_handle_t client = esp_http_client_init(&config);

    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_url(client, url);
        esp_http_client_set_header(client, "Authorization", tokenStr);
        esp_http_client_set_header(client, "Content-Type", "text/plain; charset=utf-8");
        esp_http_client_set_header(client, "Accept", "tapplication/json");
        esp_err_t result = esp_http_client_open(client, strlen(data));
        if(result != ESP_OK)
        {
            ESP_LOGI(k_LogTag, "Failed to open http client");
        }
        else
        {
            // ESP_LOGI(k_LogTag, "Http client opened!");
            int wlen = esp_http_client_write(client, data, strlen(data));
            if(wlen < 0)
            {
                ESP_LOGI(k_LogTag, "Failed to write data to http client");
            }
            int contentLength = esp_http_client_fetch_headers(client);
            if(contentLength < 0)
            {
                ESP_LOGI(k_LogTag, "Failed to fetch headers");
            }
            else
            {
                char reponseData[300];
                int dataRead = esp_http_client_read_response(client, reponseData, 300);
                if(dataRead >= 0)
                {
                    int response = esp_http_client_get_status_code(client);
                    if(response != 201)
                    {
                        ESP_LOGI(k_LogTag, "Server response: %i", response);
                        ESP_LOGI(
                            k_LogTag, "HTTP POST Status = %d, content_length = %"PRIu64,
                            esp_http_client_get_status_code(client),
                            esp_http_client_get_content_length(client)
                        );
                        ESP_LOGI(k_LogTag, "%s", reponseData);
                    }
                }
                else
                {
                    ESP_LOGI(k_LogTag, "Failed to read response");
                }
            }
        }
    }
    esp_http_client_cleanup(client);
#endif
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