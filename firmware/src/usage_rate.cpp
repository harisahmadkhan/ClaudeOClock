#include "usage_rate.h"
#include <Arduino.h>

#define SAMPLE_COUNT  5   // last 5 samples (~5 minutes at 60s interval)

static float    _samples[SAMPLE_COUNT];
static uint32_t _times[SAMPLE_COUNT];
static int      _head = 0;
static int      _count = 0;

void usage_rate_init() {
    _head = 0;
    _count = 0;
}

void usage_rate_sample(float session_pct) {
    _samples[_head] = session_pct;
    _times[_head]   = millis();
    _head = (_head + 1) % SAMPLE_COUNT;
    if (_count < SAMPLE_COUNT) _count++;
}

float usage_rate_pct_per_min() {
    if (_count < 2) return 0.0f;

    // Oldest sample index
    int oldest = (_head - _count + SAMPLE_COUNT) % SAMPLE_COUNT;
    int newest = (_head - 1 + SAMPLE_COUNT) % SAMPLE_COUNT;

    float dpct = _samples[newest] - _samples[oldest];
    uint32_t dms = _times[newest] - _times[oldest];
    if (dms == 0) return 0.0f;

    float dmin = (float)dms / 60000.0f;
    return dpct / dmin;
}

uint8_t usage_rate_group() {
    float rate = usage_rate_pct_per_min();
    if (rate >= 2.0f)  return 3;
    if (rate >= 0.5f)  return 2;
    if (rate >= 0.1f)  return 1;
    return 0;
}

uint8_t usage_zone(float session_pct) {
    if (session_pct >= 80.0f) return 2;  // red
    if (session_pct >= 50.0f) return 1;  // amber
    return 0;                             // green
}
