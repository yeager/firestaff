/* DM1 V1 Dungeon DAT Decompressor — source-locked from ReDMCSB
 * DECOMPDU.C: F0455_FLOPPY_DecompressDungeon, G0525-G0534
 * DUNGEON.C: dungeon tile/thing data structures
 * LZW.C: used for compressed dungeon data */
#ifndef FIRESTAFF_DM1_V1_DUNGEON_DECOMPRESSOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_DECOMPRESSOR_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_DD_MAX_LEVELS      16
#define DM1_DD_MAX_MAP_DIM     32   /* Max tiles per dimension */
#define DM1_DD_TILE_WALL        0
#define DM1_DD_TILE_OPEN        1
#define DM1_DD_TILE_PIT         2
#define DM1_DD_TILE_STAIRS      3
#define DM1_DD_TILE_DOOR        4
#define DM1_DD_TILE_TELEPORTER  5
#define DM1_DD_TILE_TRICK_WALL  6

/* G0525-G0528: dungeon file header fields from DECOMPDU.C */
typedef struct {
    uint32_t game_id;           /* G0525_l_GameID */
    uint16_t dungeon_id;        /* G0526_ui_DungeonID */
    int16_t  platform;          /* G0527_i_Platform */
    int16_t  format;            /* G0528_i_Format */
    uint16_t level_count;
    uint32_t total_size;
} M11_DD_FileHeader;

/* Level header parsed from dungeon data */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t difficulty;        /* Level difficulty rating */
    uint16_t creature_count;
    uint16_t room_count;        /* Number of rooms/areas */
    uint32_t data_offset;       /* Offset to tile data in decompressed buffer */
    uint32_t data_size;         /* Size of level data */
} M11_DD_LevelHeader;

/* Tile at position */
typedef struct {
    uint8_t  type;              /* DM1_DD_TILE_* */
    uint8_t  attributes;        /* Open/closed, direction, etc. */
    uint16_t thing_index;       /* Index to first thing on tile */
    uint8_t  wall_ornament;     /* Wall ornament index (0=none) */
    uint8_t  floor_ornament;    /* Floor ornament index (0=none) */
} M11_DD_Tile;

/* Creature entry from dungeon data */
typedef struct {
    uint16_t type;
    int16_t  x, y;
    uint8_t  facing;
    uint16_t hit_points;
} M11_DD_Creature;

/* Decompressor state — G0530-G0532 pattern */
typedef struct {
    M11_DD_FileHeader header;
    M11_DD_LevelHeader levels[DM1_DD_MAX_LEVELS];
    M11_DD_Tile tile_map[DM1_DD_MAX_LEVELS][DM1_DD_MAX_MAP_DIM][DM1_DD_MAX_MAP_DIM];
    M11_DD_Creature creatures[DM1_DD_MAX_LEVELS * 64]; /* Up to 64 per level */
    uint16_t creature_offsets[DM1_DD_MAX_LEVELS]; /* Start index per level */
    uint16_t creature_counts[DM1_DD_MAX_LEVELS];
    bool     level_loaded[DM1_DD_MAX_LEVELS];
    bool     compressed;        /* G0530_B_LoadingCompressedDungeon */
    uint8_t* decomp_buffer;     /* G0531 pattern */
    size_t   decomp_remaining;  /* G0532 pattern */
    bool     loaded;
} M11_DD_State;

void          m11_dd_init(M11_DD_State* state);
bool          m11_dd_load_file(M11_DD_State* state, const uint8_t* data, size_t size);
uint16_t      m11_dd_get_level_count(const M11_DD_State* state);
bool          m11_dd_decompress_level(M11_DD_State* state, int level,
                                       uint8_t* output, size_t out_size);
const M11_DD_LevelHeader* m11_dd_get_level_header(const M11_DD_State* state, int level);
const M11_DD_Tile*        m11_dd_get_tile(const M11_DD_State* state, int level,
                                           int16_t x, int16_t y);
const M11_DD_Creature*    m11_dd_get_creature(const M11_DD_State* state, int level,
                                               int16_t x, int16_t y);
void          m11_dd_close(M11_DD_State* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DUNGEON_DECOMPRESSOR_PC34_COMPAT_H */
