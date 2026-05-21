#include <Arduino.h>
#include <ArduinoJson.h>
#include <lvgl.h>

#include "boards/waveshare_amoled_216/board.h"
#include "hal/display_hal.h"
#include "hal/touch_hal.h"
#include "hal/input_hal.h"
#include "hal/power_hal.h"
#include "hal/imu_hal.h"

#include "data.h"
#include "ui.h"
#include "ble.h"
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

// Usage data
static UsageData g_usage = {};

// Parse incoming BLE JSON into g_usage
static bool _parse_json(const uint8_t* buf, uint8_t len) {
    JsonDocument doc;
    auto err = deserializeJson(doc, (const char*)buf, len);
    if (err) return false;

    g_usage.session_pct         = doc["s"]    | 0.0f;
    g_usage.session_reset_mins  = doc["sr"]   | -1;
    g_usage.weekly_pct          = doc["w"]    | 0.0f;
    g_usage.weekly_reset_mins   = doc["wr"]   | -1;
    g_usage.ai_session_pct      = doc["ai_s"] | -1.0f;
    g_usage.ai_session_reset_mins = doc["ai_sr"] | -1;
    g_usage.ai_weekly_pct       = doc["ai_w"] | -1.0f;
    g_usage.ai_weekly_reset_mins = doc["ai_wr"] | -1;
    g_usage.ok                  = doc["ok"]   | false;

    const char* st = doc["st"] | "unknown";
    strncpy(g_usage.status, st, sizeof(g_usage.status) - 1);
    g_usage.status[sizeof(g_usage.status) - 1] = '\0';

    g_usage.valid = true;
    return true;
}

// Handle touch — toggle splash / current screen
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
        } else if (cur != SCREEN_BLUETOOTH) {
            ui_show_screen(SCREEN_SPLASH);
        }
    }
    _touch_was_pressed = pressed;
}

// Animation group tracking
static uint8_t _anim_group = 0;

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

    ble_init();
    input_hal_init();

    usage_rate_init();
    splash_init();

    ui_init();
    ui_show_screen(SCREEN_SPLASH);

    Serial.println("[main] Boot complete");
}

void loop() {
    input_hal_tick();
    idle_tick();
    power_hal_tick();
    imu_hal_tick();
    ble_tick();
    _handle_touch();

    // ── Button: LEFT (GPIO 0) ───────────────────────────────────────────
    if (input_hal_just_released(INPUT_BTN_PRIMARY)) {
        idle_reset();
        uint32_t held = input_hal_held_ms(INPUT_BTN_PRIMARY);

        if (held >= 1500) {
            // Long press → Bluetooth screen
            ui_show_screen(SCREEN_BLUETOOTH);
        } else {
            // Short press → HID Space (Claude Code voice PTT)
            ble_hid_send_key(0x2C);  // HID keycode for Space
        }
    }

    // ── Button: RIGHT (GPIO 18) ─────────────────────────────────────────
    if (input_hal_just_released(INPUT_BTN_SECONDARY)) {
        idle_reset();
        // HID Shift+Tab
        ble_hid_send_key(0x2B, 0x02);  // Tab + left-shift modifier
    }

    // ── Power button (AXP PKEY) ─────────────────────────────────────────
    if (power_hal_pwr_pressed()) {
        idle_reset();
        screen_t cur = ui_current_screen();
        if (cur == SCREEN_SPLASH) {
            splash_next_anim();
        } else {
            ui_cycle_screen();
        }
    }

    // ── BLE data ────────────────────────────────────────────────────────
    if (ble_has_data()) {
        uint8_t buf[513];
        uint8_t len = ble_get_data(buf, sizeof(buf));
        if (_parse_json(buf, len)) {
            usage_rate_sample(g_usage.session_pct);
            uint8_t new_group = usage_rate_group();
            if (new_group != _anim_group) {
                _anim_group = new_group;
                splash_set_group(_anim_group);
            }
            ui_update(&g_usage);
            ble_send_ack(true);
        } else {
            ble_send_ack(false);
        }
    }

    // ── Splash animation tick ────────────────────────────────────────────
    if (ui_current_screen() == SCREEN_SPLASH && !idle_is_asleep()) {
        splash_tick();
    }

    // ── LVGL ─────────────────────────────────────────────────────────────
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
