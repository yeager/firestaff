/* test_dm2_v1_movement_collision_gate_pc34_compat.c
 *
 * Data-free DM2 V1 movement/collision regression.
 *
 * Covers:
 *  1. Wall / pit / lava / inaccessible square types block party movement.
 *  2. Ordinary traversable square types remain walkable.
 *  3. Out-of-bounds level/x/y lookups block movement and expose no tile.
 *  4. Runtime blocked-step state preserves grid position, turns to the
 *     attempted direction, suppresses the move callback, and applies
 *     the same one-tick movement gate as a successful dungeon step.
 *
 * Source-lock:
 *  ReDMCSB DEFS.H:1001-1013 defines square type extraction and the
 *  wall/corridor/pit/stairs/door/teleporter/fakewall element ordinals.
 *  ReDMCSB DUNGEON.C:1438-1475 returns a wall square when map
 *  coordinates are outside the current map bounds.
 */

#include "dm2_v1_world_model.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_game.h"
#include "dm2_v1_runtime.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;
static int runtime_move_callbacks = 0;
static int runtime_turn_callbacks = 0;
static int runtime_last_turn_from = -1;
static int runtime_last_turn_to = -1;

enum {
    DM2_SYNTHETIC_TILE_DATA_START = 44 + 28 * 16,
    DM2_SYNTHETIC_RAW_SIZE = DM2_SYNTHETIC_TILE_DATA_START + 3 * 3 * 2
};

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

static void write_le16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static void write_synthetic_tile(uint8_t raw[DM2_SYNTHETIC_RAW_SIZE],
                                 int x, int y, uint16_t tile)
{
    const int height = 3;
    int offset = DM2_SYNTHETIC_TILE_DATA_START + ((x * height + y) << 1);
    write_le16(raw + offset, tile);
}

static void build_synthetic_runtime(DM2_V1_BootProfile *profile,
                                    DM2_V1_GameState *game,
                                    DM2_V1_DungeonData *dungeon,
                                    uint8_t raw[DM2_SYNTHETIC_RAW_SIZE])
{
    memset(profile, 0, sizeof(*profile));
    memset(game, 0, sizeof(*game));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, DM2_SYNTHETIC_RAW_SIZE);

    raw[6] = 1; /* map_count */
    write_le16(raw + 44 + 12, 3); /* map 0 width override */
    write_le16(raw + 44 + 14, 3); /* map 0 height override */

    /* ReDMCSB DEFS.H:1007-1009: C00_ELEMENT_WALL blocks,
     * C01_ELEMENT_CORRIDOR is the ordinary traversable element.
     * DUNGEON.C:1371-1391 supplies N/E/S/W step deltas used below. */
    write_synthetic_tile(raw, 1, 0, 0); /* north target: C00 wall */
    write_synthetic_tile(raw, 1, 1, 1); /* start: C01 corridor */
    write_synthetic_tile(raw, 2, 1, 1); /* east target: C01 corridor */

    (void)dm2_v1_dungeon_load(dungeon, raw, DM2_SYNTHETIC_RAW_SIZE);

    game->party_x = 1;
    game->party_y = 1;
    game->party_dir = 1; /* East, so a blocked north step also turns. */
    game->current_level = 0;

    profile->dm2_state = game;
    profile->dungeon_data = dungeon;
}

static void on_runtime_move(int from_x, int from_y, int to_x, int to_y)
{
    (void)from_x;
    (void)from_y;
    (void)to_x;
    (void)to_y;
    runtime_move_callbacks++;
}

static void on_runtime_turn(int from_dir, int to_dir)
{
    runtime_turn_callbacks++;
    runtime_last_turn_from = from_dir;
    runtime_last_turn_to = to_dir;
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

static int test_runtime_blocked_step_turn_state(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    uint8_t raw[DM2_SYNTHETIC_RAW_SIZE];
    int ok;

    build_synthetic_runtime(&profile, &game, &dungeon, raw);
    if (dungeon.raw_data == NULL) {
        return 0;
    }

    dm2_v1_runtime_init(&profile);
    dm2_v1_runtime_set_outdoor(0);
    runtime_move_callbacks = 0;
    runtime_turn_callbacks = 0;
    runtime_last_turn_from = -1;
    runtime_last_turn_to = -1;
    dm2_v1_runtime_set_move_callback(on_runtime_move);
    dm2_v1_runtime_set_turn_callback(on_runtime_turn);

    ok = dm2_v1_runtime_move(0) == -1
        && dm2_v1_runtime_get_party_x() == 1
        && dm2_v1_runtime_get_party_y() == 1
        && dm2_v1_runtime_get_party_dir() == 0
        && runtime_move_callbacks == 0
        && runtime_turn_callbacks == 1
        && runtime_last_turn_from == 1
        && runtime_last_turn_to == 0
        && dm2_v1_runtime_can_move() == 0;

    ok = ok
        && dm2_v1_runtime_move(1) == -1
        && dm2_v1_runtime_get_party_x() == 1
        && dm2_v1_runtime_get_party_y() == 1
        && dm2_v1_runtime_get_party_dir() == 0
        && runtime_move_callbacks == 0
        && runtime_turn_callbacks == 1;

    dm2_v1_runtime_tick();
    ok = ok
        && dm2_v1_runtime_can_move() == 1
        && dm2_v1_runtime_move(1) == 0
        && dm2_v1_runtime_get_party_x() == 2
        && dm2_v1_runtime_get_party_y() == 1
        && dm2_v1_runtime_get_party_dir() == 1
        && runtime_move_callbacks == 1
        && runtime_turn_callbacks == 2
        && runtime_last_turn_from == 0
        && runtime_last_turn_to == 1;

    dm2_v1_runtime_set_move_callback(NULL);
    dm2_v1_runtime_set_turn_callback(NULL);
    dm2_v1_dungeon_free(&dungeon);
    return ok;
}

static int test_runtime_turn_only_keeps_position(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_GameState game;
    DM2_V1_DungeonData dungeon;
    uint8_t raw[DM2_SYNTHETIC_RAW_SIZE];
    int ok;

    build_synthetic_runtime(&profile, &game, &dungeon, raw);
    if (dungeon.raw_data == NULL) {
        return 0;
    }

    dm2_v1_runtime_init(&profile);
    dm2_v1_runtime_set_outdoor(0);
    runtime_move_callbacks = 0;
    runtime_turn_callbacks = 0;
    runtime_last_turn_from = -1;
    runtime_last_turn_to = -1;
    dm2_v1_runtime_set_move_callback(on_runtime_move);
    dm2_v1_runtime_set_turn_callback(on_runtime_turn);

    ok = dm2_v1_runtime_turn(-1) == 0
        && dm2_v1_runtime_get_party_x() == 1
        && dm2_v1_runtime_get_party_y() == 1
        && dm2_v1_runtime_get_party_dir() == 0
        && runtime_move_callbacks == 0
        && runtime_turn_callbacks == 1
        && runtime_last_turn_from == 1
        && runtime_last_turn_to == 0
        && dm2_v1_runtime_can_move() == 1;

    ok = ok
        && dm2_v1_runtime_turn(1) == 0
        && dm2_v1_runtime_get_party_x() == 1
        && dm2_v1_runtime_get_party_y() == 1
        && dm2_v1_runtime_get_party_dir() == 1
        && runtime_move_callbacks == 0
        && runtime_turn_callbacks == 2
        && runtime_last_turn_from == 0
        && runtime_last_turn_to == 1
        && dm2_v1_runtime_can_move() == 1;

    dm2_v1_runtime_set_move_callback(NULL);
    dm2_v1_runtime_set_turn_callback(NULL);
    dm2_v1_dungeon_free(&dungeon);
    return ok;
}

int main(void)
{
    printf("=== DM2 V1 Movement Collision Gate ===\n\n");

    TEST(blocking_square_types);
    TEST(walkable_square_types);
    TEST(grid_bounds_block_movement);
    TEST(out_of_bounds_type_sentinel);
    TEST(source_evidence_mentions_collision_anchors);
    TEST(runtime_blocked_step_turn_state);
    TEST(runtime_turn_only_keeps_position);

    printf("\nDM2 V1 Movement Collision Gate: %d/%d passed\n",
           tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
