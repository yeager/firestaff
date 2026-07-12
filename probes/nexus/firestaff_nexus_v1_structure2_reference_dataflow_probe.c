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
        for (entry_index = 0; entry_index < level.structure1g_entry_count;
             ++entry_index) {
            const Nexus_V1_DgnStructure1GEntry *animation =
                &level.structure1g_entries[entry_index];
            const Nexus_V1_DgnStructure2Texture *descriptor;
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
    check(nonzero_target_count > 0 && outside_target_count == 0,
          "referenced descriptor offsets remain within opaque payload spans");
    printf("Nexus DGN Structure2 reference flow: levels=%d references=%d "
           "global-local=%d mismatches=%d unique-descriptors=%d "
           "nonzero-targets=%d outside=%d; "
           "decoder/render-proof=0\n",
           levels_loaded, structure1g_reference_count, global_to_local_binding_count,
           global_to_local_mismatch_count, unique_descriptor_count,
           nonzero_target_count, outside_target_count);
    return failures == 0 ? 0 : 1;
}
