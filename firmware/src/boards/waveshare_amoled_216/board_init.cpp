#include "board.h"
#include <Wire.h>
#include <Arduino.h>

void board_init() {
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    Wire.setClock(400000);
    Serial.println("[board] Waveshare ESP32-S3-Touch-AMOLED-2.16 init complete");
}
