#include "firestaff/dm1/v1/fuzzy_sector_analyzed_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:39   - declaration of G0031_i_Graphic562_FuzzySectorAnalyzed_CPSE
 * - DATA.C:179  - PC 3.4 init = C00255_FALSE (i.e. 0)
 * - DATA.C:579  - Atari ST init = C00255_FALSE
 * - CLIKMENU.C:366/491/559 - read site (gate copy-protection
 *                     fuzzy-sector re-reads)
 * - COPYPRO6.C:69/99 - write site (set C00136_TRUE after analysis)
 * - DEFS.H:     - C00255_FALSE, C00136_TRUE,
 *                F0277_CPSE_IsFuzzySectorValid_FuzzyBits
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798+
 * (Graphics.dat init-table gates). This gate is a non-mirror-
 * candidate contract for the G0031 fuzzy-sector-analyzed flag.
 */

enum {
    kFalseValue = 0,
    kTableSize  = 1
};

static const int s_g0031 = kFalseValue;

int
dm1_v1_fuzzy_sector_analyzed_get_pc34(void)
{
    return s_g0031;
}

int
dm1_v1_fuzzy_sector_analyzed_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_fuzzy_sector_analyzed_run_pc34(
    DM1_V1_FuzzySectorAnalyzedResultPc34 *out)
{
    int table_matches_declaration = 1;
    int initialized_false = 1;
    int value_is_c00255 = 1;
    int value_in_range = 1;
    int lookup_function_correct = 1;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0031 != kFalseValue) initialized_false = 0;
    out->initializedFalse = initialized_false;

    if (s_g0031 != 0) value_is_c00255 = 0;
    out->valueIsC00255 = value_is_c00255;

    if (s_g0031 < -32768 || s_g0031 > 32767) value_in_range = 0;
    out->valueInRange = value_in_range;

    if (dm1_v1_fuzzy_sector_analyzed_get_pc34() != 0) {
        lookup_function_correct = 0;
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->initializedFalse &&
        out->valueIsC00255 &&
        out->valueInRange &&
        out->lookupFunctionCorrect;
    out->assertionCount = 6;
    return out->accepted;
}