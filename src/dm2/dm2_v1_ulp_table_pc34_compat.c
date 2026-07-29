#include "dm2_v1_ulp_table_pc34_compat.h"

/*
 * DM2 ULP table operations.
 * Source: skproject c_dballoc.cpp:120-159
 */

int dm2_v1_ulp_getp(const DM2_V1_UlpTable *table, uint32_t index,
                     DM2_V1_UlpGetpReceipt *out)
{
    if (!table || !out) {
        if (out) out->valid = 0;
        return 0;
    }
    if (!table->entries || index >= table->count) {
        out->valid = 0;
        out->blocked_out_of_range = 1;
        return 0;
    }
    out->valid = 1;
    out->blocked_out_of_range = 0;
    out->ptr = table->entries[index].ptr;
    return 1;
}

int dm2_v1_ulp_setp(DM2_V1_UlpTable *table, uint32_t index,
                     uint8_t *ptr)
{
    if (!table || !table->entries || index >= table->count)
        return 0;
    table->entries[index].ptr = ptr;
    return 1;
}

/* setl: store length with MSB set as marker */
int dm2_v1_ulp_setl(DM2_V1_UlpTable *table, uint32_t index,
                     int32_t length)
{
    if (!table || !table->entries || index >= table->count)
        return 0;
    table->entries[index].length = length | (int32_t)0x80000000;
    return 1;
}

int dm2_v1_ulp_islen(const DM2_V1_UlpTable *table, uint32_t index,
                      DM2_V1_UlpIslenReceipt *out)
{
    if (!table || !out) {
        if (out) out->valid = 0;
        return 0;
    }
    if (!table->entries || index >= table->count) {
        out->valid = 0;
        out->blocked_out_of_range = 1;
        return 0;
    }
    out->valid = 1;
    out->blocked_out_of_range = 0;
    out->is_length = (table->entries[index].length & (int32_t)0x80000000) != 0;
    return 1;
}

/* skproject c_dballoc.cpp:138-148  DM2_QUERY_GDAT_RAW_DATA_LENGTH */
int dm2_v1_ulp_query_raw_data_length(const DM2_V1_UlpTable *table,
                                      uint32_t index,
                                      DM2_V1_UlpQueryRawDataLengthReceipt *out)
{
    if (!table || !out) {
        if (out) out->valid = 0;
        return 0;
    }
    if (!table->entries || index >= table->count) {
        out->valid = 0;
        out->blocked_out_of_range = 1;
        return 0;
    }

    int32_t raw = table->entries[index].length;
    out->valid = 1;
    out->blocked_out_of_range = 0;
    out->raw_length = raw;

    if (raw & (int32_t)0x80000000) {
        out->effective_length = raw;
    } else {
        /* Pointer entry: length is stored as int16 at ptr[-2] (word before data).
         * We return the raw value; actual pointer dereference requires the caller
         * to have the data buffer available. */
        out->effective_length = 0;
    }
    return 1;
}
