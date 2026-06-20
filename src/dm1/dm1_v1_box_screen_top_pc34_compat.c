#include "firestaff/dm1/v1/box_screen_top_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0061_ai_Graphic562_Box_ScreenTop):
 * - DATA.C:109/531/1350 - declaration + PC 3.4 init + Atari init
 * - DATA.C:109 - declaration of G0061_ai_Graphic562_Box_ScreenTop[4]
 * - DATA.C:531 - PC 3.4 init { 0, 319, 0, 32 }
 * - DATA.C:1350 - post-1.3 Atari init (same values)
 * - VIDEO.C status-row fill — M524_FillScreenBox + F0132_VIDEO_Blit
 * - VIDEO.C + IO.C — M524_FillScreenBox + F0132_VIDEO_Blit for the
 *                    top status-bar area (champion icons)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-863 (Graphics.dat init-table gates batches 1-12).
 */

enum {
    kTableSize  = 4,
    kIndexOOR   = -1
};

static const int s_g0061[kTableSize] = {
    /* 0 */ 0,    /* L (left edge) */
    /* 1 */ 319,  /* R (right edge) */
    /* 2 */ 0,    /* T (top edge) */
    /* 3 */ 32    /* B (bottom edge) */
};

const int *
dm1_v1_box_screen_top_table_pc34(void)
{
    return s_g0061;
}

int
dm1_v1_box_screen_top_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_box_screen_top_get_pc34(int value_index)
{
    if (value_index < 0 || value_index >= kTableSize) {
        return kIndexOOR;
    }
    return s_g0061[value_index];
}

int
dm1_v1_box_screen_top_run_pc34(
    DM1_V1_BoxScreenTopResultPc34 *out)
{
    int table_matches_declaration = 1;
    int left_0 = 1;
    int right_319 = 1;
    int top_0 = 1;
    int bottom_32 = 1;
    int left_lt_right = 1;
    int top_lt_bottom = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0061[i];
    }
    out->tableSize = kTableSize;

    if (s_g0061[0] != 0)   left_0 = 0;
    if (s_g0061[1] != 319) right_319 = 0;
    if (s_g0061[2] != 0)   top_0 = 0;
    if (s_g0061[3] != 32)  bottom_32 = 0;
    out->left0 = left_0;
    out->right319 = right_319;
    out->top0 = top_0;
    out->bottom32 = bottom_32;

    if (s_g0061[0] >= s_g0061[1]) left_lt_right = 0;
    if (s_g0061[2] >= s_g0061[3]) top_lt_bottom = 0;
    out->leftLtRight = left_lt_right;
    out->topLtBottom = top_lt_bottom;

    {
        static const int kExpected[kTableSize] = {0, 319, 0, 32};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0061[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_box_screen_top_get_pc34(i) != s_g0061[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_box_screen_top_get_pc34(-1) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_box_screen_top_get_pc34(kTableSize) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_box_screen_top_get_pc34(999) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->left0 &&
        out->right319 &&
        out->top0 &&
        out->bottom32 &&
        out->leftLtRight &&
        out->topLtBottom &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 10;
    return out->accepted;
}