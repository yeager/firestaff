/*
 * csb_v1_version_checker_pc34_compat.c
 *
 * Source-locked per ReDMCSB MOVESENS.C CHANGE7_23 + BugsAnd
 * Changes.htm CHANGE8_06 (engine version 21 for CSB 2.1).
 */
#include "csb_v1_version_checker_pc34_compat.h"

/* CSB 2.1's engine version is 21 (CHANGE8_06 hardcodes this
 * in MOVESENS.C).  DM1 PC 3.4's engine version is 0.  Default
 * is CSB 2.1 (21). */
static int g_csb_v1_engine_version = 21;

int csb_v1_engine_version_get(void) {
    return g_csb_v1_engine_version;
}

void csb_v1_engine_version_set(int version) {
    g_csb_v1_engine_version = version;
}

int csb_v1_version_checker_passes(int dataValue) {
    /* CHANGE7_23: sensor triggers only if data_value <=
     * game_engine_version.  Edge case: dataValue == 0 is a
     * common sentinel for "no constraint" in the source
     * and is always considered passing. */
    if (dataValue == 0) return 1;
    return dataValue <= g_csb_v1_engine_version;
}

int csb_v1_is_csb_v21_mode(void) {
    /* CSB 2.1 is the canonical version for the version-
     * checker sensor; engine version 21 is its source-
     * locked value.  Other CSB versions (2.0, 2.2) may
     * have different engine versions; for v1 we treat
     * engine version 21 as the default CSB v2.1 mode. */
    return g_csb_v1_engine_version == 21;
}
