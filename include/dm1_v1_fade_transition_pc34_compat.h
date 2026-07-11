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

#define DM1_V1_FADE_STEPS_PC34       16   /* Number of fade steps (0=black, 15=full) */
#define DM1_V1_FADE_FRAME_MS_PC34    33   /* ~30fps per step = ~0.5s total fade */
#define DM1_V1_SWOOSH_STEPS_PC34     32   /* FTL swoosh palette animation steps */
#define DM1_V1_OVERLAY_MAX_TEXT_PC34 256

typedef enum {
    DM1_V1_FADE_NONE_PC34 = 0,
    DM1_V1_FADE_OUT_PC34,          /* Current screen → black */
    DM1_V1_FADE_IN_PC34,           /* Black → current screen */
    DM1_V1_FADE_TO_OVERLAY_PC34,   /* Current screen → dimmed + text overlay */
    DM1_V1_FADE_SWOOSH_PC34        /* Palette swoosh animation (FTL logo style) */
} DM1_V1_FadeModePc34;

typedef struct {
    uint16_t rgb12;  /* 12-bit Amiga color 0x0RGB */
} DM1_V1_FadeColorPc34;

typedef struct {
    DM1_V1_FadeModePc34 mode;
    int          step;          /* Current animation step */
    int          total_steps;
    DM1_V1_FadeColorPc34 saved_palette[16]; /* Palette before fade started */
    DM1_V1_FadeColorPc34 current_palette[16];
    float        dim_factor;    /* 0.0 = black, 1.0 = full brightness */

    /* Overlay text (for "Return to start menu" etc.) */
    bool         overlay_active;
    char         overlay_text[DM1_V1_OVERLAY_MAX_TEXT_PC34];
    int16_t      overlay_x;
    int16_t      overlay_y;
    uint8_t      overlay_color;

    bool         active;
} DM1_V1_FadeStatePc34;

void DM1_V1_Fade_InitPc34Compat(DM1_V1_FadeStatePc34* state);
void DM1_V1_Fade_SavePalettePc34Compat(DM1_V1_FadeStatePc34* state, const DM1_V1_FadeColorPc34 palette[16]);
void DM1_V1_Fade_StartOutPc34Compat(DM1_V1_FadeStatePc34* state);
void DM1_V1_Fade_StartInPc34Compat(DM1_V1_FadeStatePc34* state);
void DM1_V1_Fade_StartOverlayPc34Compat(DM1_V1_FadeStatePc34* state, const char* text,
                             int16_t x, int16_t y, uint8_t color);
void DM1_V1_Fade_StartSwooshPc34Compat(DM1_V1_FadeStatePc34* state);
bool DM1_V1_Fade_TickPc34Compat(DM1_V1_FadeStatePc34* state);  /* returns true while animating */
void DM1_V1_Fade_GetPalettePc34Compat(const DM1_V1_FadeStatePc34* state, DM1_V1_FadeColorPc34 out[16]);
bool DM1_V1_Fade_IsActivePc34Compat(const DM1_V1_FadeStatePc34* state);
const char* DM1_V1_Fade_GetOverlayTextPc34Compat(const DM1_V1_FadeStatePc34* state);
void DM1_V1_Fade_CancelPc34Compat(DM1_V1_FadeStatePc34* state);

#define DM1_FADE_STEPS DM1_V1_FADE_STEPS_PC34
#define DM1_FADE_FRAME_MS DM1_V1_FADE_FRAME_MS_PC34
#define DM1_SWOOSH_STEPS DM1_V1_SWOOSH_STEPS_PC34
#define DM1_OVERLAY_MAX_TEXT DM1_V1_OVERLAY_MAX_TEXT_PC34

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FADE_TRANSITION_PC34_COMPAT_H */
