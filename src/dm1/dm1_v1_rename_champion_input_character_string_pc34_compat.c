#include "firestaff/dm1/v1/rename_champion_input_character_string_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0052_ac_Graphic562_RenameChampionInputCharacterString):
 * - DATA.C:90  - declaration of G0052_ac_Graphic562_RenameChampionInputCharacterString[2]
 * - DATA.C:90/429/1105 - declaration + PC 3.4 init + Atari init
 * - DATA.C:429 - PC 3.4 init " " (single space + NUL)
 * - DATA.C:1105 - post-1.3 Atari init (same value)
 * - REVIVE.C:429 family — same rename-input UI as G0051
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-852 (Graphics.dat init-table gates batches 1-10). This
 * gate is a non-mirror-candidate contract for the G0052
 * rename-champion-input default-character string.
 */

enum {
    kTableSize  = 2,
    kOutOfRange = 0
};

static const char s_g0052[kTableSize] = {
    ' ',
    '\0'
};

const char *
dm1_v1_rename_champion_input_character_string_table_pc34(void)
{
    return s_g0052;
}

int
dm1_v1_rename_champion_input_character_string_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_rename_champion_input_character_string_get_pc34(int char_index)
{
    if (char_index < 0 || char_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)(unsigned char)s_g0052[char_index];
}

int
dm1_v1_rename_champion_input_character_string_run_pc34(
    DM1_V1_RenameChampionInputCharacterStringResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_char_space = 1;
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
        out->tableEntries[i] = (int)(unsigned char)s_g0052[i];
    }
    out->tableSize = kTableSize;

    if (s_g0052[0] != ' ') {
        first_char_space = 0;
    }
    out->firstCharSpace = first_char_space;

    if (s_g0052[kTableSize - 1] != '\0') {
        last_char_nul_terminator = 0;
    }
    out->lastCharNulTerminator = last_char_nul_terminator;

    {
        int found_nul = 0;
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0052[i] == '\0') {
                found_nul = 1;
            }
        }
        if (!found_nul) {
            nul_terminated = 0;
        }
    }
    out->nulTerminated = nul_terminated;

    {
        static const char kExpected[kTableSize] = {' ', '\0'};
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0052[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_rename_champion_input_character_string_get_pc34(i) !=
            (int)(unsigned char)s_g0052[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_rename_champion_input_character_string_get_pc34(-1) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_rename_champion_input_character_string_get_pc34(kTableSize) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_rename_champion_input_character_string_get_pc34(999) != kOutOfRange) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstCharSpace &&
        out->lastCharNulTerminator &&
        out->nulTerminated &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 7;
    return out->accepted;
}