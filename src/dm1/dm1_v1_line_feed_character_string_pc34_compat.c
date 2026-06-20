#include "firestaff/dm1/v1/line_feed_character_string_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0066_ac_Graphic562_LineFeedCharacterString):
 * - DATA.C:114/536 - declaration + PC 3.4 init
 * - DATA.C:114 - declaration
 * - DATA.C:536 - PC 3.4 init "\n"
 * - F0040_TEXT_Print newline
 * - DATA.C:1356 - post-1.3 Atari init (same value)
 * - F0040_TEXT_Print - newline character for text rendering
 *
 * Disjoint from pass784-790 + pass791-799 + pass798-863.
 */

enum {
    kTableSize  = 2,
    kOutOfRange = 0
};

static const char s_g0066[kTableSize] = {
    '\n',
    '\0'
};

const char *
dm1_v1_line_feed_character_string_table_pc34(void)
{
    return s_g0066;
}

int
dm1_v1_line_feed_character_string_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_line_feed_character_string_get_pc34(int char_index)
{
    if (char_index < 0 || char_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)(unsigned char)s_g0066[char_index];
}

int
dm1_v1_line_feed_character_string_run_pc34(
    DM1_V1_LINE_FEED_CHARACTER_STRINGResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_char_newline = 1;
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
        out->tableEntries[i] = (int)(unsigned char)s_g0066[i];
    }
    out->tableSize = kTableSize;

    if (s_g0066[0] != '\n') first_char_newline = 0;
    if (s_g0066[kTableSize - 1] != '\0') last_char_nul_terminator = 0;
    out->firstCharNewline = first_char_newline;
    out->lastCharNulTerminator = last_char_nul_terminator;

    {
        int found_nul = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0066[i] == '\0') {
                found_nul = 1;
            }
        }
        if (!found_nul) {
            nul_terminated = 0;
        }
    }
    out->nulTerminated = nul_terminated;

    {
        static const char kExpected[kTableSize] = {'\n', '\0'};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0066[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_line_feed_character_string_get_pc34(i) != (int)(unsigned char)s_g0066[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_line_feed_character_string_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_line_feed_character_string_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_line_feed_character_string_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstCharNewline &&
        out->lastCharNulTerminator &&
        out->nulTerminated &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 7;
    return out->accepted;
}
