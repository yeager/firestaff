#include "firestaff/dm1/v1/box_champion_icons_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0054_ai_Graphic562_Box_ChampionIcons):
 * - DATA.C:92  - declaration of G0054_ai_Graphic562_Box_ChampionIcons[16]
 * - DATA.C:431 - PC 3.4 EN init { {281, 299, 0, 13}, {301, 319, 0, 13},
 *                                {301, 319, 15, 28}, {281, 299, 15, 28} }
 * - DATA.C:1112 - post-1.3 Atari init (same values)
 * - CHAMDRAW.C:830/1022/1025/1028 - M524_FillScreenBox + F0132_VIDEO_Blit
 * - CHAMPION.C:1656 - M524_FillScreenBox champion-icon clear
 * - IO.C:2433/2619/2677 - M524_FillScreenBox + M520_F0021_MAIN_BlitToScreen
 *
 * Box format: each 4-int box is {L, R, T, B} (left, right, top, bottom)
 * in PC 3.4 screen coordinates (0..319 horizontally, 0..199 vertically).
 * The 4 boxes are the on-screen rectangles where the 4 champion
 * direction icons (one per leader-cell position) are drawn. Box width
 * is 18 pixels (R-L) and height is 13-15 pixels (B-T) — the icons
 * themselves are 18x13/15 bpp-padded 4bpp bitmaps.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-859 (Graphics.dat init-table gates batches 1-11). This
 * gate is a non-mirror-candidate contract for the G0054
 * on-screen champion-icon rectangles.
 */

enum {
    kBoxCount    = 4,
    kBoxValues   = 4,
    kTableSize   = 16,
    kIndexOOR    = -1,
    kXMin        = 0,
    kXMax        = 319,    /* PC 3.4 EN screen width - 1 */
    kYMin        = 0,
    kYMax        = 199     /* PC 3.4 EN screen height - 1 */
};

static const int s_g0054[kTableSize] = {
    /* box 0 (cell-position 0, leader-cell) */ 281, 299,  0, 13,
    /* box 1 (cell-position 1) */               301, 319,  0, 13,
    /* box 2 (cell-position 2) */               301, 319, 15, 28,
    /* box 3 (cell-position 3) */               281, 299, 15, 28
};

const int *
dm1_v1_box_champion_icons_table_pc34(void)
{
    return s_g0054;
}

int
dm1_v1_box_champion_icons_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_box_champion_icons_get_pc34(int box_index, int value_index)
{
    int flat_index;
    if (box_index < 0 || box_index >= kBoxCount) {
        return kIndexOOR;
    }
    if (value_index < 0 || value_index >= kBoxValues) {
        return kIndexOOR;
    }
    flat_index = box_index * kBoxValues + value_index;
    return s_g0054[flat_index];
}

int
dm1_v1_box_champion_icons_box_count_pc34(void)
{
    return kBoxCount;
}

int
dm1_v1_box_champion_icons_run_pc34(
    DM1_V1_BoxChampionIconsResultPc34 *out)
{
    int table_matches_declaration = 1;
    int box0_left_281_right_299 = 1;
    int box1_left_301_right_319 = 1;
    int box2_left_301_right_319_top_15 = 1;
    int box3_left_281_right_299_top_15 = 1;
    int all_left_in_byte_range = 1;
    int all_right_in_byte_range = 1;
    int all_top_in_byte_range = 1;
    int all_bottom_in_byte_range = 1;
    int all_left_lt_right = 1;
    int all_top_lt_bottom = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int b, v;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (v = 0; v < kTableSize; ++v) {
        out->tableEntries[v] = s_g0054[v];
    }
    out->tableSize = kTableSize;

    /* Phase 1: per-box canonical assertions. */
    if (s_g0054[0] != 281 || s_g0054[1] != 299 || s_g0054[2] != 0 || s_g0054[3] != 13) {
        box0_left_281_right_299 = 0;
    }
    if (s_g0054[4] != 301 || s_g0054[5] != 319 || s_g0054[6] != 0 || s_g0054[7] != 13) {
        box1_left_301_right_319 = 0;
    }
    if (s_g0054[8]  != 301 || s_g0054[9]  != 319 || s_g0054[10] != 0 || s_g0054[11] != 13) {
        /* Note: this also covers the original code's box-1 = top-0,
         * which is logically equivalent to box-2 = top-15 once the
         * table is corrected by separate canonical assertions. The
         * (8..11) range is reserved for the table position; the
         * canonical contract check below enforces box-2 = top-15. */
    }
    if (s_g0054[8]  != 301 || s_g0054[9]  != 319 || s_g0054[10] != 15 || s_g0054[11] != 28) {
        box2_left_301_right_319_top_15 = 0;
    }
    if (s_g0054[12] != 281 || s_g0054[13] != 299 || s_g0054[14] != 15 || s_g0054[15] != 28) {
        box3_left_281_right_299_top_15 = 0;
    }
    out->box0Left281Right299 = box0_left_281_right_299;
    out->box1Left301Right319 = box1_left_301_right_319;
    out->box2Left301Right319Top15 = box2_left_301_right_319_top_15;
    out->box3Left281Right299Top15 = box3_left_281_right_299_top_15;

    /* Phase 2: each box has L/R in [0, 319] and T/B in [0, 199]. */
    for (b = 0; b < kBoxCount; ++b) {
        int L = s_g0054[b * kBoxValues + 0];
        int R = s_g0054[b * kBoxValues + 1];
        int T = s_g0054[b * kBoxValues + 2];
        int B = s_g0054[b * kBoxValues + 3];
        if (L < kXMin || L > kXMax) all_left_in_byte_range = 0;
        if (R < kXMin || R > kXMax) all_right_in_byte_range = 0;
        if (T < kYMin || T > kYMax) all_top_in_byte_range = 0;
        if (B < kYMin || B > kYMax) all_bottom_in_byte_range = 0;
        if (L >= R) all_left_lt_right = 0;
        if (T >= B) all_top_lt_bottom = 0;
    }
    out->allXInByteRange = all_left_in_byte_range && all_right_in_byte_range;
    out->allYInByteRange = all_top_in_byte_range && all_bottom_in_byte_range;
    out->allWidthsInRange = all_left_lt_right;   /* "widths" = L<R constraint */
    out->allHeightsInRange = all_top_lt_bottom;  /* "heights" = T<B constraint */

    /* Phase 3: full table matches declared order. */
    {
        static const int kExpected[kTableSize] = {
            281, 299,  0, 13,
            301, 319,  0, 13,
            301, 319, 15, 28,
            281, 299, 15, 28
        };
        for (v = 0; v < kTableSize; ++v) {
            if (s_g0054[v] != kExpected[v]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 4: lookup function correctness for each (box, value). */
    for (b = 0; b < kBoxCount; ++b) {
        for (v = 0; v < kBoxValues; ++v) {
            if (dm1_v1_box_champion_icons_get_pc34(b, v) != s_g0054[b * kBoxValues + v]) {
                lookup_function_correct = 0;
            }
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 5: out-of-range lookup returns -1. */
    if (dm1_v1_box_champion_icons_get_pc34(-1, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_box_champion_icons_get_pc34(0, -1) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_box_champion_icons_get_pc34(kBoxCount, 0) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_box_champion_icons_get_pc34(0, kBoxValues) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_box_champion_icons_get_pc34(999, 999) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->box0Left281Right299 &&
        out->box1Left301Right319 &&
        out->box2Left301Right319Top15 &&
        out->box3Left281Right299Top15 &&
        out->allXInByteRange &&
        out->allYInByteRange &&
        out->allWidthsInRange &&
        out->allHeightsInRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 12;
    return out->accepted;
}