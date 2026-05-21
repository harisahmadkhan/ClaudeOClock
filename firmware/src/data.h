#pragma once
#include <Arduino.h>

struct UsageData {
    // Claude Code (from Anthropic API rate-limit headers)
    float session_pct;          // 0.0 – 100.0
    int   session_reset_mins;   // -1 = unknown
    float weekly_pct;           // 0.0 – 100.0
    int   weekly_reset_mins;    // -1 = unknown
    char  status[16];           // "normal" | "limited" | "unknown"

    // claude.ai (from Chrome extension via daemon)
    float ai_session_pct;       // 0.0 – 100.0, -1.0 = no data
    int   ai_session_reset_mins;
    float ai_weekly_pct;
    int   ai_weekly_reset_mins;

    bool  ok;                   // API call succeeded
    bool  valid;                // true after first successful parse
};
