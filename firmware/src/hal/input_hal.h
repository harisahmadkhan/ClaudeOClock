#pragma once
#include <stdint.h>
#include <stdbool.h>

#define INPUT_BTN_PRIMARY    0   // GPIO 0 — BOOT / LEFT
#define INPUT_BTN_SECONDARY  1   // GPIO 18 — RIGHT

void input_hal_init();
bool input_hal_is_pressed(uint8_t btn);
bool input_hal_is_held(uint8_t btn);        // currently held down
bool input_hal_just_pressed(uint8_t btn);   // edge: not pressed → pressed
bool input_hal_just_released(uint8_t btn);  // edge: pressed → released
uint32_t input_hal_held_ms(uint8_t btn);    // ms since button first pressed
void input_hal_tick();                       // call every loop()
