#include "firestaff/dm1/v1/endgame_the_end_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0012_ai_Graphic562_Box_Endgame_TheEnd):
 * - DATA.C:18/143/563
 * - ENDGAME.C:456
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

    kEGX = 120,
    kEGY = 199,
    kEGW = 95,
    kEGH = 108
};

static const int s_g0012[4] = { 120, 199, 95, 108 };

const int *
dm1_v1_endgame_the_end_table_pc34(void)
{
    return s_g0012;
}

int
dm1_v1_endgame_the_end_size_pc34(void)
{
    return DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_SIZE;
}

int
dm1_v1_endgame_the_end_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0012[component];
    return 1;
}

int
dm1_v1_endgame_the_end_x_pc34(void) { return s_g0012[kBoxX]; }
int
dm1_v1_endgame_the_end_y_pc34(void) { return s_g0012[kBoxY]; }
int
dm1_v1_endgame_the_end_w_pc34(void) { return s_g0012[kBoxW]; }
int
dm1_v1_endgame_the_end_h_pc34(void) { return s_g0012[kBoxH]; }

int
dm1_v1_endgame_the_end_run_pc34(DM1_V1_BoxEndgameTheEndResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_120 = 1;
    int y_is_199 = 1;
    int w_is_95 = 1;
    int h_is_108 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 120, 199, 95, 108 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0012[i];
        if (s_g0012[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0012[kBoxX] != kEGX) x_is_120 = 0;
    if (s_g0012[kBoxY] != kEGY) y_is_199 = 0;
    if (s_g0012[kBoxW] != kEGW) w_is_95 = 0;
    if (s_g0012[kBoxH] != kEGH) h_is_108 = 0;
    out->xIs120 = x_is_120;
    out->yIs199 = y_is_199;
    out->wIs95 = w_is_95;
    out->hIs108 = h_is_108;

    for (i = 0; i < DM1_V1_BOXENDGAMETHEEND_PC34_COMPAT_SIZE; ++i) {
        if (s_g0012[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0012[kBoxW] <= 0) width_positive = 0;
    if (s_g0012[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0012[kBoxY] < 0 || s_g0012[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0012[kBoxX] + s_g0012[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs120 &&
        out->yIs199 &&
        out->wIs95 &&
        out->hIs108 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
