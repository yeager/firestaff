/* Hash-bound Structure1F -> Structure1A owner corpus. This is deliberately
 * raw provenance: it proves neither Structure3 entry selection nor rendering. */
#include "nexus_v1_engine.h"
#include "asset_find_by_hash.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static uint64_t fnv_byte(uint64_t hash, uint8_t value)
{
    return (hash ^ value) * UINT64_C(1099511628211);
}

static uint64_t fnv_u16le(uint64_t hash, uint16_t value)
{
    hash = fnv_byte(hash, (uint8_t)value);
    return fnv_byte(hash, (uint8_t)(value >> 8));
}

static int owner_family(Nexus_V1_DgnStructure1FFamily family)
{
    return family == NEXUS_V1_DGN_STRUCTURE1F_ALCOVES ||
        family == NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS ||
        family == NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS;
}

int main(int argc, char **argv)
{
    uint64_t corpus = UINT64_C(1469598103934665603);
    int family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] = {0};
    int total = 0;
    int level;

    if (argc != 2) {
        fprintf(stderr, "usage: %s NEXUS_DATA_DIRECTORY\n", argv[0]);
        return 2;
    }
    for (level = 0; level <= 15; ++level) {
        char name[16];
        char path[1024];
        const char *md5;
        uint8_t *bytes;
        int size;
        Nexus_V1_Level data;
        Nexus_V1_DgnStructure1ARelationReceipt relation;
        int entry;

        snprintf(name, sizeof(name), "LEV%02d.DGN", level);
        md5 = nexus_v1_known_file_md5(name);
        if (snprintf(path, sizeof(path), "%s/%s", argv[1], name) >=
                (int)sizeof(path) || !md5 || !asset_file_matches_md5(path, md5) ||
            !(bytes = read_file(path, &size))) goto failed;
        memset(&data, 0, sizeof(data));
        if (nexus_v1_level_load(&data, bytes, size, level) != 0 ||
            nexus_v1_level_structure1a_relation_receipt(&data, &relation) != 0 ||
            !relation.complete || relation.missing_owner_entry_count != 0 ||
            relation.ambiguous_owner_entry_count != 0 ||
            relation.out_of_range_index_count != 0) {
            free(bytes);
            goto failed;
        }
        corpus = fnv_byte(corpus, (uint8_t)level);
        for (entry = 0; entry < data.structure1f_entry_count; ++entry) {
            const Nexus_V1_DgnStructure1FEntry *record =
                &data.structure1f_entries[entry];
            const Nexus_V1_DgnStructure1AModel *model;
            if (!owner_family(record->family)) continue;
            if (!record->structure1a_relation_valid ||
                record->structure1a_index >= (uint16_t)data.structure1a_model_count) {
                free(bytes);
                goto failed;
            }
            model = &data.structure1a_models[record->structure1a_index];
            if (record->structure1a_structure3_model_index !=
                    model->structure3_model_index ||
                record->structure1a_z_rotation != model->z_rotation) {
                free(bytes);
                goto failed;
            }
            corpus = fnv_byte(corpus, (uint8_t)record->family);
            corpus = fnv_byte(corpus, record->tag);
            corpus = fnv_u16le(corpus, record->structure1a_index);
            corpus = fnv_byte(corpus, (uint8_t)record->structure1a_owner_x);
            corpus = fnv_byte(corpus, (uint8_t)record->structure1a_owner_y);
            corpus = fnv_byte(corpus, record->model_or_aspect);
            corpus = fnv_byte(corpus, record->rotation);
            corpus = fnv_byte(corpus, record->face);
            corpus = fnv_byte(corpus, model->kind);
            corpus = fnv_byte(corpus, model->structure3_model_index);
            corpus = fnv_byte(corpus, model->z_rotation);
            ++family_count[record->family];
            ++total;
        }
        free(bytes);
    }
    printf("verified_structure1f_structure1a_owner_rows=%d\n", total);
    printf("owner_rows_alcoves=%d\n", family_count[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES]);
    printf("owner_rows_wall_decorations=%d\n",
           family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS]);
    printf("owner_rows_wall_sensors=%d\n",
           family_count[NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS]);
    printf("owner_corpus_fnv1a64=%016llx\n", (unsigned long long)corpus);
    printf("structure1a_model_entry_mapping_proven=0\n");
    printf("decoder_or_renderer_authorized=0\n");
    return 0;
failed:
    fprintf(stderr, "canonical Structure1F/Structure1A owner corpus unavailable\n");
    return 1;
}
