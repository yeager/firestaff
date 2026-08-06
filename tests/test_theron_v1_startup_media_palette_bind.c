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

static const char *default_track02_path(
    const char *leaf,
    char *out,
    size_t cap) {
    const char *home = getenv("HOME");
    if (!leaf || !out || cap == 0u) return NULL;
    if (home && home[0] != '\0') {
        (void)snprintf(out, cap, "%s/.firestaff/data/theron/%s", home, leaf);
        return out;
    }
    return NULL;
}

static void check_real_palette(
    const char *path,
    const char *md5,
    const char *label) {
    Theron_V1_World world;
    uint8_t *track02;
    size_t track02_bytes = 0u;

    track02 = read_file(path, &track02_bytes);
    if (!track02) {
        fprintf(stderr, "skipping %s (missing %s)\n", label, path ? path : "path");
        return;
    }
    memset(&world, 0, sizeof(world));
    CHECK(theron_v1_startup_media_bind_runtime_palette(
        &world, track02, track02_bytes, md5));
    CHECK(world.runtime_media.startup_palette_valid);
    CHECK(world.runtime_media.startup_palette_rgb8[0][0] == 0);
    CHECK(world.runtime_media.startup_palette_rgb8[0][1] == 0);
    CHECK(world.runtime_media.startup_palette_rgb8[0][2] == 0);
    CHECK(world.runtime_media.startup_palette_rgb8[15][0] == 255);
    CHECK(world.runtime_media.startup_palette_rgb8[15][1] == 255);
    CHECK(world.runtime_media.startup_palette_rgb8[15][2] == 255);
    fprintf(stderr, "%s palette bind from real Track 02 OK\n", label);
    free(track02);
}

static void check_real_jp_roster(const char *path, const char *md5) {
    Theron_StartupMediaStateReceipt receipt;
    uint8_t *track02;
    size_t track02_bytes = 0u;
    static const char *const names[] = {
        "THERON", "MARA", "LINOS", "HEXA",
        "HAKAR", "TIRAN", "DOTAN", "PENTAI"
    };
    static const char *const titles[] = {
        "", "GUARDIAN OF WISDO", "THE RESOLUTE", "LORD OF FEALTY",
        "THE BRAVE", "KNIGHT OF STRENGT", "MASTER OF THE WIN",
        "THE SURVIVOR"
    };
    int i;

    track02 = read_file(path, &track02_bytes);
    if (!track02) {
        fprintf(stderr, "skipping JP roster (missing %s)\n", path);
        return;
    }
    memset(&receipt, 0, sizeof(receipt));
    theron_v1_startup_media_capture_track02_state_receipt(
        track02, track02_bytes, md5, &receipt);
    CHECK(receipt.track02_variant == THERON_TRACK02_VARIANT_JP_BIN);
    CHECK(receipt.startup_roster_name_status == THERON_TRACK02_SIGNAL_OK);
    CHECK(receipt.startup_roster_name_count == 8);
    for (i = 0; i < 8; ++i) {
        CHECK(strcmp(receipt.startup_roster_names[i], names[i]) == 0);
        CHECK(strcmp(receipt.startup_roster_titles[i], titles[i]) == 0);
    }
    fprintf(stderr,
            "JP Track 02 roster bind from real media OK: %d names/titles\n",
            receipt.startup_roster_name_count);
    free(track02);
}

int main(void) {
    Theron_V1_World world;
    const char *real_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *real_md5 = getenv("FIRESTAFF_THERON_TRACK02_RAW_MD5");
    uint8_t *real_track02;
    size_t real_track02_bytes;
    char us_path[4096];
    char jp_path[4096];

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
            check_real_palette(real_path, real_md5, "environment");
        }
        free(real_track02);
    } else {
        fprintf(stderr, "skipping real Track 02 test (no FIRESTAFF_THERON_TRACK02_RAW)\n");
    }

    if (default_track02_path("TQUS02.bin", us_path, sizeof(us_path))) {
        check_real_palette(us_path, THERON_TRACK02_MD5_US_BIN, "US");
    }
    if (default_track02_path("TQJP02.bin", jp_path, sizeof(jp_path))) {
        check_real_palette(jp_path, THERON_TRACK02_MD5_JP_BIN, "JP");
        check_real_jp_roster(jp_path, THERON_TRACK02_MD5_JP_BIN);
    }

    fprintf(stderr, "Theron startup media palette bind: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
