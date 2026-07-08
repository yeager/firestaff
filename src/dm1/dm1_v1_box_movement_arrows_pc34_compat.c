#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:8    - declaration of G0002_ai_Graphic562_Box_MovementArrows[4]
 * - DATA.C:123  - PC 3.4 init { 224, 319, 124, 168 }
 * - DATA.C:543  - Atari ST init (same values)
 * - MENUDRAW.C:13 - M520_F0021_MAIN_BlitToScreen(C013_GRAPHIC_MOVEMENT_ARROWS,
 *                     G0002, C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY, 45)
 * - MENUDRAW.C:16 - F0660_(C013_GRAPHIC_MOVEMENT_ARROWS,
 *                     C009_ZONE_MOVEMENT_ARROWS, CM1_COLOR_NO_TRANSPARENCY)
 * - COMMAND.C:396-403 - G0448_as_Graphic561_SecondaryMouseInput_Movement
 *                     maps movement commands through C068..C073 zones
 * - COMMAND.C:323-328 - G0463_aai_Graphic561_Box_MovementArrows
 *                     stores forward/right/back/left hit rectangles
 * - PANEL.C:2369 - F0136_VIDEO_HatchScreenBox(G0002, C00_COLOR_BLACK)
 * - STARTUP2.C:369 - F0136_VIDEO_HatchScreenBox(G0002, C00_COLOR_BLACK)
 * - DEFS.H:2174, 3753, 3758-3763 - C013_GRAPHIC_MOVEMENT_ARROWS,
 *                   C009_ZONE_MOVEMENT_ARROWS, C068..C073 zones
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
    kArrH      = 168,

    kZoneMovementArrows = 9,
    kGraphicMovementArrows = 13,
    kOuterX = 224,
    kOuterY = 124,
    kOuterW = 96,
    kOuterH = 45,
    kGraphicX = 233,
    kGraphicY = 124,
    kGraphicW = 87,
    kGraphicH = 45
};

static const int s_g0002[4] = { 224, 319, 124, 168 };
static const int s_arrow_zone_ids[DM1_V1_MOVEMENT_ARROW_COUNT_PC34] = {
    68, 69, 70, 71, 72, 73
};
static const DM1_V1_MovementArrowRectPc34
s_arrow_rects[DM1_V1_MOVEMENT_ARROW_COUNT_PC34] = {
    { 234, 125, 19, 21 }, /* C068_ZONE_TURN_LEFT */
    { 291, 125, 19, 21 }, /* C069_ZONE_TURN_RIGHT */
    { 263, 125, 27, 21 }, /* C070_ZONE_MOVE_FORWARD */
    { 291, 147, 28, 21 }, /* C071_ZONE_MOVE_RIGHT */
    { 263, 147, 27, 21 }, /* C072_ZONE_MOVE_BACKWARD */
    { 234, 147, 28, 21 }  /* C073_ZONE_MOVE_LEFT */
};
static const int s_arrow_masks[DM1_V1_MOVEMENT_ARROW_COUNT_PC34] = {
    DM1_V1_MOVEMENT_ARROW_VIS_TURN_LEFT_PC34,
    DM1_V1_MOVEMENT_ARROW_VIS_TURN_RIGHT_PC34,
    DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34,
    DM1_V1_MOVEMENT_ARROW_VIS_RIGHT_PC34,
    DM1_V1_MOVEMENT_ARROW_VIS_BACKWARD_PC34,
    DM1_V1_MOVEMENT_ARROW_VIS_LEFT_PC34
};

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
dm1_v1_movement_arrows_zone_id_pc34(void)
{
    return kZoneMovementArrows;
}

int
dm1_v1_movement_arrows_graphic_id_pc34(void)
{
    return kGraphicMovementArrows;
}

int
dm1_v1_movement_arrows_outer_rect_pc34(DM1_V1_MovementArrowRectPc34 *out)
{
    if (!out) {
        return 0;
    }
    out->x = kOuterX;
    out->y = kOuterY;
    out->w = kOuterW;
    out->h = kOuterH;
    return 1;
}

int
dm1_v1_movement_arrows_graphic_rect_pc34(DM1_V1_MovementArrowRectPc34 *out)
{
    if (!out) {
        return 0;
    }
    out->x = kGraphicX;
    out->y = kGraphicY;
    out->w = kGraphicW;
    out->h = kGraphicH;
    return 1;
}

int
dm1_v1_movement_arrow_zone_id_pc34(int arrow_index)
{
    if (arrow_index < 0 || arrow_index >= DM1_V1_MOVEMENT_ARROW_COUNT_PC34) {
        return 0;
    }
    return s_arrow_zone_ids[arrow_index];
}

int
dm1_v1_movement_arrow_rect_pc34(int arrow_index,
                                DM1_V1_MovementArrowRectPc34 *out)
{
    if (!out ||
        arrow_index < 0 ||
        arrow_index >= DM1_V1_MOVEMENT_ARROW_COUNT_PC34) {
        return 0;
    }
    *out = s_arrow_rects[arrow_index];
    return 1;
}

int
dm1_v1_movement_arrow_visual_mask_for_command_pc34(int command)
{
    switch (command) {
        case DM1_V1_MOVEMENT_ARROW_COMMAND_TURN_LEFT_PC34:
            return DM1_V1_MOVEMENT_ARROW_VIS_TURN_LEFT_PC34;
        case DM1_V1_MOVEMENT_ARROW_COMMAND_TURN_RIGHT_PC34:
            return DM1_V1_MOVEMENT_ARROW_VIS_TURN_RIGHT_PC34;
        case DM1_V1_MOVEMENT_ARROW_COMMAND_FORWARD_PC34:
            return DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34;
        case DM1_V1_MOVEMENT_ARROW_COMMAND_RIGHT_PC34:
            return DM1_V1_MOVEMENT_ARROW_VIS_RIGHT_PC34;
        case DM1_V1_MOVEMENT_ARROW_COMMAND_BACKWARD_PC34:
            return DM1_V1_MOVEMENT_ARROW_VIS_BACKWARD_PC34;
        case DM1_V1_MOVEMENT_ARROW_COMMAND_LEFT_PC34:
            return DM1_V1_MOVEMENT_ARROW_VIS_LEFT_PC34;
        default:
            return 0;
    }
}

int
dm1_v1_movement_arrow_visual_receipt_pc34(
    int visual_mask,
    int arrow_index,
    DM1_V1_MovementArrowVisualReceiptPc34 *out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (arrow_index < 0 ||
        arrow_index >= DM1_V1_MOVEMENT_ARROW_COUNT_PC34 ||
        (visual_mask & s_arrow_masks[arrow_index]) == 0) {
        return 0;
    }
    out->accepted = 1;
    out->arrowIndex = arrow_index;
    out->visualMask = s_arrow_masks[arrow_index];
    out->rect = s_arrow_rects[arrow_index];
    out->cueColorKind = (arrow_index == DM1_V1_MOVEMENT_ARROW_INDEX_TURN_LEFT_PC34 ||
                         arrow_index == DM1_V1_MOVEMENT_ARROW_INDEX_TURN_RIGHT_PC34)
        ? 1
        : 2;
    return 1;
}

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
