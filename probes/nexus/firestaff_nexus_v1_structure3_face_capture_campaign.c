/* Emit one no-draw Saturn capture request for every bounded Structure3 face.
 * The campaign records complete Structure1F/1A attachment evidence but does
 * not infer a Structure1A-model-to-Structure3-entry mapping. */
#include "nexus_v1_engine.h"
#include "asset_find_by_hash.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_level(const char *text, int *out_level)
{
    char *end = NULL;
    long value;

    if (!text || !out_level) return 0;
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value < 0L || value > 15L)
        return 0;
    *out_level = (int)value;
    return 1;
}

static uint8_t *read_file(const char *path, int *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        size > INT_MAX || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
    return data;
}

int main(int argc, char **argv)
{
    Nexus_V1_Level level_data;
    Nexus_V1_DgnStructure3AttachmentReceipt attachments;
    Nexus_V1_DgnStructure3CaptureTargetReceipt target;
    int level;
    int last_level;
    int entry;
    int face;
    int written = 0;

    if (argc != 4 ||
        (strcmp(argv[2], "all") != 0 && !parse_level(argv[2], &level))) {
        fprintf(stderr, "usage: %s DATA_DIR LEVEL|all OUTPUT_DIRECTORY\n", argv[0]);
        return 2;
    }
    if (strcmp(argv[2], "all") == 0) level = 0;
    last_level = strcmp(argv[2], "all") == 0 ? 15 : level;
    for (; level <= last_level; ++level) {
        char path[1024];
        char lev_path[1024];
        char name[16];
        const char *md5;
        uint8_t *data;
        int data_size = 0;

        snprintf(name, sizeof(name), "LEV%02d.DGN", level);
        md5 = nexus_v1_known_file_md5(name);
        if (snprintf(lev_path, sizeof(lev_path), "%s/%s", argv[1], name) >=
                (int)sizeof(lev_path) || !md5 ||
            !asset_file_matches_md5(lev_path, md5) ||
            !(data = read_file(lev_path, &data_size))) {
            fprintf(stderr, "canonical LEV%02d bytes unavailable\n", level);
            return 1;
        }
        memset(&level_data, 0, sizeof(level_data));
        if (nexus_v1_level_load(&level_data, data, data_size, level) != 0 ||
            nexus_v1_level_structure3_attachment_receipt(
                &level_data, &attachments) != 0 || !attachments.complete ||
            !attachments.record_to_face_normal_semantics_proven ||
            attachments.normal_plane_transform_or_draw_semantics_proven) {
            fprintf(stderr, "canonical Structure1F/Structure3 route unavailable for LEV%02d\n",
                    level);
            free(data);
            return 1;
        }
        for (entry = 0; entry < level_data.structure3_directory.entry_count;
             ++entry) {
            uint32_t face_count = level_data.structure3_entry_face_counts[entry];
            for (face = 0; face < (int)face_count; ++face) {
                if (snprintf(path, sizeof(path), "%s/LEV%02d-E%04d-F%04d.target",
                             argv[3], level, entry, face) >= (int)sizeof(path) ||
                    !nexus_v1_dgn_structure3_capture_target_build(
                        &level_data, data, data_size, level, 1, (uint32_t)entry,
                        (uint32_t)face, &target) || !target.valid ||
                    !target.capture_producer_required ||
                    !target.original_saturn_capture_required || !target.no_draw_only ||
                    target.fallback_visuals_permitted ||
                    !nexus_v1_dgn_structure3_capture_target_write(path, &target)) {
                    fprintf(stderr, "could not write LEV%02d Structure3 face %d/%d\n",
                            level, entry, face);
                    free(data);
                    return 1;
                }
                ++written;
            }
        }
        free(data);
    }
    printf("wrote %d no-draw Structure3 face targets\n", written);
    printf("structure1f_face_mesh_ordinal_relation_proven=1\n");
    printf("structure1a_model_entry_mapping_proven=0\n");
    printf("original_saturn_capture_required=1\n");
    printf("decoder_or_renderer_authorized=0\n");
    return 0;
}
