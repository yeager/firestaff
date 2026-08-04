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

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DATA_TABLES_PC34_COMPAT_H */
