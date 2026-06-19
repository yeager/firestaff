#include "firestaff/dm1/v1/box_title_strikes_back_source_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:10   - declaration of G0004_ai_Graphic562_Box_Title_StrikesBack_Source[4]
 * - DATA.C:127  - PC 3.4 init { 0, 319, 0, 56 }
 * - DATA.C:547  - Atari ST init (same values)
 * - TITLE.C:134/137/335/338 - F0132_VIDEO_Blit Strikes Back source
 * - DEFS.H:     - C160_BYTE_WIDTH_SCREEN, CM1_COLOR_NO_TRANSPARENCY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835 (Graphics.dat init-table gates
 * batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14). This gate is a
 * non-mirror-candidate contract for the G0004 Strikes-Back title
 * source box.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kSBSX      = 0,
    kSBSY      = 319,
    kSBSW      = 0,
    kSBSH      = 56
};

static const int s_g0004[4] = { 0, 319, 0, 56 };

const int *
dm1_v1_box_title_strikes_back_source_table_pc34(void)
{
    return s_g0004;
}

int
dm1_v1_box_title_strikes_back_source_size_pc34(void)
{
    return DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_title_strikes_back_source_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0004[component];
    return 1;
}

int
dm1_v1_box_title_strikes_back_source_x_pc34(void) { return s_g0004[kBoxX]; }
int
dm1_v1_box_title_strikes_back_source_y_pc34(void) { return s_g0004[kBoxY]; }
int
dm1_v1_box_title_strikes_back_source_w_pc34(void) { return s_g0004[kBoxW]; }
int
dm1_v1_box_title_strikes_back_source_h_pc34(void) { return s_g0004[kBoxH]; }

int
dm1_v1_box_title_strikes_back_source_run_pc34(
    DM1_V1_BoxTitleStrikesBackSourceResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_0 = 1;
    int y_is_319 = 1;
    int w_is_0 = 1;
    int h_is_56 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 0, 319, 0, 56 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0004[i];
        if (s_g0004[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0004[kBoxX] != kSBSX) x_is_0 = 0;
    if (s_g0004[kBoxY] != kSBSY) y_is_319 = 0;
    if (s_g0004[kBoxW] != kSBSW) w_is_0 = 0;
    if (s_g0004[kBoxH] != kSBSH) h_is_56 = 0;
    out->xIs0  = x_is_0;
    out->yIs319 = y_is_319;
    out->wIs0  = w_is_0;
    out->hIs56 = h_is_56;

    for (i = 0; i < DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_SIZE; ++i) {
        if (s_g0004[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Width is allowed to be 0 (per the G0047 source-init
     * convention "no width offset"). Height must be positive.
     */
    if (s_g0004[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = 1;  /* always pass per convention */
    out->heightPositive = height_positive;

    if (s_g0004[kBoxY] < 0 || s_g0004[kBoxY] > 320) within_row_range = 0;
    out->withinRowRange = within_row_range;

    /* X=0, W=0 → end=0 (always well within bounds). */
    if (s_g0004[kBoxX] + s_g0004[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs0 &&
        out->yIs319 &&
        out->wIs0 &&
        out->hIs56 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}