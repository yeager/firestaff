#ifndef FIRESTAFF_DM2_V1_DATA_TABLES_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_DATA_TABLES_PC34_COMPAT_H

/*
 * dm2_v1_data_tables_pc34_compat.h — DM2 baked-in data tables.
 *
 * Source: skproject/SKWINSPX/src/v5/dm2data.cpp
 *
 * These are ROM-like lookup tables embedded in the DM2 executable.
 * They drive direction computation, creature AI, sound indexing,
 * viewport rendering, and GUI layout.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Direction X deltas: N=0, E=+1, S=0, W=-1 */
extern const int16_t dm2_v1_dir_dx[4];

/* Direction Y deltas: N=-1, E=0, S=+1, W=0 */
extern const int16_t dm2_v1_dir_dy[4];

/* Creature positioning offsets (6 entries) */
extern const int8_t dm2_v1_table_1d645d[6];

/* Clock-hour to sound duration (24 entries) */
extern const int8_t dm2_v1_table_1d70f0[24];

/* Sound frequency table (24 entries) */
extern const uint16_t dm2_v1_table_1d14e2[24];

/* Music map: level -> track index (64 entries = SONGLIST.DAT) */
extern const uint8_t dm2_v1_music_map[64];

/* Viewport bitmap color tables (8 entries each) */
extern const uint32_t dm2_v1_table_1d7092[8];
extern const uint32_t dm2_v1_table_1d7072[8];
extern const uint32_t dm2_v1_table_1d7052[8];
extern const uint32_t dm2_v1_table_1d7042[4];

/* Creature AI sub-skill index table (20 entries) */
extern const uint8_t dm2_v1_table_1d7029[20];

/* Creature type class table (23 entries) */
extern const uint8_t dm2_v1_table_1d7012[23];

/* Item type flags (16 entries) */
extern const uint8_t dm2_v1_table_1d6f4c[16];

/* Direction-to-position mapping (32 entries) */
extern const int8_t dm2_v1_table_1d26a8[32];

/* Creature damage class (9 entries) */
extern const int8_t dm2_v1_table_1d6290[9];

/* Creature action sub-type (5 entries) */
extern const int16_t dm2_v1_table_1d6299[5];

/* Door ordinal query (5 entries) */
extern const uint8_t dm2_v1_table_1d6f27[5];

/* Creature AI behavior flags (30 entries) */
extern const int8_t dm2_v1_table_1d62ee[30];

/* Direction reverse map (4 entries) */
extern const int8_t dm2_v1_table_1d62e8[4];

/* Creature group size offsets (4 entries) */
extern const int16_t dm2_v1_table_1d62e0[4];

/* Directional neighbor offsets, 8 dirs x 2 (dx,dy) */
extern const int16_t dm2_v1_table_1d62b0[8][2];

/* Perpendicular direction offsets (4 entries x 2) */
extern const int16_t dm2_v1_table_1d62d0[4][2];

/* Direction X deltas (i8 variant): S=0, E=+1, N=0, W=-1 */
extern const int8_t dm2_v1_table_1d3ffc[4];

/* Direction Y deltas (i8 variant): S=+1, E=0, N=-1, W=0 */
extern const int8_t dm2_v1_table_1d3ff8[4];

/* Viewport item ordinals (8 entries) */
extern const int16_t dm2_v1_table_1d27c4[8];

/* Viewport item ordinals B (10 entries) */
extern const int16_t dm2_v1_table_1d27d4[10];

/* Creature slot indices (6 entries) */
extern const int8_t dm2_v1_table_1d268e[6];

/* Tile visibility bitmask (16 entries) */
extern const int8_t dm2_v1_table_1d2660[16];

/* Tile property flags (13 entries) */
extern const int16_t dm2_v1_table_1d2670[13];

/* Creature AI attack type (8 entries) */
extern const int8_t dm2_v1_table_1d26c8[8];

/* Direction bitmask shift (4 entries) */
extern const int8_t dm2_v1_table_1d26f8[4];

/* Viewport wall ordinals (4 entries) */
extern const int16_t dm2_v1_table_1d2752[4];

/* Creature AI movement flags (44 entries) */
extern const int8_t dm2_v1_table_1d324c[44];

/* Door/wall interaction flags (16 entries) */
extern const uint16_t dm2_v1_table_1d3278[16];

/* GUI element type map (16 entries) */
extern const uint8_t dm2_v1_table_1d3298[16];

/* Door visual ordinals (18 entries) */
extern const uint16_t dm2_v1_table_1d6fee[18];

/* Door visual byte map (18 entries) */
extern const uint8_t dm2_v1_table_1d6fdc[18];

/* Wall ornament ordinals A (32 entries) */
extern const uint16_t dm2_v1_table_1d6f9c[32];

/* Wall ornament byte map A (32 entries) */
extern const uint8_t dm2_v1_table_1d6f7c[32];

/* Wall ornament byte map B (32 entries) */
extern const uint8_t dm2_v1_table_1d6f5c[32];

/* Item ordinal to wall mapping (16 entries) */
extern const uint16_t dm2_v1_table_1d6f2c[16];

/* Item ordinals for things (14 entries) */
extern const int16_t dm2_v1_table_1d6f0b[14];

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DATA_TABLES_PC34_COMPAT_H */
