/*
 * dm1_v2_filter_crt_scanlines — V2.0 CRT scanline overlay.
 *
 * Multiplies every even-numbered row (y = 0, 2, 4, ...) by
 *   gain = 1 - strength/100
 * preserving alpha. Odd rows are left untouched. This is the simplest
 * scanline approximation; we trade fidelity for portability (CPU-only,
 * no shader required).
 *
 * Source: Firestaff V2.0. No ReDMCSB original equivalent.
 */

#include "dm1v2/dm1_v2_filters.h"

int dm1_v2_filter_crt_scanlines_rgba(unsigned char* rgba, int w, int h, int strength_pct) {
    int y, x;
    int gain_num; /* gain = gain_num/100 */

    if (!rgba || w <= 0 || h <= 0) {
        return -1;
    }
    if (strength_pct <= 0) {
        return 0;
    }
    if (strength_pct > 100) strength_pct = 100;
    gain_num = 100 - strength_pct;

    for (y = 0; y < h; y += 2) {
        unsigned char* row = rgba + (y * w * 4);
        for (x = 0; x < w; ++x) {
            row[x * 4 + 0] = (unsigned char)((row[x * 4 + 0] * gain_num) / 100);
            row[x * 4 + 1] = (unsigned char)((row[x * 4 + 1] * gain_num) / 100);
            row[x * 4 + 2] = (unsigned char)((row[x * 4 + 2] * gain_num) / 100);
            /* alpha untouched */
        }
    }
    return 0;
}
