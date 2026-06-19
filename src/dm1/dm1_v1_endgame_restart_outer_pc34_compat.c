#include "firestaff/dm1/v1/endgame_restart_outer_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0013_ai_Graphic562_Box_Endgame_Restart_Outer):
 * - DATA.C:19/145/565
 * - ENDGAME.C:488
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

    kEGX = 103,
    kEGY = 217,
    kEGW = 145,
    kEGH = 159
};

static const int s_g0013[4] = { 103, 217, 145, 159 };

const int *
dm1_v1_endgame_restart_outer_table_pc34(void)
{
    return s_g0013;
}

int
dm1_v1_endgame_restart_outer_size_pc34(void)
{
    return DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_SIZE;
}

int
dm1_v1_endgame_restart_outer_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0013[component];
    return 1;
}

int
dm1_v1_endgame_restart_outer_x_pc34(void) { return s_g0013[kBoxX]; }
int
dm1_v1_endgame_restart_outer_y_pc34(void) { return s_g0013[kBoxY]; }
int
dm1_v1_endgame_restart_outer_w_pc34(void) { return s_g0013[kBoxW]; }
int
dm1_v1_endgame_restart_outer_h_pc34(void) { return s_g0013[kBoxH]; }

int
dm1_v1_endgame_restart_outer_run_pc34(DM1_V1_BoxEndgameRestartOuterResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_103 = 1;
    int y_is_217 = 1;
    int w_is_145 = 1;
    int h_is_159 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 103, 217, 145, 159 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0013[i];
        if (s_g0013[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0013[kBoxX] != kEGX) x_is_103 = 0;
    if (s_g0013[kBoxY] != kEGY) y_is_217 = 0;
    if (s_g0013[kBoxW] != kEGW) w_is_145 = 0;
    if (s_g0013[kBoxH] != kEGH) h_is_159 = 0;
    out->xIs103 = x_is_103;
    out->yIs217 = y_is_217;
    out->wIs145 = w_is_145;
    out->hIs159 = h_is_159;

    for (i = 0; i < DM1_V1_BOXENDGAMERESTARTOUTER_PC34_COMPAT_SIZE; ++i) {
        if (s_g0013[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0013[kBoxW] <= 0) width_positive = 0;
    if (s_g0013[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0013[kBoxY] < 0 || s_g0013[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0013[kBoxX] + s_g0013[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs103 &&
        out->yIs217 &&
        out->wIs145 &&
        out->hIs159 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
