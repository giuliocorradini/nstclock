#include "config_menu.h"
#include "ssd1306.h"
#include "i2c.h"

SAppMenu config_menu;
const char *config_menu_items[] = {
    "Time",
    "Measurement",
    "Connection",
    "Update",
    "Exit"
};

void config_menu_init() {
    ssd1306_createMenu(&config_menu, config_menu_items, sizeof(config_menu_items) / sizeof(char *));
}

void config_menu_callback(void *evt) {
    ssd1306_menuDown(&config_menu);
    I2C_MUTEX(ssd1306_updateMenu(&config_menu));
}
