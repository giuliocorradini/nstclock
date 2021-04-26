/*
 *  Static image example
 */

#include <stdio.h>
#include <time.h>

#include "driver/gpio.h"

#include "driver/i2c.h"
#include "ssd1306_hal/io.h"
#include "ssd1306.h"

#include "i2c.h"
#include "bmp280.h"
#include "bmp280_task.h"

#include "clock_task.h"

#include "config_menu.h"
#include "driver/gpio.h"

#include "AnimalCrossingWildWorld.h"
#include "big_AnimalCrossingWildWorld.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

SemaphoreHandle_t display_lock;
#define DISPLAY_LOCK(func)  xSemaphoreTake(display_lock, 100 / portTICK_RATE_MS);\
                            func;\
                            xSemaphoreGive(display_lock);

EventGroupHandle_t button_event;
enum fsm_interthread_event_t {
    EVENT_SHORT_PRESS   = (1<<0),
    EVENT_LONG_PRESS    = (1<<1)
};

void show_menu() {
    config_menu_showing = true;
}

void short_press() {
    xEventGroupSetBits(button_event, EVENT_SHORT_PRESS);
}

void long_press() {
    xEventGroupSetBits(button_event, EVENT_LONG_PRESS);
}

void button_fsm(void *pvParameters){
    //Configure GPIO interrupt
    gpio_pad_select_gpio(0);
    gpio_set_direction(0, GPIO_MODE_INPUT);

    static int fsm_status = 0;

    while(true) {
        bool button_pressed = (gpio_get_level(0) == 0);

        switch(fsm_status) {
            case 0: //not pressed
                if(button_pressed) {
                    fsm_status = 1;
                }
                break;
            case 1: //pressed
                if(!button_pressed) {
                    fsm_status = 0;
                    short_press();
                } else {
                    fsm_status = 2;
                }
                break;
            case 2: //pressed for more than 500ms
                if(!button_pressed) {
                    fsm_status = 0;
                    short_press();
                } else {
                    fsm_status = 3;
                }
                break;
            case 3:
                if(!button_pressed) {
                    long_press();
                    fsm_status = 0;
                }
                break;
        }

        vTaskDelay((fsm_status == 1 ? 500 : 10) / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

enum showing {
    SHOWING_WEATHER,
    SHOWING_CONFIG_MENU,
    SHOWING_CONFIG_MENU_UPDATE_LOOP,
    SHOWING_TEMPERATURE,
    SHOWING_PRESSURE
};

void app_main() {
    i2c_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(i2c_mutex);

    display_lock = xSemaphoreCreateMutex();
    xSemaphoreGive(display_lock);

    button_event = xEventGroupCreate();

    ssd1306_platform_i2cConfig_t cfg = {
        .sda = 21,
        .scl = 22
    };
    ssd1306_platform_i2cInit(I2C_NUM_1, 0, &cfg);

    ssd1306_setFreeFont(free_AnimalCrossingWildWorld9x12);
    I2C_MUTEX(ssd1306_128x64_i2c_init());

    I2C_MUTEX(ssd1306_clearScreen());

    I2C_MUTEX(ssd1306_printFixed(0, 0, "MeteoStation", STYLE_NORMAL));
    I2C_MUTEX(ssd1306_printFixed(0, 16, "Waiting for timestamp on serial", STYLE_NORMAL));

    i2c_set_controller(I2C_NUM_1);
    bmp280_queue = xQueueCreate(10, sizeof(struct bmp280_measure));
    xTaskCreate(bmp280_task, "bmp280", 4096, NULL, 6, NULL);

    int current_time;
    char timestamp[50];
    char c = 0x00;
    int i = 0;
    while(i<50 && c != '\n') {
        if((c = getchar()) != 0xff) {
            timestamp[i++] = c;
        }
    }
    sscanf(timestamp, "%d", &current_time);
    xTaskCreate(clock_task, "clock", 2048, &current_time, 3, NULL);
    I2C_MUTEX(ssd1306_clearScreen());
    I2C_MUTEX(ssd1306_printFixed(0, 0, "MeteoStation", STYLE_NORMAL));

    xTaskCreate(button_fsm, "menu_button", 2048, NULL, 7, NULL);
    config_menu_init();

    char t_string[6];
    char p_string[16];
    struct bmp280_measure measure;

    int showing = SHOWING_WEATHER;
    int button_event_bits;

    while(true) {

        time_t now;
        char timestamp[16];

        switch(showing) {
            case SHOWING_WEATHER:
                ssd1306_setFreeFont(free_AnimalCrossingWildWorld9x12);
                
                if(xQueueReceive(clock_current_time, &now, 0)) {    
                    strftime(timestamp, 16, "%H:%M:%S", localtime(&now));
                    I2C_MUTEX(ssd1306_printFixed(79, 0, timestamp, STYLE_NORMAL));
                }

                if(xQueueReceive(bmp280_queue, &measure, 50 / portTICK_RATE_MS) == pdTRUE) {
                    sprintf(t_string, "%.1f", measure.temperature);
                    sprintf(p_string, "%.1f", measure.pressure);

                    I2C_MUTEX(ssd1306_printFixed(0, 16, t_string, STYLE_NORMAL));
                    I2C_MUTEX(ssd1306_printFixed(64, 16, p_string, STYLE_NORMAL));
                }

                if((button_event_bits = xEventGroupGetBits(button_event))) {
                    xEventGroupClearBits(button_event, button_event_bits);
                    if(button_event_bits & EVENT_SHORT_PRESS) {
                        showing = SHOWING_TEMPERATURE;
                        ssd1306_clearBlock(0, 2, 128, 48);
                    }
                    if (button_event_bits & EVENT_LONG_PRESS) {
                        showing = SHOWING_CONFIG_MENU;
                    }
                }
                break;

            case SHOWING_CONFIG_MENU:
                I2C_MUTEX(ssd1306_clearScreen());
                I2C_MUTEX(ssd1306_showMenu(&config_menu));
                showing = SHOWING_CONFIG_MENU_UPDATE_LOOP;
                break;

            case SHOWING_CONFIG_MENU_UPDATE_LOOP:
                if((button_event_bits = xEventGroupGetBits(button_event))) {
                    xEventGroupClearBits(button_event, button_event_bits);
                    if(button_event_bits & EVENT_SHORT_PRESS) {
                        struct menu_event down = {.cause = BUTTON_DOWN};
                        config_menu_callback(&down);
                    }
                    if (button_event_bits & EVENT_LONG_PRESS) {
                        struct menu_event enter = {.cause = BUTTON_ENTER};
                        config_menu_callback(&enter);
                    }
                }
                break;

            case SHOWING_TEMPERATURE:
                ssd1306_setFreeFont(free_big_AnimalCrossingWildWorld16x21);
                if(xQueueReceive(bmp280_queue, &measure, 50 / portTICK_RATE_MS) == pdTRUE) {
                    sprintf(t_string, "%.1f", measure.temperature);

                    I2C_MUTEX(ssd1306_printFixed(0, 16, t_string, STYLE_NORMAL));
                    I2C_MUTEX(ssd1306_printFixed(80, 16, "°C", STYLE_NORMAL));
                }
                if((button_event_bits = xEventGroupGetBits(button_event))) {
                    xEventGroupClearBits(button_event, button_event_bits);
                    if(button_event_bits & EVENT_SHORT_PRESS) {
                        showing = SHOWING_PRESSURE;
                        ssd1306_clearBlock(0, 2, 128, 48);
                    }
                    if (button_event_bits & EVENT_LONG_PRESS) {
                        showing = SHOWING_CONFIG_MENU;
                    }
                }
                break;

            case SHOWING_PRESSURE:
                ssd1306_setFreeFont(free_big_AnimalCrossingWildWorld16x21);
                if(xQueueReceive(bmp280_queue, &measure, 50 / portTICK_RATE_MS) == pdTRUE) {
                    sprintf(p_string, "%.1f", measure.pressure);

                    I2C_MUTEX(ssd1306_printFixed(0, 16, p_string, STYLE_NORMAL));
                    I2C_MUTEX(ssd1306_printFixed(80, 16, "Pa", STYLE_NORMAL));
                }
                if((button_event_bits = xEventGroupGetBits(button_event))) {
                    xEventGroupClearBits(button_event, button_event_bits);
                    if(button_event_bits & EVENT_SHORT_PRESS) {
                        showing = SHOWING_WEATHER;
                        ssd1306_clearBlock(0, 2, 128, 48);
                    }
                    if (button_event_bits & EVENT_LONG_PRESS) {
                        showing = SHOWING_CONFIG_MENU;
                    }
                }
                break;

            default:
                break;
        }

        vTaskDelay(10 / portTICK_RATE_MS);
    }

}