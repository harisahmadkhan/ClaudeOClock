#pragma once
#include "data.h"
#include <stdint.h>

typedef enum {
    SCREEN_SPLASH      = 0,
    SCREEN_COMBINED    = 1,
    SCREEN_CLAUDE_AI   = 2,
    SCREEN_CLAUDE_CODE = 3,
    SCREEN_BLUETOOTH   = 4,
} screen_t;

void     ui_init();
void     ui_show_screen(screen_t s);
screen_t ui_current_screen();
void     ui_update(const UsageData* data);
void     ui_tick_anim();            // call every loop() for spinner animation
void     ui_cycle_screen();         // power button: COMBINED→AI→CODE→COMBINED
