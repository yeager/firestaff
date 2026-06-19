#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:8    - declaration of G0002_ai_Graphic562_Box_MovementArrows[4]
 * - DATA.C:123  - PC 3.4 init { 224, 319, 124, 168 }
 * - DATA.C:543  - Atari ST init (same values)
 * - MENUDRAW.C:13 - M520_F0021_MAIN_BlitToScreen(C013_GRAPHIC_MOVEMENT_ARROWS,
 *                     G0002, C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY, 45)
 * - PANEL.C:2369 - F0136_VIDEO_HatchScreenBox(G0002, C00_COLOR_BLACK)
 * - STARTUP2.C:369 - F0136_VIDEO_HatchScreenBox(G0002, C00_COLOR_BLACK)
 * - DEFS.H:      - C013_GRAPHIC_MOVEMENT_ARROWS, C048_BYTE_WIDTH,
 *                   C00_COLOR_BLACK
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831 (Graphics.dat init-table gates batches 1+2+3+4+5+6+
 * 7+8+9+10). This gate is a non-mirror-candidate contract for
 * the G0002 movement-arrows box.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kArrX      = 224,
    kArrY      = 319,
    kArrW      = 124,
    kArrH      = 168
};

static const int s_g0002[4] = { 224, 319, 124, 168 };

const int *
dm1_v1_box_movement_arrows_table_pc34(void)
{
    return s_g0002;
}

int
dm1_v1_box_movement_arrows_size_pc34(void)
{
    return DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_movement_arrows_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0002[component];
    return 1;
}

int
dm1_v1_box_movement_arrows_x_pc34(void) { return s_g0002[kBoxX]; }
int
dm1_v1_box_movement_arrows_y_pc34(void) { return s_g0002[kBoxY]; }
int
dm1_v1_box_movement_arrows_w_pc34(void) { return s_g0002[kBoxW]; }
int
dm1_v1_box_movement_arrows_h_pc34(void) { return s_g0002[kBoxH]; }

int
dm1_v1_box_movement_arrows_run_pc34(
    DM1_V1_BoxMovementArrowsResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_224 = 1;
    int y_is_319 = 1;
    int w_is_124 = 1;
    int h_is_168 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 224, 319, 124, 168 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0002[i];
        if (s_g0002[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0002[kBoxX] != kArrX) x_is_224 = 0;
    if (s_g0002[kBoxY] != kArrY) y_is_319 = 0;
    if (s_g0002[kBoxW] != kArrW) w_is_124 = 0;
    if (s_g0002[kBoxH] != kArrH) h_is_168 = 0;
    out->xIs224 = x_is_224;
    out->yIs319 = y_is_319;
    out->wIs124 = w_is_124;
    out->hIs168 = h_is_168;

    for (i = 0; i < DM1_V1_BOX_MOVEMENT_ARROWS_PC34_COMPAT_SIZE; ++i) {
        if (s_g0002[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0002[kBoxW] <= 0) width_positive = 0;
    if (s_g0002[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    if (s_g0002[kBoxY] > 320 || s_g0002[kBoxY] < 0) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0002[kBoxX] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs224 &&
        out->yIs319 &&
        out->wIs124 &&
        out->hIs168 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}