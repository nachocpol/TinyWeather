#include "Core.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void DelayMS(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void DelayUS(uint32_t us)
{
    DelayMS(us / 1000); // Uhm.
}
