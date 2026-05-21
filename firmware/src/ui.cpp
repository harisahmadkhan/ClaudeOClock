#include "ui.h"
#include "theme.h"
#include "fonts.h"
#include "icons.h"
#include "logo.h"
#include "hal/power_hal.h"
#include "hal/board_caps.h"
#include <lvgl.h>
#include <Arduino.h>

// ── Layout constants ──────────────────────────────────────────────────────

#define W 480
#define H 480

#define COMBINED_MARGIN     16
#define COMBINED_COL_GAP    10
#define COMBINED_PANEL_W    ((W - 3*COMBINED_MARGIN - COMBINED_COL_GAP) / 2)
#define COMBINED_PANEL_H    160
#define COMBINED_LEFT_X     COMBINED_MARGIN
#define COMBINED_RIGHT_X    (COMBINED_MARGIN + COMBINED_PANEL_W + COMBINED_COL_GAP)
#define COMBINED_ROW1_Y     80
#define COMBINED_ROW2_Y     (COMBINED_ROW1_Y + COMBINED_PANEL_H + 10)

#define HEADER_H   60
#define DETAIL_P1_Y 80
#define DETAIL_P2_Y 256
#define DETAIL_PANEL_H 160
#define DETAIL_PANEL_W (W - 2*COMBINED_MARGIN)

// ── Screens ───────────────────────────────────────────────────────────────

static lv_obj_t* _screens[5] = {};  // indexed by screen_t
static screen_t  _current = SCREEN_SPLASH;

// Splash screen overlay label (uses word list from splash.cpp)
static lv_obj_t* _spinner_splash = nullptr;

// Thinking labels on data screens — cycle through Claude Code-style words
static lv_obj_t* _spinner_combined  = nullptr;
static lv_obj_t* _spinner_ai        = nullptr;
static lv_obj_t* _spinner_code      = nullptr;

static const char* const _thinking_labels[] = {
    "Thinking", "Analyzing", "Considering", "Reflecting", "Synthesizing",
    "Reasoning", "Evaluating", "Reviewing", "Processing", "Examining",
};
static const int _thinking_count = 10;
static int      _thinking_idx  = 0;
static uint32_t _thinking_last = 0;

// Combined screen labels
static lv_obj_t* _ai_session_pct_lbl   = nullptr;
static lv_obj_t* _ai_session_bar       = nullptr;
static lv_obj_t* _ai_session_reset_lbl = nullptr;
static lv_obj_t* _ai_weekly_pct_lbl    = nullptr;
static lv_obj_t* _ai_weekly_bar        = nullptr;
static lv_obj_t* _ai_weekly_reset_lbl  = nullptr;

static lv_obj_t* _cc_session_pct_lbl   = nullptr;
static lv_obj_t* _cc_session_bar       = nullptr;
static lv_obj_t* _cc_session_reset_lbl = nullptr;
static lv_obj_t* _cc_weekly_pct_lbl    = nullptr;
static lv_obj_t* _cc_weekly_bar        = nullptr;
static lv_obj_t* _cc_weekly_reset_lbl  = nullptr;

// Detail screen labels — AI
static lv_obj_t* _ai_d_session_pct_lbl   = nullptr;
static lv_obj_t* _ai_d_session_bar       = nullptr;
static lv_obj_t* _ai_d_session_reset_lbl = nullptr;
static lv_obj_t* _ai_d_weekly_pct_lbl    = nullptr;
static lv_obj_t* _ai_d_weekly_bar        = nullptr;
static lv_obj_t* _ai_d_weekly_reset_lbl  = nullptr;

// Detail screen labels — Code
static lv_obj_t* _cc_d_session_pct_lbl   = nullptr;
static lv_obj_t* _cc_d_session_bar       = nullptr;
static lv_obj_t* _cc_d_session_reset_lbl = nullptr;
static lv_obj_t* _cc_d_weekly_pct_lbl    = nullptr;
static lv_obj_t* _cc_d_weekly_bar        = nullptr;
static lv_obj_t* _cc_d_weekly_reset_lbl  = nullptr;
static lv_obj_t* _cc_d_status_badge      = nullptr;

// BT screen
static lv_obj_t* _bt_status_lbl = nullptr;
static lv_obj_t* _bt_mac_lbl    = nullptr;

// ── Helpers ───────────────────────────────────────────────────────────────

static void _fmt_pct(char* buf, float pct) {
    if (pct < 0.0f) strcpy(buf, "---");
    else snprintf(buf, 8, "%d%%", (int)pct);
}

static void _fmt_reset(char* buf, int mins) {
    if (mins < 0) { strcpy(buf, "Reset time unknown"); return; }
    if (mins == 0) { strcpy(buf, "Resetting…"); return; }
    int d = mins / 1440;
    int h = (mins % 1440) / 60;
    int m = mins % 60;
    if (d > 0 && h > 0)      snprintf(buf, 32, "Resets in %dd %dh", d, h);
    else if (d > 0)           snprintf(buf, 32, "Resets in %dd", d);
    else if (h > 0 && m > 0) snprintf(buf, 32, "Resets in %dh %dm", h, m);
    else if (h > 0)           snprintf(buf, 32, "Resets in %dh", h);
    else                      snprintf(buf, 32, "Resets in %dm", m);
}

static lv_obj_t* _make_screen() {
    lv_obj_t* s = lv_obj_create(NULL);
    lv_obj_set_size(s, W, H);
    lv_obj_set_style_bg_color(s, THEME_BG, 0);
    lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s, 0, 0);
    lv_obj_set_style_pad_all(s, 0, 0);
    return s;
}

static lv_obj_t* _make_label(lv_obj_t* parent, const lv_font_t* font, lv_color_t color, int x, int y, const char* text) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_label_set_text(lbl, text);
    lv_obj_set_pos(lbl, x, y);
    return lbl;
}

static lv_obj_t* _make_bar(lv_obj_t* parent, int x, int y, int w) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_size(bar, w, 12);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_style_bg_color(bar, THEME_BAR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, THEME_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 6, 0);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    return bar;
}

static void _update_bar(lv_obj_t* bar, float pct) {
    if (pct < 0.0f) pct = 0.0f;
    lv_bar_set_value(bar, (int)pct, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, theme_bar_color(pct), LV_PART_INDICATOR);
}

// Pill badge: e.g. "Current ●" or "Weekly ●"
static lv_obj_t* _make_pill(lv_obj_t* parent, int x, int y, const char* text) {
    lv_obj_t* pill = lv_obj_create(parent);
    lv_obj_set_size(pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(pill, x, y);
    lv_obj_set_style_bg_color(pill, THEME_PANEL_ALT, 0);
    lv_obj_set_style_bg_opa(pill, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(pill, 10, 0);
    lv_obj_set_style_pad_hor(pill, 8, 0);
    lv_obj_set_style_pad_ver(pill, 4, 0);
    lv_obj_set_style_border_width(pill, 0, 0);
    lv_obj_t* lbl = lv_label_create(pill);
    lv_obj_set_style_text_font(lbl, FONT_SMALL, 0);
    lv_obj_set_style_text_color(lbl, THEME_DIM, 0);
    lv_label_set_text(lbl, text);
    return pill;
}

// Header (logo + title + battery icon)
static void _make_header(lv_obj_t* screen, const char* title) {
    // Logo image
    static lv_img_dsc_t logo_dsc = {
        .header = { .cf = LV_COLOR_FORMAT_RGB565, .magic = 0, .w = LOGO_W, .h = LOGO_H },
        .data_size = LOGO_W * LOGO_H * 2,
        .data = (const uint8_t*)logo_pixels,
    };
    lv_obj_t* img = lv_image_create(screen);
    lv_image_set_src(img, &logo_dsc);
    lv_obj_set_pos(img, COMBINED_MARGIN, 12);

    // Title
    _make_label(screen, FONT_MEDIUM, THEME_TEXT, COMBINED_MARGIN + LOGO_W + 10, 16, title);

    // Divider line
    lv_obj_t* line = lv_obj_create(screen);
    lv_obj_set_size(line, W - 2*COMBINED_MARGIN, 1);
    lv_obj_set_pos(line, COMBINED_MARGIN, 58);
    lv_obj_set_style_bg_color(line, THEME_PANEL_ALT, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
}

// Spinner text at bottom of screen
static lv_obj_t* _make_spinner_label(lv_obj_t* screen) {
    lv_obj_t* lbl = lv_label_create(screen);
    lv_obj_set_style_text_font(lbl, FONT_SMALL, 0);
    lv_obj_set_style_text_color(lbl, THEME_DIM, 0);
    lv_label_set_text(lbl, "✻ Cogitating…");
    lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, -10);
    return lbl;
}

// A usage panel block (label + bar + reset label)
// Returns the reset label so caller can update it.
static void _make_usage_block(
    lv_obj_t* parent, int x, int y, int w,
    const char* pill_text,
    lv_obj_t** pct_lbl_out,
    lv_obj_t** bar_out,
    lv_obj_t** reset_lbl_out)
{
    // Big percentage number
    *pct_lbl_out = _make_label(parent, FONT_LARGE, THEME_TEXT, x, y, "---");

    // Pill badge
    _make_pill(parent, x + 80, y + 8, pill_text);

    // Bar
    *bar_out = _make_bar(parent, x, y + 56, w);

    // Reset label
    *reset_lbl_out = _make_label(parent, FONT_SMALL, THEME_DIM, x, y + 80, "");
}

// ── Screen builders ───────────────────────────────────────────────────────

static void _build_splash_screen() {
    _screens[SCREEN_SPLASH] = _make_screen();
    // Splash content rendered by splash.cpp via display HAL.
    // LVGL overlay holds the splash word ticker (uses word list, not thinking labels).
    _spinner_splash = _make_label(_screens[SCREEN_SPLASH], FONT_SMALL, THEME_DIM,
        0, H - 28, "✻ Cogitating…");
    lv_obj_set_width(_spinner_splash, W);
    lv_obj_set_style_text_align(_spinner_splash, LV_TEXT_ALIGN_CENTER, 0);
}

static void _build_combined_screen() {
    lv_obj_t* s = _make_screen();
    _screens[SCREEN_COMBINED] = s;
    _make_header(s, "ClaudeOclock");

    // Left panel — claude.ai
    _make_label(s, FONT_SMALL, THEME_ORANGE, COMBINED_LEFT_X, COMBINED_ROW1_Y - 20, "claude.ai");
    _make_usage_block(s, COMBINED_LEFT_X, COMBINED_ROW1_Y, COMBINED_PANEL_W, "Current ●",
        &_ai_session_pct_lbl, &_ai_session_bar, &_ai_session_reset_lbl);
    _make_usage_block(s, COMBINED_LEFT_X, COMBINED_ROW2_Y, COMBINED_PANEL_W, "Weekly ●",
        &_ai_weekly_pct_lbl, &_ai_weekly_bar, &_ai_weekly_reset_lbl);

    // Right panel — Claude Code
    _make_label(s, FONT_SMALL, THEME_ORANGE, COMBINED_RIGHT_X, COMBINED_ROW1_Y - 20, "Claude Code");
    _make_usage_block(s, COMBINED_RIGHT_X, COMBINED_ROW1_Y, COMBINED_PANEL_W, "Current ●",
        &_cc_session_pct_lbl, &_cc_session_bar, &_cc_session_reset_lbl);
    _make_usage_block(s, COMBINED_RIGHT_X, COMBINED_ROW2_Y, COMBINED_PANEL_W, "Weekly ●",
        &_cc_weekly_pct_lbl, &_cc_weekly_bar, &_cc_weekly_reset_lbl);

    _spinner_combined = _make_spinner_label(s);
}

static void _build_ai_screen() {
    lv_obj_t* s = _make_screen();
    _screens[SCREEN_CLAUDE_AI] = s;
    _make_header(s, "claude.ai");
    _make_usage_block(s, COMBINED_MARGIN, DETAIL_P1_Y, DETAIL_PANEL_W, "Current ●",
        &_ai_d_session_pct_lbl, &_ai_d_session_bar, &_ai_d_session_reset_lbl);
    _make_usage_block(s, COMBINED_MARGIN, DETAIL_P2_Y, DETAIL_PANEL_W, "Weekly ●",
        &_ai_d_weekly_pct_lbl, &_ai_d_weekly_bar, &_ai_d_weekly_reset_lbl);
    _spinner_ai = _make_spinner_label(s);
}

static void _build_code_screen() {
    lv_obj_t* s = _make_screen();
    _screens[SCREEN_CLAUDE_CODE] = s;
    _make_header(s, "Claude Code");
    _make_usage_block(s, COMBINED_MARGIN, DETAIL_P1_Y, DETAIL_PANEL_W, "Current ●",
        &_cc_d_session_pct_lbl, &_cc_d_session_bar, &_cc_d_session_reset_lbl);
    _make_usage_block(s, COMBINED_MARGIN, DETAIL_P2_Y, DETAIL_PANEL_W, "Weekly ●",
        &_cc_d_weekly_pct_lbl, &_cc_d_weekly_bar, &_cc_d_weekly_reset_lbl);

    // Status badge (shown only when "limited")
    _cc_d_status_badge = _make_label(s, FONT_SMALL, THEME_AMBER, COMBINED_MARGIN, DETAIL_P1_Y - 24, "");

    _spinner_code = _make_spinner_label(s);
}

static void _build_bt_screen() {
    lv_obj_t* s = _make_screen();
    _screens[SCREEN_BLUETOOTH] = s;
    _make_header(s, "Bluetooth");

    _bt_status_lbl = _make_label(s, FONT_MEDIUM, THEME_TEXT, COMBINED_MARGIN, 80, "Advertising…");
    _bt_mac_lbl    = _make_label(s, FONT_SMALL, THEME_DIM, COMBINED_MARGIN, 120, "");

    _make_label(s, FONT_SMALL, THEME_DIM, COMBINED_MARGIN, 380,
        "ClaudeOclock — built from scratch with Claude Code");
    _make_label(s, FONT_SMALL, THEME_DIM, COMBINED_MARGIN, 400, "By Haris Ahmad Khan");
}

// ── Public API ─────────────────────────────────────────────────────────────

void ui_init() {
    _build_splash_screen();
    _build_combined_screen();
    _build_ai_screen();
    _build_code_screen();
    _build_bt_screen();
}

void ui_show_screen(screen_t s) {
    if (s >= 5) s = SCREEN_SPLASH;
    _current = s;
    lv_screen_load(_screens[s]);
}

screen_t ui_current_screen() {
    return _current;
}

void ui_update(const UsageData* d) {
    char buf[32];

    // Combined screen — claude.ai
    _fmt_pct(buf, d->ai_session_pct);
    lv_label_set_text(_ai_session_pct_lbl, buf);
    _update_bar(_ai_session_bar, d->ai_session_pct);
    _fmt_reset(buf, d->ai_session_reset_mins);
    lv_label_set_text(_ai_session_reset_lbl, buf);

    _fmt_pct(buf, d->ai_weekly_pct);
    lv_label_set_text(_ai_weekly_pct_lbl, buf);
    _update_bar(_ai_weekly_bar, d->ai_weekly_pct);
    _fmt_reset(buf, d->ai_weekly_reset_mins);
    lv_label_set_text(_ai_weekly_reset_lbl, buf);

    // Combined screen — Claude Code
    _fmt_pct(buf, d->session_pct);
    lv_label_set_text(_cc_session_pct_lbl, buf);
    _update_bar(_cc_session_bar, d->session_pct);
    _fmt_reset(buf, d->session_reset_mins);
    lv_label_set_text(_cc_session_reset_lbl, buf);

    _fmt_pct(buf, d->weekly_pct);
    lv_label_set_text(_cc_weekly_pct_lbl, buf);
    _update_bar(_cc_weekly_bar, d->weekly_pct);
    _fmt_reset(buf, d->weekly_reset_mins);
    lv_label_set_text(_cc_weekly_reset_lbl, buf);

    // AI detail screen
    _fmt_pct(buf, d->ai_session_pct);
    lv_label_set_text(_ai_d_session_pct_lbl, buf);
    _update_bar(_ai_d_session_bar, d->ai_session_pct);
    _fmt_reset(buf, d->ai_session_reset_mins);
    lv_label_set_text(_ai_d_session_reset_lbl, buf);

    _fmt_pct(buf, d->ai_weekly_pct);
    lv_label_set_text(_ai_d_weekly_pct_lbl, buf);
    _update_bar(_ai_d_weekly_bar, d->ai_weekly_pct);
    _fmt_reset(buf, d->ai_weekly_reset_mins);
    lv_label_set_text(_ai_d_weekly_reset_lbl, buf);

    // Code detail screen
    _fmt_pct(buf, d->session_pct);
    lv_label_set_text(_cc_d_session_pct_lbl, buf);
    _update_bar(_cc_d_session_bar, d->session_pct);
    _fmt_reset(buf, d->session_reset_mins);
    lv_label_set_text(_cc_d_session_reset_lbl, buf);

    _fmt_pct(buf, d->weekly_pct);
    lv_label_set_text(_cc_d_weekly_pct_lbl, buf);
    _update_bar(_cc_d_weekly_bar, d->weekly_pct);
    _fmt_reset(buf, d->weekly_reset_mins);
    lv_label_set_text(_cc_d_weekly_reset_lbl, buf);

    // Status badge
    bool limited = strcmp(d->status, "limited") == 0;
    lv_label_set_text(_cc_d_status_badge, limited ? "⚠ limited" : "");

    // BT screen
    if (_bt_status_lbl) {
        lv_label_set_text(_bt_status_lbl, "Advertising…");
    }
}

void ui_tick_anim() {
    lv_timer_handler();

    // Rotate thinking labels on all data screens every 1.5 seconds.
    // Uses Claude Code-style introspection words, not the splash word list.
    uint32_t now = millis();
    if (now - _thinking_last >= 1500) {
        _thinking_last = now;
        _thinking_idx  = (_thinking_idx + 1) % _thinking_count;

        char buf[32];
        snprintf(buf, sizeof(buf), "✻ %s…", _thinking_labels[_thinking_idx]);

        if (_spinner_combined) lv_label_set_text(_spinner_combined, buf);
        if (_spinner_ai)       lv_label_set_text(_spinner_ai,       buf);
        if (_spinner_code)     lv_label_set_text(_spinner_code,     buf);
    }
}

void ui_cycle_screen() {
    screen_t next;
    switch (_current) {
        case SCREEN_SPLASH:      next = SCREEN_COMBINED;    break;
        case SCREEN_COMBINED:    next = SCREEN_CLAUDE_AI;   break;
        case SCREEN_CLAUDE_AI:   next = SCREEN_CLAUDE_CODE; break;
        case SCREEN_CLAUDE_CODE: next = SCREEN_COMBINED;    break;
        default:                 next = SCREEN_COMBINED;    break;
    }
    ui_show_screen(next);
}
