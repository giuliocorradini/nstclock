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

bool config_menu_showing = false;

void config_menu_init() {
    ssd1306_createMenu(&config_menu, config_menu_items, sizeof(config_menu_items) / sizeof(char *));
}

void config_menu_callback(struct menu_event *evt) {
    switch(evt->cause) {
        case BUTTON_DOWN:
            ssd1306_menuDown(&config_menu);
            break;
        case BUTTON_UP:
            ssd1306_menuUp(&config_menu);
            break;
        case BUTTON_ENTER:
            ssd1306_menuSelection(&config_menu);
            break;
        default:
            break;
    }
    I2C_MUTEX(ssd1306_updateMenu(&config_menu));
}
