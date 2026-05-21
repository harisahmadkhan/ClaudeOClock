#include "idle.h"
#include "idle_cfg.h"
#include "hal/display_hal.h"
#include "hal/power_hal.h"
#include <Arduino.h>

static uint32_t _last_activity = 0;
static bool     _asleep        = false;
static bool     _first_wake    = true;    // first press after sleep is wake-only
static uint32_t _fade_start    = 0;
static bool     _fading        = false;

void idle_init() {
    _last_activity = millis();
    _asleep        = false;
    _first_wake    = false;
}

static void _apply_brightness(float t) {
    // t: 0.0 = off, 1.0 = full
    uint8_t level = (uint8_t)(t * IDLE_BRIGHTNESS_FULL);
    display_hal_set_brightness(level);
}

void idle_tick() {
#if IDLE_NO_SLEEP_ON_USB
    if (power_hal_is_usb_connected()) {
        if (_asleep) {
            _asleep = false;
            _first_wake = false;
            _apply_brightness(1.0f);
        }
        _last_activity = millis();
        return;
    }
#endif

    uint32_t now = millis();

    if (_fading) {
        uint32_t elapsed = now - _fade_start;
        if (elapsed >= IDLE_FADE_OUT_MS) {
            _apply_brightness(0.0f);
            _fading = false;
            _asleep = true;
            _first_wake = true;
        } else {
            float t = 1.0f - (float)elapsed / (float)IDLE_FADE_OUT_MS;
            _apply_brightness(t);
        }
        return;
    }

    if (!_asleep && (now - _last_activity) >= IDLE_TIMEOUT_MS) {
        _fading    = true;
        _fade_start = now;
    }
}

void idle_reset() {
    uint32_t now = millis();
    bool was_asleep = _asleep;
    _asleep = false;
    _fading = false;
    _last_activity = now;

    if (was_asleep) {
        // Fade in
        for (int i = 0; i <= 16; i++) {
            float t = (float)i / 16.0f;
            _apply_brightness(t);
            delay(IDLE_FADE_IN_MS / 16);
        }
        _apply_brightness(1.0f);
        _first_wake = true;
    }
}

bool idle_is_asleep() {
    return _asleep || _fading;
}
