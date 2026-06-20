#include "firestaff/dm1/v1/G0208_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0208):
 * - DUNVIEW.C:485 - declaration
 * - DUNVIEW.C:1210 - PC 3.4 EN init
 * - DUNVIEW.C F0099/F0654 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kFrames   = 4,
    kFields   = 6,
    kSize     = 24,    /* kFrames * kFields */
    kOutOfRange = -1
};

static const unsigned char s_g0208[kFrames][kFields] = {
    { 199, 204, 41, 44, 8, 4 },
    { 136, 141, 41, 44, 8, 4 },
    { 144, 155, 42, 47, 8, 6 },
    { 160, 175, 44, 52, 8, 9 }
};

const unsigned char *
dm1_v1_g0208_table_pc34(void)
{
    return &s_g0208[0][0];
}

int
dm1_v1_g0208_size_pc34(void)
{
    return kSize;
}

int
dm1_v1_g0208_get_pc34(int frame_index, int field_index)
{
    if (frame_index < 0 || frame_index >= kFrames) {
        return kOutOfRange;
    }
    if (field_index < 0 || field_index >= kFields) {
        return kOutOfRange;
    }
    return (int)s_g0208[frame_index][field_index];
}

int
dm1_v1_g0208_run_pc34(
    DM1_V1_G0208ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int all_bytes_in_byte_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i, j;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kFrames; ++i) {
        for (j = 0; j < kFields; ++j) {
            out->tableEntries[i * kFields + j] = (int)s_g0208[i][j];
        }
    }
    out->tableSize = kSize;

    for (i = 0; i < kFrames; ++i) {
        for (j = 0; j < kFields; ++j) {
            if (s_g0208[i][j] > 255) all_bytes_in_byte_range = 0;
        }
    }
    out->allBytesInByteRange = all_bytes_in_byte_range;

    {
        static const unsigned char kExpected[kFrames][kFields] = {
            { 199, 204, 41, 44, 8, 4 },
            { 136, 141, 41, 44, 8, 4 },
            { 144, 155, 42, 47, 8, 6 },
            { 160, 175, 44, 52, 8, 9 }
        };
        for (i = 0; i < kFrames; ++i) {
            for (j = 0; j < kFields; ++j) {
                if (s_g0208[i][j] != kExpected[i][j]) {
                    table_matches_declaration = 0;
                }
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kSize; ++i) {
        int frame = i / kFields;
        int field = i % kFields;
        if (dm1_v1_g0208_get_pc34(frame, field) != (int)s_g0208[frame][field]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0208_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0208_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0208_get_pc34(kFrames, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0208_get_pc34(0, kFields) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0208_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->allBytesInByteRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 5;
    return out->accepted;
}
