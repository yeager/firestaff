/*
 * colorblind_m11 — daltonization helpers for the HUD / UI.
 *
 * Implements simple linear 3x3 RGB transforms approximating common
 * daltonization filters. The intent is to lift the green/red signal
 * into a band the user can still distinguish (e.g. shifting greens
 * toward blue/orange) without recolouring the entire scene unless the
 * caller asks for it.
 *
 * No ReDMCSB equivalent — Firestaff accessibility extra. Mode 0 (Off)
 * is identity and short-circuits, so V1 launches stay bit-identical.
 */

#include "colorblind_m11.h"

#include <stddef.h>

/* Per-mode 3x3 matrix in row-major order, fixed-point x1000.
 *   r' = (m[0]*r + m[1]*g + m[2]*b) / 1000
 *   g' = (m[3]*r + m[4]*g + m[5]*b) / 1000
 *   b' = (m[6]*r + m[7]*g + m[8]*b) / 1000
 *
 * Matrices follow the daltonization tradition: simulate the
 * dichromat's perception, then shift the lost signal into a free
 * channel.  Values are intentionally conservative so the HUD stays
 * legible at normal vision too.
 */
static const int g_matrices[M11_COLORBLIND_COUNT][9] = {
    /* OFF (identity) */
    { 1000,    0,    0,
         0, 1000,    0,
         0,    0, 1000 },
    /* DEUTERANOPIA: green-weak — push green signal toward blue. */
    {  800,  200,    0,
       258,  742,    0,
         0,  142,  858 },
    /* PROTANOPIA: red-weak — borrow from green, lift red toward orange. */
    {  567,  433,    0,
       558,  442,    0,
         0,  242,  758 },
    /* TRITANOPIA: blue-weak — swap a fraction of the blue signal into
       red/green so cool tones stay distinguishable. */
    {  950,   50,    0,
         0,  433,  567,
         0,  475,  525 }
};

static const char* const g_labels[M11_COLORBLIND_COUNT] = {
    "Off",
    "Deuteranopia",
    "Protanopia",
    "Tritanopia"
};

static int clamp_byte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

const char* M11_Colorblind_GetLabel(int mode) {
    if (mode < 0 || mode >= M11_COLORBLIND_COUNT) {
        return NULL;
    }
    return g_labels[mode];
}

int M11_Colorblind_IsIdentity(int mode) {
    if (mode <= 0 || mode >= M11_COLORBLIND_COUNT) {
        return 1;
    }
    return 0;
}

void M11_Colorblind_RemapRGB(uint8_t* r, uint8_t* g, uint8_t* b, int mode) {
    int rr, gg, bb;
    const int* m;
    if (!r || !g || !b) {
        return;
    }
    if (mode <= 0 || mode >= M11_COLORBLIND_COUNT) {
        return;
    }
    m = g_matrices[mode];
    rr = (m[0] * (int)*r + m[1] * (int)*g + m[2] * (int)*b) / 1000;
    gg = (m[3] * (int)*r + m[4] * (int)*g + m[5] * (int)*b) / 1000;
    bb = (m[6] * (int)*r + m[7] * (int)*g + m[8] * (int)*b) / 1000;
    *r = (uint8_t)clamp_byte(rr);
    *g = (uint8_t)clamp_byte(gg);
    *b = (uint8_t)clamp_byte(bb);
}

void M11_Colorblind_ApplyRGBA(int mode, uint8_t* rgba, int width, int height) {
    long pixels;
    long i;
    const int* m;
    if (!rgba || width <= 0 || height <= 0) {
        return;
    }
    if (mode <= 0 || mode >= M11_COLORBLIND_COUNT) {
        return;
    }
    m = g_matrices[mode];
    pixels = (long)width * (long)height;
    for (i = 0; i < pixels; ++i) {
        uint8_t* p = rgba + (i * 4);
        int r = (int)p[0];
        int g = (int)p[1];
        int b = (int)p[2];
        int rr = (m[0] * r + m[1] * g + m[2] * b) / 1000;
        int gg = (m[3] * r + m[4] * g + m[5] * b) / 1000;
        int bb = (m[6] * r + m[7] * g + m[8] * b) / 1000;
        p[0] = (uint8_t)clamp_byte(rr);
        p[1] = (uint8_t)clamp_byte(gg);
        p[2] = (uint8_t)clamp_byte(bb);
        /* alpha (p[3]) untouched */
    }
}
