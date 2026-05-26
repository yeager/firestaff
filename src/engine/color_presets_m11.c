/*
 * color_presets_m11.c — color grading preset LUTs.
 *
 * Each preset is realised as three 256-entry per-channel LUTs that are
 * generated lazily on first use.  This keeps the binary slim (we don't
 * ship 7 * 768-byte tables in .rodata) and lets us reuse the same
 * generation routine for both straight per-channel curves and the
 * sepia cross-channel mix.
 *
 * Preset 0 (Original) is the identity and the apply routine
 * short-circuits before doing any work, so V1 launches that leave
 * dm1V2ColorPreset == 0 see zero overhead.
 *
 * Source: Firestaff V2.0 visual extras — no ReDMCSB equivalent.
 */

#include "color_presets_m11.h"

#include <stddef.h>
#include <string.h>

/* Per-channel straight curves: (gain, bias) clamped to [0,255]. */
typedef struct {
    float gainR, biasR;
    float gainG, biasG;
    float gainB, biasB;
} M11_ColorPresetCurve;

/* Hand-tuned curves for the six non-identity presets.  Values are
 * conservative enough that text and UI remain readable. */
static const M11_ColorPresetCurve g_preset_curves[M11_COLOR_PRESET_COUNT] = {
    /* 0 ORIGINAL              — identity (unused, short-circuited) */
    { 1.000f,  0.0f, 1.000f,  0.0f, 1.000f,  0.0f },
    /* 1 ATARI ST WARM         — warm tint, slight green/blue lift */
    { 1.060f,  8.0f, 1.020f,  2.0f, 0.870f, -6.0f },
    /* 2 AMIGA GOLD            — golden/amber wash */
    { 1.080f, 10.0f, 1.000f,  4.0f, 0.780f,-10.0f },
    /* 3 VGA CLEAN             — neutral, slight contrast lift */
    { 1.040f, -4.0f, 1.040f, -4.0f, 1.040f, -4.0f },
    /* 4 SEPIA                 — handled specially below */
    { 1.000f,  0.0f, 1.000f,  0.0f, 1.000f,  0.0f },
    /* 5 HIGH CONTRAST         — gain >1, big negative bias */
    { 1.400f,-40.0f, 1.400f,-40.0f, 1.400f,-40.0f },
    /* 6 COOL BLUE             — cool tint, warm channels reduced */
    { 0.870f, -6.0f, 0.970f, -2.0f, 1.080f,  8.0f }
};

static const char* g_preset_labels[M11_COLOR_PRESET_COUNT] = {
    "Original",
    "Atari ST Warm",
    "Amiga Gold",
    "VGA Clean",
    "Sepia",
    "High Contrast",
    "Cool Blue"
};

/* Generated lazily on first apply call.  One byte per channel per
 * preset slot, plus a flag indicating whether the slot is built. */
static unsigned char g_preset_lut[M11_COLOR_PRESET_COUNT][3][256];
static int g_preset_lut_built[M11_COLOR_PRESET_COUNT] = {0};

static unsigned char m11_clamp_byte_f(float v) {
    if (v < 0.0f) return 0;
    if (v > 255.0f) return 255;
    return (unsigned char)(v + 0.5f);
}

static void m11_build_straight_curve(int preset) {
    const M11_ColorPresetCurve* c = &g_preset_curves[preset];
    int i;
    for (i = 0; i < 256; ++i) {
        float fi = (float)i;
        g_preset_lut[preset][0][i] = m11_clamp_byte_f(c->gainR * fi + c->biasR);
        g_preset_lut[preset][1][i] = m11_clamp_byte_f(c->gainG * fi + c->biasG);
        g_preset_lut[preset][2][i] = m11_clamp_byte_f(c->gainB * fi + c->biasB);
    }
}

static void m11_build_sepia_lut(void) {
    /* Sepia ends up cross-channel — we still bake an approximation by
     * building a luminance ramp per channel using fixed sepia
     * multipliers.  The full cross-channel mix happens in the apply
     * fast path; the LUTs here just produce the per-channel weighting
     * for the dominant component to keep cache-friendly. */
    int i;
    for (i = 0; i < 256; ++i) {
        float fi = (float)i;
        /* Sepia weights from common standard (R 0.393 0.769 0.189) etc.
         * Here we precompute three monotone ramps used in the apply
         * path's vectorised inner loop. */
        g_preset_lut[M11_COLOR_PRESET_SEPIA][0][i] = m11_clamp_byte_f(fi * 0.393f);
        g_preset_lut[M11_COLOR_PRESET_SEPIA][1][i] = m11_clamp_byte_f(fi * 0.769f);
        g_preset_lut[M11_COLOR_PRESET_SEPIA][2][i] = m11_clamp_byte_f(fi * 0.189f);
    }
}

static void m11_ensure_preset_built(int preset) {
    if (preset <= 0 || preset >= M11_COLOR_PRESET_COUNT) {
        return;
    }
    if (g_preset_lut_built[preset]) {
        return;
    }
    if (preset == M11_COLOR_PRESET_SEPIA) {
        m11_build_sepia_lut();
    } else {
        m11_build_straight_curve(preset);
    }
    g_preset_lut_built[preset] = 1;
}

const char* M11_ColorPreset_GetLabel(int preset) {
    if (preset < 0 || preset >= M11_COLOR_PRESET_COUNT) {
        return NULL;
    }
    return g_preset_labels[preset];
}

int M11_ColorPreset_IsValid(int preset) {
    return (preset >= 0 && preset < M11_COLOR_PRESET_COUNT) ? 1 : 0;
}

int M11_ColorPreset_IsIdentity(int preset) {
    return (preset == M11_COLOR_PRESET_ORIGINAL) ? 1 : 0;
}

void M11_ColorPreset_ApplyRGBA(int preset, unsigned char* rgba, int width, int height) {
    int pixelCount;
    int i;
    if (!rgba || width <= 0 || height <= 0) {
        return;
    }
    if (!M11_ColorPreset_IsValid(preset) || preset == M11_COLOR_PRESET_ORIGINAL) {
        return;
    }
    m11_ensure_preset_built(preset);
    pixelCount = width * height;

    if (preset == M11_COLOR_PRESET_SEPIA) {
        /* Cross-channel sepia mix using standard matrix.  We use the
         * pre-built scaled ramps as an approximation of the row
         * contributions: each output channel is the sum of three
         * scaled source channels, clamped to byte. */
        for (i = 0; i < pixelCount; ++i) {
            unsigned char r = rgba[i * 4 + 0];
            unsigned char g = rgba[i * 4 + 1];
            unsigned char b = rgba[i * 4 + 2];
            int outR = g_preset_lut[M11_COLOR_PRESET_SEPIA][0][r]
                     + g_preset_lut[M11_COLOR_PRESET_SEPIA][1][g]
                     + g_preset_lut[M11_COLOR_PRESET_SEPIA][2][b];
            int outG = (int)(r * 0.349f + g * 0.686f + b * 0.168f);
            int outB = (int)(r * 0.272f + g * 0.534f + b * 0.131f);
            if (outR > 255) outR = 255;
            if (outG > 255) outG = 255;
            if (outB > 255) outB = 255;
            rgba[i * 4 + 0] = (unsigned char)outR;
            rgba[i * 4 + 1] = (unsigned char)outG;
            rgba[i * 4 + 2] = (unsigned char)outB;
            /* alpha left untouched */
        }
        return;
    }

    /* Straight per-channel LUT. */
    for (i = 0; i < pixelCount; ++i) {
        unsigned char r = rgba[i * 4 + 0];
        unsigned char g = rgba[i * 4 + 1];
        unsigned char b = rgba[i * 4 + 2];
        rgba[i * 4 + 0] = g_preset_lut[preset][0][r];
        rgba[i * 4 + 1] = g_preset_lut[preset][1][g];
        rgba[i * 4 + 2] = g_preset_lut[preset][2][b];
        /* alpha left untouched */
    }
}
