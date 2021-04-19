#ifndef __i2c_h
#define __i2c_h

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t i2c_mutex;
#define I2C_MUTEX(func) xSemaphoreTake(i2c_mutex, 1000 / portTICK_RATE_MS);\
                                    func;\
                                    xSemaphoreGive(i2c_mutex);

void i2c_configure();
void i2c_set_controller(int controller);
int i2c_get_controller();
int8_t i2c_bmp_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t i2c_bmp_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

#endif