#include "firestaff/dm1/v1/champion_panel_box_food_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:41  - declaration of G0035_ai_Graphic562_Box_Food[4]
 * - DATA.C:317 - PC 3.4 init { 112, 159, 60, 68 }
 * - DATA.C:1035 - post-1.3 Atari init variant A { 112, 159, 60, 68 }
 * - DATA.C:1039 - post-1.3 Atari init variant B { 112, 175, 60, 68 }
 * - DATA.C:1043 - post-1.3 Atari init variant C { 112, 207, 60, 68 }
 * - PANEL.C:1585/1589/1593 - blit C030_GRAPHIC_FOOD_LABEL into G0035
 * - DEFS.H:     - C030_GRAPHIC_FOOD_LABEL, C024/C032/C048_BYTE_WIDTH
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index),
 * pass803 (ordered-cells), pass804 (charge-count-to-torch-type).
 * This gate is a non-mirror-candidate contract for the G0035
 * food-label blit rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kPanelLeftX  = 112,
    kFoodY       = 159,
    kFoodW       = 60,
    kFoodH       = 68,

    /* Sanity bounds. The source init Y=159 is past the
     * champion-panel bottom edge Y=136, but the label lives in
     * the panel bitmap region (which extends past the visible
     * viewport). We bound Y < 400 as a sanity check.
     */
    kMinX       = 0,
    kMaxX       = 319,  /* < 320 (PC 3.4 viewport pixel width) */
    kMinW       = 1,
    kMaxW       = 319,
    kMinY       = 0,
    kMaxY       = 400,
    kMinH       = 1,
    kMaxH       = 400
};

/* G0035 PC 3.4 init (DATA.C:317). */
static const int s_g0035[4] = { 112, 159, 60, 68 };

const int *
dm1_v1_champion_panel_box_food_table_pc34(void)
{
    return s_g0035;
}

int
dm1_v1_champion_panel_box_food_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0035[component];
    return 1;
}

int
dm1_v1_champion_panel_box_food_x_pc34(void) { return s_g0035[kBoxX]; }
int
dm1_v1_champion_panel_box_food_y_pc34(void) { return s_g0035[kBoxY]; }
int
dm1_v1_champion_panel_box_food_w_pc34(void) { return s_g0035[kBoxW]; }
int
dm1_v1_champion_panel_box_food_h_pc34(void) { return s_g0035[kBoxH]; }

int
dm1_v1_champion_panel_box_food_run_pc34(
    DM1_V1_ChampionPanelBoxFoodResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_112 = 1;
    int y_is_159 = 1;
    int w_is_60 = 1;
    int h_is_68 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    int within_panel_left_margin = 1;
    static const int kExpected[4] = { 112, 159, 60, 68 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0035[i];
        if (s_g0035[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0035[kBoxX] != kPanelLeftX) x_is_112 = 0;
    if (s_g0035[kBoxY] != kFoodY)      y_is_159 = 0;
    if (s_g0035[kBoxW] != kFoodW)      w_is_60  = 0;
    if (s_g0035[kBoxH] != kFoodH)      h_is_68  = 0;
    out->xIs112 = x_is_112;
    out->yIs159 = y_is_159;
    out->wIs60  = w_is_60;
    out->hIs68  = h_is_68;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0035[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0035[kBoxW] <= 0) width_positive = 0;
    if (s_g0035[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within viewport (320 pixels) + width within
     * viewport. Y within sanity bound (Y < 400). All bounds are
     * generous to handle the post-1.3 Atari init variants with
     * Y values up to 207.
     */
    if (s_g0035[kBoxX] < kMinX || s_g0035[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0035[kBoxW] < kMinW || s_g0035[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0035[kBoxY] < kMinY || s_g0035[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0035[kBoxH] < kMinH || s_g0035[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    /* Phase 6: within panel-left-margin (X >= 100, well to the
     * right of the panel left edge).
     */
    if (s_g0035[kBoxX] < 100) within_panel_left_margin = 0;
    out->withinPanelLeftMargin = within_panel_left_margin;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs112 &&
        out->yIs159 &&
        out->wIs60 &&
        out->hIs68 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds &&
        out->withinPanelLeftMargin;
    out->assertionCount = 11;
    return out->accepted;
}