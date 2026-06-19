#include "firestaff/dm1/v1/champion_panel_box_object_description_circle_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:40  - declaration of G0034_ai_Graphic562_Box_ObjectDescriptionCircle[4]
 * - DATA.C:316 - PC 3.4 init { 105, 136, 53, 79 }
 * - DATA.C:1033 - post-1.3 Atari init (same values)
 * - PANEL.C:1141 - blit C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE
 *                    into G0034's box with byte width C016_BYTE_WIDTH
 * - DEFS.H:     - C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE,
 *                C016_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0034 object-description-circle blit rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kCircleX   = 105,
    kCircleY   = 136,
    kCircleW   = 53,
    kCircleH   = 79,

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

/* G0034 PC 3.4 init (DATA.C:316). */
static const int s_g0034[4] = { 105, 136, 53, 79 };

const int *
dm1_v1_champion_panel_box_object_description_circle_table_pc34(void)
{
    return s_g0034;
}

int
dm1_v1_champion_panel_box_object_description_circle_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0034[component];
    return 1;
}

int
dm1_v1_champion_panel_box_object_description_circle_x_pc34(void) { return s_g0034[kBoxX]; }
int
dm1_v1_champion_panel_box_object_description_circle_y_pc34(void) { return s_g0034[kBoxY]; }
int
dm1_v1_champion_panel_box_object_description_circle_w_pc34(void) { return s_g0034[kBoxW]; }
int
dm1_v1_champion_panel_box_object_description_circle_h_pc34(void) { return s_g0034[kBoxH]; }

int
dm1_v1_champion_panel_box_object_description_circle_run_pc34(
    DM1_V1_ChampionPanelBoxObjectDescriptionCircleResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_105 = 1;
    int y_is_136 = 1;
    int w_is_53 = 1;
    int h_is_79 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    int height_larger_than_width = 1;
    static const int kExpected[4] = { 105, 136, 53, 79 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0034[i];
        if (s_g0034[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0034[kBoxX] != kCircleX) x_is_105 = 0;
    if (s_g0034[kBoxY] != kCircleY) y_is_136 = 0;
    if (s_g0034[kBoxW] != kCircleW) w_is_53  = 0;
    if (s_g0034[kBoxH] != kCircleH) h_is_79  = 0;
    out->xIs105 = x_is_105;
    out->yIs136 = y_is_136;
    out->wIs53  = w_is_53;
    out->hIs79  = h_is_79;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0034[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0034[kBoxW] <= 0) width_positive = 0;
    if (s_g0034[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within viewport (320 pixels) + width within
     * viewport. Y within sanity bound (Y < 400).
     */
    if (s_g0034[kBoxX] < kMinX || s_g0034[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0034[kBoxW] < kMinW || s_g0034[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0034[kBoxY] < kMinY || s_g0034[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0034[kBoxH] < kMinH || s_g0034[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    /* Phase 6: circle is taller than wide (circular indicator
     * aspect — H=79, W=53).
     */
    if (s_g0034[kBoxH] <= s_g0034[kBoxW]) height_larger_than_width = 0;
    out->heightLargerThanWidth = height_larger_than_width;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs105 &&
        out->yIs136 &&
        out->wIs53 &&
        out->hIs79 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds &&
        out->heightLargerThanWidth;
    out->assertionCount = 11;
    return out->accepted;
}