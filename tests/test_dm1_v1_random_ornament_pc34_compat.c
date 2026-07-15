#include "dm1_v1_random_ornament_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL %s: got %d, expected %d\n", name, actual, expected);
        ++g_failures;
    }
}

static void test_f0169_source_formula(void)
{
    expect_int("F0169 baseline",
               dm1_v1_dungeon_get_random_ornament_index_pc34(2000, 3000, 0, 30),
               10);
    expect_int("F0169 seeded",
               dm1_v1_dungeon_get_random_ornament_index_pc34(2123, 3079, 0x1234, 30),
               18);
    expect_int("F0169 map-shaped inputs",
               dm1_v1_dungeon_get_random_ornament_index_pc34(2135, 3191, 0xbeef, 30),
               6);
    expect_int("F0169 rejects zero modulo",
               dm1_v1_dungeon_get_random_ornament_index_pc34(2000, 3000, 0, 0),
               0);
}

static void test_f0170_source_gate_and_ordinal(void)
{
    /* value1 = 2000 + (4 << 5) + 7 = 2135;
     * value2 = 3000 + (2 << 6) + 32 + 31 = 3191. */
    expect_int("F0170 ordinal is one based",
               dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
                   1, 7, 4, 7, 2, 32, 31, 0xbeef, 30),
               7);
    expect_int("F0170 rejects index outside source ornament count",
               dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
                   1, 6, 4, 7, 2, 32, 31, 0xbeef, 30),
               0);
    expect_int("F0170 respects source random-allowed flag",
               dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
                   0, 30, 4, 7, 2, 32, 31, 0xbeef, 30),
               0);
    expect_int("F0170 has no empty-map fallback",
               dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
                   1, 30, 4, 7, 2, 0, 31, 0xbeef, 30),
               0);
}

static void test_f0149_uses_only_supplied_current_map_indices(void)
{
    const int mapAlcoves[] = {1, 7, 12};

    expect_int("F0149 finds source map alcove",
               dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(7, mapAlcoves, 3),
               1);
    expect_int("F0149 rejects non-alcove source ornament",
               dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(6, mapAlcoves, 3),
               0);
    expect_int("F0149 rejects absent map list",
               dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(7, NULL, 0),
               0);
    expect_int("F0149 rejects negative index",
               dm1_v1_dungeon_is_wall_ornament_an_alcove_pc34(-1, mapAlcoves, 3),
               0);
}

static void test_f0171_pc34_wall_aspect_order_and_out_of_bounds_alcove_gate(void)
{
    int aspect[DM1_V1_SQUARE_ASPECT_COUNT_PC34] = {0};
    const int inBoundsAlcoves[] = {4, 25}; /* zero-based F0149 indices */
    const int outOfBoundsAlcoves[] = {14, 5};

    expect_int("F0171 in-bounds succeeds",
               dm1_v1_dungeon_set_random_wall_ornament_ordinals_pc34(
                   aspect, 1, 1, 1, 1, 30, 0, 4, 7, 2, 32, 31,
                   0xbeef, inBoundsAlcoves, 2),
               1);
    expect_int("F0171 back ordinal", aspect[3], 5);
    expect_int("F0171 right ordinal", aspect[4], 12);
    expect_int("F0171 front ordinal", aspect[5], 19);
    expect_int("F0171 left ordinal", aspect[6], 26);

    expect_int("F0171 out-of-bounds succeeds",
               dm1_v1_dungeon_set_random_wall_ornament_ordinals_pc34(
                   aspect, 1, 1, 1, 1, 30, 0, -1, 7, 2, 32, 31,
                   0xbeef, outOfBoundsAlcoves, 2),
               1);
    /* Index 14 (ordinal 15) and index 5 (ordinal 6) are source alcoves and
     * are removed only on the out-of-bounds branch. */
    expect_int("F0171 out-of-bounds removes back alcove", aspect[3], 0);
    expect_int("F0171 out-of-bounds keeps non-alcove right", aspect[4] != 0, 1);
    expect_int("F0171 out-of-bounds keeps non-alcove front", aspect[5] != 0, 1);
    expect_int("F0171 out-of-bounds removes left alcove", aspect[6], 0);
}

int main(void)
{
    test_f0169_source_formula();
    test_f0170_source_gate_and_ordinal();
    test_f0149_uses_only_supplied_current_map_indices();
    test_f0171_pc34_wall_aspect_order_and_out_of_bounds_alcove_gate();
    if (!strstr(dm1_v1_random_ornament_source_evidence_pc34(), "F0169") ||
        !strstr(dm1_v1_random_ornament_source_evidence_pc34(), "F0170")) {
        fprintf(stderr, "FAIL source evidence\n");
        ++g_failures;
    }
    if (g_failures) {
        fprintf(stderr, "FAIL dm1_v1_random_ornament_pc34_compat failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_random_ornament_pc34_compat\n");
    return 0;
}
