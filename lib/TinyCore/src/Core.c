#include "Core.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

void DelayMS(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void DelayUS(uint32_t us)
{
    DelayMS(us / 1000); // Uhm.
}

bool InitializeSubSystems()
{
    if(!HANDLE_OUTPUT(nvs_flash_init()))
    {
        return false;
    }

    if(!HANDLE_OUTPUT(esp_netif_init()))
    {
        return false;
    }

    return true;
}

bool HandleOutput(int errorCode, const char* file, const char* function, int line)
{
    // This is a chunky function. Tons of c*. Look into memory usage if its a problem later on.
    if(errorCode != ESP_OK)
    {
        ESP_LOGE(
            "CORE", 
            "%s[%i]  (%s::%s::%i)", 
            esp_err_to_name(errorCode), errorCode, file, function, line
        );
        return false;
    }
    return true;
}