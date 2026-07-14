#ifndef FIRESTAFF_DM2_V1_WORLD_MODEL_H
#define FIRESTAFF_DM2_V1_WORLD_MODEL_H

/*
 * dm2_v1_world_model.h — DM2 V1 Dungeon/World Data Model
 *
 * DM2 Phase 2: Map, object, tile, and world-state ingestion.
 * Provides clean in-memory representations of DM2 dungeon data
 * with proper type systems, source-lock citations, and API surface.
 *
 * Source-locks:
 *   SKULL.ASM T560          — DUNGEON_Load: header parsing, level descriptors
 *   SKULL.ASM T520          — party placement, initial position
 *   ReDMCSB DEFS.H:985-998 — DUNGEON_HEADER (44 bytes)
 *   ReDMCSB DEFS.H:1048-1116 — MAP descriptor (16 bytes)
 *   ReDMCSB HASHBUCKET.C    — square type constants, tile bitfields
 *   docs/dm2_v1_phase2_data_formats_H2254.md (SHA256 source-lock doc)
 *   docs/dm2_dungeon_files.md (DUNGEON.DAT format audit)
 *
 * Asset hashes (verified):
 *   DM2 PC English DUNGEON.DAT:  6caccd7875009e82fe2e28e7f6d6adc0 (39,437 bytes)
 *   DM2 PC French  DUNGEON.DAT:  same (shared across PC variants)
 *   DM2 PC Jewel   DUNGEON.DAT:  same
 *   DM2 PC English GRAPHICS.DAT: 25247ede4dabb6a71e5dabdfbcd5907d (~8.6 MB)
 *
 * DM2 binary format:
 *   COMPRESSED_DUNGEON_HEADER (12 bytes):
 *     uint16_t sig             0x8104 big-endian (FTL magic)
 *     uint32_t decomp_bytes    BE decompressed byte count
 *     uint16_t dungeon_id      game identifier
 *     uint16_t _reserved
 *   DUNGEON_HEADER (44 bytes, LE, same as DM1):
 *     uint16_t ornament_random_seed
 *     uint16_t raw_map_data_byte_count
 *     uint8_t  map_count
 *     uint16_t text_data_word_count
 *     uint16_t initial_party_location
 *     uint16_t square_first_thing_count
 *     uint16_t thing_count[16]
 *   MAP descriptors: 16 bytes each
 *     uint16_t raw_map_data_byte_offset
 *     uint8_t  offset_map_x
 *     uint8_t  offset_map_y
 *     uint16_t bitfield_a (level 6bits + width-1 5bits + height-1 5bits)
 *     uint16_t bitfield_b (ornaments + floor)
 *     uint16_t bitfield_c (door/creature)
 *     uint16_t bitfield_d (sets)
 *   Tile data: column-major uint16[] per level (2 bytes per square)
 *   Thing data: 16 pools of variable-length records
 *   Text data: 16-bit word array
 *
 * Source: SKULL.ASM T560 DUNGEON_Load, ReDMCSB DEFS.H:985-998, :1048-1116
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 Dungeon Binary Format Constants */

#define DM2_FTL_SIGNATURE           0x8104u
#define DM2_COMPRESSED_HEADER_SIZE  12
#define DM2_DUNGEON_HEADER_SIZE     44   /* ReDMCSB DEFS.H:985-998 */
#define DM2_MAP_DESC_SIZE           16   /* ReDMCSB DEFS.H:1048-1116 */
#define DM2_MAX_LEVELS             30
#define DM2_MAX_MAP_SIZE            64
#define DM2_MAX_THING_RECORDS       255
#define DM2_MAX_TEXT_WORDS         8192
#define DM2_MAX_THING_TYPES         16

/* DM2 square type bitfield (lower 5 bits of uint16 tile, same as DM1).
 * Source: ReDMCSB HASHBUCKET.C, DEFS.H:385-390 */
#define DM2_SQUARE_TYPE_MASK    0x001Fu
#define DM2_SQUARE_WALL_MASK    0x0020u
#define DM2_SQUARE_CORRUPT_MASK 0x0040u
#define DM2_SQUARE_ABOVE_MASK   0x0080u

/* DM2 Square Tile Types.
 * Source: ReDMCSB DEFS.H:385-390, HASHBUCKET.C */
typedef enum {
    DM2_SQUARE_FLOOR         = 0,
    DM2_SQUARE_WALL          = 1,
    DM2_SQUARE_DOOR          = 2,
    DM2_SQUARE_FLOOR_ORNATE  = 3,
    DM2_SQUARE_SECRET_DOOR   = 4,
    DM2_SQUARE_PIT           = 5,
    DM2_SQUARE_STAIRS_UP     = 6,
    DM2_SQUARE_STAIRS_DOWN   = 7,
    DM2_SQUARE_TELEPORTER    = 8,
    DM2_SQUARE_FAKE_WALL     = 9,
    DM2_SQUARE_WATER         = 10,
    DM2_SQUARE_LAVA          = 11,
    DM2_SQUARE_ASH           = 12,
    DM2_SQUARE_INACCESSIBLE  = 13,
    DM2_SQUARE_FLOOR_14      = 14,
    DM2_SQUARE_FLOOR_15      = 15,
    DM2_SQUARE_COUNT
} DM2_SquareType;

/* DM2 Level Type.
 * Source: include/dm2_v1_dungeon_loader.h, SKULL.ASM T600 outdoor */

/* Compressed Dungeon Header (12 bytes, pre-decompression).
 * Source: SKULL.ASM T560, ReDMCSB FTL.H */
typedef struct {
    uint16_t sig;              /* 0x8104 FTL magic */
    uint32_t decompressed_bytes; /* BE decompressed size */
    uint16_t dungeon_id;
    uint16_t _reserved;
} dm2_compressed_dungeon_header_t;

/* Decompressed Dungeon Header (44 bytes, matches DM1).
 * Source: ReDMCSB DEFS.H:985-998, SKULL.ASM T560 */
typedef struct {
    uint16_t ornament_random_seed;
    uint16_t raw_map_data_byte_count;
    uint8_t  map_count;
    uint16_t text_data_word_count;
    uint16_t initial_party_location;
    uint16_t square_first_thing_count;
    uint16_t thing_count[16];
} dm2_dungeon_header_t;

/* Extract party starting position from initial_party_location.
 * bits[15:12] = direction (0=N,1=E,2=S,3=W)
 * bits[11:6]  = map Y
 * bits[5:0]   = map X
 */
static inline void dm2_unpack_party_start(uint16_t loc,
                                          int *out_x, int *out_y, int *out_dir) {
    if (out_x) *out_x = (int)(loc & 0x003Fu);
    if (out_y) *out_y = (int)((loc >> 6) & 0x003Fu);
    if (out_dir) *out_dir = (int)((loc >> 12) & 0x000Fu);
}

/* Map Descriptor (16 bytes, same as DM1 MAP descriptor).
 * Source: ReDMCSB DEFS.H:1048-1116, SKULL.ASM T560 */
typedef struct {
    uint16_t raw_map_data_byte_offset;
    uint8_t  offset_map_x;
    uint8_t  offset_map_y;
    uint16_t bitfield_a;  /* level(6) + width-1(5) + height-1(5) */
    uint16_t bitfield_b;  /* wall_orn(4)+rand_wall(4)+floor(4)+rand_floor(4) */
    uint16_t bitfield_c;  /* door_orn(4)+creature_type(4)+unreff(4)+diff(4) */
    uint16_t bitfield_d;  /* floor_set(4)+wall_set(4)+door_set0(4)+door_set1(4) */
} dm2_map_descriptor_t;

/* Accessors for packed map descriptor fields.
 * Source: ReDMCSB DEFS.H:1048-1116 */
static inline int dm2_md_level(const dm2_map_descriptor_t *md) {
    return (int)(md->bitfield_a & 0x003Fu);
}
/* DM2 PC (MEDIA016): Level=bits0-5, Width=bits6-10, Height=bits11-15.
 * Source: ReDMCSB DEFS.H:1048-1116 MEDIA016 layout. */
static inline int dm2_md_width(const dm2_map_descriptor_t *md) {
    return (int)(((md->bitfield_a >> 6) & 0x001Fu) + 1);
}
static inline int dm2_md_height(const dm2_map_descriptor_t *md) {
    return (int)(((md->bitfield_a >> 11) & 0x001Fu) + 1);
}

/* DM2 Tile (one dungeon square).
 * Source: ReDMCSB HASHBUCKET.C, DEFS.H:385-390 */
typedef struct {
    uint16_t raw;
    uint8_t  type;   /* DM2_SQUARE_TYPE (lower 5 bits) */
    uint8_t  flags;  /* WALL/CORRUPT/ABOVE mask bits */
} dm2_tile_t;

/* DM2 Level (one dungeon map).
 * Source: SKULL.ASM T560, docs/dm2_v1_phase2_data_formats_H2254.md */
typedef struct {
    int level_type;  /* 0=OUTDOOR 1=INDOOR 2=BUILDING */
    int            level_index;
    int            width;
    int            height;
    int            byte_offset;
    dm2_tile_t    *tiles;  /* width * height tiles (owned), NULL if not loaded */
    int            sky_texture_index;
    int            weather_zone_index;
} dm2_level_t;

/* DM2 Dungeon World (full in-memory model).
 * Source: SKULL.ASM T560 DUNGEON_Load, docs/dm2_v1_phase2_data_formats_H2254.md */
typedef struct {
    dm2_dungeon_header_t header;
    int                  dungeon_id;
    int                  map_count;
    dm2_map_descriptor_t map_descs[DM2_MAX_LEVELS];
    dm2_level_t          levels[DM2_MAX_LEVELS];
    /* Decompressed raw data (owned, freed on cleanup) */
    uint8_t            *raw_decompressed;
    size_t              raw_decompressed_size;
    /* Thing data pools */
    uint8_t *thing_pools[DM2_MAX_THING_TYPES];
    int       thing_pool_counts[DM2_MAX_THING_TYPES];
    /* Text data (16-bit words, owned) */
    uint16_t *text_data;
    int        text_word_count;
    /* Dungeon seed for RNG initialization */
    uint32_t dungeon_seed;
    /* Platform/version */
    int platform;
} dm2_dungeon_world_t;

/* Build world model from an in-memory DUNGEON.DAT image.
 * Handles FTL decompression if data is wrapped (starts with 0x8104).
 * Returns world on success (caller owns), NULL on failure.
 * Source: SKULL.ASM T560 DUNGEON_Load, T000 file I/O */
dm2_dungeon_world_t *dm2_world_from_mem(const uint8_t *data, size_t size);

/* Build world model from an on-disk DUNGEON.DAT file.
 * Source: SKULL.ASM T000, T560 */
dm2_dungeon_world_t *dm2_world_from_file(const char *path);

/* Get tile at dungeon position. Returns pointer or NULL.
 * Source: SKULL.ASM T520 (party placement tile access) */
const dm2_tile_t *dm2_world_get_tile(const dm2_dungeon_world_t *world,
                                      int level, int x, int y);

/* Get normalized square type (0-15) at position.
 * Returns DM2_SQUARE_COUNT if out of range.
 * Source: DM2 Phase 4 movement (wall collision checks) */
int dm2_world_get_tile_type(const dm2_dungeon_world_t *world,
                             int level, int x, int y);

/* Check if square is passable by party.
 * Returns 1 if walkable, 0 if blocked (wall, pit, lava).
 * Source: SKULL.ASM T520 movement and collision */
int dm2_world_is_walkable(const dm2_dungeon_world_t *world,
                           int level, int x, int y);

/* Check if level is outdoor.
 * Returns 1 for OUTDOOR levels, 0 for INDOOR/BUILDING.
 * Source: SKULL.ASM T600 outdoor tick */
int dm2_world_is_outdoor(const dm2_dungeon_world_t *world, int level);

/* Free world model and all owned resources.
 * Safe to call with NULL. */
void dm2_world_free(dm2_dungeon_world_t *world);

/* Source-lock citation string. */
const char *dm2_world_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_WORLD_MODEL_H */
