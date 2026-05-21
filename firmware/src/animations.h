#pragma once
#include <stdint.h>

// All pixel-art animations for the ClaudeOClock splash screen.
// Each frame is a 20×20 grid of palette indices (uint8_t[400]).
// Each animation has its own 10-color RGB565 palette.
//
// Creature design: Claude-orange round body, two eyes, small arms/legs.
// Occupies roughly central 12×16 cells of the 20×20 grid.
//
// Palette index legend (shared convention across animations):
//   0 = background (black)       1 = primary orange body
//   2 = cream highlight           3 = dark shadow/outline
//   4 = near-black (eyes)         5 = white accent
//   6 = special effect color A    7 = special effect color B
//   8 = special effect color C    9 = mid-orange body detail

typedef struct {
    const char*    name;
    uint8_t        frame_count;
    uint16_t       frame_ms;
    const uint16_t palette[10];
    const uint8_t* frames;
} SplashAnim;

// ── Row macro helpers ────────────────────────────────────────────────────
// Each macro expands to 20 comma-separated uint8_t values (one row).

#define _E  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // empty row
#define _T  0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,  // head top (8 wide)
#define _W  0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // head wide (10 wide)
#define _H  0,0,0,0,1,1,2,1,1,1,1,1,1,2,1,1,0,0,0,0,  // highlight row
#define _B  0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,  // body row
#define _O  0,0,0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,0,0,  // eyes open
#define _C  0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,  // eyes closed row (= _B)
#define _F  0,0,0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,0,0,  // eyes (same as _O)
#define _A  0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,  // arm row (wide)
#define _S  0,0,0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,0,0,  // smile row
#define _X  0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,  // arm row (no smile)
#define _L  0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,  // belly
#define _M  0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // waist
#define _N  0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // hip
#define _G  0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,  // legs
#define _Q  0,0,0,0,0,0,3,3,0,0,0,0,3,3,0,0,0,0,0,0,  // feet

// Base creature (neutral idle pose, 20 rows):
#define BASE_CREATURE  _E _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E

// Variant rows
#define _O2  0,0,0,0,1,1,3,3,1,1,1,1,3,3,1,1,0,0,0,0,  // eyes squinted (stress)
#define _OW  0,0,0,0,1,3,4,4,4,1,1,4,4,4,3,1,0,0,0,0,  // eyes wide open (surprised)
#define _OL  0,0,0,0,1,1,4,4,1,1,1,1,1,1,1,1,0,0,0,0,  // left eye only visible
#define _OR  0,0,0,0,1,1,1,1,1,1,1,1,4,4,1,1,0,0,0,0,  // right eye only visible
#define _SR  0,0,0,1,1,1,1,1,3,3,3,1,1,1,1,1,1,0,0,0,  // smile right-leaning
#define _SL  0,0,0,1,1,1,1,3,3,3,1,1,1,1,1,1,1,0,0,0,  // smile left-leaning
#define _FW  0,0,0,1,3,1,1,1,1,1,1,1,1,1,1,3,1,0,0,0,  // frown / wide mouth
#define _AU  0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,  // arms up (wider)
#define _AD  0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,  // arms down (= waist)
#define _H2  0,0,0,0,1,1,1,2,1,1,1,1,2,1,1,1,0,0,0,0,  // highlight v2
#define _Z1  0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,  // z1 bottom
#define _Z2  0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,  // z2 mid
#define _Z3  0,0,0,0,0,6,6,6,0,0,0,0,0,0,0,0,0,0,0,0,  // z3 top
#define _SW  0,0,0,1,7,1,1,1,1,1,1,1,1,1,7,1,1,0,0,0,  // sweat left+right
#define _S2  0,7,0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,7,0,  // smile + sweat drops
#define _SP  0,0,0,1,1,1,1,8,8,8,8,8,1,1,1,1,1,0,0,0,  // sparkle row
#define _BT  0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,2,0,0,0,0,  // body with top cream tinge
#define _GL  0,0,0,0,0,0,1,1,3,3,0,0,0,0,0,0,0,0,0,0,  // left leg only
#define _GR  0,0,0,0,0,0,0,0,0,0,3,3,1,1,0,0,0,0,0,0,  // right leg

// Body shifted up by 1 (bounce up) — rows shifted, extra empty at bottom
#define BASE_UP  _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E _E
// Body shifted down by 1 (bounce down)
#define BASE_DN  _E _E _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E
// Body shifted left by 2 cols — done per-row
#define _TL  0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
#define _WL  0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
#define _HL  0,0,1,1,2,1,1,1,1,1,1,2,1,1,0,0,0,0,0,0,
#define _BL  0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
#define _OLs 0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,0,0,0,0,
#define _ALs 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
#define _SLs 0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,0,0,0,0,
#define _LLs 0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
#define _MLs 0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
#define _GLs 0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
#define _QLs 0,0,0,0,3,3,0,0,0,0,3,3,0,0,0,0,0,0,0,0,
#define BASE_LEFT  _E _E _TL _WL _HL _BL _OLs _OLs _BL _ALs _SLs _ALs _LLs _MLs _MLs _GLs _GLs _QLs _E _E
// Body shifted right by 2 cols
#define _TR  0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
#define _WR  0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
#define _HR  0,0,0,0,0,0,1,1,2,1,1,1,1,2,1,1,1,1,0,0,
#define _BR  0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
#define _ORs 0,0,0,0,0,0,1,1,4,4,1,1,1,1,4,4,1,1,0,0,
#define _ARs 0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
#define _SRs 0,0,0,0,0,1,1,1,1,3,3,3,3,3,1,1,1,1,1,0,
#define _LRs 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
#define _MRs 0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
#define _GRs 0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,
#define _QRs 0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,3,3,0,0,0,
#define BASE_RIGHT  _E _E _TR _WR _HR _BR _ORs _ORs _BR _ARs _SRs _ARs _LRs _MRs _MRs _GRs _GRs _QRs _E _E

// ── anim_idle_breathe ────────────────────────────────────────────────────
// Creature pulses gently. Highlight brightens then dims.

static const uint8_t _breathe_frames[4 * 400] = {
    // Frame 0 — exhale (dim)
    BASE_CREATURE
    // Frame 1 — slight inhale
    _E _E _T _W _H2 _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 2 — full inhale (extra highlight at top)
    _E _T _W _H _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E _E
    // Frame 3 — back to slight
    _E _E _T _W _H2 _B _O _O _B _A _S _X _L _M _N _G _G _Q _E _E
};

static const SplashAnim anim_idle_breathe = {
    .name        = "idle_breathe",
    .frame_count = 4,
    .frame_ms    = 300,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _breathe_frames,
};

// ── anim_idle_blink ──────────────────────────────────────────────────────

static const uint8_t _blink_frames[6 * 400] = {
    // Frame 0 — eyes open
    BASE_CREATURE
    // Frame 1 — eyes open
    BASE_CREATURE
    // Frame 2 — eyes open
    BASE_CREATURE
    // Frame 3 — eyes half closed (only top row of eyes visible)
    _E _E _T _W _H _B _O _C _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 4 — eyes closed
    _E _E _T _W _H _B _C _C _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 5 — eyes open again
    BASE_CREATURE
};

static const SplashAnim anim_idle_blink = {
    .name        = "idle_blink",
    .frame_count = 6,
    .frame_ms    = 300,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _blink_frames,
};

// ── anim_idle_sleep ──────────────────────────────────────────────────────
// Eyes closed, zzz bubbles float upward.

static const uint8_t _sleep_frames[4 * 400] = {
    // Frame 0 — eyes closed, z appears low
    _E _E _T _W _H _B _C _C _B _A _S _X _L _M _N _G _G _Q _Z1 _E
    // Frame 1 — z moves up
    _E _E _T _W _H _B _C _C _B _A _S _X _L _M _N _G _G _Q _Z2 _Z1
    // Frame 2 — z higher, new z appears
    _E _E _T _W _H _B _C _C _B _A _S _X _L _M _N _G _G _Z3 _Z2 _Z1
    // Frame 3 — zs drift further
    _E _E _T _W _H _B _C _C _B _A _S _X _L _M _Z3 _Z2 _Z1 _E _E _E
};

static const SplashAnim anim_idle_sleep = {
    .name        = "idle_sleep",
    .frame_count = 4,
    .frame_ms    = 400,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _sleep_frames,
};

// ── anim_work_think ──────────────────────────────────────────────────────
// Creature with thought bubble, pondering expression.

static const uint8_t _think_frames[4 * 400] = {
    // Frame 0 — eyes look up-right
    _E _E _T _W _H _B _OL _OL _B _A _SR _X _L _M _N _G _G _Q _E _E
    // Frame 1 — thought bubble dot appears (index 5=white)
    _E _E _T _W _H _B _OL _OL _B _A _SR _X _L _M _N _G _G _Q
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,
    // Frame 2 — bigger thought bubble
    _E
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,
    _T _W _H _B _OL _OL _B _A _SR _X _L _M _N _G _G _Q
    0,0,0,0,0,0,0,0,0,0,0,0,0,5,5,5,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,5,0,0,0,0,
    // Frame 3 — back to frame 0
    _E _E _T _W _H _B _OL _OL _B _A _SR _X _L _M _N _G _G _Q _E _E
};

static const SplashAnim anim_work_think = {
    .name        = "work_think",
    .frame_count = 4,
    .frame_ms    = 150,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _think_frames,
};

// ── anim_work_type ───────────────────────────────────────────────────────
// Creature bobbing while typing.

static const uint8_t _type_frames[4 * 400] = {
    // Frame 0 — normal
    BASE_CREATURE
    // Frame 1 — slightly up
    BASE_UP
    // Frame 2 — normal
    BASE_CREATURE
    // Frame 3 — slightly down
    BASE_DN
};

static const SplashAnim anim_work_type = {
    .name        = "work_type",
    .frame_count = 4,
    .frame_ms    = 150,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _type_frames,
};

// ── anim_look_around ─────────────────────────────────────────────────────
// Eyes dart left, center, right, center.

static const uint8_t _look_frames[4 * 400] = {
    // Frame 0 — eyes center
    BASE_CREATURE
    // Frame 1 — eyes left
    _E _E _T _W _H _B _OL _OL _B _A _SL _X _L _M _N _G _G _Q _E _E
    // Frame 2 — eyes center
    BASE_CREATURE
    // Frame 3 — eyes right
    _E _E _T _W _H _B _OR _OR _B _A _SR _X _L _M _N _G _G _Q _E _E
};

static const SplashAnim anim_look_around = {
    .name        = "look_around",
    .frame_count = 4,
    .frame_ms    = 150,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _look_frames,
};

// ── anim_dance_sway ──────────────────────────────────────────────────────
// Body sways left–center–right–center.

static const uint8_t _sway_frames[4 * 400] = {
    BASE_LEFT
    BASE_CREATURE
    BASE_RIGHT
    BASE_CREATURE
};

static const SplashAnim anim_dance_sway = {
    .name        = "dance_sway",
    .frame_count = 4,
    .frame_ms    = 100,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _sway_frames,
};

// ── anim_dance_bounce ────────────────────────────────────────────────────
// Body bounces up and down.

static const uint8_t _bounce_frames[4 * 400] = {
    BASE_CREATURE
    BASE_UP
    BASE_CREATURE
    BASE_DN
};

static const SplashAnim anim_dance_bounce = {
    .name        = "dance_bounce",
    .frame_count = 4,
    .frame_ms    = 100,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _bounce_frames,
};

// ── anim_surprise ────────────────────────────────────────────────────────
// Startled — eyes wide, small jump.

static const uint8_t _surprise_frames[4 * 400] = {
    // Frame 0 — normal
    BASE_CREATURE
    // Frame 1 — eyes wide, arms raised
    _E _E _T _W _H _B _OW _OW _B _AU _S _AU _L _M _N _G _G _Q _E _E
    // Frame 2 — jump up
    _T _W _H _B _OW _OW _B _AU _S _AU _L _M _N _G _G _Q _E _E _E _E
    // Frame 3 — back to normal
    BASE_CREATURE
};

static const SplashAnim anim_surprise = {
    .name        = "surprise",
    .frame_count = 4,
    .frame_ms    = 100,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _surprise_frames,
};

// ── anim_panic_spin ──────────────────────────────────────────────────────
// Creature "spinning" — 4 rotation phases.

static const uint8_t _spin_frames[4 * 400] = {
    // Frame 0 — normal
    BASE_CREATURE
    // Frame 1 — leaning right (body right, head left)
    _E _E _TR _WR _HR _BR _ORs _ORs _BR _ARs _SRs _ARs _LRs _MRs _MRs _GRs _GRs _QRs _E _E
    // Frame 2 — upside-ish (body shifted with wide eyes, squint)
    _E _E _T _W _H _B _O2 _O2 _B _AU _FW _AU _L _M _N _G _G _Q _E _E
    // Frame 3 — leaning left
    BASE_LEFT
};

static const SplashAnim anim_panic_spin = {
    .name        = "panic_spin",
    .frame_count = 4,
    .frame_ms    = 60,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _spin_frames,
};

// ── anim_panic_sweat ─────────────────────────────────────────────────────
// Stressed creature with sweat drops (index 7 = yellow).

static const uint8_t _sweat_frames[4 * 400] = {
    // Frame 0 — stressed eyes
    _E _E _T _W _H _B _O2 _O2 _B _A _S _X _L _M _N _G _G _Q _E _E
    // Frame 1 — sweat drops appear
    _E _E _T _W _SW _B _O2 _O2 _B _A _S2 _X _L _M _N _G _G _Q _E _E
    // Frame 2 — more sweat
    _E
    0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,
    _T _W _SW _B _O2 _O2 _B _A _S2 _X _L _M _N _G _G _Q
    0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
    _E
    // Frame 3 — drops dissipate
    _E _E _T _W _H _B _O2 _O2 _B _A _S _X _L _M _N _G _G _Q _E _E
};

static const SplashAnim anim_panic_sweat = {
    .name        = "panic_sweat",
    .frame_count = 4,
    .frame_ms    = 60,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _sweat_frames,
};

// ── anim_dance_dj ────────────────────────────────────────────────────────
// Manic DJ mode — arms flailing, sparkles.

static const uint8_t _dj_frames[4 * 400] = {
    // Frame 0 — arms up, sparkles
    _SP _E _T _W _H _B _OW _OW _B _AU _S _AU _L _M _N _G _G _Q _E _E
    // Frame 1 — bounce up, arms different
    _E _SP _W _H _B _OW _OW _B _AU _SP _AU _L _M _N _G _G _Q _E _E _E
    // Frame 2 — arms wide, more sparkles
    _SP _E _T _W _H _B _OW _OW _B _AU _S _AU _L _M _N _G _G _Q _SP _E
    // Frame 3 — bounce down
    _E _E _T _W _H _B _OW _OW _B _AU _S _AU _L _SP _N _G _G _Q _E _E
};

static const SplashAnim anim_dance_dj = {
    .name        = "dance_dj",
    .frame_count = 4,
    .frame_ms    = 60,
    .palette     = {0x0000, 0xDB6B, 0xFD3E, 0x4900, 0x0800, 0xFFFF, 0x07FF, 0xFFE0, 0xF800, 0xAD55},
    .frames      = _dj_frames,
};

// ── Animation groups ─────────────────────────────────────────────────────

#define SPLASH_ANIM_COUNT 12

static const SplashAnim* const splash_anims[SPLASH_ANIM_COUNT] = {
    // Group 0 — idle (0–2)
    &anim_idle_breathe,
    &anim_idle_blink,
    &anim_idle_sleep,
    // Group 1 — normal (3–5)
    &anim_work_think,
    &anim_work_type,
    &anim_look_around,
    // Group 2 — active (6–8)
    &anim_dance_sway,
    &anim_dance_bounce,
    &anim_surprise,
    // Group 3 — heavy/panic (9–11)
    &anim_panic_spin,
    &anim_panic_sweat,
    &anim_dance_dj,
};

// Group index → animation index range [start, end)
static const uint8_t splash_group_start[4] = { 0, 3, 6, 9 };
static const uint8_t splash_group_end[4]   = { 3, 6, 9, 12 };
