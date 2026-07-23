/*
 * Nexus V1 Multi-Level DGN Playability Regression Test
 * ======================================================
 * Verifies that every present retail LEV*.DGN loads as a 64x64 Structure1B
 * grid and that a passable starting square reaches at least 10 squares by
 * flood-fill.  The test is skip-safe: if no local Nexus data is staged it
 * exits with code 77.
 *
 * Source-lock:
 *   DMWeb DGN — http://dmweb.free.fr/ ("Dungeon Master Nexus DGN files",
 *             fetched 2026-05-28) for Structure1B wall/floor/door decoding.
 *   ReDMCSB — DUNGEON.C / COMMAND.C / MOVESENS.C for movement passability.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "nexus_v1_dungeon.h"
#include "nexus_v1_squares.h"

static int g_pass = 0;
static int g_fail = 0;

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

static const char *resolve_data_dir(void)
{
    const char *env = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    static char path[1024];
    const char *home;

    if (env && env[0]) return env;

    home = getenv("HOME");
    if (!home || !home[0]) home = "/Users/bosse";
    snprintf(path, sizeof(path), "%s/.firestaff/data/nexus", home);
    return path;
}

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

int main(void)
{
    const char *data_dir = resolve_data_dir();
    int level_index;
    int any_present = 0;

    printf("Nexus V1 multi-level DGN playability regression test\n");
    printf("Data dir: %s\n", data_dir);

    for (level_index = 0; level_index < 16; level_index++) {
        char path[1024];
        uint8_t *dgn_data;
        int dgn_size;
        Nexus_V1_Level level;
        int start_x, start_y;
        int floor_count, reachable;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN",
                 data_dir, level_index);
        if (access(path, R_OK) != 0) {
            printf("  [SKIP] LEV%02d.DGN not present\n", level_index);
            continue;
        }
        any_present = 1;

        dgn_data = read_file(path, &dgn_size);
        if (!dgn_data) {
            printf("  [FAIL] LEV%02d.DGN could not be read\n", level_index);
            g_fail++;
            continue;
        }

        memset(&level, 0, sizeof(level));
        if (nexus_v1_level_load(&level, dgn_data, dgn_size, level_index) != 0 ||
                level.width != 64 || level.height != 64) {
            printf("  [FAIL] LEV%02d.DGN did not load as 64x64\n", level_index);
            g_fail++;
            free(dgn_data);
            continue;
        }
        g_pass++;

        floor_count = count_squares(&level, NEXUS_SQUARE_FLOOR);
        if (floor_count <= 0) {
            printf("  [FAIL] LEV%02d.DGN has no floor squares\n", level_index);
            g_fail++;
            free(dgn_data);
            continue;
        }
        g_pass++;

        if (!find_start_square(&level, &start_x, &start_y)) {
            printf("  [FAIL] LEV%02d.DGN has no valid start square\n",
                   level_index);
            g_fail++;
            free(dgn_data);
            continue;
        }
        g_pass++;

        reachable = flood_fill_reachable(&level, start_x, start_y);
        printf("  [INFO] LEV%02d.DGN floor=%d reachable=%d start=(%d,%d)\n",
               level_index, floor_count, reachable, start_x, start_y);
        if (reachable < 10) {
            printf("  [FAIL] LEV%02d.DGN reachable <%d\n", level_index, 10);
            g_fail++;
        } else {
            g_pass++;
        }

        free(dgn_data);
    }

    if (!any_present) {
        printf("SKIP: no retail Nexus LEV*.DGN files available\n");
        return 77;
    }

    printf("Results: %d PASS, %d FAIL\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
