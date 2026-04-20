#ifndef REDMCSB_VGA_PALETTE_PC34_COMPAT_H
#define REDMCSB_VGA_PALETTE_PC34_COMPAT_H

/*
 * VGA palette for CSB PC 3.4, brightest dungeon view palette (index 0).
 *
 * The original game uses a 16-color VGA palette derived from the standard
 * EGA palette. The video driver (loaded as a TSR before the game) sets the
 * VGA DAC registers to these RGB values. Six brightness levels exist for
 * dungeon darkness; this provides the brightest (palette index 0) which is
 * also what is used for the title screen and menus.
 *
 * RGB values are 6-bit VGA DAC values (0-63) scaled to 8-bit (0-255).
 * The scaling is: rgb8 = (vga6 * 255 + 31) / 63, which matches the
 * standard VGA-to-truecolor conversion.
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
