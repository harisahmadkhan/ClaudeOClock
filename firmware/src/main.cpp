#include <Arduino.h>
#include <lvgl.h>

#include "boards/waveshare_amoled_216/board.h"
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "hal/input_hal.h"
#include "hal/power_hal.h"
#include "hal/imu_hal.h"

#include "data.h"
#include "ui.h"
#include "wifi.h"
#include "wifi_setup.h"
#include "splash.h"
#include "idle.h"
#include "usage_rate.h"

// LVGL touch input driver
static lv_indev_t* _touch_indev = nullptr;

static void _touch_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    int16_t x, y;
    bool pressed;
    touch_hal_read(&x, &y, &pressed);
    data->point.x = x;
    data->point.y = y;
    data->state   = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// Animation group tracking
static uint8_t _anim_group = 0;

// Called by wifi.cpp /data handler on each successful push from daemon
static void _on_wifi_data(const UsageData* d) {
    usage_rate_sample(d->session_pct);
    uint8_t grp = usage_zone(d->session_pct);
    if (grp != _anim_group) {
        _anim_group = grp;
        splash_set_group(_anim_group);
    }
    ui_update(d);
}

// Touch: tap anywhere toggles current screen ↔ SPLASH
static bool _touch_was_pressed = false;

static void _handle_touch() {
    int16_t x, y;
    bool pressed;
    touch_hal_read(&x, &y, &pressed);

    if (pressed && !_touch_was_pressed) {
        idle_reset();
        screen_t cur = ui_current_screen();
        if (cur == SCREEN_SPLASH) {
            ui_show_screen(SCREEN_COMBINED);
        } else if (cur != SCREEN_WIFI_SETUP) {
            ui_show_screen(SCREEN_SPLASH);
        }
    }
    _touch_was_pressed = pressed;
}

// Power button long-press detection (>2 s → WiFi setup)
static uint32_t _pwr_press_start = 0;
static bool     _pwr_held        = false;
static bool     _pwr_long_fired  = false;

static void _handle_pwr_button() {
    bool pressed = power_hal_pwr_pressed();

    if (pressed && !_pwr_held) {
        _pwr_held       = true;
        _pwr_long_fired = false;
        _pwr_press_start = millis();
    }

    if (_pwr_held && !_pwr_long_fired) {
        if (millis() - _pwr_press_start >= 2000) {
            // Long press — go to WiFi setup from any screen
            _pwr_long_fired = true;
            idle_reset();
            ui_show_screen(SCREEN_WIFI_SETUP);
        }
    }

    if (!pressed && _pwr_held) {
        // Released
        if (!_pwr_long_fired) {
            // Short press — cycle data screens
            idle_reset();
            screen_t cur = ui_current_screen();
            if (cur == SCREEN_SPLASH) {
                splash_next_anim();
            } else if (cur != SCREEN_WIFI_SETUP) {
                ui_cycle_screen();
            }
        }
        _pwr_held = false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n[main] ClaudeOclock booting…");

    board_init();
    display_hal_init();
    display_hal_begin();
    idle_init();
    power_hal_init();
    imu_hal_init();
    touch_hal_init();

    // Register touch with LVGL
    _touch_indev = lv_indev_create();
    lv_indev_set_type(_touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(_touch_indev, _touch_read_cb);

    input_hal_init();
    usage_rate_init();
    splash_init();

    ui_init();
    wifi_init(_on_wifi_data);   // connects or shows SCREEN_WIFI_SETUP if no creds

    // Only show splash if wifi_init didn't redirect to setup screen
    if (ui_current_screen() != SCREEN_WIFI_SETUP) {
        ui_show_screen(SCREEN_SPLASH);
    }

    Serial.println("[main] Boot complete");
}

void loop() {
    input_hal_tick();
    idle_tick();
    power_hal_tick();
    imu_hal_tick();

    _handle_touch();
    _handle_pwr_button();

    wifi_tick();

    // Service wifi_setup AP server when on setup screen
    if (ui_current_screen() == SCREEN_WIFI_SETUP) {
        wifi_setup_tick();
    }

    // Splash animation tick
    if (ui_current_screen() == SCREEN_SPLASH && !idle_is_asleep()) {
        splash_tick();
    }

    ui_tick_anim();

    if (!idle_is_asleep()) {
        display_hal_tick();
    }

    // Serial screenshot command
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "screenshot") {
            Serial.println("[screenshot] Not yet implemented — connect display HAL framebuffer dump");
        }
    }

    delay(5);
}
