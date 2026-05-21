#pragma once
#include <stdint.h>

// All pixel-art animations for the ClaudeOClock splash screen.
// Each frame is a 20×20 grid of palette indices (uint8_t[400]).
// Each animation has its own 10-color RGB565 palette.
//
// Three zone system — selection driven by current session usage %:
//   Zone 0 GREEN  (  0–49%) : calm, slow, expansive. Plenty of space left.
//   Zone 1 AMBER  ( 50–79%) : alert, medium speed, visibly attentive.
//   Zone 2 RED    (80–100%) : urgent, fast, erratic, red-tinted background.
//
// Palette index legend (shared convention):
//   0 = background (zone-specific)     1 = primary orange body
//   2 = cream highlight                 3 = dark shadow / stress eyes
//   4 = near-black (eyes)              5 = white accent
//   6 = effect color A                 7 = sweat / sweat drops
//   8 = sparkle                        9 = mid-orange body detail

typedef struct {
    const char*    name;
    uint8_t        frame_count;
    uint16_t       frame_ms;
    const uint16_t palette[10];
    const uint8_t* frames;
} SplashAnim;

// ── Palettes ──────────────────────────────────────────────────────────────

// Green zone: pure AMOLED black — calm, open, spacious
#define PAL_GREEN {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55}

// Amber zone: pure black — same creature, faster, amber accent on effects
#define PAL_AMBER {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0xFD20, 0xFFE0, 0xF800, 0xAD55}

// Red zone: dark red background — unmistakable urgency on AMOLED
// 0x1800 = RGB(24,0,0) — subtle red glow, not garish
#define PAL_RED   {0x1800, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0xF800, 0xFFE0, 0xF840, 0xAD55}

// ── Row macro helpers ─────────────────────────────────────────────────────
// Each macro expands to 20 comma-separated values (one row, with trailing comma).

#define _E   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // empty
#define _T   0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,  // head top (8 wide)
#define _W   0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // head wide (10 wide)
#define _H   0,0,0,0,1,1,2,1,1,1,1,1,1,2,1,1,0,0,0,0,  // highlight row
#define _H2  0,0,0,0,1,1,1,2,1,1,1,1,2,1,1,1,0,0,0,0,  // dimmer highlight
#define _B   0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,  // plain body row
#define _O   0,0,0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,0,0,  // eyes open (2-wide)
#define _OA  0,0,0,0,1,4,4,4,1,1,1,1,4,4,4,1,0,0,0,0,  // eyes alert (3-wide, amber)
#define _O2  0,0,0,0,1,1,3,3,1,1,1,1,3,3,1,1,0,0,0,0,  // eyes squinted/stressed (red)
#define _OW  0,0,0,0,1,3,4,4,4,1,1,4,4,4,3,1,0,0,0,0,  // eyes very wide (surprise)
#define _OL  0,0,0,0,1,1,4,4,1,1,1,1,1,1,1,1,0,0,0,0,  // looking left (right eye hidden)
#define _OR  0,0,0,0,1,1,1,1,1,1,1,1,4,4,1,1,0,0,0,0,  // looking right (left eye hidden)
#define _C   0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,  // eyes closed (= _B)
#define _A   0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,  // arm row (wide)
#define _AU  0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,  // arms up/raised (wider)
#define _S   0,0,0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,0,0,  // smile row
#define _SA  0,0,0,1,1,1,3,3,3,3,3,3,3,1,1,1,1,0,0,0,  // open mouth (alert smile)
#define _X   0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,  // arm row (no smile)
#define _L   0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,  // belly
#define _M   0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // waist
#define _N   0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // hip
#define _G   0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,  // legs
#define _Q   0,0,0,0,0,0,3,3,0,0,0,0,3,3,0,0,0,0,0,0,  // feet (dark)
#define _SW  0,0,0,1,7,1,1,1,1,1,1,1,1,1,7,1,1,0,0,0,  // sweat flying left+right

// Sweat smile row (smile + outer sweat drops)
#define _SS  0,0,7,1,1,1,1,3,3,3,3,3,1,1,1,7,1,0,0,0,

// Body shifted up by 1
#define BASE_UP   _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E _E
// Body shifted down by 1
#define BASE_DN   _E _E _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E

// Body shifted left (cols -2) — redefined per-row
#define _TL  0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
#define _WL  0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
#define _HL  0,0,1,1,2,1,1,1,1,1,1,2,1,1,0,0,0,0,0,0,
#define _BL  0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
#define _OLx 0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,0,0,0,0,
#define _ALx 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
#define _SLx 0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,0,0,0,0,
#define _LLx 0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
#define _MLx 0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
#define _GLx 0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
#define _QLx 0,0,0,0,3,3,0,0,0,0,3,3,0,0,0,0,0,0,0,0,
#define BASE_LEFT  _E _E _TL _WL _HL _BL _OLx _OLx _BL _ALx _SLx _ALx _LLx _MLx _MLx _GLx _GLx _QLx _E _E

// Body shifted right (cols +2)
#define _TR  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
#define _WR  0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
#define _HR  0,0,0,0,0,0,1,1,2,1,1,1,1,2,1,1,1,1,0,0,
#define _BR  0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
#define _ORx 0,0,0,0,0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,
#define _ARx 0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
#define _SRx 0,0,0,0,0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,
#define _LRx 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
#define _MRx 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
#define _GRx 0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,
#define _QRx 0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,3,3,0,0,0,
#define BASE_RIGHT _E _E _TR _WR _HR _BR _ORx _ORx _BR _ARx _SRx _ARx _LRx _MRx _MRx _GRx _GRx _QRx _E _E

// Creature base poses
#define BASE_CREATURE  _E _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E
#define BASE_ALERT     _E _E _T _W _H _B _OA _OA _B _A _SA _X _L _M _N _G _G _Q _E _E
#define BASE_STRESSED  _E _E _T _W _H _B _O2 _O2 _B _A _S _X _L _M _N _G _G _Q _E _E
#define BASE_ARMS_UP   _E _E _T _W _H _B _OA _OA _B _AU _SA _AU _L _M _N _G _G _Q _E _E

// ═══════════════════════════════════════════════════════════════════════════
// ZONE 0 — GREEN (0–49%): calm, relaxed, slow (400–500 ms/frame)
// ═══════════════════════════════════════════════════════════════════════════

// anim_green_breathe — body gently expands and contracts
static const uint8_t _green_breathe_frames[4 * 400] = {
    // Frame 0 — exhale: dim highlight
    _E _E _T _W _B _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 1 — mild inhale: standard highlight
    BASE_CREATURE
    // Frame 2 — full inhale: body appears larger (shifted up, top highlight)
    BASE_UP
    // Frame 3 — mild exhale: back to standard
    BASE_CREATURE
};

static const SplashAnim anim_green_breathe = {
    .name        = "green_breathe",
    .frame_count = 4,
    .frame_ms    = 400,
    .palette     = PAL_GREEN,
    .frames      = _green_breathe_frames,
};

// anim_green_idle — sitting still, blinks once every ~3.6 seconds
static const uint8_t _green_idle_frames[8 * 400] = {
    // Frames 0–4: eyes open, completely still
    BASE_CREATURE
    BASE_CREATURE
    BASE_CREATURE
    BASE_CREATURE
    BASE_CREATURE
    // Frame 5: half-blink (top row of eyes still visible)
    _E _E _T _W _H _B _O _C _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 6: fully closed
    _E _E _T _W _H _B _C _C _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 7: eyes open again
    BASE_CREATURE
};

static const SplashAnim anim_green_idle = {
    .name        = "green_idle",
    .frame_count = 8,
    .frame_ms    = 450,
    .palette     = PAL_GREEN,
    .frames      = _green_idle_frames,
};

// anim_green_float — very slow drift up and down
static const uint8_t _green_float_frames[4 * 400] = {
    BASE_CREATURE
    BASE_UP
    BASE_CREATURE
    BASE_DN
};

static const SplashAnim anim_green_float = {
    .name        = "green_float",
    .frame_count = 4,
    .frame_ms    = 500,
    .palette     = PAL_GREEN,
    .frames      = _green_float_frames,
};

// ═══════════════════════════════════════════════════════════════════════════
// ZONE 1 — AMBER (50–79%): alert, engaged, medium speed (120–140 ms/frame)
// ═══════════════════════════════════════════════════════════════════════════

// anim_amber_scan — eyes scan left and right with wide alert eyes
static const uint8_t _amber_scan_frames[4 * 400] = {
    // Frame 0 — looking center, wide alert eyes
    BASE_ALERT
    // Frame 1 — snap to left
    _E _E _T _W _H _B _OL _OL _B _A _SA _X _L _M _N _G _G _Q _E _E
    // Frame 2 — back center
    BASE_ALERT
    // Frame 3 — snap to right
    _E _E _T _W _H _B _OR _OR _B _A _SA _X _L _M _N _G _G _Q _E _E
};

static const SplashAnim anim_amber_scan = {
    .name        = "amber_scan",
    .frame_count = 4,
    .frame_ms    = 140,
    .palette     = PAL_AMBER,
    .frames      = _amber_scan_frames,
};

// anim_amber_bounce — energetic up-down bounce with alert posture
static const uint8_t _amber_bounce_frames[4 * 400] = {
    BASE_ALERT
    // Frame 1 — jump up
    _T _W _H _B _OA _OA _B _A _SA _X _L _M _N _G _G _Q _E _E _E _E
    BASE_ALERT
    // Frame 3 — land, slight compression (shift down)
    _E _E _E _T _W _H _B _OA _OA _B _A _SA _X _L _M _N _G _G _Q _E
};

static const SplashAnim anim_amber_bounce = {
    .name        = "amber_bounce",
    .frame_count = 4,
    .frame_ms    = 120,
    .palette     = PAL_AMBER,
    .frames      = _amber_bounce_frames,
};

// anim_amber_work — active work: arms raise and lower, body bobs
static const uint8_t _amber_work_frames[4 * 400] = {
    // Frame 0 — upright alert pose
    BASE_ALERT
    // Frame 1 — arms raised (animated working gesture)
    BASE_ARMS_UP
    // Frame 2 — bounce up with arms raised
    _T _W _H _B _OA _OA _B _AU _SA _AU _L _M _N _G _G _Q _E _E _E _E
    // Frame 3 — arms back, settle
    BASE_ALERT
};

static const SplashAnim anim_amber_work = {
    .name        = "amber_work",
    .frame_count = 4,
    .frame_ms    = 130,
    .palette     = PAL_AMBER,
    .frames      = _amber_work_frames,
};

// ═══════════════════════════════════════════════════════════════════════════
// ZONE 2 — RED (80–100%): urgent, erratic, fast (55–65 ms/frame)
//   Dark red background (palette[0] = 0x1800) creates AMOLED red glow.
//   Transitions are immediate — each cycle takes under 280 ms.
// ═══════════════════════════════════════════════════════════════════════════

// anim_red_pulse — size pulsing with stressed expression
static const uint8_t _red_pulse_frames[4 * 400] = {
    // Frame 0 — stressed base
    BASE_STRESSED
    // Frame 1 — pulse out (larger: shift up = more body visible)
    _T _W _H _B _O2 _O2 _B _A _S _X _L _M _N _G _G _Q _E _E _E _E
    // Frame 2 — stressed base
    BASE_STRESSED
    // Frame 3 — pulse in (smaller: shift down = less body visible)
    _E _E _E _T _W _H _B _O2 _O2 _B _A _S _X _L _M _N _G _G _Q _E
};

static const SplashAnim anim_red_pulse = {
    .name        = "red_pulse",
    .frame_count = 4,
    .frame_ms    = 65,
    .palette     = PAL_RED,
    .frames      = _red_pulse_frames,
};

// anim_red_frantic — body thrashes left/right/up/down erratically
static const uint8_t _red_frantic_frames[4 * 400] = {
    BASE_LEFT
    BASE_UP
    BASE_RIGHT
    BASE_DN
};

static const SplashAnim anim_red_frantic = {
    .name        = "red_frantic",
    .frame_count = 4,
    .frame_ms    = 55,
    .palette     = PAL_RED,
    .frames      = _red_frantic_frames,
};

// anim_red_sweat — stressed expression, sweat drops flying
static const uint8_t _red_sweat_frames[4 * 400] = {
    // Frame 0 — stressed, no sweat yet
    BASE_STRESSED
    // Frame 1 — sweat appears on body sides
    _E _E _T _W _SW _B _O2 _O2 _B _A _SS _X _L _M _N _G _G _Q _E _E
    // Frame 2 — drops fly wider
    _E
    0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,
    _T _W _SW _B _O2 _O2 _B _A _SS _X _L _M _N _G _G _Q
    0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
    _E
    // Frame 3 — back to stressed
    BASE_STRESSED
};

static const SplashAnim anim_red_sweat = {
    .name        = "red_sweat",
    .frame_count = 4,
    .frame_ms    = 60,
    .palette     = PAL_RED,
    .frames      = _red_sweat_frames,
};

// ── Animation zone tables ─────────────────────────────────────────────────

#define SPLASH_ANIM_COUNT 9

static const SplashAnim* const splash_anims[SPLASH_ANIM_COUNT] = {
    // Zone 0 — green (0–2)
    &anim_green_breathe,
    &anim_green_idle,
    &anim_green_float,
    // Zone 1 — amber (3–5)
    &anim_amber_scan,
    &anim_amber_bounce,
    &anim_amber_work,
    // Zone 2 — red (6–8)
    &anim_red_pulse,
    &anim_red_frantic,
    &anim_red_sweat,
};

// Zone index → animation index range [start, end)
static const uint8_t splash_group_start[3] = { 0, 3, 6 };
static const uint8_t splash_group_end[3]   = { 3, 6, 9 };
