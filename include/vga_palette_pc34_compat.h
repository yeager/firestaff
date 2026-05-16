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
#define VGA_PALETTE_PC34_SPECIAL_PALETTE_COUNT 2

#define VGA_PALETTE_PC34_SPECIAL_CREDITS  0
#define VGA_PALETTE_PC34_SPECIAL_ENTRANCE 1

/* Brightest palette (index 0) — title/menu/brightest dungeon */
extern const unsigned char G9010_auc_VgaPaletteBrightest_Compat[16][3];

/* All 6 brightness levels: [paletteIndex][colorIndex][rgb] */
extern const unsigned char G9010_auc_VgaPaletteAll_Compat[6][16][3];

/* Special 16-colour VGA palettes from VIDEODRV.C / DRAWVIEW.C.
   These are used by original DM PC 3.4 for full-screen credits and
   entrance-door presentation paths, independent of dungeon brightness. */
extern const unsigned char G9011_auc_VgaPaletteCredits_Compat[16][3];
extern const unsigned char G9012_auc_VgaPaletteEntrance_Compat[16][3];
extern const unsigned char G9013_auc_VgaPaletteSpecial_Compat[VGA_PALETTE_PC34_SPECIAL_PALETTE_COUNT][16][3];

/* Map a 4-bit pixel value to an RGB triplet using the specified
   brightness palette level (0=brightest, 5=darkest).
   Returns pointer to a static 3-byte array, or NULL on error. */
const unsigned char* F9010_VGA_GetColorRgb_Compat(
    unsigned char colorIndex,
    unsigned int paletteLevel);

/* Map a 4-bit pixel value through one of the source-backed special
   palettes.  specialPalette is VGA_PALETTE_PC34_SPECIAL_* above. */
const unsigned char* F9011_VGA_GetSpecialColorRgb_Compat(
    unsigned char colorIndex,
    unsigned int specialPalette);

#endif
