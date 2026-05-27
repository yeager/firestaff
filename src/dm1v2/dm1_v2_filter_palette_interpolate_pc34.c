/*
 * dm1_v2_filter_palette_interpolate — DM1 V2 Phase 4 per-pixel brightness
 * interpolation on the indexed framebuffer.
 *
 * DM1 encodes brightness in 4 bits (0-15) per pixel. The canonical palette
 * has 6 discrete brightness levels (0=brightest, 5=darkest) at source values
 * 15, 12, 9, 6, 3, 0. Any intermediate per-pixel value should blend between
 * the two adjacent canonical levels, producing smooth gradients rather than
 * discrete banding.
 *
 * For example, per-pixel level 9 (source) maps to canonical level 2, but
 * per-pixel level 10 blends 60% level-2 + 40% level-1. Per-pixel 15 maps to
 * canonical level 0 (brightest, no blend needed).
 *
 * Implementation: operates in-place on the indexed framebuffer using a
 * single scanline scratch buffer (no heap allocation). After interpolation,
 * the 4-bit level field encodes the blended value — this is safe because
 * the V1 framebuffer-to-RGBA expansion reads the level field, not the
 * original indexed byte. By writing back a "smoothed" level, we effectively
 * get smooth palette interpolation without any change to the RGB expansion
 * logic.
 *
 * Source: Firestaff DM1 V2 Phase 4. No ReDMCSB original equivalent.
 */

#include "dm1v2/dm1_v2_filters.h"
#include "render_sdl_m11.h" /* M11_FB_INDEX_MASK / M11_FB_LEVEL_MASK / M11_FB_LEVEL_SHIFT */

#include <stdint.h>
#include <string.h>

#define DM1_V2_INTERPOLATE_MAX_FB_BYTES (M11_FB_WIDTH * M11_FB_HEIGHT)

/* Map 4-bit per-pixel level (0-15) to canonical palette index (0-5).
 * Source level 15 -> canonical 0 (brightest), source level 0 -> canonical 5. */
static int dm1_pixel_level_to_canonical(int pixelLevel4) {
    /* pixelLevel4: 0=darkest, 15=brightest
     * canonical:   0=brightest, 5=darkest */
    if (pixelLevel4 <= 0)  return 5;
    if (pixelLevel4 <= 3)  return 4;
    if (pixelLevel4 <= 6)  return 3;
    if (pixelLevel4 <= 9)  return 2;
    if (pixelLevel4 <= 12) return 1;
    return 0; /* 13-15 */
}

/* Returns the fraction [0.0, 1.0) of how far pixelLevel sits between
 * canonical level 'c' and 'c-1' (the next brighter canonical level).
 * pixelLevel is the 4-bit field (0-15). */
static float dm1_interpolation_factor(int pixelLevel4) {
    /* Canonical boundaries in 4-bit space: 15, 12, 9, 6, 3, 0
     * Each canonical level spans 3 units of the 4-bit field. */
    if (pixelLevel4 >= 13) return 0.0f;
    /* fraction of the current canonical bucket */
    int bucket = (15 - pixelLevel4) / 3; /* 0=bucket0(12-15), 1=bucket1(9-12)... */
    int bucketStart = 15 - (bucket + 1) * 3; /* inclusive start of bucket */
    int bucketEnd   = 15 - bucket * 3;         /* exclusive end */
    if (bucketEnd <= bucketStart) return 0.0f;
    return (float)(pixelLevel4 - bucketStart) / (float)(bucketEnd - bucketStart);
}

int dm1_v2_filter_palette_interpolate_indexed(unsigned char* fb, int w, int h, int strength_pct) {
    static unsigned char scratch[DM1_V2_INTERPOLATE_MAX_FB_BYTES];
    int x, y;

    if (!fb || w <= 2 || h <= 2) return -1;
    if ((long)w * (long)h > (long)sizeof(scratch)) return -1;
    if (strength_pct <= 0) return 0;
    if (strength_pct > 100) strength_pct = 100;

    memcpy(scratch, fb, (size_t)(w * h));

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            unsigned char byte = scratch[y * w + x];
            int idx4 = byte & M11_FB_INDEX_MASK;
            int lvl4 = (byte & M11_FB_LEVEL_MASK) >> M11_FB_LEVEL_SHIFT;

            int canon = dm1_pixel_level_to_canonical(lvl4);

            /* Brightest canonical level (0) has nothing brighter — no blend. */
            if (canon == 0) continue;

            /* Compute fractional position between current (darker=canon) and
             * next-brightter (canon-1). For example: canon=2, next=1.
             * canonical bucket: lvl4=9->canon2, lvl4=6->canon3.
             * Fraction 0.0 = at current canon (darker), 1.0 = at canon-1 (brighter). */
            float raw_frac = dm1_interpolation_factor(lvl4);
            float frac = raw_frac * ((float)strength_pct / 100.0f);

            /* Determine target canonical level based on blend:
             * frac near 0 -> use current (darker) canonical
             * frac near 1 -> use next (brighter) canonical
             * We blend by moving the 4-bit level field toward the brighter
             * canonical's 4-bit midpoint. Canonical levels map:
             *   canon 5 -> 4-bit midpoint 1.5 -> field value 2
             *   canon 4 -> 4-bit midpoint 4.5 -> field value 5
             *   canon 3 -> 4-bit midpoint 7.5 -> field value 8
             *   canon 2 -> 4-bit midpoint 10.5 -> field value 11
             *   canon 1 -> 4-bit midpoint 13.5 -> field value 14
             *   canon 0 -> 4-bit midpoint 15   -> field value 15 */
            static const uint8_t s_canon_midpoint[6] = { 15, 14, 11, 8, 5, 2 };
            int target_midpoint = s_canon_midpoint[canon - 1]; /* brighter canonical's midpoint */

            /* Interpolate the 4-bit level field toward the brighter midpoint.
             * Start from current bucket's floor, blend to target. */
            static const uint8_t s_canon_floor[6] = { 12, 9, 6, 3, 0, 0 };
            int current_floor = s_canon_floor[canon];

            int blended4 = current_floor + (int)((float)(target_midpoint - current_floor) * frac + 0.5f);
            if (blended4 > 15) blended4 = 15;
            if (blended4 < 0) blended4 = 0;

            fb[y * w + x] = (unsigned char)((blended4 << M11_FB_LEVEL_SHIFT) | (idx4 & M11_FB_INDEX_MASK));
        }
    }
    return 0;
}