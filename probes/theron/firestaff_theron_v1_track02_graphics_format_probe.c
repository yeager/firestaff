/* Hash-gated Track 02 graphics-format reconnaissance probe.
 *
 * The synthetic path exercises only scanner rules.  When staged JP/US media
 * is available, the optional path prints the concrete candidate catalog but
 * deliberately makes no decoder or rendering claim from it.
 */

#include "asset_status_m12.h"
#include "asset_find_by_hash.h"
#include "firestaff_theron_media_classify.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_SECTOR_BYTES 2352u
#define USER_DATA_OFFSET 16u
#define FIXTURE_SECTORS 3u

static int g_failures = 0;

static void check(int condition, const char *label) {
    if (!condition) {
        printf("FAIL %s\n", label);
        ++g_failures;
    }
}

static void put_le16(uint8_t *p, uint16_t value) {
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8u);
}

static void build_pair(uint8_t *jp, uint8_t *us, size_t byte_count) {
    uint8_t *jp_data;
    uint8_t *us_data;
    size_t i;

    memset(jp, 0, byte_count);
    memset(us, 0, byte_count);
    jp_data = jp + USER_DATA_OFFSET;
    us_data = us + RAW_SECTOR_BYTES + USER_DATA_OFFSET;

    /* Strict 16-word HuC6260 shape: index 0 black, 15 distinct colours. */
    for (i = 0u; i < THERON_TRACK02_4BPP_PALETTE_ENTRY_COUNT; ++i) {
        put_le16(jp_data + 0x40u + i * 2u, (uint16_t)(i * 0x0011u));
    }
    /* Eight-entry LE table with a 0x400-byte stride. */
    for (i = 0u; i < 8u; ++i) {
        put_le16(jp_data + 0x100u + i * 2u, (uint16_t)(0x0020u + i * 0x0400u));
    }
    memcpy(us_data, jp_data, THERON_TRACK02_RAW_USER_DATA_BYTES);
}

static int has_format(const Theron_Track02GraphicsFormatCatalog *catalog,
                      Theron_Track02GraphicsFormat format) {
    size_t i;
    for (i = 0u; i < catalog->candidate_count; ++i) {
        if (catalog->candidates[i].format == format) return 1;
    }
    return 0;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size) {
    FILE *file;
    long length;
    uint8_t *data;
    *out_data = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)length;
    return 1;
}

static const char *resolve_track02_path(const char *env_name,
                                        const char *file_name,
                                        char *fallback,
                                        size_t fallback_size) {
    const char *value = getenv(env_name);
    const char *home = getenv("HOME");
    if (value && value[0]) return value;
    if (!home || !home[0] || !fallback || fallback_size == 0u) return NULL;
    if (snprintf(fallback, fallback_size,
                 "%s/.firestaff/data/theron/%s", home, file_name) < 0) {
        return NULL;
    }
    return fallback;
}

static const char *resolve_clonecd_path(char *fallback, size_t fallback_size) {
    const char *value = getenv("FIRESTAFF_THERON_US_CLONECD_ZIP");
    const char *home = getenv("HOME");
    if (value && value[0]) return value;
    if (!home || !home[0] || !fallback || fallback_size == 0u) return NULL;
    if (snprintf(fallback, fallback_size,
                 "%s/.firestaff/data/theron/Dungeon-Master-Therons-Quest_TurboGrafx-CD_EN.zip",
                 home) < 0) return NULL;
    return fallback;
}

static void probe_real_media(void) {
    char jp_fallback[512];
    char us_fallback[512];
    char clonecd_fallback[512];
    const char *jp_path = resolve_track02_path(
        "FIRESTAFF_THERON_TRACK02_JP_BIN",
        "Dungeon Master - Theron's Quest (Japan) (Rev 1) (Track 02).bin",
        jp_fallback, sizeof(jp_fallback));
    const char *us_path = resolve_track02_path(
        "FIRESTAFF_THERON_TRACK02_US_BIN", "TQUS02.bin",
        us_fallback, sizeof(us_fallback));
    const char *clonecd_path = resolve_clonecd_path(
        clonecd_fallback, sizeof(clonecd_fallback));
    uint8_t *jp = NULL;
    uint8_t *us = NULL;
    size_t jp_size = 0u;
    size_t us_size = 0u;
    char jp_md5[33];
    char us_md5[33];
    Theron_Track02GraphicsFormatCatalog catalog;
    size_t i;

    if (jp_path && us_path && read_file(jp_path, &jp, &jp_size) &&
        read_file(us_path, &us, &us_size) && m12_file_md5_hex(jp_path, jp_md5) &&
        m12_file_md5_hex(us_path, us_md5)) {
        /* Existing loose JP/US comparison path. */
    } else {
        FirestaffTheronMediaStatus clonecd;
        free(jp);
        free(us);
        jp = NULL;
        us = NULL;
        jp_size = 0u;
        us_size = 0u;
        if (!jp_path || !clonecd_path || !read_file(jp_path, &jp, &jp_size) ||
            !m12_file_md5_hex(jp_path, jp_md5) ||
            FirestaffTheronMedia_ClassifyPath(clonecd_path, &clonecd) != 0 ||
            !clonecd.track02_path[0] ||
            !asset_read_path_alloc(clonecd.track02_path, &us, &us_size) ||
            !m12_file_md5_hex(clonecd.track02_path, us_md5)) {
            printf("SKIP real JP/US Track 02 scan: no verified loose US BIN or CloneCD Track 02 slice\n");
            free(jp);
            free(us);
            return;
        }
    }
    if (!jp || !us) {
        free(jp);
        free(us);
        return;
    }
    check(strcmp(jp_md5, THERON_TRACK02_MD5_JP_BIN) == 0 &&
              (strcmp(us_md5, THERON_TRACK02_MD5_US_BIN) == 0 ||
               strcmp(us_md5, THERON_TRACK02_MD5_US_CLONECD_BIN) == 0),
          "real media hashes are the verified JP/US or JP/CloneCD pair");
    if (g_failures == 0) {
        check(theron_v1_track02_catalog_graphics_format_candidates(
                  jp, jp_size, jp_md5, us, us_size, us_md5, &catalog) ==
                  THERON_TRACK02_SIGNAL_OK && catalog.valid,
              "real media graphics-format scan completes");
        printf("FORMAT-SCAN us-variant=%s sectors=%zu matching-nonzero=%zu palette-shapes=%zu stride-shapes=%zu retained=%zu overflow=%zu compression=%d decode=%d\n",
               catalog.us_variant == THERON_TRACK02_VARIANT_US_CLONECD_RAW ? "clonecd-index01" : "raw-bin",
               catalog.compared_sector_count, catalog.matching_nonzero_sector_count,
               catalog.huc6260_palette_candidate_count,
               catalog.le16_stride_table_candidate_count,
               catalog.candidate_count, catalog.overflow_count,
               catalog.compression_signature_detected,
               theron_v1_track02_graphics_format_catalog_can_decode(&catalog));
        for (i = 0u; i < catalog.candidate_count; ++i) {
            const Theron_Track02GraphicsFormatCandidate *candidate = &catalog.candidates[i];
            printf("FORMAT %s jp_raw=0x%zx us_raw=0x%zx bytes=%zu hash=0x%08x first=0x%04x stride=0x%04x\n",
                   theron_v1_track02_graphics_format_name(candidate->format),
                   candidate->jp_raw_offset, candidate->us_raw_offset,
                   candidate->byte_count, (unsigned)candidate->payload_checksum,
                   candidate->first_word, candidate->stride);
        }
    }
    free(jp);
    free(us);
}

int main(void) {
    uint8_t jp[RAW_SECTOR_BYTES * FIXTURE_SECTORS];
    uint8_t us[RAW_SECTOR_BYTES * FIXTURE_SECTORS];
    uint8_t clone_source_jp[RAW_SECTOR_BYTES * FIXTURE_SECTORS];
    uint8_t clone_source_us[RAW_SECTOR_BYTES * FIXTURE_SECTORS];
    uint8_t *clone_jp;
    uint8_t *clone_us;
    Theron_Track02GraphicsFormatCatalog catalog;

    build_pair(jp, us, sizeof(jp));
    check(theron_v1_track02_catalog_graphics_format_candidates(
              jp, sizeof(jp), THERON_TRACK02_MD5_JP_BIN,
              us, sizeof(us), THERON_TRACK02_MD5_US_BIN, &catalog) ==
              THERON_TRACK02_SIGNAL_OK && catalog.valid,
          "synthetic JP/US catalog is accepted");
    check(catalog.matching_nonzero_sector_count == 1u &&
              catalog.huc6260_palette_candidate_count >= 1u &&
              catalog.le16_stride_table_candidate_count >= 1u &&
              has_format(&catalog, THERON_TRACK02_GRAPHICS_FORMAT_HUC6260_PALETTE_4BPP) &&
              has_format(&catalog, THERON_TRACK02_GRAPHICS_FORMAT_LE16_STRIDE_TABLE),
          "synthetic catalog finds palette and table shapes only in matched data");
    check(catalog.compression_signature_detected == 0 && catalog.decoder_blocked &&
              !theron_v1_track02_graphics_format_catalog_can_decode(&catalog),
          "candidate catalog cannot enable an unproven decoder");

    us[RAW_SECTOR_BYTES + USER_DATA_OFFSET + 0x40u] ^= 1u;
    check(theron_v1_track02_catalog_graphics_format_candidates(
              jp, sizeof(jp), THERON_TRACK02_MD5_JP_BIN,
              us, sizeof(us), THERON_TRACK02_MD5_US_BIN, &catalog) ==
              THERON_TRACK02_SIGNAL_OK && catalog.candidate_count == 0u,
          "one JP/US mismatch rejects that sector's candidate shapes");
    check(theron_v1_track02_catalog_graphics_format_candidates(
              jp, sizeof(jp), THERON_TRACK02_MD5_US_BIN,
              us, sizeof(us), THERON_TRACK02_MD5_JP_BIN, &catalog) ==
              THERON_TRACK02_SIGNAL_UNSUPPORTED_VARIANT,
          "catalog rejects reversed or unknown variant identity");

    clone_jp = (uint8_t *)calloc(THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR +
                                 FIXTURE_SECTORS, RAW_SECTOR_BYTES);
    clone_us = (uint8_t *)calloc(FIXTURE_SECTORS, RAW_SECTOR_BYTES);
    check(clone_jp != NULL && clone_us != NULL,
          "CloneCD alignment fixture allocates");
    if (clone_jp && clone_us) {
        build_pair(clone_source_jp, clone_source_us, sizeof(clone_source_jp));
        memcpy(clone_jp + THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR * RAW_SECTOR_BYTES,
               clone_source_jp, sizeof(clone_source_jp));
        /* build_pair's conventional US image places matching data one raw
         * sector later; CloneCD starts directly at Track 02 INDEX 01. */
        memcpy(clone_us, clone_source_us + RAW_SECTOR_BYTES,
               sizeof(clone_source_us) - RAW_SECTOR_BYTES);
        check(theron_v1_track02_catalog_graphics_format_candidates(
                  clone_jp,
                  (THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR + FIXTURE_SECTORS) * RAW_SECTOR_BYTES,
                  THERON_TRACK02_MD5_JP_BIN,
                  clone_us, FIXTURE_SECTORS * RAW_SECTOR_BYTES,
                  THERON_TRACK02_MD5_US_CLONECD_BIN, &catalog) ==
                  THERON_TRACK02_SIGNAL_OK && catalog.valid &&
                  catalog.us_variant == THERON_TRACK02_VARIANT_US_CLONECD_RAW,
              "CloneCD comparison applies the documented INDEX 01 alignment");
    }
    free(clone_jp);
    free(clone_us);

    probe_real_media();
    printf("summary: fail=%d\n", g_failures);
    return g_failures ? 1 : 0;
}
