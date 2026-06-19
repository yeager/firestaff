#include "firestaff/dm1/v1/endgame_champion_mirror_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0015_ai_Graphic562_Box_Endgame_ChampionMirror):
 * - DATA.C:21/149/569
 * - ENDGAME.C:347/389/390
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

    kEGX = 11,
    kEGY = 74,
    kEGW = 7,
    kEGH = 49
};

static const int s_g0015[4] = { 11, 74, 7, 49 };

const int *
dm1_v1_endgame_champion_mirror_table_pc34(void)
{
    return s_g0015;
}

int
dm1_v1_endgame_champion_mirror_size_pc34(void)
{
    return DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_SIZE;
}

int
dm1_v1_endgame_champion_mirror_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0015[component];
    return 1;
}

int
dm1_v1_endgame_champion_mirror_x_pc34(void) { return s_g0015[kBoxX]; }
int
dm1_v1_endgame_champion_mirror_y_pc34(void) { return s_g0015[kBoxY]; }
int
dm1_v1_endgame_champion_mirror_w_pc34(void) { return s_g0015[kBoxW]; }
int
dm1_v1_endgame_champion_mirror_h_pc34(void) { return s_g0015[kBoxH]; }

int
dm1_v1_endgame_champion_mirror_run_pc34(DM1_V1_BoxEndgameChampionMirrorResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_11 = 1;
    int y_is_74 = 1;
    int w_is_7 = 1;
    int h_is_49 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 11, 74, 7, 49 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0015[i];
        if (s_g0015[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0015[kBoxX] != kEGX) x_is_11 = 0;
    if (s_g0015[kBoxY] != kEGY) y_is_74 = 0;
    if (s_g0015[kBoxW] != kEGW) w_is_7 = 0;
    if (s_g0015[kBoxH] != kEGH) h_is_49 = 0;
    out->xIs11 = x_is_11;
    out->yIs74 = y_is_74;
    out->wIs7 = w_is_7;
    out->hIs49 = h_is_49;

    for (i = 0; i < DM1_V1_BOXENDGAMECHAMPIONMIRROR_PC34_COMPAT_SIZE; ++i) {
        if (s_g0015[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0015[kBoxW] <= 0) width_positive = 0;
    if (s_g0015[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0015[kBoxY] < 0 || s_g0015[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0015[kBoxX] + s_g0015[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs11 &&
        out->yIs74 &&
        out->wIs7 &&
        out->hIs49 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
