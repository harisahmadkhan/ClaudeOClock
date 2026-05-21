#pragma once

// Waveshare ESP32-S3-Touch-AMOLED-2.16 pin assignments

// Display (CO5300 QSPI)
#define BOARD_DISP_CS     12
#define BOARD_DISP_SCLK   38
#define BOARD_DISP_SDIO0   4
#define BOARD_DISP_SDIO1   5
#define BOARD_DISP_SDIO2   6
#define BOARD_DISP_SDIO3   7
#define BOARD_DISP_RST     2

// I2C bus (touch + PMU + IMU)
#define BOARD_I2C_SDA     15
#define BOARD_I2C_SCL     14

// Touch (CST9220)
#define BOARD_TOUCH_INT   11
#define BOARD_TOUCH_RST    2   // shared with display reset
#define BOARD_TOUCH_ADDR  0x5A

// PMU (AXP2101)
#define BOARD_PMU_ADDR    0x34

// IMU (QMI8658)
#define BOARD_IMU_ADDR    0x6B

// Buttons
#define BOARD_BTN_LEFT     0   // BOOT button
#define BOARD_BTN_RIGHT   18

// Display geometry
#define BOARD_LCD_WIDTH   480
#define BOARD_LCD_HEIGHT  480

void board_init();
