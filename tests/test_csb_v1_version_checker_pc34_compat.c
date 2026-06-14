/*
 * test_csb_v1_version_checker_pc34_compat.c
 *
 * CSB V1 Version Checker Sensor (Dungeon GAP 3, CHANGE7_23,
 * CHANGE8_06).  Source-locked per ReDMCSB MOVESENS.C
 * CHANGE7_23 (version-gated floor sensor) and BugsAndChanges.htm
 * CHANGE8_06 (engine version 21 hardcoded for CSB 2.1).
 */
#include "csb_v1_version_checker_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 version checker sensor (CHANGE7_23/8_06) ===\n");

    /* Default engine version is CSB 2.1 = 21. */
    CHECK(csb_v1_engine_version_get() == 21,
          "default engine version is 21 (CSB 2.1, CHANGE8_06)");

    /* csb_v1_is_csb_v21_mode. */
    CHECK(csb_v1_is_csb_v21_mode() == 1,
          "default mode is CSB 2.1 (engine version 21)");

    /* Version checker trigger: dataValue <= engineVersion. */
    CHECK(csb_v1_version_checker_passes(0) == 1,
          "dataValue=0 always passes (sentinel for no constraint)");
    CHECK(csb_v1_version_checker_passes(1) == 1,
          "dataValue=1 <= 21 (passes)");
    CHECK(csb_v1_version_checker_passes(21) == 1,
          "dataValue=21 == 21 (boundary, passes)");
    CHECK(csb_v1_version_checker_passes(22) == 0,
          "dataValue=22 > 21 (does not pass)");
    CHECK(csb_v1_version_checker_passes(100) == 0,
          "dataValue=100 > 21 (does not pass)");

    /* Toggling engine version. */
    csb_v1_engine_version_set(0);
    CHECK(csb_v1_engine_version_get() == 0,
          "engine version can be set to 0 (DM1 mode)");
    CHECK(csb_v1_is_csb_v21_mode() == 0,
          "engine version 0 is not CSB 2.1");
    CHECK(csb_v1_version_checker_passes(0) == 1,
          "dataValue=0 always passes (even at version 0)");
    CHECK(csb_v1_version_checker_passes(1) == 0,
          "dataValue=1 > 0 (does not pass at DM1 mode)");

    csb_v1_engine_version_set(50);
    CHECK(csb_v1_version_checker_passes(50) == 1,
          "dataValue=50 == 50 (boundary, passes at v50)");
    CHECK(csb_v1_version_checker_passes(49) == 1,
          "dataValue=49 < 50 (passes at v50)");
    CHECK(csb_v1_version_checker_passes(51) == 0,
          "dataValue=51 > 50 (does not pass at v50)");

    /* Negative dataValue treated as "always passes" (treat as
     * "no constraint").  Defensive envelope. */
    CHECK(csb_v1_version_checker_passes(-1) == 1,
          "dataValue=-1 treated as always passes (defensive)");

    /* Reset to default. */
    csb_v1_engine_version_set(21);
    CHECK(csb_v1_engine_version_get() == 21,
          "engine version can be reset to 21");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
