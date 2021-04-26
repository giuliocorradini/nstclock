#include "clock_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"
#include "time.h"
#include "ssd1306.h"

QueueHandle_t clock_current_time = NULL;

void clock_task(void *pvParameter) {
    if(!clock_current_time)
        clock_current_time = xQueueCreate(1, sizeof(time_t));

    time_t now = *(int *)pvParameter;

    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();

    while(true) {
        xQueueOverwrite(clock_current_time, &now);
        now++;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}