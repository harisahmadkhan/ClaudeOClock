#include "wifi_setup.h"
#include "ui.h"
#include "theme.h"
#include "fonts.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <qrcode.h>
#include <lvgl.h>
#include <Arduino.h>

#define SOFTAP_SSID   "ClaudeOclock-Setup"
#define AP_IP         "192.168.4.1"
#define AP_PORT       80

// Sub-states
typedef enum { STATE_SCAN, STATE_QR } setup_state_t;

static setup_state_t  _state          = STATE_SCAN;
static lv_obj_t*      _screen         = nullptr;
static String         _selected_ssid;

// Scan sub-state widgets
static lv_obj_t*  _list              = nullptr;
static lv_obj_t*  _scan_btn          = nullptr;

// QR sub-state widgets
static lv_obj_t*  _qr_container      = nullptr;
static lv_obj_t*  _qr_canvas         = nullptr;
static lv_obj_t*  _ssid_label        = nullptr;
static lv_obj_t*  _ip_label          = nullptr;
static lv_obj_t*  _status_label      = nullptr;
static lv_obj_t*  _back_btn          = nullptr;

// SoftAP web server for password entry
static WebServer* _ap_server         = nullptr;
static bool       _ap_active         = false;
static bool       _credentials_saved = false;

// Canvas pixel buffer for QR code (128×128 pixels, RGB565)
#define QR_CANVAS_PX 128
static lv_color_t _qr_buf[QR_CANVAS_PX * QR_CANVAS_PX];

// ── Helpers ───────────────────────────────────────────────────────────────────

static void _stop_ap() {
    if (_ap_active) {
        if (_ap_server) {
            _ap_server->stop();
            delete _ap_server;
            _ap_server = nullptr;
        }
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        _ap_active = false;
        Serial.println("[wifi_setup] SoftAP closed");
    }
}

static void _draw_qr(const char* url) {
    QRCode qr;
    uint8_t qr_data[qrcode_getBufferSize(3)];
    qrcode_initText(&qr, qr_data, 3, ECC_LOW, url);

    int modules = qr.size;
    int cell    = QR_CANVAS_PX / (modules + 4);  // +4 for quiet zone
    int offset  = (QR_CANVAS_PX - cell * modules) / 2;

    // White background
    for (int i = 0; i < QR_CANVAS_PX * QR_CANVAS_PX; i++) {
        _qr_buf[i] = lv_color_white();
    }

    // Draw modules
    for (int y = 0; y < modules; y++) {
        for (int x = 0; x < modules; x++) {
            lv_color_t c = qrcode_getModule(&qr, x, y)
                           ? lv_color_black()
                           : lv_color_white();
            for (int dy = 0; dy < cell; dy++) {
                for (int dx = 0; dx < cell; dx++) {
                    int px = offset + x * cell + dx;
                    int py = offset + y * cell + dy;
                    if (px < QR_CANVAS_PX && py < QR_CANVAS_PX) {
                        _qr_buf[py * QR_CANVAS_PX + px] = c;
                    }
                }
            }
        }
    }

    lv_canvas_set_buffer(_qr_canvas, _qr_buf,
                         QR_CANVAS_PX, QR_CANVAS_PX,
                         LV_COLOR_FORMAT_RGB565);
    lv_obj_invalidate(_qr_canvas);
}

// ── SoftAP web server ─────────────────────────────────────────────────────────

static const char _FORM_HTML[] =
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>ClaudeOclock WiFi Setup</title>"
    "<style>body{font-family:sans-serif;max-width:400px;margin:40px auto;padding:0 16px}"
    "input{width:100%;padding:10px;margin:8px 0;box-sizing:border-box;font-size:16px}"
    "button{width:100%;padding:12px;background:#d97757;color:#fff;border:none;"
    "font-size:16px;border-radius:6px;cursor:pointer}</style></head><body>"
    "<h2>ClaudeOclock WiFi Setup</h2>"
    "<p>Connecting to: <strong>%s</strong></p>"
    "<form method='POST' action='/connect'>"
    "<input type='password' name='pw' placeholder='WiFi Password' autofocus>"
    "<button type='submit'>Connect</button></form>"
    "<p style='color:#888;font-size:14px'>built with curiosity by Haris Ahmad Khan</p>"
    "</body></html>";

static const char _OK_HTML[] =
    "<!DOCTYPE html><html><body style='font-family:sans-serif;text-align:center;margin-top:80px'>"
    "<h2 style='color:#6a8a4a'>&#10003; Connected!</h2>"
    "<p>ClaudeOclock is now connected to your network.</p>"
    "<p>You can close this page.</p></body></html>";

static const char _ERR_HTML[] =
    "<!DOCTYPE html><html><body style='font-family:sans-serif;text-align:center;margin-top:80px'>"
    "<h2 style='color:#b83030'>&#10007; Wrong password</h2>"
    "<p>Could not connect. Please try again.</p>"
    "<a href='/'>Back</a></body></html>";

static void _ap_serve_form() {
    char buf[512];
    snprintf(buf, sizeof(buf), _FORM_HTML, _selected_ssid.c_str());
    _ap_server->send(200, "text/html", buf);
}

static void _ap_handle_connect() {
    if (!_ap_server->hasArg("pw")) {
        _ap_server->send(400, "text/plain", "Missing password");
        return;
    }
    String pw = _ap_server->arg("pw");

    // Save credentials tentatively and attempt connection
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(_selected_ssid.c_str(), pw.c_str());

    if (_status_label) lv_label_set_text(_status_label, "Connecting…");

    uint32_t t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 12000) {
        delay(200);
        if (_ap_server) _ap_server->handleClient();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Preferences prefs;
        prefs.begin("wifi", false);
        prefs.putString("ssid",     _selected_ssid);
        prefs.putString("password", pw);
        prefs.end();
        _credentials_saved = true;
        _ap_server->send(200, "text/html", _OK_HTML);
        if (_status_label) lv_label_set_text(_status_label, "Connected! Rebooting…");
        Serial.println("[wifi_setup] Credentials saved — rebooting");
        delay(2000);
        ESP.restart();
    } else {
        _ap_server->send(200, "text/html", _ERR_HTML);
        if (_status_label) lv_label_set_text(_status_label, "Wrong password — try again");
        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
    }
}

static void _start_ap() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SOFTAP_SSID);
    Serial.printf("[wifi_setup] SoftAP \"%s\" started — IP: %s\n", SOFTAP_SSID, AP_IP);

    _ap_server = new WebServer(AP_PORT);
    _ap_server->on("/",        HTTP_GET,  _ap_serve_form);
    _ap_server->on("/connect", HTTP_POST, _ap_handle_connect);
    _ap_server->begin();
    _ap_active = true;
}

// ── Sub-state transitions ─────────────────────────────────────────────────────

static void _show_scan_state();
static void _show_qr_state(const String& ssid);

// LVGL list item tap handler
static void _on_ssid_tap(lv_event_t* e) {
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    const char* ssid = lv_label_get_text(lbl);
    _show_qr_state(String(ssid));
}

static void _on_scan_btn(lv_event_t* e) {
    _show_scan_state();
}

static void _on_back_btn(lv_event_t* e) {
    _stop_ap();
    _show_scan_state();
}

static void _hide_all() {
    if (_list)         lv_obj_add_flag(_list,         LV_OBJ_FLAG_HIDDEN);
    if (_scan_btn)     lv_obj_add_flag(_scan_btn,     LV_OBJ_FLAG_HIDDEN);
    if (_qr_container) lv_obj_add_flag(_qr_container, LV_OBJ_FLAG_HIDDEN);
    if (_back_btn)     lv_obj_add_flag(_back_btn,     LV_OBJ_FLAG_HIDDEN);
}

static void _show_scan_state() {
    _state = STATE_SCAN;
    _hide_all();

    // Populate list with scan results
    lv_obj_clean(_list);
    lv_obj_clear_flag(_list, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_scan_btn, LV_OBJ_FLAG_HIDDEN);

    int n = WiFi.scanNetworks();
    if (n == 0) {
        lv_obj_t* lbl = lv_label_create(_list);
        lv_label_set_text(lbl, "No networks found");
        lv_obj_set_style_text_color(lbl, THEME_DIM, 0);
    } else {
        for (int i = 0; i < n && i < 8; i++) {
            lv_obj_t* btn = lv_btn_create(_list);
            lv_obj_set_width(btn, lv_pct(100));
            lv_obj_set_style_bg_color(btn, THEME_PANEL, 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(btn, 6, 0);
            lv_obj_set_style_pad_all(btn, 10, 0);
            lv_obj_add_event_cb(btn, _on_ssid_tap, LV_EVENT_CLICKED, nullptr);

            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, WiFi.SSID(i).c_str());
            lv_obj_set_style_text_font(lbl, FONT_SMALL, 0);
            lv_obj_set_style_text_color(lbl, THEME_TEXT, 0);
        }
    }
    WiFi.scanDelete();
}

static void _show_qr_state(const String& ssid) {
    _state = STATE_SCAN;  // will be set to STATE_QR below
    _selected_ssid = ssid;
    _hide_all();

    lv_obj_clear_flag(_qr_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_back_btn,     LV_OBJ_FLAG_HIDDEN);

    if (_ssid_label) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Connect to: %s", ssid.c_str());
        lv_label_set_text(_ssid_label, buf);
    }
    if (_ip_label)     lv_label_set_text(_ip_label,     "http://" AP_IP);
    if (_status_label) lv_label_set_text(_status_label, "Scan QR or visit link on your phone");

    _draw_qr("http://" AP_IP);
    _start_ap();
    _state = STATE_QR;
}

// ── Screen builder ────────────────────────────────────────────────────────────

lv_obj_t* wifi_setup_build() {
    _screen = lv_obj_create(NULL);
    lv_obj_set_size(_screen, 480, 480);
    lv_obj_set_style_bg_color(_screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_screen, 0, 0);
    lv_obj_set_style_pad_all(_screen, 0, 0);

    // Title
    lv_obj_t* title = lv_label_create(_screen);
    lv_label_set_text(title, "WiFi Setup");
    lv_obj_set_style_text_font(title, FONT_MEDIUM, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xfaf9f5), 0);
    lv_obj_set_pos(title, 16, 16);

    lv_obj_t* sub = lv_label_create(_screen);
    lv_label_set_text(sub, "Select your network:");
    lv_obj_set_style_text_font(sub, FONT_SMALL, 0);
    lv_obj_set_style_text_color(sub, lv_color_hex(0x9b9990), 0);
    lv_obj_set_pos(sub, 16, 52);

    // Scan list (sub-state A)
    _list = lv_obj_create(_screen);
    lv_obj_set_size(_list, 448, 340);
    lv_obj_set_pos(_list, 16, 76);
    lv_obj_set_style_bg_color(_list, lv_color_hex(0x1a1a18), 0);
    lv_obj_set_style_bg_opa(_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_list, 0, 0);
    lv_obj_set_style_radius(_list, 8, 0);
    lv_obj_set_flex_flow(_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(_list, 8, 0);
    lv_obj_set_style_pad_row(_list, 6, 0);

    _scan_btn = lv_btn_create(_screen);
    lv_obj_set_size(_scan_btn, 160, 40);
    lv_obj_align(_scan_btn, LV_ALIGN_BOTTOM_MID, 0, -12);
    lv_obj_set_style_bg_color(_scan_btn, lv_color_hex(0x242420), 0);
    lv_obj_set_style_radius(_scan_btn, 8, 0);
    lv_obj_add_event_cb(_scan_btn, _on_scan_btn, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* scan_lbl = lv_label_create(_scan_btn);
    lv_label_set_text(scan_lbl, "Scan again");
    lv_obj_set_style_text_font(scan_lbl, FONT_SMALL, 0);
    lv_obj_center(scan_lbl);

    // QR container (sub-state B)
    _qr_container = lv_obj_create(_screen);
    lv_obj_set_size(_qr_container, 448, 360);
    lv_obj_set_pos(_qr_container, 16, 76);
    lv_obj_set_style_bg_color(_qr_container, lv_color_hex(0x1a1a18), 0);
    lv_obj_set_style_bg_opa(_qr_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_qr_container, 0, 0);
    lv_obj_set_style_radius(_qr_container, 8, 0);
    lv_obj_set_style_pad_all(_qr_container, 12, 0);
    lv_obj_add_flag(_qr_container, LV_OBJ_FLAG_HIDDEN);

    _ssid_label = lv_label_create(_qr_container);
    lv_obj_set_style_text_font(_ssid_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(_ssid_label, lv_color_hex(0xfaf9f5), 0);
    lv_obj_set_pos(_ssid_label, 0, 0);

    _qr_canvas = lv_canvas_create(_qr_container);
    lv_obj_set_size(_qr_canvas, QR_CANVAS_PX, QR_CANVAS_PX);
    lv_obj_set_pos(_qr_canvas, 0, 28);

    _ip_label = lv_label_create(_qr_container);
    lv_obj_set_style_text_font(_ip_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(_ip_label, lv_color_hex(0xd97757), 0);
    lv_obj_set_pos(_ip_label, QR_CANVAS_PX + 12, 28);

    lv_obj_t* instr = lv_label_create(_qr_container);
    lv_label_set_text(instr, "Or open on your phone\nand enter your WiFi\npassword.");
    lv_obj_set_style_text_font(instr, FONT_SMALL, 0);
    lv_obj_set_style_text_color(instr, lv_color_hex(0x9b9990), 0);
    lv_obj_set_pos(instr, QR_CANVAS_PX + 12, 56);

    _status_label = lv_label_create(_qr_container);
    lv_obj_set_style_text_font(_status_label, FONT_SMALL, 0);
    lv_obj_set_style_text_color(_status_label, lv_color_hex(0x9b9990), 0);
    lv_obj_set_pos(_status_label, 0, QR_CANVAS_PX + 36);

    // Back button (sub-state B only)
    _back_btn = lv_btn_create(_screen);
    lv_obj_set_size(_back_btn, 120, 40);
    lv_obj_align(_back_btn, LV_ALIGN_BOTTOM_MID, 0, -12);
    lv_obj_set_style_bg_color(_back_btn, lv_color_hex(0x242420), 0);
    lv_obj_set_style_radius(_back_btn, 8, 0);
    lv_obj_add_flag(_back_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(_back_btn, _on_back_btn, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* back_lbl = lv_label_create(_back_btn);
    lv_label_set_text(back_lbl, "Back");
    lv_obj_set_style_text_font(back_lbl, FONT_SMALL, 0);
    lv_obj_center(back_lbl);

    // Footer credit
    lv_obj_t* credit = lv_label_create(_screen);
    lv_label_set_text(credit, "built and designed with curiosity by Haris Ahmad Khan");
    lv_obj_set_style_text_font(credit, FONT_SMALL, 0);
    lv_obj_set_style_text_color(credit, lv_color_hex(0x4a4a46), 0);
    lv_obj_align(credit, LV_ALIGN_BOTTOM_MID, 0, -56);

    return _screen;
}

void wifi_setup_enter() {
    _show_scan_state();
}

void wifi_setup_tick() {
    if (_ap_active && _ap_server) {
        _ap_server->handleClient();
    }
}
