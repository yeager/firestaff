
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
static uint8_t g_dungeon_attributes[DQ_MAX_LEVELS][DQ_MAX_H][DQ_MAX_W];
static int g_dungeon_level_w[DQ_MAX_LEVELS];
static int g_dungeon_level_h[DQ_MAX_LEVELS];
static int g_dungeon_level_count = 0;
static int g_current_level = 0;
static int g_dungeon_loaded = 0;
static int g_start_x = 0, g_start_y = 0, g_start_dir = 0;

static uint16_t r16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

int fs_dungeon_load_dat(const uint8_t *data, int size) {
    /* ReDMCSB DUNGEON.C F0150/F0151 and DMWeb's Dungeon Files format:
     * header (44), MAP descriptors (16 each), column SFT bases, SFT table,
     * text words, all typed thing records, then the raw map block.  The old
     * reader jumped directly from the descriptors to the raw map bytes (or
     * guessed from EOF), so it could interpret object bytes as walls. */
    static const unsigned char thing_bytes[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    int level_count;
    int lv;
    int total_columns = 0;
    size_t cursor;
    size_t raw_base;
    uint16_t raw_map_byte_count;
    uint16_t sft_count;
    uint16_t text_word_count;
    uint16_t thing_counts[16];

    g_dungeon_loaded = 0;
    g_dungeon_level_count = 0;
    memset(g_dungeon_grid, 0, sizeof(g_dungeon_grid));
    memset(g_dungeon_attributes, 0, sizeof(g_dungeon_attributes));
    memset(g_dungeon_level_w, 0, sizeof(g_dungeon_level_w));
    memset(g_dungeon_level_h, 0, sizeof(g_dungeon_level_h));
    g_current_level = 0;
    g_start_x = g_start_y = g_start_dir = 0;

    if (!data || size < 44) return -1;
    level_count = data[4];
    if (level_count <= 0 || level_count > DQ_MAX_LEVELS) return -1;

    raw_map_byte_count = r16(data + 2);
    text_word_count = r16(data + 6);
    sft_count = r16(data + 10);
    for (lv = 0; lv < 16; ++lv) {
        thing_counts[lv] = r16(data + 12 + lv * 2);
    }

    {
        uint16_t ipl = r16(data + 8);
        g_start_x = ipl & 0x1F;
        g_start_y = (ipl >> 5) & 0x1F;
        g_start_dir = (ipl >> 10) & 0x03;
    }

    cursor = 44;
    if (cursor + (size_t)level_count * 16u > (size_t)size) return -1;
    for (lv = 0; lv < level_count; ++lv) {
        const uint8_t *map = data + cursor + (size_t)lv * 16u;
        uint16_t word_a = r16(map + 8);
        int width = (int)((word_a >> 6) & 0x1F) + 1;
        int height = (int)((word_a >> 11) & 0x1F) + 1;
        g_dungeon_level_w[lv] = width;
        g_dungeon_level_h[lv] = height;
        total_columns += width;
    }
    cursor += (size_t)level_count * 16u;

    /* The raw map base is derived from the complete typed-data prefix, not
     * from a filename, EOF guess, or a fallback offset. */
    if (cursor > (size_t)size ||
        (size_t)total_columns > (SIZE_MAX - cursor) / 2u) return -1;
    cursor += (size_t)total_columns * 2u;
    if ((size_t)sft_count > (SIZE_MAX - cursor) / 2u) return -1;
    cursor += (size_t)sft_count * 2u;
    if ((size_t)text_word_count > (SIZE_MAX - cursor) / 2u) return -1;
    cursor += (size_t)text_word_count * 2u;
    for (lv = 0; lv < 16; ++lv) {
        size_t bytes = (size_t)thing_counts[lv] * thing_bytes[lv];
        if (bytes > (size_t)size - cursor) return -1;
        cursor += bytes;
    }
    raw_base = cursor;
    if (raw_map_byte_count == 0 || raw_base > (size_t)size ||
        (size_t)raw_map_byte_count > (size_t)size - raw_base) return -1;

    for (lv = 0; lv < level_count; ++lv) {
        const uint8_t *map = data + 44u + (size_t)lv * 16u;
        size_t map_offset = r16(map);
        int width = g_dungeon_level_w[lv];
        int height = g_dungeon_level_h[lv];
        int creature_count = (r16(map + 12) >> 4) & 0x0F;
        int wall_count = r16(map + 10) & 0x0F;
        int floor_count = (r16(map + 10) >> 8) & 0x0F;
        int door_count = r16(map + 12) & 0x0F;
        size_t square_count = (size_t)width * (size_t)height;
        size_t map_span = square_count + (size_t)creature_count +
            (size_t)wall_count + (size_t)floor_count + (size_t)door_count;
        size_t col;
        size_t row;

        if (map_offset > (size_t)raw_map_byte_count ||
            map_span > (size_t)raw_map_byte_count - map_offset) {
            g_dungeon_loaded = 0;
            return -1;
        }
        for (col = 0; col < (size_t)width; ++col) {
            for (row = 0; row < (size_t)height; ++row) {
                uint8_t square = data[raw_base + map_offset + col * (size_t)height + row];
                g_dungeon_grid[lv][row][col] = (uint8_t)((square >> 5) & 0x07);
                g_dungeon_attributes[lv][row][col] = square & 0x1F;
            }
        }
    }

    g_dungeon_level_count = level_count;
    g_dungeon_loaded = 1;
    return level_count;
}

void fs_dungeon_set_level(int level) {
    if (level >= 0 && level < g_dungeon_level_count)
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
    return (sq == 4) ? 1 : 0;
}

int fs_dungeon_get_door_state(int x, int y) {
    if (!g_dungeon_loaded || x < 0 || y < 0 ||
        x >= g_dungeon_level_w[g_current_level] ||
        y >= g_dungeon_level_h[g_current_level] ||
        g_dungeon_grid[g_current_level][y][x] != 4) {
        return 0;
    }
    return g_dungeon_attributes[g_current_level][y][x] & 0x07;
}

int fs_dungeon_get_wall_ornament(int x, int y, int dir) {
    /* The former arithmetic here invented an ornament ordinal from map
     * coordinates and magic constants.  F0170/F0172 select real ordinals
     * through the original runtime's random state and aspect tables; this
     * legacy bridge has neither receipt.  Do not paint a plausible wall.
     * M11's source-owned viewport remains the only ornament consumer. */
    (void)x;
    (void)y;
    (void)dir;
    return 0;
}

int fs_dungeon_get_floor_ornament(int x, int y) {
    /* See fs_dungeon_get_wall_ornament: without F0170/F0172's authenticated
     * random/aspect state, no floor-ornament ordinal is source-owned. */
    (void)x;
    (void)y;
    return 0;
}

int fs_dungeon_get_width(void) {
    return g_dungeon_loaded && g_current_level < g_dungeon_level_count
        ? g_dungeon_level_w[g_current_level] : 0;
}

int fs_dungeon_get_height(void) {
    return g_dungeon_loaded && g_current_level < g_dungeon_level_count
        ? g_dungeon_level_h[g_current_level] : 0;
}

const uint8_t *fs_dungeon_get_grid(void) {
    if (!g_dungeon_loaded) return NULL;
    return &g_dungeon_grid[g_current_level][0][0];
}

int fs_dungeon_get_start_x(void) { return g_start_x; }
int fs_dungeon_get_start_y(void) { return g_start_y; }
int fs_dungeon_get_start_dir(void) { return g_start_dir; }
