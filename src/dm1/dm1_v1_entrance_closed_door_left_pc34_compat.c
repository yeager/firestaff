#include "firestaff/dm1/v1/entrance_closed_door_left_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0010_ai_Graphic562_Box_Entrance_ClosedDoorLeft):
 * - DATA.C:16/139/559
 * - ENTRANCE.C:574/578
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

    kENX = 0,
    kENY = 104,
    kENW = 30,
    kENH = 190
};

static const int s_g0010[4] = { 0, 104, 30, 190 };

const int *
dm1_v1_entrance_closed_door_left_table_pc34(void)
{
    return s_g0010;
}

int
dm1_v1_entrance_closed_door_left_size_pc34(void)
{
    return DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_SIZE;
}

int
dm1_v1_entrance_closed_door_left_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0010[component];
    return 1;
}

int
dm1_v1_entrance_closed_door_left_x_pc34(void) { return s_g0010[kBoxX]; }
int
dm1_v1_entrance_closed_door_left_y_pc34(void) { return s_g0010[kBoxY]; }
int
dm1_v1_entrance_closed_door_left_w_pc34(void) { return s_g0010[kBoxW]; }
int
dm1_v1_entrance_closed_door_left_h_pc34(void) { return s_g0010[kBoxH]; }

int
dm1_v1_entrance_closed_door_left_run_pc34(DM1_V1_BoxEntranceClosedDoorLeftResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_0 = 1;
    int y_is_104 = 1;
    int w_is_30 = 1;
    int h_is_190 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 0, 104, 30, 190 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0010[i];
        if (s_g0010[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0010[kBoxX] != kENX) x_is_0 = 0;
    if (s_g0010[kBoxY] != kENY) y_is_104 = 0;
    if (s_g0010[kBoxW] != kENW) w_is_30 = 0;
    if (s_g0010[kBoxH] != kENH) h_is_190 = 0;
    out->xIs0 = x_is_0;
    out->yIs104 = y_is_104;
    out->wIs30 = w_is_30;
    out->hIs190 = h_is_190;

    for (i = 0; i < DM1_V1_BOXENTRANCECLOSEDDOORLEFT_PC34_COMPAT_SIZE; ++i) {
        if (s_g0010[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0010[kBoxW] <= 0) width_positive = 0;
    if (s_g0010[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0010[kBoxY] < 0 || s_g0010[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0010[kBoxX] + s_g0010[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs0 &&
        out->yIs104 &&
        out->wIs30 &&
        out->hIs190 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
