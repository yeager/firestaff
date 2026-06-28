/* test_dm2_v1_movement_collision_gate_pc34_compat.c
 *
 * Data-free DM2 V1 movement/collision regression.
 *
 * Covers:
 *  1. Wall / pit / lava / inaccessible square types block party movement.
 *  2. Ordinary traversable square types remain walkable.
 *  3. Out-of-bounds level/x/y lookups block movement and expose no tile.
 *
 * Source-lock:
 *  ReDMCSB DEFS.H:1001-1013 defines square type extraction and the
 *  wall/corridor/pit/stairs/door/teleporter/fakewall element ordinals.
 *  ReDMCSB DUNGEON.C:1438-1475 returns a wall square when map
 *  coordinates are outside the current map bounds.
 */

#include "dm2_v1_world_model.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %-52s", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } else { \
        printf("  FAIL\n"); \
    } \
} while (0)

static void build_synthetic_world(dm2_dungeon_world_t *world,
                                  dm2_tile_t tiles[9])
{
    memset(world, 0, sizeof(*world));
    memset(tiles, 0, sizeof(dm2_tile_t) * 9u);

    world->map_count = 1;
    world->levels[0].level_index = 0;
    world->levels[0].width = 3;
    world->levels[0].height = 3;
    world->levels[0].tiles = tiles;

    tiles[0].type = DM2_SQUARE_FLOOR;
    tiles[1].type = DM2_SQUARE_WALL;
    tiles[2].type = DM2_SQUARE_DOOR;

    tiles[3].type = DM2_SQUARE_PIT;
    tiles[4].type = DM2_SQUARE_LAVA;
    tiles[5].type = DM2_SQUARE_INACCESSIBLE;

    tiles[6].type = DM2_SQUARE_STAIRS_UP;
    tiles[7].type = DM2_SQUARE_TELEPORTER;
    tiles[8].type = DM2_SQUARE_FAKE_WALL;
}

static int test_blocking_square_types(void)
{
    dm2_dungeon_world_t world;
    dm2_tile_t tiles[9];
    build_synthetic_world(&world, tiles);

    return dm2_world_is_walkable(&world, 0, 1, 0) == 0
        && dm2_world_is_walkable(&world, 0, 0, 1) == 0
        && dm2_world_is_walkable(&world, 0, 1, 1) == 0
        && dm2_world_is_walkable(&world, 0, 2, 1) == 0;
}

static int test_walkable_square_types(void)
{
    dm2_dungeon_world_t world;
    dm2_tile_t tiles[9];
    build_synthetic_world(&world, tiles);

    return dm2_world_is_walkable(&world, 0, 0, 0) == 1
        && dm2_world_is_walkable(&world, 0, 2, 0) == 1
        && dm2_world_is_walkable(&world, 0, 0, 2) == 1
        && dm2_world_is_walkable(&world, 0, 1, 2) == 1
        && dm2_world_is_walkable(&world, 0, 2, 2) == 1;
}

static int test_grid_bounds_block_movement(void)
{
    dm2_dungeon_world_t world;
    dm2_tile_t tiles[9];
    build_synthetic_world(&world, tiles);

    return dm2_world_get_tile(&world, 0, -1, 0) == NULL
        && dm2_world_get_tile(&world, 0, 3, 0) == NULL
        && dm2_world_get_tile(&world, 0, 0, -1) == NULL
        && dm2_world_get_tile(&world, 0, 0, 3) == NULL
        && dm2_world_get_tile(&world, -1, 0, 0) == NULL
        && dm2_world_get_tile(&world, 1, 0, 0) == NULL
        && dm2_world_is_walkable(&world, 0, -1, 0) == 0
        && dm2_world_is_walkable(&world, 0, 3, 0) == 0
        && dm2_world_is_walkable(&world, 0, 0, -1) == 0
        && dm2_world_is_walkable(&world, 0, 0, 3) == 0
        && dm2_world_is_walkable(&world, -1, 0, 0) == 0
        && dm2_world_is_walkable(&world, 1, 0, 0) == 0;
}

static int test_out_of_bounds_type_sentinel(void)
{
    dm2_dungeon_world_t world;
    dm2_tile_t tiles[9];
    build_synthetic_world(&world, tiles);

    return dm2_world_get_tile_type(&world, 0, -1, 0) == DM2_SQUARE_COUNT
        && dm2_world_get_tile_type(&world, 0, 3, 0) == DM2_SQUARE_COUNT
        && dm2_world_get_tile_type(&world, 1, 0, 0) == DM2_SQUARE_COUNT
        && dm2_world_get_tile_type(NULL, 0, 0, 0) == DM2_SQUARE_COUNT;
}

static int test_source_evidence_mentions_collision_anchors(void)
{
    const char *evidence = dm2_world_source_evidence();
    return evidence != NULL
        && strstr(evidence, "party placement") != NULL
        && strstr(evidence, "square type constants") != NULL;
}

int main(void)
{
    printf("=== DM2 V1 Movement Collision Gate ===\n\n");

    TEST(blocking_square_types);
    TEST(walkable_square_types);
    TEST(grid_bounds_block_movement);
    TEST(out_of_bounds_type_sentinel);
    TEST(source_evidence_mentions_collision_anchors);

    printf("\nDM2 V1 Movement Collision Gate: %d/%d passed\n",
           tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
