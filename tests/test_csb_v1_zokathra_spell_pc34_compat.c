/*
 * test_csb_v1_zokathra_spell_pc34_compat.c
 *
 * CSB V1 Mechanics GAP 3 — ZOKATHRA spell power variant.
 * Source-locked per M13_PLAN.md:337,346 + DEFS.H:1774
 * (C7_SPELL_TYPE_OTHER_ZOKATHRA = 7) + Magic.cpp in the
 * decompilation.
 */
#include "csb_v1_zokathra_spell_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 Mechanics GAP 3 — ZOKATHRA spell power (v2.7.16) ===\n");

    /* Default: CSB 2.1 mode (Firestaff default). */
    csb_v1_zokathra_mode_set(1);
    CHECK(csb_v1_zokathra_mode_get() == 1,
          "default mode is CSB 2.1");

    /* CSB 2.1: ZOKATHRA has 50 kinetic energy. */
    CHECK(csb_v1_zokathra_spell_power(1) == 50,
          "CSB ZOKATHRA spell.kinetic = 50 (M13_PLAN.md:337,346)");

    /* CSB 2.1: explicit 1/0 is the same as the toggle. */
    CHECK(csb_v1_zokathra_spell_power(1) == 50,
          "csb_v1_zokathra_spell_power(1) = 50 (explicit CSB)");

    /* DM1 PC 3.4: ZOKATHRA is a no-op (defensive envelope). */
    csb_v1_zokathra_mode_set(0);
    CHECK(csb_v1_zokathra_mode_get() == 0,
          "mode can be set to DM1");
    CHECK(csb_v1_zokathra_spell_power(0) == 0,
          "DM1 ZOKATHRA spell.kinetic = 0 (no-op, no timeline)");

    /* CSB 2.1 gives a HIGHER value than DM1 (Materializer-tier
     * fireball vs defensive envelope). */
    csb_v1_zokathra_mode_set(1);
    CHECK(csb_v1_zokathra_spell_power(1) > csb_v1_zokathra_spell_power(0),
          "CSB ZOKATHRA > DM1 ZOKATHRA (Materializer-tier vs no-op)");

    /* Reset to default. */
    csb_v1_zokathra_mode_set(1);
    CHECK(csb_v1_zokathra_mode_get() == 1,
          "mode can be reset to CSB");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
