#include "../../hal/board_caps.h"
#include "board.h"

static const BoardCaps _caps = {
    .width        = BOARD_LCD_WIDTH,
    .height       = BOARD_LCD_HEIGHT,
    .name         = "Waveshare ESP32-S3-Touch-AMOLED-2.16",
    .button_count = 2,
    .has_touch    = true,
    .has_battery  = true,
    .has_imu      = true,
    .has_ble      = true,
};

const BoardCaps& board_caps() {
    return _caps;
}
