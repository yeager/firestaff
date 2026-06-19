#include "firestaff/dm1/v1/endgame_restart_inner_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0014_ai_Graphic562_Box_Endgame_Restart_Inner):
 * - DATA.C:20/147/567
 * - ENDGAME.C:489
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

    kEGX = 105,
    kEGY = 215,
    kEGW = 147,
    kEGH = 157
};

static const int s_g0014[4] = { 105, 215, 147, 157 };

const int *
dm1_v1_endgame_restart_inner_table_pc34(void)
{
    return s_g0014;
}

int
dm1_v1_endgame_restart_inner_size_pc34(void)
{
    return DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_SIZE;
}

int
dm1_v1_endgame_restart_inner_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0014[component];
    return 1;
}

int
dm1_v1_endgame_restart_inner_x_pc34(void) { return s_g0014[kBoxX]; }
int
dm1_v1_endgame_restart_inner_y_pc34(void) { return s_g0014[kBoxY]; }
int
dm1_v1_endgame_restart_inner_w_pc34(void) { return s_g0014[kBoxW]; }
int
dm1_v1_endgame_restart_inner_h_pc34(void) { return s_g0014[kBoxH]; }

int
dm1_v1_endgame_restart_inner_run_pc34(DM1_V1_BoxEndgameRestartInnerResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_105 = 1;
    int y_is_215 = 1;
    int w_is_147 = 1;
    int h_is_157 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 105, 215, 147, 157 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0014[i];
        if (s_g0014[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0014[kBoxX] != kEGX) x_is_105 = 0;
    if (s_g0014[kBoxY] != kEGY) y_is_215 = 0;
    if (s_g0014[kBoxW] != kEGW) w_is_147 = 0;
    if (s_g0014[kBoxH] != kEGH) h_is_157 = 0;
    out->xIs105 = x_is_105;
    out->yIs215 = y_is_215;
    out->wIs147 = w_is_147;
    out->hIs157 = h_is_157;

    for (i = 0; i < DM1_V1_BOXENDGAMERESTARTINNER_PC34_COMPAT_SIZE; ++i) {
        if (s_g0014[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0014[kBoxW] <= 0) width_positive = 0;
    if (s_g0014[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0014[kBoxY] < 0 || s_g0014[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0014[kBoxX] + s_g0014[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs105 &&
        out->yIs215 &&
        out->wIs147 &&
        out->hIs157 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
