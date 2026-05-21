#pragma once
#include <stdint.h>

void    imu_hal_init();
void    imu_hal_tick();
uint8_t imu_hal_rotation_quadrant();  // 0=0°, 1=90°, 2=180°, 3=270°
