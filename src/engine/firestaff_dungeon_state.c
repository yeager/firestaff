
#include "firestaff_dungeon_query.h"
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════
 * DM1 DUNGEON.DAT parser — extract the map grid.
 *
 * Source: ReDMCSB DUNGEON.C F0150_DUNGEON_ReadData
 *
 * DM1 PC34 DUNGEON.DAT structure (simplified):
 *   Header: misc global data
 *   Map data: per-level, each level has dimensions + column/row counts
 *   Square data: 10 bytes per square, packed
 *
 * Square type (bits 0-4 of first word):
 *   0 = wall, 1 = open/corridor, 2 = pit, 3 = stairs,
 *   4 = door, 5 = teleporter, 6 = trick wall, 7 = ??
 *
 * For now: extract square types into a flat 32x32 grid per level.
 * ═══════════════════════════════════════════════════════════════ */

#define DQ_MAX_W 32
#define DQ_MAX_H 32
#define DQ_MAX_LEVELS 16

static uint8_t g_dungeon_grid[DQ_MAX_LEVELS][DQ_MAX_H][DQ_MAX_W];
static int g_dungeon_level_w[DQ_MAX_LEVELS];
static int g_dungeon_level_h[DQ_MAX_LEVELS];
static int g_current_level = 0;
static int g_dungeon_loaded = 0;

static uint16_t r16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

int fs_dungeon_load_dat(const uint8_t *data, int size) {
    /* DM1 PC34 DUNGEON.DAT parsing:
     * Offset 0-1: number of map levels (uint16)
     * Then per level:
     *   - x_offset, y_offset (uint16 each) — unused for grid extraction
     *   - width, height (stored as col_count-1, row_count-1)
     * Then square data follows.
     *
     * This is a simplified parser that reads the map header
     * and populates square types. */
    int off = 0;
    int level_count, lv;

    if (!data || size < 20) return -1;
    memset(g_dungeon_grid, 0, sizeof(g_dungeon_grid));

    /* Read global header */
    /* DM1 PC34: first 14 bytes = global dungeon header */
    /* Bytes 12-13 = map count (number of levels) */
    level_count = r16(data + 12);
    if (level_count <= 0 || level_count > DQ_MAX_LEVELS)
        level_count = 14; /* DM1 has 14 levels (0-13) */
    off = 14;

    printf("DUNGEON.DAT: %d levels, %d bytes\n", level_count, size);

    /* Read per-level map headers */
    /* Each map header: 8 bytes (offset_x:2, offset_y:2, col_count:1, row_count:1, pad:2) */
    for (lv = 0; lv < level_count && off + 8 <= size; lv++) {
        int col_count = data[off + 4] + 1;
        int row_count = data[off + 5] + 1;
        if (col_count > DQ_MAX_W) col_count = DQ_MAX_W;
        if (row_count > DQ_MAX_H) row_count = DQ_MAX_H;
        g_dungeon_level_w[lv] = col_count;
        g_dungeon_level_h[lv] = row_count;
        off += 8;
    }

    /* Read square data — each square is 2 words (4 bytes) in packed format.
     * Square type is bits 0-2 of the first word.
     * DM1 stores squares column-major (all rows for col 0, then col 1, etc.) */
    for (lv = 0; lv < level_count; lv++) {
        int w = g_dungeon_level_w[lv];
        int h = g_dungeon_level_h[lv];
        for (int col = 0; col < w; col++) {
            for (int row = 0; row < h; row++) {
                if (off + 2 <= size) {
                    uint16_t word = r16(data + off);
                    int sq_type = word & 0x07; /* bits 0-2 = square type */
                    g_dungeon_grid[lv][row][col] = (uint8_t)sq_type;
                    off += 2; /* skip to next square (DM1 uses variable-length entries) */
                }
            }
        }
    }

    g_dungeon_loaded = 1;
    printf("DUNGEON.DAT: parsed %d levels, level 0 is %dx%d\n",
        level_count, g_dungeon_level_w[0], g_dungeon_level_h[0]);

    /* Debug: print level 0 grid snippet around Hall of Champions */
    printf("Level 0 grid around (11,29):\n");
    for (int y = 27; y < 32 && y < g_dungeon_level_h[0]; y++) {
        printf("  y=%2d: ", y);
        for (int x = 9; x < 14 && x < g_dungeon_level_w[0]; x++) {
            printf("%d ", g_dungeon_grid[0][y][x]);
        }
        printf("\n");
    }

    return level_count;
}

void fs_dungeon_set_level(int level) {
    if (level >= 0 && level < DQ_MAX_LEVELS)
        g_current_level = level;
}

int fs_dungeon_get_square_type(int x, int y) {
    if (!g_dungeon_loaded) return 0; /* wall if not loaded */
    if (x < 0 || x >= g_dungeon_level_w[g_current_level]) return 0;
    if (y < 0 || y >= g_dungeon_level_h[g_current_level]) return 0;
    return g_dungeon_grid[g_current_level][y][x];
}

int fs_dungeon_get_door_type(int x, int y) {
    int sq = fs_dungeon_get_square_type(x, y);
    return (sq == 4) ? 1 : 0; /* 4=door → type 1 (wood) */
}

int fs_dungeon_get_door_state(int x, int y) {
    (void)x; (void)y;
    return 0; /* closed by default */
}

int fs_dungeon_get_wall_ornament(int x, int y, int dir) {
    (void)x; (void)y; (void)dir;
    return 0;
}

int fs_dungeon_get_floor_ornament(int x, int y) {
    (void)x; (void)y;
    return 0;
}

int fs_dungeon_get_width(void) {
    return g_dungeon_loaded ? g_dungeon_level_w[g_current_level] : 0;
}

int fs_dungeon_get_height(void) {
    return g_dungeon_loaded ? g_dungeon_level_h[g_current_level] : 0;
}

const uint8_t *fs_dungeon_get_grid(void) {
    if (!g_dungeon_loaded) return NULL;
    return &g_dungeon_grid[g_current_level][0][0];
}
