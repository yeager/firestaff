#include "firestaff/dm1/v1/underscore_character_string_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0051_ac_Graphic562_UnderscoreCharacterString):
 * - DATA.C:89  - declaration of G0051_ac_Graphic562_UnderscoreCharacterString[2]
 * - DATA.C:89/428/1104 - declaration + PC 3.4 init + Atari init
 * - DATA.C:428 - PC 3.4 init "_" (underscore + NUL)
 * - DATA.C:1104 - post-1.3 Atari init (same value)
 * - REVIVE.C:429 - M521_F0040_TEXT_Print underscore-character blit
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-852 (Graphics.dat init-table gates batches 1-10). This
 * gate is a non-mirror-candidate contract for the G0051
 * underscore-character string.
 */

enum {
    kTableSize  = 2,
    kOutOfRange = 0
};

static const char s_g0051[kTableSize] = {
    '_',
    '\0'
};

const char *
dm1_v1_underscore_character_string_table_pc34(void)
{
    return s_g0051;
}

int
dm1_v1_underscore_character_string_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_underscore_character_string_get_pc34(int char_index)
{
    if (char_index < 0 || char_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)(unsigned char)s_g0051[char_index];
}

int
dm1_v1_underscore_character_string_run_pc34(
    DM1_V1_UnderscoreCharacterStringResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_char_underscore = 1;
    int last_char_nul_terminator = 1;
    int nul_terminated = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)(unsigned char)s_g0051[i];
    }
    out->tableSize = kTableSize;

    /* Phase 1: first char is underscore (0x5F). */
    if (s_g0051[0] != '_') {
        first_char_underscore = 0;
    }
    out->firstCharUnderscore = first_char_underscore;

    /* Phase 2: last char is NUL terminator. */
    if (s_g0051[kTableSize - 1] != '\0') {
        last_char_nul_terminator = 0;
    }
    out->lastCharNulTerminator = last_char_nul_terminator;

    /* Phase 3: full string is NUL-terminated. */
    {
        int found_nul = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0051[i] == '\0') {
                found_nul = 1;
            }
        }
        if (!found_nul) {
            nul_terminated = 0;
        }
    }
    out->nulTerminated = nul_terminated;

    /* Phase 4: table matches declared order. */
    {
        static const char kExpected[kTableSize] = {'_', '\0'};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0051[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 5: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_underscore_character_string_get_pc34(i) !=
            (int)(unsigned char)s_g0051[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 6: out-of-range lookup returns 0. */
    if (dm1_v1_underscore_character_string_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_underscore_character_string_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_underscore_character_string_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstCharUnderscore &&
        out->lastCharNulTerminator &&
        out->nulTerminated &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 7;
    return out->accepted;
}