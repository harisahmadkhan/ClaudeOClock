#pragma once
#include <lvgl.h>

// Background
#define THEME_BG          lv_color_hex(0x000000)
#define THEME_PANEL       lv_color_hex(0x1a1a18)
#define THEME_PANEL_ALT   lv_color_hex(0x242420)

// Text
#define THEME_TEXT        lv_color_hex(0xfaf9f5)
#define THEME_DIM         lv_color_hex(0x9b9990)

// Accent
#define THEME_ORANGE      lv_color_hex(0xd97757)
#define THEME_ORANGE_DIM  lv_color_hex(0x8a4c38)

// Usage bar status colors
#define THEME_GREEN       lv_color_hex(0x6a8a4a)
#define THEME_AMBER       lv_color_hex(0xd97757)
#define THEME_RED         lv_color_hex(0xb83030)

// Bar track (unfilled portion)
#define THEME_BAR_BG      lv_color_hex(0x2a2a26)

// Helper: pick bar color by percentage
static inline lv_color_t theme_bar_color(float pct) {
    if (pct >= 80.0f) return THEME_RED;
    if (pct >= 50.0f) return THEME_AMBER;
    return THEME_GREEN;
}
