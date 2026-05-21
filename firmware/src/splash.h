#pragma once
#include <stdint.h>

void splash_init();
void splash_tick();                      // call every loop()
void splash_set_group(uint8_t group);    // 0–3
void splash_next_anim();                 // cycle within group
uint8_t splash_current_group();
