/* DM1 V1 Fade/Transition Overlay — source-locked from ReDMCSB
 * SWSH.C: SWSH palette animation via Setcolor() XBIOS calls
 * PALETTE.C: F1122 set palette, palette update on vblank
 * TITLE.C: F0437 DrawTitle transitions */

#include "dm1_v1_fade_transition_pc34_compat.h"
#include <string.h>

void m11_fade_init(M11_FadeState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_FadeState));
    state->dim_factor = 1.0f;
}

void m11_fade_save_palette(M11_FadeState* state, const M11_FadeColor palette[16]) {
    if (!state || !palette) return;
    memcpy(state->saved_palette, palette, sizeof(M11_FadeColor) * 16);
    memcpy(state->current_palette, palette, sizeof(M11_FadeColor) * 16);
}

/* Dim a single 12-bit color by factor (0.0=black, 1.0=full) */
static M11_FadeColor dim_color(M11_FadeColor c, float factor) {
    M11_FadeColor result;
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

static void apply_dim(M11_FadeState* state) {
    for (int i = 0; i < 16; i++) {
        state->current_palette[i] = dim_color(state->saved_palette[i],
                                               state->dim_factor);
    }
}

void m11_fade_start_out(M11_FadeState* state) {
    if (!state) return;
    state->mode = M11_FADE_OUT;
    state->step = 0;
    state->total_steps = DM1_FADE_STEPS;
    state->dim_factor = 1.0f;
    state->overlay_active = false;
    state->active = true;
}

void m11_fade_start_in(M11_FadeState* state) {
    if (!state) return;
    state->mode = M11_FADE_IN;
    state->step = 0;
    state->total_steps = DM1_FADE_STEPS;
    state->dim_factor = 0.0f;
    state->overlay_active = false;
    state->active = true;
}

void m11_fade_start_overlay(M11_FadeState* state, const char* text,
                             int16_t x, int16_t y, uint8_t color) {
    if (!state) return;
    state->mode = M11_FADE_TO_OVERLAY;
    state->step = 0;
    state->total_steps = DM1_FADE_STEPS / 2; /* Faster fade to dim */
    state->dim_factor = 1.0f;
    state->overlay_active = true;
    state->overlay_x = x;
    state->overlay_y = y;
    state->overlay_color = color;
    state->active = true;

    if (text) {
        size_t len = strlen(text);
        if (len >= DM1_OVERLAY_MAX_TEXT) len = DM1_OVERLAY_MAX_TEXT - 1;
        memcpy(state->overlay_text, text, len);
        state->overlay_text[len] = '\0';
    }
}

void m11_fade_start_swoosh(M11_FadeState* state) {
    if (!state) return;
    state->mode = M11_FADE_SWOOSH;
    state->step = 0;
    state->total_steps = DM1_SWOOSH_STEPS;
    state->dim_factor = 0.0f;
    state->overlay_active = false;
    state->active = true;

    /* Start with black palette */
    for (int i = 0; i < 16; i++) {
        state->current_palette[i].rgb12 = 0x000;
    }
}

bool m11_fade_tick(M11_FadeState* state) {
    if (!state || !state->active) return false;

    state->step++;
    if (state->step >= state->total_steps) {
        state->active = false;
        /* Snap to final state */
        switch (state->mode) {
            case M11_FADE_OUT:
                state->dim_factor = 0.0f;
                break;
            case M11_FADE_IN:
            case M11_FADE_SWOOSH:
                state->dim_factor = 1.0f;
                break;
            case M11_FADE_TO_OVERLAY:
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
        case M11_FADE_OUT:
            state->dim_factor = 1.0f - t;
            break;
        case M11_FADE_IN:
            state->dim_factor = t;
            break;
        case M11_FADE_TO_OVERLAY:
            /* Fade from 1.0 → 0.3 */
            state->dim_factor = 1.0f - (t * 0.7f);
            break;
        case M11_FADE_SWOOSH:
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

void m11_fade_get_palette(const M11_FadeState* state, M11_FadeColor out[16]) {
    if (!state || !out) return;
    memcpy(out, state->current_palette, sizeof(M11_FadeColor) * 16);
}

bool m11_fade_is_active(const M11_FadeState* state) {
    return state && state->active;
}

const char* m11_fade_get_overlay_text(const M11_FadeState* state) {
    if (!state || !state->overlay_active) return NULL;
    return state->overlay_text;
}

void m11_fade_cancel(M11_FadeState* state) {
    if (!state) return;
    state->active = false;
    state->overlay_active = false;
    state->mode = M11_FADE_NONE;
    /* Restore original palette */
    memcpy(state->current_palette, state->saved_palette,
           sizeof(M11_FadeColor) * 16);
    state->dim_factor = 1.0f;
}
