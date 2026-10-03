#include "theron_v1_level_descriptor.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_normalized(const char *path, size_t *out_size) {
    FILE *fp;
    long raw_size;
    uint8_t *raw;
    uint8_t *user_data;
    size_t sectors;
    size_t i;
    size_t bytes_read;
    int read_error;
    int close_result;

    if (out_size) *out_size = 0u;
    if (!path || !path[0]) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    raw_size = ftell(fp);
    if (raw_size <= 0 || (uintmax_t)raw_size > (uintmax_t)SIZE_MAX ||
        raw_size % 2352 != 0 ||
        fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    sectors = (size_t)raw_size / 2352u;
    if (sectors > SIZE_MAX / 2048u) {
        fclose(fp);
        return NULL;
    }
    raw = (uint8_t *)malloc((size_t)raw_size);
    user_data = (uint8_t *)malloc(sectors * 2048u);
    if (!raw || !user_data) {
        free(raw);
        free(user_data);
        fclose(fp);
        return NULL;
    }
    bytes_read = fread(raw, 1, (size_t)raw_size, fp);
    read_error = ferror(fp);
    close_result = fclose(fp);
    if (bytes_read != (size_t)raw_size || read_error || close_result != 0) {
        free(raw);
        free(user_data);
        return NULL;
    }
    for (i = 0; i < sectors; ++i)
        (void)memcpy(user_data + i * 2048u, raw + i * 2352u + 16u, 2048u);
    free(raw);
    if (out_size) *out_size = sectors * 2048u;
    return user_data;
}

static int resolve_track02_path(const char *region, const char **path_out,
                                char *fallback, size_t fallback_size) {
    const char *home = getenv("HOME");
    const char *override = strcmp(region, "jp") == 0
        ? getenv("FIRESTAFF_THERON_TRACK02_JP_RAW")
        : getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *filename = strcmp(region, "jp") == 0
        ? "TQJP02.bin" : "TQUS02.bin";
    FILE *file;
    int written;

    if (override && override[0]) {
        *path_out = override;
        return 0;
    }
    if (!home || !home[0]) return 77;
    written = snprintf(fallback, fallback_size,
                       "%s/.firestaff/data/theron/%s", home, filename);
    if (written < 0 || (size_t)written >= fallback_size) {
        fputs("FAIL: HOME path for Track 02 media is too long\n", stderr);
        return 1;
    }
    errno = 0;
    file = fopen(fallback, "rb");
    if (!file) {
        if (errno == ENOENT || errno == ENOTDIR) return 77;
        fprintf(stderr, "FAIL: cannot open default %s Track 02 media: %s\n",
                strcmp(region, "jp") == 0 ? "JP" : "US",
                strerror(errno));
        return 1;
    }
    if (fclose(file) != 0) {
        fputs("FAIL: could not close default Track 02 media\n", stderr);
        return 1;
    }
    *path_out = fallback;
    return 0;
}

static int test_authentic_regional_receipt(const char *region,
                                           const uint8_t *track02,
                                           size_t track02_size) {
    Theron_LevelDescriptor parsed[THERON_LEVEL_DESCRIPTOR_COUNT];
    Theron_LevelDescriptorCorpusReceipt receipt;
    if (strcmp(region, "us") == 0) {
        if (!theron_v1_level_descriptor_read_authenticated_track02(
                track02, track02_size,
                "f23601102138f87c33025877767ebf76", parsed,
                THERON_LEVEL_DESCRIPTOR_COUNT, &receipt)) {
            fputs("FAIL: selected media is not the authentic US Track 02 image\n",
                  stderr);
            return 1;
        }
        assert(receipt.valid && !receipt.zero_fill && receipt.records_available);
        assert(receipt.source_fnv1a == 0x7aa82bc7u);
        assert(parsed[16].data_size == 0xE000);
        assert(parsed[52].cumulative_sector_offset == 2);
        assert(!theron_v1_level_descriptor_read_authenticated_track02(
            track02, track02_size, "b7afb338ad31be1025b53f9aff12d73a",
            parsed, THERON_LEVEL_DESCRIPTOR_COUNT, &receipt));
    } else {
        memset(parsed, 0xA5, sizeof(parsed));
        memset(&receipt, 0, sizeof(receipt));
        if (!theron_v1_level_descriptor_read_authenticated_track02(
                track02, track02_size,
                "b7afb338ad31be1025b53f9aff12d73a", parsed,
                THERON_LEVEL_DESCRIPTOR_COUNT, &receipt)) {
            fputs("FAIL: selected media is not the authentic JP Track 02 image\n",
                  stderr);
            return 1;
        }
        assert(receipt.valid && receipt.zero_fill && !receipt.records_available);
        assert(receipt.source_fnv1a == 0x63d8ddfdu);
        for (size_t i = 0u; i < THERON_LEVEL_DESCRIPTOR_COUNT; ++i) {
            assert(parsed[i].flags == 0u);
            assert(parsed[i].data_size == 0u);
        }
        assert(!theron_v1_level_descriptor_read_authenticated_track02(
            track02, track02_size, "f23601102138f87c33025877767ebf76",
            parsed, THERON_LEVEL_DESCRIPTOR_COUNT, &receipt));
    }
    return 0;
}

int main(int argc, char **argv) {
    char fallback[4096];
    const char *track02_path = NULL;
    uint8_t *track02;
    size_t track02_size = 0u;
    int resolve_result;

    if (argc != 2 || (strcmp(argv[1], "static") != 0 &&
                      strcmp(argv[1], "us") != 0 &&
                      strcmp(argv[1], "jp") != 0)) {
        fputs("usage: test_theron_v1_level_descriptor static|us|jp\n", stderr);
        return 2;
    }

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

    if (strcmp(argv[1], "static") == 0) {
        printf("PASS: theron_v1_level_descriptor_static\n");
        return 0;
    }
    resolve_result = resolve_track02_path(argv[1], &track02_path, fallback,
                                          sizeof(fallback));
    if (resolve_result != 0) {
        if (resolve_result == 77) {
            printf("SKIP: default %s Track 02 media unavailable\n",
                   strcmp(argv[1], "jp") == 0 ? "JP" : "US");
        }
        return resolve_result;
    }
    track02 = read_normalized(track02_path, &track02_size);
    if (!track02) {
        fputs("FAIL: could not read selected Track 02 media\n", stderr);
        return 1;
    }

    if (test_authentic_regional_receipt(argv[1], track02,
                                        track02_size) != 0) {
        free(track02);
        return 1;
    }
    free(track02);

    printf("PASS: theron_v1_level_descriptor_%s_real_media\n", argv[1]);
    return 0;
}
