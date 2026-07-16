#include "dm2_v1_interface_action_table.h"

#include <string.h>

int dm2_v1_interface_action_table_parse(const uint8_t *raw,
                                        size_t raw_size,
                                        DM2_V1_InterfaceActionTable *out)
{
    size_t group_bytes = 0u;
    size_t cursor;
    uint8_t group_count;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!raw || raw_size < 1u) return 0;

    group_count = raw[0];
    if (group_count == 0u || raw_size < 1u + (size_t)group_count) return 0;
    for (uint8_t i = 0u; i < group_count; ++i) {
        group_bytes += raw[1u + i];
    }
    cursor = 1u + (size_t)group_count;
    if (group_bytes > raw_size - cursor ||
        group_bytes > raw_size - cursor - group_bytes) {
        return 0;
    }

    out->raw = raw;
    out->raw_size = raw_size;
    out->group_count = group_count;
    out->group_bytes = group_bytes;
    for (uint8_t i = 0u; i < group_count; ++i) {
        out->group_size[i] = raw[1u + i];
        out->group_a[i] = raw + cursor;
        cursor += out->group_size[i];
    }
    for (uint8_t i = 0u; i < group_count; ++i) {
        out->group_b[i] = raw + cursor;
        cursor += out->group_size[i];
    }
    out->command_tail = raw + cursor;
    out->command_tail_size = raw_size - cursor;
    return 1;
}

int dm2_v1_interface_action_table_load(const DM2_V1_AssetLoader *loader,
                                       DM2_V1_InterfaceActionTable *out)
{
    const uint8_t *raw;
    size_t raw_size = 0u;

    raw = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW7, DM2_GDAT_INTERFACE_RAW_ACTION_TABLE,
        &raw_size);
    return dm2_v1_interface_action_table_parse(raw, raw_size, out);
}
