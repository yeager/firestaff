#include "csb_v1_magic_rune_cost_pc34_compat.h"

#include <string.h>

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t value = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        value ^= bytes[i];
        value *= 16777619u;
    }
    return value;
}

int csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_MagicRuneCostTablePc34 *out_table)
{
    CSB_V1_MagicRuneCostTablePc34 table;
    size_t i;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!decoded_graphic || !out_table ||
        decoded_size != CSB_V1_MAGIC_RUNE_TABLE_DECODED_SIZE_PC34) return 0;

    memset(&table, 0, sizeof(table));
    memcpy(table.power_multiplier,
           decoded_graphic + CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34,
           sizeof(table.power_multiplier));
    memcpy(table.base_cost,
           decoded_graphic + CSB_V1_MAGIC_RUNE_BASE_COST_OFFSET_PC34,
           sizeof(table.base_cost));
    for (i = 0u; i < sizeof(table.power_multiplier); ++i) {
        if (table.power_multiplier[i] == 0u) return 0;
    }
    table.decoded_payload_fnv1a = fnv1a32(decoded_graphic, decoded_size);
    if (table.decoded_payload_fnv1a == 0u) return 0;
    table.valid = 1;
    *out_table = table;
    return 1;
}

int csb_v1_magic_rune_cost_table_from_cache_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    CSB_V1_MagicRuneCostTablePc34 *out_table)
{
    uint8_t payload[CSB_V1_MAGIC_RUNE_TABLE_DECODED_SIZE_PC34];
    CSB_V1_CSBGraphicsEntrySpan span;
    size_t written = 0u;
    int rc;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!cache || !cache->loaded || !cache->file_buffer || !out_table) return 0;
    rc = csb_v1_csbgraphics_dat_entry_span(
        cache->file_buffer, cache->file_size,
        CSB_V1_MAGIC_RUNE_TABLE_GRAPHICS_ENTRY_PC34, &span);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        span.decompressed_size != sizeof(payload)) return 0;
    rc = csb_v1_csbgraphics_dat_decode_entry(
        cache->file_buffer, cache->file_size,
        CSB_V1_MAGIC_RUNE_TABLE_GRAPHICS_ENTRY_PC34,
        payload, sizeof(payload), &written);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK || written != sizeof(payload)) {
        return 0;
    }
    return csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
        payload, written, out_table);
}

int csb_v1_magic_rune_cost_compute_pc34(
    const CSB_V1_MagicRuneCostTablePc34 *table,
    int symbol_step, int symbol_index, int power_rune, int *out_cost)
{
    int cost;
    int power_index;

    if (out_cost) *out_cost = 0;
    if (!table || !table->valid || !out_cost || symbol_step < 0 ||
        symbol_step >= CSB_V1_MAGIC_RUNE_ROW_COUNT_PC34 || symbol_index < 0 ||
        symbol_index >= CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34) return 0;
    cost = table->base_cost[symbol_step][symbol_index];
    if (symbol_step != 0) {
        power_index = power_rune - 96;
        if (power_index < 0 ||
            power_index >= CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34) return 0;
        cost = (cost * table->power_multiplier[power_index]) >> 3;
    }
    *out_cost = cost;
    return 1;
}
