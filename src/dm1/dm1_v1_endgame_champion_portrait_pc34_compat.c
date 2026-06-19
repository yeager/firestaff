#include "firestaff/dm1/v1/endgame_champion_portrait_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0016_ai_Graphic562_Box_Endgame_ChampionPortrait):
 * - DATA.C:22/151/571
 * - ENDGAME.C:353/357/391/392
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

enum {
    kBoxX = 0,
    kBoxY = 1,
    kBoxW = 2,
    kBoxH = 3,

    kEGX = 27,
    kEGY = 58,
    kEGW = 13,
    kEGH = 41
};

static const int s_g0016[4] = { 27, 58, 13, 41 };

const int *
dm1_v1_endgame_champion_portrait_table_pc34(void)
{
    return s_g0016;
}

int
dm1_v1_endgame_champion_portrait_size_pc34(void)
{
    return DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_SIZE;
}

int
dm1_v1_endgame_champion_portrait_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0016[component];
    return 1;
}

int
dm1_v1_endgame_champion_portrait_x_pc34(void) { return s_g0016[kBoxX]; }
int
dm1_v1_endgame_champion_portrait_y_pc34(void) { return s_g0016[kBoxY]; }
int
dm1_v1_endgame_champion_portrait_w_pc34(void) { return s_g0016[kBoxW]; }
int
dm1_v1_endgame_champion_portrait_h_pc34(void) { return s_g0016[kBoxH]; }

int
dm1_v1_endgame_champion_portrait_run_pc34(DM1_V1_BoxEndgameChampionPortraitResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_27 = 1;
    int y_is_58 = 1;
    int w_is_13 = 1;
    int h_is_41 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 27, 58, 13, 41 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0016[i];
        if (s_g0016[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0016[kBoxX] != kEGX) x_is_27 = 0;
    if (s_g0016[kBoxY] != kEGY) y_is_58 = 0;
    if (s_g0016[kBoxW] != kEGW) w_is_13 = 0;
    if (s_g0016[kBoxH] != kEGH) h_is_41 = 0;
    out->xIs27 = x_is_27;
    out->yIs58 = y_is_58;
    out->wIs13 = w_is_13;
    out->hIs41 = h_is_41;

    for (i = 0; i < DM1_V1_BOXENDGAMECHAMPIONPORTRAIT_PC34_COMPAT_SIZE; ++i) {
        if (s_g0016[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0016[kBoxW] <= 0) width_positive = 0;
    if (s_g0016[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0016[kBoxY] < 0 || s_g0016[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0016[kBoxX] + s_g0016[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs27 &&
        out->yIs58 &&
        out->wIs13 &&
        out->hIs41 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
