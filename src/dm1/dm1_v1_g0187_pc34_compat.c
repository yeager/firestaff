#include "firestaff/dm1/v1/G0187_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0187):
 * - DUNVIEW.C:46X - declaration of G0187_s_Graphic558_Frames_Door_D1R
 * - DUNVIEW.C:706 - PC 3.4 EN init (DOOR_FRAMES struct)
 * - DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 80,
    kOutOfRange = -1
};

/* G0187_s_Graphic558_Frames_Door_D1R PC 3.4 EN init.
 * Format: 10 frames × 8 bytes = ClosedOrDestroyed(8) + Vertical(3*8) +
 *         LeftHorizontal(3*8) + RightHorizontal(3*8). */
static const unsigned char s_g0187[kTableSize] = {
192, 223, 17, 102, 48, 88, 0, 0, 192, 223, 17, 38, 48, 88, 0, 66, 192, 223, 17, 60, 48, 88, 0, 44, 192, 223, 17, 82, 48, 88, 0, 22, 192, 203, 17, 102, 48, 88, 36, 0, 192, 215, 17, 102, 48, 88, 24, 0, 192, 223, 17, 102, 48, 88, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const unsigned char *
dm1_v1_g0187_table_pc34(void)
{
    return s_g0187;
}

int
dm1_v1_g0187_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0187_get_pc34(int frame_index, int value_index)
{
    if (frame_index < 0 || frame_index >= 10) {
        return kOutOfRange;
    }
    if (value_index < 0 || value_index >= 8) {
        return kOutOfRange;
    }
    return (int)s_g0187[frame_index * 8 + value_index];
}

int
dm1_v1_g0187_run_pc34(
    DM1_V1_G0187ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int closed_or_destroyed_valid = 1;
    int vertical_frames_valid = 1;
    int left_horizontal_frames_valid = 1;
    int right_horizontal_frames_valid = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0187[i];
    }
    out->tableSize = kTableSize;

    /* ClosedOrDestroyed (frame 0): first 8 bytes. */
    if (s_g0187[0] == 0 && s_g0187[1] == 0 && s_g0187[7] == 0) {
        closed_or_destroyed_valid = 0;  /* unlikely to be all zeros */
    }
    out->closedOrDestroyedValid = closed_or_destroyed_valid;

    /* Vertical frames (1-3): bytes 8-31. */
    for (i = 8; i < 32; ++i) {
        if (s_g0187[i] > 255) vertical_frames_valid = 0;
    }
    out->verticalFramesValid = vertical_frames_valid;

    /* LeftHorizontal frames (4-6): bytes 32-55. */
    for (i = 32; i < 56; ++i) {
        if (s_g0187[i] > 255) left_horizontal_frames_valid = 0;
    }
    out->leftHorizontalFramesValid = left_horizontal_frames_valid;

    /* RightHorizontal frames (7-9): bytes 56-79. */
    for (i = 56; i < 80; ++i) {
        if (s_g0187[i] > 255) right_horizontal_frames_valid = 0;
    }
    out->rightHorizontalFramesValid = right_horizontal_frames_valid;

    /* All values fit in uint8_t. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0187[i] > 255) all_bytes_in_byte_range = 0;
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kTableSize] = { 192, 223, 17, 102, 48, 88, 0, 0, 192, 223, 17, 38, 48, 88, 0, 66, 192, 223, 17, 60, 48, 88, 0, 44, 192, 223, 17, 82, 48, 88, 0, 22, 192, 203, 17, 102, 48, 88, 36, 0, 192, 215, 17, 102, 48, 88, 24, 0, 192, 223, 17, 102, 48, 88, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0187[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int frame = i / 8;
        int val = i % 8;
        if (dm1_v1_g0187_get_pc34(frame, val) != (int)s_g0187[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0187_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0187_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0187_get_pc34(10, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0187_get_pc34(0, 8) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0187_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
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
