/*
 * csb_v2_filter_crt_scanlines_pc34.c — V2.0 CRT scanline overlay for CSB.
 *
 * Multiplies every even-numbered row (y = 0, 2, 4, ...) by
 *   gain = 1 - strength_pct/100
 * preserving alpha. Odd rows are left untouched. This is the simplest
 * scanline approximation; we trade fidelity for portability (CPU-only,
 * no shader required).
 *
 * Source: Firestaff V2.0. No ReDMCSB original equivalent.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   No ReDMCSB original — CRT scanlines are a Firestaff V2.0
 *   presentation enhancement targeting modern flat-panel displays.
 *   CSB viewport: DUNVIEW.C:6204-6218 (door panel), DUNVIEW.C:6816-6831 (field draw)
 */

#include "csb_v2_filters.h"
#include "render_sdl_m11.h"

#include <stdint.h>
#include <string.h>

int csb_v2_filter_crt_scanlines_rgba(uint8_t* rgba, int w, int h,
 int strength_pct)
{
    int y, x;

    if (!rgba || w <= 0 || h <= 0) {
        return -1;
    }
    if (strength_pct <= 0) {
        return 0;
    }
    if (strength_pct > 100) {
        strength_pct = 100;
    }
    /* gain = (100 - strength_pct) / 100 as integer numerator (gain_num/100) */
    int gain_num = 100 - strength_pct;

    for (y = 0; y < h; y += 2) {
        uint8_t* row = rgba + (y * w * 4);
        for (x = 0; x < w; ++x) {
            row[x * 4 + 0] = (uint8_t)((row[x * 4 + 0] * gain_num) / 100);
            row[x * 4 + 1] = (uint8_t)((row[x * 4 + 1] * gain_num) / 100);
            row[x * 4 + 2] = (uint8_t)((row[x * 4 + 2] * gain_num) / 100);
            /* alpha untouched */
        }
    }
    return 0;
}
