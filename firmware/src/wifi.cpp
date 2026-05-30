#include "wifi.h"
#include "ui.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <esp_random.h>
#include <Arduino.h>

#define MDNS_HOSTNAME  "claudeoclock"
#define HTTP_PORT      80
#define AUTH_HEADER    "X-ClaudeOclock-Token"

static WebServer       _server(HTTP_PORT);
static wifi_data_cb_t  _data_cb        = nullptr;
static String          _token;
static bool            _token_claimed  = false;
static bool            _server_started = false;
static uint32_t        _last_reconnect = 0;
static UsageData       _usage          = {};

// ── Token management ─────────────────────────────────────────────────────────

static void _load_or_generate_token() {
    Preferences prefs;
    prefs.begin("auth", true);
    _token         = prefs.getString("token", "");
    _token_claimed = prefs.getBool("claimed", false);
    prefs.end();

    if (_token.length() != 32) {
        // First boot — generate 32-char hex token via hardware RNG
        uint8_t raw[16];
        for (int i = 0; i < 16; i++) raw[i] = (uint8_t)(esp_random() & 0xFF);
        char hex[33];
        for (int i = 0; i < 16; i++) sprintf(hex + i * 2, "%02x", raw[i]);
        hex[32] = '\0';
        _token = String(hex);

        Preferences w;
        w.begin("auth", false);
        w.putString("token", _token);
        w.putBool("claimed", false);
        w.end();
        Serial.printf("[wifi] Token generated: %s\n", _token.c_str());
    }
}

// ── JSON parser ───────────────────────────────────────────────────────────────

static bool _parse_json(const String& body) {
    JsonDocument doc;
    if (deserializeJson(doc, body)) return false;

    _usage.session_pct            = doc["s"]     | 0.0f;
    _usage.session_reset_mins     = doc["sr"]    | -1;
    _usage.weekly_pct             = doc["w"]     | 0.0f;
    _usage.weekly_reset_mins      = doc["wr"]    | -1;
    _usage.ai_session_pct         = doc["ai_s"]  | -1.0f;
    _usage.ai_session_reset_mins  = doc["ai_sr"] | -1;
    _usage.ai_weekly_pct          = doc["ai_w"]  | -1.0f;
    _usage.ai_weekly_reset_mins   = doc["ai_wr"] | -1;
    _usage.ok                     = doc["ok"]    | false;

    const char* st = doc["st"] | "unknown";
    strncpy(_usage.status, st, sizeof(_usage.status) - 1);
    _usage.status[sizeof(_usage.status) - 1] = '\0';
    _usage.valid = true;
    return true;
}

// ── HTTP handlers ─────────────────────────────────────────────────────────────

static void _handle_data() {
    if (_server.header(AUTH_HEADER) != _token) {
        _server.send(401, "application/json", "{\"ok\":false,\"err\":\"unauthorized\"}");
        return;
    }
    if (_parse_json(_server.arg("plain"))) {
        if (_data_cb) _data_cb(&_usage);
        _server.send(200, "application/json", "{\"ok\":true}");
        Serial.printf("[wifi] Data: session=%d%% weekly=%d%%\n",
            (int)_usage.session_pct, (int)_usage.weekly_pct);
    } else {
        _server.send(400, "application/json", "{\"ok\":false}");
    }
}

static void _handle_setup_token() {
    if (_token_claimed) {
        _server.send(404, "application/json", "{\"ok\":false,\"err\":\"already claimed\"}");
        return;
    }
    _server.send(200, "text/plain", _token);
    _token_claimed = true;
    Preferences prefs;
    prefs.begin("auth", false);
    prefs.putBool("claimed", true);
    prefs.end();
    Serial.println("[wifi] Token claimed by daemon installer");
}

// ── HTTP server startup ───────────────────────────────────────────────────────

static void _start_server() {
    const char* headers[] = { AUTH_HEADER };
    _server.collectHeaders(headers, 1);
    _server.on("/data",        HTTP_POST, _handle_data);
    _server.on("/setup/token", HTTP_GET,  _handle_setup_token);
    _server.begin();
    _server_started = true;
    Serial.println("[wifi] HTTP server on port 80");
}

// ── Public API ────────────────────────────────────────────────────────────────

void wifi_init(wifi_data_cb_t on_data) {
    _data_cb = on_data;
    _load_or_generate_token();

    Preferences prefs;
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("password", "");
    prefs.end();

    if (ssid.length() == 0) {
        Serial.println("[wifi] No credentials saved — showing setup screen");
        ui_show_screen(SCREEN_WIFI_SETUP);
        return;
    }

    Serial.printf("[wifi] Connecting to \"%s\"…\n", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    uint32_t t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
        delay(200);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[wifi] Connection failed — showing setup screen");
        ui_show_screen(SCREEN_WIFI_SETUP);
        return;
    }

    Serial.printf("[wifi] Connected — IP: %s\n", WiFi.localIP().toString().c_str());

    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        Serial.println("[wifi] mDNS: claudeoclock.local");
    } else {
        Serial.println("[wifi] mDNS start failed");
    }

    _start_server();
}

void wifi_tick() {
    if (_server_started) {
        _server.handleClient();
        MDNS.update();
    }

    // Attempt reconnect every 30 s if connection dropped
    if (_server_started && WiFi.status() != WL_CONNECTED) {
        uint32_t now = millis();
        if (now - _last_reconnect >= 30000) {
            _last_reconnect = now;
            Serial.println("[wifi] Reconnecting…");
            WiFi.reconnect();
        }
    }
}
