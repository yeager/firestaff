/* DM1 V1 Fade/Transition Overlay — source-locked from ReDMCSB
 * SWSH.C: SWSH palette animation via Setcolor() XBIOS calls
 * PALETTE.C: F1122 set palette, palette update on vblank
 * TITLE.C: F0437 DrawTitle transitions */

#include "dm1_v1_fade_transition_pc34_compat.h"
#include <string.h>

void DM1_V1_Fade_InitPc34Compat(DM1_V1_FadeStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_FadeStatePc34));
    state->dim_factor = 1.0f;
}

void DM1_V1_Fade_SavePalettePc34Compat(DM1_V1_FadeStatePc34* state, const DM1_V1_FadeColorPc34 palette[16]) {
    if (!state || !palette) return;
    memcpy(state->saved_palette, palette, sizeof(DM1_V1_FadeColorPc34) * 16);
    memcpy(state->current_palette, palette, sizeof(DM1_V1_FadeColorPc34) * 16);
}

/* Dim a single 12-bit color by factor (0.0=black, 1.0=full) */
static DM1_V1_FadeColorPc34 dim_color(DM1_V1_FadeColorPc34 c, float factor) {
    DM1_V1_FadeColorPc34 result;
    uint8_t r = (c.rgb12 >> 8) & 0xF;
    uint8_t g = (c.rgb12 >> 4) & 0xF;
    uint8_t b = c.rgb12 & 0xF;
    r = (uint8_t)(r * factor + 0.5f);
    g = (uint8_t)(g * factor + 0.5f);
    b = (uint8_t)(b * factor + 0.5f);
    if (r > 0xF) r = 0xF;
    if (g > 0xF) g = 0xF;
    if (b > 0xF) b = 0xF;
    result.rgb12 = (uint16_t)((r << 8) | (g << 4) | b);
    return result;
}

static void apply_dim(DM1_V1_FadeStatePc34* state) {
    for (int i = 0; i < 16; i++) {
        state->current_palette[i] = dim_color(state->saved_palette[i],
                                               state->dim_factor);
    }
}

void DM1_V1_Fade_StartOutPc34Compat(DM1_V1_FadeStatePc34* state) {
    if (!state) return;
    state->mode = DM1_V1_FADE_OUT_PC34;
    state->step = 0;
    state->total_steps = DM1_V1_FADE_STEPS_PC34;
    state->dim_factor = 1.0f;
    state->overlay_active = false;
    state->active = true;
}

void DM1_V1_Fade_StartInPc34Compat(DM1_V1_FadeStatePc34* state) {
    if (!state) return;
    state->mode = DM1_V1_FADE_IN_PC34;
    state->step = 0;
    state->total_steps = DM1_V1_FADE_STEPS_PC34;
    state->dim_factor = 0.0f;
    state->overlay_active = false;
    state->active = true;
}

void DM1_V1_Fade_StartOverlayPc34Compat(DM1_V1_FadeStatePc34* state, const char* text,
                             int16_t x, int16_t y, uint8_t color) {
    if (!state) return;
    state->mode = DM1_V1_FADE_TO_OVERLAY_PC34;
    state->step = 0;
    state->total_steps = DM1_V1_FADE_STEPS_PC34 / 2; /* Faster fade to dim */
    state->dim_factor = 1.0f;
    state->overlay_active = true;
    state->overlay_x = x;
    state->overlay_y = y;
    state->overlay_color = color;
    state->active = true;

    if (text) {
        size_t len = strlen(text);
        if (len >= DM1_V1_OVERLAY_MAX_TEXT_PC34) len = DM1_V1_OVERLAY_MAX_TEXT_PC34 - 1;
        memcpy(state->overlay_text, text, len);
        state->overlay_text[len] = '\0';
    }
}

void DM1_V1_Fade_StartSwooshPc34Compat(DM1_V1_FadeStatePc34* state) {
    if (!state) return;
    state->mode = DM1_V1_FADE_SWOOSH_PC34;
    state->step = 0;
    state->total_steps = DM1_V1_SWOOSH_STEPS_PC34;
    state->dim_factor = 0.0f;
    state->overlay_active = false;
    state->active = true;

    /* Start with black palette */
    for (int i = 0; i < 16; i++) {
        state->current_palette[i].rgb12 = 0x000;
    }
}

bool DM1_V1_Fade_TickPc34Compat(DM1_V1_FadeStatePc34* state) {
    if (!state || !state->active) return false;

    state->step++;
    if (state->step >= state->total_steps) {
        state->active = false;
        /* Snap to final state */
        switch (state->mode) {
            case DM1_V1_FADE_OUT_PC34:
                state->dim_factor = 0.0f;
                break;
            case DM1_V1_FADE_IN_PC34:
            case DM1_V1_FADE_SWOOSH_PC34:
                state->dim_factor = 1.0f;
                break;
            case DM1_V1_FADE_TO_OVERLAY_PC34:
                state->dim_factor = 0.3f; /* Dim to 30% for readable overlay */
                break;
            default:
                break;
        }
        apply_dim(state);
        return false;
    }

    float t = (float)state->step / (float)state->total_steps;

    switch (state->mode) {
        case DM1_V1_FADE_OUT_PC34:
            state->dim_factor = 1.0f - t;
            break;
        case DM1_V1_FADE_IN_PC34:
            state->dim_factor = t;
            break;
        case DM1_V1_FADE_TO_OVERLAY_PC34:
            /* Fade from 1.0 → 0.3 */
            state->dim_factor = 1.0f - (t * 0.7f);
            break;
        case DM1_V1_FADE_SWOOSH_PC34:
            /* SWSH.C pattern: light up colors sequentially */
            {
                int colors_lit = (int)(t * 16.0f);
                for (int i = 0; i < 16; i++) {
                    if (i <= colors_lit) {
                        state->current_palette[i] = state->saved_palette[i];
                    } else {
                        state->current_palette[i].rgb12 = 0x000;
                    }
                }
            }
            return true;
        default:
            state->active = false;
            return false;
    }

    apply_dim(state);
    return true;
}

void DM1_V1_Fade_GetPalettePc34Compat(const DM1_V1_FadeStatePc34* state, DM1_V1_FadeColorPc34 out[16]) {
    if (!state || !out) return;
    memcpy(out, state->current_palette, sizeof(DM1_V1_FadeColorPc34) * 16);
}

bool DM1_V1_Fade_IsActivePc34Compat(const DM1_V1_FadeStatePc34* state) {
    return state && state->active;
}

const char* DM1_V1_Fade_GetOverlayTextPc34Compat(const DM1_V1_FadeStatePc34* state) {
    if (!state || !state->overlay_active) return NULL;
    return state->overlay_text;
}

void DM1_V1_Fade_CancelPc34Compat(DM1_V1_FadeStatePc34* state) {
    if (!state) return;
    state->active = false;
    state->overlay_active = false;
    state->mode = DM1_V1_FADE_NONE_PC34;
    /* Restore original palette */
    memcpy(state->current_palette, state->saved_palette,
           sizeof(DM1_V1_FadeColorPc34) * 16);
    state->dim_factor = 1.0f;
}
