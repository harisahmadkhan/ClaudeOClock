#include "../../hal/power_hal.h"
#include "board.h"
#include <XPowersLib.h>
#include <Arduino.h>

static XPowersAXP2101 _pmu;
static bool _pmu_ok = false;
static bool _pwr_pressed = false;

void power_hal_init() {
    _pmu_ok = _pmu.begin(Wire, BOARD_PMU_ADDR, BOARD_I2C_SDA, BOARD_I2C_SCL);
    if (!_pmu_ok) {
        Serial.println("[power] AXP2101 not found");
        return;
    }
    _pmu.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);
    _pmu.enableBattDetection();
    _pmu.enableVbusVoltageMeasure();
    _pmu.enableBattVoltageMeasure();
    Serial.println("[power] AXP2101 ready");
}

void power_hal_tick() {
    if (!_pmu_ok) return;
    if (_pmu.isPekeyShortPress()) {
        _pwr_pressed = true;
        _pmu.clearIRQ();
    }
}

uint8_t power_hal_battery_pct() {
    if (!_pmu_ok) return 0;
    return (uint8_t)_pmu.getBatteryPercent();
}

bool power_hal_is_charging() {
    if (!_pmu_ok) return false;
    return _pmu.isCharging();
}

bool power_hal_is_usb_connected() {
    if (!_pmu_ok) return false;
    return _pmu.isVbusIn();
}

bool power_hal_pwr_pressed() {
    if (_pwr_pressed) {
        _pwr_pressed = false;
        return true;
    }
    return false;
}
