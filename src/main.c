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

SemaphoreHandle_t display_lock;
#define DISPLAY_LOCK(func)  xSemaphoreTake(display_lock, 100 / portTICK_RATE_MS);\
                            func;\
                            xSemaphoreGive(display_lock);

void show_menu() {
    config_menu_showing = true;
}

void short_press() {
    DISPLAY_LOCK(
    if(config_menu_showing) {
        struct menu_event down = {.cause = BUTTON_DOWN};
        config_menu_callback(&down);
    } else {
        show_menu();
    });
}

void long_press() {
    if(config_menu_showing) {
        struct menu_event enter = {.cause = BUTTON_ENTER};
        config_menu_callback(&enter);
    } else {
        show_menu();
    }
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
};

void app_main() {
    i2c_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(i2c_mutex);

    display_lock = xSemaphoreCreateMutex();
    xSemaphoreGive(display_lock);

    ssd1306_platform_i2cConfig_t cfg = {
        .sda = 21,
        .scl = 22
    };
    ssd1306_platform_i2cInit(I2C_NUM_1, 0, &cfg);

    ssd1306_setFreeFont(free_calibri11x12);
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
    I2C_MUTEX(ssd1306_printFixed(0, 0, "WeatherStation", STYLE_NORMAL));

    xTaskCreate(button_fsm, "menu_button", 2048, NULL, 7, NULL);
    config_menu_init();

    char t_string[6];
    char p_string[16];
    struct bmp280_measure measure;

    int showing = SHOWING_WEATHER;

    while(true) {

        switch(showing) {
            case SHOWING_WEATHER:
                if(xQueueReceive(bmp280_queue, &measure, 1100 / portTICK_RATE_MS) == pdTRUE) {
                    sprintf(t_string, "%.1f", measure.temperature);
                    sprintf(p_string, "%.1f", measure.pressure);

                    I2C_MUTEX(ssd1306_printFixed(0, 16, t_string, STYLE_NORMAL));
                    I2C_MUTEX(ssd1306_printFixed(64, 16, p_string, STYLE_NORMAL));
                }
                if(config_menu_showing) {
                    showing = SHOWING_CONFIG_MENU;
                }
                break;
            case SHOWING_CONFIG_MENU:
                I2C_MUTEX(ssd1306_clearScreen());
                I2C_MUTEX(ssd1306_showMenu(&config_menu));
                break;
            default:
                break;
        }

        vTaskDelay(20 / portTICK_RATE_MS);
    }

}