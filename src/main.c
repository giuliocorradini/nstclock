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

struct bmp280_measure {
    double temperature;
    double pressure;
};
QueueHandle_t bmp280_queue;

void bmp_delay_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_RATE_MS);
}

void bmp280_task(void *pvParameter) {

    int8_t rslt;
    struct bmp280_dev bmp = {
        .delay_ms = bmp_delay_ms,
        .dev_id = BMP280_I2C_ADDR_PRIM,
        .intf = BMP280_I2C_INTF,
        .read = i2c_bmp_reg_read,
        .write = i2c_bmp_reg_write
    };
    struct bmp280_config conf;
    struct bmp280_uncomp_data ucomp_data;

    rslt = bmp280_init(&bmp);

    rslt = bmp280_get_config(&conf, &bmp);

    conf = (struct bmp280_config){
        .filter = BMP280_FILTER_COEFF_2,
        .os_temp = BMP280_OS_4X,
        .os_pres = BMP280_OS_2X,
        .odr = BMP280_ODR_1000_MS
    };

    rslt = bmp280_set_config(&conf, &bmp);

    rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);

    double temp, pres;
    while(true) {
        rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

        rslt = bmp280_get_comp_temp_double(&temp, ucomp_data.uncomp_temp, &bmp);
        rslt = bmp280_get_comp_pres_double(&pres, ucomp_data.uncomp_press, &bmp);

        struct bmp280_measure measure = {
            .temperature = temp,
            .pressure = pres
        };
        
        xQueueSend(bmp280_queue,
            &measure,
            100 / portTICK_RATE_MS);

        printf("Temperature: %.2f, pressure: %.1f\n",
               temp,
               pres);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}


void clock_task(void *pvParameter) {
    time_t now = 1618857352;
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

void app_main() {
    i2c_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(i2c_mutex);

    ssd1306_platform_i2cConfig_t cfg = {
        .sda = 21,
        .scl = 22
    };
    ssd1306_platform_i2cInit(I2C_NUM_1, 0, &cfg);

    ssd1306_setFixedFont(ssd1306xled_font6x8);
    I2C_MUTEX(ssd1306_128x64_i2c_init());

    I2C_MUTEX(ssd1306_clearScreen());

    I2C_MUTEX(ssd1306_print("OpenWisar"));

    i2c_set_controller(I2C_NUM_1);
    bmp280_queue = xQueueCreate(10, sizeof(struct bmp280_measure));
    xTaskCreate(bmp280_task, "bmp280", 4096, NULL, 6, NULL);

    xTaskCreate(clock_task, "clock", 2048, NULL, 3, NULL);

    char t_string[6];
    char p_string[16];
    struct bmp280_measure measure;

    while(true) {
        if(xQueueReceive(bmp280_queue, &measure, 1100 / portTICK_RATE_MS) == pdTRUE) {
            sprintf(t_string, "%.1f", measure.temperature);
            sprintf(p_string, "%.1f", measure.pressure);

            I2C_MUTEX(ssd1306_printFixed(0, 16, t_string, STYLE_NORMAL));
            I2C_MUTEX(ssd1306_printFixed(64, 16, p_string, STYLE_NORMAL));
        }
    }

}