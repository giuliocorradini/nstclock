#include "i2c.h"
#include "esp_err.h"
#include "driver/i2c.h"

void i2c_configure() {
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = 22,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_1, &config));
    i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 128, 128, 0);
}

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs such as "bmg250_soft_reset",
 *      "bmg250_set_foc", "bmg250_perform_self_test"  and so on.
 *
 *  @param[in] period_ms  : the required wait time in milliseconds.
 *  @return void.
 *
 */
void delay_ms(uint32_t period_ms)
{
    vTaskDelay(1000 / portTICK_RATE_MS);
}

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose value is to be written.
 *  @param[in] length   : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    //  Write pointer to register (it'll be read in the next transaction)
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1 | I2C_MASTER_WRITE), true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, reg_data, length, true);
    i2c_master_stop(cmd);

    err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 200 / portTICK_RATE_MS);

    return err;
}

/*!
 *  @brief Function for reading the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : Sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[out] reg_data    : Pointer to the data buffer to store the read data.
 *  @param[in] length   : No of bytes to read.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
    esp_err_t err;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    //  Write pointer to register (it'll be read in the next transaction)
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1 | I2C_MASTER_WRITE), true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1 | I2C_MASTER_READ), true);
    i2c_master_read(cmd, reg_data, length, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    err = i2c_master_cmd_begin(I2C_NUM_1, cmd, 200 / portTICK_RATE_MS);

    return err;
}