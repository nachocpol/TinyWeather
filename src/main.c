#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

void app_main() 
{
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    while(true)
    {
        gpio_set_level(GPIO_NUM_18, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_NUM_18, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);

        ESP_LOGI("TEST", "Hello world");
    }
}