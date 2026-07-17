#include "nexus_v1_sndlev_map_provenance.h"

#include <stdio.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int main(void)
{
    uint8_t bytes[34] = {0};
    Nexus_V1_SndlevMapProvenanceReceipt table;
    Nexus_V1_SndlevMapRowProvenanceReceipt row;
    uint64_t source_fnv;

    bytes[24] = 1U;
    bytes[32] = 0xffU;
    bytes[33] = 0xffU;
    source_fnv = fnv1a64(bytes, sizeof(bytes));
    if (!nexus_v1_sndlev_map_provenance_parse(
            bytes, sizeof(bytes), source_fnv, &table) || !table.valid ||
        table.record_count != 1U || table.terminator_offset != 32U ||
        table.playback_permitted ||
        !nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), source_fnv, 0U, &row) || !row.valid ||
        row.row_index != 0U || row.row_offset != 24U || row.row_length != 8U ||
        row.table_fnv1a64 != table.table_fnv1a64 || row.playback_permitted)
        return 1;

    bytes[24] ^= 1U;
    if (nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), source_fnv, 0U, &row)) return 1;
    bytes[24] ^= 1U;
    if (nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), fnv1a64(bytes, sizeof(bytes)), 1U, &row) ||
        nexus_v1_sndlev_map_row_provenance_parse(
            bytes, sizeof(bytes), source_fnv ^ UINT64_C(1), 0U, &row)) return 1;

    bytes[32] = 0U;
    if (nexus_v1_sndlev_map_provenance_parse(
            bytes, sizeof(bytes), fnv1a64(bytes, sizeof(bytes)), &table)) return 1;
    puts("test_nexus_v1_sndlev_map_provenance: PASS");
    return 0;
}
