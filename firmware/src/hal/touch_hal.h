#pragma once
#include <stdint.h>
#include <stdbool.h>

void touch_hal_init();
bool touch_hal_read(int16_t* x, int16_t* y, bool* pressed);
