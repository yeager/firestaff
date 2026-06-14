/*
 * test_csb_v1_dungeon_header_pc34_compat.c
 *
 * CSB V1 Dungeon Header parsing (Dungeon GAP 1, level count
 * 24 vs 14).  Source-locked per ReDMCSB CEDTINC8.C:101-118
 * (CSBGAME.DAT vs DMSAVE.DAT dispatch) and M13_PLAN.md:303
 * (header layouts).
 */
#include "csb_v1_dungeon_header_pc34_compat.h"
#include "csb_v1_dungeon_world_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    printf("=== CSB V1 dungeon header (level count 24 vs 14) ===\n");

    /* DM1 PC 3.4: 14 levels. */
    CHECK(csb_v1_dungeon_header_num_level(CSB_V1_DUNGEON_VARIANT_DM1_PC34) == 14,
          "DM1 PC 3.4 NumLevel() = 14 (CEDTINC8.C:101-118)");

    /* CSB: 24 levels. */
    CHECK(csb_v1_dungeon_header_num_level(CSB_V1_DUNGEON_VARIANT_CSB) == 24,
          "CSB NumLevel() = 24 (CEDTINC8.C:101-118, M13_PLAN.md:303)");

    /* CSB_V1_DUNGEON_VARIANT_CSB is 1, not 0. */
    CHECK(CSB_V1_DUNGEON_VARIANT_CSB == 1, "CSB variant tag is 1");
    CHECK(CSB_V1_DUNGEON_VARIANT_DM1_PC34 == 0, "DM1 variant tag is 0");

    /* The default is DM1 for unknown variants. */
    CHECK(csb_v1_dungeon_header_num_level(-1) == 14,
          "unknown variant defaults to DM1 (14)");
    CHECK(csb_v1_dungeon_header_num_level(99) == 14,
          "out-of-range variant defaults to DM1 (14)");

    /* CSB detection. */
    CHECK(csb_v1_dungeon_header_is_csb(CSB_V1_DUNGEON_VARIANT_CSB) == 1,
          "is_csb(CSB) = 1");
    CHECK(csb_v1_dungeon_header_is_csb(CSB_V1_DUNGEON_VARIANT_DM1_PC34) == 0,
          "is_csb(DM1) = 0");
    CHECK(csb_v1_dungeon_header_is_csb(-1) == 0,
          "is_csb(unknown) = 0");

    /* CSB_MAX_LEVELS accommodates the source-locked 24. */
    CHECK(CSB_MAX_LEVELS == 24,
          "CSB_MAX_LEVELS = 24 (was 16 in v1, bumped for CSB source lock)");
    CHECK(CSB_MAX_LEVELS >= 24,
          "CSB_MAX_LEVELS array can hold 24 levels");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
