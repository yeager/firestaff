/*
 * Nexus V1 Mechanics Playability Probe — Real DGN Levels 00–15
 * ==============================================================
 * Headless mechanics verification against the authentic retail LEV*.DGN
 * geometry.  Unlike the synthetic parity probe, this probe loads the real
 * Track 1 level bytes for every level and exercises movement / blocking / door
 * gates on the actual 64x64 Structure1B grid.
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_nexus_v1_mechanics_playability_probe
 *   SDL_VIDEODRIVER=dummy \
 *     FIRESTAFF_NEXUS_DATA_DIR=/path/to/data ./build/firestaff_nexus_v1_mechanics_playability_probe
 * Or:
 *   ctest --test-dir build -R firestaff_nexus_v1_mechanics_playability -j4 --output-on-failure
 *
 * Source-lock references:
 *   DMWeb DGN — http://dmweb.free.fr/ ("Dungeon Master Nexus DGN files",
 *             fetched 2026-05-28) for DGN Structure1B 64x64 8-byte/cell format.
 *   ReDMCSB — Starcraft's decompilation WIP20210206/Toolchains/Common/Source/
 *             DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C, CLIKMENU.C.
 *             Primary source lock for movement, collision, and door key checks.
 */

/* ── System headers ─────────────────────────────────────────────────── */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/* ── Nexus V1 headers ──────────────────────────────────────────────── */
#include "nexus_v1_engine.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_squares.h"

/* Nexus_V1_Engine and Nexus_V1_Level are large; keep probe frames shallow. */
#if defined(__GNUC__) || defined(__clang__)
#define PROBE_NOINLINE __attribute__((noinline))
#else
#define PROBE_NOINLINE
#endif

/* ═══════════════════════════════════════════════════════════════════════
 * CHECK macro — accumulate pass/fail counts and print result
 * ═══════════════════════════════════════════════════════════════════════ */
static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond_, msg_) do { \
    if (cond_) { \
        printf("  [PASS] %s\n", (msg_)); \
        g_pass++; \
    } else { \
        printf("  [FAIL] %s\n", (msg_)); \
        g_fail++; \
    } \
} while (0)

/* ── File helpers ───────────────────────────────────────────────────── */
static uint8_t *read_file(const char *path, int *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
            (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)length;
    return bytes;
}

static const char *resolve_data_dir(int argc, char **argv)
{
    const char *env;
    static char path[1024];
    const char *home;

    if (argc >= 2 && argv[1] && argv[1][0]) return argv[1];

    env = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    if (env && env[0]) return env;

    home = getenv("HOME");
    if (!home || !home[0]) home = "/Users/bosse";
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus", home);
    return path;
}

/* ── Geometry helpers ───────────────────────────────────────────────── */
static int count_squares(const Nexus_V1_Level *level, int type)
{
    int x, y, count = 0;
    for (y = 0; y < level->height; y++)
        for (x = 0; x < level->width; x++)
            if (level->squares[y][x] == (uint8_t)type) count++;
    return count;
}

static int find_start_square(const Nexus_V1_Level *level, int *out_x, int *out_y)
{
    int x, y;
    /* DM1 entrance is (11,29); try it first, otherwise scan for floor. */
    if (level->squares[29][11] == NEXUS_SQUARE_FLOOR) {
        *out_x = 11;
        *out_y = 29;
        return 1;
    }
    for (y = 1; y < level->height - 1; y++) {
        for (x = 1; x < level->width - 1; x++) {
            if (level->squares[y][x] == NEXUS_SQUARE_FLOOR) {
                *out_x = x;
                *out_y = y;
                return 1;
            }
        }
    }
    return 0;
}

static int find_adjacent_floor(const Nexus_V1_Level *level,
                               int sx, int sy, int *out_x, int *out_y)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int i;
    for (i = 0; i < 4; i++) {
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        if (nx >= 0 && nx < level->width && ny >= 0 && ny < level->height &&
                level->squares[ny][nx] == NEXUS_SQUARE_FLOOR) {
            *out_x = nx;
            *out_y = ny;
            return 1;
        }
    }
    return 0;
}

static int find_adjacent_wall(const Nexus_V1_Level *level,
                              int sx, int sy, int *out_x, int *out_y)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int i;
    for (i = 0; i < 4; i++) {
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        if (nx >= 0 && nx < level->width && ny >= 0 && ny < level->height &&
                level->squares[ny][nx] == NEXUS_SQUARE_WALL) {
            *out_x = nx;
            *out_y = ny;
            return 1;
        }
    }
    return 0;
}

static int find_adjacent_door(const Nexus_V1_Level *level,
                              int sx, int sy, int *out_x, int *out_y)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int i;
    for (i = 0; i < 4; i++) {
        int nx = sx + dx[i];
        int ny = sy + dy[i];
        if (nx >= 0 && nx < level->width && ny >= 0 && ny < level->height &&
                level->squares[ny][nx] == NEXUS_SQUARE_DOOR) {
            *out_x = nx;
            *out_y = ny;
            return 1;
        }
    }
    return 0;
}

static int flood_fill_reachable(const Nexus_V1_Level *level, int sx, int sy)
{
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    int visited[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE] = {{0}};
    int queue[NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE][2];
    int head = 0, tail = 0, count = 0;
    int i;

    if (sx < 0 || sx >= level->width || sy < 0 || sy >= level->height) return 0;
    if (level->squares[sy][sx] == NEXUS_SQUARE_WALL) return 0;

    visited[sy][sx] = 1;
    queue[tail][0] = sx;
    queue[tail][1] = sy;
    tail++;
    count++;

    while (head < tail) {
        int cx = queue[head][0];
        int cy = queue[head][1];
        head++;
        for (i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= level->width || ny < 0 || ny >= level->height)
                continue;
            if (visited[ny][nx]) continue;
            if (level->squares[ny][nx] == NEXUS_SQUARE_WALL) continue;
            visited[ny][nx] = 1;
            queue[tail][0] = nx;
            queue[tail][1] = ny;
            tail++;
            count++;
        }
    }
    return count;
}

/* ── Engine setup helpers ───────────────────────────────────────────── */
static void setup_minimal_engine(Nexus_V1_Engine *engine,
                                 Nexus_V1_Level *level,
                                 uint8_t *dgn_data, int dgn_size)
{
    memset(engine, 0, sizeof(*engine));
    memcpy(&engine->current_level, level, sizeof(*level));
    engine->level_loaded = 1;
    engine->audio_enabled = 1;
    engine->audio.initialized = 1;
    engine->audio.sfx_enabled = 1;
    engine->current_level_dgn_data = dgn_data;
    engine->current_level_dgn_size = dgn_size;
}

static void setup_party(Nexus_V1_Engine *engine)
{
    Nexus_V1_Champion *leader;
    nexus_v1_champions_init(&engine->champions);
    nexus_v1_champion_recruit(&engine->champions, 0);
    engine->champions.leader_index = 0;
    leader = &engine->champions.champions[0];
    /* Give the leader baseline stamina/health so movement does not kill them
     * during the short probe run. */
    leader->health = leader->max_health;
    leader->stamina = leader->max_stamina;
}

/* ═══════════════════════════════════════════════════════════════════════
 * Probe: real LEVxx.DGN playability for one level
 * ═══════════════════════════════════════════════════════════════════════ */
static PROBE_NOINLINE void probe_real_level_playability(const char *data_dir,
                                                        int level_index)
{
    char path[1024];
    uint8_t *dgn_data;
    int dgn_size;
    Nexus_V1_Level level;
    Nexus_V1_Engine engine;
    Nexus_MechanicsState st;
    int start_x, start_y;
    int fx, fy, wx, wy, dx_, dy_;
    int reachable;
    int floor_count, wall_count, door_count;

    printf("\n[Real-DGN Playability Probe -- LEV%02d.DGN]\n", level_index);
    printf("  Data dir: %s\n", data_dir);

    snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
    dgn_data = read_file(path, &dgn_size);
    CHECK(dgn_data != NULL, "LEV file loads from real data dir");
    if (!dgn_data) return;

    memset(&level, 0, sizeof(level));
    CHECK(nexus_v1_level_load(&level, dgn_data, dgn_size, level_index) == 0,
          "nexus_v1_level_load succeeds on real LEV file");
    if (level.width == 0) {
        free(dgn_data);
        return;
    }

    CHECK(level.width == 64 && level.height == 64,
          "real LEV file is 64x64");

    floor_count = count_squares(&level, NEXUS_SQUARE_FLOOR);
    wall_count = count_squares(&level, NEXUS_SQUARE_WALL);
    door_count = count_squares(&level, NEXUS_SQUARE_DOOR);
    printf("  Level %02d squares: floor=%d wall=%d door=%d\n",
           level_index, floor_count, wall_count, door_count);

    CHECK(floor_count > 0, "level has at least one floor square");

    /* LEV00 is the title/entrance level and decodes to essentially no walls.
     * Use a fallback boundary-block test for it; playable levels must have
     * real walls.  Source-lock: DMWeb DGN Structure1B cell format. */
    if (level_index == 0 && wall_count == 0) {
        printf("  [INFO] LEV00 is the non-playable title/entrance level; "
               "using OOB boundary for blocking test\n");
    }

    CHECK(find_start_square(&level, &start_x, &start_y) == 1,
          "a valid starting floor square is found");
    if (start_x == 0 && start_y == 0 && floor_count == 0) {
        free(dgn_data);
        return;
    }
    printf("  Start square: (%d,%d) type=%d\n",
           start_x, start_y, level.squares[start_y][start_x]);

    setup_minimal_engine(&engine, &level, dgn_data, dgn_size);
    setup_party(&engine);
    nexus_doors_init();

    nexus_mechanics_init(&st, start_x, start_y, NEXUS_DIR_NORTH);
    st.map_index = level_index;

    CHECK(nexus_mechanics_party_alive(&st, &engine) == 1,
          "party is alive at start");
    CHECK(st.party_x == start_x && st.party_y == start_y,
          "mechanics state starts at chosen square");

    /* Verify level boundary reads as wall. */
    CHECK(nexus_v1_level_get_square(&engine.current_level, -1, start_y) ==
          NEXUS_SQUARE_WALL,
          "OOB x=-1 returns wall");
    CHECK(nexus_v1_level_get_square(&engine.current_level, start_x, -1) ==
          NEXUS_SQUARE_WALL,
          "OOB y=-1 returns wall");
    CHECK(nexus_v1_level_get_square(&engine.current_level, 64, start_y) ==
          NEXUS_SQUARE_WALL,
          "OOB x=64 returns wall");

    /* Find an adjacent floor square and walk onto it. */
    if (find_adjacent_floor(&level, start_x, start_y, &fx, &fy)) {
        int needed_dir = NEXUS_DIR_NORTH;
        int actual_dx = fx - start_x;
        int actual_dy = fy - start_y;
        if (actual_dx == 1) needed_dir = NEXUS_DIR_EAST;
        else if (actual_dy == 1) needed_dir = NEXUS_DIR_SOUTH;
        else if (actual_dx == -1) needed_dir = NEXUS_DIR_WEST;

        st.party_dir = needed_dir;
        nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
        nexus_mechanics_tick(&st, &engine);

        CHECK(st.party_x == fx && st.party_y == fy,
              "party moves onto adjacent real floor square");

        /* Movement sets a step cooldown; clear it so the following turn
         * commands execute immediately in this headless probe. */
        st.move_cooldown_ticks = 0;

        /* Turn in place. */
        nexus_mechanics_push_command(&st, NEXUS_CMD_TURN_RIGHT);
        nexus_mechanics_tick(&st, &engine);
        CHECK(st.party_dir == (needed_dir + 1) % 4,
              "turn right rotates party on real floor");

        nexus_mechanics_push_command(&st, NEXUS_CMD_TURN_LEFT);
        nexus_mechanics_tick(&st, &engine);
        CHECK(st.party_dir == needed_dir,
              "turn left restores original facing");
    } else {
        CHECK(0, "no adjacent floor square to walk onto");
    }

    /* Verify movement blocking.  If the real DGN decoded any adjacent wall
     * squares, test them; otherwise fall back to the OOB map boundary, which
     * the level API guarantees returns wall. */
    if (find_adjacent_wall(&level, start_x, start_y, &wx, &wy)) {
        int needed_dir = NEXUS_DIR_NORTH;
        int actual_dx = wx - start_x;
        int actual_dy = wy - start_y;
        if (actual_dx == 1) needed_dir = NEXUS_DIR_EAST;
        else if (actual_dy == 1) needed_dir = NEXUS_DIR_SOUTH;
        else if (actual_dx == -1) needed_dir = NEXUS_DIR_WEST;

        nexus_mechanics_init(&st, start_x, start_y, needed_dir);
        st.map_index = level_index;
        nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
        nexus_mechanics_tick(&st, &engine);

        CHECK(st.party_x == start_x && st.party_y == start_y,
              "real wall square blocks forward movement");
    } else {
        /* No in-bounds walls decoded: verify the map edge blocks instead. */
        nexus_mechanics_init(&st, 0, start_y, NEXUS_DIR_WEST);
        st.map_index = level_index;
        nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
        nexus_mechanics_tick(&st, &engine);
        CHECK(st.party_x == 0 && st.party_y == start_y,
              "map edge (OOB) blocks westward movement");

        nexus_mechanics_init(&st, start_x, 0, NEXUS_DIR_NORTH);
        st.map_index = level_index;
        nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
        nexus_mechanics_tick(&st, &engine);
        CHECK(st.party_x == start_x && st.party_y == 0,
              "map edge (OOB) blocks northward movement");
    }

    /* Door check: if a door is adjacent to the start, verify it blocks without
     * a key and (optionally) that the correct key opens it.  Doors are rare
     * right at the entrance, so this is reported but not hard-required. */
    if (find_adjacent_door(&level, start_x, start_y, &dx_, &dy_)) {
        int needed_dir = NEXUS_DIR_NORTH;
        int actual_dx = dx_ - start_x;
        int actual_dy = dy_ - start_y;
        if (actual_dx == 1) needed_dir = NEXUS_DIR_EAST;
        else if (actual_dy == 1) needed_dir = NEXUS_DIR_SOUTH;
        else if (actual_dx == -1) needed_dir = NEXUS_DIR_WEST;

        nexus_mechanics_init(&st, start_x, start_y, needed_dir);
        st.map_index = level_index;
        nexus_mechanics_push_command(&st, NEXUS_CMD_FORWARD);
        nexus_mechanics_tick(&st, &engine);

        CHECK(st.party_x == start_x && st.party_y == start_y,
              "real door square blocks movement without key");
        printf("  [INFO] verified door blocking at (%d,%d)\n", dx_, dy_);
    } else {
        printf("  [INFO] no adjacent door found from start; skip door check\n");
    }

    /* Flood-fill reachability from start.  A playable dungeon entrance should
     * connect to a non-trivial number of passable squares. */
    reachable = flood_fill_reachable(&level, start_x, start_y);
    printf("  Reachable passable squares from start: %d\n", reachable);
    CHECK(reachable >= 10,
          "at least 10 squares are reachable from starting position");

    /* The engine was set up with a borrowed DGN buffer; clear the pointer so
     * shutdown does not attempt to free externally owned memory. */
    engine.current_level_dgn_data = NULL;
}

/* ═══════════════════════════════════════════════════════════════════════
 * Main entry point
 * ═══════════════════════════════════════════════════════════════════════ */
int main(int argc, char **argv)
{
    const char *data_dir;
    int level_index;
    int any_present = 0;

    printf("=================================================================\n");
    printf("  Nexus V1 Mechanics Playability Probe (Real LEV00-LEV15.DGN)\n");
    printf("  Source: DMWeb DGN format (64x64 grid, 8 bytes/cell)\n");
    printf("          ReDMCSB DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C\n");
    printf("=================================================================\n");

    data_dir = resolve_data_dir(argc, argv);

    for (level_index = 0; level_index < 16; level_index++) {
        char lev_path[1024];
        snprintf(lev_path, sizeof(lev_path), "%s/LEV%02d.DGN",
                 data_dir, level_index);
        if (access(lev_path, R_OK) == 0) {
            any_present = 1;
            probe_real_level_playability(data_dir, level_index);
        } else {
            printf("\n[LEV%02d.DGN not available at %s -- skipped]\n",
                   level_index, lev_path);
        }
    }

    if (!any_present) {
        printf("SKIP: no retail Nexus LEV*.DGN files available at %s\n",
               data_dir);
        printf("      Set FIRESTAFF_NEXUS_DATA_DIR or pass data dir as argv[1].\n");
        return 0; /* skip-safe: no local corpus is not a failure */
    }

    printf("\n=================================================================\n");
    printf("  Results: %d PASS, %d FAIL  (%s)\n",
           g_pass, g_fail,
           g_fail == 0 ? "ALL CHECKS PASSED" : "ONE OR MORE CHECKS FAILED");
    printf("=================================================================\n");

    return g_fail > 0 ? 1 : 0;
}
