/*
 * test_dm1_v1_melee_target_half_square_pc34_compat.c
 *
 * Source-locked to ReDMCSB GROUP.C F0176/F0177 and PROJEXPL.C F0229.
 * F0177 walks the F0229 ordered attack cells, but the actual creature
 * hit-test is delegated to F0176. Half-square creatures occupy two
 * adjacent cells according to group-facing parity, and a single centered
 * creature is present on all cells. This pins that melee targeting does
 * not silently treat those source cases as full-square occupants.
 */

#include "dm1_v1_combat_pc34_compat.h"

#include <stdio.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

static DM1_CreatureGroup one_creature_group(int size, int cell)
{
    DM1_CreatureGroup group;

    dm1_combat_init_group(&group);
    group.count = 0;
    group.info.size = size;
    group.creatures[0].cell = cell;
    group.creatures[0].health = 10;
    return group;
}

static int test_full_square_requires_exact_cell(void)
{
    DM1_CreatureGroup group = one_creature_group(DM1_CREATURE_SIZE_FULL_SQUARE, 1);

    CHECK(dm1_get_melee_target(&group, 0, 1, 1) == 0,
          "full-square creature at cell 1 is targetable by exact ordered cell");

    group.creatures[0].cell = 3;
    CHECK(dm1_get_melee_target(&group, 0, 1, 1) == 0,
          "full-square fallback still finds later ordered exact cell");

    return 0;
}

static int test_half_square_matches_adjacent_cells_by_group_direction(void)
{
    DM1_CreatureGroup group = one_creature_group(DM1_CREATURE_SIZE_HALF_SQUARE, 2);

    /* championCell=0, partyDirection=1 selects F0229 row {1,2,0,3}.
     * With groupDirection parity matching query cell 1, F0176 first maps
     * query cell 1 to previous cell 0 and then matches cell 0 or 1. A
     * half-square creature based at cell 2 must therefore not be hit by
     * the first ordered query, but it must be hit by the second query. */
    CHECK(dm1_get_melee_target(&group, 0, 1, 1) == 0,
          "half-square creature at cell 2 is reached through second ordered cell");

    group.creatures[0].cell = 0;
    CHECK(dm1_get_melee_target(&group, 0, 1, 1) == 0,
          "half-square creature at cell 0 is reached through first parity-adjusted cell");

    group.creatures[0].cell = 3;
    CHECK(dm1_get_melee_target(&group, 0, 1, 0) == 0,
          "half-square creature uses groupDirection parity when query parity differs");

    return 0;
}

static int test_single_centered_creature_is_present_on_all_cells(void)
{
    DM1_CreatureGroup group =
        one_creature_group(DM1_CREATURE_SIZE_FULL_SQUARE, DM1_GROUP_CELLS_SINGLE_CENTERED);

    CHECK(dm1_get_melee_target(&group, 0, 0, 0) == 0,
          "single centered creature is reachable from north-facing ordered cells");
    CHECK(dm1_get_melee_target(&group, 1, 1, 2) == 0,
          "single centered creature is reachable from east-facing ordered cells");
    CHECK(dm1_get_melee_target(&group, 2, 2, 1) == 0,
          "single centered creature is reachable from south-facing ordered cells");
    CHECK(dm1_get_melee_target(&group, 3, 3, 3) == 0,
          "single centered creature is reachable from west-facing ordered cells");

    return 0;
}

int main(void)
{
    CHECK(test_full_square_requires_exact_cell() == 0,
          "full-square melee target baseline");
    CHECK(test_half_square_matches_adjacent_cells_by_group_direction() == 0,
          "half-square melee target parity");
    CHECK(test_single_centered_creature_is_present_on_all_cells() == 0,
          "single centered melee target parity");

    printf("dm1_v1_melee_target_half_square_pc34_compat: ok\n");
    return 0;
}
