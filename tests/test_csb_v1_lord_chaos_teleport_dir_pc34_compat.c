/*
 * test_csb_v1_lord_chaos_teleport_dir_pc34_compat.c
 *
 * CSB V1 combat/mechanics GAP 3 -- CHANGE7_19_FIX / BUG0_69.
 *
 * Source-lock:
 *   ReDMCSB GROUP.C:2208-2215 initializes Lord Chaos' danger
 *   teleport direction before the double-square move branch:
 *     primaryDir = M004_RANDOM(4);
 *     secondaryDir = M017_NEXT(primaryDir);
 *   ReDMCSB DEFS.H:458/461 defines M017_NEXT(value) as
 *   (value + 1) & 3.
 *
 * Bounded scope: data-free direction-pair gate only.  This does not
 * claim full Lord Chaos AI parity, live teleporter traversal, or CSB
 * end-to-end playability.
 */
#include "csb_v1_dungeon_world_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static int g_rng_calls = 0;
static int g_rng_index = 0;
static const int g_rng_values[] = { 0, 1, 2, 3, 4, 7, -1, -6 };

static int scripted_random4(void)
{
    int value = g_rng_values[g_rng_index %
                             (int)(sizeof(g_rng_values) / sizeof(g_rng_values[0]))];
    ++g_rng_calls;
    ++g_rng_index;
    return value;
}

static void reset_rng(void)
{
    g_rng_calls = 0;
    g_rng_index = 0;
}

static void test_primary_helper_masks_to_source_range(void)
{
    int i;

    reset_rng();
    for (i = 0; i < (int)(sizeof(g_rng_values) / sizeof(g_rng_values[0])); ++i) {
        int raw = g_rng_values[i];
        int expected = raw & 3;
        int actual = csb_bugfix_lord_chaos_teleport_dir(scripted_random4);
        CHECK(actual == expected, "primary helper returns M004_RANDOM(4) masked to 0..3");
        CHECK(actual >= 0 && actual <= 3, "primary helper never returns an out-of-range direction");
    }
    CHECK(g_rng_calls == (int)(sizeof(g_rng_values) / sizeof(g_rng_values[0])),
          "primary helper consumes exactly one RNG value per direction");
}

static void test_pair_helper_source_assignment(void)
{
    int i;

    reset_rng();
    for (i = 0; i < (int)(sizeof(g_rng_values) / sizeof(g_rng_values[0])); ++i) {
        int primary = -1;
        int secondary = -1;
        int expected_primary = g_rng_values[i] & 3;
        int expected_secondary = (expected_primary + 1) & 3;

        CHECK(csb_bugfix_lord_chaos_teleport_dirs(scripted_random4,
                                                  &primary,
                                                  &secondary) == 0,
              "pair helper accepts valid output pointers");
        CHECK(primary == expected_primary,
              "pair helper primaryDir matches M004_RANDOM(4)");
        CHECK(secondary == expected_secondary,
              "pair helper secondaryDir matches M017_NEXT(primaryDir)");
    }
    CHECK(g_rng_calls == (int)(sizeof(g_rng_values) / sizeof(g_rng_values[0])),
          "pair helper consumes exactly one RNG value per direction pair");
}

static void test_null_and_deterministic_fallback(void)
{
    int primary = -1;
    int secondary = -1;

    reset_rng();
    CHECK(csb_bugfix_lord_chaos_teleport_dirs(NULL, &primary, &secondary) == 0,
          "NULL RNG uses deterministic fallback direction");
    CHECK(primary == 0, "NULL RNG fallback primaryDir is 0");
    CHECK(secondary == 1, "NULL RNG fallback secondaryDir is M017_NEXT(0)");
    CHECK(g_rng_calls == 0, "NULL RNG fallback consumes no RNG");

    CHECK(csb_bugfix_lord_chaos_teleport_dirs(scripted_random4,
                                              NULL,
                                              &secondary) == -1,
          "pair helper rejects NULL primaryDir output");
    CHECK(csb_bugfix_lord_chaos_teleport_dirs(scripted_random4,
                                              &primary,
                                              NULL) == -1,
          "pair helper rejects NULL secondaryDir output");
    CHECK(g_rng_calls == 0, "NULL output rejection happens before RNG consumption");
}

int main(void)
{
    printf("=== CSB V1 Lord Chaos teleport direction BUG0_69 gate ===\n");

    test_primary_helper_masks_to_source_range();
    test_pair_helper_source_assignment();
    test_null_and_deterministic_fallback();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    if (g_fail == 0) {
        puts("ok: CHANGE7_19 Lord Chaos danger teleport direction is initialized deterministically and secondaryDir follows M017_NEXT(primaryDir)");
    }
    return g_fail == 0 ? 0 : 1;
}
