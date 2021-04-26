#include "bmp280_task.h"

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
        
        if(xQueueSend(bmp280_queue, &measure, 100 / portTICK_RATE_MS) == errQUEUE_FULL) {
            xQueueReset(bmp280_queue);
            xQueueSend(bmp280_queue, &measure, 100 / portTICK_RATE_MS);
        }

        printf("Temperature: %.2f, pressure: %.1f\n",
               temp,
               pres);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}
