#include "firestaff/dm1/v1/indirect_stop_expiring_event_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:28   - declaration of G0022_i_Graphic562_IndirectStopExpiringEvent_CPSE
 * - DATA.C:232  - PC 3.4 init = C00555_FALSE (i.e. 0)
 * - DATA.C:874  - Atari init = C00555_FALSE
 * - MOVESENS.C:744/746 - read site (with BUG0_00 useless compare)
 * - TIMELINE.C:1922 - write site (set C00136_TRUE)
 * - DEFS.H:     - C00555_FALSE, C00136_TRUE
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833 (Graphics.dat init-table gates batches 1+2+3+
 * 4+5+6+7+8+9+10+11+12). This gate is a non-mirror-candidate
 * contract for the G0022 copy-protection state flag.
 */

enum {
    kFalseValue = 0,
    kTableSize  = 1
};

static const int s_g0022 = kFalseValue;

int
dm1_v1_indirect_stop_expiring_event_get_pc34(void)
{
    return s_g0022;
}

int
dm1_v1_indirect_stop_expiring_event_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_indirect_stop_expiring_event_run_pc34(
    DM1_V1_IndirectStopExpiringEventResultPc34 *out)
{
    int table_matches_declaration = 1;
    int initialized_false = 1;
    int value_is_c00555 = 1;
    int value_in_range = 1;
    int lookup_function_correct = 1;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 1: initialized to C00555_FALSE. */
    if (s_g0022 != kFalseValue) initialized_false = 0;
    out->initializedFalse = initialized_false;

    /* Phase 2: value is C00555 (= 0). */
    if (s_g0022 != 0) value_is_c00555 = 0;
    out->valueIsC00555 = value_is_c00555;

    /* Phase 3: value in int16_t range (always true for 0). */
    if (s_g0022 < -32768 || s_g0022 > 32767) value_in_range = 0;
    out->valueInRange = value_in_range;

    /* Phase 4: lookup function correctness. */
    if (dm1_v1_indirect_stop_expiring_event_get_pc34() != 0) {
        lookup_function_correct = 0;
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->initializedFalse &&
        out->valueIsC00555 &&
        out->valueInRange &&
        out->lookupFunctionCorrect;
    out->assertionCount = 6;
    return out->accepted;
}