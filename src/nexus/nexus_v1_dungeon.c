
#include "nexus_v1_dungeon.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}
static uint16_t rb16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }

static const Nexus_V1_DgnStructure2Texture *
nexus_v1_level_get_structure2_texture(const Nexus_V1_Level *level,
                                      uint16_t image_id)
{
    int index;
    if (!level || !level->structure2_texture_table_valid) return NULL;
    for (index = 0; index < level->structure2_texture_count; ++index) {
        if (level->structure2_textures[index].image_id == image_id)
            return &level->structure2_textures[index];
    }
    return NULL;
}

static int nexus_v1_level_find_structure2_texture(
    const Nexus_V1_Level *level, uint16_t image_id)
{
    return nexus_v1_level_get_structure2_texture(level, image_id) != NULL;
}

int nexus_v1_level_structure2_source_envelope_valid(
    const Nexus_V1_Level *level)
{
    if (!level) return 0;
    return level->structure2_payload.valid &&
           level->structure2_payload.descriptor_offset_envelope_valid;
}

static int nexus_v1_level_copy_structure3_payload(
    Nexus_V1_Level *level, const uint8_t *data, int size)
{
    uint16_t block_offset;
    uint16_t block_count;
    int byte_offset;
    int byte_size;
    unsigned char seen[UINT8_MAX + 1U];
    uint32_t hash = 2166136261u;
    int block_index;
    int nonzero_block_run_start = -1;
    int nonzero_block_run_length = 0;
    int nonzero_byte_run_start = -1;
    int nonzero_byte_run_length = 0;
    int byte_index;

    if (!level || !data || size < NEXUS_DGN_BLOCK_SIZE) return -1;
    /* DMWeb DGN container: Structure3's block offset/count follow the
     * Structure2 header pair. No field inside the resulting span is decoded. */
    block_offset = rb16(data + 0x1c);
    block_count = rb16(data + 0x1e);
    if (block_offset == 0U && block_count == 0U) return 0;
    if (block_offset == 0U || block_count == 0U ||
        block_offset > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        block_count > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) return -1;
    byte_offset = (int)block_offset * NEXUS_DGN_BLOCK_SIZE;
    byte_size = (int)block_count * NEXUS_DGN_BLOCK_SIZE;
    if (byte_offset > size || byte_size > size - byte_offset) return -1;
    level->structure3_payload.declared = 1;
    level->structure3_payload.block_offset = (int)block_offset;
    level->structure3_payload.block_count = (int)block_count;
    level->structure3_payload.byte_offset = byte_offset;
    level->structure3_payload.byte_size = byte_size;
    level->structure3_payload.first_nonzero_byte_offset = -1;
    level->structure3_payload.last_nonzero_byte_offset = -1;
    level->structure3_payload.first_nonzero_byte_run_offset = -1;
    level->structure3_payload.last_nonzero_byte_run_offset = -1;
    level->structure3_payload.first_nonzero_block_index = -1;
    level->structure3_payload.last_nonzero_block_index = -1;
    level->structure3_payload.first_nonzero_block_run_start_block_index = -1;
    level->structure3_payload.last_nonzero_block_run_start_block_index = -1;
    level->structure3_payload.complete_block_count = (int)block_count;
    memset(seen, 0, sizeof(seen));
    for (block_index = 0; block_index < (int)block_count; ++block_index) {
        int block_nonzero = 0;
        int block_byte;
        for (block_byte = 0; block_byte < NEXUS_DGN_BLOCK_SIZE; ++block_byte) {
            uint8_t value;
            byte_index = block_index * NEXUS_DGN_BLOCK_SIZE + block_byte;
            value = data[byte_offset + byte_index];
            hash ^= value;
            hash *= 16777619u;
            if (value == 0U) {
                ++level->structure3_payload.zero_byte_count;
                if (nonzero_byte_run_length > 0) {
                    if (level->structure3_payload
                            .first_nonzero_byte_run_byte_count == 0) {
                        level->structure3_payload
                            .first_nonzero_byte_run_byte_count =
                            nonzero_byte_run_length;
                    }
                    level->structure3_payload.last_nonzero_byte_run_offset =
                        nonzero_byte_run_start;
                    level->structure3_payload.last_nonzero_byte_run_byte_count =
                        nonzero_byte_run_length;
                }
                nonzero_byte_run_start = -1;
                nonzero_byte_run_length = 0;
            } else {
                ++block_nonzero;
                ++level->structure3_payload.nonzero_byte_count;
                if (level->structure3_payload.first_nonzero_byte_offset < 0) {
                    level->structure3_payload.first_nonzero_byte_offset = byte_index;
                }
                level->structure3_payload.last_nonzero_byte_offset = byte_index;
                if (nonzero_byte_run_length == 0) {
                    ++level->structure3_payload.nonzero_byte_run_count;
                    nonzero_byte_run_start = byte_index;
                    if (level->structure3_payload
                            .first_nonzero_byte_run_offset < 0) {
                        level->structure3_payload.first_nonzero_byte_run_offset =
                            byte_index;
                    }
                }
                ++nonzero_byte_run_length;
                if (nonzero_byte_run_length >
                    level->structure3_payload.longest_nonzero_byte_run) {
                    level->structure3_payload.longest_nonzero_byte_run =
                        nonzero_byte_run_length;
                }
            }
            if (!seen[value]) {
                seen[value] = 1U;
                ++level->structure3_payload.distinct_byte_value_count;
            }
            if (byte_index > 0 && value != data[byte_offset + byte_index - 1]) {
                ++level->structure3_payload.byte_transition_count;
            }
        }
        if (block_nonzero == 0) {
            ++level->structure3_payload.zero_block_count;
            if (nonzero_block_run_length > 0) {
                if (level->structure3_payload.first_nonzero_block_run_block_count ==
                    0) {
                    level->structure3_payload
                        .first_nonzero_block_run_block_count =
                        nonzero_block_run_length;
                }
                level->structure3_payload.last_nonzero_block_run_start_block_index =
                    nonzero_block_run_start;
                level->structure3_payload.last_nonzero_block_run_block_count =
                    nonzero_block_run_length;
            }
            nonzero_block_run_start = -1;
            nonzero_block_run_length = 0;
        } else {
            ++level->structure3_payload.nonzero_block_count;
            if (level->structure3_payload.first_nonzero_block_index < 0) {
                level->structure3_payload.first_nonzero_block_index = block_index;
            }
            level->structure3_payload.last_nonzero_block_index = block_index;
            if (nonzero_block_run_length == 0) {
                ++level->structure3_payload.nonzero_block_run_count;
                nonzero_block_run_start = block_index;
                if (level->structure3_payload
                        .first_nonzero_block_run_start_block_index < 0) {
                    level->structure3_payload
                        .first_nonzero_block_run_start_block_index = block_index;
                }
            }
            ++nonzero_block_run_length;
            if (nonzero_block_run_length >
                level->structure3_payload.longest_nonzero_block_run) {
                level->structure3_payload.longest_nonzero_block_run =
                    nonzero_block_run_length;
            }
        }
    }
    if (nonzero_byte_run_length > 0) {
        if (level->structure3_payload.first_nonzero_byte_run_byte_count == 0) {
            level->structure3_payload.first_nonzero_byte_run_byte_count =
                nonzero_byte_run_length;
        }
        level->structure3_payload.last_nonzero_byte_run_offset =
            nonzero_byte_run_start;
        level->structure3_payload.last_nonzero_byte_run_byte_count =
            nonzero_byte_run_length;
    }
    if (nonzero_block_run_length > 0) {
        if (level->structure3_payload.first_nonzero_block_run_block_count == 0) {
            level->structure3_payload.first_nonzero_block_run_block_count =
                nonzero_block_run_length;
        }
        level->structure3_payload.last_nonzero_block_run_start_block_index =
            nonzero_block_run_start;
        level->structure3_payload.last_nonzero_block_run_block_count =
            nonzero_block_run_length;
    }
    level->structure3_payload.raw_payload_hash = hash ? hash : 1U;
    level->structure3_payload.valid = 1;
    level->structure3_payload.face_semantics_proven = 0;
    return 0;
}

static int nexus_v1_level_copy_structure2_textures(Nexus_V1_Level *level,
                                                    const uint8_t *data,
                                                    int size)
{
    uint16_t structure2_block;
    uint16_t structure2_blocks;
    uint32_t structure2_useful;
    int structure2_offset;
    int structure2_size;
    int cursor;

    if (!level || !data || size < NEXUS_DGN_BLOCK_SIZE) return -1;
    structure2_block = rb16(data + 0x14);
    structure2_blocks = rb16(data + 0x16);
    structure2_useful = rb32(data + 0x18);
    if (structure2_block == 0U && structure2_blocks == 0U &&
        structure2_useful == 0U) return 0;
    if (structure2_block == 0U || structure2_blocks == 0U ||
        structure2_useful < NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES ||
        structure2_useful > (uint32_t)INT_MAX) return -1;
    structure2_offset = (int)structure2_block * NEXUS_DGN_BLOCK_SIZE;
    structure2_size = (int)structure2_blocks * NEXUS_DGN_BLOCK_SIZE;
    if (structure2_offset > size || structure2_size > size - structure2_offset ||
        structure2_useful > (uint32_t)structure2_size) return -1;

    /* DMWeb establishes only this envelope: Descriptor[20]... FFFF followed
     * by raw palette/image bytes. The bytes after FFFF have no corpus-proven
     * record grammar yet, so keep their span bounded and prohibit promotion. */
    for (cursor = 0; cursor <= (int)structure2_useful - 2;) {
        const uint8_t *src = data + structure2_offset + cursor;
        Nexus_V1_DgnStructure2Texture *dst;
        uint16_t image_id = rb16(src);
        if (image_id == 0xffffU) {
            int descriptor_index;
            int opaque_offset = cursor + 2;
            int opaque_index;
            uint32_t observed_offsets[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES * 2];
            int observed_offset_count = 0;
            level->structure2_texture_table_valid = 1;
            level->structure2_payload.descriptor_bytes = cursor;
            level->structure2_payload.terminator_offset = cursor;
            level->structure2_payload.opaque_payload_offset = opaque_offset;
            level->structure2_payload.opaque_payload_size =
                (int)structure2_useful - opaque_offset;
            for (opaque_index = opaque_offset;
                 opaque_index < (int)structure2_useful;
                 ++opaque_index) {
                if (data[structure2_offset + opaque_index] == 0U) {
                    ++level->structure2_payload.opaque_payload_zero_byte_count;
                } else {
                    ++level->structure2_payload.opaque_payload_nonzero_byte_count;
                }
            }
            for (opaque_index = opaque_offset;
                 opaque_index + 1 < (int)structure2_useful;
                 opaque_index += 2) {
                ++level->structure2_payload.opaque_payload_complete_pair_count;
                if (data[structure2_offset + opaque_index] == 0U &&
                    data[structure2_offset + opaque_index + 1] == 0U) {
                    ++level->structure2_payload.opaque_payload_zero_pair_count;
                } else {
                    ++level->structure2_payload.opaque_payload_nonzero_pair_count;
                }
            }
            level->structure2_payload.opaque_payload_trailing_byte_count =
                level->structure2_payload.opaque_payload_size & 1;
            for (descriptor_index = 0;
                 descriptor_index < level->structure2_texture_count;
                 ++descriptor_index) {
                const Nexus_V1_DgnStructure2Texture *descriptor =
                    &level->structure2_textures[descriptor_index];
                uint32_t offsets[2];
                int offset_index;

                offsets[0] = descriptor->image_relative_offset;
                offsets[1] = descriptor->palette_relative_offset;
                for (offset_index = 0; offset_index < 2; ++offset_index) {
                    uint32_t relative_offset = offsets[offset_index];
                    int observed_index;
                    int already_observed = 0;
                    if (relative_offset == 0U) continue;
                    ++level->structure2_payload.nonzero_descriptor_offset_count;
                    for (observed_index = 0;
                         observed_index < observed_offset_count;
                         ++observed_index) {
                        if (observed_offsets[observed_index] == relative_offset) {
                            already_observed = 1;
                            break;
                        }
                    }
                    if (already_observed) {
                        ++level->structure2_payload
                              .nonzero_descriptor_offset_reused_count;
                    } else {
                        observed_offsets[observed_offset_count++] = relative_offset;
                        ++level->structure2_payload
                              .nonzero_descriptor_offset_unique_count;
                    }
                    if (relative_offset >= (uint32_t)opaque_offset &&
                        relative_offset < structure2_useful) {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_in_opaque_payload_count;
                    } else {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_outside_opaque_payload_count;
                    }
                    if (relative_offset >= (uint32_t)opaque_offset &&
                        relative_offset <= structure2_useful - 2U) {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_word_bounded_count;
                    }
                    if ((relative_offset & 1U) != 0U) {
                        ++level->structure2_payload
                            .nonzero_descriptor_offsets_unaligned_count;
                    }
                }
            }
            level->structure2_payload.local_payload_offset_pattern_observed =
                level->structure2_payload.nonzero_descriptor_offset_count > 0 &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_outside_opaque_payload_count == 0;
            level->structure2_payload
                .local_payload_word_aligned_offset_pattern_observed =
                level->structure2_payload.local_payload_offset_pattern_observed &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_unaligned_count == 0;
            level->structure2_payload
                .local_payload_word_bounded_offset_pattern_observed =
                level->structure2_payload.local_payload_offset_pattern_observed &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_word_bounded_count ==
                    level->structure2_payload.nonzero_descriptor_offset_count;
            /* Preserve a format-envelope gate separately from the measured
             * corpus patterns above. Zero offsets are allowed; each present
             * target must remain aligned and fully bounded in its descriptor
             * envelope before Structure1G can hand it to a host. */
            level->structure2_payload.descriptor_offset_envelope_valid =
                level->structure2_payload
                    .nonzero_descriptor_offsets_outside_opaque_payload_count == 0 &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_unaligned_count == 0 &&
                level->structure2_payload
                    .nonzero_descriptor_offsets_word_bounded_count ==
                    level->structure2_payload.nonzero_descriptor_offset_count;
            level->structure2_payload.valid = 1;
            /* No decoder may promote this opaque span into a material. */
            level->structure2_payload.material_or_image_data_proven = 0;
            return 0;
        }
        if (cursor > (int)structure2_useful -
                NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES ||
            level->structure2_texture_count >= NEXUS_DGN_MAX_STRUCTURE2_TEXTURES ||
            image_id != (uint16_t)level->structure2_texture_count) return -1;
        dst = &level->structure2_textures[level->structure2_texture_count];
        dst->image_id = image_id;
        dst->encoding = rb16(src + 2);
        dst->palette_id = rb16(src + 4);
        dst->width = rb16(src + 6);
        dst->height = rb16(src + 8);
        dst->image_relative_offset = rb32(src + 12);
        dst->palette_relative_offset = rb32(src + 16);
        level->structure2_texture_count++;
        cursor += NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES;
    }
    return -1;
}

static int nexus_v1_dgn_parse_structure1g(
    Nexus_V1_DgnStructure1GTable *out_table,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    const Nexus_V1_DgnStructure1PostGridPointer *span;
    Nexus_V1_DgnStructure1GTable table;
    const uint8_t *src;
    int descriptor;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!out_table || !data || !layout) return -1;
    span = &layout->post_grid[1];
    if (!span->present) return 0;
    if (!span->bounded || span->relative_offset < 0 ||
        span->relative_offset > layout->useful_size - NEXUS_DGN_STRUCTURE1G_HEADER_BYTES)
        return -1;

    memset(&table, 0, sizeof(table));
    table.relative_offset = span->relative_offset;
    /* Structure1 header pointers are not address ordered. Structure1G ends
     * at its own FF FF instruction, so its enclosing bound is useful
     * Structure1 data, never the next named header pointer. */
    table.size = layout->useful_size - span->relative_offset;
    src = data + layout->structure1_offset + span->relative_offset;
    table.descriptor_count = (int)rb16(src);
    table.animation_data_relative_offset = (int)rb16(src + 2);
    if (table.descriptor_count < 1 ||
        table.descriptor_count > NEXUS_DGN_MAX_STRUCTURE1G_ENTRIES ||
        table.animation_data_relative_offset !=
            NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
            table.descriptor_count * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES ||
        table.animation_data_relative_offset > table.size) return -1;

    for (descriptor = 0; descriptor < table.descriptor_count; ++descriptor) {
        const uint8_t *entry = src + NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
            descriptor * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES;
        uint16_t sequence_word_offset;
        int next_sequence_word_offset;
        int sequence_byte_offset;
        int cursor;
        int terminated = 0;
        if (descriptor == table.descriptor_count - 1) {
            if (entry[0] != 0xffU) return -1;
            continue;
        }
        if (entry[0] == 0xffU || entry[1] != 0U || rb16(entry + 2) != 1U)
            return -1;
        sequence_word_offset = rb16(entry + 6);
        next_sequence_word_offset = descriptor + 1 < table.descriptor_count - 1
            ? (int)rb16(entry + NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES + 6)
            : (table.size - table.animation_data_relative_offset) / 4;
        sequence_byte_offset = table.animation_data_relative_offset +
            (int)sequence_word_offset * 4;
        if (sequence_byte_offset < table.animation_data_relative_offset ||
            sequence_word_offset >= (uint16_t)next_sequence_word_offset ||
            sequence_byte_offset > table.size - 4 ||
            rb16(src + sequence_byte_offset) != rb16(entry + 4)) return -1;
        for (cursor = sequence_byte_offset;
             cursor < table.animation_data_relative_offset +
                 next_sequence_word_offset * 4;
             cursor += 4) {
            uint16_t instruction = rb16(src + cursor);
            if (instruction == 0xffffU) {
                terminated = 1;
                break;
            }
            if (instruction == 0xfffeU) {
                int target = (int)(int16_t)rb16(src + cursor + 2);
                int instruction_index =
                    (cursor - sequence_byte_offset) / 4;
                if (target >= 0 || -target > instruction_index) return -1;
                table.goto_instruction_count++;
                continue;
            }
            if (instruction < NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX)
                return -1;
            table.image_instruction_count++;
        }
        if (!terminated) return -1;
        table.animated_texture_count++;
        table.sequence_count++;
    }
    table.valid = 1;
    *out_table = table;
    return 0;
}

static int nexus_v1_dgn_parse_structure1a(
    Nexus_V1_DgnStructure1ATable *out_table,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    const uint8_t *src;
    Nexus_V1_DgnStructure1ATable table;
    uint32_t count;
    uint32_t relative_offset;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!out_table || !data || !layout) return -1;
    src = data + layout->structure1_offset;
    count = rb32(src + 0x0c);
    relative_offset = rb32(src + 0x10);
    if (relative_offset != 0x38U ||
        count > NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES ||
        count > (uint32_t)(INT_MAX / NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES)) {
        return -1;
    }
    memset(&table, 0, sizeof(table));
    table.relative_offset = (int)relative_offset;
    table.entry_count = (int)count;
    table.size = table.entry_count * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
    if (table.relative_offset > layout->structure1b_relative_offset ||
        table.size > layout->structure1b_relative_offset - table.relative_offset) {
        return -1;
    }
    table.valid = 1;
    *out_table = table;
    return 0;
}

static int nexus_v1_dgn_parse_structure1f(
    Nexus_V1_DgnStructure1FTable *out_table,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    static const int record_sizes[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {8, 12, 16, 12, 12, 16};
    static const uint8_t tags[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        {0x10U, 0x11U, 0x12U, 0x20U, 0x21U, 0x22U};
    const Nexus_V1_DgnStructure1PostGridPointer *span;
    Nexus_V1_DgnStructure1FTable table;
    const uint8_t *src;
    int cursor;
    int family;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!out_table || !data || !layout) return -1;
    span = &layout->post_grid[5];
    if (!span->present || !span->bounded ||
        span->size_to_next < NEXUS_DGN_STRUCTURE1F_HEADER_BYTES) return -1;

    memset(&table, 0, sizeof(table));
    table.relative_offset = span->relative_offset;
    table.size = span->size_to_next;
    src = data + layout->structure1_offset + span->relative_offset;
    table.wall_sensor_first_texture_index = rb16(src);
    table.wall_sensor_first_model_index = rb16(src + 2);
    cursor = NEXUS_DGN_STRUCTURE1F_HEADER_BYTES;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int count = (int)rb16(src + 4 + family * 2);
        int bytes;
        if (count > NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES ||
            count > (INT_MAX - cursor) / record_sizes[family]) return -1;
        bytes = count * record_sizes[family];
        if (bytes > table.size - cursor ||
            table.total_entry_count > NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES - count)
            return -1;
        table.family_count[family] = count;
        table.family_offset[family] = span->relative_offset + cursor;
        table.family_record_size[family] = record_sizes[family];
        table.total_entry_count += count;
        cursor += bytes;
    }
    /* Structure1F is the final useful Structure1 span. Its six counts must
     * account for the complete span: padding belongs outside useful data. */
    if (cursor != table.size) return -1;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int record;
        const uint8_t *records = data + layout->structure1_offset +
            table.family_offset[family];
        for (record = 0; record < table.family_count[family]; ++record) {
            const uint8_t *entry = records + record * record_sizes[family];
            if (entry[0] != tags[family]) return -1;
        }
    }
    /* Exact final-span counts and the six source tags are enough to retain
     * a Structure1F declaration.  Keep it even when direct-cell validation
     * below fails so host handoff cannot silently lose an original record. */
    table.declared = 1;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        int record;
        const uint8_t *records = data + layout->structure1_offset +
            table.family_offset[family];
        for (record = 0; record < table.family_count[family]; ++record) {
            const uint8_t *entry = records + record * record_sizes[family];
            /* Structure1Fa through Structure1Fc carry documented 64x64
             * coordinates. Alcove and wall records bind through Structure1A. */
            if (family <= NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS &&
                (entry[1] >= NEXUS_MAX_MAP_SIZE || entry[2] >= NEXUS_MAX_MAP_SIZE)) {
                *out_table = table;
                return 0;
            }
        }
    }
    table.valid = 1;
    *out_table = table;
    return 0;
}

static int nexus_v1_decode_structure1b_collision_ref(const uint8_t *cell) {
    if (!cell) {
        return 0;
    }
    return (int)((((unsigned)cell[6] & 0x0FU) << 8) | (unsigned)cell[7]);
}

static uint8_t nexus_v1_decode_structure1b_wall_material(
    const uint8_t *cell, int wall_dir) {
    /* DMWeb DGN Structure1B: byte 3 holds north/east surface ids and
     * byte 4 holds south/west ids. The directional bit is the missing link
     * between the level's material references and the mesh command. */
    return cell[(wall_dir & 3) < 2 ? 3 : 4];
}

static uint16_t nexus_v1_decode_structure1b_post_grid_0x30_ref(
    const uint8_t *cell) {
    return (uint16_t)((((unsigned)cell[5] << 4) |
                       ((unsigned)cell[6] >> 4)) & 0x0fffU);
}

static int nexus_v1_decode_structure1b_structure1a_ref(
    const uint8_t *cell, uint16_t *out_ref)
{
    if (!cell || !out_ref || (cell[4] & 0x80U) == 0U) return 0;
    *out_ref = (uint16_t)((((unsigned)cell[5] << 4) |
                           ((unsigned)cell[6] >> 4)) & 0x0fffU);
    return 1;
}

static int nexus_v1_post_grid_0x30_ref_is_bounded(
    const Nexus_V1_DgnStructure1Layout *layout, int ref) {
    if (ref == 0 || ref == 0x0fff) {
        return 1;
    }
    return layout && layout->post_grid_0x30_records.valid &&
        ref >= 0 &&
        ref < layout->post_grid_0x30_records.typed_prefix_record_count;
}

static uint8_t nexus_v1_decode_structure1b_floor_material(const uint8_t *cell) {
    return (uint8_t)((rb16(cell) >> 7) & 0x1fU);
}

static int nexus_v1_decode_structure1b_floor_animation_id(
    const uint8_t *cell, uint8_t *out_id) {
    if (!cell || !out_id || (cell[4] & 0x0fU) != 3U) return 0;
    *out_id = nexus_v1_decode_structure1b_floor_material(cell);
    return 1;
}

static uint8_t nexus_v1_decode_structure1b_ceiling_material(
    const uint8_t *header, const uint8_t *cell) {
    unsigned selection = (rb16(cell) >> 1) & 3U;
    return selection == 0U ? 0U : header[8 + selection];
}

static int nexus_v1_decode_structure1b_cell(const uint8_t *cell) {
    uint16_t flags;
    unsigned collision;
    if (!cell) {
        return 0;
    }
    flags = rb16(cell);
    collision = (unsigned)nexus_v1_decode_structure1b_collision_ref(cell);
    if (collision == 0x0FFFU) {
        return 0; /* wall / cannot enter */
    }
    if ((flags & 0x0001U) != 0) {
        return 8; /* door present */
    }
    return 1; /* free corridor/floor */
}

int nexus_v1_dgn_structure1_layout(Nexus_V1_DgnStructure1Layout *out_layout,
                                   const uint8_t *data,
                                   int size) {
    static const int header_offsets[NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT] =
        {0x18, 0x1c, 0x24, 0x2c, 0x30, 0x34};
    Nexus_V1_DgnStructure1Layout layout;
    uint16_t block;
    uint16_t blocks;
    uint32_t useful;
    int i;

    if (out_layout) memset(out_layout, 0, sizeof(*out_layout));
    if (!out_layout || !data || size < NEXUS_DGN_BLOCK_SIZE) return -1;
    block = rb16(data + 0x0c);
    blocks = rb16(data + 0x0e);
    useful = rb32(data + 0x10);
    if (block == 0 || blocks == 0 ||
        block > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        blocks > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) return -1;
    memset(&layout, 0, sizeof(layout));
    layout.structure1_offset = (int)block * NEXUS_DGN_BLOCK_SIZE;
    if (layout.structure1_offset + 0x38 > size ||
        (int)blocks * NEXUS_DGN_BLOCK_SIZE > size - layout.structure1_offset ||
        useful > (uint32_t)((int)blocks * NEXUS_DGN_BLOCK_SIZE)) return -1;
    layout.useful_size = (int)useful;
    layout.structure1b_relative_offset =
        (int)rb32(data + layout.structure1_offset + 0x14);
    if (layout.structure1b_relative_offset < 0 ||
        layout.structure1b_relative_offset > layout.useful_size ||
        layout.structure1b_relative_offset + NEXUS_DGN_STRUCTURE1B_BYTES >
            layout.useful_size) return -1;
    layout.structure1b_end_relative_offset =
        layout.structure1b_relative_offset + NEXUS_DGN_STRUCTURE1B_BYTES;
    layout.post_grid_offset = layout.structure1b_end_relative_offset;
    layout.post_grid_size = layout.useful_size - layout.post_grid_offset;
    for (i = 0; i < NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT; ++i) {
        Nexus_V1_DgnStructure1PostGridPointer *pointer = &layout.post_grid[i];
        int next = layout.useful_size;
        int j;
        pointer->header_offset = header_offsets[i];
        pointer->relative_offset =
            (int)rb32(data + layout.structure1_offset + pointer->header_offset);
        pointer->present = pointer->relative_offset != 0;
        if (!pointer->present) continue;
        if (pointer->relative_offset < layout.post_grid_offset ||
            pointer->relative_offset > layout.useful_size) return -1;
        for (j = 0; j < NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT; ++j) {
            int candidate = (int)rb32(data + layout.structure1_offset +
                                      header_offsets[j]);
            if (candidate > pointer->relative_offset && candidate < next)
                next = candidate;
        }
        pointer->size_to_next = next - pointer->relative_offset;
        pointer->bounded = 1;
    }
    {
        const Nexus_V1_DgnStructure1PostGridPointer *collision_span =
            &layout.post_grid[0];
        int record_count;

        if (collision_span->present && collision_span->bounded &&
            collision_span->size_to_next > 0 &&
            collision_span->size_to_next %
                NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES == 0) {
            record_count = collision_span->size_to_next /
                           NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES;
            if (data[layout.structure1_offset +
                     collision_span->relative_offset] ==
                    (uint8_t)record_count) {
                layout.structure1c.relative_offset =
                    collision_span->relative_offset;
                layout.structure1c.size = collision_span->size_to_next;
                layout.structure1c.record_size =
                    NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES;
                layout.structure1c.record_count = record_count;
                layout.structure1c.indexed_record_count = record_count - 1;
                layout.structure1c.valid = 1;
            }
        }
    }
    {
        const Nexus_V1_DgnStructure1PostGridPointer *zero_span =
            &layout.post_grid[2];
        const Nexus_V1_DgnStructure1PostGridPointer *records =
            &layout.post_grid[4];
        int i;

        if (zero_span->present && zero_span->bounded &&
            zero_span->size_to_next == NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES) {
            int all_zero = 1;
            for (i = 0; i < zero_span->size_to_next; ++i) {
                if (data[layout.structure1_offset + zero_span->relative_offset + i] != 0) {
                    all_zero = 0;
                    break;
                }
            }
            if (all_zero) {
                layout.post_grid_0x24_zero_span.relative_offset =
                    zero_span->relative_offset;
                layout.post_grid_0x24_zero_span.size = zero_span->size_to_next;
                layout.post_grid_0x24_zero_span.valid = 1;
            }
        }
        if (records->present && records->bounded && records->size_to_next > 0 &&
            records->size_to_next % NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES == 0) {
            unsigned char values[NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES][256];
            int record;
            int byte;
            layout.post_grid_0x30_records.relative_offset = records->relative_offset;
            layout.post_grid_0x30_records.size = records->size_to_next;
            layout.post_grid_0x30_records.record_size =
                NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
            layout.post_grid_0x30_records.record_count =
                records->size_to_next / NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
            layout.post_grid_0x30_records.opaque_tail_record_count = 1;
            layout.post_grid_0x30_records.typed_prefix_record_count =
                layout.post_grid_0x30_records.record_count - 1;
            layout.post_grid_0x30_records.first_row_ordinal_flagged_prefix_record =
                -1;
            layout.post_grid_0x30_records.last_row_ordinal_flagged_prefix_record =
                -1;
            memset(values, 0, sizeof(values));
            for (record = 0;
                 record < layout.post_grid_0x30_records.record_count;
                 ++record) {
                const uint8_t *src = data + layout.structure1_offset +
                    records->relative_offset +
                    record * NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
                for (byte = 0; byte < NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
                     ++byte) {
                    if (!values[byte][src[byte]]) {
                        values[byte][src[byte]] = 1U;
                        layout.post_grid_0x30_records
                            .field_distinct_value_count[byte]++;
                    }
                }
            }
            if (layout.post_grid_0x30_records.typed_prefix_record_count > 0) {
                int ordinal_valid = 1;
                for (record = 0;
                     record < layout.post_grid_0x30_records.typed_prefix_record_count;
                     ++record) {
                    const uint8_t *src = data + layout.structure1_offset +
                        records->relative_offset +
                        record * NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES;
                    uint8_t ordinal =
                        src[NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE];
                    if ((ordinal & NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK) !=
                            (uint8_t)record ||
                        (ordinal & ~(NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK |
                                     NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK)) != 0U) {
                        ordinal_valid = 0;
                        break;
                    }
                    if ((ordinal & NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK) !=
                        0U) {
                        layout.post_grid_0x30_records
                            .row_ordinal_flagged_prefix_record_count++;
                        if (layout.post_grid_0x30_records
                                .first_row_ordinal_flagged_prefix_record < 0) {
                            layout.post_grid_0x30_records
                                .first_row_ordinal_flagged_prefix_record = record;
                        }
                        layout.post_grid_0x30_records
                            .last_row_ordinal_flagged_prefix_record = record;
                    }
                }
                layout.post_grid_0x30_records.row_ordinal_prefix_valid =
                    ordinal_valid;
            }
            layout.post_grid_0x30_records.valid =
                layout.post_grid_0x30_records.row_ordinal_prefix_valid;
        }
    }
    /* DMWeb DGN files: pointer 0x34 is Structure1F, not an opaque tail.
     * Its counted families are decoded only when the entire final useful span
     * has an exact, source-backed layout. */
    (void)nexus_v1_dgn_parse_structure1a(&layout.structure1a, data, &layout);
    (void)nexus_v1_dgn_parse_structure1f(&layout.structure1f, data, &layout);
    (void)nexus_v1_dgn_parse_structure1g(&layout.structure1g, data, &layout);
    layout.valid = 1;
    *out_layout = layout;
    return 0;
}

int nexus_v1_dgn_geometry_info(Nexus_V1_DgnGeometryInfo *out_info,
                               const uint8_t *data,
                               int size) {
    Nexus_V1_DgnGeometryInfo info;
    uint16_t structure1_block;
    uint16_t structure1_blocks;
    uint32_t structure1_useful;
    int structure1_offset;
    int structure1_size;
    const uint8_t *structure1;
    uint32_t structure1b_rel;
    int structure1b_offset;
    int geometry_offset;
    int geometry_size;
    Nexus_V1_DgnStructure1Layout layout;
    unsigned char seen_refs[4096];
    unsigned char seen_post_grid_0x30_refs[4096];
    int y;
    int x;

    if (out_info) {
        memset(out_info, 0, sizeof(*out_info));
    }
    if (!out_info || !data || size < NEXUS_DGN_BLOCK_SIZE) {
        return -1;
    }

    memset(&info, 0, sizeof(info));
    structure1_block = rb16(data + 0x0C);
    structure1_blocks = rb16(data + 0x0E);
    structure1_useful = rb32(data + 0x10);
    if (structure1_block == 0 || structure1_blocks == 0 ||
        structure1_blocks > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE) ||
        structure1_block > (uint16_t)(INT_MAX / NEXUS_DGN_BLOCK_SIZE)) {
        return -1;
    }

    structure1_offset = (int)structure1_block * NEXUS_DGN_BLOCK_SIZE;
    structure1_size = (int)structure1_blocks * NEXUS_DGN_BLOCK_SIZE;
    if (structure1_offset < NEXUS_DGN_BLOCK_SIZE ||
        structure1_offset + 0x38 > size ||
        structure1_size <= 0 ||
        structure1_offset + structure1_size > size ||
        structure1_useful > (uint32_t)structure1_size) {
        return -1;
    }

    structure1 = data + structure1_offset;
    structure1b_rel = rb32(structure1 + 0x14);
    if (structure1[2] != 0x40 || structure1[3] != 0x40 ||
        structure1b_rel > (uint32_t)structure1_size ||
        structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES > structure1_useful) {
        return -1;
    }

    structure1b_offset = structure1_offset + (int)structure1b_rel;
    geometry_offset = structure1b_offset + NEXUS_DGN_STRUCTURE1B_BYTES;
    geometry_size = (int)structure1_useful -
                    ((int)structure1b_rel + NEXUS_DGN_STRUCTURE1B_BYTES);
    if (structure1b_offset < structure1_offset ||
        geometry_offset > size ||
        geometry_size < 0 ||
        geometry_offset + geometry_size > size) {
        return -1;
    }
    if (nexus_v1_dgn_structure1_layout(&layout, data, size) != 0) {
        return -1;
    }

    /*
     * DMWeb DGN Structure1B source-lock:
     * bytes 5..7 pack two 12-bit values; Firestaff's current renderer needs
     * the low collision descriptor reference to be bounded before a real
     * Structure1C/mesh reader can replace the procedural fallback.
     */
    memset(seen_refs, 0, sizeof(seen_refs));
    memset(seen_post_grid_0x30_refs, 0, sizeof(seen_post_grid_0x30_refs));
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            int off = structure1b_offset +
                      ((y * NEXUS_MAX_MAP_SIZE + x) *
                       NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
            int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
            int post_grid_0x30_ref =
                nexus_v1_decode_structure1b_post_grid_0x30_ref(data + off);
            if (ref != 0 && ref != 0x0FFF) {
                info.collision_ref_count++;
                if (!seen_refs[ref]) {
                    seen_refs[ref] = 1U;
                    info.collision_ref_unique_count++;
                }
                if (ref > info.max_collision_ref) {
                    info.max_collision_ref = ref;
                }
            }
            if (post_grid_0x30_ref != 0 && post_grid_0x30_ref != 0x0FFF) {
                info.post_grid_0x30_ref_count++;
                if (!seen_post_grid_0x30_refs[post_grid_0x30_ref]) {
                    seen_post_grid_0x30_refs[post_grid_0x30_ref] = 1U;
                    info.post_grid_0x30_ref_unique_count++;
                }
                if (post_grid_0x30_ref > info.max_post_grid_0x30_ref) {
                    info.max_post_grid_0x30_ref = post_grid_0x30_ref;
                }
                if (!nexus_v1_post_grid_0x30_ref_is_bounded(
                        &layout, post_grid_0x30_ref)) {
                    info.post_grid_0x30_invalid_ref_count++;
                    if (info.first_invalid_post_grid_0x30_ref == 0) {
                        info.first_invalid_post_grid_0x30_ref =
                            post_grid_0x30_ref;
                    }
                }
            }
        }
    }

    info.dmweb_container = 1;
    info.structure1_offset = structure1_offset;
    info.structure1_size = structure1_size;
    info.structure1_useful_size = (int)structure1_useful;
    info.structure1b_offset = structure1b_offset;
    info.structure1b_size = NEXUS_DGN_STRUCTURE1B_BYTES;
    info.geometry_offset = geometry_offset;
    info.geometry_size = geometry_size;
    if (layout.structure1c.valid) {
        info.structure1c_offset = structure1_offset +
                                  layout.structure1c.relative_offset;
        info.structure1c_size = layout.structure1c.size;
        info.structure1c_record_count = layout.structure1c.record_count;
        info.structure1c_indexed_record_count =
            layout.structure1c.indexed_record_count;
        info.collision_records_valid =
            info.max_collision_ref < layout.structure1c.record_count;
    }
    info.post_grid_0x24_zero_span_valid =
        layout.post_grid_0x24_zero_span.valid;
    info.post_grid_0x24_zero_span_size = layout.post_grid_0x24_zero_span.size;
    info.post_grid_0x30_record_table_valid =
        layout.post_grid_0x30_records.valid;
    info.post_grid_0x30_record_count =
        layout.post_grid_0x30_records.record_count;
    info.post_grid_0x30_typed_prefix_record_count =
        layout.post_grid_0x30_records.typed_prefix_record_count;
    info.post_grid_0x30_opaque_tail_record_count =
        layout.post_grid_0x30_records.opaque_tail_record_count;
    info.post_grid_0x30_row_ordinal_prefix_valid =
        layout.post_grid_0x30_records.row_ordinal_prefix_valid;
    info.post_grid_0x30_row_ordinal_flagged_prefix_record_count =
        layout.post_grid_0x30_records.row_ordinal_flagged_prefix_record_count;
    info.post_grid_0x30_first_row_ordinal_flagged_prefix_record =
        layout.post_grid_0x30_records.first_row_ordinal_flagged_prefix_record;
    info.post_grid_0x30_last_row_ordinal_flagged_prefix_record =
        layout.post_grid_0x30_records.last_row_ordinal_flagged_prefix_record;
    info.post_grid_0x30_ref_value_count = info.post_grid_0x30_ref_count;
    info.structure1f_declared = layout.structure1f.declared;
    info.structure1f_valid = layout.structure1f.valid;
    info.structure1f_total_entry_count = layout.structure1f.total_entry_count;
    memcpy(info.structure1f_family_count, layout.structure1f.family_count,
           sizeof(info.structure1f_family_count));
    info.structure1g_present = layout.post_grid[1].present;
    info.structure1g_valid = layout.structure1g.valid;
    info.structure1g_animated_texture_count =
        layout.structure1g.animated_texture_count;
    info.structure1g_sequence_count = layout.structure1g.sequence_count;
    info.post_grid_0x30_references_valid =
        info.post_grid_0x30_record_table_valid &&
        info.post_grid_0x30_invalid_ref_count == 0;
    if (info.collision_records_valid &&
        info.post_grid_0x30_record_table_valid &&
        info.post_grid_0x30_row_ordinal_prefix_valid &&
        info.post_grid_0x30_references_valid) {
        info.mesh_ready = 1;
    }

    *out_info = info;
    return 0;
}

static void nexus_v1_level_copy_structure1g_entries(
    Nexus_V1_Level *level,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    const Nexus_V1_DgnStructure1GTable *table;
    const uint8_t *src;
    int descriptor;
    int output = 0;
    if (!level || !data || !layout || !layout->structure1g.valid) return;
    table = &layout->structure1g;
    src = data + layout->structure1_offset + table->relative_offset;
    for (descriptor = 0; descriptor < table->descriptor_count - 1; ++descriptor) {
        const uint8_t *entry = src + NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
            descriptor * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES;
        const int sequence_offset = table->animation_data_relative_offset +
            (int)rb16(entry + 6) * 4;
        int cursor;
        Nexus_V1_DgnStructure1GEntry *dst = &level->structure1g_entries[output++];
        dst->animation_id = entry[0];
        dst->first_image_index = rb16(entry + 4);
        if (dst->first_image_index >= NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) {
            dst->first_structure2_image_id = (uint16_t)(
                dst->first_image_index - NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX);
            dst->first_structure2_image_valid =
                nexus_v1_level_find_structure2_texture(
                    level, dst->first_structure2_image_id);
        }
        dst->sequence_word_offset = rb16(entry + 6);
        for (cursor = sequence_offset; cursor <= table->size - 4; cursor += 4) {
            uint16_t instruction = rb16(src + cursor);
            dst->sequence_instruction_count++;
            if (instruction == 0xffffU) break;
            if (instruction == 0xfffeU) dst->goto_instruction_count++;
            else {
                uint16_t local_image_id;
                dst->image_instruction_count++;
                /* The parser accepts an instruction only after validating
                 * the Structure1G image-space lower bound. Keep a second
                 * defensive check here because level-load must never turn a
                 * malformed instruction into a local-table lookup. */
                if (instruction < NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) {
                    dst->structure2_image_instruction_unbound_count++;
                    continue;
                }
                local_image_id = (uint16_t)(
                    instruction - NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX);
                if (nexus_v1_level_find_structure2_texture(level,
                                                            local_image_id)) {
                    dst->structure2_image_instruction_bound_count++;
                } else {
                    dst->structure2_image_instruction_unbound_count++;
                }
            }
        }
    }
    level->structure1g_entry_count = output;
}

static void nexus_v1_level_finalize_structure1g_structure2_bindings(
    Nexus_V1_Level *level)
{
    int entry;

    if (!level) return;
    if (!level->geometry_info.structure1g_present) {
        level->structure1g_structure2_bindings_complete = 1;
        return;
    }

    /* DMWeb DGN Structure1G uses the global image space while Structure2
     * owns the local descriptor table. Keep the already proven
     * global-to-local relation whole: one unresolved instruction means this
     * level cannot claim a usable animated-material declaration. This does
     * not decode the opaque payload or make any material drawable. */
    level->structure1g_structure2_bindings_complete =
        level->structure2_texture_table_valid &&
        level->structure2_payload.valid &&
        level->structure2_payload.descriptor_offset_envelope_valid &&
        level->structure1g_entry_count > 0;
    for (entry = 0;
         entry < level->structure1g_entry_count &&
         level->structure1g_structure2_bindings_complete;
         ++entry) {
        const Nexus_V1_DgnStructure1GEntry *declaration =
            &level->structure1g_entries[entry];
        if (!declaration->first_structure2_image_valid ||
            declaration->structure2_image_instruction_unbound_count != 0) {
            level->structure1g_structure2_bindings_complete = 0;
        }
    }
}

static void nexus_v1_level_copy_structure1f_entries(
    Nexus_V1_Level *level,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    int family;
    int output = 0;
    if (!level || !data || !layout || !layout->structure1f.valid) return;
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        const int count = layout->structure1f.family_count[family];
        const int record_size = layout->structure1f.family_record_size[family];
        const uint8_t *records = data + layout->structure1_offset +
            layout->structure1f.family_offset[family];
        int record;
        for (record = 0; record < count; ++record) {
            const uint8_t *src = records + record * record_size;
            Nexus_V1_DgnStructure1FEntry *dst =
                &level->structure1f_entries[output++];
            dst->family = (Nexus_V1_DgnStructure1FFamily)family;
            dst->tag = src[0];
            switch (family) {
            case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
                dst->x = src[1]; dst->y = src[2]; dst->location = src[3];
                dst->item_id = src[4]; dst->attribute1 = src[5];
                dst->attribute2 = src[7];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
                dst->x = src[1]; dst->y = src[2];
                dst->offset_x = (int8_t)src[3]; dst->offset_y = (int8_t)src[4];
                dst->model_or_aspect = src[5]; dst->rotation = src[6];
                dst->type_or_control = src[7]; dst->width = src[8];
                dst->height = src[9];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
                dst->x = src[1]; dst->y = src[2];
                dst->model_or_aspect = src[5]; dst->rotation = src[6];
                dst->width = src[10]; dst->height = src[11];
                dst->type_or_control = src[12]; dst->destination_x = src[13];
                dst->destination_y = src[14]; dst->destination_orientation = src[15];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES:
                dst->face = src[1]; dst->structure1a_index = rb16(src + 2);
                dst->rotation = src[4]; dst->offset_x = (int8_t)src[5];
                dst->offset_y = (int8_t)src[6]; dst->item_id = src[7];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
                dst->face = src[1]; dst->structure1a_index = rb16(src + 2);
                dst->rotation = src[4]; dst->offset_x = (int8_t)src[5];
                dst->offset_y = (int8_t)src[6]; dst->model_or_aspect = src[7];
                break;
            case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
                dst->face = src[1]; dst->structure1a_index = rb16(src + 2);
                dst->rotation = src[4]; dst->offset_x = (int8_t)src[5];
                dst->offset_y = (int8_t)src[6]; dst->model_or_aspect = src[7];
                dst->type_or_control = src[12]; dst->destination_x = src[13];
                dst->destination_y = src[14]; dst->destination_orientation = src[15];
                break;
            }
        }
    }
    level->structure1f_entry_count = output;
}

static void nexus_v1_level_copy_structure1a_models(
    Nexus_V1_Level *level,
    const uint8_t *data,
    const Nexus_V1_DgnStructure1Layout *layout)
{
    int index;

    if (!level || !data || !layout || !layout->structure1a.valid) return;
    for (index = 0; index < layout->structure1a.entry_count; ++index) {
        const uint8_t *src = data + layout->structure1_offset +
            layout->structure1a.relative_offset +
            index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
        level->structure1a_models[index].kind = src[0];
        level->structure1a_models[index].structure3_model_index = src[1];
        level->structure1a_models[index].z_rotation = src[2];
    }
    level->structure1a_model_count = layout->structure1a.entry_count;
    level->structure1a_table_valid = 1;
}

static void nexus_v1_level_resolve_structure1a_relations(Nexus_V1_Level *level)
{
    int owner_count[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int owner_x[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int owner_y[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int x;
    int y;
    int entry;

    if (!level || !level->structure1a_table_valid) return;
    memset(owner_count, 0, sizeof(owner_count));
    memset(owner_x, 0, sizeof(owner_x));
    memset(owner_y, 0, sizeof(owner_y));
    for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
        for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
            uint16_t ref;
            if (!level->structure1a_owner_ref_valid[y][x]) continue;
            ref = level->structure1a_owner_refs[y][x];
            if (ref >= (uint16_t)level->structure1a_model_count) continue;
            ++owner_count[ref];
            owner_x[ref] = x;
            owner_y[ref] = y;
        }
    }
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
            record->structure1a_index >= (uint16_t)level->structure1a_model_count ||
            owner_count[record->structure1a_index] != 1) continue;
        record->structure1a_relation_valid = 1;
        record->structure1a_owner_x = owner_x[record->structure1a_index];
        record->structure1a_owner_y = owner_y[record->structure1a_index];
        record->structure1a_structure3_model_index =
            level->structure1a_models[record->structure1a_index]
                .structure3_model_index;
        record->structure1a_z_rotation =
            level->structure1a_models[record->structure1a_index].z_rotation;
    }
}

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index) {
    if (!level || !data || size < 64) return -1;
    memset(level, 0, sizeof(*level));

    /*
     * DMWeb source-lock:
     *   http://dmweb.free.fr/community/documentation/dungeon-master-nexus/dgn-files/
     *   DGN files are 2048-byte block containers. Header offsets at 0x0C,
     *   0x0E and 0x10 locate Structure1; Structure1 offset 0x14 locates
     *   Structure1B, always 0x8000 bytes: 64x64 cells, 8 bytes each.
     */

    if (size >= NEXUS_DGN_BLOCK_SIZE) {
        Nexus_V1_DgnGeometryInfo info;
        if (nexus_v1_dgn_geometry_info(&info, data, size) == 0) {
            int y, x;
            Nexus_V1_DgnStructure1Layout layout;
            level->width = NEXUS_MAX_MAP_SIZE;
            level->height = NEXUS_MAX_MAP_SIZE;
            for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                    int off = info.structure1b_offset +
                              ((y * NEXUS_MAX_MAP_SIZE + x) *
                               NEXUS_DGN_STRUCTURE1B_CELL_BYTES);
                    int ref = nexus_v1_decode_structure1b_collision_ref(data + off);
                    level->squares[y][x] = (uint8_t)nexus_v1_decode_structure1b_cell(data + off);
                    level->collision_refs[y][x] = (uint16_t)ref;
                    level->floor_material_refs[y][x] =
                        nexus_v1_decode_structure1b_floor_material(data + off);
                    level->floor_animation_ids[y][x] = 0xffU;
                    if (nexus_v1_decode_structure1b_floor_animation_id(
                            data + off, &level->floor_animation_ids[y][x])) {
                        int entry;
                        level->structure1g_floor_animation_cell_count++;
                        for (entry = 0; entry < level->structure1g_entry_count;
                             ++entry) {
                            if (level->structure1g_entries[entry].animation_id ==
                                level->floor_animation_ids[y][x]) {
                                level->structure1g_floor_animation_bound_count++;
                                break;
                            }
                        }
                    }
                    level->ceiling_material_refs[y][x] =
                        nexus_v1_decode_structure1b_ceiling_material(
                            data + info.structure1_offset, data + off);
                    level->wall_material_refs[y][x][0] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 0);
                    level->wall_material_refs[y][x][1] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 1);
                    level->wall_material_refs[y][x][2] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 2);
                    level->wall_material_refs[y][x][3] =
                        nexus_v1_decode_structure1b_wall_material(data + off, 3);
                    level->floor_heights[y][x] = (int8_t)data[off + 3];
                    level->floor_slopes[y][x] =
                        (uint8_t)((rb16(data + off) >> 4) & 3U);
                    level->floor_rotations[y][x] =
                        (uint8_t)(rb16(data + off) >> 14);
                    level->post_grid_0x30_refs[y][x] =
                        nexus_v1_decode_structure1b_post_grid_0x30_ref(data + off);
                    level->structure1a_owner_ref_valid[y][x] =
                        nexus_v1_decode_structure1b_structure1a_ref(
                            data + off, &level->structure1a_owner_refs[y][x]) ? 1U : 0U;
                }
            }
            /* DMWeb proves Structure1B's 12-bit reference into a bounded
             * Structure1C record table, but not the four record bytes as
             * line/circle coordinates. Keep those original references for
             * later evidence; never turn opaque bytes into collision shapes. */
            level->has_3d_geometry = 1;
            level->geometry_offset = info.geometry_offset;
            level->geometry_size = size - info.geometry_offset;
            level->geometry_info = info;
            if (nexus_v1_dgn_structure1_layout(&layout, data, size) == 0) {
                if (nexus_v1_level_copy_structure2_textures(level, data, size) != 0) {
                    level->structure2_texture_count = 0;
                    level->structure2_texture_table_valid = 0;
                }
                if (nexus_v1_level_copy_structure3_payload(level, data, size) != 0) {
                    return -1;
                }
                nexus_v1_level_copy_structure1a_models(level, data, &layout);
                nexus_v1_level_copy_structure1f_entries(level, data, &layout);
                nexus_v1_level_resolve_structure1a_relations(level);
                nexus_v1_level_copy_structure1g_entries(level, data, &layout);
                nexus_v1_level_finalize_structure1g_structure2_bindings(level);
                level->structure1g_floor_animation_bound_count = 0;
                for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                    for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                        int entry;
                        if (level->floor_animation_ids[y][x] == 0xffU) continue;
                        for (entry = 0; entry < level->structure1g_entry_count;
                             ++entry) {
                            if (level->structure1g_entries[entry].animation_id ==
                                level->floor_animation_ids[y][x]) {
                                level->structure1g_floor_animation_bound_count++;
                                break;
                            }
                        }
                    }
                }
            }
            printf("Nexus level %d: 64x64 Structure1B, payload=%d bytes, mesh_span=%d bytes, refs=%d/%d [DMWeb DGN]\n",
                   level_index, level->geometry_size, info.geometry_size,
                   info.collision_ref_unique_count, info.collision_ref_count);
            return 0;
        }
    }

    printf("Nexus level %d: could not parse DGN header (size=%d)\n",
           level_index, size);
    return -1;
}

int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0; /* wall */
    return level->squares[y][x];
}

int nexus_v1_level_get_collision_ref(const Nexus_V1_Level *level, int x, int y) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return 0x0fff;
    return (int)level->collision_refs[y][x];
}

int nexus_v1_level_get_material_ref(const Nexus_V1_Level *level, int x, int y,
                                    Nexus_V1_DgnRenderCommandKind kind,
                                    int wall_dir) {
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return -1;
    if (kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
        kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        if (kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
            return level->ceiling_material_refs[y][x];
        return level->floor_material_refs[y][x];
    }
    return level->wall_material_refs[y][x][wall_dir & 3];
}

int nexus_v1_level_get_cell_geometry(const Nexus_V1_Level *level, int x, int y,
                                     Nexus_V1_DgnCellGeometry *out_cell) {
    Nexus_V1_DgnCellGeometry cell;
    int corner;

    if (!out_cell) return -1;
    memset(out_cell, 0, sizeof(*out_cell));
    if (!level || x < 0 || x >= level->width || y < 0 || y >= level->height)
        return -1;

    memset(&cell, 0, sizeof(cell));
    cell.square_type = level->squares[y][x];
    cell.collision_ref = level->collision_refs[y][x];
    cell.post_grid_0x30_ref = level->post_grid_0x30_refs[y][x];
    cell.floor_material_ref = level->floor_material_refs[y][x];
    cell.ceiling_material_ref = level->ceiling_material_refs[y][x];
    memcpy(cell.wall_material_refs, level->wall_material_refs[y][x],
           sizeof(cell.wall_material_refs));
    cell.floor_slope = level->floor_slopes[y][x];
    cell.floor_rotation = level->floor_rotations[y][x];
    for (corner = 0; corner < 4; ++corner)
        cell.floor_height[corner] = level->floor_heights[y][x];
    if (cell.floor_slope == 2U && x + 1 < level->width) {
        cell.floor_height[1] = cell.floor_height[2] =
            level->floor_heights[y][x + 1];
    } else if (cell.floor_slope == 3U && y + 1 < level->height) {
        cell.floor_height[2] = cell.floor_height[3] =
            level->floor_heights[y + 1][x];
    }
    for (corner = 0; corner < 4; ++corner)
        cell.ceiling_height[corner] = (int8_t)(cell.floor_height[corner] + 32);
    if (cell.collision_ref < NEXUS_DGN_MAX_COLLISION_SECTORS)
        cell.collision_sector = level->collision_sectors[cell.collision_ref];
    cell.post_grid_0x30_row_prefix_valid =
        level->geometry_info.post_grid_0x30_row_ordinal_prefix_valid &&
        (cell.post_grid_0x30_ref == 0 ||
         cell.post_grid_0x30_ref == 0x0fffU ||
         cell.post_grid_0x30_ref <
             (uint16_t)level->geometry_info
                 .post_grid_0x30_typed_prefix_record_count);
    *out_cell = cell;
    return 0;
}

int nexus_v1_level_structure1f_spatial_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FSpatialReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FSpatialReceipt receipt;
    unsigned char direct_cell_seen[NEXUS_MAX_MAP_SIZE * NEXUS_MAX_MAP_SIZE];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(direct_cell_seen, 0, sizeof(direct_cell_seen));
    if (!level || !level->geometry_info.structure1f_valid ||
        level->structure1f_entry_count < 0 ||
        level->structure1f_entry_count !=
            level->geometry_info.structure1f_total_entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.typed_entry_count = level->structure1f_entry_count;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        switch (record->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
            ++receipt.item_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            ++receipt.floor_decoration_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            ++receipt.floor_sensor_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES:
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
            ++receipt.structure1a_bound_entry_count;
            continue;
        default:
            *out_receipt = receipt;
            return 0;
        }
        if (record->x >= (uint8_t)level->width ||
            record->y >= (uint8_t)level->height) {
            *out_receipt = receipt;
            return 0;
        }
        {
            int cell = (int)record->y * NEXUS_MAX_MAP_SIZE + record->x;
            if (direct_cell_seen[cell]) {
                ++receipt.direct_coordinate_duplicate_cell_count;
            } else {
                direct_cell_seen[cell] = 1;
                ++receipt.direct_coordinate_unique_cell_count;
            }
        }
        ++receipt.direct_coordinate_entry_count;
    }
    receipt.valid = receipt.typed_entry_count ==
        receipt.direct_coordinate_entry_count +
            receipt.structure1a_bound_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_boundary_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ABoundaryReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ABoundaryReceipt receipt;
    unsigned char index_seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(index_seen, 0, sizeof(index_seen));
    if (!level || !level->geometry_info.structure1f_valid ||
        level->structure1f_entry_count < 0 ||
        level->structure1f_entry_count !=
            level->geometry_info.structure1f_total_entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        switch (record->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ALCOVES:
            ++receipt.alcove_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS:
            ++receipt.wall_decoration_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS:
            ++receipt.wall_sensor_entry_count;
            break;
        default:
            continue;
        }
        ++receipt.entry_count;
        if (record->structure1a_index == 0U) {
            ++receipt.zero_index_count;
        } else {
            ++receipt.nonzero_index_count;
        }
        if (index_seen[record->structure1a_index]) {
            ++receipt.duplicate_index_count;
        } else {
            index_seen[record->structure1a_index] = 1;
            ++receipt.unique_index_count;
        }
        if (record->structure1a_index > receipt.highest_index) {
            receipt.highest_index = record->structure1a_index;
        }
    }
    receipt.valid = receipt.entry_count ==
        receipt.alcove_entry_count + receipt.wall_decoration_entry_count +
        receipt.wall_sensor_entry_count &&
        receipt.entry_count == receipt.zero_index_count +
        receipt.nonzero_index_count &&
        receipt.entry_count == receipt.unique_index_count +
        receipt.duplicate_index_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_relation_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ARelationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ARelationReceipt receipt;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    if (!level || !level->geometry_info.structure1f_valid) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.table_entry_count = level->structure1a_model_count;
    receipt.table_valid = level->structure1a_table_valid &&
        receipt.table_entry_count >= 0 &&
        receipt.table_entry_count <= NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!receipt.table_valid ||
            record->structure1a_index >= (uint16_t)receipt.table_entry_count) {
            ++receipt.out_of_range_index_count;
        } else if (record->structure1a_relation_valid) {
            ++receipt.resolved_entry_count;
        } else {
            int x;
            int y;
            int owners = 0;
            for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
                for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                    if (level->structure1a_owner_ref_valid[y][x] &&
                        level->structure1a_owner_refs[y][x] ==
                            record->structure1a_index) ++owners;
                }
            }
            if (owners == 0) ++receipt.missing_owner_entry_count;
            else ++receipt.ambiguous_owner_entry_count;
        }
    }
    receipt.complete = receipt.table_valid &&
        receipt.structure1f_bound_entry_count == receipt.resolved_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure3_model_reference_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3ModelReferenceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3ModelReferenceReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t model_index;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_model_reference_count;
        model_index = record->structure1a_structure3_model_index;
        if (model_index == 0U) ++receipt.zero_model_index_count;
        else ++receipt.nonzero_model_index_count;
        if (seen[model_index]) {
            ++receipt.duplicate_model_index_count;
        } else {
            seen[model_index] = 1U;
            ++receipt.unique_model_index_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_model_reference_count ==
            receipt.structure1f_bound_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1a_transform_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ATransformSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1ATransformSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_selector_count;
        selector = record->structure1a_z_rotation;
        if (selector == 0U) ++receipt.zero_selector_count;
        else ++receipt.nonzero_selector_count;
        if (selector > receipt.highest_selector) receipt.highest_selector = selector;
        if (seen[selector]) ++receipt.duplicate_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_selector_count == receipt.structure1f_bound_entry_count;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_face_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFaceSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFaceSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_face_selector_count;
        selector = record->face;
        if (selector == 0U) ++receipt.zero_face_selector_count;
        else ++receipt.nonzero_face_selector_count;
        if (selector > receipt.highest_face_selector)
            receipt.highest_face_selector = selector;
        if (seen[selector]) ++receipt.duplicate_face_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_face_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_face_selector_count == receipt.structure1f_bound_entry_count;
    receipt.face_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_rotation_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FRotationSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FRotationSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_rotation_selector_count;
        selector = record->rotation;
        if (selector == 0U) ++receipt.zero_rotation_selector_count;
        else ++receipt.nonzero_rotation_selector_count;
        if (selector > receipt.highest_rotation_selector)
            receipt.highest_rotation_selector = selector;
        if (seen[selector]) ++receipt.duplicate_rotation_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_rotation_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_rotation_selector_count == receipt.structure1f_bound_entry_count;
    receipt.rotation_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_face_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->face << 8) | record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else { seen[pair] = 1U; ++receipt.unique_pair_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.structure1f_bound_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_offset_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FOffsetPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FOffsetPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.structure1f_bound_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_offset_pair_count;
        pair = (uint16_t)(((uint16_t)(uint8_t)record->offset_x << 8) |
                          (uint8_t)record->offset_y);
        if (record->offset_x == 0 && record->offset_y == 0)
            ++receipt.zero_offset_pair_count;
        else
            ++receipt.nonzero_offset_pair_count;
        if (receipt.resolved_offset_pair_count == 1) {
            receipt.minimum_offset_x = receipt.maximum_offset_x = record->offset_x;
            receipt.minimum_offset_y = receipt.maximum_offset_y = record->offset_y;
        } else {
            if (record->offset_x < receipt.minimum_offset_x)
                receipt.minimum_offset_x = record->offset_x;
            if (record->offset_x > receipt.maximum_offset_x)
                receipt.maximum_offset_x = record->offset_x;
            if (record->offset_y < receipt.minimum_offset_y)
                receipt.minimum_offset_y = record->offset_y;
            if (record->offset_y > receipt.maximum_offset_y)
                receipt.maximum_offset_y = record->offset_y;
        }
        if (seen[pair]) ++receipt.duplicate_offset_pair_count;
        else { seen[pair] = 1U; ++receipt.unique_offset_pair_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_offset_pair_count == receipt.structure1f_bound_entry_count;
    receipt.offset_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS &&
            record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) continue;
        ++receipt.wall_payload_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_payload_selector_count;
        if (record->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS)
            ++receipt.wall_decoration_selector_count;
        else
            ++receipt.wall_sensor_selector_count;
        selector = record->model_or_aspect;
        if (selector == 0U) ++receipt.zero_payload_selector_count;
        else ++receipt.nonzero_payload_selector_count;
        if (selector > receipt.highest_payload_selector)
            receipt.highest_payload_selector = selector;
        if (seen[selector]) ++receipt.duplicate_payload_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_payload_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_payload_selector_count == receipt.wall_payload_entry_count;
    receipt.payload_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_destination_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        int previous;
        int duplicate = 0;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) continue;
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_destination_count;
        if (record->destination_x == 0U && record->destination_y == 0U &&
            record->destination_orientation == 0U) {
            ++receipt.zero_destination_count;
        } else {
            ++receipt.nonzero_destination_count;
        }
        if (record->destination_x > receipt.highest_destination_x)
            receipt.highest_destination_x = record->destination_x;
        if (record->destination_y > receipt.highest_destination_y)
            receipt.highest_destination_y = record->destination_y;
        if (record->destination_orientation >
            receipt.highest_destination_orientation) {
            receipt.highest_destination_orientation =
                record->destination_orientation;
        }
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior =
                &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS &&
                prior->structure1a_relation_valid &&
                prior->destination_x == record->destination_x &&
                prior->destination_y == record->destination_y &&
                prior->destination_orientation == record->destination_orientation) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_destination_count;
        else ++receipt.unique_destination_count;
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_destination_count == receipt.wall_sensor_entry_count;
    receipt.destination_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) continue;
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_control_selector_count;
        selector = record->type_or_control;
        if (selector == 0U) ++receipt.zero_control_selector_count;
        else ++receipt.nonzero_control_selector_count;
        if (selector > receipt.highest_control_selector)
            receipt.highest_control_selector = selector;
        if (seen[selector]) ++receipt.duplicate_control_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_control_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_control_selector_count == receipt.wall_sensor_entry_count;
    receipt.control_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint32_t tuple;
        int previous;
        int duplicate = 0;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) {
            continue;
        }
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_tuple_count;
        tuple = ((uint32_t)record->type_or_control << 24) |
            ((uint32_t)record->destination_x << 16) |
            ((uint32_t)record->destination_y << 8) |
            record->destination_orientation;
        if (tuple == 0U) ++receipt.zero_tuple_count;
        else ++receipt.nonzero_tuple_count;
        if (tuple > receipt.highest_tuple) receipt.highest_tuple = tuple;
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior =
                &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS &&
                prior->structure1a_relation_valid &&
                prior->type_or_control == record->type_or_control &&
                prior->destination_x == record->destination_x &&
                prior->destination_y == record->destination_y &&
                prior->destination_orientation == record->destination_orientation) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_tuple_count;
        else ++receipt.unique_tuple_count;
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_tuple_count == receipt.wall_sensor_entry_count;
    receipt.tuple_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) {
            continue;
        }
        ++receipt.wall_sensor_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.wall_sensor_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS) {
            continue;
        }
        ++receipt.wall_decoration_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.wall_decoration_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_alcove_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.alcove_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_payload_selector_count;
        selector = record->item_id;
        if (selector == 0U) ++receipt.zero_payload_selector_count;
        else ++receipt.nonzero_payload_selector_count;
        if (selector > receipt.highest_payload_selector)
            receipt.highest_payload_selector = selector;
        if (seen[selector]) ++receipt.duplicate_payload_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_payload_selector_count; }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_payload_selector_count == receipt.alcove_entry_count;
    receipt.payload_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&relation, 0, sizeof(relation));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1a_relation_receipt(
                      level, &relation) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1a_relation_complete = relation.complete;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) continue;
        ++receipt.alcove_entry_count;
        if (!record->structure1a_relation_valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->item_id << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1a_relation_complete &&
        receipt.resolved_pair_count == receipt.alcove_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint8_t selector;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) continue;
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_control_selector_count;
        selector = record->type_or_control;
        if (selector == 0U) ++receipt.zero_control_selector_count;
        else ++receipt.nonzero_control_selector_count;
        if (selector > receipt.highest_control_selector)
            receipt.highest_control_selector = selector;
        if (seen[selector]) ++receipt.duplicate_control_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_control_selector_count; }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_control_selector_count == receipt.floor_sensor_entry_count;
    receipt.control_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_destination_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        int previous;
        int duplicate = 0;
        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) continue;
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_destination_count;
        if (record->destination_x == 0U && record->destination_y == 0U &&
            record->destination_orientation == 0U) ++receipt.zero_destination_count;
        else ++receipt.nonzero_destination_count;
        if (record->destination_x > receipt.highest_destination_x)
            receipt.highest_destination_x = record->destination_x;
        if (record->destination_y > receipt.highest_destination_y)
            receipt.highest_destination_y = record->destination_y;
        if (record->destination_orientation > receipt.highest_destination_orientation)
            receipt.highest_destination_orientation = record->destination_orientation;
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior = &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS &&
                prior->destination_x == record->destination_x &&
                prior->destination_y == record->destination_y &&
                prior->destination_orientation == record->destination_orientation) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_destination_count;
        else ++receipt.unique_destination_count;
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_destination_count == receipt.floor_sensor_entry_count;
    receipt.destination_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_pair_count == receipt.floor_sensor_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        ++receipt.floor_sensor_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->width << 8) | record->height);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_pair_count == receipt.floor_sensor_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        uint8_t selector;
        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue;
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_payload_selector_count;
        selector = record->model_or_aspect;
        if (selector == 0U) ++receipt.zero_payload_selector_count;
        else ++receipt.nonzero_payload_selector_count;
        if (selector > receipt.highest_payload_selector)
            receipt.highest_payload_selector = selector;
        if (seen[selector]) ++receipt.duplicate_payload_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_payload_selector_count; }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_payload_selector_count == receipt.floor_decoration_entry_count;
    receipt.payload_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT8_MAX + 1U];
    int entry;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record = &level->structure1f_entries[entry];
        uint8_t selector;
        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue;
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_rotation_selector_count;
        selector = record->rotation;
        if (selector == 0U) ++receipt.zero_rotation_selector_count;
        else ++receipt.nonzero_rotation_selector_count;
        if (selector > receipt.highest_rotation_selector)
            receipt.highest_rotation_selector = selector;
        if (seen[selector]) ++receipt.duplicate_rotation_selector_count;
        else { seen[selector] = 1U; ++receipt.unique_rotation_selector_count; }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_rotation_selector_count == receipt.floor_decoration_entry_count;
    receipt.rotation_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) {
            continue;
        }
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->model_or_aspect << 8) |
                          record->rotation);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_pair_count == receipt.floor_decoration_entry_count;
    receipt.pair_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        int previous;
        int duplicate = 0;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) {
            continue;
        }
        ++receipt.floor_decoration_entry_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_tuple_count;
        if (record->type_or_control == 0U && record->width == 0U &&
            record->height == 0U) {
            ++receipt.zero_tuple_count;
        } else {
            ++receipt.nonzero_tuple_count;
        }
        if (record->type_or_control > receipt.highest_type_or_control) {
            receipt.highest_type_or_control = record->type_or_control;
        }
        if (record->width > receipt.highest_width) {
            receipt.highest_width = record->width;
        }
        if (record->height > receipt.highest_height) {
            receipt.highest_height = record->height;
        }
        for (previous = 0; previous < entry; ++previous) {
            const Nexus_V1_DgnStructure1FEntry *prior =
                &level->structure1f_entries[previous];
            if (prior->family == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
                prior->type_or_control == record->type_or_control &&
                prior->width == record->width && prior->height == record->height) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) ++receipt.duplicate_tuple_count;
        else ++receipt.unique_tuple_count;
    }
    receipt.complete = receipt.structure1f_spatial_valid &&
        receipt.resolved_tuple_count == receipt.floor_decoration_entry_count;
    receipt.tuple_semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_item_attribute_pair_receipt(const Nexus_V1_Level *l, Nexus_V1_DgnStructure1FItemAttributePairReceipt *o) { Nexus_V1_DgnStructure1FItemAttributePairReceipt r; Nexus_V1_DgnStructure1FSpatialReceipt s; unsigned char seen[UINT16_MAX + 1U]; int i; if (!o) return -1; memset(&r,0,sizeof(r)); memset(&s,0,sizeof(s)); memset(seen,0,sizeof(seen)); if (!l || nexus_v1_level_structure1f_spatial_receipt(l,&s)) {*o=r;return 0;} r.spatial_valid=s.valid; for(i=0;i<l->structure1f_entry_count;i++){const Nexus_V1_DgnStructure1FEntry *e=&l->structure1f_entries[i];uint16_t p;if(e->family!=NEXUS_V1_DGN_STRUCTURE1F_ITEMS)continue;++r.item_count;if(!s.valid)continue;++r.resolved_pair_count;p=((uint16_t)e->attribute1<<8)|e->attribute2;if(seen[p])++r.duplicate_pair_count;else{seen[p]=1;++r.unique_pair_count;}}r.complete=r.spatial_valid&&r.item_count==r.resolved_pair_count;*o=r;return 0;}
int nexus_v1_level_structure1f_item_location_pair_receipt(const Nexus_V1_Level *l, Nexus_V1_DgnStructure1FItemLocationPairReceipt *o) { Nexus_V1_DgnStructure1FItemLocationPairReceipt r; Nexus_V1_DgnStructure1FSpatialReceipt s; unsigned char seen[UINT16_MAX + 1U]; int i; if (!o) return -1; memset(&r,0,sizeof(r)); memset(&s,0,sizeof(s)); memset(seen,0,sizeof(seen)); if (!l || nexus_v1_level_structure1f_spatial_receipt(l,&s)) {*o=r;return 0;} r.spatial_valid=s.valid; for(i=0;i<l->structure1f_entry_count;i++){const Nexus_V1_DgnStructure1FEntry *e=&l->structure1f_entries[i];uint16_t p;if(e->family!=NEXUS_V1_DGN_STRUCTURE1F_ITEMS)continue;++r.item_count;if(!s.valid)continue;++r.resolved_pair_count;p=((uint16_t)e->location<<8)|e->item_id;if(seen[p])++r.duplicate_pair_count;else{seen[p]=1;++r.unique_pair_count;}}r.complete=r.spatial_valid&&r.item_count==r.resolved_pair_count;*o=r;return 0;}

int nexus_v1_level_structure1f_item_coordinate_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt receipt;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    unsigned char seen[UINT16_MAX + 1U];
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    memset(&spatial, 0, sizeof(spatial));
    memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &spatial) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.spatial_valid = spatial.valid;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        uint16_t pair;

        if (record->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
        ++receipt.item_count;
        if (!spatial.valid) continue;
        ++receipt.resolved_pair_count;
        pair = (uint16_t)(((uint16_t)record->x << 8) | record->y);
        if (pair == 0U) ++receipt.zero_pair_count;
        else ++receipt.nonzero_pair_count;
        if (pair > receipt.highest_pair) receipt.highest_pair = pair;
        if (seen[pair]) ++receipt.duplicate_pair_count;
        else {
            seen[pair] = 1U;
            ++receipt.unique_pair_count;
        }
    }
    receipt.complete = receipt.spatial_valid &&
        receipt.resolved_pair_count == receipt.item_count;
    receipt.semantics_proven = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_structure1f_floor_decoration_offset_pair_receipt(const Nexus_V1_Level *level, Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt *out)
{
    Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt r; Nexus_V1_DgnStructure1FSpatialReceipt s; unsigned char seen[UINT16_MAX + 1U]; int i;
    if (!out) return -1; memset(&r, 0, sizeof(r)); memset(&s, 0, sizeof(s)); memset(seen, 0, sizeof(seen));
    if (!level || nexus_v1_level_structure1f_spatial_receipt(level, &s) != 0) { *out = r; return 0; }
    r.structure1f_spatial_valid = s.valid;
    for (i = 0; i < level->structure1f_entry_count; ++i) { const Nexus_V1_DgnStructure1FEntry *e = &level->structure1f_entries[i]; uint16_t p;
        if (e->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) continue; ++r.entry_count; if (!s.valid) continue; ++r.resolved_pair_count;
        p = (uint16_t)(((uint16_t)(uint8_t)e->offset_x << 8) | (uint8_t)e->offset_y); if (p) ++r.nonzero_pair_count; else ++r.zero_pair_count; if (seen[p]) ++r.duplicate_pair_count; else { seen[p] = 1; ++r.unique_pair_count; }
    }
    r.complete = r.structure1f_spatial_valid && r.entry_count == r.resolved_pair_count; *out = r; return 0;
}

int nexus_v1_level_structure3_payload_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3PayloadReceipt *out_receipt)
{
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (level) *out_receipt = level->structure3_payload;
    return 0;
}

int nexus_v1_level_structure3_ordinal_correlation_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt receipt;
    Nexus_V1_DgnStructure3ModelReferenceReceipt model_references;
    int entry;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.highest_model_index = -1;
    if (!level) {
        *out_receipt = receipt;
        return 0;
    }
    (void)nexus_v1_level_structure3_model_reference_receipt(
        level, &model_references);
    receipt.structure1a_relation_complete = model_references.complete;
    receipt.structure3_payload_valid = level->structure3_payload.valid;
    receipt.structure3_block_count = level->structure3_payload.block_count;
    receipt.structure3_nonzero_byte_run_count =
        level->structure3_payload.nonzero_byte_run_count;
    receipt.structure3_nonzero_block_run_count =
        level->structure3_payload.nonzero_block_run_count;
    for (entry = 0; entry < level->structure1f_entry_count; ++entry) {
        const Nexus_V1_DgnStructure1FEntry *record =
            &level->structure1f_entries[entry];
        int model_index;
        if (record->family < NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
            !record->structure1a_relation_valid) continue;
        model_index = (int)record->structure1a_structure3_model_index;
        ++receipt.resolved_model_reference_count;
        if (model_index > receipt.highest_model_index) {
            receipt.highest_model_index = model_index;
        }
        if (model_index > receipt.structure3_block_count) {
            ++receipt.model_index_exceeds_block_count;
        }
        if (model_index > receipt.structure3_nonzero_byte_run_count) {
            ++receipt.model_index_exceeds_nonzero_byte_run_count;
        }
        if (model_index > receipt.structure3_nonzero_block_run_count) {
            ++receipt.model_index_exceeds_nonzero_block_run_count;
        }
        if (model_index >= receipt.structure3_block_count) {
            receipt.zero_based_block_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 || model_index > receipt.structure3_block_count) {
            receipt.one_based_block_ordinal_mapping_disproven = 1;
        }
        if (model_index >= receipt.structure3_nonzero_byte_run_count) {
            receipt.zero_based_byte_run_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 ||
            model_index > receipt.structure3_nonzero_byte_run_count) {
            receipt.one_based_byte_run_ordinal_mapping_disproven = 1;
        }
        if (model_index >= receipt.structure3_nonzero_block_run_count) {
            receipt.zero_based_run_ordinal_mapping_disproven = 1;
        }
        if (model_index == 0 ||
            model_index > receipt.structure3_nonzero_block_run_count) {
            receipt.one_based_run_ordinal_mapping_disproven = 1;
        }
    }
    receipt.direct_block_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_block_ordinal_mapping_disproven &&
        receipt.one_based_block_ordinal_mapping_disproven;
    receipt.direct_byte_run_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_byte_run_ordinal_mapping_disproven &&
        receipt.one_based_byte_run_ordinal_mapping_disproven;
    receipt.direct_run_ordinal_mapping_disproven =
        receipt.resolved_model_reference_count > 0 &&
        receipt.zero_based_run_ordinal_mapping_disproven &&
        receipt.one_based_run_ordinal_mapping_disproven;
    receipt.valid = receipt.structure1a_relation_complete &&
        receipt.structure3_payload_valid;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_level_dgn_structure1_host_provenance_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1HostProvenanceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1HostProvenanceReceipt receipt;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_MISSING;
    if (!level || level->width <= 0 || level->height <= 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.structure1f_declared = level->geometry_info.structure1f_declared;
    receipt.structure1f_valid = level->geometry_info.structure1f_valid;
    receipt.structure1f_typed_entry_count = level->structure1f_entry_count;
    (void)nexus_v1_level_structure1f_spatial_receipt(
        level, &receipt.structure1f_spatial);
    (void)nexus_v1_level_structure1a_boundary_receipt(
        level, &receipt.structure1a_boundary);
    (void)nexus_v1_level_structure1a_relation_receipt(
        level, &receipt.structure1a_relation);

    if (!receipt.structure1f_declared) {
        receipt.status = NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_ABSENT;
        receipt.can_prepare_runtime_dgn = 1;
    } else if (!receipt.structure1f_valid ||
               !receipt.structure1f_spatial.valid ||
               !receipt.structure1a_boundary.valid) {
        receipt.status =
            NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1F_LAYOUT;
    } else if (receipt.structure1f_spatial.structure1a_bound_entry_count > 0 ||
               receipt.structure1a_boundary.entry_count > 0) {
        if (receipt.structure1a_relation.complete) {
            receipt.status =
                NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_RESOLVED_STRUCTURE1A;
            receipt.can_prepare_runtime_dgn = 1;
        } else {
            receipt.status =
                NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION;
        }
    } else {
        receipt.status = NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_DIRECT;
        receipt.can_prepare_runtime_dgn = 1;
    }
    receipt.blocks_real_dgn_mesh_render =
        receipt.can_prepare_runtime_dgn ? 0 : 1;
    *out_receipt = receipt;
    return 0;
}

const char *nexus_v1_dgn_structure1_host_provenance_status_name(
    Nexus_V1_DgnStructure1HostProvenanceStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_ABSENT:
        return "ready-no-structure1f";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_DIRECT:
        return "ready-direct-structure1f";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_RESOLVED_STRUCTURE1A:
        return "ready-resolved-structure1a";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1F_LAYOUT:
        return "blocked-structure1f-layout";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION:
        return "blocked-structure1a-relation";
    case NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_MISSING:
    default:
        return "missing";
    }
}

int nexus_v1_level_move_allowed(const Nexus_V1_Level *level,
                                int from_x, int from_y,
                                int to_x, int to_y) {
    Nexus_V1_DgnCellGeometry cell;

    (void)from_x;
    (void)from_y;

    if (nexus_v1_level_get_cell_geometry(level, to_x, to_y, &cell) != 0 ||
        cell.square_type == 0 || cell.collision_ref == 0x0fffU)
        return 0;
    return 1;
}

int nexus_v1_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt) {
    const Nexus_V1_DgnGeometryInfo *info;

    if (!out_receipt) {
        return -1;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    out_receipt->fallback_visuals_permitted = 0;

    if (!level || level->width <= 0 || level->height <= 0) {
        return 0;
    }

    info = &level->geometry_info;
    out_receipt->width = level->width;
    out_receipt->height = level->height;
    out_receipt->dmweb_container = info->dmweb_container;
    out_receipt->mesh_ready = info->mesh_ready;
    out_receipt->geometry_offset = info->geometry_offset;
    out_receipt->geometry_size = info->geometry_size;
    out_receipt->collision_ref_count = info->collision_ref_count;
    out_receipt->collision_ref_unique_count =
        info->collision_ref_unique_count;
    out_receipt->max_collision_ref = info->max_collision_ref;
    out_receipt->post_grid_0x30_ref_count =
        info->post_grid_0x30_ref_count;
    out_receipt->post_grid_0x30_ref_unique_count =
        info->post_grid_0x30_ref_unique_count;
    out_receipt->max_post_grid_0x30_ref =
        info->max_post_grid_0x30_ref;
    out_receipt->post_grid_0x30_references_valid =
        info->post_grid_0x30_references_valid;
    out_receipt->post_grid_0x30_invalid_ref_count =
        info->post_grid_0x30_invalid_ref_count;
    out_receipt->first_invalid_post_grid_0x30_ref =
        info->first_invalid_post_grid_0x30_ref;
    out_receipt->post_grid_0x30_ref_value_count =
        info->post_grid_0x30_ref_value_count;
    out_receipt->post_grid_0x24_zero_span_valid =
        info->post_grid_0x24_zero_span_valid;
    out_receipt->post_grid_0x30_record_table_valid =
        info->post_grid_0x30_record_table_valid;
    out_receipt->post_grid_0x30_record_count =
        info->post_grid_0x30_record_count;
    out_receipt->post_grid_0x30_typed_prefix_record_count =
        info->post_grid_0x30_typed_prefix_record_count;
    out_receipt->post_grid_0x30_opaque_tail_record_count =
        info->post_grid_0x30_opaque_tail_record_count;
    out_receipt->post_grid_0x30_row_ordinal_prefix_valid =
        info->post_grid_0x30_row_ordinal_prefix_valid;
    out_receipt->post_grid_0x30_row_ordinal_flagged_prefix_record_count =
        info->post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    out_receipt->post_grid_0x30_first_row_ordinal_flagged_prefix_record =
        info->post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    out_receipt->post_grid_0x30_last_row_ordinal_flagged_prefix_record =
        info->post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    out_receipt->structure1f_declared = info->structure1f_declared;
    out_receipt->structure1f_valid = info->structure1f_valid;
    out_receipt->structure1f_total_entry_count =
        info->structure1f_total_entry_count;
    memcpy(out_receipt->structure1f_family_count,
           info->structure1f_family_count,
           sizeof(out_receipt->structure1f_family_count));
    out_receipt->structure1f_typed_entry_count = level->structure1f_entry_count;
    (void)nexus_v1_level_structure1f_spatial_receipt(
        level, &out_receipt->structure1f_spatial);
    (void)nexus_v1_level_structure1a_boundary_receipt(
        level, &out_receipt->structure1a_boundary);
    (void)nexus_v1_level_structure1a_relation_receipt(
        level, &out_receipt->structure1a_relation);
    (void)nexus_v1_level_structure3_model_reference_receipt(
        level, &out_receipt->structure3_model_references);
    (void)nexus_v1_level_structure1a_transform_selector_receipt(
        level, &out_receipt->structure1a_transform_selectors);
    (void)nexus_v1_level_structure1f_face_selector_receipt(
        level, &out_receipt->structure1f_face_selectors);
    (void)nexus_v1_level_structure1f_rotation_selector_receipt(
        level, &out_receipt->structure1f_rotation_selectors);
    (void)nexus_v1_level_structure1f_face_rotation_pair_receipt(
        level, &out_receipt->structure1f_face_rotation_pairs);
    (void)nexus_v1_level_structure1f_offset_pair_receipt(
        level, &out_receipt->structure1f_offset_pairs);
    (void)nexus_v1_level_structure1f_wall_payload_selector_receipt(
        level, &out_receipt->structure1f_wall_payload_selectors);
    (void)nexus_v1_level_structure1f_wall_sensor_destination_receipt(
        level, &out_receipt->structure1f_wall_sensor_destinations);
    (void)nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
        level, &out_receipt->structure1f_wall_sensor_control_selectors);
    (void)nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
        level, &out_receipt->structure1f_wall_sensor_control_destination_tuples);
    (void)nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_wall_sensor_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_wall_decoration_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_alcove_payload_selector_receipt(
        level, &out_receipt->structure1f_alcove_payload_selectors);
    (void)nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
        level, &out_receipt->structure1f_alcove_payload_rotation_pairs);
    (void)nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
        level, &out_receipt->structure1f_floor_sensor_control_selectors);
    (void)nexus_v1_level_structure1f_floor_sensor_destination_receipt(
        level, &out_receipt->structure1f_floor_sensor_destinations);
    (void)nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_floor_sensor_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
        level, &out_receipt->structure1f_floor_sensor_extent_pairs);
    (void)nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
        level, &out_receipt->structure1f_floor_decoration_payload_selectors);
    (void)nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
        level, &out_receipt->structure1f_floor_decoration_rotation_selectors);
    (void)nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
        level, &out_receipt->structure1f_floor_decoration_model_rotation_pairs);
    (void)nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
        level, &out_receipt->structure1f_floor_decoration_control_extents);
    (void)nexus_v1_level_structure1f_item_attribute_pair_receipt(
        level, &out_receipt->structure1f_item_attribute_pairs);
    (void)nexus_v1_level_structure1f_item_location_pair_receipt(
        level, &out_receipt->structure1f_item_location_pairs);
    (void)nexus_v1_level_structure1f_item_coordinate_pair_receipt(
        level, &out_receipt->structure1f_item_coordinate_pairs);
    (void)nexus_v1_level_structure1f_floor_decoration_offset_pair_receipt(
        level, &out_receipt->structure1f_floor_decoration_offset_pairs);
    (void)nexus_v1_level_structure3_payload_receipt(
        level, &out_receipt->structure3_payload);
    out_receipt->structure1g_present = info->structure1g_present;
    out_receipt->structure1g_valid = info->structure1g_valid;
    out_receipt->structure1g_animated_texture_count =
        info->structure1g_animated_texture_count;
    out_receipt->structure1g_sequence_count = info->structure1g_sequence_count;
    out_receipt->structure1g_floor_animation_cell_count =
        level->structure1g_floor_animation_cell_count;
    out_receipt->structure1g_floor_animation_bound_count =
        level->structure1g_floor_animation_bound_count;
    for (int entry = 0; entry < level->structure1g_entry_count; ++entry) {
        out_receipt->structure1g_image_instruction_count +=
            level->structure1g_entries[entry].image_instruction_count;
        out_receipt->structure1g_goto_instruction_count +=
            level->structure1g_entries[entry].goto_instruction_count;
        out_receipt->structure1g_structure2_image_instruction_bound_count +=
            level->structure1g_entries[entry]
                .structure2_image_instruction_bound_count;
        out_receipt->structure1g_structure2_image_instruction_unbound_count +=
            level->structure1g_entries[entry]
                .structure2_image_instruction_unbound_count;
    }
    out_receipt->structure1g_structure2_bindings_complete =
        level->structure1g_structure2_bindings_complete;
    out_receipt->structure2_descriptor_offset_envelope_valid =
        level->structure2_payload.descriptor_offset_envelope_valid;

    if (!info->dmweb_container) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK;
    } else if (info->structure1g_present && !info->structure1g_valid) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS;
    } else if (info->structure1g_present &&
               level->structure2_texture_table_valid &&
               level->structure2_payload.valid &&
               !level->structure2_payload.descriptor_offset_envelope_valid) {
        /* A descriptor ID alone is not an admissible original source if one
         * of its raw targets crosses the only proven Structure2 envelope. */
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE;
    } else if (info->structure1g_present &&
               !level->structure1g_structure2_bindings_complete) {
        /* A syntactically bounded Structure1G program still cannot be
         * promoted when any real image instruction misses Structure2. */
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
    } else if (info->structure1f_declared && !info->structure1f_valid) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT;
    } else if (info->structure1f_valid &&
               out_receipt->structure1f_spatial.valid &&
               out_receipt->structure1f_spatial.structure1a_bound_entry_count > 0) {
        /* A resolved Structure1A owner names only a Structure3 model index.
         * Structure3's mesh/face payload grammar is still unparsed, so do
         * not convert this receipt into a draw or omit it from the scene. */
        if (!out_receipt->structure3_model_references.complete) {
            out_receipt->status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS;
        } else if (!out_receipt->structure3_payload.valid) {
            out_receipt->status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_MESH;
        } else {
            out_receipt->status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS;
        }
    } else if (info->mesh_ready) {
        out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH;
        out_receipt->can_render_dgn_mesh = 1;
    } else if (!info->post_grid_0x30_record_table_valid) {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY;
    } else if (!info->post_grid_0x30_references_valid) {
        /* The retail corpus proves only the ordinal-typed prefix. A packed
         * Structure1B reference into the opaque tail cannot be presented as
         * a generic descriptor shortage or substituted with fallback art. */
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE;
    } else {
        out_receipt->status =
            NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET;
    }

    out_receipt->blocks_real_dgn_mesh_render =
        out_receipt->can_render_dgn_mesh ? 0 : 1;
    return 0;
}

const char *nexus_v1_dgn_renderer_handoff_status_name(
    Nexus_V1_DgnRendererHandoffStatus status) {
    switch (status) {
    case NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING: return "missing";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH: return "ready-mesh";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY:
        return "blocked-no-geometry";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET:
        return "blocked-descriptor-budget";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK:
        return "blocked-legacy-fallback";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS:
        return "blocked-structure-semantics";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE:
        return "blocked-structure2-source";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS:
        return "blocked-structure1f-semantics";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE:
        return "blocked-structure1f-reference";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE:
        return "blocked-structure2-envelope";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT:
        return "blocked-structure1f-layout";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_MESH:
        return "blocked-structure3-mesh";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS:
        return "blocked-structure3-face-semantics";
    case NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1B_SELECTOR:
        return "blocked-structure1b-selector";
    default: return "unknown";
    }
}

static void nexus_v1_dgn_plan_project_quad(Nexus_V1_DgnRenderCommand *command);

static int nexus_v1_dgn_plan_push(
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *receipt,
    Nexus_V1_DgnRenderCommand command) {
    if (!receipt) {
        return -1;
    }
    if (!commands || receipt->command_count >= max_commands) {
        receipt->blocks_real_dgn_mesh_render = 1;
        receipt->plan_ready = 0;
        return -1;
    }
    nexus_v1_dgn_plan_project_quad(&command);
    commands[receipt->command_count++] = command;
    receipt->source_cell_count++;
    if (command.kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
        receipt->floor_count++;
        if (command.material_source_kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
            receipt->floor_material_command_count++;
        }
    } else if (command.kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
        receipt->ceiling_count++;
        if (command.material_source_kind ==
            NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
            receipt->ceiling_material_command_count++;
        }
    } else {
        receipt->wall_count++;
        if (command.material_source_kind != NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
            command.material_source_kind != NEXUS_V1_DGN_RENDER_COMMAND_CEILING) {
            receipt->wall_material_command_count++;
        }
    }
    if (command.post_grid_0x30_ref != 0U &&
        command.post_grid_0x30_ref != 0x0FFFU) {
        receipt->post_grid_0x30_reference_command_count++;
        if (command.post_grid_0x30_row_prefix_valid)
            receipt->post_grid_0x30_valid_reference_command_count++;
        if (receipt->first_post_grid_0x30_ref == 0) {
            receipt->first_post_grid_0x30_ref = command.post_grid_0x30_ref;
        }
        if ((int)command.post_grid_0x30_ref >
            receipt->max_post_grid_0x30_ref) {
            receipt->max_post_grid_0x30_ref = command.post_grid_0x30_ref;
        }
    }
    return 0;
}

static Nexus_V1_DgnRenderCommand nexus_v1_dgn_plan_command(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRenderCommandKind kind,
    int x,
    int y,
    int depth,
    int lateral,
    int wall_dir) {
    Nexus_V1_DgnRenderCommand command;
    memset(&command, 0, sizeof(command));
    command.animated_texture_structure1g_entry_index = -1;
    command.kind = kind;
    command.x = x;
    command.y = y;
    command.depth = depth;
    command.lateral = lateral;
    Nexus_V1_DgnCellGeometry cell;
    (void)nexus_v1_level_get_cell_geometry(level, x, y, &cell);
    command.square_type = cell.square_type;
    command.wall_dir = wall_dir & 3;
    command.collision_ref = cell.collision_ref;
    command.post_grid_0x30_ref = cell.post_grid_0x30_ref;
    command.collision_sector = cell.collision_sector;
    command.post_grid_0x30_row_prefix_valid =
        cell.post_grid_0x30_row_prefix_valid;
    command.floor_rotation = cell.floor_rotation;
    command.floor_slope = cell.floor_slope;
    memcpy(command.floor_height, cell.floor_height, sizeof(command.floor_height));
    memcpy(command.ceiling_height, cell.ceiling_height,
           sizeof(command.ceiling_height));
    command.material_id = (uint8_t)nexus_v1_level_get_material_ref(
        level, x, y, kind, wall_dir);
    if (kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR &&
        level->floor_animation_ids[y][x] != 0xffU) {
        int entry;
        command.animated_texture_declared = 1;
        command.animated_texture_id = level->floor_animation_ids[y][x];
        for (entry = 0; entry < level->structure1g_entry_count; ++entry) {
            if (level->structure1g_entries[entry].animation_id ==
                command.animated_texture_id) {
                command.animated_texture_structure1g_entry_index = entry;
                command.animated_texture_structure1g_sequence_word_offset =
                    level->structure1g_entries[entry].sequence_word_offset;
                command.animated_texture_first_image_index =
                    level->structure1g_entries[entry].first_image_index;
                command.animated_texture_structure2_image_id =
                    level->structure1g_entries[entry].first_structure2_image_id;
                command.animated_texture_structure2_image_valid =
                    level->structure1g_entries[entry].first_structure2_image_valid;
                command.animated_texture_host_route =
                    NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR;
                break;
            }
        }
    }
    switch (kind) {
    case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
        command.material_source_kind = NEXUS_V1_DGN_RENDER_COMMAND_FLOOR;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(32 - depth);
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
        command.material_source_kind = NEXUS_V1_DGN_RENDER_COMMAND_CEILING;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(16 - depth);
        break;
    default:
        command.material_source_kind = kind;
        command.palette_index = command.material_id;
        command.draw_order = (uint8_t)(48 - depth);
        break;
    }
    return command;
}

static void nexus_v1_dgn_plan_bind_direct_structure1f(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRenderCommand *commands,
    Nexus_V1_DgnRenderPlanReceipt *receipt)
{
    int entry_index;

    if (!level || !commands || !receipt ||
        !receipt->structure1f_spatial.valid) {
        return;
    }
    for (entry_index = 0; entry_index < level->structure1f_entry_count;
         ++entry_index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[entry_index];
        int command_index;
        int visible = 0;
        uint8_t family_mask;

        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        switch (entry->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
            family_mask = NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            family_mask =
                NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_FLOOR_DECORATION;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            family_mask =
                NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_FLOOR_SENSOR;
            break;
        default:
            continue;
        }
        for (command_index = 0; command_index < receipt->command_count;
             ++command_index) {
            if (commands[command_index].x == entry->x &&
                commands[command_index].y == entry->y) {
                visible = 1;
                /* DMWeb's Structure1F families here carry a direct floor
                 * cell coordinate. Retain their provenance on the exact
                 * FLOOR command only; applying it to co-located walls or a
                 * ceiling would invent a material/mesh relation. The visible
                 * record remains a no-draw gate below even if a plan has no
                 * floor command for its source cell. */
                if (commands[command_index].kind !=
                    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
                    continue;
                }
                const int item_already_attached =
                    (commands[command_index].structure1f_direct_family_mask &
                     NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM) != 0;
                if (commands[command_index].structure1f_direct_entry_count ==
                    0) {
                    ++receipt->structure1f_plan_direct_command_count;
                    ++receipt->structure1f_plan_direct_floor_command_count;
                }
                ++commands[command_index].structure1f_direct_entry_count;
                commands[command_index].structure1f_direct_family_mask |=
                    family_mask;
                ++receipt->structure1f_plan_direct_command_entry_count;
                ++receipt->structure1f_plan_direct_floor_command_entry_count;
                if (entry->family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS) {
                    ++receipt->structure1f_plan_item_floor_command_entry_count;
                    if (!item_already_attached) {
                        ++receipt->structure1f_plan_item_floor_command_count;
                    }
                }
            }
        }
        if (!visible) {
            continue;
        }
        ++receipt->structure1f_plan_direct_entry_count;
        switch (entry->family) {
        case NEXUS_V1_DGN_STRUCTURE1F_ITEMS:
            ++receipt->structure1f_plan_item_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS:
            ++receipt->structure1f_plan_floor_decoration_entry_count;
            break;
        case NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS:
            ++receipt->structure1f_plan_floor_sensor_entry_count;
            break;
        default:
            break;
        }
    }
}

static int16_t nexus_v1_dgn_view_clamp(int value) {
    if (value < -NEXUS_V1_DGN_VIEWPORT_UNITS) return -NEXUS_V1_DGN_VIEWPORT_UNITS;
    if (value > NEXUS_V1_DGN_VIEWPORT_UNITS * 2) return NEXUS_V1_DGN_VIEWPORT_UNITS * 2;
    return (int16_t)value;
}

static int nexus_v1_dgn_view_x(int lateral_half, int z_half) {
    return (NEXUS_V1_DGN_VIEWPORT_UNITS / 2) +
        (lateral_half * NEXUS_V1_DGN_VIEWPORT_UNITS / 2) / z_half;
}

static int nexus_v1_dgn_view_floor_y(int z_half) {
    return 400 + (768 / z_half);
}

/* Structure1B byte 3 stores signed 1/32 world-unit floor heights. Keep the
 * copied host plan on the same vertical projection as the material viewport:
 * a 32-unit ceiling over a zero-height floor reaches the old ceiling baseline. */
static int nexus_v1_dgn_view_height_y(int z_half, int8_t height) {
    return nexus_v1_dgn_view_floor_y(z_half) -
        ((int)height * 1280) / (32 * z_half);
}

static void nexus_v1_dgn_plan_set_quad(Nexus_V1_DgnRenderCommand *command,
                                       int x0, int y0, int x1, int y1,
                                       int x2, int y2, int x3, int y3) {
    command->quad_x[0] = nexus_v1_dgn_view_clamp(x0);
    command->quad_y[0] = nexus_v1_dgn_view_clamp(y0);
    command->quad_x[1] = nexus_v1_dgn_view_clamp(x1);
    command->quad_y[1] = nexus_v1_dgn_view_clamp(y1);
    command->quad_x[2] = nexus_v1_dgn_view_clamp(x2);
    command->quad_y[2] = nexus_v1_dgn_view_clamp(y2);
    command->quad_x[3] = nexus_v1_dgn_view_clamp(x3);
    command->quad_y[3] = nexus_v1_dgn_view_clamp(y3);
}

static void nexus_v1_dgn_plan_project_quad(Nexus_V1_DgnRenderCommand *command) {
    int near_z = command->depth * 2 + 1;
    int far_z = near_z + 2;
    int left_half = command->lateral * 2 - 1;
    int right_half = left_half + 2;
    int near_left = nexus_v1_dgn_view_x(left_half, near_z);
    int near_right = nexus_v1_dgn_view_x(right_half, near_z);
    int far_left = nexus_v1_dgn_view_x(left_half, far_z);
    int far_right = nexus_v1_dgn_view_x(right_half, far_z);

    switch (command->kind) {
    case NEXUS_V1_DGN_RENDER_COMMAND_FLOOR:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->floor_height[0]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->floor_height[1]),
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->floor_height[2]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->floor_height[3]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_CEILING:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->ceiling_height[0]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->ceiling_height[3]),
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->ceiling_height[2]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->ceiling_height[1]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->floor_height[0]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->floor_height[1]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->ceiling_height[1]),
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->ceiling_height[0]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT:
        nexus_v1_dgn_plan_set_quad(command,
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->floor_height[0]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->floor_height[3]),
            far_left, nexus_v1_dgn_view_height_y(far_z,
                                                  command->ceiling_height[3]),
            near_left, nexus_v1_dgn_view_height_y(near_z,
                                                   command->ceiling_height[0]));
        break;
    case NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT:
        nexus_v1_dgn_plan_set_quad(command,
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->floor_height[2]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->floor_height[1]),
            near_right, nexus_v1_dgn_view_height_y(near_z,
                                                    command->ceiling_height[1]),
            far_right, nexus_v1_dgn_view_height_y(far_z,
                                                   command->ceiling_height[2]));
        break;
    default:
        break;
    }
}

int nexus_v1_level_build_dgn_view_render_plan(
    const Nexus_V1_Level *level,
    int party_x,
    int party_y,
    int party_dir,
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *out_receipt) {
    static const int dir_dx[4] = {0, 1, 0, -1};
    static const int dir_dy[4] = {-1, 0, 1, 0};
    static const int left_dx[4] = {-1, 0, 1, 0};
    static const int left_dy[4] = {0, -1, 0, 1};
    Nexus_V1_DgnRendererHandoffReceipt handoff;
    Nexus_V1_DgnRenderPlanReceipt receipt;
    int pdir;
    int depth;

    if (!out_receipt) {
        return -1;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt.first_blocking_x = -1;
    receipt.first_blocking_y = -1;
    receipt.first_blocking_depth = -1;
    receipt.fallback_visuals_permitted = 0;
    if (commands && max_commands > 0) {
        memset(commands, 0,
               (size_t)max_commands * sizeof(Nexus_V1_DgnRenderCommand));
    }

    if (!level || max_commands < NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }
    if (nexus_v1_level_dgn_renderer_handoff_receipt(level, &handoff) != 0) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.status = handoff.status;
    receipt.post_grid_0x30_row_ordinal_flagged_prefix_record_count =
        handoff.post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    receipt.post_grid_0x30_first_row_ordinal_flagged_prefix_record =
        handoff.post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    receipt.post_grid_0x30_last_row_ordinal_flagged_prefix_record =
        handoff.post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    receipt.structure1f_declared = handoff.structure1f_declared;
    receipt.structure1f_valid = handoff.structure1f_valid;
    receipt.structure1f_total_entry_count = handoff.structure1f_total_entry_count;
    memcpy(receipt.structure1f_family_count, handoff.structure1f_family_count,
           sizeof(receipt.structure1f_family_count));
    receipt.structure1f_typed_entry_count = handoff.structure1f_typed_entry_count;
    receipt.structure1f_spatial = handoff.structure1f_spatial;
    receipt.structure1a_boundary = handoff.structure1a_boundary;
    receipt.structure1a_relation = handoff.structure1a_relation;
    receipt.structure3_model_references = handoff.structure3_model_references;
    receipt.structure1a_transform_selectors = handoff.structure1a_transform_selectors;
    receipt.structure1f_face_selectors = handoff.structure1f_face_selectors;
    receipt.structure1f_rotation_selectors = handoff.structure1f_rotation_selectors;
    receipt.structure1f_face_rotation_pairs = handoff.structure1f_face_rotation_pairs;
    receipt.structure1f_offset_pairs = handoff.structure1f_offset_pairs;
    receipt.structure1f_wall_payload_selectors =
        handoff.structure1f_wall_payload_selectors;
    receipt.structure1f_wall_sensor_destinations =
        handoff.structure1f_wall_sensor_destinations;
    receipt.structure1f_wall_sensor_control_selectors =
        handoff.structure1f_wall_sensor_control_selectors;
    receipt.structure1f_wall_sensor_control_destination_tuples =
        handoff.structure1f_wall_sensor_control_destination_tuples;
    receipt.structure1f_wall_sensor_model_rotation_pairs =
        handoff.structure1f_wall_sensor_model_rotation_pairs;
    receipt.structure1f_wall_decoration_model_rotation_pairs =
        handoff.structure1f_wall_decoration_model_rotation_pairs;
    receipt.structure1f_alcove_payload_selectors =
        handoff.structure1f_alcove_payload_selectors;
    receipt.structure1f_alcove_payload_rotation_pairs =
        handoff.structure1f_alcove_payload_rotation_pairs;
    receipt.structure1f_floor_sensor_control_selectors =
        handoff.structure1f_floor_sensor_control_selectors;
    receipt.structure1f_floor_sensor_destinations =
        handoff.structure1f_floor_sensor_destinations;
    receipt.structure1f_floor_sensor_model_rotation_pairs =
        handoff.structure1f_floor_sensor_model_rotation_pairs;
    receipt.structure1f_floor_sensor_extent_pairs =
        handoff.structure1f_floor_sensor_extent_pairs;
    receipt.structure1f_floor_decoration_payload_selectors =
        handoff.structure1f_floor_decoration_payload_selectors;
    receipt.structure1f_floor_decoration_rotation_selectors =
        handoff.structure1f_floor_decoration_rotation_selectors;
    receipt.structure1f_floor_decoration_model_rotation_pairs =
        handoff.structure1f_floor_decoration_model_rotation_pairs;
    receipt.structure1f_floor_decoration_control_extents =
        handoff.structure1f_floor_decoration_control_extents;
    receipt.structure1f_item_attribute_pairs = handoff.structure1f_item_attribute_pairs;
    receipt.structure1f_item_location_pairs = handoff.structure1f_item_location_pairs;
    receipt.structure1f_item_coordinate_pairs =
        handoff.structure1f_item_coordinate_pairs;
    receipt.structure1f_floor_decoration_offset_pairs =
        handoff.structure1f_floor_decoration_offset_pairs;
    receipt.structure3_payload = handoff.structure3_payload;
    receipt.structure1g_present = handoff.structure1g_present;
    receipt.structure1g_valid = handoff.structure1g_valid;
    receipt.structure1g_animated_texture_count =
        handoff.structure1g_animated_texture_count;
    receipt.structure1g_sequence_count = handoff.structure1g_sequence_count;
    receipt.structure1g_floor_animation_cell_count =
        handoff.structure1g_floor_animation_cell_count;
    receipt.structure1g_floor_animation_bound_count =
        handoff.structure1g_floor_animation_bound_count;
    receipt.structure1g_image_instruction_count =
        handoff.structure1g_image_instruction_count;
    receipt.structure1g_goto_instruction_count =
        handoff.structure1g_goto_instruction_count;
    receipt.structure1g_structure2_image_instruction_bound_count =
        handoff.structure1g_structure2_image_instruction_bound_count;
    receipt.structure1g_structure2_image_instruction_unbound_count =
        handoff.structure1g_structure2_image_instruction_unbound_count;
    receipt.structure1g_structure2_bindings_complete =
        handoff.structure1g_structure2_bindings_complete;
    if (handoff.status != NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH ||
        !handoff.can_render_dgn_mesh) {
        receipt.blocks_real_dgn_mesh_render = 1;
        *out_receipt = receipt;
        return 0;
    }

    pdir = party_dir & 3;
    for (depth = 0; depth < NEXUS_V1_DGN_VIEW_DISTANCE; ++depth) {
        int cx = party_x + dir_dx[pdir] * depth;
        int cy = party_y + dir_dy[pdir] * depth;
        int lx = cx + left_dx[pdir];
        int ly = cy + left_dy[pdir];
        int rx = cx - left_dx[pdir];
        int ry = cy - left_dy[pdir];
        int sq = nexus_v1_level_get_square(level, cx, cy);
        int sq_l = nexus_v1_level_get_square(level, lx, ly);
        int sq_r = nexus_v1_level_get_square(level, rx, ry);

        if (sq != 0) {
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        cx, cy, depth, 0, pdir)) != 0) {
                break;
            }
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        cx, cy, depth, 0, pdir)) != 0) {
                break;
            }
            if (sq_l != 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        lx, ly, depth, -1, pdir));
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        lx, ly, depth, -1, pdir));
            }
            if (sq_r != 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_FLOOR,
                        rx, ry, depth, 1, pdir));
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_CEILING,
                        rx, ry, depth, 1, pdir));
            }
            if (sq_l == 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT,
                        cx, cy, depth, -1, (pdir + 3) & 3));
            }
            if (sq_r == 0) {
                (void)nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT,
                        cx, cy, depth, 1, (pdir + 1) & 3));
            }
        } else {
            if (nexus_v1_dgn_plan_push(
                    commands,
                    max_commands,
                    &receipt,
                    nexus_v1_dgn_plan_command(
                        level,
                        NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT,
                        cx, cy, depth, 0, (pdir + 2) & 3)) == 0) {
                receipt.first_blocking_x = cx;
                receipt.first_blocking_y = cy;
                receipt.first_blocking_depth = depth;
            }
            break;
        }
        if (receipt.blocks_real_dgn_mesh_render) {
            break;
        }
    }

    if (!receipt.blocks_real_dgn_mesh_render) {
        int command_index;
        nexus_v1_dgn_plan_bind_direct_structure1f(level, commands, &receipt);
        /* DMWeb DGN Structure1F documents direct 64x64 source cells for
         * items, floor decorations, and sensors, but not their Saturn draw
         * or trigger ABI. Do not render the surrounding DGN as a complete
         * runtime scene while one is visible: that would silently omit a
         * real record. */
        if (receipt.structure1f_plan_item_entry_count > 0 ||
            receipt.structure1f_plan_floor_decoration_entry_count > 0 ||
            receipt.structure1f_plan_floor_sensor_entry_count > 0) {
            receipt.status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS;
            receipt.blocks_real_dgn_mesh_render = 1;
            receipt.fallback_visuals_permitted = 0;
            *out_receipt = receipt;
            return 0;
        }
        for (command_index = 0; command_index < receipt.command_count;
             ++command_index) {
            if (commands[command_index].animated_texture_declared) {
                receipt.animated_material_command_count++;
                /* DMWeb DGN files, Structure1B byte4 / Structure1G / Structure2:
                 * animated-floor IDs route through the local Structure2 image
                 * descriptor (global ID - 0x14c). The host still has no verified
                 * Structure2 payload decoder, so a descriptor is provenance,
                 * not a drawable DMDF/BPK surface or static substitution. */
                receipt.unresolved_animated_material_count++;
            }
        }
        /* A Structure2 descriptor is not a pixel/palette decoder. Keep all
         * declared animated commands no-draw until the engine can supply a
         * separately evidenced image route. Static Structure1B MNS commands
         * do not set this counter and remain eligible for their own route. */
        if (receipt.unresolved_animated_material_count > 0) {
            receipt.status =
                NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE;
            receipt.blocks_real_dgn_mesh_render = 1;
            receipt.fallback_visuals_permitted = 0;
            *out_receipt = receipt;
            return 0;
        }
        receipt.material_semantics_complete =
            receipt.floor_material_command_count == receipt.floor_count &&
            receipt.ceiling_material_command_count == receipt.ceiling_count &&
            receipt.wall_material_command_count == receipt.wall_count;
        receipt.plan_ready =
            receipt.command_count > 0 && receipt.material_semantics_complete
                ? 1 : 0;
        receipt.blocks_real_dgn_mesh_render =
            receipt.plan_ready ? 0 : 1;
    }
    *out_receipt = receipt;
    return 0;
}

static int nexus_v1_dgn_structure2_offset_word_bounded(
    const Nexus_V1_Level *level, uint32_t relative_offset)
{
    const Nexus_V1_DgnStructure2Payload *payload;
    uint32_t opaque_start;
    uint32_t opaque_end;

    if (!level) return 0;
    payload = &level->structure2_payload;
    if (!payload->valid || !payload->descriptor_offset_envelope_valid)
        return 0;
    if (relative_offset == 0U) return 1;
    if ((relative_offset & 1U) != 0U || payload->opaque_payload_size < 2)
        return 0;
    opaque_start = (uint32_t)payload->opaque_payload_offset;
    opaque_end = opaque_start + (uint32_t)payload->opaque_payload_size;
    return relative_offset >= opaque_start && relative_offset <= opaque_end - 2U;
}

int nexus_v1_dgn_bind_structure2_animated_floor_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnStructure2FloorCommandSource *out_sources,
    int max_sources, Nexus_V1_DgnStructure2FloorCommandSourceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure2FloorCommandSourceReceipt receipt;
    int command_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !commands || command_count < 0 || !out_sources ||
        max_sources < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (command_index = 0; command_index < command_count; ++command_index) {
        const Nexus_V1_DgnRenderCommand *command = &commands[command_index];
        const Nexus_V1_DgnStructure1GEntry *structure1g;
        const Nexus_V1_DgnStructure2Texture *texture;
        Nexus_V1_DgnStructure2FloorCommandSource *source;

        if (!command->animated_texture_declared) continue;
        ++receipt.animated_floor_command_count;
        if (command->kind != NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
            command->animated_texture_host_route !=
                NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR ||
            !command->animated_texture_structure2_image_valid) {
            ++receipt.blocked_invalid_command_count;
            continue;
        }
        if (command->animated_texture_structure1g_entry_index < 0 ||
            command->animated_texture_structure1g_entry_index >=
                level->structure1g_entry_count) {
            ++receipt.blocked_structure1g_provenance_count;
            continue;
        }
        structure1g = &level->structure1g_entries[
            command->animated_texture_structure1g_entry_index];
        if (structure1g->animation_id != command->animated_texture_id ||
            structure1g->sequence_word_offset !=
                command->animated_texture_structure1g_sequence_word_offset ||
            structure1g->first_image_index !=
                command->animated_texture_first_image_index ||
            !structure1g->first_structure2_image_valid ||
            structure1g->first_structure2_image_id !=
                command->animated_texture_structure2_image_id) {
            ++receipt.blocked_structure1g_provenance_count;
            continue;
        }
        ++receipt.structure1g_provenance_count;
        if (command->animated_texture_first_image_index <
                NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX ||
            (uint16_t)(command->animated_texture_first_image_index -
                NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX) !=
                command->animated_texture_structure2_image_id) {
            ++receipt.blocked_global_image_index_count;
            continue;
        }
        ++receipt.global_image_index_binding_count;
        if (structure1g->sequence_instruction_count <= 0 ||
            structure1g->structure2_image_instruction_bound_count +
                structure1g->structure2_image_instruction_unbound_count !=
                    structure1g->image_instruction_count ||
            structure1g->structure2_image_instruction_unbound_count != 0) {
            ++receipt.blocked_sequence_provenance_count;
            continue;
        }
        ++receipt.complete_sequence_provenance_count;
        if (!nexus_v1_level_structure2_source_envelope_valid(level) ||
            !level->structure1g_structure2_bindings_complete) {
            ++receipt.blocked_source_envelope_count;
            continue;
        }
        texture = nexus_v1_level_get_structure2_texture(
            level, command->animated_texture_structure2_image_id);
        if (!texture) {
            ++receipt.blocked_missing_descriptor_count;
            continue;
        }
        if (!nexus_v1_dgn_structure2_offset_word_bounded(
                level, texture->image_relative_offset) ||
            !nexus_v1_dgn_structure2_offset_word_bounded(
                level, texture->palette_relative_offset)) {
            ++receipt.blocked_descriptor_offset_envelope_count;
            continue;
        }
        ++receipt.descriptor_offset_envelope_count;
        if (receipt.source_command_count >= max_sources) {
            ++receipt.blocked_invalid_command_count;
            continue;
        }
        source = &out_sources[receipt.source_command_count++];
        memset(source, 0, sizeof(*source));
        source->command_index = command_index;
        source->structure1g_entry_index =
            command->animated_texture_structure1g_entry_index;
        source->structure1g_sequence_word_offset =
            command->animated_texture_structure1g_sequence_word_offset;
        source->structure1g_global_image_index =
            command->animated_texture_first_image_index;
        source->structure1g_sequence_instruction_count =
            structure1g->sequence_instruction_count;
        source->structure1g_sequence_image_instruction_count =
            structure1g->image_instruction_count;
        source->structure1g_sequence_goto_instruction_count =
            structure1g->goto_instruction_count;
        source->structure1g_sequence_bound_image_count =
            structure1g->structure2_image_instruction_bound_count;
        source->structure1g_sequence_unbound_image_count =
            structure1g->structure2_image_instruction_unbound_count;
        source->image_id = texture->image_id;
        source->encoding = texture->encoding;
        source->palette_id = texture->palette_id;
        source->width = texture->width;
        source->height = texture->height;
        source->image_relative_offset = texture->image_relative_offset;
        source->palette_relative_offset = texture->palette_relative_offset;
        source->image_offset_word_bounded = 1;
        source->palette_offset_word_bounded = 1;
        source->structure2_source_envelope_valid = 1;
        source->payload_decoder_proven = 0;
        source->draw_authorized = 0;
    }
    receipt.complete = receipt.animated_floor_command_count > 0 &&
        receipt.structure1g_provenance_count ==
            receipt.animated_floor_command_count &&
        receipt.global_image_index_binding_count ==
            receipt.animated_floor_command_count &&
        receipt.complete_sequence_provenance_count ==
            receipt.animated_floor_command_count &&
        receipt.descriptor_offset_envelope_count ==
            receipt.animated_floor_command_count &&
        receipt.source_command_count == receipt.animated_floor_command_count &&
        receipt.blocked_invalid_command_count == 0 &&
        receipt.blocked_structure1g_provenance_count == 0 &&
        receipt.blocked_global_image_index_count == 0 &&
        receipt.blocked_sequence_provenance_count == 0 &&
        receipt.blocked_descriptor_offset_envelope_count == 0 &&
        receipt.blocked_missing_descriptor_count == 0 &&
        receipt.blocked_source_envelope_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_bind_direct_structure1f_floor_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnStructure1FDirectFloorCommandSource *out_sources,
    int max_sources,
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt receipt;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !commands || command_count < 0 || !out_sources ||
        max_sources < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (entry_index = 0; entry_index < level->structure1f_entry_count;
         ++entry_index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[entry_index];
        int command_index;

        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
            continue;
        }
        for (command_index = 0; command_index < command_count;
             ++command_index) {
            Nexus_V1_DgnStructure1FDirectFloorCommandSource *source;
            if (commands[command_index].kind !=
                    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
                commands[command_index].x != entry->x ||
                commands[command_index].y != entry->y) {
                continue;
            }
            ++receipt.visible_direct_entry_count;
            if (receipt.floor_command_source_count >= max_sources) {
                ++receipt.blocked_capacity_count;
                continue;
            }
            source = &out_sources[receipt.floor_command_source_count++];
            memset(source, 0, sizeof(*source));
            source->command_index = command_index;
            source->entry_index = entry_index;
            source->entry = *entry;
            source->draw_authorized = 0;
            if (entry->family == NEXUS_V1_DGN_STRUCTURE1F_ITEMS) {
                ++receipt.item_floor_command_source_count;
            } else if (entry->family ==
                       NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS) {
                ++receipt.floor_decoration_command_source_count;
            } else {
                ++receipt.floor_sensor_command_source_count;
            }
        }
    }
    receipt.complete = receipt.visible_direct_entry_count > 0 &&
        receipt.floor_command_source_count == receipt.visible_direct_entry_count &&
        receipt.blocked_capacity_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_bind_structure1a_owned_cell_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count,
    Nexus_V1_DgnStructure1FStructure1ACommandSource *out_sources,
    int max_sources,
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt receipt;
    int entry_index;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !commands || command_count < 0 || !out_sources ||
        max_sources < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (entry_index = 0; entry_index < level->structure1f_entry_count;
         ++entry_index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[entry_index];
        int command_index;

        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ALCOVES &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS &&
            entry->family != NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS) {
            continue;
        }
        if (!entry->structure1a_relation_valid) {
            ++receipt.blocked_missing_relation_count;
            continue;
        }
        for (command_index = 0; command_index < command_count;
             ++command_index) {
            Nexus_V1_DgnStructure1FStructure1ACommandSource *source;

            if (commands[command_index].kind !=
                    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
                commands[command_index].x != entry->structure1a_owner_x ||
                commands[command_index].y != entry->structure1a_owner_y) {
                continue;
            }
            ++receipt.visible_owned_entry_count;
            if (receipt.floor_command_source_count >= max_sources) {
                ++receipt.blocked_capacity_count;
                continue;
            }
            source = &out_sources[receipt.floor_command_source_count++];
            memset(source, 0, sizeof(*source));
            source->command_index = command_index;
            source->entry_index = entry_index;
            source->entry = *entry;
            source->owner_x = entry->structure1a_owner_x;
            source->owner_y = entry->structure1a_owner_y;
            source->structure3_model_index =
                entry->structure1a_structure3_model_index;
            source->z_rotation = entry->structure1a_z_rotation;
            source->draw_authorized = 0;
            if (entry->family == NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
                ++receipt.alcove_floor_command_source_count;
            } else if (entry->family ==
                       NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS) {
                ++receipt.wall_decoration_floor_command_source_count;
            } else {
                ++receipt.wall_sensor_floor_command_source_count;
            }
        }
    }
    receipt.complete = receipt.visible_owned_entry_count > 0 &&
        receipt.floor_command_source_count == receipt.visible_owned_entry_count &&
        receipt.blocked_missing_relation_count == 0 &&
        receipt.blocked_capacity_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_item_ibs_parse_verified(const uint8_t *data, int size,
                                     int source_hash_verified,
                                     Nexus_V1_ItemIbsBank *out_bank)
{
    enum {
        ITEM_DECLARATIONS = 0x0800,
        ITEM_DECLARATION_BYTES = 40,
        ITEM_PALETTES = 0x3000,
        ITEM_ASSOCIATIONS = 0x3100,
        ITEM_IMAGES = 0x3300,
        ITEM_IMAGE_BYTES = 128,
        ITEM_FLOOR_DECLARATIONS = 0xa800,
        ITEM_FLOOR_DECLARATION_BYTES = 20,
        ITEM_FLOOR_PAYLOAD_START = 0xb098
    };
    int i;
    if (!out_bank) return -1;
    memset(out_bank, 0, sizeof(*out_bank));
    /* DMWeb Nexus ITEM.IBS: 0x18800-byte table.  The source verifier owns
     * identity; the decoder refuses to make an unverified byte stream live. */
    if (!data || size != NEXUS_V1_ITEM_IBS_BYTES || !source_hash_verified)
        return -1;
    for (i = 0; i < NEXUS_V1_ITEM_IBS_DECLARATION_COUNT; ++i) {
        const uint8_t *decl = data + ITEM_DECLARATIONS +
            i * ITEM_DECLARATION_BYTES;
        if (decl[0] != (uint8_t)i) return -1;
        out_bank->inventory_association[i] = rb16(decl + 0x14);
        out_bank->floor_image[i] = rb16(decl + 0x16);
        /* DMWeb records 19 item declarations without an inventory image as
         * FFFF.  They are valid source records, but cannot become an icon
         * material or a substitute for a floor image. */
        if (out_bank->inventory_association[i] >= 256U &&
            out_bank->inventory_association[i] != 0xffffU) return -1;
    }
    for (i = 0; i < NEXUS_V1_ITEM_IBS_PALETTE_COUNT * 16; ++i) {
        out_bank->palette_bgr555[i / 16][i % 16] =
            rb16(data + ITEM_PALETTES + i * 2);
    }
    for (i = 0; i < 256; ++i) {
        uint8_t palette = data[ITEM_ASSOCIATIONS + i * 2];
        uint8_t image = data[ITEM_ASSOCIATIONS + i * 2 + 1];
        /* FF00 denotes an unused association, not a valid surface. */
        if (palette == 0xffU && image == 0x00U) {
            out_bank->association_palette[i] = 0xffU;
            out_bank->association_image[i] = 0xffU;
        } else if (palette < NEXUS_V1_ITEM_IBS_PALETTE_COUNT &&
                   image < NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT) {
            out_bank->association_palette[i] = palette;
            out_bank->association_image[i] = image;
        } else {
            return -1;
        }
    }
    memcpy(out_bank->regular_image_texels, data + ITEM_IMAGES,
           sizeof(out_bank->regular_image_texels));
    {
        uint16_t inherited_palette[16] = {0};
        int have_inherited_palette = 0;
        int terminator_offset = -1;
        for (i = 0; i <= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT; ++i) {
            const uint8_t *decl = data + ITEM_FLOOR_DECLARATIONS +
                i * ITEM_FLOOR_DECLARATION_BYTES;
            uint16_t image_ordinal = rb16(decl);
            uint32_t image_offset = rb32(decl + 12);
            uint32_t palette_offset = rb32(decl + 16);
            Nexus_V1_ItemIbsFloorImage *floor;
            int color;
            if (image_ordinal == 0xffffU) {
                terminator_offset = (int)image_offset;
                break;
            }
            if (i == NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT ||
                image_ordinal >= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT ||
                out_bank->floor_image_count >= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT ||
                image_offset < (uint32_t)(ITEM_FLOOR_PAYLOAD_START - ITEM_FLOOR_DECLARATIONS) ||
                image_offset >= (uint32_t)(NEXUS_V1_ITEM_IBS_BYTES - ITEM_FLOOR_DECLARATIONS))
                return -1;
            floor = &out_bank->floor_images[out_bank->floor_image_count++];
            /* The on-disc declaration uses a 0..108 ordinal.  Item
             * declarations address the combined image space, so its special
             * floor image is regular-image-count + this ordinal.  This is
             * confirmed by the canonical ITEM.IBS where floor refs are
             * 266..331 while these declarations are 43..108. */
            floor->image_id = (uint16_t)(
                NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT + image_ordinal);
            floor->encoding = rb16(decl + 2);
            floor->palette_id = rb16(decl + 4);
            floor->width = rb16(decl + 6);
            floor->height = rb16(decl + 8);
            floor->image_offset = image_offset;
            if (!floor->width || !floor->height || floor->encoding != 8U)
                return -1;
            if (palette_offset != 0U) {
                if (palette_offset > (uint32_t)(NEXUS_V1_ITEM_IBS_BYTES -
                                                  ITEM_FLOOR_DECLARATIONS - 32))
                    return -1;
                for (color = 0; color < 16; ++color) {
                    inherited_palette[color] = rb16(data + ITEM_FLOOR_DECLARATIONS +
                                                     palette_offset + color * 2);
                }
                have_inherited_palette = 1;
            }
            if (!have_inherited_palette) return -1;
            memcpy(floor->palette_bgr555, inherited_palette,
                   sizeof(floor->palette_bgr555));
            floor->palette_bound = 1;
        }
        if (terminator_offset < ITEM_FLOOR_PAYLOAD_START - ITEM_FLOOR_DECLARATIONS ||
            terminator_offset > NEXUS_V1_ITEM_IBS_BYTES - ITEM_FLOOR_DECLARATIONS)
            return -1;
        for (i = 0; i < out_bank->floor_image_count; ++i) {
            uint32_t next = (uint32_t)terminator_offset;
            int j;
            for (j = 0; j < out_bank->floor_image_count; ++j) {
                uint32_t candidate = out_bank->floor_images[j].image_offset;
                if (candidate > out_bank->floor_images[i].image_offset && candidate < next)
                    next = candidate;
            }
            uint32_t packed_bytes;
            uint32_t expected_packed_bytes;
            if (next <= out_bank->floor_images[i].image_offset) return -1;
            out_bank->floor_images[i].image_bytes =
                next - out_bank->floor_images[i].image_offset;
            expected_packed_bytes =
                ((uint32_t)out_bank->floor_images[i].width *
                 (uint32_t)out_bank->floor_images[i].height) / 2U;
            /* The verified retail corpus establishes encoding 0008 as a
             * packed 4bpp payload: every descriptor has at least WxH/2
             * bytes before the next image, with any remaining bytes being a
             * following 32-byte palette record.  Do not promote a different
             * encoding or a truncated surface. */
            if (out_bank->floor_images[i].encoding != 8U ||
                expected_packed_bytes == 0U ||
                expected_packed_bytes >
                    NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_PACKED_BYTES ||
                out_bank->floor_images[i].image_bytes < expected_packed_bytes)
                return -1;
            packed_bytes = expected_packed_bytes;
            memcpy(out_bank->floor_images[i].packed_4bpp_texels,
                   data + ITEM_FLOOR_DECLARATIONS +
                       out_bank->floor_images[i].image_offset,
                   packed_bytes);
            out_bank->floor_images[i].packed_4bpp_bytes = packed_bytes;
            out_bank->floor_images[i].packed_4bpp_valid = 1;
        }
    }
    out_bank->source_hash_verified = 1;
    out_bank->valid = 1;
    return 0;
}

int nexus_v1_dgn_bind_structure1f_item_materials(
    const Nexus_V1_Level *level, const Nexus_V1_ItemIbsBank *bank,
    const Nexus_V1_DgnRenderCommand *commands, int command_count,
    Nexus_V1_DgnStructure1FItemMaterialBinding *out_bindings,
    int max_bindings, Nexus_V1_DgnStructure1FItemMaterialReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FItemMaterialReceipt receipt;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !bank || !bank->valid || !bank->source_hash_verified ||
        !commands || command_count < 0 || !out_bindings || max_bindings < 0) {
        *out_receipt = receipt;
        return -1;
    }
    receipt.source_hash_verified = 1;
    for (i = 0; i < level->structure1f_entry_count; ++i) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[i];
        int command_index = -1;
        int j;
        uint16_t association;
        uint16_t floor_image;
        uint8_t palette;
        uint8_t image;
        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
        ++receipt.item_entry_count;
        if (entry->item_id >= NEXUS_V1_ITEM_IBS_DECLARATION_COUNT) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        for (j = 0; j < command_count; ++j) {
            if (commands[j].x == entry->x && commands[j].y == entry->y &&
                commands[j].kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
                command_index = j;
                break;
            }
        }
        if (command_index < 0) {
            ++receipt.blocked_missing_command_count;
            continue;
        }
        ++receipt.command_candidate_count;
        floor_image = bank->floor_image[entry->item_id];
        /* DMWeb ITEM.IBS explicitly defines FFFF as "same as inventory".
         * A separate floor image has a different, still-unproved codec. */
        if (floor_image != 0xffffU) {
            const Nexus_V1_ItemIbsFloorImage *special = NULL;
            for (j = 0; j < bank->floor_image_count; ++j) {
                if (bank->floor_images[j].image_id == floor_image) {
                    special = &bank->floor_images[j];
                    break;
                }
            }
            if (!special || !special->palette_bound ||
                !special->packed_4bpp_valid) {
                ++receipt.blocked_special_floor_image_count;
                continue;
            }
            if (receipt.bound_regular_inventory_count +
                    receipt.bound_special_floor_palette_count >= max_bindings) {
                ++receipt.blocked_invalid_item_count;
                continue;
            }
            j = receipt.bound_regular_inventory_count +
                receipt.bound_special_floor_palette_count;
            out_bindings[j].entry_index = i;
            out_bindings[j].command_index = command_index;
            out_bindings[j].source_x = entry->x;
            out_bindings[j].source_y = entry->y;
            out_bindings[j].item_id = entry->item_id;
            out_bindings[j].palette_index = 0xffU;
            out_bindings[j].image_index = 0xffU;
            out_bindings[j].palette_bgr555 = special->palette_bgr555;
            out_bindings[j].packed_4bpp_texels = NULL;
            out_bindings[j].special_floor_image = special;
            ++receipt.bound_special_floor_palette_count;
            ++receipt.bound_special_floor_texture_count;
            continue;
        }
        association = bank->inventory_association[entry->item_id];
        if (association == 0xffffU) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        palette = bank->association_palette[association];
        image = bank->association_image[association];
        if (palette >= NEXUS_V1_ITEM_IBS_PALETTE_COUNT ||
            image >= NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT ||
            receipt.bound_regular_inventory_count >= max_bindings) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        j = receipt.bound_regular_inventory_count +
            receipt.bound_special_floor_palette_count;
        out_bindings[j].entry_index = i;
        out_bindings[j].command_index =
            command_index;
        out_bindings[j].source_x = entry->x;
        out_bindings[j].source_y = entry->y;
        out_bindings[j].item_id =
            entry->item_id;
        out_bindings[j].palette_index =
            palette;
        out_bindings[j].image_index = image;
        out_bindings[j].palette_bgr555 =
            bank->palette_bgr555[palette];
        out_bindings[j].packed_4bpp_texels =
            bank->regular_image_texels[image];
        out_bindings[j].special_floor_image = NULL;
        ++receipt.bound_regular_inventory_count;
    }
    receipt.complete = receipt.item_entry_count > 0 &&
        receipt.blocked_special_floor_image_count == 0 &&
        receipt.blocked_missing_command_count == 0 &&
        receipt.blocked_invalid_item_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_consume_structure1f_item_floor_materials(
    const Nexus_V1_DgnStructure1FItemMaterialBinding *bindings,
    int binding_count, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnCommandPacked4BppMaterial *out_materials,
    int max_materials, Nexus_V1_DgnCommandPacked4BppMaterialReceipt *out_receipt)
{
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt receipt;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!bindings || binding_count < 0 || !commands || command_count < 0 ||
        !out_materials || max_materials < 0) {
        *out_receipt = receipt;
        return -1;
    }
    for (i = 0; i < binding_count; ++i) {
        const Nexus_V1_DgnStructure1FItemMaterialBinding *binding =
            &bindings[i];
        const Nexus_V1_ItemIbsFloorImage *floor =
            binding->special_floor_image;
        Nexus_V1_DgnCommandPacked4BppMaterial *material;
        uint32_t expected_bytes;
        if (!floor) continue;
        ++receipt.special_floor_binding_count;
        if (binding->command_index < 0 || binding->command_index >= command_count ||
            commands[binding->command_index].kind !=
                NEXUS_V1_DGN_RENDER_COMMAND_FLOOR) {
            ++receipt.blocked_invalid_command_count;
            continue;
        }
        if (commands[binding->command_index].x != binding->source_x ||
            commands[binding->command_index].y != binding->source_y) {
            ++receipt.blocked_source_cell_mismatch_count;
            continue;
        }
        ++receipt.source_cell_match_count;
        expected_bytes = ((uint32_t)floor->width * (uint32_t)floor->height) / 2U;
        if (!floor->palette_bound || !floor->packed_4bpp_valid ||
            floor->encoding != 8U || !floor->width || !floor->height ||
            expected_bytes == 0U || floor->packed_4bpp_bytes != expected_bytes ||
            receipt.command_material_count >= max_materials) {
            ++receipt.blocked_invalid_binding_count;
            continue;
        }
        material = &out_materials[receipt.command_material_count++];
        memset(material, 0, sizeof(*material));
        material->command_index = binding->command_index;
        material->source_entry_index = binding->entry_index;
        material->source_x = binding->source_x;
        material->source_y = binding->source_y;
        material->item_id = binding->item_id;
        material->image_id = floor->image_id;
        material->encoding = floor->encoding;
        material->width = floor->width;
        material->height = floor->height;
        material->packed_4bpp_bytes = floor->packed_4bpp_bytes;
        material->palette_bgr555 = floor->palette_bgr555;
        material->packed_4bpp_texels = floor->packed_4bpp_texels;
        /* The parser can only be reached through a hash-verified bank.
         * Retain that provenance at the DGN command boundary. */
        material->source_hash_verified = 1;
        material->packed_4bpp_valid = 1;
        material->texel_order_proven = 0;
        material->draw_authorized = 0;
    }
    receipt.source_hash_verified = receipt.command_material_count > 0;
    receipt.complete = receipt.special_floor_binding_count > 0 &&
        receipt.source_cell_match_count == receipt.special_floor_binding_count &&
        receipt.command_material_count == receipt.special_floor_binding_count &&
        receipt.blocked_invalid_binding_count == 0 &&
        receipt.blocked_invalid_command_count == 0 &&
        receipt.blocked_source_cell_mismatch_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_dgn_structure1f_item_ibs_coverage(
    const Nexus_V1_Level *level, const Nexus_V1_ItemIbsBank *bank,
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt *out_receipt)
{
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt receipt;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!level || !bank || !bank->valid || !bank->source_hash_verified) {
        *out_receipt = receipt;
        return -1;
    }
    receipt.source_hash_verified = 1;
    for (i = 0; i < level->structure1f_entry_count; ++i) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &level->structure1f_entries[i];
        uint16_t floor_image;
        int j;
        if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS) continue;
        ++receipt.dgn_item_entry_count;
        if (entry->item_id >= NEXUS_V1_ITEM_IBS_DECLARATION_COUNT) {
            ++receipt.blocked_invalid_item_count;
            continue;
        }
        floor_image = bank->floor_image[entry->item_id];
        if (floor_image == 0xffffU) {
            ++receipt.inventory_inherited_item_count;
            continue;
        }
        ++receipt.special_floor_reference_count;
        for (j = 0; j < bank->floor_image_count; ++j) {
            const Nexus_V1_ItemIbsFloorImage *floor = &bank->floor_images[j];
            if (floor->image_id != floor_image) continue;
            if (floor->encoding != 8U || !floor->palette_bound ||
                !floor->packed_4bpp_valid) {
                ++receipt.blocked_unsupported_encoding_count;
            } else {
                ++receipt.special_floor_0008_count;
            }
            break;
        }
        if (j == bank->floor_image_count)
            ++receipt.blocked_missing_floor_image_count;
    }
    receipt.complete = receipt.dgn_item_entry_count > 0 &&
        receipt.special_floor_reference_count == receipt.special_floor_0008_count &&
        receipt.blocked_invalid_item_count == 0 &&
        receipt.blocked_missing_floor_image_count == 0 &&
        receipt.blocked_unsupported_encoding_count == 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
    const Nexus_V1_ItemIbsFloorImage *floor,
    const Nexus_V1_ItemIbs0008Vdp1Provenance *provenance,
    uint8_t *out_texels, int max_texels,
    Nexus_V1_ItemIbs0008CodecReceipt *out_receipt)
{
    Nexus_V1_ItemIbs0008CodecReceipt receipt;
    uint32_t expected_bytes;
    int texel_count;
    int i;
    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    receipt.fallback_visuals_permitted = 0;
    if (!floor || !out_texels || max_texels < 0) {
        *out_receipt = receipt;
        return -1;
    }
    expected_bytes = ((uint32_t)floor->width * (uint32_t)floor->height) / 2U;
    receipt.descriptor_0008_verified = floor->encoding == 8U;
    receipt.packed_span_verified = expected_bytes > 0U &&
        expected_bytes <= NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_PACKED_BYTES &&
        floor->packed_4bpp_valid && floor->packed_4bpp_bytes == expected_bytes;
    receipt.palette_bound = floor->palette_bound;
    texel_count = (int)(expected_bytes * 2U);
    if (!provenance || !provenance->item_ibs_hash_verified ||
        !provenance->original_vdp1_command_stream_verified ||
        !provenance->vdp1_16_colour_mode_verified ||
        !provenance->vdp1_high_nibble_first_verified) {
        receipt.blocked_missing_vdp1_command_provenance = 1;
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_hash_verified = 1;
    if (!receipt.descriptor_0008_verified || !receipt.packed_span_verified ||
        !receipt.palette_bound || texel_count > max_texels) {
        *out_receipt = receipt;
        return 0;
    }
    /* Mednafen Saturn VDP1 TexFetch(), colour modes 0/1: x=0 consumes the
     * high nibble of the first byte.  This is reachable only through the
     * original-command provenance gate above, never from ITEM.IBS alone. */
    for (i = 0; i < (int)expected_bytes; ++i) {
        uint8_t packed = floor->packed_4bpp_texels[i];
        out_texels[i * 2] = (uint8_t)(packed >> 4);
        out_texels[i * 2 + 1] = (uint8_t)(packed & 0x0fU);
    }
    receipt.decoded_texel_count = texel_count;
    receipt.decode_authorized = 1;
    *out_receipt = receipt;
    return 0;
}
