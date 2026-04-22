#ifndef REDMCSB_VGA_PALETTE_PC34_COMPAT_H
#define REDMCSB_VGA_PALETTE_PC34_COMPAT_H

/*
 * VGA palette for DM/CSB PC 3.4 — original VGA DAC values.
 *
 * Source: VIDEODRV.C from ReDMCSB (G8149_ICON, G8151–G8156 LIGHT0–LIGHT5).
 * The original game uses a CUSTOM 16-color VGA palette (NOT EGA).
 * The video driver TSR programs the VGA DAC via ports 0x3C8/0x3C9.
 *
 * RGB values are VGA 6-bit DAC values (0-63) scaled to 8-bit (0-255)
 * via: rgb8 = (vga6 << 2) | (vga6 >> 4).
 *
 * Six brightness levels exist for dungeon darkness. Each level has
 * independently tuned per-color values (NOT linear attenuation).
 * Color 4 (Cyan) is invariant across all six brightness levels.
 */

#define VGA_PALETTE_PC34_COLOR_COUNT 16
#define VGA_PALETTE_PC34_BRIGHTNESS_LEVELS 6

/* Brightest palette (index 0) — title/menu/brightest dungeon */
extern const unsigned char G9010_auc_VgaPaletteBrightest_Compat[16][3];

/* All 6 brightness levels: [paletteIndex][colorIndex][rgb] */
extern const unsigned char G9010_auc_VgaPaletteAll_Compat[6][16][3];

/* Map a 4-bit pixel value to an RGB triplet using the specified
   brightness palette level (0=brightest, 5=darkest).
   Returns pointer to a static 3-byte array, or NULL on error. */
const unsigned char* F9010_VGA_GetColorRgb_Compat(
    unsigned char colorIndex,
    unsigned int paletteLevel);

#endif
