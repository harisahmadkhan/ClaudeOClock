#include "../../hal/display_hal.h"
#include "board.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>

// CO5300 QSPI display driver via Arduino_GFX
// Cannot rotate via MADCTL — rotation done by CPU pixel remapping in strips.

#define BUF_LINES  40

static Arduino_DataBus* _bus = nullptr;
static Arduino_GFX*     _gfx = nullptr;
static uint16_t*         _buf1 = nullptr;
static uint16_t*         _buf2 = nullptr;
static lv_display_t*     _lv_disp = nullptr;

static uint8_t _brightness = 255;

// LVGL flush callback — sends rendered strip to the display
static void _flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    _gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)px_map, w, h);
    lv_display_flush_ready(disp);
}

// LVGL tick source
static uint32_t _tick_cb() {
    return (uint32_t)millis();
}

void display_hal_init() {
    _bus = new Arduino_QSPI(
        BOARD_DISP_CS,
        BOARD_DISP_SCLK,
        BOARD_DISP_SDIO0,
        BOARD_DISP_SDIO1,
        BOARD_DISP_SDIO2,
        BOARD_DISP_SDIO3
    );
    _gfx = new Arduino_CO5300(_bus, BOARD_DISP_RST, 0 /*rotation*/, false /*IPS*/,
                               BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);

    // Allocate LVGL draw buffers from PSRAM
    _buf1 = (uint16_t*)heap_caps_malloc(
        (size_t)BOARD_LCD_WIDTH * BUF_LINES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    _buf2 = (uint16_t*)heap_caps_malloc(
        (size_t)BOARD_LCD_WIDTH * BUF_LINES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!_buf1 || !_buf2) {
        Serial.println("[display] FATAL: PSRAM alloc failed — check qio_opi");
        while (1) delay(1000);
    }
}

void display_hal_begin() {
    if (!_gfx->begin()) {
        Serial.println("[display] FATAL: GFX begin() failed");
        while (1) delay(1000);
    }
    _gfx->fillScreen(BLACK);

    lv_init();
    lv_tick_set_cb(_tick_cb);

    _lv_disp = lv_display_create(BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
    lv_display_set_flush_cb(_lv_disp, _flush_cb);
    lv_display_set_buffers(_lv_disp, _buf1, _buf2,
        (uint32_t)BOARD_LCD_WIDTH * BUF_LINES * sizeof(uint16_t),
        LV_DISPLAY_RENDER_MODE_PARTIAL);

    Serial.println("[display] CO5300 AMOLED ready");
}

void display_hal_draw_bitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* data) {
    _gfx->draw16bitRGBBitmap(x, y, (uint16_t*)data, w, h);
}

void display_hal_set_brightness(uint8_t level) {
    _brightness = level;
    // CO5300 brightness via display command — AXP2101 controls backlight on this board
    // Actual dimming handled by power_hal via AXP2101 DLDO2/BLDO1
    (void)level;
}

void display_hal_tick() {
    // Nothing needed — LVGL timer_handler drives rendering
}

void display_hal_round_area(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    _gfx->fillRect(x, y, w, h, color);
}

uint16_t display_hal_width()  { return BOARD_LCD_WIDTH; }
uint16_t display_hal_height() { return BOARD_LCD_HEIGHT; }
