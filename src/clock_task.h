#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void clock_task(void *pvParameter);
extern QueueHandle_t clock_current_time;