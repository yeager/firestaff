
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
static int g_start_x = 0, g_start_y = 0, g_start_dir = 0;

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

    /* Read DUNGEON_HEADER (ReDMCSB DEFS.H):
     * Offset 0-1: OrnamentRandomSeed
     * Offset 2-3: RawMapDataByteCount
     * Offset 4:   MapCount
     * Offset 5:   Padding
     * Offset 6-7: TextDataWordCount
     * Offset 8-9: InitialPartyLocation (bits 0-4=X, 5-9=Y, 10-11=Dir)
     * Offset 10-11: SquareFirstThingCount
     * Offset 12+:  ThingCount[16] (32 bytes) */
    level_count = data[4];
    if (level_count <= 0 || level_count > DQ_MAX_LEVELS)
        level_count = 14;

    /* Extract initial party location from header */
    {
        uint16_t ipl = r16(data + 8);
        g_start_x = ipl & 0x1F;
        g_start_y = (ipl >> 5) & 0x1F;
        g_start_dir = (ipl >> 10) & 0x03;
        printf("DUNGEON.DAT: start position (%d,%d) facing %s\n",
            g_start_x, g_start_y,
            g_start_dir == 0 ? "North" : g_start_dir == 1 ? "East" :
            g_start_dir == 2 ? "South" : "West");
    }

    /* Skip header: 12 bytes base + 32 bytes ThingCount = 44 bytes
     * But actually the map headers follow immediately after */
    off = 12 + 2 + 16 * 2; /* header + SquareFirstThingCount + ThingCount[16] */
    if (off > size) off = 14; /* fallback */

    printf("DUNGEON.DAT: %d levels, %d bytes\n", level_count, size);

    /* Read MAP headers — ReDMCSB DEFS.H MAP struct:
     * 16 bytes per map:
     *   uint16 RawMapDataByteOffset
     *   uint16 aUnreferenced
     *   uint16 bUnreferenced
     *   uint8  OffsetMapX
     *   uint8  OffsetMapY
     *   uint16 word_A: Level(6):Width(5):Height(5) — PC bit order
     *   uint16 word_B: ornament counts
     *   uint16 word_C: door/creature/difficulty
     *   uint16 word_D: floor/wall/door sets */
    int map_raw_offsets[DQ_MAX_LEVELS];
    for (lv = 0; lv < level_count && off + 16 <= size; lv++) {
        map_raw_offsets[lv] = r16(data + off);
        uint16_t word_a = r16(data + off + 8);
        int width = ((word_a >> 6) & 0x1F) + 1;
        int height = ((word_a >> 11) & 0x1F) + 1;
        if (width > DQ_MAX_W) width = DQ_MAX_W;
        if (height > DQ_MAX_H) height = DQ_MAX_H;
        g_dungeon_level_w[lv] = width;
        g_dungeon_level_h[lv] = height;
        printf("  Map %d: %dx%d (raw_offset=%d)\n", lv, width, height, map_raw_offsets[lv]);
        off += 16;
    }

    /* Square data follows map headers.
     * ReDMCSB DUNGEON.C F0150: reads column-major, 10 bytes per square.
     * But for PC34 uncompressed: square data starts after headers,
     * and each square is stored as a 10-byte record.
     *
     * Actually: RawMapDataByteCount at offset 2-3 = total bytes of raw map data.
     * Square data offset = DUNGEON_HEADER(44) + MapCount * MAP(16) = current off.
     * Each square: 5 uint16 = 10 bytes.
     * Square type is bits 0-4 of first word (element type):
     *   0=wall, 1=corridor, 2=pit, 3=stairs, 4=door, 5=teleporter, 6=fakewall */
    int sq_data_start = off;
    for (lv = 0; lv < level_count; lv++) {
        int w = g_dungeon_level_w[lv];
        int h = g_dungeon_level_h[lv];
        /* Squares stored column-major: for each column, read all rows */
        for (int col = 0; col < w; col++) {
            for (int row = 0; row < h; row++) {
                if (off + 10 <= size) {
                    uint16_t word = r16(data + off);
                    int sq_type = (word >> 5) & 0x07; /* bits 5-7 = element type (ReDMCSB M034_SQUARE_TYPE) */
                    g_dungeon_grid[lv][row][col] = (uint8_t)sq_type;
                    off += 10;
                }
            }
        }
    }

    g_dungeon_loaded = 1;
    printf("DUNGEON.DAT: parsed %d levels, level 0 is %dx%d\n",
        level_count, g_dungeon_level_w[0], g_dungeon_level_h[0]);

    /* Debug: print level 0 grid */
    printf("Level 0 (%dx%d) grid:\n", g_dungeon_level_w[0], g_dungeon_level_h[0]);
    for (int y = 0; y < g_dungeon_level_h[0] && y < 8; y++) {
        printf("  y=%2d: ", y);
        for (int x = 0; x < g_dungeon_level_w[0] && x < 18; x++) {
            printf("%d", g_dungeon_grid[0][y][x]);
        }
        printf("\n");
    }
    printf("  Start: (%d,%d) dir=%d\n", g_start_x, g_start_y, g_start_dir);

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

int fs_dungeon_get_start_x(void) { return g_start_x; }
int fs_dungeon_get_start_y(void) { return g_start_y; }
int fs_dungeon_get_start_dir(void) { return g_start_dir; }
