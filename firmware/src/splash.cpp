#include "splash.h"
#include "animations.h"
#include "hal/display_hal.h"
#include <Arduino.h>

// Grid constants
#define GRID       20
#define CELL_SIZE  24   // 480 / 20 = 24 px per cell

// Status text rotated from this word list
static const char* const _words[] = {
    "Accomplishing","Actioning","Actualizing","Baking","Booping","Brewing",
    "Calculating","Cerebrating","Channelling","Churning","Cogitating","Coalescing",
    "Computing","Concocting","Conjuring","Considering","Contemplating","Cooking",
    "Crafting","Creating","Crunching","Deciphering","Deliberating","Determining",
    "Divining","Doing","Effecting","Elucidating","Enchanting","Envisioning",
    "Finagling","Forging","Forming","Frolicking","Generating","Germinating",
    "Hatching","Herding","Hustling","Ideating","Imagining","Inferring","Manifesting",
    "Marinating","Meandering","Moseying","Mulling","Mustering","Musing","Noodling",
    "Percolating","Perusing","Philosophising","Pondering","Pontificating",
    "Processing","Puzzling","Reticulating","Ruminating","Scheming","Schlepping",
    "Shimmying","Simmering","Smooshing","Spelunking","Spinning","Stewing","Sussing",
    "Synthesizing","Thinking","Tinkering","Transmuting","Unfurling","Vibing",
    "Wandering","Whirring","Wizarding","Working","Wrangling",
};
static const int _word_count = sizeof(_words) / sizeof(_words[0]);

static uint8_t  _group        = 0;
static uint8_t  _anim_idx     = 0;      // index into splash_anims[]
static uint8_t  _frame        = 0;
static uint32_t _last_frame   = 0;
static uint32_t _last_word    = 0;
static int      _word_idx     = 0;

// Render a single frame of the current animation to the display
static void _render_frame(const SplashAnim* anim, uint8_t frame_no) {
    const uint8_t* pixels = anim->frames + (uint32_t)frame_no * 400;

    // We draw cell by cell. Each cell is CELL_SIZE × CELL_SIZE pixels,
    // filled with the palette color for that index.
    // Uses a temporary 24×24 buffer per cell (1152 bytes — fits on stack).
    uint16_t cell_buf[CELL_SIZE * CELL_SIZE];

    for (int row = 0; row < GRID; row++) {
        for (int col = 0; col < GRID; col++) {
            uint8_t idx = pixels[row * GRID + col];
            uint16_t color = anim->palette[idx & 0x0F];

            // Fill cell buffer
            for (int p = 0; p < CELL_SIZE * CELL_SIZE; p++) {
                cell_buf[p] = color;
            }
            display_hal_draw_bitmap(
                col * CELL_SIZE, row * CELL_SIZE,
                CELL_SIZE, CELL_SIZE,
                cell_buf
            );
        }
    }
}

void splash_init() {
    _group      = 0;
    _anim_idx   = splash_group_start[0];
    _frame      = 0;
    _last_frame = millis();
    _last_word  = millis();
    _word_idx   = random(_word_count);
}

void splash_tick() {
    const SplashAnim* anim = splash_anims[_anim_idx];
    uint32_t now = millis();

    // Advance animation frame
    if (now - _last_frame >= anim->frame_ms) {
        _last_frame = now;
        _frame = (_frame + 1) % anim->frame_count;
        _render_frame(anim, _frame);
    }

    // Rotate status word every 4 seconds
    if (now - _last_word >= 4000) {
        _last_word = now;
        _word_idx = (_word_idx + 1) % _word_count;
    }
}

void splash_set_group(uint8_t group) {
    if (group > 2) group = 2;
    if (group == _group) return;
    _group    = group;
    _anim_idx = splash_group_start[group];
    _frame    = 0;
    _last_frame = millis();
}

void splash_next_anim() {
    _anim_idx++;
    if (_anim_idx >= splash_group_end[_group]) {
        _anim_idx = splash_group_start[_group];
    }
    _frame = 0;
    _last_frame = millis();
}

uint8_t splash_current_group() {
    return _group;
}

const char* splash_current_word() {
    return _words[_word_idx];
}
