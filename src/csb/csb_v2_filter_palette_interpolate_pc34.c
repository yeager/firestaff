/*
 * csb_v2_filter_palette_interpolate_pc34.c — V2.0 per-pixel brightness
 * interpolation on indexed framebuffer for CSB.
 *
 * CSB encodes brightness in 4 bits (0-15) per pixel. The canonical
 * palette has 6 discrete brightness levels (0=brightest, 5=darkest)
 * at source values 15, 12, 9, 6, 3, 0. Any intermediate per-pixel
 * value blends between the two adjacent canonical levels, producing
 * smooth gradients rather than discrete banding.
 *
 * For example, per-pixel level 9 (source) maps to canonical level 2,
 * but per-pixel level 10 blends 60% level-2 + 40% level-1.
 *
 * Implementation: operates in-place on the indexed framebuffer using a
 * single scanline scratch buffer (no heap allocation). After inter-
 * polation, the 4-bit level field encodes the blended value — this is
 * safe because the V1 framebuffer-to-RGBA expansion reads the level
 * field, not the original indexed byte.
 *
 * Source: Firestaff V2.0. No ReDMCSB original equivalent.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   DATA.C:359-360   k_source_palette_light_amount_floor[]
 *   PANEL.C:418-428  G0304_i_DungeonViewPaletteIndex (6 levels)
 *   No ReDMCSB original — presentation enhancement.
 */

#include "csb_v2_filters.h"
#include "render_sdl_m11.h"

#include <stdint.h>
#include <string.h>

#define CSB_V2_INTERPOLATE_MAX_FB_BYTES (M11_FB_WIDTH * M11_FB_HEIGHT)

/* Map 4-bit per-pixel level (0-15) to canonical palette index (0-5).
 * Source level 15 -> canonical 0 (brightest), source level 0 -> canonical 5. */
static int csb_pixel_level_to_canonical(int pixelLevel4)
{
    if (pixelLevel4 <= 0)  return 5;
    if (pixelLevel4 <= 3)  return 4;
    if (pixelLevel4 <= 6)  return 3;
    if (pixelLevel4 <= 9)  return 2;
    if (pixelLevel4 <= 12) return 1;
    return 0; /* 13-15 */
}

/* Returns the fraction [0.0, 1.0) of how far pixelLevel sits between
 * canonical level 'canon' and 'canon-1' (the next brighter canonical
 * level). pixelLevel is the 4-bit field (0-15). */
static float csb_interpolation_factor(int pixelLevel4)
{
    /* Canonical boundaries in 4-bit space: 15, 12, 9, 6, 3, 0
     * Each canonical level spans 3 units of the 4-bit field. */
    if (pixelLevel4 >= 13) return 0.0f;
    int bucket = (15 - pixelLevel4) / 3;
    int bucketStart = 15 - (bucket + 1) * 3;
    int bucketEnd   = 15 - bucket * 3;
    if (bucketEnd <= bucketStart) return 0.0f;
    return (float)(pixelLevel4 - bucketStart)
 / (float)(bucketEnd - bucketStart);
}

int csb_v2_filter_palette_interpolate_indexed(uint8_t* fb, int w, int h,
    int strength_pct)
{
    static uint8_t scratch[CSB_V2_INTERPOLATE_MAX_FB_BYTES];
    int x, y;

    if (!fb || w <= 2 || h <= 2) return -1;
    if ((long)w * (long)h > (long)sizeof(scratch)) return -1;
    if (strength_pct <= 0) return 0;
    if (strength_pct > 100) strength_pct = 100;

    memcpy(scratch, fb, (size_t)(w * h));

    /* Canonical bucket midpoints: 4-bit level field midpoints for each
     * canonical level (0=brightest, 5=darkest). */
    static const uint8_t s_canon_midpoint[CSB_V2_PALETTE_LEVELS] = {
15, /* canon 0: midpoint15 */
        14, /* canon 1: midpoint 13.5 →14 */
        11, /* canon 2: midpoint 10.5 → 11 */
        8,  /* canon 3: midpoint 7.5  → 8  */
        5,  /* canon 4: midpoint 4.5  → 5  */
        2   /* canon 5: midpoint 1.5  → 2  */
    };

    /* Canonical bucket floors: lowest4-bit level in each canonical bucket. */
    static const uint8_t s_canon_floor[CSB_V2_PALETTE_LEVELS] = {
        12, /* canon 0: bucket12-15 */
        9,  /* canon 1: bucket 9-12  */
        6,  /* canon 2: bucket 6-9   */
        3,  /* canon 3: bucket 3-6   */
        0,  /* canon 4: bucket 0-3   */
        0   /* canon 5: floor */
    };

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            uint8_t byte = scratch[y * w + x];
            int idx4  = byte& M11_FB_INDEX_MASK;
            int lvl4  = (byte & M11_FB_LEVEL_MASK) >> M11_FB_LEVEL_SHIFT;

            int canon = csb_pixel_level_to_canonical(lvl4);

            /* Brightest canonical level (0) has nothing brighter — no blend. */
            if (canon == 0) continue;

            /* Compute fractional position between current (darker=canon) and
             * next-brighter (canon-1) canonical level. */
            float raw_frac = csb_interpolation_factor(lvl4);
            float frac = raw_frac * ((float)strength_pct / 100.0f);

            int target_midpoint = s_canon_midpoint[canon - 1];
            int current_floor   = s_canon_floor[canon];

            int blended4 = current_floor
                + (int)((float)(target_midpoint - current_floor) * frac + 0.5f);
            if (blended4 > 15) blended4 = 15;
            if (blended4 < 0)  blended4 = 0;

            fb[y * w + x] = (uint8_t)(
                (blended4 << M11_FB_LEVEL_SHIFT) | (idx4 & M11_FB_INDEX_MASK));
        }
    }
    return 0;
}
