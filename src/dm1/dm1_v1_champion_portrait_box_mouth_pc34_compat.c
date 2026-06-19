#include "firestaff/dm1/v1/champion_portrait_box_mouth_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:79   - declaration of G0048_ai_Graphic562_Box_Mouth[4]
 * - DATA.C:423  - PC 3.4 init { 55, 72, 12, 29 }
 * - DATA.C:1095 - post-1.3 Atari init (same values)
 * - CHAMDRAW.C:914 - blit mouth graphic into G0048's box with byte
 *                     width C016_BYTE_WIDTH and color
 *                     C12_COLOR_DARKEST_GRAY (depth 18)
 * - DEFS.H:     - C016_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816 (Graphics.dat init-
 * table gates batches 1+2+3+4+5). This gate is a non-mirror-
 * candidate contract for the G0048 mouth-overlay blit rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kMouthX    = 55,
    kMouthY    = 72,
    kMouthW    = 12,
    kMouthH    = 29,

    /* Sanity bounds. The mouth box lives inside the portrait region
     * (roughly 50..100 wide, 0..100 tall).
     */
    kMinX      = 0,
    kMaxX      = 319,
    kMinW      = 1,
    kMaxW      = 319,
    kMinY      = 0,
    kMaxY      = 400,
    kMinH      = 1,
    kMaxH      = 400
};

/* G0048 PC 3.4 init (DATA.C:423). */
static const int s_g0048[4] = { 55, 72, 12, 29 };

const int *
dm1_v1_champion_portrait_box_mouth_table_pc34(void)
{
    return s_g0048;
}

int
dm1_v1_champion_portrait_box_mouth_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0048[component];
    return 1;
}

int
dm1_v1_champion_portrait_box_mouth_x_pc34(void) { return s_g0048[kBoxX]; }
int
dm1_v1_champion_portrait_box_mouth_y_pc34(void) { return s_g0048[kBoxY]; }
int
dm1_v1_champion_portrait_box_mouth_w_pc34(void) { return s_g0048[kBoxW]; }
int
dm1_v1_champion_portrait_box_mouth_h_pc34(void) { return s_g0048[kBoxH]; }

int
dm1_v1_champion_portrait_box_mouth_run_pc34(
    DM1_V1_ChampionPortraitBoxMouthResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_55 = 1;
    int y_is_72 = 1;
    int w_is_12 = 1;
    int h_is_29 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    int height_larger_than_width = 1;
    static const int kExpected[4] = { 55, 72, 12, 29 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0048[i];
        if (s_g0048[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0048[kBoxX] != kMouthX) x_is_55 = 0;
    if (s_g0048[kBoxY] != kMouthY) y_is_72 = 0;
    if (s_g0048[kBoxW] != kMouthW) w_is_12 = 0;
    if (s_g0048[kBoxH] != kMouthH) h_is_29 = 0;
    out->xIs55 = x_is_55;
    out->yIs72 = y_is_72;
    out->wIs12 = w_is_12;
    out->hIs29 = h_is_29;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0048[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0048[kBoxW] <= 0) width_positive = 0;
    if (s_g0048[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within viewport (320 pixels) + width within
     * viewport. Y within sanity bound (Y < 400).
     */
    if (s_g0048[kBoxX] < kMinX || s_g0048[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0048[kBoxW] < kMinW || s_g0048[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0048[kBoxY] < kMinY || s_g0048[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0048[kBoxH] < kMinH || s_g0048[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    /* Phase 6: mouth is taller than wide (small vertical mouth box:
     * H=29, W=12).
     */
    if (s_g0048[kBoxH] <= s_g0048[kBoxW]) height_larger_than_width = 0;
    out->heightLargerThanWidth = height_larger_than_width;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs55 &&
        out->yIs72 &&
        out->wIs12 &&
        out->hIs29 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds &&
        out->heightLargerThanWidth;
    out->assertionCount = 11;
    return out->accepted;
}