
#ifndef FIRESTAFF_DM2_V1_DUNGEON_LOADER_H
#define FIRESTAFF_DM2_V1_DUNGEON_LOADER_H
#include <stdint.h>

/* DM2: The Legend of Skullkeep (1993)
 * Uses enhanced dungeon.dat format:
 *   - Outdoor levels (sky, trees, buildings)
 *   - Indoor dungeon levels (similar to DM1)
 *   - Multi-floor buildings within outdoor areas
 *   - Extended creature type table
 *   - Weather zones (rain, fog)
 * Source: SKULL.ASM (522128 lines disassembly) */

/* PROBE_NOTES — DM2 DUNGEON.DAT header contract (PC English, 39437 bytes):
 *
 *   Byte offset  0: uint16_le: 0x0000 (reserved/padding)
 *   Byte offset  2: uint16_le: 0x4731 ("G1" format magic/version, ASCII)
 *   Byte offset  4: uint16_le: 0x002c (44) — first level data offset or header size
 *   Byte offset  6: uint16_le: 0x001c (28) — LEVEL COUNT
 *   Byte offset  8: uint16_le: 0x0101 (257) — dungeon seed
 *   Byte offset 10: uint16_le: 0x0938 (2360) — dungeon flags/metadata
 *   Byte offset 12: uint16_le: 0x0035 (53) — ???
 *   Byte offset 14: uint16_le: 0x00d9 (217) — ???
 *   Byte offset 16: uint16_le: 0x0240 (576) — ???
 *   ...
 *   The PC G1 real-data path now reads 16-byte skproject-compatible
 *   Map_definitions from byte 44 and byte-sized column-major map squares
 *   from the trailing map-data block.  Tile type is stored in the high
 *   three bits and bit 0x10 marks a thing-list square.  The bounded
 *   legacy loader path still accepts older Firestaff synthetic 16-bit map
 *   fixtures.
 *
 *   Confirmed against: SKULL.ASM T560 DUNGEON_Load, local DUNGEON.DAT probe.
 *   Confirmed loader contract: level_count/map_count is byte offset 6. */

#define DM2_V1_MAX_LEVELS 30
#define DM2_V1_MAX_MAP_SIZE 64

typedef enum {
    DM2_LEVEL_OUTDOOR = 0,
    DM2_LEVEL_INDOOR,
    DM2_LEVEL_BUILDING,
} DM2_LevelType;

typedef struct {
    int level_count;
    DM2_LevelType level_types[DM2_V1_MAX_LEVELS];
    int level_widths[DM2_V1_MAX_LEVELS];
    int level_heights[DM2_V1_MAX_LEVELS];
    int level_offsets[DM2_V1_MAX_LEVELS];
    int map_offset_x[DM2_V1_MAX_LEVELS];
    int map_offset_y[DM2_V1_MAX_LEVELS];
    int map_door_set0[DM2_V1_MAX_LEVELS];
    int map_door_set1[DM2_V1_MAX_LEVELS];
    int square_bytes;
    int raw_map_data_base;
    int column_index_base;
    int square_first_thing_base;
    int square_first_thing_count;
    int text_data_base;
    int text_word_count;
    int thing_data_bases[16];
    int thing_type_counts[16];
    uint8_t *raw_data;
    int raw_size;
    /* DM2 outdoor extension */
    int sky_texture_index;
    int weather_zone_count;
} DM2_V1_DungeonData;

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out, const uint8_t *dat, int size);
int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_get_tile_raw(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_set_tile_raw(DM2_V1_DungeonData *d, int level, int x, int y, uint16_t raw);
int dm2_v1_dungeon_get_first_thing(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_get_next_thing(const DM2_V1_DungeonData *d, uint16_t thing);
int dm2_v1_dungeon_find_thing_of_type(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int desired_type,
    int max_steps);
const uint8_t *dm2_v1_dungeon_get_thing_record(
    const DM2_V1_DungeonData *d,
    uint16_t thing,
    int *out_type,
    int *out_index,
    int *out_size);
int dm2_v1_dungeon_is_outdoor(const DM2_V1_DungeonData *d, int level);
void dm2_v1_dungeon_free(DM2_V1_DungeonData *d);
const char *dm2_v1_dungeon_source_evidence(void);
#endif
