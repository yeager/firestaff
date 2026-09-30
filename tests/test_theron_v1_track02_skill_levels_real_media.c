#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_champion_strings.h"
#include "theron_v1_track02_skill_rank_source.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_BYTES 2048u
#define RAW_USER_DATA_OFFSET 16u
#define US_SKILL_LEVELS_UD_OFFSET 0x1C9B6Bu
#define JP_SKILL_LEVELS_UD_OFFSET 0x089333u

static const uint8_t k_master_rank_glyphs[6] = {
    0x60u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u
};

static const char *media_path(const char *edition)
{
    const char *override;
    const char *home = getenv("HOME");
    static char path[1024];

    if (strcmp(edition, "us") == 0)
        override = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    else
        override = getenv("FIRESTAFF_THERON_JP_TRACK02_BIN");
    if (override && override[0]) return override;
    if (!home || !home[0]) return NULL;
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/theron/%s",
                 home, strcmp(edition, "us") == 0 ? "TQUS02.bin" : "TQJP02.bin")
        >= (int)sizeof(path)) return NULL;
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

static int extract_user_data(const uint8_t *raw, size_t raw_size,
                             uint8_t **out, size_t *out_size)
{
    size_t sector_count;
    uint8_t *user_data;

    if (!raw || raw_size % RAW_SECTOR_BYTES != 0u) return 0;
    sector_count = raw_size / RAW_SECTOR_BYTES;
    if (sector_count > SIZE_MAX / USER_DATA_BYTES) return 0;
    *out_size = sector_count * USER_DATA_BYTES;
    user_data = (uint8_t *)malloc(*out_size);
    if (!user_data) return 0;
    for (size_t sector = 0u; sector < sector_count; ++sector) {
        memcpy(user_data + sector * USER_DATA_BYTES,
               raw + sector * RAW_SECTOR_BYTES + RAW_USER_DATA_OFFSET,
               USER_DATA_BYTES);
    }
    *out = user_data;
    return 1;
}

int main(int argc, char **argv)
{
    const int is_us = argc == 2 && strcmp(argv[1], "us") == 0;
    const int is_jp = argc == 2 && strcmp(argv[1], "jp") == 0;
    const char *edition = is_us ? "US" : "JP Rev. 1";
    const char *path;
    const char *expected_md5;
    const char *override;
    uint8_t *raw = NULL;
    uint8_t *user_data = NULL;
    size_t raw_size = 0u;
    size_t user_data_size = 0u;
    size_t cursor;
    int result = 1;

    if (!is_us && !is_jp) {
        fputs("usage: test_theron_v1_track02_skill_levels_real_media us|jp\n",
              stderr);
        return 2;
    }
    path = media_path(is_us ? "us" : "jp");
    override = getenv(is_us ? "FIRESTAFF_THERON_US_TRACK02_BIN"
                            : "FIRESTAFF_THERON_JP_TRACK02_BIN");
    expected_md5 = is_us ? THERON_V1_TRACK02_MD5_US_BIN
                         : THERON_TRACK02_MD5_JP_BIN;
    cursor = is_us ? US_SKILL_LEVELS_UD_OFFSET : JP_SKILL_LEVELS_UD_OFFSET;

    if (!path || !read_file(path, &raw, &raw_size)) {
        if (override && override[0]) {
            fprintf(stderr, "FAIL: explicit %s Track 02 BIN cannot be read\n",
                    edition);
            return 1;
        }
        printf("SKIP: authentic %s Track 02 BIN is not available\n", edition);
        return 77;
    }
    if (!theron_v1_track02_raw_bytes_match_md5(
            raw, raw_size, expected_md5)) {
        fprintf(stderr, "FAIL: %s Track 02 BIN does not match the authenticated edition\n",
                edition);
        goto cleanup;
    }
    if (!extract_user_data(raw, raw_size, &user_data, &user_data_size) ||
        cursor >= user_data_size) {
        fprintf(stderr, "FAIL: unsupported %s Track 02 sector layout\n", edition);
        goto cleanup;
    }

    for (unsigned int rank = 0u;
         rank < THERON_TRACK02_SKILL_LEVEL_COUNT; ++rank) {
        const uint8_t *terminator;
        size_t source_size;

        if (cursor >= user_data_size ||
            !(terminator = (const uint8_t *)memchr(
                  user_data + cursor, 0, user_data_size - cursor))) {
            fprintf(stderr, "FAIL: missing %s skill-rank source record %u\n",
                    edition,
                    rank);
            goto cleanup;
        }
        source_size = (size_t)(terminator - (user_data + cursor));
        if (is_us) {
            const char *display =
                theron_v1_track02_us_skill_level_name(rank);
            if (!display) {
                fprintf(stderr, "FAIL: missing US rank name %u\n", rank);
                goto cleanup;
            }
            if (rank >= 8u && rank <= 13u) {
                size_t text_size = strlen("MASTER");
                if (source_size != text_size + 2u ||
                    user_data[cursor] != k_master_rank_glyphs[rank - 8u] ||
                    user_data[cursor + 1u] != (uint8_t)' ' ||
                    memcmp(user_data + cursor + 2u, "MASTER", text_size) != 0 ||
                    strcmp(display, "MASTER") != 0) {
                    fprintf(stderr,
                            "FAIL: US skill-rank source/glyph binding %u\n",
                            rank);
                    goto cleanup;
                }
            } else if (source_size != strlen(display) ||
                       memcmp(user_data + cursor, display, source_size) != 0) {
                fprintf(stderr, "FAIL: US skill-rank source text binding %u\n",
                        rank);
                goto cleanup;
            }
        } else {
            const uint8_t *record = NULL;
            size_t record_size = 0u;
            if (!theron_v1_track02_jp_skill_level_source_record(
                    rank, &record, &record_size) ||
                source_size != record_size ||
                memcmp(user_data + cursor, record, record_size) != 0) {
                fprintf(stderr,
                        "FAIL: JP raw skill-rank record binding %u\n", rank);
                goto cleanup;
            }
            if (rank >= 8u && rank <= 13u &&
                (record_size != sizeof("\x60 MASTER") - 1u ||
                 record[0] != k_master_rank_glyphs[rank - 8u])) {
                fprintf(stderr,
                        "FAIL: JP custom skill-rank glyph binding %u\n", rank);
                goto cleanup;
            }
        }
        cursor += source_size + 1u;
    }

    printf("PASS: %s skill-rank source records match retail bytes\n",
           edition);
    result = 0;

cleanup:
    free(user_data);
    free(raw);
    return result;
}
