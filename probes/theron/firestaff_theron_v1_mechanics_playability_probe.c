/*
 * firestaff_theron_v1_mechanics_playability_probe.c
 *
 * Theron's Quest V1 — Real-Data Mechanics Playability Probe
 *
 * Headless mechanics verification against the authentic JP/US Track 02
 * Hall-of-Records and all seven dungeons. Unlike the synthetic cross-route
 * probe, this probe contains no constructed levels or object tables: it loads
 * real startup and full-dungeon bytes, then exercises movement, turning,
 * blocking and stairs on the decoded Track 02 grids. Stair attributes remain
 * unresolved per docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §2.3;
 * this probe verifies the real-data path remains fail-closed.
 *
 * Run:
 *   ./build/firestaff_theron_v1_mechanics_playability_probe
 *   FIRESTAFF_THERON_DATA_DIR=/path/to/data ./build/firestaff_theron_v1_mechanics_playability_probe
 *   ctest --test-dir build -R theron_v1_mechanics_playability -j2 --output-on-failure
 *
 * Source-lock references:
 *   THQUEST.ASM T520/T560/T600/T700/T800/T900
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *   docs/source-lock/movement_features.md (ReDMCSB MOVESENS.C)
 *   Real JP/US Track 02 BINs under ~/.firestaff/data/theron
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "asset_status_m12.h"
#include "theron_v1_champions.h"
#include "theron_v1_combat.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_dungeon_loader.h"
#include "theron_v1_world.h"

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

/* ── Test bookkeeping ──────────────────────────────────────────────── */
static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond_, msg_) do { \
    if (cond_) { \
        printf("  [PASS] %s\n", (msg_)); \
        g_pass++; \
    } else { \
        printf("  [FAIL] %s\n", (msg_)); \
        g_fail++; \
    } \
} while (0)

#define CHECK_INT(msg_, got_, want_) do { \
    int g_ = (got_); \
    int w_ = (want_); \
    if (g_ == w_) { \
        printf("  [PASS] %s\n", (msg_)); \
        g_pass++; \
    } else { \
        printf("  [FAIL] %s — got=%d want=%d\n", (msg_), g_, w_); \
        g_fail++; \
    } \
} while (0)

/* ── File helpers ──────────────────────────────────────────────────── */
static int file_exists(const char *path) {
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    *out_size = 0;
    fp = fopen(path, "rb");
    if (!fp || fseek(fp, 0, SEEK_END) != 0 ||
        (size = ftell(fp)) <= 0 || fseek(fp, 0, SEEK_SET) != 0) {
        if (fp) fclose(fp);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *out_size = (size_t)size;
    return data;
}

static const char *resolve_data_dir(int argc, char **argv) {
    const char *env;
    static char path[1024];
    const char *home;

    if (argc >= 2 && argv[1] && argv[1][0]) return argv[1];

    env = getenv("FIRESTAFF_THERON_DATA_DIR");
    if (env && env[0]) return env;

    home = getenv("HOME");
    if (!home || !home[0]) home = "<local-home>";
    snprintf(path, sizeof(path), "%s%s.firestaff%sdata%stheron",
             home, PATH_SEP, PATH_SEP, PATH_SEP);
    return path;
}

static void build_path(char *out, size_t out_cap,
                       const char *dir, const char *name) {
    snprintf(out, out_cap, "%s%s%s", dir, PATH_SEP, name);
}

/* ── Geometry helpers on the loaded level ──────────────────────────── */
static int find_floor_with_neighbours(const Theron_V1_Level *level,
                                      int *out_x, int *out_y) {
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int x, y, i;

    for (y = 1; y < level->height - 1; y++) {
        for (x = 1; x < level->width - 1; x++) {
            if (level->squares[y][x] != THERON_SQUARE_FLOOR) continue;
            int has_wall = 0;
            int has_floor = 0;
            for (i = 0; i < 4; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                uint8_t t = level->squares[ny][nx];
                if (t == THERON_SQUARE_WALL || t == THERON_SQUARE_SECRET) {
                    has_wall = 1;
                } else if (t == THERON_SQUARE_FLOOR) {
                    has_floor = 1;
                }
            }
            if (has_wall && has_floor) {
                *out_x = x;
                *out_y = y;
                return 1;
            }
        }
    }
    return 0;
}

static int find_adjacent_wall(const Theron_V1_Level *level,
                              int sx, int sy,
                              int *out_x, int *out_y) {
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int i;
    for (i = 0; i < 4; i++) {
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        if (nx >= 0 && nx < level->width && ny >= 0 && ny < level->height) {
            uint8_t t = level->squares[ny][nx];
            if (t == THERON_SQUARE_WALL || t == THERON_SQUARE_SECRET) {
                *out_x = nx;
                *out_y = ny;
                return 1;
            }
        }
    }
    return 0;
}

static int find_adjacent_floor(const Theron_V1_Level *level,
                               int sx, int sy,
                               int *out_x, int *out_y) {
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int i;
    for (i = 0; i < 4; i++) {
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        if (nx >= 0 && nx < level->width && ny >= 0 && ny < level->height &&
            level->squares[ny][nx] == THERON_SQUARE_FLOOR) {
            *out_x = nx;
            *out_y = ny;
            return 1;
        }
    }
    return 0;
}

static int find_adjacent_door_approach(const Theron_V1_Level *level,
                                       int sx, int sy,
                                       int *out_x, int *out_y) {
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int i;
    for (i = 0; i < 4; i++) {
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        uint8_t tile;
        if (nx < 0 || nx >= level->width || ny < 0 || ny >= level->height)
            continue;
        tile = level->squares[ny][nx];
        /* Start only on ordinary floor.  A stair, pit, teleporter, or other
         * special square would confound a door-blocking observation with a
         * second mechanic. */
        if (tile == THERON_SQUARE_FLOOR) {
            *out_x = nx;
            *out_y = ny;
            return 1;
        }
    }
    return 0;
}

static int direction_from_delta(int dx, int dy) {
    if (dy < 0) return THERON_DIR_NORTH;
    if (dy > 0) return THERON_DIR_SOUTH;
    if (dx > 0) return THERON_DIR_EAST;
    if (dx < 0) return THERON_DIR_WEST;
    return THERON_DIR_NORTH;
}

typedef struct {
    int current_dungeon;
    int current_level;
    int party_x;
    int party_y;
    int party_direction;
    int transition_pending;
    Theron_TransitionType transition_type;
    int transition_target_level;
    int transition_spawn_x;
    int transition_spawn_y;
    unsigned int source_actuator_event_count;
    uint64_t world_tick;
} Theron_StairMovementSnapshot;

static Theron_StairMovementSnapshot stair_movement_snapshot(
    const Theron_V1_World *world) {
    Theron_StairMovementSnapshot snapshot;
    snapshot.current_dungeon = world->current_dungeon;
    snapshot.current_level = world->current_level;
    snapshot.party_x = world->party.leader_x;
    snapshot.party_y = world->party.leader_y;
    snapshot.party_direction = world->party.leader_dir;
    snapshot.transition_pending = world->transition_pending;
    snapshot.transition_type = world->transition_type;
    snapshot.transition_target_level = world->transition_target_level;
    snapshot.transition_spawn_x = world->transition_spawn_x;
    snapshot.transition_spawn_y = world->transition_spawn_y;
    snapshot.source_actuator_event_count =
        world->source_actuator_event_count;
    snapshot.world_tick = world->world_tick;
    return snapshot;
}

static int stair_movement_state_matches(
    const Theron_V1_World *world,
    const Theron_StairMovementSnapshot *snapshot) {
    return world && snapshot &&
        world->current_dungeon == snapshot->current_dungeon &&
        world->current_level == snapshot->current_level &&
        world->party.leader_x == snapshot->party_x &&
        world->party.leader_y == snapshot->party_y &&
        world->party.leader_dir == snapshot->party_direction &&
        world->transition_pending == snapshot->transition_pending &&
        world->transition_type == snapshot->transition_type &&
        world->transition_target_level == snapshot->transition_target_level &&
        world->transition_spawn_x == snapshot->transition_spawn_x &&
        world->transition_spawn_y == snapshot->transition_spawn_y &&
        world->source_actuator_event_count ==
            snapshot->source_actuator_event_count &&
        world->world_tick == snapshot->world_tick;
}

static void test_authentic_door_boundaries(
    const uint8_t *user_data, size_t user_data_size,
    Theron_Track02Variant variant) {
    Theron_V1_World *world = (Theron_V1_World *)calloc(1u, sizeof(*world));
    int dungeon_door_records[THERON_DUNGEON_COUNT] = {0};
    int dungeon_doors_with_approach[THERON_DUNGEON_COUNT] = {0};
    int dungeon_rejected_interactions[THERON_DUNGEON_COUNT] = {0};
    int loaded_dungeons = 0;
    int door_records = 0;
    int invalid_source_doors = 0;
    int doors_without_floor_approach = 0;
    int doors_overlapped_by_active_creature = 0;
    int doors_with_approach = 0;
    int rejected_interactions = 0;

    if (!world) {
        CHECK_INT("allocate authentic door boundary world", 0, 1);
        return;
    }
    for (int dungeon_id = 1; dungeon_id <= THERON_DUNGEON_COUNT;
         ++dungeon_id) {
        Theron_DungeonLoadResult result;
        theron_v1_world_init(world);
        world->current_dungeon = dungeon_id;
        if (theron_v1_track02_load_full_dungeon_for_variant(
                world, dungeon_id, user_data, user_data_size,
                variant, &result) != 0) {
            continue;
        }
        ++loaded_dungeons;
        for (int i = 0; i < world->object_count; ++i) {
            Theron_V1_Object *door = &world->objects[i];
            const Theron_V1_Level *level;
            int approach_x = -1;
            int approach_y = -1;
            int occupied_by_active_creature = 0;
            int door_x;
            int door_y;
            int door_level;
            int valid;

            if (door->type != THERON_OBJTYPE_DOOR) continue;
            ++door_records;
            ++dungeon_door_records[dungeon_id - 1];
            if (!door->source_origin_valid ||
                door->source_category != THERON_CAT_DOOR ||
                door->source_raw_size != 4u ||
                door->dungeon_id != dungeon_id ||
                door->source_dungeon != dungeon_id ||
                door->level < 0 || door->level >= result.levels_loaded ||
                door->source_level != door->level ||
                door->source_x != door->x || door->source_y != door->y) {
                ++invalid_source_doors;
                continue;
            }
            level = &world->levels[dungeon_id - 1][door->level];
            if (door->x < 0 || door->x >= level->width ||
                door->y < 0 || door->y >= level->height ||
                level->squares[door->y][door->x] != THERON_SQUARE_DOOR) {
                ++invalid_source_doors;
                continue;
            }
            if (!find_adjacent_door_approach(
                    level, door->x, door->y, &approach_x, &approach_y)) {
                ++doors_without_floor_approach;
                continue;
            }
            /* Movement resolves active creatures before door collision.  Do
             * not let an unrelated combat route masquerade as door evidence,
             * or place the test party on a creature's occupied approach. */
            for (int creature_index = 0;
                 creature_index < world->creature_count; ++creature_index) {
                const Theron_V1_Creature *creature =
                    &world->creatures[creature_index];
                if (creature->dungeon_id == dungeon_id &&
                    creature->level == door->level &&
                    ((creature->x == door->x && creature->y == door->y) ||
                     (creature->x == approach_x &&
                      creature->y == approach_y)) &&
                    (creature->flags & THERON_CF_ACTIVE)) {
                    occupied_by_active_creature = 1;
                    break;
                }
            }
            if (occupied_by_active_creature) {
                ++doors_overlapped_by_active_creature;
                continue;
            }
            ++doors_with_approach;
            ++dungeon_doors_with_approach[dungeon_id - 1];
            door_x = door->x;
            door_y = door->y;
            door_level = door->level;
            world->current_level = door_level;
            world->party.leader_x = approach_x;
            world->party.leader_y = approach_y;
            world->party.leader_dir = direction_from_delta(
                door_x - approach_x, door_y - approach_y);
            valid = door->state == THERON_DOOR_STATE_CLOSED &&
                theron_v1_move_party_original_command(
                    world, THERON_ORIGINAL_COMMAND_MOVE_FORWARD) ==
                    THERON_MOVE_BLOCKED &&
                world->party.leader_x == approach_x &&
                world->party.leader_y == approach_y &&
                theron_v1_door_open(world, door_x, door_y) == -1 &&
                theron_v1_click_route(world, door_x, door_y,
                                      THERON_CMD_USE) == -1 &&
                door->state == THERON_DOOR_STATE_CLOSED;
            if (valid) {
                ++rejected_interactions;
                ++dungeon_rejected_interactions[dungeon_id - 1];
            }
        }
    }
    CHECK_INT("all seven authentic dungeons load for door checks",
              loaded_dungeons, THERON_DUNGEON_COUNT);
    CHECK_INT("authentic door objects retain exact source provenance",
              invalid_source_doors, 0);
    CHECK_INT("authentic source doors have adjacent approaches",
              doors_with_approach > 0 &&
                  doors_with_approach <= door_records, 1);
    CHECK_INT("authentic door records are fully classified",
              invalid_source_doors + doors_without_floor_approach +
                  doors_overlapped_by_active_creature + doors_with_approach,
              door_records);
    CHECK_INT("each approached authentic door blocks movement and rejects unbound interaction",
              rejected_interactions, doors_with_approach);
    printf("  authentic doors: records=%d invalid=%d no-floor-approach=%d active-creature-overlap=%d approached=%d fail-closed=%d\n",
           door_records, invalid_source_doors, doors_without_floor_approach,
           doors_overlapped_by_active_creature, doors_with_approach,
           rejected_interactions);
    for (int dungeon = 0; dungeon < THERON_DUNGEON_COUNT; ++dungeon) {
        printf("  dungeon %d authentic doors: records=%d approached=%d fail-closed=%d\n",
               dungeon + 1, dungeon_door_records[dungeon],
               dungeon_doors_with_approach[dungeon],
               dungeon_rejected_interactions[dungeon]);
        CHECK_INT("each dungeon's approached authentic doors fail closed",
                  dungeon_rejected_interactions[dungeon],
                  dungeon_doors_with_approach[dungeon]);
    }
    free(world);
}

/* ── World setup ───────────────────────────────────────────────────── */
static void setup_world_from_level(Theron_V1_World *world,
                                   const Theron_V1_Level *level) {
    memset(world, 0, sizeof(*world));
    world->current_dungeon = THERON_DUNGEON_1_AKUTUBA;
    world->current_level = 0;
    world->level_loaded[0][0] = 1;
    memcpy(&world->levels[0][0], level, sizeof(*level));

    world->party.leader_x = level->start_x;
    world->party.leader_y = level->start_y;
    world->party.leader_dir = level->start_dir;
    /* Movement evidence needs only the authenticated map and its source
     * pose. Do not seed fixture champions, stats or gold beside real media;
     * roster/state consumers have separate regional source receipts. */
}

/* ── Real-data test cases ──────────────────────────────────────────── */
static void test_real_level_loaded(const Theron_V1_Level *level) {
    printf("[test:real_level_loaded]\n");
    CHECK_INT("real level width is 32", level->width, 32);
    CHECK_INT("real level height is 27", level->height, 27);
    CHECK_INT("real level start x in bounds",
              level->start_x >= 0 && level->start_x < level->width, 1);
    CHECK_INT("real level start y in bounds",
              level->start_y >= 0 && level->start_y < level->height, 1);
    CHECK_INT("real level start dir is cardinal",
              level->start_dir >= 0 && level->start_dir < THERON_DIR_COUNT, 1);
}

static void test_turning_on_real_grid(Theron_V1_World *world) {
    int start_dir;
    printf("[test:turning_on_real_grid]\n");

    start_dir = world->party.leader_dir;
    theron_v1_turn_party(world, +1);
    CHECK_INT("turn right increments dir",
              world->party.leader_dir, (start_dir + 1) % THERON_DIR_COUNT);

    theron_v1_turn_party(world, -1);
    CHECK_INT("turn left restores dir", world->party.leader_dir, start_dir);

    theron_v1_turn_party(world, -1);
    CHECK_INT("turn left wraps correctly",
              world->party.leader_dir,
              (start_dir + THERON_DIR_COUNT - 1) % THERON_DIR_COUNT);

    theron_v1_turn_party(world, +1);
    CHECK_INT("turn right restores dir", world->party.leader_dir, start_dir);
}

static void test_wall_blocking_on_real_grid(Theron_V1_World *world,
                                            const Theron_V1_Level *level) {
    int wx, wy, sx, sy;
    int original_x, original_y, original_dir;
    int dir;
    int moved;
    printf("[test:wall_blocking_on_real_grid]\n");

    /* Pick a floor square with a wall neighbour; face the wall. */
    if (!find_floor_with_neighbours(level, &sx, &sy)) {
        printf("  [SKIP] no floor square with both wall and floor neighbours\n");
        g_skip++;
        return;
    }
    if (!find_adjacent_wall(level, sx, sy, &wx, &wy)) {
        printf("  [SKIP] no adjacent wall found\n");
        g_skip++;
        return;
    }

    original_x = sx;
    original_y = sy;
    original_dir = world->party.leader_dir;

    /* Face the wall */
    dir = direction_from_delta(wx - sx, wy - sy);
    world->party.leader_x = sx;
    world->party.leader_y = sy;
    world->party.leader_dir = dir;

    moved = theron_v1_move_party(world, dir);
    CHECK_INT("move into wall is blocked", moved, THERON_MOVE_BLOCKED);
    CHECK_INT("wall block preserves x", world->party.leader_x, original_x);
    CHECK_INT("wall block preserves y", world->party.leader_y, original_y);

    world->party.leader_dir = original_dir;
}

static void test_floor_movement_on_real_grid(Theron_V1_World *world,
                                             const Theron_V1_Level *level) {
    int sx, sy, fx, fy;
    int original_dir;
    int dir;
    int moved;
    printf("[test:floor_movement_on_real_grid]\n");

    if (!find_floor_with_neighbours(level, &sx, &sy)) {
        printf("  [SKIP] no floor square with both wall and floor neighbours\n");
        g_skip++;
        return;
    }
    if (!find_adjacent_floor(level, sx, sy, &fx, &fy)) {
        printf("  [SKIP] no adjacent floor found\n");
        g_skip++;
        return;
    }

    original_dir = world->party.leader_dir;

    world->party.leader_x = sx;
    world->party.leader_y = sy;
    dir = direction_from_delta(fx - sx, fy - sy);
    world->party.leader_dir = dir;

    moved = theron_v1_move_party(world, dir);
    CHECK_INT("floor move succeeds", moved, THERON_MOVE_OK);
    CHECK_INT("floor move updates x", world->party.leader_x, fx);
    CHECK_INT("floor move updates y", world->party.leader_y, fy);
    CHECK_INT("floor move updates dir", world->party.leader_dir, dir);

    world->party.leader_dir = original_dir;
}

static void test_original_commands_on_real_grid(
    Theron_V1_World *world, const Theron_V1_Level *level) {
    static const uint8_t commands[] = {
        THERON_ORIGINAL_COMMAND_MOVE_FORWARD,
        THERON_ORIGINAL_COMMAND_MOVE_RIGHT,
        THERON_ORIGINAL_COMMAND_MOVE_BACKWARD,
        THERON_ORIGINAL_COMMAND_MOVE_LEFT
    };
    int sx, sy, fx, fy, wx, wy;
    int floor_dir, wall_dir;
    size_t i;

    printf("[test:original_commands_on_real_grid]\n");
    if (!find_floor_with_neighbours(level, &sx, &sy) ||
        !find_adjacent_floor(level, sx, sy, &fx, &fy) ||
        !find_adjacent_wall(level, sx, sy, &wx, &wy)) {
        printf("  [SKIP] need one real floor with floor and wall neighbours\n");
        g_skip++;
        return;
    }
    floor_dir = direction_from_delta(fx - sx, fy - sy);
    wall_dir = direction_from_delta(wx - sx, wy - sy);
    for (i = 0u; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        int relative = (int)commands[i] -
                       THERON_ORIGINAL_COMMAND_MOVE_FORWARD;
        int facing = (floor_dir - relative + THERON_DIR_COUNT) & 3;
        int result;
        world->party.leader_x = sx;
        world->party.leader_y = sy;
        world->party.leader_dir = facing;
        result = theron_v1_move_party_original_command(world, commands[i]);
        CHECK_INT("original command crosses real floor", result,
                  THERON_MOVE_OK);
        CHECK_INT("original command real-floor x", world->party.leader_x, fx);
        CHECK_INT("original command real-floor y", world->party.leader_y, fy);
        CHECK_INT("relative command preserves facing",
                  world->party.leader_dir, facing);

        facing = (wall_dir - relative + THERON_DIR_COUNT) & 3;
        world->party.leader_x = sx;
        world->party.leader_y = sy;
        world->party.leader_dir = facing;
        result = theron_v1_move_party_original_command(world, commands[i]);
        CHECK_INT("original command blocks on real wall", result,
                  THERON_MOVE_BLOCKED);
        CHECK_INT("real wall preserves x", world->party.leader_x, sx);
        CHECK_INT("real wall preserves y", world->party.leader_y, sy);
        CHECK_INT("blocked relative command preserves facing",
                  world->party.leader_dir, facing);
    }
}

static void test_get_move_result_on_real_grid(Theron_V1_World *world,
                                              const Theron_V1_Level *level) {
    int sx, sy, wx, wy, fx, fy;
    int dir;
    printf("[test:get_move_result_on_real_grid]\n");

    if (!find_floor_with_neighbours(level, &sx, &sy)) {
        printf("  [SKIP] no floor square with both wall and floor neighbours\n");
        g_skip++;
        return;
    }
    if (!find_adjacent_wall(level, sx, sy, &wx, &wy) ||
        !find_adjacent_floor(level, sx, sy, &fx, &fy)) {
        printf("  [SKIP] need adjacent wall and floor\n");
        g_skip++;
        return;
    }

    world->party.leader_x = sx;
    world->party.leader_y = sy;

    dir = direction_from_delta(wx - sx, wy - sy);
    CHECK_INT("get_move_result wall = BLOCKED",
              theron_v1_get_move_result(world, dir), THERON_MOVE_BLOCKED);

    dir = direction_from_delta(fx - sx, fy - sy);
    CHECK_INT("get_move_result floor = OK",
              theron_v1_get_move_result(world, dir), THERON_MOVE_OK);
}

static void test_sound_validation(void) {
    printf("[test:sound_validation]\n");
    CHECK_INT("unbound Track 01 sound remains blocked",
              theron_v1_sound_is_valid(THERON_SOUND_BOOT_MUSIC), 0);
    CHECK_INT("sound invalid for negative id",
              theron_v1_sound_is_valid((Theron_SoundID)-1), 0);
    CHECK_INT("sound invalid for out-of-range id",
              theron_v1_sound_is_valid(THERON_SOUND_COUNT), 0);
}

static void test_creature_spawn_and_combat_gate(Theron_V1_World *world,
                                                const Theron_V1_Level *level) {
    int sx, sy, fx, fy, dir, cid;
    printf("[test:creature_spawn_and_combat_gate]\n");

    if (!find_floor_with_neighbours(level, &sx, &sy) ||
        !find_adjacent_floor(level, sx, sy, &fx, &fy)) {
        printf("  [SKIP] need adjacent floor squares\n");
        g_skip++;
        return;
    }

    setup_world_from_level(world, level);
    world->party.leader_x = sx;
    world->party.leader_y = sy;

    /* The source monster occurrence and the bank-switched RNG consumer are
     * not both available in this level-0 probe. Production must retain the
     * source level without publishing a host-seeded creature. */
    cid = theron_v1_creature_spawn(world, THERON_CREATURE_SHADO,
                                   world->current_dungeon,
                                   world->current_level, fx, fy);
    CHECK_INT("source creature spawn remains blocked", cid, -1);
    CHECK_INT("source creature count remains zero",
              theron_v1_creature_count(world, world->current_dungeon,
                                       world->current_level), 0);

    /* Attack the creature from the adjacent square. */
    dir = direction_from_delta(fx - sx, fy - sy);
    world->party.leader_dir = dir;
    CHECK_INT("source champion attack remains blocked",
              theron_v1_champion_attack(world, world->party.active_slot, cid), -1);
}

static void test_creature_drop(Theron_V1_World *world,
                               const Theron_V1_Level *level) {
    int sx, sy, fx, fy, cid, object_before;
    printf("[test:creature_drop]\n");

    if (!find_floor_with_neighbours(level, &sx, &sy) ||
        !find_adjacent_floor(level, sx, sy, &fx, &fy)) {
        printf("  [SKIP] need adjacent floor squares\n");
        g_skip++;
        return;
    }

    setup_world_from_level(world, level);
    world->party.leader_x = sx;
    world->party.leader_y = sy;
    object_before = world->object_count;

    /* No source-owned spawn record/RNG consumer is available here, so the
     * production route must not create a creature or a drop. */
    cid = theron_v1_creature_spawn(world, THERON_CREATURE_AKUTUBA,
                                   world->current_dungeon,
                                   world->current_level, fx, fy);
    CHECK_INT("source drop spawn remains blocked", cid, -1);
    CHECK_INT("source drop kill remains blocked",
              theron_v1_creature_kill(world, cid), -1);
    CHECK_INT("source drop object count unchanged",
              world->object_count, object_before);
}

static void test_object_table_decode_and_apply_real_data(
    const uint8_t *data,
    size_t size,
    const char *md5,
    Theron_V1_World *world) {

    Theron_Track02InitialLevelObjectTableReceipt receipt;
    Theron_Track02SignalStatus status;
    int before;
    int rc;

    printf("[test:object_table_decode_and_apply_real_data]\n");

    status = theron_v1_track02_decode_initial_level_object_table(
        data, size, md5, &receipt);
    /* The JP/US Track 02 tails are now source-proven as empty compact object
     * tables (count 0, all-zero tail).  The decoder must accept them. */
    CHECK_INT("object table decoder returns OK",
              status, THERON_TRACK02_SIGNAL_OK);
    CHECK_INT("object table decoder marks receipt valid",
              receipt.valid, 1);
    CHECK_INT("object table semantics proven",
              receipt.object_table_semantics_proven, 1);
    CHECK_INT("object table promotion unblocked",
              receipt.promotion_blocked, 0);
    CHECK_INT("Hall-of-Records object table is empty",
              (int)receipt.object_table.record_count, 0);

    before = world->object_count;
    rc = theron_v1_world_apply_track02_object_table(
        world, THERON_DUNGEON_1_AKUTUBA, 0, &receipt.object_table);
    CHECK_INT("apply empty object table succeeds", rc, 0);
    CHECK_INT("empty object table leaves object count unchanged",
              world->object_count, before);
}

static void test_real_full_dungeon_and_stairs(
    const uint8_t *data, size_t size, const char *md5,
    Theron_Track02Variant variant) {
    Theron_V1_World *world;
    Theron_DungeonLoadResult result;
    uint8_t *user_data = NULL;
    size_t sector_count = 0u, user_data_size = 0u, copied_size = 0u;
    int authentic_stair_cells = 0;
    int tested_stair_cells = 0;
    int blocked_stair_cells = 0;
    int loaded_dungeons = 0;
    int expected_stair_cells = 0;
    int expected_tested_stair_cells = 0;

    if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        expected_stair_cells = 171;
        expected_tested_stair_cells = 39;
    } else if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        expected_stair_cells = 170;
        expected_tested_stair_cells = 42;
    }

    printf("[test:real_full_dungeon_and_stairs]\n");
    world = (Theron_V1_World *)calloc(1u, sizeof(*world));
    if (!world) {
        printf("  [FAIL] allocate full real dungeon world\n");
        g_fail++;
        return;
    }
    if (theron_v1_track02_raw_user_data_size(
            size, md5, &sector_count, &user_data_size) !=
            THERON_TRACK02_SIGNAL_OK || sector_count == 0u ||
        !(user_data = (uint8_t *)malloc(user_data_size)) ||
        theron_v1_track02_copy_raw_user_data(
            data, size, md5, user_data, user_data_size, &copied_size) !=
            THERON_TRACK02_SIGNAL_OK || copied_size != user_data_size) {
        printf("  [FAIL] normalize authentic raw sectors\n");
        g_fail++;
        free(user_data);
        free(world);
        return;
    }
    for (int dungeon_id = 1; dungeon_id <= THERON_DUNGEON_COUNT;
         ++dungeon_id) {
        theron_v1_world_init(world);
        world->current_dungeon = dungeon_id;
        if (theron_v1_track02_load_full_dungeon_for_variant(
                world, (Theron_DungeonID)dungeon_id,
                user_data, user_data_size, variant, &result) != 0) {
            continue;
        }
        ++loaded_dungeons;
        for (int level_index = 0; level_index < result.levels_loaded;
             ++level_index) {
            const Theron_V1_Level *level =
                &world->levels[dungeon_id - 1][level_index];
            if (!level->source_header_verified) continue;
            for (int y = 0; y < level->height; ++y) {
                for (int x = 0; x < level->width; ++x) {
                    uint8_t tile = level->squares[y][x];
                    int approach_x = -1;
                int approach_y = -1;
                int direction;
                Theron_StairMovementSnapshot before;
                Theron_MoveResult query;

                    if (tile != THERON_SQUARE_STAIRS_UP &&
                        tile != THERON_SQUARE_STAIRS_DOWN &&
                        tile != THERON_SQUARE_STAIRS_UNRESOLVED)
                        continue;
                    ++authentic_stair_cells;
                    if (!find_adjacent_floor(level, x, y,
                                             &approach_x, &approach_y))
                        continue;

                    direction = direction_from_delta(
                        x - approach_x, y - approach_y);
                    world->current_level = level_index;
                world->party.leader_x = approach_x;
                world->party.leader_y = approach_y;
                world->party.leader_dir = direction;
                before = stair_movement_snapshot(world);
                query = theron_v1_get_move_result(world, direction);
                if (query == THERON_MOVE_BLOCKED &&
                    stair_movement_state_matches(world, &before) &&
                    theron_v1_move_party_original_command(
                        world, THERON_ORIGINAL_COMMAND_MOVE_FORWARD) ==
                        THERON_MOVE_BLOCKED &&
                    stair_movement_state_matches(world, &before)) {
                        ++blocked_stair_cells;
                    }
                    ++tested_stair_cells;
                }
            }
        }
    }
    test_authentic_door_boundaries(user_data, user_data_size, variant);
    printf("  authentic stair cells=%d, approached and tested=%d, ",
           authentic_stair_cells, tested_stair_cells);
    printf("query/move fail-closed and transactional=%d\n",
           blocked_stair_cells);
    CHECK_INT("all seven authentic campaign dungeons load for stair checks",
              loaded_dungeons, THERON_DUNGEON_COUNT);
    CHECK_INT("regional authentic stair census matches the locked source",
              authentic_stair_cells, expected_stair_cells);
    CHECK_INT("regional approachable-stair coverage matches the locked source",
              tested_stair_cells, expected_tested_stair_cells);
    CHECK_INT("every approachable authentic stair fails closed atomically",
              tested_stair_cells, blocked_stair_cells);
    free(user_data);
    free(world);
}

/* Exercise the source-backed movement rules across every authenticated
 * dungeon level, not just Akutuba's startup map.  The source loader supplies
 * every grid; this routine selects existing floor/wall neighbours and never
 * creates map tiles, objects, or a transition destination. */
static void test_real_campaign_movement(
    const uint8_t *data, size_t size, const char *md5,
    Theron_Track02Variant variant) {
    uint8_t *user_data = NULL;
    size_t sector_count = 0u, user_data_size = 0u, copied_size = 0u;
    Theron_V1_World *world = NULL;
    int verified_levels = 0;
    int floor_level_checks = 0;
    int wall_level_checks = 0;
    int failed_dungeons = 0;

    printf("[test:real_campaign_movement]\n");
    if (theron_v1_track02_raw_user_data_size(
            size, md5, &sector_count, &user_data_size) !=
            THERON_TRACK02_SIGNAL_OK || sector_count == 0u ||
        !(user_data = (uint8_t *)malloc(user_data_size)) ||
        theron_v1_track02_copy_raw_user_data(
            data, size, md5, user_data, user_data_size, &copied_size) !=
            THERON_TRACK02_SIGNAL_OK || copied_size != user_data_size) {
        printf("  [FAIL] normalize authentic campaign sectors\n");
        g_fail++;
        free(user_data);
        return;
    }
    world = (Theron_V1_World *)calloc(1u, sizeof(*world));
    if (!world) {
        printf("  [FAIL] allocate authentic campaign world\n");
        g_fail++;
        free(user_data);
        return;
    }

    for (int dungeon_id = 1; dungeon_id <= THERON_DUNGEON_COUNT;
         ++dungeon_id) {
        Theron_DungeonLoadResult result;
        int dungeon_levels = 0;
        int dungeon_floor_moves = 0;
        int dungeon_wall_blocks = 0;
        theron_v1_world_init(world);
        world->current_dungeon = dungeon_id;
        if (theron_v1_track02_load_full_dungeon_for_variant(
                world, dungeon_id, user_data, user_data_size,
                variant, &result) != 0 || result.levels_loaded <= 0) {
            printf("  [FAIL] load authentic dungeon %d\n", dungeon_id);
            g_fail++;
            failed_dungeons++;
            continue;
        }

        for (int level_index = 0; level_index < result.levels_loaded;
             ++level_index) {
            const Theron_V1_Level *level =
                &world->levels[dungeon_id - 1][level_index];
            int floor_x = -1, floor_y = -1;
            int floor_to_x = -1, floor_to_y = -1;
            int wall_x = -1, wall_y = -1;
            int wall_to_x = -1, wall_to_y = -1;
            if (!level->source_header_verified) {
                printf("  [FAIL] dungeon %d level %d lacks verified header\n",
                       dungeon_id, level_index);
                g_fail++;
                continue;
            }
            verified_levels++;
            dungeon_levels++;
            for (int y = 1; y < level->height - 1; ++y) {
                for (int x = 1; x < level->width - 1; ++x) {
                    if (level->squares[y][x] != THERON_SQUARE_FLOOR) continue;
                    if (floor_x < 0 && find_adjacent_floor(
                            level, x, y, &floor_to_x, &floor_to_y)) {
                        floor_x = x;
                        floor_y = y;
                    }
                    if (wall_x < 0 && find_adjacent_wall(
                            level, x, y, &wall_to_x, &wall_to_y)) {
                        wall_x = x;
                        wall_y = y;
                    }
                }
            }

            world->current_level = level_index;
            if (floor_x >= 0) {
                world->party.leader_x = floor_x;
                world->party.leader_y = floor_y;
                world->party.leader_dir = direction_from_delta(
                    floor_to_x - floor_x, floor_to_y - floor_y);
                if (theron_v1_move_party_original_command(
                        world, THERON_ORIGINAL_COMMAND_MOVE_FORWARD) !=
                        THERON_MOVE_OK ||
                    world->party.leader_x != floor_to_x ||
                    world->party.leader_y != floor_to_y) {
                    printf("  [FAIL] dungeon %d level %d real floor movement\n",
                           dungeon_id, level_index);
                    g_fail++;
                } else {
                    floor_level_checks++;
                    dungeon_floor_moves++;
                }
            }

            if (wall_x >= 0) {
                world->party.leader_x = wall_x;
                world->party.leader_y = wall_y;
                world->party.leader_dir = direction_from_delta(
                    wall_to_x - wall_x, wall_to_y - wall_y);
                if (theron_v1_move_party_original_command(
                        world, THERON_ORIGINAL_COMMAND_MOVE_FORWARD) !=
                        THERON_MOVE_BLOCKED ||
                    world->party.leader_x != wall_x ||
                    world->party.leader_y != wall_y) {
                    printf("  [FAIL] dungeon %d level %d real wall blocking\n",
                           dungeon_id, level_index);
                    g_fail++;
                } else {
                    wall_level_checks++;
                    dungeon_wall_blocks++;
                }
            }
        }
        if (dungeon_levels > 0 && dungeon_floor_moves > 0 &&
            dungeon_wall_blocks > 0) {
            printf("  [PASS] dungeon %d: %d authentic levels; %d floor moves, %d wall blocks\n",
                   dungeon_id, dungeon_levels, dungeon_floor_moves,
                   dungeon_wall_blocks);
            g_pass++;
        } else {
            printf("  [FAIL] dungeon %d movement coverage: levels=%d floor=%d wall=%d\n",
                   dungeon_id, dungeon_levels, dungeon_floor_moves,
                   dungeon_wall_blocks);
            g_fail++;
            failed_dungeons++;
        }
    }
    CHECK_INT("all seven authentic dungeons load", failed_dungeons, 0);
    CHECK_INT("authentic campaign has verified levels", verified_levels > 0, 1);
    CHECK_INT("all seven dungeons exercise authentic floor movement",
              floor_level_checks > 0, 1);
    CHECK_INT("all seven dungeons exercise authentic wall blocking",
              wall_level_checks > 0, 1);
    free(world);
    free(user_data);
}

/* Run the production teleporter resolver over the coordinate-linked records
 * loaded from every authentic dungeon. Valid chains must publish a bounded
 * destination; closed, incomplete, or cyclic routes must leave party and
 * transition state untouched. */
static void test_real_campaign_teleporters(
    const uint8_t *data, size_t size, const char *md5,
    Theron_Track02Variant variant) {
    uint8_t *user_data = NULL;
    size_t sector_count = 0u, user_data_size = 0u, copied_size = 0u;
    Theron_V1_World *world = NULL;
    int source_teleporters = 0;
    int resolved = 0;
    int blocked = 0;
    int disabled = 0;
    int enabled = 0;
    int enabled_blocked = 0;
    int blocked_bad_level = 0;
    int blocked_bad_coordinate = 0;
    int blocked_wall_target = 0;
    int blocked_missing_endpoint = 0;
    int blocked_active_chain = 0;
    int blocked_other = 0;

    printf("[test:real_campaign_teleporter_resolution]\n");
    if (theron_v1_track02_raw_user_data_size(
            size, md5, &sector_count, &user_data_size) !=
            THERON_TRACK02_SIGNAL_OK || sector_count == 0u ||
        !(user_data = (uint8_t *)malloc(user_data_size)) ||
        theron_v1_track02_copy_raw_user_data(
            data, size, md5, user_data, user_data_size, &copied_size) !=
            THERON_TRACK02_SIGNAL_OK || copied_size != user_data_size) {
        printf("  [FAIL] normalize authentic teleporter sectors\n");
        g_fail++;
        free(user_data);
        return;
    }
    world = (Theron_V1_World *)calloc(1u, sizeof(*world));
    if (!world) {
        printf("  [FAIL] allocate authentic teleporter world\n");
        g_fail++;
        free(user_data);
        return;
    }

    for (int dungeon_id = 1; dungeon_id <= THERON_DUNGEON_COUNT;
         ++dungeon_id) {
        Theron_DungeonLoadResult result;
        theron_v1_world_init(world);
        world->current_dungeon = dungeon_id;
        if (theron_v1_track02_load_full_dungeon_for_variant(
                world, dungeon_id, user_data, user_data_size,
                variant, &result) != 0 || result.levels_loaded <= 0) {
            printf("  [FAIL] load authentic dungeon %d teleporters\n",
                   dungeon_id);
            g_fail++;
            continue;
        }
        for (int i = 0; i < world->object_count; ++i) {
            Theron_V1_Object *source = &world->objects[i];
            int before_x, before_y, before_level, status;
            if (source->dungeon_id != dungeon_id ||
                source->type != THERON_OBJTYPE_TELEPORTER ||
                !(source->flags & THERON_OBJ_F_TRACK02_COORD_LINK))
                continue;
            source_teleporters++;
            if (source->state == 0u) disabled++;
            else enabled++;
            world->current_level = source->level;
            before_x = world->party.leader_x;
            before_y = world->party.leader_y;
            before_level = world->current_level;
            world->transition_pending = 0;
            world->transition_type = 0;
            world->transition_target_level = -1;
            status = theron_v1_teleporter_resolve(
                world, source->x, source->y);
            if (status == 0 && world->transition_pending &&
                world->transition_type == THERON_TRANSITION_TELEPORTER &&
                world->transition_target_level >= 0 &&
                world->transition_target_level < THERON_MAX_LEVELS_PER_DUNGEON &&
                world->level_loaded[dungeon_id - 1]
                                   [world->transition_target_level] &&
                world->party.leader_x == world->transition_spawn_x &&
                world->party.leader_y == world->transition_spawn_y) {
                resolved++;
            } else if (status < 0 && !world->transition_pending &&
                       world->transition_type == 0 &&
                       world->transition_target_level == -1 &&
                       world->party.leader_x == before_x &&
                       world->party.leader_y == before_y &&
                       world->current_level == before_level) {
                blocked++;
                if (source->state != 0u) {
                    int target_level = (source->linked_id >> 10) & 0x3f;
                    int target_x = source->linked_id & 0x1f;
                    int target_y = (source->linked_id >> 5) & 0x1f;
                    const Theron_V1_Level *target_level_data = NULL;
                    const Theron_V1_Object *target = NULL;
                    enabled_blocked++;
                    if (target_level < 0 ||
                        target_level >= THERON_MAX_LEVELS_PER_DUNGEON ||
                        !world->level_loaded[dungeon_id - 1][target_level]) {
                        blocked_bad_level++;
                    } else {
                        target_level_data =
                            &world->levels[dungeon_id - 1][target_level];
                        if (target_x < 0 || target_x >= target_level_data->width ||
                            target_y < 0 || target_y >= target_level_data->height) {
                            blocked_bad_coordinate++;
                        } else if (target_level_data->squares[target_y][target_x] ==
                                   THERON_SQUARE_WALL) {
                            blocked_wall_target++;
                        } else if (target_level_data->squares[target_y][target_x] ==
                                   THERON_SQUARE_TELEPORTER) {
                            for (int j = 0; j < world->object_count; ++j) {
                                const Theron_V1_Object *candidate =
                                    &world->objects[j];
                                if (candidate->type == THERON_OBJTYPE_TELEPORTER &&
                                    candidate->dungeon_id == dungeon_id &&
                                    candidate->level == target_level &&
                                    candidate->x == target_x &&
                                    candidate->y == target_y) {
                                    target = candidate;
                                    break;
                                }
                            }
                            if (!target || !(target->flags &
                                    THERON_OBJ_F_TRACK02_COORD_LINK)) {
                                blocked_missing_endpoint++;
                            } else if (target->state != 0u) {
                                blocked_active_chain++;
                            } else {
                                blocked_other++;
                            }
                        } else {
                            blocked_other++;
                        }
                    }
                }
            } else {
                printf("  [FAIL] dungeon %d teleporter at %d,%d violated commit/rollback boundary (status=%d)\n",
                       dungeon_id, source->x, source->y, status);
                g_fail++;
            }
        }
    }
    CHECK_INT("authentic campaign has coordinate-linked teleporters",
              source_teleporters, 335);
    CHECK_INT("authentic campaign disabled teleporter count",
              disabled, 165);
    CHECK_INT("authentic campaign enabled teleporter count", enabled, 170);
    CHECK_INT("authentic campaign resolver commits", resolved, 72);
    CHECK_INT("authentic campaign resolver fails closed", enabled_blocked, 98);
    CHECK_INT("authentic open links targeting walls remain blocked",
              blocked_wall_target, 89);
    CHECK_INT("authentic active-link chains remain blocked",
              blocked_active_chain, 9);
    CHECK_INT("every authentic teleporter commits or rolls back",
              resolved + blocked, source_teleporters);
    printf("  teleporter records=%d disabled=%d enabled=%d resolved=%d fail-closed=%d enabled-fail-closed=%d [bad-level=%d bad-coordinate=%d wall=%d missing-endpoint=%d active-chain=%d other=%d]\n",
           source_teleporters, disabled, enabled, resolved, blocked,
           enabled_blocked, blocked_bad_level, blocked_bad_coordinate,
           blocked_wall_target, blocked_missing_endpoint,
           blocked_active_chain, blocked_other);
    free(world);
    free(user_data);
}

/* ── Probe one real Track 02 image ─────────────────────────────────── */
static void probe_real_track02(const char *label,
                               const char *path,
                               const char *expected_md5,
                               Theron_Track02Variant variant) {
    char local_md5[33];
    uint8_t *data = NULL;
    size_t size = 0;
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
    Theron_Track02StartupSemanticHandoff semantic_handoff;
    Theron_Track02StartupRuntimeReceipt startup_receipt;
    Theron_V1_Level level;
    Theron_Track02LevelHandoff handoff;
    Theron_Track02LevelHandoffStatus status;
    Theron_V1_World world;

    printf("\n== %s: %s ==\n", label, path);

    if (!file_exists(path)) {
        printf("SKIP %s: no Track 02 image at %s\n", label, path);
        g_skip++;
        return;
    }
    if (!m12_file_md5_hex(path, local_md5)) {
        printf("FAIL %s: could not compute MD5 for %s\n", label, path);
        g_fail++;
        return;
    }
    if (strcmp(local_md5, expected_md5) != 0) {
        printf("FAIL %s: MD5 %s does not match expected %s\n",
               label, local_md5, expected_md5);
        g_fail++;
        return;
    }
    if (!(data = read_file(path, &size))) {
        printf("FAIL %s: could not read %s\n", label, path);
        g_fail++;
        return;
    }

    memset(&signal, 0, sizeof(signal));
    signal_status = theron_v1_track02_find_bank_signal(data, size,
                                                       local_md5, &signal);
    if (signal_status != THERON_TRACK02_SIGNAL_OK || signal.anchor_count == 0u) {
        printf("FAIL %s: bank signal status %s anchors=%zu\n",
               label,
               theron_v1_track02_signal_status_name(signal_status),
               signal.anchor_count);
        g_fail++;
        free(data);
        return;
    }

    memset(&semantic_handoff, 0, sizeof(semantic_handoff));
    memset(&startup_receipt, 0, sizeof(startup_receipt));
    CHECK_INT("real startup semantic handoff uses authenticated level header",
              theron_v1_track02_bind_startup_semantic_handoff(
                  data, size, local_md5, signal.descriptor_offsets[0],
                  &semantic_handoff),
              THERON_TRACK02_LEVEL_HANDOFF_OK);
    CHECK_INT("real startup semantic handoff is runtime-ready",
              semantic_handoff.ready_for_runtime, 1);
    CHECK_INT("real startup seed comes from level header",
              (int)semantic_handoff.startup_seed, (int)0x0108e938u);
    CHECK_INT("real startup runtime receipt is valid without synthetic table",
              theron_v1_track02_startup_runtime_receipt_from_handoff(
                  &semantic_handoff, &startup_receipt), 1);
    CHECK_INT("real runtime progression seed is the authentic header seed",
              (int)startup_receipt.progression_seed0, (int)0x0108e938u);
    CHECK_INT("real runtime receipt keeps fallback visuals blocked",
              startup_receipt.fallback_visuals_allowed, 0);

    memset(&level, 0, sizeof(level));
    memset(&handoff, 0, sizeof(handoff));
    status = theron_v1_track02_load_initial_level_candidate(
        data,
        size,
        local_md5,
        signal.descriptor_offsets[0],
        THERON_DUNGEON_1_AKUTUBA,
        0,
        &level,
        &handoff);

    if (status != THERON_TRACK02_LEVEL_HANDOFF_OK || !handoff.loaded) {
        printf("FAIL %s: initial level load status=%s loaded=%d\n",
               label,
               theron_v1_track02_level_handoff_status_name(status),
               handoff.loaded);
        g_fail++;
        free(data);
        return;
    }

    test_real_level_loaded(&level);

    setup_world_from_level(&world, &level);
    test_turning_on_real_grid(&world);

    setup_world_from_level(&world, &level);
    test_wall_blocking_on_real_grid(&world, &level);

    setup_world_from_level(&world, &level);
    test_floor_movement_on_real_grid(&world, &level);

    setup_world_from_level(&world, &level);
    test_original_commands_on_real_grid(&world, &level);

    setup_world_from_level(&world, &level);
    test_get_move_result_on_real_grid(&world, &level);

    setup_world_from_level(&world, &level);
    test_creature_spawn_and_combat_gate(&world, &level);

    setup_world_from_level(&world, &level);
    test_creature_drop(&world, &level);

    setup_world_from_level(&world, &level);
    test_object_table_decode_and_apply_real_data(data, size, local_md5, &world);

    test_real_full_dungeon_and_stairs(data, size, local_md5, variant);
    test_real_campaign_movement(data, size, local_md5, variant);
    test_real_campaign_teleporters(data, size, local_md5, variant);

    free(data);
}

/* ── Entry point ───────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    const char *data_dir;
    char path_us[1024];
    char path_jp[1024];

    printf("Theron V1 real-data mechanics playability probe\n");
    printf("Source: THQUEST.ASM T520/T600 + real JP/US Track 02 BINs\n\n");

    data_dir = resolve_data_dir(argc, argv);
    build_path(path_us, sizeof(path_us), data_dir, "TQUS02.bin");
    build_path(path_jp, sizeof(path_jp), data_dir, "TQJP02.bin");

    test_sound_validation();

    probe_real_track02("US", path_us, THERON_TRACK02_MD5_US_BIN,
                       THERON_TRACK02_VARIANT_US_BIN);
    probe_real_track02("JP", path_jp, THERON_TRACK02_MD5_JP_BIN,
                       THERON_TRACK02_VARIANT_JP_BIN);

    printf("\n== Summary ==\n");
    printf("PASS: %d  FAIL: %d  SKIP: %d\n", g_pass, g_fail, g_skip);

    if (g_fail > 0) return 1;
    /* A probe that only skips because no real data is staged still reports
     * success; the SKIP count tells the caller no positive real-data evidence
     * was produced. */
    return 0;
}
