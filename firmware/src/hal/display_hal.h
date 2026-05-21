#pragma once
#include <stdint.h>
#include <stdbool.h>

void     display_hal_init();
void     display_hal_begin();
void     display_hal_draw_bitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data);
void     display_hal_set_brightness(uint8_t level);
void     display_hal_tick();
void     display_hal_round_area(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
uint16_t display_hal_width();
uint16_t display_hal_height();
