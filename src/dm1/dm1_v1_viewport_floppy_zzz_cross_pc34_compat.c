#include "firestaff/dm1/v1/viewport_floppy_zzz_cross_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0041_ai_Graphic562_Box_ViewportFloppyZzzCross):
 * - DATA.C:37/175/595
 * - PANEL.C:2379
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

    kVPX = 174,
    kVPY = 218,
    kVPW = 2,
    kVPH = 12
};

static const int s_g0041[4] = { 174, 218, 2, 12 };

const int *
dm1_v1_viewport_floppy_zzz_cross_table_pc34(void)
{
    return s_g0041;
}

int
dm1_v1_viewport_floppy_zzz_cross_size_pc34(void)
{
    return DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_SIZE;
}

int
dm1_v1_viewport_floppy_zzz_cross_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0041[component];
    return 1;
}

int
dm1_v1_viewport_floppy_zzz_cross_x_pc34(void) { return s_g0041[kBoxX]; }
int
dm1_v1_viewport_floppy_zzz_cross_y_pc34(void) { return s_g0041[kBoxY]; }
int
dm1_v1_viewport_floppy_zzz_cross_w_pc34(void) { return s_g0041[kBoxW]; }
int
dm1_v1_viewport_floppy_zzz_cross_h_pc34(void) { return s_g0041[kBoxH]; }

int
dm1_v1_viewport_floppy_zzz_cross_run_pc34(DM1_V1_BoxViewportFloppyZzzCrossResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_174 = 1;
    int y_is_218 = 1;
    int w_is_2 = 1;
    int h_is_12 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 174, 218, 2, 12 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0041[i];
        if (s_g0041[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0041[kBoxX] != kVPX) x_is_174 = 0;
    if (s_g0041[kBoxY] != kVPY) y_is_218 = 0;
    if (s_g0041[kBoxW] != kVPW) w_is_2 = 0;
    if (s_g0041[kBoxH] != kVPH) h_is_12 = 0;
    out->xIs174 = x_is_174;
    out->yIs218 = y_is_218;
    out->wIs2 = w_is_2;
    out->hIs12 = h_is_12;

    for (i = 0; i < DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_SIZE; ++i) {
        if (s_g0041[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0041[kBoxW] <= 0) width_positive = 0;
    if (s_g0041[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0041[kBoxY] < 0 || s_g0041[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0041[kBoxX] + s_g0041[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs174 &&
        out->yIs218 &&
        out->wIs2 &&
        out->hIs12 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}
