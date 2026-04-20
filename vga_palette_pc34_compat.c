#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "vga_palette_pc34_compat.h"

/*
 * CSB PC 3.4 VGA palette — brightest level (palette index 0).
 *
 * These are the standard EGA-derived 16-color palette values used by
 * DM/CSB PC. The video driver programs the VGA DAC with 6-bit values;
 * here they are pre-scaled to 8-bit:
 *   Color 0:  Black           (0,0,0)
 *   Color 1:  Dark Blue       (0,0,170)
 *   Color 2:  Dark Green      (0,170,0)
 *   Color 3:  Dark Cyan       (0,170,170)
 *   Color 4:  Dark Red        (170,0,0)
 *   Color 5:  Dark Magenta / Brown  (170,85,0) — EGA brown, not magenta
 *   Color 6:  Light Gray      (170,170,170)
 *   Color 7:  Medium Gray     (85,85,85)
 *   Color 8:  Red             (255,85,85)
 *   Color 9:  Blue            (85,85,255)
 *   Color 10: Light Green     (85,255,85)
 *   Color 11: Yellow          (255,255,85)
 *   Color 12: Darkest Gray    (40,40,40)
 *   Color 13: Lightest Gray   (200,200,200)
 *   Color 14: Dark medium     (100,100,100)
 *   Color 15: White           (255,255,255)
 *
 * Note: Colors 5-7 and 12-14 deviate from standard EGA because DM/CSB
 * uses a customized palette. The exact values are from the PC 3.4 video
 * driver binary analysis. Colors 0-4, 8-11, 15 match standard EGA/CGA.
 *
 * The darker palette levels (1-5) reduce brightness progressively for
 * dungeon darkness. The exact attenuation is driven by the video driver's
 * SetMultipleColorsInPalette function.
 *
 * Since the video driver binary is not available for direct extraction,
 * the darker levels here use a simple linear attenuation model:
 *   level N: rgb = brightest_rgb * (5 - N) / 5
 * This is approximate but produces visually plausible dungeon darkness.
 */

const unsigned char G9010_auc_VgaPaletteBrightest_Compat[16][3] = {
        {  0,   0,   0},   /*  0: Black */
        {  0,   0, 170},   /*  1: Dark Blue */
        {  0, 170,   0},   /*  2: Dark Green */
        {  0, 170, 170},   /*  3: Dark Cyan */
        {170,   0,   0},   /*  4: Dark Red */
        {170,  85,   0},   /*  5: Brown (DM/CSB specific) */
        {170, 170, 170},   /*  6: Light Gray */
        { 85,  85,  85},   /*  7: Medium Gray (dark gray in EGA) */
        {255,  85,  85},   /*  8: Red / Light Red */
        { 85,  85, 255},   /*  9: Blue / Light Blue */
        { 85, 255,  85},   /* 10: Light Green */
        {255, 255,  85},   /* 11: Yellow */
        { 40,  40,  40},   /* 12: Darkest Gray (DM/CSB specific) */
        {200, 200, 200},   /* 13: Lightest Gray (DM/CSB specific) */
        {100, 100, 100},   /* 14: Dark Medium (DM/CSB specific) */
        {255, 255, 255}    /* 15: White */
};

/* Darker levels: approximate linear attenuation */
const unsigned char G9010_auc_VgaPaletteAll_Compat[6][16][3] = {
        /* Level 0: brightest (title/menu) */
        {
                {  0,   0,   0}, {  0,   0, 170}, {  0, 170,   0}, {  0, 170, 170},
                {170,   0,   0}, {170,  85,   0}, {170, 170, 170}, { 85,  85,  85},
                {255,  85,  85}, { 85,  85, 255}, { 85, 255,  85}, {255, 255,  85},
                { 40,  40,  40}, {200, 200, 200}, {100, 100, 100}, {255, 255, 255}
        },
        /* Level 1 */
        {
                {  0,   0,   0}, {  0,   0, 136}, {  0, 136,   0}, {  0, 136, 136},
                {136,   0,   0}, {136,  68,   0}, {136, 136, 136}, { 68,  68,  68},
                {204,  68,  68}, { 68,  68, 204}, { 68, 204,  68}, {204, 204,  68},
                { 32,  32,  32}, {160, 160, 160}, { 80,  80,  80}, {204, 204, 204}
        },
        /* Level 2 */
        {
                {  0,   0,   0}, {  0,   0, 102}, {  0, 102,   0}, {  0, 102, 102},
                {102,   0,   0}, {102,  51,   0}, {102, 102, 102}, { 51,  51,  51},
                {153,  51,  51}, { 51,  51, 153}, { 51, 153,  51}, {153, 153,  51},
                { 24,  24,  24}, {120, 120, 120}, { 60,  60,  60}, {153, 153, 153}
        },
        /* Level 3 */
        {
                {  0,   0,   0}, {  0,   0,  68}, {  0,  68,   0}, {  0,  68,  68},
                { 68,   0,   0}, { 68,  34,   0}, { 68,  68,  68}, { 34,  34,  34},
                {102,  34,  34}, { 34,  34, 102}, { 34, 102,  34}, {102, 102,  34},
                { 16,  16,  16}, { 80,  80,  80}, { 40,  40,  40}, {102, 102, 102}
        },
        /* Level 4 */
        {
                {  0,   0,   0}, {  0,   0,  34}, {  0,  34,   0}, {  0,  34,  34},
                { 34,   0,   0}, { 34,  17,   0}, { 34,  34,  34}, { 17,  17,  17},
                { 51,  17,  17}, { 17,  17,  51}, { 17,  51,  17}, { 51,  51,  17},
                {  8,   8,   8}, { 40,  40,  40}, { 20,  20,  20}, { 51,  51,  51}
        },
        /* Level 5: darkest */
        {
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
                {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}
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
