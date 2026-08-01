#include "theron_v1_startup_media.h"
#include "theron_v1_world.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static uint8_t *read_file(const char *path, size_t *out_bytes) {
    FILE *file;
    long bytes;
    uint8_t *data;

    if (!path || !out_bytes || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (bytes = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = malloc((size_t)bytes);
    if (!data || fread(data, 1u, (size_t)bytes, file) != (size_t)bytes) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_bytes = (size_t)bytes;
    return data;
}

int main(void) {
    Theron_V1_World world;
    const char *real_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *real_md5 = getenv("FIRESTAFF_THERON_TRACK02_RAW_MD5");
    uint8_t *real_track02;
    size_t real_track02_bytes;

    memset(&world, 0, sizeof(world));
    CHECK(!theron_v1_startup_media_bind_runtime_palette(NULL, NULL, 0, NULL));
    CHECK(!theron_v1_startup_media_bind_runtime_palette(&world, NULL, 0, NULL));
    CHECK(!world.runtime_media.startup_palette_valid);

    real_track02 = read_file(real_path, &real_track02_bytes);
    if (real_track02 && real_md5) {
        const Theron_Track02Variant variant =
            theron_v1_track02_variant_for_md5(real_md5);

        if (variant == THERON_TRACK02_VARIANT_US_BIN ||
            variant == THERON_TRACK02_VARIANT_JP_BIN) {
            memset(&world, 0, sizeof(world));
            CHECK(theron_v1_startup_media_bind_runtime_palette(
                &world, real_track02, real_track02_bytes, real_md5));
            CHECK(world.runtime_media.startup_palette_valid);
            CHECK(world.runtime_media.startup_palette_rgb8[0][0] == 0);
            CHECK(world.runtime_media.startup_palette_rgb8[0][1] == 0);
            CHECK(world.runtime_media.startup_palette_rgb8[0][2] == 0);
            CHECK(world.runtime_media.startup_palette_rgb8[15][0] == 255);
            CHECK(world.runtime_media.startup_palette_rgb8[15][1] == 255);
            CHECK(world.runtime_media.startup_palette_rgb8[15][2] == 255);
            fprintf(stderr, "palette bind from real Track 02 OK\n");
        }
        free(real_track02);
    } else {
        fprintf(stderr, "skipping real Track 02 test (no FIRESTAFF_THERON_TRACK02_RAW)\n");
    }

    fprintf(stderr, "Theron startup media palette bind: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
