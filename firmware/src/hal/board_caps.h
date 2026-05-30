#pragma once
#include <stdint.h>
#include <stdbool.h>

struct BoardCaps {
    uint16_t width;
    uint16_t height;
    const char* name;
    uint8_t  button_count;
    bool has_touch;
    bool has_battery;
    bool has_imu;
    bool has_wifi;
};

// Implemented in boards/<name>/caps.cpp
const BoardCaps& board_caps();
