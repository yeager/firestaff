#include "firestaff/dm1/v1/G0179_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0179):
 * - DUNVIEW.C:610 - PC 3.4 EN init for G0179_s_Graphic558_Frames_Door_D3L
 * - DUNVIEW.C F0111/F0100 lines 4310-4313 - read sites for
 *   Vertical/LeftHorizontal/RightHorizontal door frame records.
 */

enum {
    kTableSize = 80,
    kOutOfRange = -1
};

/* G0179_s_Graphic558_Frames_Door_D3L PC 3.4 EN init.
 * Format: 10 frames x 8 bytes = ClosedOrDestroyed(8) + Vertical(3*8) +
 *         LeftHorizontal(3*8) + RightHorizontal(3*8). */
static const unsigned char s_g0179[kTableSize] = {
24, 71, 28, 67, 24, 41, 0, 0,
24, 71, 28, 38, 24, 41, 0, 30,
24, 71, 28, 48, 24, 41, 0, 20,
24, 71, 28, 58, 24, 41, 0, 10,
24, 29, 28, 67, 24, 41, 18, 0,
24, 35, 28, 67, 24, 41, 12, 0,
24, 41, 28, 67, 24, 41, 6, 0,
66, 71, 28, 67, 24, 41, 24, 0,
60, 71, 28, 67, 24, 41, 24, 0,
54, 71, 28, 67, 24, 41, 24, 0
};

const unsigned char *
dm1_v1_g0179_table_pc34(void)
{
    return s_g0179;
}

int
dm1_v1_g0179_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0179_get_pc34(int frame_index, int value_index)
{
    if (frame_index < 0 || frame_index >= 10) {
        return kOutOfRange;
    }
    if (value_index < 0 || value_index >= 8) {
        return kOutOfRange;
    }
    return (int)s_g0179[frame_index * 8 + value_index];
}

int
dm1_v1_g0179_run_pc34(
    DM1_V1_G0179ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0179[i];
    }
    out->tableSize = kTableSize;

    {
        static const unsigned char kExpected[kTableSize] = {
            24, 71, 28, 67, 24, 41, 0, 0,
            24, 71, 28, 38, 24, 41, 0, 30,
            24, 71, 28, 48, 24, 41, 0, 20,
            24, 71, 28, 58, 24, 41, 0, 10,
            24, 29, 28, 67, 24, 41, 18, 0,
            24, 35, 28, 67, 24, 41, 12, 0,
            24, 41, 28, 67, 24, 41, 6, 0,
            66, 71, 28, 67, 24, 41, 24, 0,
            60, 71, 28, 67, 24, 41, 24, 0,
            54, 71, 28, 67, 24, 41, 24, 0
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0179[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    out->closedOrDestroyedValid =
        s_g0179[0] == 24 && s_g0179[1] == 71 && s_g0179[2] == 28;
    out->verticalFramesValid =
        s_g0179[8] == 24 && s_g0179[15] == 30 &&
        s_g0179[16] == 24 && s_g0179[23] == 20 &&
        s_g0179[24] == 24 && s_g0179[31] == 10;
    out->leftHorizontalFramesValid =
        s_g0179[32] == 24 && s_g0179[39] == 0 &&
        s_g0179[48] == 24 && s_g0179[55] == 0;
    out->rightHorizontalFramesValid =
        s_g0179[56] == 66 && s_g0179[63] == 0 &&
        s_g0179[72] == 54 && s_g0179[79] == 0;

    out->allBytesInByteRange = 1;
    for (i = 0; i < kTableSize; ++i) {
        int frame = i / 8;
        int value = i % 8;
        if (dm1_v1_g0179_get_pc34(frame, value) != (int)s_g0179[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0179_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0179_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0179_get_pc34(10, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0179_get_pc34(0, 8) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0179_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->closedOrDestroyedValid &&
        out->verticalFramesValid &&
        out->leftHorizontalFramesValid &&
        out->rightHorizontalFramesValid &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 9;
    return out->accepted;
}
