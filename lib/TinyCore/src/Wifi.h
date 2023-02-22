#ifndef WIFI_H
#define WIFI_H

#include "stdint.h"
#include "stdbool.h"

typedef struct
{
    char m_SSID[33];
    int8_t m_RSSI;
    //wifi_auth_mode_t m_AuthMode;
} AccessPointInfo;


bool Wifi_Initialize();

#endif