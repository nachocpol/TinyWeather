#ifndef WIFI_H
#define WIFI_H

#include "stdint.h"
#include "stdbool.h"

enum WifiAuthMode
{
    OPEN = 0,
    WEP,
    WPA_PSK,
    WPA2_PSK,
    WPA_WPA2_PSK,
    WPA2_ENTERPRISE,
    WPA3_PSK,
    WPA2_WPA3_PSK,
    WAPI_PSK,
    OWE,
};

typedef struct
{
    char m_SSID[33];
    int8_t m_RSSI;
    enum WifiAuthMode m_AuthMode;
} AccessPointInfo;

bool Wifi_Initialize();

void Wifi_SmartConfig();

void Wifi_ScanAPs(AccessPointInfo* accessPoints, uint16_t* maxAccessPoints);

#endif