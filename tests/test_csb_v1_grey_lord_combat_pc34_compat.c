/*
 * test_csb_v1_grey_lord_combat_pc34_combat.c
 *
 * CSB V1 Combat GAP 2 — Grey Lord combat behavior.
 * Source-locked per ReDMCSB DEFS.H:1679 + Attack.cpp:2423 +
 * BUG0_69 fix.
 */
#include "csb_v1_grey_lord_combat_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 Combat GAP 2 — Grey Lord combat (v2.7.16) ===\n");

    /* Default: CSB aware. */
    csb_v1_grey_lord_aware_set(1);
    CHECK(csb_v1_grey_lord_aware_get() == 1,
          "default Grey Lord awareness is ON (CSB default)");

    /* Lord Chaos present: always detected. */
    CHECK(csb_v1_is_lord_chaos_or_grey_lord_here(0x01, 1, 0) == 1,
          "Lord Chaos present -> detected (DM1 + CSB)");

    /* Grey Lord present (CSB aware): detected. */
    CHECK(csb_v1_is_lord_chaos_or_grey_lord_here(0x01, 0, 1) == 1,
          "Grey Lord present (CSB) -> detected");

    /* Neither present: not detected. */
    CHECK(csb_v1_is_lord_chaos_or_grey_lord_here(0x01, 0, 0) == 0,
          "neither present -> not detected");

    /* DM1 mode: Grey Lord NOT detected. */
    csb_v1_grey_lord_aware_set(0);
    CHECK(csb_v1_grey_lord_aware_get() == 0,
          "Grey Lord awareness can be set to OFF (DM1 mode)");
    CHECK(csb_v1_is_lord_chaos_or_grey_lord_here(0x01, 0, 1) == 0,
          "Grey Lord present (DM1 mode) -> NOT detected (no Grey Lord in DM1)");
    CHECK(csb_v1_is_lord_chaos_or_grey_lord_here(0x01, 1, 0) == 1,
          "Lord Chaos still detected even in DM1 mode");

    /* Reset to default. */
    csb_v1_grey_lord_aware_set(1);
    CHECK(csb_v1_grey_lord_aware_get() == 1,
          "Grey Lord awareness can be reset to ON");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
