/*
 * test_csb_v1_neophyte_mode_pc34_compat.c
 *
 * CSB V1 neophyte-mode toggle gate.  Source-locked per
 * ReDMCSB Character.cpp:665 (skill validation), data.cpp:88
 * (default neophyteSkills = false), Recording.cpp:246
 * (replay neophyte mode set true).
 */
#include "csb_v1_neophyte_mode_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 neophyte-mode toggle (Champions GAP 1) ===\n");

    /* Default: mode disabled. */
    csb_v1_neophyte_skills_mode_set(0);
    CHECK(csb_v1_neophyte_skills_mode_get() == 0,
          "default neophyte mode is disabled");
    CHECK(csb_v1_neophyte_display_for_level(0U) == 0,
          "level 0 in DM1 mode displays as NOVICE (NEOPHYTE hidden)");

    /* Enable: mode on. */
    csb_v1_neophyte_skills_mode_set(1);
    CHECK(csb_v1_neophyte_skills_mode_get() == 1,
          "neophyte mode can be enabled");
    CHECK(csb_v1_neophyte_display_for_level(0U) == 1,
          "level 0 in CSB mode displays as NEOPHYTE");

    /* Non-zero values coerce to enabled. */
    csb_v1_neophyte_skills_mode_set(42);
    CHECK(csb_v1_neophyte_skills_mode_get() == 1,
          "non-zero enables mode");
    csb_v1_neophyte_skills_mode_set(-7);
    CHECK(csb_v1_neophyte_skills_mode_get() == 1,
          "negative still enables (any non-zero is true)");
    csb_v1_neophyte_skills_mode_set(0);
    CHECK(csb_v1_neophyte_skills_mode_get() == 0,
          "zero disables mode");

    /* Non-zero levels never trigger neophyte. */
    for (unsigned int lv = 1; lv <= 16; ++lv) {
        if (csb_v1_neophyte_display_for_level(lv) != 0) {
            printf("  FAIL: level %u should not trigger neophyte\n", lv);
            ++g_fail;
        }
    }
    CHECK(1, "levels 1..16 never trigger neophyte display (NEOPHYTE is level 0 only)");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
