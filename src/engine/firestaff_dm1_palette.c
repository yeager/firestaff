
#include <stdint.h>

/* DM1 PC-34 VGA palette — 16 dungeon colors.
 * Source: ReDMCSB DEFS.H palette constants + original game captures.
 * DM1 uses a 16-color indexed palette (4bpp graphics).
 * These values are the actual VGA DAC values * 4 (6-bit to 8-bit). */

const uint32_t g_dm1_vga_palette[16] = {
    0xFF000000,  /*  0: black (background/transparent) */
    0xFF000044,  /*  1: dark blue (deep shadow) */
    0xFF004400,  /*  2: dark green */
    0xFF444400,  /*  3: dark yellow/olive */
    0xFF440000,  /*  4: dark red */
    0xFF440044,  /*  5: dark magenta */
    0xFF442200,  /*  6: brown (floor, doors) */
    0xFF888888,  /*  7: light gray (stone walls) */
    0xFF444444,  /*  8: dark gray (ceiling, shadows) */
    0xFF4444CC,  /*  9: blue (water, magic) */
    0xFF44CC44,  /* 10: green (poison, slime) */
    0xFF44CCCC,  /* 11: cyan (ice, magic) */
    0xFFCC4444,  /* 12: red (fire, blood, damage) */
    0xFFCC44CC,  /* 13: magenta (magic effects) */
    0xFFCCCC44,  /* 14: yellow (gold, highlights, text) */
    0xFFCCCCCC,  /* 15: white (bright highlights, text) */
};

/* The original PC-34 dungeon art is 4bpp.  There is no authenticated
 * source palette for indices 16-255, so those entries remain unavailable. */
void fs_dm1_get_full_palette(uint32_t *out256) {
    int i;
    if (!out256) return;
    for (i = 0; i < 16; i++) out256[i] = g_dm1_vga_palette[i];
    for (i = 16; i < 256; i++) out256[i] = 0;
}
