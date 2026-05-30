/*
 * csb_v2_filter_palette_correct_pc34.c — V2.0 palette LUT builder for CSB.
 *
 * Builds a [levels][16][3] RGB LUT from G9010_auc_VgaPaletteAll_Compat
 * with gamma / brightness / contrast adjustments. The LUT is consumed
 * by csb_v2_asset_pipeline_process() when palette_enhanced is set,
 * replacing the raw VGA palette in the framebuffer-to-RGBA expansion.
 *
 * Source: Firestaff V2.0 filtered presentation. No ReDMCSB original
 * equivalent: ReDMCSB writes VGA DAC values directly via PALETTE.C /
 * VIDEODRV.C; modern flat panels need gamma compensation to approximate
 * the perceived CRT look.
 *
 * Source-lock anchors (ReDMCSB Common Toolchains):
 *   DATA.C:359-360 k_source_palette_light_amount_floor[]
 *   PANEL.C:418-428 G0304_i_DungeonViewPaletteIndex (6 levels)
 *   ENTRANCE.C:409-441  C28_ENTRANCE_CSB (CSB-specific palette)
 *   G9010_auc_VgaPaletteAll_Compat (include/vga_palette_pc34_compat.h)
 */

#include "csb_v2_filters.h"
#include "csb_v2_asset_pipeline_pc34.h"
#include "vga_palette_pc34_compat.h"

#include <math.h>
#include <stdint.h>

static int clampi(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int csb_v2_filter_palette_build_lut(int gamma100,
    int brightness,
    int contrast,
    uint8_t out_lut[CSB_V2_PALETTE_LEVELS][16][3])
{
    int level, idx, ch;
    double gamma, bright, gain;

    if (!out_lut) {
        return -1;
    }

    gamma100   = clampi(gamma100,   80, 260);
    brightness = clampi(brightness, -50,50);
    contrast   = clampi(contrast,   -50,  50);

    gamma = (double)gamma100 / 100.0;
    bright = (double)brightness / 100.0;
    gain   = 1.0 + ((double)contrast / 100.0);

    for (level = 0; level < CSB_V2_PALETTE_LEVELS; ++level) {
        for (idx = 0; idx < 16; ++idx) {
            for (ch = 0; ch < 3; ++ch) {
                double n = (double)G9010_auc_VgaPaletteAll_Compat[level][idx][ch]
                           / 255.0;
                double y;
                int o;

                /* gamma curve toward CRT response */
                if (gamma > 0.0) {
                    n = pow(n, 1.0 / gamma);
                }

                /* contrast around 0.5 mid-grey */
                y = (n - 0.5) * gain + 0.5;

                /* brightness bias */
                y += bright;

                if (y < 0.0) y = 0.0;
                if (y > 1.0) y = 1.0;

                o = (int)(y * 255.0 + 0.5);
                if (o < 0)   o = 0;
                if (o > 255) o = 255;

                out_lut[level][idx][ch] = (uint8_t)o;
            }
        }
    }
    return 0;
}
