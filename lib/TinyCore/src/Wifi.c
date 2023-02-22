#include "Wifi.h"

#include "esp_event.h"
#include "esp_system.h"

typedef struct
{
    bool m_Initialized;
} WifiInstance;

WifiInstance g_WifiInstance = {
    .m_Initialized = false
};

bool Wifi_Initialize()
{
    return true;
}
