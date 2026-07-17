#include "dm2_v1_gdat_wall_b073_interpreter.h"

#include <string.h>

static uint32_t hash_bytes(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    while (count-- != 0u) { hash ^= *bytes++; hash *= 16777619u; }
    return hash;
}

static int raw7_layout(const uint8_t *raw7, size_t raw7_size,
                       size_t *out_groups_offset, size_t *out_values_offset,
                       size_t *out_lookup_offset, uint8_t *out_group_count)
{
    size_t data_bytes = 0u;
    size_t i;
    uint8_t group_count;

    if (!raw7 || raw7_size < 2u || !out_groups_offset || !out_values_offset ||
        !out_lookup_offset || !out_group_count || raw7[0] == 0u) return 0;
    group_count = raw7[0];
    if ((size_t)group_count > raw7_size - 1u) return 0;
    for (i = 0u; i < group_count; ++i) {
        size_t length = raw7[1u + i];
        /* c_gdatfile.cpp:1930-2003 allocates one nine-byte descriptor per
         * group, with four bytes available at +1 and +5. */
        if (length == 0u || length > 4u || data_bytes > SIZE_MAX - length)
            return 0;
        data_bytes += length;
    }
    *out_groups_offset = 1u + group_count;
    if (data_bytes > (raw7_size - *out_groups_offset) / 2u) return 0;
    *out_values_offset = *out_groups_offset + data_bytes;
    *out_lookup_offset = *out_values_offset + data_bytes;
    if (raw7_size - *out_lookup_offset < 512u) return 0;
    *out_group_count = group_count;
    return 1;
}

static int transform_palette_byte(const uint8_t *raw7, size_t raw7_size,
                                  size_t groups_offset, size_t values_offset,
                                  size_t lookup_offset, uint8_t group_count,
                                  uint8_t source_value, uint8_t light,
                                  uint16_t alpha, uint8_t *out_value)
{
    size_t lookup = lookup_offset + (size_t)source_value * 2u;
    size_t group_offset;
    uint8_t group, subindex, length, selected, value;
    uint16_t scaled;
    uint8_t i;

    if (!out_value || lookup > raw7_size || raw7_size - lookup < 2u ||
        light > 64u) return 0;
    group = raw7[lookup];
    subindex = raw7[lookup + 1u];
    if (group >= group_count) return 0;
    group_offset = groups_offset;
    for (i = 0u; i < group; ++i) group_offset += raw7[1u + i];
    length = raw7[1u + group];
    if (length == 0u || length > 4u || subindex >= length ||
        group_offset + length > values_offset || values_offset +
        (group_offset - groups_offset) + length > lookup_offset) return 0;

    /* c_querydb.cpp:2581-2619: lookup selects the per-group scale, then the
     * light-adjusted value selects an interval in ddat.v1e020c[+1]. */
    scaled = (uint16_t)(((uint16_t)(64u - light) *
                         raw7[group_offset + subindex]) >> 6);
    selected = 0u;
    while ((uint8_t)(selected + 1u) < length) {
        uint8_t low = raw7[group_offset + selected];
        uint8_t high = raw7[group_offset + selected + 1u];
        if (scaled >= low && scaled <= high) {
            if ((uint16_t)(scaled - low) > (uint16_t)(high - scaled))
                ++selected;
            break;
        }
        ++selected;
    }
    value = raw7[values_offset + (group_offset - groups_offset) + selected];
    /* c_querydb.cpp:2622-2662 skips alpha output by walking toward the
     * closest adjacent interval.  The wall caller supplies only one alpha
     * sentinel; -1 remains a normal non-matching value. */
    if (value == (uint8_t)alpha) {
        int left = (int)selected - 1;
        int right = (int)selected + 1;
        for (;;) {
            if (left < 0) selected = (uint8_t)right++;
            else if (right >= length) selected = (uint8_t)left--;
            else if ((int)scaled - raw7[group_offset + left] >=
                     (int)raw7[group_offset + right] - (int)scaled)
                selected = (uint8_t)left--;
            else selected = (uint8_t)right++;
            if (selected >= length) return 0;
            value = raw7[values_offset + (group_offset - groups_offset) + selected];
            if (value != (uint8_t)alpha) break;
        }
    }
    *out_value = value;
    return 1;
}

int dm2_v1_gdat_wall_b073_interpreter_build(
    const DM2_V1_GdatB073InputReceipt *input,
    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
    uint8_t command_index,
    const DM2_V1_GdatWallB073Raw7LoaderReceipt *raw7,
    uint8_t *cache_palette_bytes,
    uint16_t cache_palette_bytes_count,
    uint16_t cache_allocation,
    uint32_t cache_identity,
    DM2_V1_GdatWallB073InterpreterReceipt *out_receipt,
    DM2_V1_GdatWallB073OutputReceipt *out_output)
{
    const DM2_V1_GdatWallM11Command *command;
    uint8_t source_palette[256];
    size_t groups_offset, values_offset, lookup_offset;
    uint8_t group_count;
    uint32_t input_hash, output_hash, hash;
    uint16_t i;

    if (!out_receipt || !out_output) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(out_output, 0, sizeof(*out_output));
    if (!input || !wall_plan || !raw7 || !input->valid || !input->no_draw ||
        !input->identity_hash || !wall_plan->valid || !wall_plan->command_hash ||
        command_index >= wall_plan->command_count || !raw7->valid ||
        !raw7->no_draw || !raw7->identity_hash || !raw7->raw7 ||
        !raw7->raw7_size || raw7->wall_hash != wall_plan->command_hash ||
        raw7->cache_allocation != cache_allocation ||
        raw7->cache_identity != cache_identity || !cache_palette_bytes ||
        cache_palette_bytes_count != 256u || !cache_allocation ||
        !cache_identity || !input->input.cache_owned ||
        input->input.cache_allocation != cache_allocation ||
        input->input.raw7_identity != raw7->raw7_hash) return 0;
    command = &wall_plan->commands[command_index];
    if (!command->pixels || !command->palette_hash ||
        command->palette_hash != input->input.palette_identity ||
        !command->raw_hash || !command->decoded_hash ||
        !command->material_receipt_hash ||
        hash_bytes(command->palette16, 16u) != command->palette_hash ||
        memcmp(cache_palette_bytes, command->palette16, 16u) != 0 ||
        !raw7_layout(raw7->raw7, raw7->raw7_size, &groups_offset,
                     &values_offset, &lookup_offset, &group_count)) return 0;

    input_hash = hash_bytes(cache_palette_bytes, 256u);
    memcpy(source_palette, cache_palette_bytes, sizeof(source_palette));
    for (i = 0u; i < 256u; ++i) {
        if (!transform_palette_byte(raw7->raw7, raw7->raw7_size, groups_offset,
                                    values_offset, lookup_offset, group_count,
                                    source_palette[i], input->input.light,
                                    input->input.alpha_mask,
                                    &cache_palette_bytes[i])) {
            memcpy(cache_palette_bytes, source_palette, sizeof(source_palette));
            return 0;
        }
    }
    output_hash = hash_bytes(cache_palette_bytes, 256u);
    if (!output_hash || !dm2_v1_gdat_wall_b073_output_receipt_build(
            input, wall_plan, command_index, cache_palette_bytes, 256u,
            cache_allocation, cache_identity, out_output)) {
        memcpy(cache_palette_bytes, source_palette, sizeof(source_palette));
        memset(out_output, 0, sizeof(*out_output));
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->no_draw = 1;
    out_receipt->command_index = command_index;
    out_receipt->cache_palette_bytes = cache_palette_bytes;
    out_receipt->cache_palette_bytes_count = 256u;
    out_receipt->cache_allocation = cache_allocation;
    out_receipt->input_cache_hash = input_hash;
    out_receipt->output_cache_hash = output_hash;
    out_receipt->raw7_hash = raw7->raw7_hash;
    out_receipt->cache_identity = cache_identity;
    out_receipt->wall_hash = wall_plan->command_hash;
    hash = 2166136261u;
    hash ^= input->identity_hash; hash *= 16777619u;
    hash ^= raw7->identity_hash; hash *= 16777619u;
    hash ^= input_hash; hash *= 16777619u;
    hash ^= output_hash; hash *= 16777619u;
    hash ^= out_output->identity_hash; hash *= 16777619u;
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}
