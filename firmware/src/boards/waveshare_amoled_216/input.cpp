#include "../../hal/input_hal.h"
#include "board.h"
#include <Arduino.h>

#define BTN_COUNT 2

static const uint8_t _pins[BTN_COUNT] = { BOARD_BTN_LEFT, BOARD_BTN_RIGHT };

static bool     _prev[BTN_COUNT]    = {};
static bool     _cur[BTN_COUNT]     = {};
static uint32_t _press_ms[BTN_COUNT] = {};

void input_hal_init() {
    for (int i = 0; i < BTN_COUNT; i++) {
        pinMode(_pins[i], INPUT_PULLUP);
    }
}

void input_hal_tick() {
    for (int i = 0; i < BTN_COUNT; i++) {
        _prev[i] = _cur[i];
        _cur[i]  = (digitalRead(_pins[i]) == LOW);  // active-low
        if (_cur[i] && !_prev[i]) _press_ms[i] = millis();
    }
}

bool input_hal_is_pressed(uint8_t btn) {
    return btn < BTN_COUNT && _cur[btn];
}

bool input_hal_is_held(uint8_t btn) {
    return input_hal_is_pressed(btn);
}

bool input_hal_just_pressed(uint8_t btn) {
    return btn < BTN_COUNT && _cur[btn] && !_prev[btn];
}

bool input_hal_just_released(uint8_t btn) {
    return btn < BTN_COUNT && !_cur[btn] && _prev[btn];
}

uint32_t input_hal_held_ms(uint8_t btn) {
    if (btn >= BTN_COUNT || !_cur[btn]) return 0;
    return millis() - _press_ms[btn];
}
