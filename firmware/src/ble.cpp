#include "ble.h"
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <Arduino.h>

// ── UUIDs ─────────────────────────────────────────────────────────────────

#define DEVICE_NAME   "ClaudeOclock"
#define SERVICE_UUID  "c1a7d001-c1a7-d001-c1a7-d001c1a7d001"
#define RX_CHAR_UUID  "c1a7d001-c1a7-d001-c1a7-d001c1a7d002"
#define TX_CHAR_UUID  "c1a7d001-c1a7-d001-c1a7-d001c1a7d003"
#define REQ_CHAR_UUID "c1a7d001-c1a7-d001-c1a7-d001c1a7d004"

// ── State ─────────────────────────────────────────────────────────────────

static NimBLEServer*         _server      = nullptr;
static NimBLECharacteristic* _rx_char     = nullptr;  // daemon writes here
static NimBLECharacteristic* _tx_char     = nullptr;  // ack notify
static NimBLECharacteristic* _req_char    = nullptr;  // refresh req notify
static NimBLEHIDDevice*      _hid         = nullptr;
static NimBLECharacteristic* _hid_input   = nullptr;

static volatile bool _data_ready = false;
static uint8_t       _rx_buf[513];
static uint8_t       _rx_len = 0;

static bool _data_connected = false;
static bool _hid_connected  = false;

// ── Server callbacks ──────────────────────────────────────────────────────

class ServerCB : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* server, NimBLEConnInfo& info) override {
        Serial.printf("[BLE] Client connected: %s\n", info.getAddress().toString().c_str());
        // Allow up to 2 simultaneous connections (data + HID)
        if (server->getConnectedCount() < 2) {
            NimBLEDevice::startAdvertising();
        }
    }
    void onDisconnect(NimBLEServer* server, NimBLEConnInfo& info, int reason) override {
        Serial.printf("[BLE] Client disconnected, reason=%d\n", reason);
        NimBLEDevice::startAdvertising();
    }
};

// ── RX characteristic callbacks ───────────────────────────────────────────

class RxCB : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* ch, NimBLEConnInfo& info) override {
        auto val = ch->getValue();
        if (val.size() > 0 && val.size() <= 512) {
            memcpy(_rx_buf, val.data(), val.size());
            _rx_buf[val.size()] = '\0';
            _rx_len = (uint8_t)val.size();
            _data_ready = true;
        }
    }
};

// ── HID report descriptor — single key keyboard ───────────────────────────

static const uint8_t _hid_descriptor[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection (Application)
    0x85, 0x01,  //   Report ID (1)
    0x05, 0x07,  //   Usage Page (Key Codes)
    0x19, 0xE0,  //   Usage Minimum (Left Control)
    0x29, 0xE7,  //   Usage Maximum (Right GUI)
    0x15, 0x00,  //   Logical Minimum (0)
    0x25, 0x01,  //   Logical Maximum (1)
    0x75, 0x01,  //   Report Size (1)
    0x95, 0x08,  //   Report Count (8)
    0x81, 0x02,  //   Input (Data, Variable, Absolute) — modifiers
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x08,  //   Report Size (8)
    0x81, 0x03,  //   Input (Constant) — reserved
    0x95, 0x06,  //   Report Count (6)
    0x75, 0x08,  //   Report Size (8)
    0x15, 0x00,  //   Logical Minimum (0)
    0x25, 0x65,  //   Logical Maximum (101)
    0x05, 0x07,  //   Usage Page (Key Codes)
    0x19, 0x00,  //   Usage Minimum (0)
    0x29, 0x65,  //   Usage Maximum (101)
    0x81, 0x00,  //   Input (Data, Array, Absolute)
    0xC0,        // End Collection
};

// ── Init ──────────────────────────────────────────────────────────────────

void ble_init() {
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    _server = NimBLEDevice::createServer();
    _server->setCallbacks(new ServerCB());

    // ── Custom data service ────────────────────────────────────────────────
    NimBLEService* svc = _server->createService(SERVICE_UUID);

    _rx_char = svc->createCharacteristic(RX_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    _rx_char->setCallbacks(new RxCB());

    _tx_char = svc->createCharacteristic(TX_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    _req_char = svc->createCharacteristic(REQ_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    svc->start();

    // ── HID keyboard service ───────────────────────────────────────────────
    _hid = new NimBLEHIDDevice(_server);
    _hid->setManufacturer("ClaudeOClock");
    _hid->setPnp(0x02, 0x05AC, 0x820A, 0x0110);
    _hid->setHidInfo(0x00, 0x01);
    _hid->reportMap((uint8_t*)_hid_descriptor, sizeof(_hid_descriptor));

    _hid_input = _hid->inputReport(1);
    _hid->startServices();

    // ── Advertising ────────────────────────────────────────────────────────
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->setName(DEVICE_NAME);
    adv->addServiceUUID(NimBLEUUID((uint16_t)0x1812));  // HID in primary
    adv->setScanResponse(true);
    // Custom service UUID in scan response (for daemon discovery)
    NimBLEAdvertisementData scanResp;
    scanResp.addServiceUUID(SERVICE_UUID);
    adv->setScanResponseData(scanResp);
    adv->start();

    Serial.println("[BLE] Advertising as '" DEVICE_NAME "'");
}

void ble_tick() {
    // Heartbeat: request refresh from daemon if we haven't received data in >90s
    // (Currently handled by daemon polling, not needed here)
}

bool ble_has_data() {
    return _data_ready;
}

uint8_t ble_get_data(uint8_t* buf, uint8_t max_len) {
    if (!_data_ready) return 0;
    uint8_t len = _rx_len < max_len ? _rx_len : max_len;
    memcpy(buf, _rx_buf, len);
    _data_ready = false;
    return len;
}

void ble_send_ack(bool ok) {
    if (_tx_char && _server->getConnectedCount() > 0) {
        const char* ack = ok ? "ok" : "err";
        _tx_char->setValue((uint8_t*)ack, strlen(ack));
        _tx_char->notify();
    }
}

bool ble_is_connected() {
    return _server && _server->getConnectedCount() > 0;
}

bool ble_hid_is_connected() {
    return _hid_input != nullptr && _server && _server->getConnectedCount() > 0;
}

void ble_hid_send_key(uint8_t keycode, uint8_t modifier) {
    if (!_hid_input) return;
    // Key press
    uint8_t report[8] = { modifier, 0, keycode, 0, 0, 0, 0, 0 };
    _hid_input->setValue(report, sizeof(report));
    _hid_input->notify();
    delay(10);
    // Key release
    memset(report, 0, sizeof(report));
    _hid_input->setValue(report, sizeof(report));
    _hid_input->notify();
}
