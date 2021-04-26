#include "clock_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c.h"
#include "time.h"
#include "ssd1306.h"

void clock_task(void *pvParameter) {
    time_t now = *(int *)pvParameter;
    char timestamp[16];

    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();

    while(true) {
        strftime(timestamp, 16, "%H:%M:%S", localtime(&now));
        I2C_MUTEX(ssd1306_printFixed(79, 0, timestamp, STYLE_NORMAL));
        now++;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}