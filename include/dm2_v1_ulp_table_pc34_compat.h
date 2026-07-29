#ifndef FIRESTAFF_DM2_V1_ULP_TABLE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_ULP_TABLE_PC34_COMPAT_H

/*
 * DM2 ULP (Unified Lookup Pointer) table.
 * Source: skproject c_dballoc.cpp:118-159 (c_ulp class).
 *
 * Each entry is a union: either a pointer to data (xp_00) or a length
 * value with MSB set (l_00 | 0x80000000). The table is indexed by
 * GDAT entry index (t_dbidx).
 *
 * getp(n)     → returns pointer at slot n
 * setp(n, p)  → sets pointer at slot n
 * setl(n, l)  → sets length at slot n (with MSB marker)
 * islen(n)    → returns true if slot n contains a length (MSB set)
 */

#include <stdint.h>
#include <stddef.h>

typedef union {
    uint8_t *ptr;
    int32_t length;
} DM2_V1_UlpEntry;

typedef struct {
    DM2_V1_UlpEntry *entries;
    uint32_t count;
} DM2_V1_UlpTable;

typedef struct {
    int valid;
    int blocked_out_of_range;
    uint8_t *ptr;
} DM2_V1_UlpGetpReceipt;

typedef struct {
    int valid;
    int blocked_out_of_range;
    int is_length;
} DM2_V1_UlpIslenReceipt;

typedef struct {
    int valid;
    int blocked_out_of_range;
    int32_t raw_length;
    int32_t effective_length;
} DM2_V1_UlpQueryRawDataLengthReceipt;

int dm2_v1_ulp_getp(const DM2_V1_UlpTable *table, uint32_t index,
                     DM2_V1_UlpGetpReceipt *out);

int dm2_v1_ulp_setp(DM2_V1_UlpTable *table, uint32_t index,
                     uint8_t *ptr);

int dm2_v1_ulp_setl(DM2_V1_UlpTable *table, uint32_t index,
                     int32_t length);

int dm2_v1_ulp_islen(const DM2_V1_UlpTable *table, uint32_t index,
                      DM2_V1_UlpIslenReceipt *out);

int dm2_v1_ulp_query_raw_data_length(const DM2_V1_UlpTable *table,
                                      uint32_t index,
                                      DM2_V1_UlpQueryRawDataLengthReceipt *out);

/* splitlong: store 32-bit value as 4 little-endian bytes.
 * Source: skproject c_dballoc.cpp:164-170 (s_hex6::splitlong). */
static inline void dm2_v1_splitlong(uint8_t out[4], int32_t value) {
    out[0] = (uint8_t)(value);
    out[1] = (uint8_t)(value >> 8);
    out[2] = (uint8_t)(value >> 16);
    out[3] = (uint8_t)(value >> 24);
}

#endif
