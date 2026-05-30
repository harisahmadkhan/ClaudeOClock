#pragma once
#include <lvgl.h>

lv_obj_t* wifi_setup_build();  // called once from ui_init(), returns LVGL screen
void       wifi_setup_enter(); // call when switching to SCREEN_WIFI_SETUP
void       wifi_setup_tick();  // call in loop() when SCREEN_WIFI_SETUP is active
