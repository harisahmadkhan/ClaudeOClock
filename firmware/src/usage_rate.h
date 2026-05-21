#pragma once
#include <stdint.h>

// Tracks short-term rate of change of session usage (%/min)
// to drive animation group selection (0–3).

void    usage_rate_init();
void    usage_rate_sample(float session_pct);   // call once per data push
uint8_t usage_rate_group();                     // returns 0–3
float   usage_rate_pct_per_min();
