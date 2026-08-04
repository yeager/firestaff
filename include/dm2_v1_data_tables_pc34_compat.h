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

typedef struct { uint8_t a; uint8_t b; } dm2_s_bb;
typedef struct { uint8_t a; uint8_t b; uint16_t w; } dm2_s_bbw;
typedef struct { uint16_t a; uint16_t b; uint16_t c; uint8_t d; } dm2_s_wwwb;
typedef struct { uint16_t a; uint16_t b; uint16_t c; } dm2_s_www;
typedef struct { uint8_t v[4]; } dm2_s_4bytearray;
typedef struct { int16_t w_00; int16_t w_02; } dm2_s_ww2;
typedef struct { int16_t w_00; int8_t b_02; int8_t b_03; int8_t b_04; int8_t b_05; int16_t w_06; } dm2_s_wbbbbw;

/* Direction X deltas: N=0, E=+1, S=0, W=-1 */
extern const int16_t dm2_v1_dir_dx[4];

/* Direction Y deltas: N=-1, E=0, S=+1, W=0 */
extern const int16_t dm2_v1_dir_dy[4];

extern const int8_t dm2_v1_table_1d645d[6];
extern const int8_t dm2_v1_table_1d70f0[24];
extern const uint16_t dm2_v1_table_1d14e2[24];
extern const uint8_t dm2_v1_music_map[64];
extern const uint32_t dm2_v1_table_1d7092[8];
extern const uint32_t dm2_v1_table_1d7072[8];
extern const uint32_t dm2_v1_table_1d7052[8];
extern const uint32_t dm2_v1_table_1d7042[4];
extern const uint8_t dm2_v1_table_1d7029[20];
extern const uint8_t dm2_v1_table_1d7012[23];
extern const uint8_t dm2_v1_table_1d6f4c[16];
extern const int8_t dm2_v1_table_1d26a8[32];
extern const int8_t dm2_v1_table_1d6290[9];
extern const int16_t dm2_v1_table_1d6299[5];
extern const uint8_t dm2_v1_table_1d6f27[5];
extern const int8_t dm2_v1_table_1d62ee[30];
extern const int8_t dm2_v1_table_1d62e8[4];
extern const int16_t dm2_v1_table_1d62e0[4];
extern const int16_t dm2_v1_table_1d62b0[8][2];
extern const int16_t dm2_v1_table_1d62d0[4][2];
extern const int8_t dm2_v1_table_1d3ffc[4];
extern const int8_t dm2_v1_table_1d3ff8[4];
extern const int16_t dm2_v1_table_1d27c4[8];
extern const int16_t dm2_v1_table_1d27d4[10];
extern const int8_t dm2_v1_table_1d268e[6];
extern const int8_t dm2_v1_table_1d2660[16];
extern const int16_t dm2_v1_table_1d2670[13];
extern const int8_t dm2_v1_table_1d26c8[8];
extern const int8_t dm2_v1_table_1d26f8[4];
extern const int16_t dm2_v1_table_1d2752[4];
extern const int8_t dm2_v1_table_1d324c[44];
extern const uint16_t dm2_v1_table_1d3278[16];
extern const uint8_t dm2_v1_table_1d3298[16];
extern const uint16_t dm2_v1_table_1d6fee[18];
extern const uint8_t dm2_v1_table_1d6fdc[18];
extern const uint16_t dm2_v1_table_1d6f9c[32];
extern const uint8_t dm2_v1_table_1d6f7c[32];
extern const uint8_t dm2_v1_table_1d6f5c[32];
extern const uint16_t dm2_v1_table_1d6f2c[16];
extern const int16_t dm2_v1_table_1d6f0b[14];

/* Batch 3: remaining const tables from dm2data.cpp */
extern const int8_t dm2_v1_table_1d6702[16];
extern const int8_t dm2_v1_table_1d6712[21];
extern const int16_t dm2_v1_table_1d672b[9];
extern const int16_t dm2_v1_table_1d673d[7];
extern const uint8_t dm2_v1_table_1d281c[16];
extern const int8_t dm2_v1_table_1d282c[16];
extern const uint8_t dm2_v1_table_1d631a[60];
extern const uint8_t dm2_v1_table_1d6356[263];
extern const uint8_t dm2_v1_vsgame[120];
extern const dm2_s_4bytearray dm2_v1_table_1d26d0[8];
extern const dm2_s_4bytearray dm2_v1_table_1d26f0[2];
extern const uint8_t dm2_v1_table_1d275a[32][2];
extern const dm2_s_bbw dm2_v1_table_1d3ed5[10];
extern const dm2_s_wwwb dm2_v1_table_1d3d23[62];
extern const uint8_t dm2_v1_table_1d3cd0[83];
extern const dm2_s_bbw dm2_v1_table_1d3ba0[76];
extern const uint8_t dm2_v1_table_1d6afe[23];
extern const dm2_s_4bytearray dm2_v1_table_1d6a74[23];
extern const uint8_t dm2_v1_table_1d6ad0[23][2];
extern const uint8_t dm2_v1_table_1d6b43[23];
extern const uint8_t dm2_v1_table_1d6b5a[23];
extern const int8_t dm2_v1_table_1d6b2c[23];
extern const int8_t dm2_v1_table_1d6b15[23];
extern const int8_t dm2_v1_table_1d6a54[4][4];
extern const int8_t dm2_v1_table_1d6a64[4][4];
extern const int8_t dm2_v1_table_1d6b71[5];
extern const uint8_t dm2_v1_table_1d6efd[14];
extern const uint8_t dm2_v1_table_1d6ee1[14][2];
extern const uint8_t dm2_v1_table_1d6ed3[14];
extern const dm2_s_bb dm2_v1_table_1d6eb3[16];
extern const int8_t dm2_v1_table_1d6ea8[3];
extern const int8_t dm2_v1_table_1d6eab[4];
extern const int8_t dm2_v1_table_1d6eaf[4];
extern const dm2_s_bb dm2_v1_table_1d6e68[4][8];
extern const uint8_t dm2_v1_table_1d6e51[23];
extern const uint8_t dm2_v1_table_1d6797[37];
extern const int8_t dm2_v1_table_1d6e41[16];
extern const int8_t dm2_v1_table_1d6e35[12];
extern const uint16_t dm2_v1_table_1d6c70[16];
extern const int8_t dm2_v1_table_1d6c90[16];
extern const uint8_t dm2_v1_table_1d6ca0[16];
extern const uint8_t dm2_v1_table_1d6cb0[16];
extern const int16_t dm2_v1_table_1d6cc0[16];
extern const int8_t dm2_v1_table_1d6c10[5];
extern const int8_t dm2_v1_table_1d6c19[5];
extern const int8_t dm2_v1_table_1d6c1e[23];
extern const int8_t dm2_v1_table_1d6c35[23];
extern const int16_t dm2_v1_table_1d6c4c[9];
extern const uint8_t dm2_v1_table_1d6c5e[9];
extern const int8_t dm2_v1_table_1d6c67[9];
extern const uint8_t dm2_v1_table_1d6c06[5];
extern const int8_t dm2_v1_table_1d6c0b[5];
extern const dm2_s_www dm2_v1_table_1d6a31[5];
extern const int8_t dm2_v1_table_1d6e03[26][2];
extern const int8_t dm2_v1_table_1d6de3[16][2];
extern const int8_t dm2_v1_table_1d6dd3[16];
extern const int8_t dm2_v1_table_1d69aa[6];
extern const int8_t dm2_v1_table_1d69a2[6];
extern const uint16_t dm2_v1_table_1d6998[5];
extern const uint8_t dm2_v1_table_1d6984[20];
extern const int8_t dm2_v1_table_1d6980[4];
extern const int8_t dm2_v1_table_1d69b0[32];
extern const int8_t dm2_v1_table_1d69d0[4];
extern const uint8_t dm2_v1_table_1d6b76[132];
extern const int16_t dm2_v1_table_1d6d3c[6];
extern const int8_t dm2_v1_table_1d6d48[6];
extern const int8_t dm2_v1_table_1d6d4e[6];
extern const int8_t dm2_v1_table_1d6d54[6];
extern const uint8_t dm2_v1_table_1d6ce0[92];
extern const uint8_t dm2_v1_table_1d6d5a[4][5];
extern const dm2_s_bb dm2_v1_table_1d6d6e[4];
extern const uint8_t dm2_v1_table_1d6d76[2][4];
extern const uint8_t dm2_v1_table_1d6d7e[2][4];
extern const int8_t dm2_v1_table_1d6d86[2];
extern const int8_t dm2_v1_table_1d6d88[25];
extern const int8_t dm2_v1_table_1d6da1[25];
extern const int8_t dm2_v1_table_1d6dba[25];
extern const int8_t dm2_v1_table_1d67d9[7];
extern const int8_t dm2_v1_table_1d67e0[5][6];
extern const int8_t dm2_v1_table_1d67fe[4];
extern const int16_t dm2_v1_table_1d292c[32];
extern const uint8_t dm2_v1_table_1d70b4[17];

/* Batch 4: runtime-loaded tables (from bin/v5 dat files) */
extern const int8_t dm2_v1_table_1d7108[128];
extern const int8_t dm2_v1_table_1d6802[272];
extern const dm2_s_ww2 dm2_v1_table_1d39bc[121];
extern const dm2_s_www dm2_v1_table_1d338c[264];
extern const int8_t dm2_v1_table_1d296c[63][36];
extern const dm2_s_wbbbbw dm2_v1_table_1d653c[55];

/* GDAT command-string type prefixes (table1d6912) */
extern const char *const dm2_v1_table_1d6912[18];

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DATA_TABLES_PC34_COMPAT_H */
