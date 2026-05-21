#pragma once
#include <stdbool.h>

void idle_init();
void idle_tick();
void idle_reset();      // call on any user input
bool idle_is_asleep();
