#include "theron_v1_level_descriptor.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_normalized(const char *name, size_t *out_size) {
    const char *home = getenv("HOME");
    char path[1024];
    FILE *fp;
    long raw_size;
    uint8_t *raw;
    uint8_t *user_data;
    size_t sectors;
    size_t i;

    if (out_size) *out_size = 0u;
    if (!home) return NULL;
    (void)snprintf(path, sizeof(path), "%s/.firestaff/data/theron/%s",
                   home, name);
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    assert(fseek(fp, 0, SEEK_END) == 0);
    raw_size = ftell(fp);
    assert(raw_size > 0 && raw_size % 2352 == 0);
    sectors = (size_t)raw_size / 2352u;
    assert(fseek(fp, 0, SEEK_SET) == 0);
    raw = (uint8_t *)malloc((size_t)raw_size);
    user_data = (uint8_t *)malloc(sectors * 2048u);
    assert(raw && user_data);
    assert(fread(raw, 1, (size_t)raw_size, fp) == (size_t)raw_size);
    assert(fclose(fp) == 0);
    for (i = 0; i < sectors; ++i)
        (void)memcpy(user_data + i * 2048u, raw + i * 2352u + 16u, 2048u);
    free(raw);
    if (out_size) *out_size = sectors * 2048u;
    return user_data;
}

static void test_authentic_regional_receipts(void) {
    Theron_LevelDescriptor parsed[THERON_LEVEL_DESCRIPTOR_COUNT];
    Theron_LevelDescriptorCorpusReceipt receipt;
    size_t us_size = 0u;
    size_t jp_size = 0u;
    uint8_t *us = read_normalized("TQUS02.bin", &us_size);
    uint8_t *jp = read_normalized("TQJP02.bin", &jp_size);

    if (!us || !jp) {
        printf("SKIP: authentic US/JP Track 02 BINs not available\n");
        free(us);
        free(jp);
        return;
    }
    assert(theron_v1_level_descriptor_read_authenticated_track02(
        us, us_size, "f23601102138f87c33025877767ebf76",
        parsed, THERON_LEVEL_DESCRIPTOR_COUNT, &receipt));
    assert(receipt.valid && !receipt.zero_fill && receipt.records_available);
    assert(receipt.source_fnv1a == 0x7aa82bc7u);
    assert(parsed[16].data_size == 0xE000);
    assert(parsed[52].cumulative_sector_offset == 2);
    assert(!theron_v1_level_descriptor_read_authenticated_track02(
        jp, jp_size, "f23601102138f87c33025877767ebf76",
        parsed, THERON_LEVEL_DESCRIPTOR_COUNT, &receipt));

    memset(parsed, 0xA5, sizeof(parsed));
    memset(&receipt, 0, sizeof(receipt));
    assert(theron_v1_level_descriptor_read_authenticated_track02(
        jp, jp_size, "b7afb338ad31be1025b53f9aff12d73a",
        parsed, THERON_LEVEL_DESCRIPTOR_COUNT, &receipt));
    assert(receipt.valid && receipt.zero_fill && !receipt.records_available);
    assert(receipt.source_fnv1a == 0x63d8ddfdu);
    for (size_t i = 0u; i < THERON_LEVEL_DESCRIPTOR_COUNT; ++i) {
        assert(parsed[i].flags == 0u);
        assert(parsed[i].data_size == 0u);
    }
    assert(!theron_v1_level_descriptor_read_authenticated_track02(
        us, us_size, "b7afb338ad31be1025b53f9aff12d73a",
        parsed, THERON_LEVEL_DESCRIPTOR_COUNT, &receipt));
    free(us);
    free(jp);
}

int main(void) {
    assert(theron_v1_level_descriptor_count() == 53);

    const Theron_LevelDescriptor *d0 = theron_v1_level_descriptor(0);
    (void)d0;
    assert(d0 != NULL);
    assert(d0->flags == 1);
    assert(d0->sector_count == 2);
    assert(d0->data_size == 0x0876);
    assert(d0->cumulative_sector_offset == 2);

    const Theron_LevelDescriptor *d16 = theron_v1_level_descriptor(16);
    (void)d16;
    assert(d16 != NULL);
    assert(d16->sector_count == 28);
    assert(d16->data_size == 0xE000);

    const Theron_LevelDescriptor *d42 = theron_v1_level_descriptor(42);
    (void)d42;
    assert(d42 != NULL);
    assert(d42->sector_count == 1);
    assert(d42->data_size == 0x0280);
    assert(d42->cumulative_sector_offset == 232);

    const Theron_LevelDescriptor *d52 = theron_v1_level_descriptor(52);
    (void)d52;
    assert(d52 != NULL);
    assert(d52->sector_count == 1);
    assert(d52->data_size == 0x023D);
    assert(d52->cumulative_sector_offset == 2);

    assert(theron_v1_level_descriptor(53) == NULL);

    for (unsigned int i = 0; i < 53; i++) {
        const Theron_LevelDescriptor *d = theron_v1_level_descriptor(i);
        assert(d != NULL);
        assert(d->flags == 1);
        assert(d->sector_count >= 1);
        assert(d->data_size > 0);
    }

    test_authentic_regional_receipts();

    printf("PASS: theron_v1_level_descriptor\n");
    return 0;
}
