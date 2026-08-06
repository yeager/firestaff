#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    PALETTE_OFFSET = 16u,
    ORIGINAL_TRACE_PALETTE_RAW_OFFSET_US = 0x2a06a0u,
    ORIGINAL_TRACE_PALETTE_RAW_OFFSET_JP = 0x29fd70u
};

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                          const uint8_t *data,
                                          int data_size,
                                          int dungeon_id,
                                          int sub_level_index) {
    (void)level;
    (void)data;
    (void)data_size;
    (void)dungeon_id;
    (void)sub_level_index;
    return THERON_MAP_ERR_NULL;
}

void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world) {
    (void)world;
}

static void write_palette(uint8_t *bytes) {
    size_t i;

    for (i = 0u; i < THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT; ++i) {
        const uint16_t raw_word = (uint16_t)(i & 0x01ffu);
        bytes[i * 2u] = (uint8_t)raw_word;
        bytes[i * 2u + 1u] = (uint8_t)(raw_word >> 8u);
    }
}

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

static const char *default_track02_path(const char *filename,
                                        char *path,
                                        size_t path_size) {
    const char *home = getenv("HOME");

    if (!home || !filename || !path || path_size == 0u) return NULL;
    if (snprintf(path, path_size, "%s/.firestaff/data/theron/%s", home,
                 filename) >= (int)path_size) {
        return NULL;
    }
    return path;
}

static void check_real_palette(const char *path,
                               const char *md5,
                               Theron_Track02Variant expected_variant,
                               size_t raw_offset) {
    uint8_t *track02;
    size_t track02_bytes = 0u;
    Theron_Track02PaletteWindowEvidence evidence;

    track02 = read_file(path, &track02_bytes);
    if (!track02) return;
    CHECK(theron_v1_track02_variant_for_md5(md5) == expected_variant);
    {
        const Theron_Track02SignalStatus status =
            theron_v1_track02_inspect_4bpp_palette_window(
                track02, track02_bytes, md5, raw_offset,
                &evidence);
        if (status != THERON_TRACK02_SIGNAL_OK) {
            fprintf(stderr, "palette status=%s path=%s bytes=%zu\n",
                    theron_v1_track02_signal_status_name(status), path,
                    track02_bytes);
        }
        CHECK(status == THERON_TRACK02_SIGNAL_OK);
    }
    CHECK(evidence.variant == expected_variant);
    CHECK(evidence.raw_offset == raw_offset);
    CHECK(evidence.raw_offset_is_user_data);
    CHECK(evidence.format_valid);
    CHECK(evidence.palette.valid);
    CHECK(evidence.palette.nonblack_entry_count > 0u);
    CHECK(!evidence.semantic_binding_verified);
    CHECK(!evidence.promotion_allowed);
    CHECK(!theron_v1_track02_palette_window_evidence_can_promote(&evidence));
    free(track02);
}

/* Keep the old caller-supplied offset test on the US media path explicit. */
static void check_env_palette(const char *path, const char *md5) {
    uint8_t *track02;
    size_t track02_bytes = 0u;
    Theron_Track02PaletteWindowEvidence evidence;

    track02 = read_file(path, &track02_bytes);
    if (track02 && md5) {
        const Theron_Track02Variant variant =
            theron_v1_track02_variant_for_md5(md5);
        const Theron_Track02SignalStatus status =
            theron_v1_track02_inspect_4bpp_palette_window(
                track02, track02_bytes, md5,
                ORIGINAL_TRACE_PALETTE_RAW_OFFSET_US, &evidence);

        if (variant == THERON_TRACK02_VARIANT_US_BIN ||
            variant == THERON_TRACK02_VARIANT_JP_BIN) {
            if (status == THERON_TRACK02_SIGNAL_OK) {
                CHECK(evidence.variant == variant);
                CHECK(evidence.raw_offset == ORIGINAL_TRACE_PALETTE_RAW_OFFSET_US);
                CHECK(evidence.raw_offset_is_user_data);
                CHECK(evidence.format_valid);
                CHECK(!evidence.semantic_binding_verified);
                CHECK(!evidence.promotion_allowed);
                CHECK(!theron_v1_track02_palette_window_evidence_can_promote(
                    &evidence));
            } else {
                CHECK(!evidence.format_valid);
                CHECK(!evidence.promotion_allowed);
            }
        }
    }
    free(track02);
}

int main(void) {
    uint8_t iso_fixture[PALETTE_OFFSET + THERON_TRACK02_4BPP_PALETTE_BYTES] = {0};
    Theron_Track02PaletteWindowEvidence evidence;
    const char *real_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *real_md5 = getenv("FIRESTAFF_THERON_TRACK02_RAW_MD5");
    char us_path[4096];
    char jp_path[4096];

    write_palette(iso_fixture + PALETTE_OFFSET);
    CHECK(theron_v1_track02_inspect_4bpp_palette_window(
              iso_fixture, sizeof(iso_fixture), THERON_TRACK02_MD5_US_ISO,
              PALETTE_OFFSET, &evidence) == THERON_TRACK02_SIGNAL_OK);
    CHECK(evidence.variant == THERON_TRACK02_VARIANT_US_ISO);
    CHECK(evidence.raw_offset == PALETTE_OFFSET);
    CHECK(evidence.user_data_offset == PALETTE_OFFSET);
    CHECK(evidence.byte_count == THERON_TRACK02_4BPP_PALETTE_BYTES);
    CHECK(evidence.raw_offset_is_user_data);
    CHECK(evidence.format_valid);
    CHECK(evidence.palette.valid);
    CHECK(evidence.palette.nonblack_entry_count == 15u);
    CHECK(!evidence.semantic_binding_verified);
    CHECK(!evidence.promotion_allowed);
    CHECK(!theron_v1_track02_palette_window_evidence_can_promote(&evidence));

    iso_fixture[PALETTE_OFFSET + 1u] = 0xfeu;
    CHECK(theron_v1_track02_inspect_4bpp_palette_window(
              iso_fixture, sizeof(iso_fixture), THERON_TRACK02_MD5_US_ISO,
              PALETTE_OFFSET, &evidence) == THERON_TRACK02_SIGNAL_NOT_FOUND);
    CHECK(evidence.variant == THERON_TRACK02_VARIANT_UNKNOWN);
    CHECK(evidence.raw_offset == 0u);
    CHECK(evidence.byte_count == 0u);
    CHECK(!evidence.format_valid);
    CHECK(!evidence.palette.valid);
    CHECK(!theron_v1_track02_palette_window_evidence_can_promote(&evidence));

    check_env_palette(real_path, real_md5);

    check_real_palette(
        default_track02_path("TQUS02.bin", us_path, sizeof(us_path)),
        THERON_TRACK02_MD5_US_BIN, THERON_TRACK02_VARIANT_US_BIN,
        ORIGINAL_TRACE_PALETTE_RAW_OFFSET_US);
    check_real_palette(
        default_track02_path("TQJP02.bin", jp_path, sizeof(jp_path)),
        THERON_TRACK02_MD5_JP_BIN, THERON_TRACK02_VARIANT_JP_BIN,
        ORIGINAL_TRACE_PALETTE_RAW_OFFSET_JP);

    printf("test_theron_v1_track02_palette_window: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
