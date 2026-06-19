#include "firestaff/dm1/v1/champion_panel_box_leader_hand_object_name_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:34  - declaration of G0028_ai_Graphic562_Box_LeaderHandObjectName[4]
 * - DATA.C:262 - PC 3.4 init { 233, 319, 33, 38 }
 * - DATA.C:923 - post-1.3 Atari init (same values)
 * - OBJECT.C:281 - F0486_OBJECT_DrawObjectIcon? Actually F0486_OBJECT_DrawSlot
 *                   reads G0028 for the leader-hand object name fill
 * - DEFS.H:     - C00_COLOR_BLACK
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808/809 (Graphics.dat init-table
 * gates batches 1+2+3+4). This gate is a non-mirror-candidate
 * contract for the G0028 leader-hand-object-name black-fill
 * rectangle.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kLeaderX   = 233,
    kLeaderY   = 319,
    kLeaderW   = 33,
    kLeaderH   = 38,

    /* Sanity bounds. */
    kMinX      = 0,
    kMaxX      = 511,
    kMinW      = 1,
    kMaxW      = 511,
    kMinY      = 0,
    kMaxY      = 400,
    kMinH      = 1,
    kMaxH      = 400
};

/* G0028 PC 3.4 init (DATA.C:262). */
static const int s_g0028[4] = { 233, 319, 33, 38 };

const int *
dm1_v1_champion_panel_box_leader_hand_object_name_table_pc34(void)
{
    return s_g0028;
}

int
dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0028[component];
    return 1;
}

int
dm1_v1_champion_panel_box_leader_hand_object_name_x_pc34(void) { return s_g0028[kBoxX]; }
int
dm1_v1_champion_panel_box_leader_hand_object_name_y_pc34(void) { return s_g0028[kBoxY]; }
int
dm1_v1_champion_panel_box_leader_hand_object_name_w_pc34(void) { return s_g0028[kBoxW]; }
int
dm1_v1_champion_panel_box_leader_hand_object_name_h_pc34(void) { return s_g0028[kBoxH]; }

int
dm1_v1_champion_panel_box_leader_hand_object_name_run_pc34(
    DM1_V1_ChampionPanelBoxLeaderHandObjectNameResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_233 = 1;
    int y_is_319 = 1;
    int w_is_33 = 1;
    int h_is_38 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_reasonable_bounds = 1;
    static const int kExpected[4] = { 233, 319, 33, 38 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < 4; ++i) {
        out->tableEntries[i] = s_g0028[i];
        if (s_g0028[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0028[kBoxX] != kLeaderX) x_is_233 = 0;
    if (s_g0028[kBoxY] != kLeaderY) y_is_319 = 0;
    if (s_g0028[kBoxW] != kLeaderW) w_is_33  = 0;
    if (s_g0028[kBoxH] != kLeaderH) h_is_38  = 0;
    out->xIs233 = x_is_233;
    out->yIs319 = y_is_319;
    out->wIs33  = w_is_33;
    out->hIs38  = h_is_38;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < 4; ++i) {
        if (s_g0028[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0028[kBoxW] <= 0) width_positive = 0;
    if (s_g0028[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: X within 0..511 (extended-VGA / double-buffered),
     * Y within sanity bound (Y < 400). The init Y=319 is past the
     * standard 200-pixel viewport but inside the 400-line render
     * buffer the post-1.3 Atari family uses.
     */
    if (s_g0028[kBoxX] < kMinX || s_g0028[kBoxX] > kMaxX) within_reasonable_bounds = 0;
    if (s_g0028[kBoxW] < kMinW || s_g0028[kBoxW] > kMaxW) within_reasonable_bounds = 0;
    if (s_g0028[kBoxY] < kMinY || s_g0028[kBoxY] > kMaxY) within_reasonable_bounds = 0;
    if (s_g0028[kBoxH] < kMinH || s_g0028[kBoxH] > kMaxH) within_reasonable_bounds = 0;
    out->withinReasonableBounds = within_reasonable_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs233 &&
        out->yIs319 &&
        out->wIs33 &&
        out->hIs38 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinReasonableBounds;
    out->assertionCount = 10;
    return out->accepted;
}