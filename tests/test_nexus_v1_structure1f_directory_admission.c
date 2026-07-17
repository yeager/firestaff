#include "nexus_v1_structure1f_directory_admission.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, int *size)
{
    FILE *file;
    long length;
    uint8_t *bytes;
    *size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) || (length = ftell(file)) <= 0 ||
        length > INT_MAX || fseek(file, 0, SEEK_SET)) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)length);
    if (!bytes || fread(bytes, 1U, (size_t)length, file) != (size_t)length) {
        free(bytes); fclose(file); return NULL;
    }
    fclose(file);
    *size = (int)length;
    return bytes;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_LevCorpusDiscoveryReceipt corpus;
    Nexus_V1_Structure1FDirectoryAdmissionReceipt receipt;
    uint32_t level;

    if (!data_dir || !*data_dir) return 77;
    if (!nexus_v1_lev_corpus_discover_direct(data_dir, &corpus)) return 77;
    for (level = 0U; level < NEXUS_V1_LEV_CORPUS_LEVEL_COUNT; ++level) {
        Nexus_V1_LevCorpusDirectLevelIdentity identity = corpus.levels[level];
        uint8_t *bytes;
        int size;
        uint32_t family;
        uint32_t total = 0U;
        uint8_t saved;

        if (!(bytes = read_file(identity.direct_path, &size)) ||
            !nexus_v1_structure1f_directory_admit(&identity, bytes, size, &receipt) ||
            !receipt.valid || !receipt.direct_identity_bound ||
            !receipt.parser_layout_bound || !receipt.family_directory_bound ||
            !receipt.no_draw_only || receipt.geometry_semantics_permitted ||
            receipt.texture_semantics_permitted || receipt.draw_permitted ||
            receipt.level_index != level || receipt.package_fnv1a64 != identity.fnv1a64) {
            free(bytes); return 1;
        }
        for (family = 0U; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
            const Nexus_V1_Structure1FDirectoryFamilyReceipt *row = &receipt.families[family];
            if (!row->record_size || row->record_length != row->record_count * row->record_size ||
                row->record_offset < receipt.directory_offset + NEXUS_DGN_STRUCTURE1F_HEADER_BYTES ||
                row->record_offset + row->record_length >
                    receipt.directory_offset + receipt.directory_length ||
                (row->record_count && !row->record_fnv1a64) ||
                (!row->record_count && row->record_fnv1a64)) {
                free(bytes); return 1;
            }
            total += row->record_count;
        }
        if (total != receipt.total_record_count) { free(bytes); return 1; }
        saved = bytes[receipt.directory_offset];
        bytes[receipt.directory_offset] ^= 1U;
        if (nexus_v1_structure1f_directory_admit(&identity, bytes, size, &receipt) || receipt.valid) {
            free(bytes); return 1;
        }
        bytes[receipt.directory_offset] = saved;
        identity.byte_count--;
        if (nexus_v1_structure1f_directory_admit(&identity, bytes, size, &receipt) || receipt.valid) {
            free(bytes); return 1;
        }
        free(bytes);
    }
    puts("Structure1F direct corpus directory admission: PASS");
    return 0;
}
