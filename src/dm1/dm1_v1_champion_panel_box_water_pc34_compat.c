#include "firestaff/dm1/v1/champion_panel_box_water_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:42  - declaration of G0036_ai_Graphic562_Box_Water[4]
 * - DATA.C:318 - PC 3.4 init { 112, 159, 83, 91 }
 * - DATA.C:1036 - post-1.3 Atari init (same values)
 * - PANEL.C:1586/1590/1594 - blit C031_GRAPHIC_WATER_LABEL into G0036
 * - DEFS.H:     - C031_GRAPHIC_WATER_LABEL,
 *                C024/C032_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0036 water-label blit rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kWaterX    = 112,
    kWaterY    = 159,
    kWaterW    = 83,
    kWaterH    = 91,

    /* Food label X (for cross-alignment check; see pass805). */
    kFoodX     = 112,

    /* Sanity bounds. */
    kMinX      = 0,
    kMaxX      = 319,
    kMinW      = 1,
    kMaxW      = 319,
    kMinY      = 0,
    kMaxY      = 400,
    kMinH      = 1,
    kMaxH      = 400
};

/* G0036 PC 3.4 init (DATA.C:318). */
static const int s_g0036[4] = { 112, 159, 83, 91 };

const int *
dm1_v1_champion_panel_box_water_table_pc34(void)
{
    return s_g0036;
}

int
dm1_v1_champion_panel_box_water_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0036[component];
    return 1;
}

int
dm1_v1_champion_panel_box_water_x_pc34(void) { return s_g0036[kBoxX]; }
int
dm1_v1_champion_panel_box_water_y_pc34(void) { return s_g0036[kBoxY]; }
int
dm1_v1_champion_panel_box_water_w_pc34(void) { return s_g0036[kBoxW]; }
int
dm1_v1_champion_panel_box_water_h_pc34(void) { return s_g0036[kBoxH]; }

int
dm1_v1_champion_panel_box_water_run_pc34(
    DM1_V1_ChampionPanelBoxWaterResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_112 = 1;
    int y_is_159 = 1;
    int w_is_83 = 1;
    int h_is_91 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    int x_aligned_with_food_label = 1;
    static const int kExpected[4] = { 112, 159, 83, 91 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0036[i];
        if (s_g0036[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0036[kBoxX] != kWaterX) x_is_112 = 0;
    if (s_g0036[kBoxY] != kWaterY) y_is_159 = 0;
    if (s_g0036[kBoxW] != kWaterW) w_is_83  = 0;
    if (s_g0036[kBoxH] != kWaterH) h_is_91  = 0;
    out->xIs112 = x_is_112;
    out->yIs159 = y_is_159;
    out->wIs83  = w_is_83;
    out->hIs91  = h_is_91;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0036[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0036[kBoxW] <= 0) width_positive = 0;
    if (s_g0036[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within viewport (320 pixels) + width within
     * viewport. Y within sanity bound (Y < 400).
     */
    if (s_g0036[kBoxX] < kMinX || s_g0036[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0036[kBoxW] < kMinW || s_g0036[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0036[kBoxY] < kMinY || s_g0036[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0036[kBoxH] < kMinH || s_g0036[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    /* Phase 6: water X aligned with food label X (both 112 — paired
     * labels in the same panel column).
     */
    if (s_g0036[kBoxX] != kFoodX) x_aligned_with_food_label = 0;
    out->xAlignedWithFoodLabel = x_aligned_with_food_label;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs112 &&
        out->yIs159 &&
        out->wIs83 &&
        out->hIs91 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds &&
        out->xAlignedWithFoodLabel;
    out->assertionCount = 11;
    return out->accepted;
}