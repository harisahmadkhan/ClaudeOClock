#pragma once
#include <stdint.h>
#include <stdbool.h>

void    power_hal_init();
void    power_hal_tick();
uint8_t power_hal_battery_pct();
bool    power_hal_is_charging();
bool    power_hal_is_usb_connected();
bool    power_hal_pwr_pressed();   // AXP PKEY short press event
