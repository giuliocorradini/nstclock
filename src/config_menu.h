#include "ssd1306.h"
#include "menu.h"
#include "stdbool.h"

extern SAppMenu config_menu;
extern bool config_menu_showing;

void config_menu_init();
void config_menu_callback(struct menu_event *evt);