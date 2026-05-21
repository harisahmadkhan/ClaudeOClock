#pragma once
#include <stdint.h>
#include <stdbool.h>

void ble_init();
void ble_tick();

bool    ble_has_data();
uint8_t ble_get_data(uint8_t* buf, uint8_t max_len);
void    ble_send_ack(bool ok);

bool ble_is_connected();       // data connection (daemon)
bool ble_hid_is_connected();   // HID connection (OS)

// HID key codes
void ble_hid_send_key(uint8_t keycode, uint8_t modifier = 0);
