#include "firestaff/dm1/v1/box_screen_bottom_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0063_ai_Graphic562_Box_*):
 * - DATA.C:111/533/1352 - declaration + PC 3.4 init + Atari init
 * - DATA.C:111 - declaration
 * - DATA.C:533 - PC 3.4 init { 0, 319, 169, 199 }
 * - DATA.C:1352 - post-1.3 Atari init (same values)
 * - VIDEO.C status-row fill — M524_FillScreenBox + F0132_VIDEO_Blit
 *
 * Disjoint from pass784-790 + pass791-799 + pass798-863.
 * - G0063_ai_Graphic562_Box_ScreenBottom — see DATA.C reference
 */

/* Variable: G0063_ai_Graphic562_Box_ScreenBottom */
enum {
    kTableSize  = 4,
    kIndexOOR   = -1
};

static const int s_g0063[kTableSize] = {
    /* 0 */ 0,  /* L */
    /* 1 */ 319,  /* R */
    /* 2 */ 169,  /* T */
    /* 3 */ 199   /* B */
};

const int *
dm1_v1_box_screen_bottom_table_pc34(void)
{
    return s_g0063;
}

int
dm1_v1_box_screen_bottom_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_box_screen_bottom_get_pc34(int value_index)
{
    if (value_index < 0 || value_index >= kTableSize) {
        return kIndexOOR;
    }
    return s_g0063[value_index];
}

int
dm1_v1_box_screen_bottom_run_pc34(
    DM1_V1_BOX_SCREEN_BOTTOMResultPc34 *out)
{
    int table_matches_declaration = 1;
    int left_ok = 1;
    int right_ok = 1;
    int top_ok = 1;
    int bottom_ok = 1;
    int left_lt_right = 1;
    int top_lt_bottom = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0063[i];
    }
    out->tableSize = kTableSize;

    if (s_g0063[0] != 0) left_ok = 0;
    if (s_g0063[1] != 319) right_ok = 0;
    if (s_g0063[2] != 169) top_ok = 0;
    if (s_g0063[3] != 199) bottom_ok = 0;
    out->leftOk = left_ok;
    out->rightOk = right_ok;
    out->topOk = top_ok;
    out->bottomOk = bottom_ok;

    if (s_g0063[0] >= s_g0063[1]) left_lt_right = 0;
    if (s_g0063[2] >= s_g0063[3]) top_lt_bottom = 0;
    out->leftLtRight = left_lt_right;
    out->topLtBottom = top_lt_bottom;

    {
        static const int kExpected[kTableSize] = { 0, 319, 169, 199 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0063[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_box_screen_bottom_get_pc34(i) != s_g0063[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_box_screen_bottom_get_pc34(-1) != kIndexOOR) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_box_screen_bottom_get_pc34(kTableSize) != kIndexOOR) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_box_screen_bottom_get_pc34(999) != kIndexOOR) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->leftOk &&
        out->rightOk &&
        out->topOk &&
        out->bottomOk &&
        out->leftLtRight &&
        out->topLtBottom &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 11;
    return out->accepted;
}
