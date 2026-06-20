#include "firestaff/dm1/v1/reincarnate_special_characters_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0053_ac_Graphic562_ReincarnateSpecialCharacters):
 * - DATA.C:91  - declaration of G0053_ac_Graphic562_ReincarnateSpecialCharacters[6]
 * - DATA.C:91/430/1111 - declaration + PC 3.4 init + Atari init
 * - DATA.C:430 - PC 3.4 init { ',', '.', ';', ':', ' ' } + NUL terminator at index 5
 * - DATA.C:1111 - post-1.3 Atari init (same values)
 * - REVIVE.C family — used by reincarnate-UI punctuation handling
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-852 (Graphics.dat init-table gates batches 1-10). This
 * gate is a non-mirror-candidate contract for the G0053
 * reincarnate-special-characters array.
 */

enum {
    kTableSize  = 6,
    kOutOfRange = 0
};

static const char s_g0053[kTableSize] = {
    ',',
    '.',
    ';',
    ':',
    ' ',
    '\0'
};

const char *
dm1_v1_reincarnate_special_characters_table_pc34(void)
{
    return s_g0053;
}

int
dm1_v1_reincarnate_special_characters_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_reincarnate_special_characters_get_pc34(int char_index)
{
    if (char_index < 0 || char_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)(unsigned char)s_g0053[char_index];
}

int
dm1_v1_reincarnate_special_characters_run_pc34(
    DM1_V1_ReincarnateSpecialCharactersResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_char_comma = 1;
    int second_char_period = 1;
    int third_char_semicolon = 1;
    int fourth_char_colon = 1;
    int fifth_char_space = 1;
    int sixth_char_nul_terminator = 1;
    int nul_terminated = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)(unsigned char)s_g0053[i];
    }
    out->tableSize = kTableSize;

    if (s_g0053[0] != ',') first_char_comma = 0;
    if (s_g0053[1] != '.') second_char_period = 0;
    if (s_g0053[2] != ';') third_char_semicolon = 0;
    if (s_g0053[3] != ':') fourth_char_colon = 0;
    if (s_g0053[4] != ' ') fifth_char_space = 0;
    if (s_g0053[kTableSize - 1] != '\0') sixth_char_nul_terminator = 0;
    out->firstCharComma = first_char_comma;
    out->secondCharPeriod = second_char_period;
    out->thirdCharSemicolon = third_char_semicolon;
    out->fourthCharColon = fourth_char_colon;
    out->fifthCharSpace = fifth_char_space;
    out->sixthCharNulTerminator = sixth_char_nul_terminator;

    {
        int found_nul = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0053[i] == '\0') {
                found_nul = 1;
            }
        }
        if (!found_nul) {
            nul_terminated = 0;
        }
    }
    out->nulTerminated = nul_terminated;

    {
        static const char kExpected[kTableSize] = {',', '.', ';', ':', ' ', '\0'};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0053[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_reincarnate_special_characters_get_pc34(i) !=
            (int)(unsigned char)s_g0053[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_reincarnate_special_characters_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_reincarnate_special_characters_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_reincarnate_special_characters_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstCharComma &&
        out->secondCharPeriod &&
        out->thirdCharSemicolon &&
        out->fourthCharColon &&
        out->fifthCharSpace &&
        out->sixthCharNulTerminator &&
        out->nulTerminated &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 11;
    return out->accepted;
}