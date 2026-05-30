
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
 *   Level descriptors follow header, 8 bytes each:
 *     byte[0]: level_type (0=OUTDOOR, 1=INDOOR, 2=BUILDING)
 *     byte[1]: level_width (1-64)
 *     byte[2]: level_height (1-64)
 *     bytes[4-5]: offset low word (LE uint16)
 *     bytes[6-7]: offset high word (LE uint16)
 *   Tile data: column-major uint16[level_width * level_height]
 *   Square type stored in lower 5 bits (0x1F mask).
 *
 *   Confirmed against: SKULL.ASM T560 DUNGEON_Load, local DUNGEON.DAT probe.
 *   NOTE: Current loader reads level_count from byte offset 0 (returns 0).
 *   Correct reading: byte offset 6. Stub needs SKULL.ASM confirmation update. */

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
    uint8_t *raw_data;
    int raw_size;
    /* DM2 outdoor extension */
    int sky_texture_index;
    int weather_zone_count;
} DM2_V1_DungeonData;

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out, const uint8_t *dat, int size);
int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_get_tile_raw(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_is_outdoor(const DM2_V1_DungeonData *d, int level);
void dm2_v1_dungeon_free(DM2_V1_DungeonData *d);
const char *dm2_v1_dungeon_source_evidence(void);
#endif

