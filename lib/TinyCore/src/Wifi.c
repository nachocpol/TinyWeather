#include "Wifi.h"
#include "Core.h"
#include "Util.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_smartconfig.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include "string.h"

#define WIFI_CONNECTED_BIT BIT0

typedef struct
{
    bool m_Initialized;
    uint32_t m_ConnectionRetryCout;
    EventGroupHandle_t m_Flags;
} WifiInstance;

WifiInstance g_WifiInstance = {
    .m_Initialized = false,
    .m_ConnectionRetryCout = 0,
};

void Wifi_EventHandler(void* userData, esp_event_base_t eventBase, int32_t id, void* data)
{
    if(eventBase == WIFI_EVENT)
    {
        if(id == WIFI_EVENT_STA_DISCONNECTED)
        {
            if(g_WifiInstance.m_ConnectionRetryCout > 0)
            {
                ESP_LOGI("WIFI", "Atempting to reconnect to wifi...");
                g_WifiInstance.m_ConnectionRetryCout--;
                esp_wifi_connect();
            }

            xEventGroupClearBits(g_WifiInstance.m_Flags, WIFI_CONNECTED_BIT);
        }
        else
        {
            ESP_LOGI("WIFI", "WiFi Event:%li", id);
        }
    }
    else if(eventBase == IP_EVENT)
    {
        if(id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*)data;
            ESP_LOGI("WIFI", "Device IP:" IPSTR, IP2STR(&event->ip_info.ip));

            xEventGroupSetBits(g_WifiInstance.m_Flags, WIFI_CONNECTED_BIT);
        }
        else
        {
            ESP_LOGI("WIFI-IP", "IP Event:%li", id);
        }
    }
}

bool Wifi_Initialize()
{
    if(g_WifiInstance.m_Initialized)
    {
        return true;
    }

    // Initialize core wifi system
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta(); // Needed for IP. Prob used internally by Wifi handlers
    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_init(&wifiConfig))
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_set_mode(WIFI_MODE_STA))
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_start())

    // Register wifi related events
    HANDLE_OUTPUT_RETURN_FALSE(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &Wifi_EventHandler, NULL, NULL)
    );
    HANDLE_OUTPUT_RETURN_FALSE(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &Wifi_EventHandler, NULL, NULL)
    );

    g_WifiInstance.m_Flags = xEventGroupCreate();

    g_WifiInstance.m_Initialized = true;

    return true;
}

bool Wifi_Connect(AccessPointInfo* info, const char* password)
{
    if(info == NULL || password == NULL)
    {
        return false;
    }

    // TODO: Check if we are already connected or pendering connection

    wifi_config_t staConfig = {
        .sta = {
            .threshold.authmode = (wifi_auth_mode_t)info->m_AuthMode,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH
        }
    };
    StrToArray(info->m_SSID, staConfig.sta.ssid, 33);
    StrToArray(password, staConfig.sta.password, 64);

    g_WifiInstance.m_ConnectionRetryCout = 10; // Allow for up to 10 tries

    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_set_config(WIFI_IF_STA, &staConfig));
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_connect());


    return true;
}

void Wifi_SmartConfig()
{
    
}

void Wifi_ScanAPs(AccessPointInfo* accessPoints, uint16_t* maxAccessPoints)
{
    if(!g_WifiInstance.m_Initialized)
    {
        return;
    }

    // Enable search of hidden SSIDs
    wifi_scan_config_t scanConfig = {0};
    scanConfig.show_hidden = 1;
    scanConfig.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    if(!HANDLE_OUTPUT(esp_wifi_scan_start(&scanConfig, true)))
    {
        return;
    }

    wifi_ap_record_t scannedAccessPoints[12];
    uint16_t totalAccessPoints = 12;
    if(!HANDLE_OUTPUT(esp_wifi_scan_get_ap_records(&totalAccessPoints, scannedAccessPoints)))
    {
        return;
    }

    if(totalAccessPoints < *maxAccessPoints)
    {
        *maxAccessPoints = totalAccessPoints;
    }

    // Store found APs in the array (figure out array size based on the user input)
    for(int i = 0; i < *maxAccessPoints; ++i)
    {        
        // Copy to the output array
        memcpy(accessPoints[i].m_SSID, scannedAccessPoints[i].ssid, 33);
        accessPoints[i].m_RSSI = scannedAccessPoints[i].rssi;
        accessPoints[i].m_AuthMode = (enum WifiAuthMode)scannedAccessPoints[i].authmode;        
    }
}

bool Wifi_Connected()
{
    return xEventGroupGetBits(g_WifiInstance.m_Flags) & WIFI_CONNECTED_BIT;
}