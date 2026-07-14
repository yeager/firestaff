/* Real-data SAL corpus receipt. This is intentionally a byte-comparison
 * probe: it does not classify any shared byte as a codec field or sample. */
#include "nexus_v1_sal_corpus_receipt.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned char *read_file(const char *path, uint32_t *out_size) {
    FILE *file;
    long size;
    unsigned char *data;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        size > 0xffffffffL || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (uint32_t)size;
    return data;
}

int main(int argc, char **argv) {
    const char *root = argc > 1 ? argv[1] : "/Users/bosse/.firestaff/data/nexus";
    const uint8_t *banks[NEXUS_V1_AUDIO_LEVEL_COUNT] = {0};
    uint32_t sizes[NEXUS_V1_AUDIO_LEVEL_COUNT] = {0};
    Nexus_V1_SalSharedPrefixReceipt receipt;
    int level;

    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level) {
        Nexus_V1_AudioReceipt expected;
        char path[1024];
        (void)snprintf(path, sizeof(path), "%s/SNDLEV%02d.SAL", root, level);
        banks[level] = read_file(path, &sizes[level]);
        if (!banks[level] ||
            nexus_v1_audio_expected_asset(NEXUS_V1_AUDIO_KIND_SAL_BANK,
                                          level, &expected) != NEXUS_V1_AUDIO_OK ||
            sizes[level] != expected.expected_size) {
            printf("SKIP missing-or-size-mismatched %s\n", path);
            while (level-- > 0) free((void *)banks[level]);
            return 0;
        }
    }
    if (nexus_v1_audio_sal_shared_prefix_receipt(banks, sizes, &receipt) !=
            NEXUS_V1_AUDIO_OK ||
        !receipt.complete || receipt.shared_prefix_byte_count != 0x45bb5U ||
        receipt.first_divergent_offset != 0x45bb5U ||
        receipt.codec_or_playback_authorized != 0) {
        printf("FAIL shared=%u divergent-bank=%d divergent-offset=%u complete=%d\n",
               receipt.shared_prefix_byte_count, receipt.first_divergent_bank_index,
               receipt.first_divergent_offset, receipt.complete);
        for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level)
            free((void *)banks[level]);
        return 1;
    }
    printf("PASS shared-prefix=%u first-divergence=0x%x bank=%d no-codec-route\n",
           receipt.shared_prefix_byte_count, receipt.first_divergent_offset,
           receipt.first_divergent_bank_index);
    for (level = 0; level < NEXUS_V1_AUDIO_LEVEL_COUNT; ++level)
        free((void *)banks[level]);
    return 0;
}
