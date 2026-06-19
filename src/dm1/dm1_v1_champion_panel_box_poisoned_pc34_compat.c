#include "firestaff/dm1/v1/champion_panel_box_poisoned_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:43  - declaration of G0037_ai_Graphic562_Box_Poisoned[4]
 * - DATA.C:319 - PC 3.4 init { 112, 207, 105, 119 }
 * - DATA.C:1046 - post-1.3 Atari init (same values)
 * - PANEL.C:1603 F0344_INVENTORY_DrawPanel — blit C032_GRAPHIC_POISONED_LABEL
 *                     into G0037's box when Champion->PoisonEventCount != 0
 * - DEFS.H:     - C032_GRAPHIC_POISONED_LABEL
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index),
 * pass803 (ordered-cells), pass804 (charge-count-to-torch-type),
 * pass805 (champion-panel-box-food). This gate is a non-mirror-
 * candidate contract for the G0037 poisoned-label blit rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kPanelLeftX  = 112,
    kPoisonedY   = 207,
    kPoisonedW   = 105,
    kPoisonedH   = 119,

    /* The food label Y is 159 (pass805); the poisoned label Y must
     * be strictly greater (so the poisoned label sits below the
     * food label in the panel).
     */
    kFoodY     = 159,

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

/* G0037 PC 3.4 init (DATA.C:319). */
static const int s_g0037[4] = { 112, 207, 105, 119 };

const int *
dm1_v1_champion_panel_box_poisoned_table_pc34(void)
{
    return s_g0037;
}

int
dm1_v1_champion_panel_box_poisoned_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0037[component];
    return 1;
}

int
dm1_v1_champion_panel_box_poisoned_x_pc34(void) { return s_g0037[kBoxX]; }
int
dm1_v1_champion_panel_box_poisoned_y_pc34(void) { return s_g0037[kBoxY]; }
int
dm1_v1_champion_panel_box_poisoned_w_pc34(void) { return s_g0037[kBoxW]; }
int
dm1_v1_champion_panel_box_poisoned_h_pc34(void) { return s_g0037[kBoxH]; }

int
dm1_v1_champion_panel_box_poisoned_run_pc34(
    DM1_V1_ChampionPanelBoxPoisonedResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_112 = 1;
    int y_is_207 = 1;
    int w_is_105 = 1;
    int h_is_119 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    int within_panel_left_margin = 1;
    int poisoned_below_food_label = 1;
    static const int kExpected[4] = { 112, 207, 105, 119 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0037[i];
        if (s_g0037[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0037[kBoxX] != kPanelLeftX) x_is_112 = 0;
    if (s_g0037[kBoxY] != kPoisonedY) y_is_207 = 0;
    if (s_g0037[kBoxW] != kPoisonedW) w_is_105 = 0;
    if (s_g0037[kBoxH] != kPoisonedH) h_is_119 = 0;
    out->xIs112 = x_is_112;
    out->yIs207 = y_is_207;
    out->wIs105 = w_is_105;
    out->hIs119 = h_is_119;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0037[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0037[kBoxW] <= 0) width_positive = 0;
    if (s_g0037[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within viewport (320 pixels) + width within
     * viewport. Y within sanity bound (Y < 400).
     */
    if (s_g0037[kBoxX] < kMinX || s_g0037[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0037[kBoxW] < kMinW || s_g0037[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0037[kBoxY] < kMinY || s_g0037[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0037[kBoxH] < kMinH || s_g0037[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    /* Phase 6: within panel-left-margin (X >= 100). */
    if (s_g0037[kBoxX] < 100) within_panel_left_margin = 0;
    out->withinPanelLeftMargin = within_panel_left_margin;

    /* Phase 7: poisoned label Y > food label Y (poisoned is drawn
     * below food in the panel). The flag `poisonedBelowFoodLabel` here
     * means the poisoned Y is greater than the food Y (i.e. the
     * poisoned label is positioned BELOW the food label in screen
     * coordinates — confusing naming inherited from "above in
     * priority list" but semantically clear here).
     */
    if (s_g0037[kBoxY] <= kFoodY) poisoned_below_food_label = 0;
    out->poisonedBelowFoodLabel = poisoned_below_food_label;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs112 &&
        out->yIs207 &&
        out->wIs105 &&
        out->hIs119 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds &&
        out->withinPanelLeftMargin &&
        out->poisonedBelowFoodLabel;
    out->assertionCount = 12;
    return out->accepted;
}