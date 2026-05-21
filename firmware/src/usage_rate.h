#pragma once
#include <stdint.h>

// Tracks short-term rate of change of session usage (%/min).
// usage_zone() maps a raw percentage to a splash animation zone (0–2).

void    usage_rate_init();
void    usage_rate_sample(float session_pct);   // call once per data push
uint8_t usage_rate_group();                     // rate-based group 0–3 (internal)
float   usage_rate_pct_per_min();

// Percentage-threshold zone for splash animation selection:
//   0 = green  (  0–49%)
//   1 = amber  ( 50–79%)
//   2 = red    (80–100%)
uint8_t usage_zone(float session_pct);
