#pragma once
#include <lvgl.h>

// Font selection:
// After running generate_fonts.bat + patch_fonts.py, add -DFONTS_GENERATED
// to build_flags in platformio.ini. Until then, LVGL built-in fonts are used.
#ifdef FONTS_GENERATED
    extern const lv_font_t font_main_large;   // Inter Bold 52pt
    extern const lv_font_t font_main_medium;  // Inter Regular 28pt
    extern const lv_font_t font_main_small;   // Inter Regular 18pt
    extern const lv_font_t font_mono;         // Inter Regular 24pt

    #define FONT_LARGE   (&font_main_large)
    #define FONT_MEDIUM  (&font_main_medium)
    #define FONT_SMALL   (&font_main_small)
    #define FONT_MONO    (&font_mono)
#else
    // Placeholder — renders but in wrong size until generate_fonts.bat is run
    #define FONT_LARGE   LV_FONT_DEFAULT
    #define FONT_MEDIUM  LV_FONT_DEFAULT
    #define FONT_SMALL   LV_FONT_DEFAULT
    #define FONT_MONO    LV_FONT_DEFAULT
#endif
