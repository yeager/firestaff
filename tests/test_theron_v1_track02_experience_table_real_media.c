#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_experience_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_BYTES 2048u
#define RAW_USER_DATA_OFFSET 16u
#define US_TABLE_UD_OFFSET 0x1DA890u
#define JP_TABLE_UD_OFFSET 0x1DA0BCu
#define TABLE_BYTES (THERON_TRACK02_EXPERIENCE_ENTRY_COUNT * 2u)
#define TABLE_CONTEXT_BYTES 32u

typedef struct Track02Media {
    uint8_t *raw;
    size_t raw_size;
    uint8_t *user_data;
    size_t user_data_size;
} Track02Media;

static const uint8_t k_table_context[TABLE_CONTEXT_BYTES] = {
    0x00u, 0x00u, 0x00u, 0x00u, 0x3Cu, 0x00u, 0x32u, 0x00u,
    0x00u, 0x01u, 0x00u, 0x01u, 0x00u, 0x01u, 0x00u, 0x01u,
    0x03u, 0x00u, 0x03u, 0x00u, 0x03u, 0x00u, 0x03u, 0x00u,
    0x00u, 0x00u, 0x0Au, 0x00u, 0x36u, 0x00u, 0x5Au, 0x00u,
};

static const char *media_path(const char *edition)
{
    const int is_us = strcmp(edition, "us") == 0;
    const char *override = getenv(is_us ? "FIRESTAFF_THERON_US_TRACK02_BIN"
                                        : "FIRESTAFF_THERON_JP_TRACK02_BIN");
    const char *home = getenv("HOME");
    static char path[1024];

    if (override && override[0]) return override;
    if (!home || !home[0]) return NULL;
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/theron/%s",
                 home, is_us ? "TQUS02.bin" : "TQJP02.bin") >=
        (int)sizeof(path)) return NULL;
    return path;
}

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *bytes;

    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes) {
        fclose(file);
        return 0;
    }
    if (fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)length;
    return 1;
}

static int extract_user_data(Track02Media *media)
{
    size_t sector_count;

    if (!media->raw || media->raw_size % RAW_SECTOR_BYTES != 0u) return 0;
    sector_count = media->raw_size / RAW_SECTOR_BYTES;
    if (sector_count > SIZE_MAX / USER_DATA_BYTES) return 0;
    media->user_data_size = sector_count * USER_DATA_BYTES;
    media->user_data = (uint8_t *)malloc(media->user_data_size);
    if (!media->user_data) return 0;
    for (size_t sector = 0u; sector < sector_count; ++sector) {
        memcpy(media->user_data + sector * USER_DATA_BYTES,
               media->raw + sector * RAW_SECTOR_BYTES + RAW_USER_DATA_OFFSET,
               USER_DATA_BYTES);
    }
    return 1;
}

static void free_media(Track02Media *media)
{
    free(media->user_data);
    free(media->raw);
    memset(media, 0, sizeof(*media));
}

static int load_media(const char *edition, Track02Media *media)
{
    const int is_us = strcmp(edition, "us") == 0;
    const char *path = media_path(edition);
    const char *override = getenv(is_us ? "FIRESTAFF_THERON_US_TRACK02_BIN"
                                        : "FIRESTAFF_THERON_JP_TRACK02_BIN");
    const char *expected_md5 = is_us ? THERON_V1_TRACK02_MD5_US_BIN
                                     : THERON_TRACK02_MD5_JP_BIN;

    memset(media, 0, sizeof(*media));
    if (!path || !read_file(path, &media->raw, &media->raw_size)) {
        if (override && override[0]) {
            fprintf(stderr, "FAIL: explicit %s Track 02 BIN cannot be read\n",
                    is_us ? "US" : "JP Rev. 1");
            return 1;
        }
        printf("SKIP: authentic %s Track 02 BIN is not available\n",
               is_us ? "US" : "JP Rev. 1");
        return 77;
    }
    if (!theron_v1_track02_raw_bytes_match_md5(
            media->raw, media->raw_size, expected_md5)) {
        fprintf(stderr,
                "FAIL: %s Track 02 BIN does not match the authenticated edition\n",
                is_us ? "US" : "JP Rev. 1");
        free_media(media);
        return 1;
    }
    if (!extract_user_data(media)) {
        fprintf(stderr, "FAIL: unsupported %s Track 02 sector layout\n",
                is_us ? "US" : "JP Rev. 1");
        free_media(media);
        return 1;
    }
    return 0;
}

static size_t table_offset(const char *edition)
{
    return strcmp(edition, "us") == 0 ? US_TABLE_UD_OFFSET
                                        : JP_TABLE_UD_OFFSET;
}

static int verify_table(const char *edition, const Track02Media *media)
{
    const size_t offset = table_offset(edition);

    if (offset < TABLE_CONTEXT_BYTES ||
        offset > media->user_data_size ||
        TABLE_BYTES > media->user_data_size - offset) {
        fprintf(stderr, "FAIL: %s experience-data span is out of bounds\n",
                strcmp(edition, "us") == 0 ? "US" : "JP Rev. 1");
        return 0;
    }
    if (memcmp(media->user_data + offset - TABLE_CONTEXT_BYTES,
               k_table_context, sizeof(k_table_context)) != 0) {
        fprintf(stderr, "FAIL: %s preceding data block does not match\n",
                strcmp(edition, "us") == 0 ? "US" : "JP Rev. 1");
        return 0;
    }
    for (unsigned int i = 0u; i < THERON_TRACK02_EXPERIENCE_ENTRY_COUNT; ++i) {
        const size_t entry_offset = offset + (size_t)i * 2u;
        const unsigned int actual =
            (unsigned int)media->user_data[entry_offset] |
            ((unsigned int)media->user_data[entry_offset + 1u] << 8u);
        const unsigned int expected =
            theron_v1_track02_us_experience_threshold(i);
        if (actual != expected) {
            fprintf(stderr,
                    "FAIL: %s 64-word source table differs at word %u\n",
                    strcmp(edition, "us") == 0 ? "US" : "JP Rev. 1", i);
            return 0;
        }
    }
    return 1;
}

static int verify_single_edition(const char *edition)
{
    Track02Media media;
    int status = load_media(edition, &media);

    if (status != 0) return status;
    if (!verify_table(edition, &media)) {
        free_media(&media);
        return 1;
    }
    printf("PASS: %s 64-word Track 02 data matches authenticated bytes\n",
           strcmp(edition, "us") == 0 ? "US" : "JP Rev. 1");
    free_media(&media);
    return 0;
}

static int verify_regional_identity(void)
{
    Track02Media us;
    Track02Media jp;
    int status = load_media("us", &us);

    if (status != 0) return status;
    status = load_media("jp", &jp);
    if (status != 0) {
        free_media(&us);
        return status;
    }
    if (!verify_table("us", &us) || !verify_table("jp", &jp) ||
        memcmp(us.user_data + US_TABLE_UD_OFFSET - TABLE_CONTEXT_BYTES,
               jp.user_data + JP_TABLE_UD_OFFSET - TABLE_CONTEXT_BYTES,
               TABLE_CONTEXT_BYTES + TABLE_BYTES) != 0) {
        fputs("FAIL: regional Track 02 data/context is not byte-identical\n",
              stderr);
        free_media(&jp);
        free_media(&us);
        return 1;
    }
    puts("PASS: US/JP 16-word context and 64-word data blocks are byte-identical");
    free_media(&jp);
    free_media(&us);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "us") == 0)
        return verify_single_edition("us");
    if (argc == 2 && strcmp(argv[1], "jp") == 0)
        return verify_single_edition("jp");
    if (argc == 2 && strcmp(argv[1], "identity") == 0)
        return verify_regional_identity();
    if (argc != 1) {
        fputs("usage: test_theron_v1_track02_experience_table_real_media "
              "[us|jp|identity]\n", stderr);
        return 2;
    }
    return 0;
}
