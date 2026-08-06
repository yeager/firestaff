
#include "firestaff_dungeon_query.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
static uint8_t g_dungeon_random_wall_ornament_count[DQ_MAX_LEVELS];
static uint8_t g_dungeon_random_floor_ornament_count[DQ_MAX_LEVELS];
static int g_dungeon_level_count = 0;
static int g_current_level = 0;
static int g_dungeon_loaded = 0;
static int g_start_x = 0, g_start_y = 0, g_start_dir = 0;
static uint16_t g_ornament_random_seed = 0;
static uint16_t *g_columns_cumulative_sft = NULL;
static uint16_t *g_square_first_things = NULL;
static uint8_t *g_thing_bytes[16];
static uint16_t g_thing_counts[16];
static int g_column_count = 0;
static int g_square_first_thing_count = 0;

static uint16_t r16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

static const uint8_t g_thing_byte_count[16] = {
    4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
};

static void fs_dungeon_clear_thing_lookup(void) {
    int type;
    free(g_columns_cumulative_sft);
    free(g_square_first_things);
    g_columns_cumulative_sft = NULL;
    g_square_first_things = NULL;
    g_column_count = 0;
    g_square_first_thing_count = 0;
    for (type = 0; type < 16; ++type) {
        free(g_thing_bytes[type]);
        g_thing_bytes[type] = NULL;
        g_thing_counts[type] = 0;
    }
}

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
    fs_dungeon_clear_thing_lookup();
    g_dungeon_level_count = 0;
    memset(g_dungeon_grid, 0, sizeof(g_dungeon_grid));
    memset(g_dungeon_attributes, 0, sizeof(g_dungeon_attributes));
    memset(g_dungeon_level_w, 0, sizeof(g_dungeon_level_w));
    memset(g_dungeon_level_h, 0, sizeof(g_dungeon_level_h));
    memset(g_dungeon_random_wall_ornament_count, 0,
           sizeof(g_dungeon_random_wall_ornament_count));
    memset(g_dungeon_random_floor_ornament_count, 0,
           sizeof(g_dungeon_random_floor_ornament_count));
    g_current_level = 0;
    g_start_x = g_start_y = g_start_dir = 0;
    g_ornament_random_seed = 0;

    if (!data || size < 44) return -1;
    level_count = data[4];
    if (level_count <= 0 || level_count > DQ_MAX_LEVELS) return -1;

    raw_map_byte_count = r16(data + 2);
    g_ornament_random_seed = r16(data);
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
    g_columns_cumulative_sft = (uint16_t *)calloc((size_t)total_columns,
                                                   sizeof(*g_columns_cumulative_sft));
    if (!g_columns_cumulative_sft) return -1;
    for (lv = 0; lv < total_columns; ++lv) {
        g_columns_cumulative_sft[lv] = r16(data + cursor + (size_t)lv * 2u);
    }
    g_column_count = total_columns;
    cursor += (size_t)total_columns * 2u;
    if ((size_t)sft_count > (SIZE_MAX - cursor) / 2u) return -1;
    if (sft_count > 0) {
        g_square_first_things = (uint16_t *)calloc((size_t)sft_count,
                                                    sizeof(*g_square_first_things));
        if (!g_square_first_things) {
            fs_dungeon_clear_thing_lookup();
            return -1;
        }
        for (lv = 0; lv < sft_count; ++lv) {
            g_square_first_things[lv] = r16(data + cursor + (size_t)lv * 2u);
        }
    }
    g_square_first_thing_count = sft_count;
    cursor += (size_t)sft_count * 2u;
    if ((size_t)text_word_count > (SIZE_MAX - cursor) / 2u) return -1;
    cursor += (size_t)text_word_count * 2u;
    for (lv = 0; lv < 16; ++lv) {
        size_t bytes = (size_t)thing_counts[lv] * thing_bytes[lv];
        if (bytes > (size_t)size - cursor) {
            fs_dungeon_clear_thing_lookup();
            return -1;
        }
        g_thing_counts[lv] = thing_counts[lv];
        if (bytes > 0) {
            g_thing_bytes[lv] = (uint8_t *)malloc(bytes);
            if (!g_thing_bytes[lv]) {
                fs_dungeon_clear_thing_lookup();
                return -1;
            }
            memcpy(g_thing_bytes[lv], data + cursor, bytes);
        }
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
        int random_wall_count = (r16(map + 10) >> 4) & 0x0F;
        int floor_count = (r16(map + 10) >> 8) & 0x0F;
        int random_floor_count = (r16(map + 10) >> 12) & 0x0F;
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
        g_dungeon_random_wall_ornament_count[lv] =
            (uint8_t)random_wall_count;
        g_dungeon_random_floor_ornament_count[lv] =
            (uint8_t)random_floor_count;
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

static uint16_t fs_dungeon_next_thing(uint16_t thing) {
    unsigned int type = (thing >> 10) & 0x0fu;
    unsigned int index = thing & 0x03ffu;
    if (type >= 16 || index >= g_thing_counts[type] ||
        !g_thing_bytes[type] || g_thing_byte_count[type] < 2) {
        return 0xfffeu;
    }
    return r16(g_thing_bytes[type] + (size_t)index * g_thing_byte_count[type]);
}

static int fs_dungeon_sensor_ornament_override(int x, int y, int wanted_cell) {
    int column = x;
    int row;
    int index;
    uint16_t thing;
    int guard = 0;

    if (!g_square_first_things || !g_columns_cumulative_sft) return -1;
    for (row = 0; row < g_current_level; ++row) {
        column += g_dungeon_level_w[row];
    }
    if (column < 0 || column >= g_column_count) return -1;
    index = g_columns_cumulative_sft[column];
    for (row = 0; row < y; ++row) {
        if (g_dungeon_attributes[g_current_level][row][x] & 0x10) ++index;
    }
    if (index < 0 || index >= g_square_first_thing_count) return -1;
    thing = g_square_first_things[index];
    while (thing != 0xffffu && thing != 0xfffeu && guard++ < 64) {
        unsigned int type = (thing >> 10) & 0x0fu;
        unsigned int item = thing & 0x03ffu;
        if (type == 3 && item < g_thing_counts[3] && g_thing_bytes[3] &&
            (wanted_cell < 0 || ((thing >> 14) & 0x03u) ==
                                (unsigned int)(wanted_cell & 3))) {
            uint16_t bits = r16(g_thing_bytes[3] + (size_t)item * 8u + 4u);
            return (int)((bits >> 12) & 0x0fu);
        }
        thing = fs_dungeon_next_thing(thing);
    }
    return -1;
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
    static const uint8_t front_face_mask[4] = { 0x02, 0x01, 0x08, 0x04 };
    uint8_t square;
    uint16_t value1;
    uint16_t value2;
    uint32_t mixed;
    int random_count;
    int random_index;
    int sensor_ordinal;

    if (!g_dungeon_loaded || x < 0 || y < 0 ||
        x >= g_dungeon_level_w[g_current_level] ||
        y >= g_dungeon_level_h[g_current_level] || dir < 0 || dir > 3) {
        return 0;
    }
    square = (uint8_t)((g_dungeon_grid[g_current_level][y][x] << 5) |
                       g_dungeon_attributes[g_current_level][y][x]);
    /* F0172 calculates the random wall stream independently from the
     * Thing-list flag. A C03 sensor on the viewed wall cell then replaces it;
     * another kind of Thing, or a sensor on another cell, does not erase the
     * calculated source ordinal. */
    if ((square >> 5) != 0) {
        return 0;
    }
    random_count = g_dungeon_random_wall_ornament_count[g_current_level];
    random_index = 30;
    if ((square & front_face_mask[dir]) != 0 && random_count > 0) {
        /* ReDMCSB DUNGEON.C F0171's front-face call is its second direction
         * increment: sourceY=(y+1)*(((dir+2)&3)+1), then F0170 mixes it with
         * the exact DUNGEON.DAT seed and current-map dimensions. */
        value1 = (uint16_t)(2000 + x * 32 +
                            (y + 1) * (((dir + 2) & 3) + 1));
        value2 = (uint16_t)(3000 + g_current_level * 64 +
                            g_dungeon_level_w[g_current_level] +
                            g_dungeon_level_h[g_current_level]);
        mixed = (((uint32_t)value1 * 31417u) >> 1) +
                ((uint32_t)value2 * 11u) + g_ornament_random_seed;
        random_index = (int)((mixed >> 2) % 30u);
    }
    sensor_ordinal = -1;
    if (square & 0x10) {
        /* F0172's viewed wall face is opposite the viewing direction. */
        sensor_ordinal = fs_dungeon_sensor_ornament_override(
            x, y, (dir + 2) & 3);
    }
    if (sensor_ordinal >= 0) return sensor_ordinal;
    return random_index < random_count ? random_index + 1 : 0;
}

int fs_dungeon_get_floor_ornament(int x, int y) {
    uint8_t square;
    uint16_t value1;
    uint16_t value2;
    uint32_t mixed;
    int element;
    int random_count;
    int random_index;
    int sensor_ordinal;

    if (!g_dungeon_loaded || x < 0 || y < 0 ||
        x >= g_dungeon_level_w[g_current_level] ||
        y >= g_dungeon_level_h[g_current_level]) {
        return 0;
    }
    square = (uint8_t)((g_dungeon_grid[g_current_level][y][x] << 5) |
                       g_dungeon_attributes[g_current_level][y][x]);
    element = square >> 5;
    /* F0172 admits random floor ornament only on corridor/pit/teleporter
     * cells. A source sensor overrides that value but a non-sensor chain does
     * not remove it. */
    if ((element != 1 && element != 2 && element != 5) ||
        !g_dungeon_loaded) {
        return 0;
    }
    random_count = g_dungeon_random_floor_ornament_count[g_current_level];
    random_index = 30;
    if ((square & 0x08) != 0 && random_count > 0) {
        value1 = (uint16_t)(2000 + x * 32 + y);
        value2 = (uint16_t)(3000 + g_current_level * 64 +
                            g_dungeon_level_w[g_current_level] +
                            g_dungeon_level_h[g_current_level]);
        mixed = (((uint32_t)value1 * 31417u) >> 1) +
                ((uint32_t)value2 * 11u) + g_ornament_random_seed;
        random_index = (int)((mixed >> 2) % 30u);
    }
    sensor_ordinal = -1;
    if (square & 0x10) {
        sensor_ordinal = fs_dungeon_sensor_ornament_override(x, y, -1);
    }
    if (sensor_ordinal >= 0) return sensor_ordinal;
    return random_index < random_count ? random_index + 1 : 0;
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
