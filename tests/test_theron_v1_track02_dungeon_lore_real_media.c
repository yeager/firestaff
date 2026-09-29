#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_track02_dungeon_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_BYTES 2048u
#define RAW_USER_DATA_OFFSET 16u

static const size_t k_story_offsets[THERON_TRACK02_DUNGEON_COUNT] = {
    0x27613Eu, 0x2762C4u, 0x276432u, 0x276665u,
    0x2767E2u, 0x276958u, 0x276B43u
};

static const size_t k_story_lengths[THERON_TRACK02_DUNGEON_COUNT] = {
    388u, 364u, 561u, 379u, 372u, 489u, 392u
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
    FILE *file;
    long length;
    uint8_t *bytes;

    file = fopen(path, "rb");
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

static int make_user_data(const uint8_t *raw, size_t raw_size,
                          uint8_t **out, size_t *out_size)
{
    uint8_t *user_data;
    size_t sector_count;

    if (!raw || !out || !out_size) return 0;
    if (raw_size % RAW_SECTOR_BYTES != 0u) return 0;
    sector_count = raw_size / RAW_SECTOR_BYTES;
    if (sector_count > SIZE_MAX / USER_DATA_BYTES) return 0;
    *out_size = sector_count * USER_DATA_BYTES;
    user_data = (uint8_t *)malloc(*out_size);
    if (!user_data) return 0;
    for (size_t i = 0u; i < sector_count; ++i) {
        memcpy(user_data + i * USER_DATA_BYTES,
               raw + i * RAW_SECTOR_BYTES + RAW_USER_DATA_OFFSET,
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
    int result = 1;

    if (!path || !read_file(path, &raw, &raw_size)) {
        puts("SKIP: authentic US Track 02 BIN is not available");
        return 77;
    }
    if (!theron_v1_track02_raw_bytes_match_md5(
            raw, raw_size, "f23601102138f87c33025877767ebf76")) {
        fputs("FAIL: US Track 02 BIN does not match the authenticated edition\n",
              stderr);
        goto cleanup;
    }
    if (!make_user_data(raw, raw_size, &user_data, &user_data_size)) {
        fputs("FAIL: authentic US Track 02 BIN has an unsupported sector layout\n",
              stderr);
        goto cleanup;
    }

    for (unsigned int i = 0u; i < THERON_TRACK02_DUNGEON_COUNT; ++i) {
        const char *story = theron_v1_track02_us_dungeon_story(i);
        size_t story_size = story ? strlen(story) : 0u;
        int source_record_in_bounds =
            k_story_offsets[i] <= user_data_size &&
            k_story_lengths[i] <= user_data_size - k_story_offsets[i];
        size_t mismatch = 0u;
        if (story && source_record_in_bounds) {
            while (mismatch < story_size &&
                   mismatch < k_story_lengths[i] &&
                   (unsigned char)story[mismatch] ==
                       user_data[k_story_offsets[i] + mismatch]) {
                ++mismatch;
            }
        }
        if (!story || story_size != k_story_lengths[i] ||
            !source_record_in_bounds ||
            memcmp(story, user_data + k_story_offsets[i],
                   k_story_lengths[i]) != 0) {
            fprintf(stderr,
                    "FAIL: US dungeon story %u differs from authenticated "
                    "Track 02 bytes at UD 0x%zX (literal=%zu, source=%zu, "
                    "first mismatch=%zu)\n",
                    i, k_story_offsets[i], story_size, k_story_lengths[i],
                    mismatch);
            if (story && source_record_in_bounds &&
                mismatch < k_story_lengths[i]) {
                size_t begin = mismatch > 6u ? mismatch - 6u : 0u;
                size_t end = mismatch + 6u;
                if (end > k_story_lengths[i]) end = k_story_lengths[i];
                fputs("  literal/source bytes:", stderr);
                for (size_t j = begin; j < end; ++j) {
                    fprintf(stderr, " %02x/%02x",
                            j < story_size ? (unsigned char)story[j] : 0u,
                            user_data[k_story_offsets[i] + j]);
                }
                fputc('\n', stderr);
            }
            goto cleanup;
        }
    }
    puts("PASS: all seven US dungeon stories match authenticated Track 02 bytes");
    result = 0;

cleanup:
    free(user_data);
    free(raw);
    return result;
}
