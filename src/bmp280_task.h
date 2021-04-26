#include "i2c.h"
#include "bmp280.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct bmp280_measure {
    double temperature;
    double pressure;
};

extern QueueHandle_t bmp280_queue;

void bmp280_task(void *pvParameter);