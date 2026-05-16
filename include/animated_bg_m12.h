/*
 * animated_bg_m12.h
 *
 * Background animation system for the M12 launcher startup menu.
 * Provides multiple visual presets (scrolling dungeon stone, torch
 * ambience, starfield, static colour) that render into an RGBA32
 * buffer each frame.  The modern menu renderer composites its UI
 * on top of whichever background animation is active.
 *
 * All presets are tick-driven: the caller advances the animation
 * clock and this module renders the current frame.  No threads,
 * no SDL dependency — pure C99 rendering into a caller-owned buffer.
 */

#ifndef FIRESTAFF_ANIMATED_BG_M12_H
#define FIRESTAFF_ANIMATED_BG_M12_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Preset enumeration ─────────────────────────────────────────── */

typedef enum {
    M12_BG_PRESET_STATIC = 0,       /* Plain solid colour (no animation) */
    M12_BG_PRESET_DUNGEON_CRAWL,    /* Scrolling stone/brick tiles        */
    M12_BG_PRESET_TORCH_AMBIENCE,   /* Flickering warm torchlight glow    */
    M12_BG_PRESET_STARFIELD,        /* Classic parallax starfield         */
    M12_BG_PRESET_COUNT
} M12_BgPreset;

/* ── Configuration ──────────────────────────────────────────────── */

typedef struct {
    M12_BgPreset preset;    /* Which animation to use                    */
    int   speed;            /* 1 (slow) – 5 (fast), default 3           */
    int   density;          /* Particle/element density 1-5, default 3  */
} M12_BgConfig;

/* Fill *cfg with safe defaults (STATIC preset, speed 3, density 3). */
void M12_Bg_ConfigDefaults(M12_BgConfig* cfg);

/* ── Animation state (opaque internals) ─────────────────────────── */

enum {
    M12_BG_MAX_PARTICLES  = 256,
    M12_BG_MAX_STARS      = 512,
    M12_BG_TILE_SIZE      = 64
};

typedef struct {
    /* Dungeon-crawl scrolling */
    float scrollY;

    /* Torch ambience */
    float flickerPhase;
    float flickerIntensity;

    /* Starfield */
    struct {
        float x, y, z;
    } stars[M12_BG_MAX_STARS];
    int starCount;

    /* Particle effects (shared across presets) */
    struct {
        float x, y;
        float vx, vy;
        float life;         /* 0.0 = dead, 1.0 = full */
        uint32_t color;     /* packed RGBA */
    } particles[M12_BG_MAX_PARTICLES];
    int particleCount;

    /* Internal RNG state (xorshift32) */
    uint32_t rngState;

    /* Tick counter */
    uint32_t tickCount;

    /* Cached config */
    M12_BgConfig config;

    /* Initialised flag */
    int initialised;
} M12_BgState;

/* ── API ────────────────────────────────────────────────────────── */

/* Initialise or re-initialise animation state for the given config.
 * Safe to call multiple times (resets all internal state). */
void M12_Bg_Init(M12_BgState* state, const M12_BgConfig* cfg);

/* Advance the animation by one tick.  Call at your frame rate
 * (typically 60 Hz for the launcher menu). */
void M12_Bg_Tick(M12_BgState* state);

/* Render the current animation frame into an RGBA32 buffer.
 * Buffer must be at least width * height * 4 bytes.
 * Pixel format: R, G, B, A (one byte each), matching
 * SDL_PIXELFORMAT_RGBA32. */
void M12_Bg_Render(const M12_BgState* state,
                   unsigned char* rgba,
                   int width,
                   int height);

/* Return a human-readable label for the preset (never NULL). */
const char* M12_Bg_PresetLabel(M12_BgPreset preset);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_ANIMATED_BG_M12_H */
