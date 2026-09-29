#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_champion_strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_BYTES 2048u
#define RAW_USER_DATA_OFFSET 16u
#define US_SKILL_LEVELS_UD_OFFSET 0x1C9B6Bu

static const uint8_t k_master_rank_glyphs[6] = {
    0x60u, 0x61u, 0x62u, 0x63u, 0x64u, 0x65u
};

static const char *media_path(void)
{
    const char *override = getenv("FIRESTAFF_THERON_US_TRACK02_BIN");
    const char *home = getenv("HOME");
    static char path[1024];

    if (override && override[0]) return override;
    if (!home || !home[0]) return NULL;
    if (snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin",
                 home) >= (int)sizeof(path)) return NULL;
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

int main(void)
{
    const char *path = media_path();
    uint8_t *raw = NULL;
    uint8_t *user_data = NULL;
    size_t raw_size = 0u;
    size_t user_data_size = 0u;
    size_t cursor = US_SKILL_LEVELS_UD_OFFSET;
    int result = 1;

    if (!path || !read_file(path, &raw, &raw_size)) {
        puts("SKIP: authentic US Track 02 BIN is not available");
        return 77;
    }
    if (!theron_v1_track02_raw_bytes_match_md5(
            raw, raw_size, THERON_V1_TRACK02_MD5_US_BIN)) {
        fputs("FAIL: US Track 02 BIN does not match the authenticated edition\n",
              stderr);
        goto cleanup;
    }
    if (!extract_user_data(raw, raw_size, &user_data, &user_data_size) ||
        cursor >= user_data_size) {
        fputs("FAIL: unsupported US Track 02 sector layout\n", stderr);
        goto cleanup;
    }

    for (unsigned int rank = 0u;
         rank < THERON_TRACK02_SKILL_LEVEL_COUNT; ++rank) {
        const char *display = theron_v1_track02_us_skill_level_name(rank);
        const uint8_t *terminator;
        size_t source_size;

        if (!display || cursor >= user_data_size ||
            !(terminator = (const uint8_t *)memchr(
                  user_data + cursor, 0, user_data_size - cursor))) {
            fprintf(stderr, "FAIL: missing US skill-rank source record %u\n",
                    rank);
            goto cleanup;
        }
        source_size = (size_t)(terminator - (user_data + cursor));
        if (rank >= 8u && rank <= 13u) {
            size_t text_size = strlen("MASTER");
            if (source_size != text_size + 2u ||
                user_data[cursor] != k_master_rank_glyphs[rank - 8u] ||
                user_data[cursor + 1u] != (uint8_t)' ' ||
                memcmp(user_data + cursor + 2u, "MASTER", text_size) != 0 ||
                strcmp(display, "MASTER") != 0) {
                fprintf(stderr,
                        "FAIL: US skill-rank source/glyph binding %u\n", rank);
                goto cleanup;
            }
        } else if (source_size != strlen(display) ||
                   memcmp(user_data + cursor, display, source_size) != 0) {
            fprintf(stderr, "FAIL: US skill-rank source text binding %u\n",
                    rank);
            goto cleanup;
        }
        cursor += source_size + 1u;
    }

    puts("PASS: US skill-rank names and custom glyph records match retail bytes");
    result = 0;

cleanup:
    free(user_data);
    free(raw);
    return result;
}
