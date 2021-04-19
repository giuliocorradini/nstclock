#ifndef __i2c_h
#define __i2c_h

#include <stdint.h>

void i2c_configure();
void i2c_set_controller(int controller);
int i2c_get_controller();
int8_t i2c_bmp_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t i2c_bmp_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

#endif