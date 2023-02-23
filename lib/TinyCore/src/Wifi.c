#include "Wifi.h"
#include "Core.h"

#include "esp_event.h"
#include "esp_system.h"
#include "esp_smartconfig.h"
#include "esp_wifi.h"

#include "string.h"

typedef struct
{
    bool m_Initialized;
} WifiInstance;

WifiInstance g_WifiInstance = {
    .m_Initialized = false
};

bool Wifi_Initialize()
{
    if(g_WifiInstance.m_Initialized)
    {
        return true;
    }

    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_init(&wifiConfig))
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_set_mode(WIFI_MODE_STA))
    HANDLE_OUTPUT_RETURN_FALSE(esp_wifi_start())

    g_WifiInstance.m_Initialized = true;

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