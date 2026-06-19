#include "firestaff/dm1/v1/champion_panel_box_panel_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:38  - declaration of G0032_ai_Graphic562_Box_Panel[4]
 * - DATA.C:314 - PC 3.4 init { 80, 223, 52, 124 }
 * - DATA.C:1031 - post-1.3 Atari init (same values)
 * - PANEL.C:967/1140/1582/1600/1611 - blit C023/C020/C040 panel
 *                    graphics into G0032's box with byte width C072
 * - CHEST.C - blit C025_GRAPHIC_PANEL_OPEN_CHEST
 * - REVIVE.C - blit C027_GRAPHIC_PANEL_RENAME_CHAMPION
 * - DEFS.H:     - C020/C023/C025/C027/C029/C040_GRAPHIC_PANEL_*,
 *                C072_BYTE_WIDTH, panel colors
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0032 panel-backdrop blit rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kPanelX    = 80,
    kPanelY    = 223,
    kPanelW    = 52,
    kPanelH    = 124,

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

/* G0032 PC 3.4 init (DATA.C:314). */
static const int s_g0032[4] = { 80, 223, 52, 124 };

const int *
dm1_v1_champion_panel_box_panel_table_pc34(void)
{
    return s_g0032;
}

int
dm1_v1_champion_panel_box_panel_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0032[component];
    return 1;
}

int
dm1_v1_champion_panel_box_panel_x_pc34(void) { return s_g0032[kBoxX]; }
int
dm1_v1_champion_panel_box_panel_y_pc34(void) { return s_g0032[kBoxY]; }
int
dm1_v1_champion_panel_box_panel_w_pc34(void) { return s_g0032[kBoxW]; }
int
dm1_v1_champion_panel_box_panel_h_pc34(void) { return s_g0032[kBoxH]; }

int
dm1_v1_champion_panel_box_panel_run_pc34(
    DM1_V1_ChampionPanelBoxPanelResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_80 = 1;
    int y_is_223 = 1;
    int w_is_52 = 1;
    int h_is_124 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    int height_larger_than_width = 1;
    static const int kExpected[4] = { 80, 223, 52, 124 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0032[i];
        if (s_g0032[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0032[kBoxX] != kPanelX) x_is_80 = 0;
    if (s_g0032[kBoxY] != kPanelY) y_is_223 = 0;
    if (s_g0032[kBoxW] != kPanelW) w_is_52 = 0;
    if (s_g0032[kBoxH] != kPanelH) h_is_124 = 0;
    out->xIs80  = x_is_80;
    out->yIs223 = y_is_223;
    out->wIs52  = w_is_52;
    out->hIs124 = h_is_124;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0032[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0032[kBoxW] <= 0) width_positive = 0;
    if (s_g0032[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within viewport (320 pixels) + width within
     * viewport. Y within sanity bound (Y < 400).
     */
    if (s_g0032[kBoxX] < kMinX || s_g0032[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0032[kBoxW] < kMinW || s_g0032[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0032[kBoxY] < kMinY || s_g0032[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0032[kBoxH] < kMinH || s_g0032[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    /* Phase 6: panel is taller than wide (vertical panel design). */
    if (s_g0032[kBoxH] <= s_g0032[kBoxW]) height_larger_than_width = 0;
    out->heightLargerThanWidth = height_larger_than_width;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs80 &&
        out->yIs223 &&
        out->wIs52 &&
        out->hIs124 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds &&
        out->heightLargerThanWidth;
    out->assertionCount = 11;
    return out->accepted;
}