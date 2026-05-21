#include "../../hal/imu_hal.h"
#include "board.h"
#include <SensorLib.h>
#include <Arduino.h>

static SensorQMI8658 _imu;
static bool _imu_ok = false;

void imu_hal_init() {
    _imu_ok = _imu.begin(Wire, BOARD_IMU_ADDR, BOARD_I2C_SDA, BOARD_I2C_SCL);
    if (!_imu_ok) {
        Serial.println("[imu] QMI8658 not found");
        return;
    }
    _imu.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_100Hz);
    _imu.enableAccelerometer();
    Serial.println("[imu] QMI8658 ready");
}

void imu_hal_tick() {
    // Readings consumed on demand via imu_hal_rotation_quadrant()
}

uint8_t imu_hal_rotation_quadrant() {
    if (!_imu_ok) return 0;

    IMUdata acc;
    if (!_imu.getAccelerometer(acc.x, acc.y, acc.z)) return 0;

    // Determine screen quadrant from dominant gravity axis
    float ax = acc.x, ay = acc.y;
    if (fabsf(ax) > fabsf(ay)) {
        return (ax > 0) ? 1 : 3;
    } else {
        return (ay > 0) ? 0 : 2;
    }
}
