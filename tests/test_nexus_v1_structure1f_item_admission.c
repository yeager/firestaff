#include "nexus_v1_structure1f_item_admission.h"

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
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *size = (int)length;
    return bytes;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_LevCorpusDiscoveryReceipt corpus;
    uint32_t level;
    uint32_t admitted = 0U;

    if (!data_dir || !*data_dir) return 77;
    if (!nexus_v1_lev_corpus_discover_direct(data_dir, &corpus)) return 77;

    for (level = 0U; level < NEXUS_V1_LEV_CORPUS_LEVEL_COUNT; ++level) {
        Nexus_V1_Structure1FDirectoryAdmissionReceipt directory;
        Nexus_V1_Structure1FItemAdmissionReceipt receipt;
        uint8_t *bytes;
        int size;
        uint8_t original_tag;
        uint32_t other_level;

        bytes = read_file(corpus.levels[level].direct_path, &size);
        if (!bytes || !nexus_v1_structure1f_directory_admit(
                &corpus.levels[level], bytes, size, &directory)) {
            free(bytes);
            return 1;
        }
        if (!directory.families[NEXUS_V1_DGN_STRUCTURE1F_ITEMS].record_count) {
            free(bytes);
            continue;
        }
        if (!nexus_v1_structure1f_item_admit(
                &corpus.levels[level], bytes, size, &directory, 0U, &receipt) ||
            !receipt.valid || !receipt.coordinate_pair_bound ||
            receipt.source_tag != 0x10U ||
            receipt.cell_ordinal != (uint16_t)(receipt.y * 64U + receipt.x) ||
            memcmp(receipt.opaque_tail, bytes + receipt.record_offset + 3U,
                   sizeof(receipt.opaque_tail)) != 0 ||
            receipt.mesh_semantics_permitted || receipt.face_semantics_permitted ||
            receipt.texture_semantics_permitted || receipt.draw_permitted) {
            free(bytes);
            return 1;
        }
        ++admitted;

        other_level = (level + 1U) % NEXUS_V1_LEV_CORPUS_LEVEL_COUNT;
        if (nexus_v1_structure1f_item_admit(
                &corpus.levels[other_level], bytes, size, &directory, 0U,
                &receipt) || receipt.valid) {
            free(bytes);
            return 1;
        }

        original_tag = bytes[receipt.record_offset];
        bytes[receipt.record_offset] ^= 1U;
        if (nexus_v1_structure1f_item_admit(
                &corpus.levels[level], bytes, size, &directory, 0U, &receipt) ||
            receipt.valid) {
            free(bytes);
            return 1;
        }
        bytes[receipt.record_offset] = original_tag;
        if (nexus_v1_structure1f_item_admit(
                &corpus.levels[level], bytes, size, &directory,
                directory.families[NEXUS_V1_DGN_STRUCTURE1F_ITEMS].record_count,
                &receipt) || receipt.valid) {
            free(bytes);
            return 1;
        }
        free(bytes);
    }

    if (!admitted) return 1;
    puts("Structure1F item admission: PASS");
    return 0;
}
