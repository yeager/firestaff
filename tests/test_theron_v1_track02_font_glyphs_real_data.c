#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asset_status_m12.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_font_glyphs.h"

#define THERON_US_TRACK02_GLYPH_UD_OFFSET 0x09A000u

int main(void) {
    const char *path = getenv("THERON_TRACK02_US_BIN");
    char observed_md5[33];
    FILE *file;
    long file_size;
    size_t data_size;
    uint8_t *data;
    uint8_t source_glyphs[THERON_TRACK02_FONT_GLYPH_COUNT *
                          THERON_TRACK02_FONT_BYTES_PER_GLYPH];
    size_t source_user_data_offset = 0u;
    const size_t source_sector =
        THERON_US_TRACK02_GLYPH_UD_OFFSET /
        THERON_TRACK02_RAW_USER_DATA_BYTES;
    const size_t source_within_sector =
        THERON_US_TRACK02_GLYPH_UD_OFFSET %
        THERON_TRACK02_RAW_USER_DATA_BYTES;
    const size_t source_raw_offset =
        source_sector * THERON_TRACK02_RAW_SECTOR_BYTES +
        THERON_TRACK02_RAW_USER_DATA_OFFSET + source_within_sector;
    size_t i;

    if (!path || !path[0]) {
        printf("SKIP: THERON_TRACK02_US_BIN is not configured\n");
        return 77;
    }
    file = fopen(path, "rb");
    if (!file) {
        printf("SKIP: authentic US Track 02 media is unavailable\n");
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
        strcmp(observed_md5, THERON_TRACK02_MD5_US_BIN) != 0) {
        fclose(file);
        fprintf(stderr, "FAIL: configured media is not authentic US Track 02\n");
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
            data, data_size, THERON_TRACK02_MD5_US_BIN,
            source_raw_offset, sizeof(source_glyphs),
            source_glyphs, sizeof(source_glyphs),
            &source_user_data_offset) != THERON_TRACK02_SIGNAL_OK ||
        source_user_data_offset != THERON_US_TRACK02_GLYPH_UD_OFFSET) {
        free(data);
        fprintf(stderr, "FAIL: could not read the source-bound glyph span\n");
        return 1;
    }
    free(data);

    for (i = 0u; i < THERON_TRACK02_FONT_GLYPH_COUNT; ++i) {
        const uint8_t *glyph = theron_v1_track02_font_glyph((unsigned int)i);
        if (!glyph ||
            memcmp(glyph,
                   source_glyphs +
                       i * THERON_TRACK02_FONT_BYTES_PER_GLYPH,
                   THERON_TRACK02_FONT_BYTES_PER_GLYPH) != 0) {
            fprintf(stderr,
                    "FAIL: checked-in glyph %zu differs from US Track 02\n",
                    i);
            return 1;
        }
    }

    printf("PASS: all %u glyphs match hash-verified US Track 02 UD 0x09A000\n",
           THERON_TRACK02_FONT_GLYPH_COUNT);
    return 0;
}
