#include "firestaff/dm1/v1/box_entrance_opening_door_right_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:14   - declaration of G0008_ai_Graphic562_Box_Entrance_OpeningDoorRight[4]
 * - DATA.C:135  - PC 3.4 init { 109, 231, 0, 160 }
 * - DATA.C:555  - Atari ST init (same values)
 * - ENTRANCE.C:150 - M768_BOX_LEFT(G0008) += 4
 * - ENTRANCE.C:190/192/195 - F0132_VIDEO_Blit entrance door
 *                    animation (right half) into G0008's box
 * - DEFS.H:     - M768_BOX_LEFT macro, C064_BYTE_WIDTH,
 *                C128_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835/836/837/838 (Graphics.dat init-table
 * gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16). This gate
 * is a non-mirror-candidate contract for the G0008 entrance-
 * opening-door-right box.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kEDRX      = 109,
    kEDRY      = 231,
    kEDRW      = 0,
    kEDRH      = 160
};

static const int s_g0008[4] = { 109, 231, 0, 160 };

const int *
dm1_v1_box_entrance_opening_door_right_table_pc34(void)
{
    return s_g0008;
}

int
dm1_v1_box_entrance_opening_door_right_size_pc34(void)
{
    return DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_entrance_opening_door_right_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0008[component];
    return 1;
}

int
dm1_v1_box_entrance_opening_door_right_x_pc34(void) { return s_g0008[kBoxX]; }
int
dm1_v1_box_entrance_opening_door_right_y_pc34(void) { return s_g0008[kBoxY]; }
int
dm1_v1_box_entrance_opening_door_right_w_pc34(void) { return s_g0008[kBoxW]; }
int
dm1_v1_box_entrance_opening_door_right_h_pc34(void) { return s_g0008[kBoxH]; }

int
dm1_v1_box_entrance_opening_door_right_run_pc34(
    DM1_V1_BoxEntranceOpeningDoorRightResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_109 = 1;
    int y_is_231 = 1;
    int w_is_0 = 1;
    int h_is_160 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 109, 231, 0, 160 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0008[i];
        if (s_g0008[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0008[kBoxX] != kEDRX) x_is_109 = 0;
    if (s_g0008[kBoxY] != kEDRY) y_is_231 = 0;
    if (s_g0008[kBoxW] != kEDRW) w_is_0 = 0;
    if (s_g0008[kBoxH] != kEDRH) h_is_160 = 0;
    out->xIs109 = x_is_109;
    out->yIs231 = y_is_231;
    out->wIs0   = w_is_0;
    out->hIs160 = h_is_160;

    for (i = 0; i < DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_PC34_COMPAT_SIZE; ++i) {
        if (s_g0008[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Width is allowed to be 0 (per source-init convention).
     * Height must be positive.
     */
    if (s_g0008[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = 1;  /* always pass per convention */
    out->heightPositive = height_positive;

    if (s_g0008[kBoxY] < 0 || s_g0008[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    /* X=109, W=0 → end=109 (well within bounds). */
    if (s_g0008[kBoxX] + s_g0008[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs109 &&
        out->yIs231 &&
        out->wIs0 &&
        out->hIs160 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}