/*
 * Original Nexus DGN Structure1G -> Structure2 reference receipt.
 *
 * This probe intentionally stops at descriptor-relative byte windows. It does
 * not decode opaque payload bytes, name image/palette records, or render a
 * surface. Its only claim is that original Structure1G references select
 * bounded local Structure2 descriptors whose nonzero numeric offsets stay in
 * the descriptor envelope's post-FFFF opaque span.
 */

#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static int read_file(const char *path, uint8_t **out_data, int *out_size) {
    FILE *file;
    long length;
    uint8_t *data;

    if (!path || !out_data || !out_size) return 0;
    *out_data = NULL;
    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0L || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1U, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (int)length;
    return 1;
}

int main(int argc, char **argv) {
    const char *data_dir = argc > 1 ? argv[1] : NULL;
    char default_dir[1024];
    const char *home;
    int level_index;
    int levels_loaded = 0;
    int structure1g_reference_count = 0;
    int global_to_local_binding_count = 0;
    int global_to_local_mismatch_count = 0;
    int raw_sequence_binding_count = 0;
    int raw_sequence_mismatch_count = 0;
    int sequence_image_instruction_count = 0;
    int sequence_image_descriptor_mismatch_count = 0;
    int sequence_goto_instruction_count = 0;
    int sequence_goto_target_mismatch_count = 0;
    int sequence_terminator_count = 0;
    int sequence_unclassified_instruction_count = 0;
    int unique_descriptor_count = 0;
    int nonzero_target_count = 0;
    int outside_target_count = 0;

    if (!data_dir) {
        home = getenv("HOME");
        if (!home || snprintf(default_dir, sizeof(default_dir),
                              "%s/.firestaff/data/nexus", home) <= 0) {
            puts("SKIP: no Nexus data directory argument or HOME");
            return 0;
        }
        data_dir = default_dir;
    }
    for (level_index = 0; level_index < 16; ++level_index) {
        char path[1200];
        uint8_t *data = NULL;
        int size = 0;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure1Layout layout;
        unsigned char seen[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES];
        int entry_index;

        memset(&level, 0, sizeof(level));
        memset(seen, 0, sizeof(seen));
        if (snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir,
                     level_index) <= 0 || !read_file(path, &data, &size)) {
            fprintf(stderr, "FAIL: LEV%02d.DGN is unavailable\n", level_index);
            ++failures;
            continue;
        }
        check(nexus_v1_level_load(&level, data, size, level_index) == 0 &&
                  level.structure2_texture_table_valid &&
                  level.structure2_payload.valid &&
                  level.structure2_payload.material_or_image_data_proven == 0,
              "real DGN retains the bounded, non-decoding Structure2 envelope");
        check(nexus_v1_dgn_structure1_layout(&layout, data, size) == 0 &&
                  layout.valid &&
                  (level.structure1g_entry_count == 0 || layout.structure1g.valid),
              "real DGN retains the validated Structure1G sequence envelope");
        for (entry_index = 0; entry_index < level.structure1g_entry_count;
             ++entry_index) {
            const Nexus_V1_DgnStructure1GEntry *animation =
                &level.structure1g_entries[entry_index];
            const Nexus_V1_DgnStructure2Texture *descriptor;
            const uint8_t *structure1g;
            const uint8_t *raw_descriptor;
            int animation_data_relative_offset;
            uint16_t sequence_word_offset;
            uint16_t next_sequence_word_offset;
            int sequence_byte_offset;
            int next_sequence_byte_offset;
            int cursor;
            int terminated = 0;
            uint32_t offsets[2];
            int offset_index;

            check(animation->first_structure2_image_valid &&
                      animation->first_structure2_image_id <
                          (uint16_t)level.structure2_texture_count,
                  "Structure1G reference selects a local Structure2 descriptor");
            if (!animation->first_structure2_image_valid ||
                animation->first_structure2_image_id >=
                    (uint16_t)level.structure2_texture_count) continue;
            ++structure1g_reference_count;
            descriptor = &level.structure2_textures[
                animation->first_structure2_image_id];
            structure1g = data + layout.structure1_offset +
                layout.structure1g.relative_offset;
            raw_descriptor = structure1g + NEXUS_DGN_STRUCTURE1G_HEADER_BYTES +
                entry_index * NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES;
            animation_data_relative_offset = (int)read_be16(structure1g + 2);
            sequence_word_offset = read_be16(raw_descriptor + 6);
            sequence_byte_offset = animation_data_relative_offset +
                (int)sequence_word_offset * 4;
            if (sequence_byte_offset < animation_data_relative_offset ||
                sequence_byte_offset > layout.structure1g.size - 4 ||
                read_be16(raw_descriptor + 4) != animation->first_image_index ||
                read_be16(structure1g + sequence_byte_offset) !=
                    animation->first_image_index) {
                ++raw_sequence_mismatch_count;
            } else {
                ++raw_sequence_binding_count;
            }
            next_sequence_word_offset = entry_index + 1 <
                level.structure1g_entry_count
                ? read_be16(raw_descriptor +
                            NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES + 6)
                : (uint16_t)((layout.structure1g.size -
                              animation_data_relative_offset) / 4);
            next_sequence_byte_offset = animation_data_relative_offset +
                (int)next_sequence_word_offset * 4;
            for (cursor = sequence_byte_offset;
                 cursor < next_sequence_byte_offset; cursor += 4) {
                uint16_t instruction = read_be16(structure1g + cursor);
                uint16_t local_image_id;
                if (instruction == 0xffffU) {
                    terminated = 1;
                    ++sequence_terminator_count;
                    break;
                }
                if (instruction == 0xfffeU) {
                    int target_words =
                        (int)(int16_t)read_be16(structure1g + cursor + 2);
                    int target_byte_offset = cursor + target_words * 4;
                    ++sequence_goto_instruction_count;
                    if (target_words >= 0 || target_byte_offset <
                            sequence_byte_offset || target_byte_offset >= cursor ||
                        (target_byte_offset - sequence_byte_offset) % 4 != 0) {
                        ++sequence_goto_target_mismatch_count;
                    }
                    continue;
                }
                ++sequence_image_instruction_count;
                if (instruction < 0x014cU) {
                    ++sequence_image_descriptor_mismatch_count;
                    ++sequence_unclassified_instruction_count;
                    continue;
                }
                local_image_id = (uint16_t)(instruction - 0x014cU);
                if (local_image_id >= (uint16_t)level.structure2_texture_count ||
                    level.structure2_textures[local_image_id].image_id !=
                        local_image_id) {
                    ++sequence_image_descriptor_mismatch_count;
                }
            }
            if (!terminated) ++sequence_unclassified_instruction_count;
            if (animation->first_image_index < 0x014cU ||
                (uint16_t)(animation->first_image_index - 0x014cU) !=
                    animation->first_structure2_image_id ||
                descriptor->image_id != animation->first_structure2_image_id) {
                ++global_to_local_mismatch_count;
            } else {
                ++global_to_local_binding_count;
            }
            if (!seen[animation->first_structure2_image_id]) {
                seen[animation->first_structure2_image_id] = 1U;
                ++unique_descriptor_count;
            }
            offsets[0] = descriptor->image_relative_offset;
            offsets[1] = descriptor->palette_relative_offset;
            for (offset_index = 0; offset_index < 2; ++offset_index) {
                uint32_t offset = offsets[offset_index];
                uint32_t opaque_begin =
                    (uint32_t)level.structure2_payload.opaque_payload_offset;
                uint32_t opaque_end = opaque_begin +
                    (uint32_t)level.structure2_payload.opaque_payload_size;
                if (offset == 0U) continue;
                ++nonzero_target_count;
                if (offset < opaque_begin || offset >= opaque_end) {
                    ++outside_target_count;
                }
            }
        }
        ++levels_loaded;
        free(data);
    }
    check(levels_loaded == 16, "all 16 original LEV DGN files load");
    check(structure1g_reference_count > 0,
          "original corpus contains Structure1G-to-Structure2 references");
    check(unique_descriptor_count > 0,
          "Structure1G references reach at least one local descriptor");
    check(global_to_local_binding_count == structure1g_reference_count &&
              global_to_local_mismatch_count == 0,
          "global Structure1G image indexes bind their local Structure2 IDs");
    check(raw_sequence_binding_count == structure1g_reference_count &&
              raw_sequence_mismatch_count == 0,
          "Structure1G first-image fields match original sequence instructions");
    check(sequence_image_instruction_count > 0 &&
              sequence_image_descriptor_mismatch_count == 0,
          "Structure1G sequence image indexes bind local Structure2 descriptors");
    check(sequence_goto_target_mismatch_count == 0,
          "Structure1G backward gotos stay within their original sequences");
    check(sequence_terminator_count == structure1g_reference_count &&
              sequence_unclassified_instruction_count == 0,
          "Structure1G sequences terminate without unclassified instructions");
    check(nonzero_target_count > 0 && outside_target_count == 0,
          "referenced descriptor offsets remain within opaque payload spans");
    printf("Nexus DGN Structure2 reference flow: levels=%d references=%d "
           "global-local=%d mismatches=%d raw-sequences=%d sequence-mismatches=%d "
           "sequence-images=%d image-mismatches=%d unique-descriptors=%d "
           "gotos=%d goto-mismatches=%d terminators=%d unclassified=%d "
           "nonzero-targets=%d outside=%d; "
           "decoder/render-proof=0\n",
           levels_loaded, structure1g_reference_count, global_to_local_binding_count,
           global_to_local_mismatch_count, raw_sequence_binding_count,
           raw_sequence_mismatch_count, sequence_image_instruction_count,
           sequence_image_descriptor_mismatch_count, unique_descriptor_count,
           sequence_goto_instruction_count, sequence_goto_target_mismatch_count,
           sequence_terminator_count, sequence_unclassified_instruction_count,
           nonzero_target_count, outside_target_count);
    return failures == 0 ? 0 : 1;
}
