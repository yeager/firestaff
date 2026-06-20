#include "firestaff/dm1/v1/G0487_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0487):
 * - MENU.C:18 - declaration of G0487_as_Graphic560_Spells[M530_SPELL_COUNT]
 * - MENU.C:50-86 - PC 3.4 EN init (25 spell entries)
 * - MENU.C F0452/F0456/F0412 - read sites
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-790.
 */

enum {
    kTableSize  = 200,
    kOutOfRange = -1
};

/* G0487_as_Graphic560_Spells[25] PC 3.4 EN init (MENU.C:50-86).
 * 25 entries × 8 bytes each = 200 bytes. Byte layout (little-endian):
 *   bytes 0-3: Symbols (uint32_t, big-endian)
 *   byte 4: BaseRequiredSkillLevel (uint8_t)
 *   byte 5: SkillIndex (uint8_t)
 *   bytes 6-7: Attributes (uint16_t, big-endian) */
static const unsigned char s_g0487[kTableSize] = {
0, 102, 111, 0, 2, 15, 120, 67, 0, 102, 112, 115, 1, 18, 72, 99, 0, 104, 109, 119, 3, 17, 180, 51, 0, 104, 108, 0, 3, 19, 108, 114, 0, 104, 109, 118, 3, 18, 132, 35, 0, 104, 110, 118, 4, 17, 120, 34, 0, 104, 111, 118, 4, 17, 88, 3, 0, 105, 0, 0, 1, 16, 60, 83, 0, 105, 111, 0, 3, 16, 168, 2, 0, 105, 112, 114, 4, 13, 60, 113, 0, 105, 112, 117, 4, 15, 112, 131, 0, 106, 109, 0, 1, 18, 80, 50, 0, 106, 108, 0, 1, 19, 64, 98, 0, 106, 111, 119, 1, 15, 48, 19, 0, 107, 0, 0, 1, 17, 60, 66, 0, 102, 112, 0, 2, 15, 100, 193, 0, 102, 0, 0, 2, 13, 60, 177, 0, 102, 112, 116, 4, 13, 60, 129, 0, 102, 112, 117, 4, 13, 60, 145, 0, 103, 0, 0, 1, 13, 128, 225, 0, 103, 112, 0, 1, 13, 104, 161, 0, 104, 112, 115, 4, 13, 60, 97, 0, 107, 112, 118, 3, 2, 252, 209, 0, 107, 108, 0, 2, 19, 120, 49, 0, 107, 110, 118, 0, 3, 60, 115
};

const unsigned char *
dm1_v1_g0487_table_pc34(void)
{
    return s_g0487;
}

int
dm1_v1_g0487_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_g0487_get_pc34(int spell_index, int byte_offset)
{
    if (spell_index < 0 || spell_index >= 25) {
        return kOutOfRange;
    }
    if (byte_offset < 0 || byte_offset >= 8) {
        return kOutOfRange;
    }
    return (int)s_g0487[spell_index * 8 + byte_offset];
}

int
dm1_v1_g0487_run_pc34(
    DM1_V1_G0487ResultPc34 *out)
{
    int table_matches_declaration = 1;
    int spell_0_shield_symbols = 1;
    int all_skills_in_valid_range = 1;
    int all_base_req_in_byte_range = 1;
    int all_attrs_in_uint16_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_g0487[i];
    }
    out->tableSize = kTableSize;

    /* Spell 0 (Shield): Symbols = 0x00666F00. */
    if (s_g0487[0] != 0x00 || s_g0487[1] != 0x66 || s_g0487[2] != 0x6F || s_g0487[3] != 0x00) {
        spell_0_shield_symbols = 0;
    }
    out->spell0ShieldSymbols = spell_0_shield_symbols;

    /* All SkillIndex values in [0, 19]. */
    for (i = 5; i < kTableSize; i += 8) {
        if (s_g0487[i] > 19) all_skills_in_valid_range = 0;
    }
    out->allSkillsInValidRange = all_skills_in_valid_range;

    /* All BaseRequiredSkillLevel values in [0, 4]. */
    for (i = 4; i < kTableSize; i += 8) {
        if (s_g0487[i] > 4) all_base_req_in_byte_range = 0;
    }
    out->allBaseReqInByteRange = all_base_req_in_byte_range;

    /* All Attributes values in uint16_t range. */
    for (i = 6; i < kTableSize; i += 8) {
        if (s_g0487[i] > 255 || s_g0487[i + 1] > 255) all_attrs_in_uint16_range = 0;
    }
    out->allAttrsInUint16Range = all_attrs_in_uint16_range;

    {
        static const unsigned char kExpected[kTableSize] = { 0, 102, 111, 0, 2, 15, 120, 67, 0, 102, 112, 115, 1, 18, 72, 99, 0, 104, 109, 119, 3, 17, 180, 51, 0, 104, 108, 0, 3, 19, 108, 114, 0, 104, 109, 118, 3, 18, 132, 35, 0, 104, 110, 118, 4, 17, 120, 34, 0, 104, 111, 118, 4, 17, 88, 3, 0, 105, 0, 0, 1, 16, 60, 83, 0, 105, 111, 0, 3, 16, 168, 2, 0, 105, 112, 114, 4, 13, 60, 113, 0, 105, 112, 117, 4, 15, 112, 131, 0, 106, 109, 0, 1, 18, 80, 50, 0, 106, 108, 0, 1, 19, 64, 98, 0, 106, 111, 119, 1, 15, 48, 19, 0, 107, 0, 0, 1, 17, 60, 66, 0, 102, 112, 0, 2, 15, 100, 193, 0, 102, 0, 0, 2, 13, 60, 177, 0, 102, 112, 116, 4, 13, 60, 129, 0, 102, 112, 117, 4, 13, 60, 145, 0, 103, 0, 0, 1, 13, 128, 225, 0, 103, 112, 0, 1, 13, 104, 161, 0, 104, 112, 115, 4, 13, 60, 97, 0, 107, 112, 118, 3, 2, 252, 209, 0, 107, 108, 0, 2, 19, 120, 49, 0, 107, 110, 118, 0, 3, 60, 115 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0487[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    for (i = 0; i < kTableSize; ++i) {
        int spell = i / 8;
        int off = i % 8;
        if (dm1_v1_g0487_get_pc34(spell, off) != (int)s_g0487[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    if (dm1_v1_g0487_get_pc34(-1, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0487_get_pc34(0, -1) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0487_get_pc34(25, 0) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0487_get_pc34(0, 8) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    if (dm1_v1_g0487_get_pc34(999, 999) != kOutOfRange) lookup_out_of_range_returns_minus_one = 0;
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->spell0ShieldSymbols &&
        out->allSkillsInValidRange &&
        out->allBaseReqInByteRange &&
        out->allAttrsInUint16Range &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 8;
    return out->accepted;
}
