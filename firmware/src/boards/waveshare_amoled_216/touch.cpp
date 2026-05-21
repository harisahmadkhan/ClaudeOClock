#include "../../hal/touch_hal.h"
#include "board.h"
#include <SensorLib.h>
#include <Arduino.h>

static SensorCST9220 _touch;

void touch_hal_init() {
    if (!_touch.begin(Wire, BOARD_TOUCH_ADDR, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("[touch] CST9220 init failed");
        return;
    }
    // Required for 2.16" board orientation
    _touch.setSwapXY(true);
    _touch.setMirrorXY(true, false);
    Serial.println("[touch] CST9220 ready");
}

bool touch_hal_read(int16_t* x, int16_t* y, bool* pressed) {
    if (_touch.isPressed()) {
        *pressed = true;
        *x = _touch.getX();
        *y = _touch.getY();
        return true;
    }
    *pressed = false;
    return false;
}
