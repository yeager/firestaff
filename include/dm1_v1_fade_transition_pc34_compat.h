/* DM1 V1 Fade/Transition Overlay — source-locked from ReDMCSB
 * SWSH.C: FTL logo palette animation (swoosh)
 * PALETTE.C: F1122 palette set, vblank-synced palette changes
 * TITLE.C: F0437 DrawTitle transition to gameplay
 *
 * Provides screen fade-out/fade-in, text overlay on dimmed background,
 * and swoosh-style palette animation for title/menu transitions. */
#ifndef FIRESTAFF_DM1_V1_FADE_TRANSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FADE_TRANSITION_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_FADE_STEPS       16   /* Number of fade steps (0=black, 15=full) */
#define DM1_FADE_FRAME_MS    33   /* ~30fps per step = ~0.5s total fade */
#define DM1_SWOOSH_STEPS     32   /* FTL swoosh palette animation steps */
#define DM1_OVERLAY_MAX_TEXT 256

typedef enum {
    M11_FADE_NONE = 0,
    M11_FADE_OUT,          /* Current screen → black */
    M11_FADE_IN,           /* Black → current screen */
    M11_FADE_TO_OVERLAY,   /* Current screen → dimmed + text overlay */
    M11_FADE_SWOOSH        /* Palette swoosh animation (FTL logo style) */
} M11_FadeMode;

typedef struct {
    uint16_t rgb12;  /* 12-bit Amiga color 0x0RGB */
} M11_FadeColor;

typedef struct {
    M11_FadeMode mode;
    int          step;          /* Current animation step */
    int          total_steps;
    M11_FadeColor saved_palette[16]; /* Palette before fade started */
    M11_FadeColor current_palette[16];
    float        dim_factor;    /* 0.0 = black, 1.0 = full brightness */

    /* Overlay text (for "Return to start menu" etc.) */
    bool         overlay_active;
    char         overlay_text[DM1_OVERLAY_MAX_TEXT];
    int16_t      overlay_x;
    int16_t      overlay_y;
    uint8_t      overlay_color;

    bool         active;
} M11_FadeState;

void m11_fade_init(M11_FadeState* state);
void m11_fade_save_palette(M11_FadeState* state, const M11_FadeColor palette[16]);
void m11_fade_start_out(M11_FadeState* state);
void m11_fade_start_in(M11_FadeState* state);
void m11_fade_start_overlay(M11_FadeState* state, const char* text,
                             int16_t x, int16_t y, uint8_t color);
void m11_fade_start_swoosh(M11_FadeState* state);
bool m11_fade_tick(M11_FadeState* state);  /* returns true while animating */
void m11_fade_get_palette(const M11_FadeState* state, M11_FadeColor out[16]);
bool m11_fade_is_active(const M11_FadeState* state);
const char* m11_fade_get_overlay_text(const M11_FadeState* state);
void m11_fade_cancel(M11_FadeState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FADE_TRANSITION_PC34_COMPAT_H */
