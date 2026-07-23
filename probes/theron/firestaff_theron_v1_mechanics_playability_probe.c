/*
 * firestaff_theron_v1_mechanics_playability_probe.c
 *
 * Theron's Quest V1 — Real-Data Mechanics Playability Probe
 *
 * Headless mechanics verification against the authentic JP/US Track 02
 * Hall-of-Records level-0 grid.  Unlike the synthetic cross-route probe,
 * this probe loads the real startup candidate bytes and exercises movement,
 * turning, and blocking on the actual 32x27 loader-accepted grid.
 *
 * Run:
 *   ./build/firestaff_theron_v1_mechanics_playability_probe
 *   FIRESTAFF_THERON_DATA_DIR=/path/to/data ./build/firestaff_theron_v1_mechanics_playability_probe
 *   ctest --test-dir build -R theron_v1_mechanics_playability -j4 --output-on-failure
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
#include "theron_v1_world.h"

#if defined(_WIN32)
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

/* ── Audio stubs required by mechanics.c link ───────────────────────── */
int theron_v1_play_sound(Theron_SoundID id) {
    (void)id;
    return 0;
}

int theron_v1_sound_is_valid(Theron_SoundID id) {
    return id >= 0 && id < THERON_SOUND_COUNT;
}

/* ── Combat/creature stubs required by mechanics.c link ─────────────── */
void theron_v1_champion_die(Theron_V1_World *w, int s) {
    (void)w;
    (void)s;
}

void theron_v1_creature_ai_tick(Theron_V1_World *w) {
    (void)w;
}

Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *w,
                                           int lvl,
                                           int x,
                                           int y) {
    (void)w;
    (void)lvl;
    (void)x;
    (void)y;
    return NULL;
}

int theron_v1_champion_attack(Theron_V1_World *w,
                              int champ_slot,
                              int creature_id) {
    (void)w;
    (void)champ_slot;
    (void)creature_id;
    return -1;
}

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
    if (!home || !home[0]) home = "/Users/bosse";
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

static int direction_from_delta(int dx, int dy) {
    if (dy < 0) return THERON_DIR_NORTH;
    if (dy > 0) return THERON_DIR_SOUTH;
    if (dx > 0) return THERON_DIR_EAST;
    if (dx < 0) return THERON_DIR_WEST;
    return THERON_DIR_NORTH;
}

/* ── World setup ───────────────────────────────────────────────────── */
static void setup_world_from_level(Theron_V1_World *world,
                                   const Theron_V1_Level *level) {
    memset(world, 0, sizeof(*world));
    world->current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    world->current_level = 0;
    world->level_loaded[0][0] = 1;
    memcpy(&world->levels[0][0], level, sizeof(*level));

    world->party.leader_x = level->start_x;
    world->party.leader_y = level->start_y;
    world->party.leader_dir = level->start_dir;
    world->party.active_slot = 0;
    world->party.gold = 1000;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &world->party.champions[i];
        c->alive = 1;
        c->health = 50;
        c->max_health = 50;
        c->stamina = 50;
        c->max_stamina = 50;
        c->food = 50;
        c->water = 50;
    }
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

/* ── Probe one real Track 02 image ─────────────────────────────────── */
static void probe_real_track02(const char *label,
                               const char *path,
                               const char *expected_md5) {
    char local_md5[33];
    uint8_t *data = NULL;
    size_t size = 0;
    Theron_Track02BankSignal signal;
    Theron_Track02SignalStatus signal_status;
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

    memset(&level, 0, sizeof(level));
    memset(&handoff, 0, sizeof(handoff));
    status = theron_v1_track02_load_initial_level_candidate(
        data,
        size,
        local_md5,
        signal.descriptor_offsets[0],
        THERON_DUNGEON_1_HALL_OF_RECORDS,
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
    test_get_move_result_on_real_grid(&world, &level);

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

    probe_real_track02("US", path_us, THERON_TRACK02_MD5_US_BIN);
    probe_real_track02("JP", path_jp, THERON_TRACK02_MD5_JP_BIN);

    printf("\n== Summary ==\n");
    printf("PASS: %d  FAIL: %d  SKIP: %d\n", g_pass, g_fail, g_skip);

    if (g_fail > 0) return 1;
    /* A probe that only skips because no real data is staged still reports
     * success; the SKIP count tells the caller no positive real-data evidence
     * was produced. */
    return 0;
}
