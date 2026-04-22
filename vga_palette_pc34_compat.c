#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "vga_palette_pc34_compat.h"

/*
 * DM/CSB PC 3.4 VGA palette — original VGA DAC values.
 *
 * Source: VIDEODRV.C from ReDMCSB (G8149_ICON, G8151–G8156 LIGHT0–LIGHT5).
 * Verified against palette-recovery/recovered_palette.json.
 *
 * The DM PC VGA palette is NOT standard EGA. It is a custom 16-color palette
 * programmed via VGA DAC ports 0x3C8/0x3C9 by the video driver TSR.
 *
 * VGA 6-bit to 8-bit conversion: rgb8 = (vga6 << 2) | (vga6 >> 4)
 *
 *   Color  0: Black        (  0,  0,  0)  VGA6 ( 0, 0, 0)
 *   Color  1: Gray          (109,109,109)  VGA6 (27,27,27)
 *   Color  2: Light Gray    (146,146,146)  VGA6 (36,36,36)
 *   Color  3: Brown         (109, 36,  0)  VGA6 (27, 9, 0)
 *   Color  4: Cyan          (  0,219,219)  VGA6 ( 0,54,54)  — invariant across all brightness levels
 *   Color  5: Dark Brown    (146, 73,  0)  VGA6 (36,18, 0)
 *   Color  6: Dark Green    (  0,146,  0)  VGA6 ( 0,36, 0)
 *   Color  7: Green         (  0,219,  0)  VGA6 ( 0,54, 0)
 *   Color  8: Red           (255,  0,  0)  VGA6 (63, 0, 0)
 *   Color  9: Orange/Gold   (255,182,  0)  VGA6 (63,45, 0)
 *   Color 10: Tan/Skin      (219,146,109)  VGA6 (54,36,27)
 *   Color 11: Yellow        (255,255,  0)  VGA6 (63,63, 0)
 *   Color 12: Dark Gray     ( 73, 73, 73)  VGA6 (18,18,18)
 *   Color 13: Silver        (182,182,182)  VGA6 (45,45,45)
 *   Color 14: Blue          (  0,  0,255)  VGA6 ( 0, 0,63)
 *   Color 15: White         (255,255,255)  VGA6 (63,63,63)
 *
 * Brightness levels LIGHT0 (brightest) through LIGHT5 (near-black) are
 * NOT linearly attenuated — each level has independently tuned per-color
 * values from the original VIDEODRV.C source. Color 4 (Cyan) is invariant
 * across all six levels (used for water/special effects).
 */

const unsigned char G9010_auc_VgaPaletteBrightest_Compat[16][3] = {
        {  0,   0,   0},   /*  0: Black */
        {109, 109, 109},   /*  1: Gray */
        {146, 146, 146},   /*  2: Light Gray */
        {109,  36,   0},   /*  3: Brown */
        {  0, 219, 219},   /*  4: Cyan (invariant) */
        {146,  73,   0},   /*  5: Dark Brown */
        {  0, 146,   0},   /*  6: Dark Green */
        {  0, 219,   0},   /*  7: Green */
        {255,   0,   0},   /*  8: Red */
        {255, 182,   0},   /*  9: Orange/Gold */
        {219, 146, 109},   /* 10: Tan/Skin */
        {255, 255,   0},   /* 11: Yellow */
        { 73,  73,  73},   /* 12: Dark Gray */
        {182, 182, 182},   /* 13: Silver */
        {  0,   0, 255},   /* 14: Blue */
        {255, 255, 255}    /* 15: White */
};

/* All 6 brightness levels: original per-level lookup from VIDEODRV.C */
const unsigned char G9010_auc_VgaPaletteAll_Compat[6][16][3] = {
        /* LIGHT0: brightest (title/menu/max brightness viewport) */
        {
                {  0,   0,   0}, {109, 109, 109}, {146, 146, 146}, {109,  36,   0},
                {  0, 219, 219}, {146,  73,   0}, {  0, 146,   0}, {  0, 219,   0},
                {255,   0,   0}, {255, 182,   0}, {219, 146, 109}, {255, 255,   0},
                { 73,  73,  73}, {182, 182, 182}, {  0,   0, 255}, {255, 255, 255}
        },
        /* LIGHT1 — G8152 */
        {
                {  0,   0,   0}, { 73,  73,  73}, {109, 109, 109}, {109,  36,   0},
                {  0, 219, 219}, {146,  36,   0}, {  0, 109,   0}, {  0, 182,   0},
                {219,   0,   0}, {219, 146,   0}, {182, 109,  73}, {255, 219,   0},
                { 36,  36,  36}, {146, 146, 146}, {  0,   0, 219}, {219, 219, 219}
        },
        /* LIGHT2 — G8153 */
        {
                {  0,   0,   0}, { 36,  36,  36}, { 73,  73,  73}, { 73,  36,   0},
                {  0, 219, 219}, {109,  36,   0}, {  0,  73,   0}, {  0, 146,   0},
                {182,   0,   0}, {182, 109,   0}, {146,  73,  36}, {255, 182,   0},
                {  0,   0,   0}, {109, 109, 109}, {  0,   0, 182}, {182, 182, 182}
        },
        /* LIGHT3 — G8154 */
        {
                {  0,   0,   0}, {  0,   0,   0}, { 36,  36,  36}, { 36,   0,   0},
                {  0, 219, 219}, { 73,  36,   0}, {  0,  36,   0}, {  0, 109,   0},
                {146,   0,   0}, {146,  73,   0}, {109,  36,   0}, {219, 146,   0},
                {  0,   0,   0}, { 73,  73,  73}, {  0,   0, 146}, {146, 146, 146}
        },
        /* LIGHT4 — G8155 */
        {
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
                {  0, 219, 219}, { 36,   0,   0}, {  0,   0,   0}, {  0,  73,   0},
                {109,   0,   0}, {109,  36,   0}, { 73,   0,   0}, {182, 109,   0},
                {  0,   0,   0}, { 36,  36,  36}, {  0,   0, 109}, {109, 109, 109}
        },
        /* LIGHT5: near-black — G8156 (NOT all-zero: 8 colors have residual light) */
        {
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
                {  0, 219, 219}, {  0,   0,   0}, {  0,   0,   0}, {  0,  36,   0},
                { 73,   0,   0}, { 73,   0,   0}, { 36,   0,   0}, {109,  73,   0},
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,  73}, { 73,  73,  73}
        }
};

const unsigned char* F9010_VGA_GetColorRgb_Compat(
unsigned char colorIndex SEPARATOR
unsigned int  paletteLevel FINAL_SEPARATOR
{
        if (colorIndex >= 16U) {
                return 0;
        }
        if (paletteLevel >= 6U) {
                return 0;
        }
        return G9010_auc_VgaPaletteAll_Compat[paletteLevel][colorIndex];
}
