#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset_status_m12.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_font_glyphs.h"

static int verify_region(const char *region, const char *environment_name,
                         const char *expected_md5,
                         size_t expected_user_data_offset) {
    const char *path = getenv(environment_name);
    char observed_md5[33];
    FILE *file;
    long file_size;
    size_t data_size;
    uint8_t *data;
    uint8_t source_glyphs[THERON_TRACK02_FONT_GLYPH_COUNT *
                          THERON_TRACK02_FONT_BYTES_PER_GLYPH];
    size_t source_user_data_offset = 0u;
    const size_t source_sector = expected_user_data_offset /
        THERON_TRACK02_RAW_USER_DATA_BYTES;
    const size_t source_within_sector = expected_user_data_offset %
        THERON_TRACK02_RAW_USER_DATA_BYTES;
    const size_t source_raw_offset = source_sector *
        THERON_TRACK02_RAW_SECTOR_BYTES +
        THERON_TRACK02_RAW_USER_DATA_OFFSET + source_within_sector;

    if (!path || !path[0]) {
        printf("SKIP: %s is not configured\n", environment_name);
        return 77;
    }
    file = fopen(path, "rb");
    if (!file) {
        printf("SKIP: authentic %s Track 02 media is unavailable\n", region);
        return 77;
    }
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        fprintf(stderr, "FAIL: could not size configured Track 02 media\n");
        return 1;
    }
    data_size = (size_t)file_size;
    if (data_size > 32u * 1024u * 1024u) {
        fclose(file);
        fprintf(stderr, "FAIL: configured Track 02 media exceeds test bound\n");
        return 1;
    }
    if (!m12_file_md5_hex(path, observed_md5) ||
        strcmp(observed_md5, expected_md5) != 0) {
        fclose(file);
        fprintf(stderr, "FAIL: configured media is not authentic %s Track 02\n",
                region);
        return 1;
    }
    data = (uint8_t *)malloc(data_size);
    if (!data || fread(data, 1u, data_size, file) != data_size) {
        free(data);
        fclose(file);
        fprintf(stderr, "FAIL: could not read hash-verified Track 02 media\n");
        return 1;
    }
    fclose(file);

    if (theron_v1_track02_copy_raw_user_data_range(
            data, data_size, expected_md5, source_raw_offset,
            sizeof(source_glyphs), source_glyphs, sizeof(source_glyphs),
            &source_user_data_offset) != THERON_TRACK02_SIGNAL_OK ||
        source_user_data_offset != expected_user_data_offset) {
        free(data);
        fprintf(stderr, "FAIL: could not read the %s source-bound glyph span\n",
                region);
        return 1;
    }
    free(data);

    for (size_t i = 0u; i < THERON_TRACK02_FONT_GLYPH_COUNT; ++i) {
        const uint8_t *glyph = theron_v1_track02_font_glyph((unsigned int)i);
        if (!glyph ||
            memcmp(glyph,
                   source_glyphs +
                       i * THERON_TRACK02_FONT_BYTES_PER_GLYPH,
                   THERON_TRACK02_FONT_BYTES_PER_GLYPH) != 0) {
            fprintf(stderr,
                    "FAIL: checked-in glyph %zu differs from %s Track 02\n",
                    i, region);
            return 1;
        }
    }

    printf("PASS: all %u glyphs match hash-verified %s Track 02 UD 0x%06zx\n",
           THERON_TRACK02_FONT_GLYPH_COUNT, region,
           expected_user_data_offset);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "FAIL: select exactly one font media region (US or JP)\n");
        return 2;
    }
    if (strcmp(argv[1], "us") == 0) {
        return verify_region(
            "US", "THERON_TRACK02_US_BIN", THERON_TRACK02_MD5_US_BIN,
            0x09A000u);
    }
    if (strcmp(argv[1], "jp") == 0) {
        return verify_region(
            "JP Rev. 1", "THERON_TRACK02_JP_BIN",
            THERON_TRACK02_MD5_JP_BIN, 0x099800u);
    }
    fprintf(stderr, "FAIL: unsupported font media region '%s'\n", argv[1]);
    return 2;
}
