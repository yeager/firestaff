#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_dungeon_handoff.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

enum {
    RAW_SECTOR_BYTES = THERON_V1_TRACK02_RAW_SECTOR_BYTES,
    JP_ZERO_STUB_BYTES = 149u * THERON_V1_TRACK02_MODE1_USER_DATA_BYTES,
    US_CANDIDATE_OFFSET = 0x7015b4u,
    US_DESCRIPTOR_OFFSET = 0x710904u,
    RAW_BYTES = ((US_DESCRIPTOR_OFFSET + 18u + RAW_SECTOR_BYTES - 1u) /
                 RAW_SECTOR_BYTES) * RAW_SECTOR_BYTES
};

static void write_us_receipt_bytes(unsigned char *raw) {
    static const unsigned char descriptor[] = {
        0x20, 0x00, 0x20, 0x04, 0x20, 0x08, 0x20, 0x0c, 0x20,
        0x10, 0x20, 0x14, 0x20, 0x18, 0x20, 0x1c, 0x20, 0x20
    };
    static const unsigned char header[] = {
        0x00, 0x20, 0x00, 0x1b, 0x01, 0x08, 0xe9, 0x38, 0x00, 0x26
    };

    memcpy(raw + US_DESCRIPTOR_OFFSET, descriptor, sizeof(descriptor));
    memcpy(raw + US_CANDIDATE_OFFSET, header, sizeof(header));
}

static Theron_V1DungeonHandoffFacts valid_facts(unsigned char *raw) {
    static Theron_V1RuntimeAdmissionReceipt admission = {1, 1};
    Theron_V1DungeonHandoffFacts facts = {
        &admission, 1, THERON_V1_TRACK02_MD5_US_BIN, raw, RAW_BYTES, 225u
    };
    return facts;
}

static unsigned char *read_raw_track02(const char *path, size_t *out_bytes) {
    FILE *file;
    long file_bytes;
    unsigned char *bytes;

    if (!path || !out_bytes || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 ||
        (file_bytes = ftell(file)) <= 0 ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)file_bytes);
    if (!bytes || fread(bytes, 1u, (size_t)file_bytes, file) !=
        (size_t)file_bytes) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_bytes = (size_t)file_bytes;
    return bytes;
}

static unsigned char *project_jp_cue_iso_from_raw_bin(
    size_t *out_bytes, int *out_invalid_source) {
    const char *path = getenv("FIRESTAFF_THERON_JP_TRACK02_RAW");
    const char *root = getenv("FIRESTAFF_THERON_DATA_DIR");
    const char *home = getenv("HOME");
    char standard_path[4096];
    int explicit_override = path && path[0];
    unsigned char *raw;
    unsigned char *projection;
    size_t raw_bytes = 0u;
    size_t sector_count;
    size_t output_offset = 0u;

    if (out_bytes) *out_bytes = 0u;
    if (out_invalid_source) *out_invalid_source = 0;
    if (!out_bytes || !out_invalid_source) return NULL;
    if (explicit_override) {
        raw = read_raw_track02(path, &raw_bytes);
    } else {
        if (root && root[0]) {
            if (snprintf(standard_path, sizeof(standard_path), "%s/TQJP02.bin",
                         root) >= (int)sizeof(standard_path)) return NULL;
        } else {
            if (!home || !home[0] ||
                snprintf(standard_path, sizeof(standard_path),
                         "%s/.firestaff/data/theron/TQJP02.bin", home) >=
                    (int)sizeof(standard_path)) return NULL;
        }
        {
            FILE *probe = fopen(standard_path, "rb");
            if (!probe) return NULL;
            fclose(probe);
        }
        raw = read_raw_track02(standard_path, &raw_bytes);
    }
    if (!raw) {
        *out_invalid_source = 1;
        return NULL;
    }
    if (raw_bytes != THERON_V1_TRACK02_JP_BIN_BYTES ||
        raw_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        !theron_v1_track02_raw_bytes_match_md5(
            raw, raw_bytes, THERON_V1_TRACK02_MD5_JP_BIN)) {
        free(raw);
        *out_invalid_source = 1;
        return NULL;
    }

    sector_count = raw_bytes / THERON_V1_TRACK02_RAW_SECTOR_BYTES;
    if (sector_count <= THERON_V1_TRACK02_JP_CUE_PREGAP_SECTORS ||
        (sector_count - THERON_V1_TRACK02_JP_CUE_PREGAP_SECTORS) *
            THERON_V1_TRACK02_MODE1_USER_DATA_BYTES !=
                THERON_V1_TRACK02_JP_CUE_ISO_BYTES) {
        free(raw);
        *out_invalid_source = 1;
        return NULL;
    }
    projection = malloc(THERON_V1_TRACK02_JP_CUE_ISO_BYTES);
    if (!projection) {
        free(raw);
        *out_invalid_source = 1;
        return NULL;
    }
    for (size_t sector = THERON_V1_TRACK02_JP_CUE_PREGAP_SECTORS;
         sector < sector_count; ++sector) {
        const size_t source_offset =
            sector * THERON_V1_TRACK02_RAW_SECTOR_BYTES +
            THERON_V1_TRACK02_MODE1_HEADER_BYTES;
        memcpy(projection + output_offset, raw + source_offset,
               THERON_V1_TRACK02_MODE1_USER_DATA_BYTES);
        output_offset += THERON_V1_TRACK02_MODE1_USER_DATA_BYTES;
    }
    free(raw);
    if (output_offset != THERON_V1_TRACK02_JP_CUE_ISO_BYTES ||
        !theron_v1_track02_raw_bytes_match_md5(
            projection, output_offset, THERON_V1_TRACK02_MD5_JP_CUE_ISO)) {
        free(projection);
        *out_invalid_source = 1;
        return NULL;
    }
    *out_bytes = output_offset;
    return projection;
}

int main(void) {
    static const unsigned char md5_vector[] = "abc";
    unsigned char *raw = calloc(1u, RAW_BYTES);
    Theron_V1DungeonHandoffFacts facts;
    Theron_V1RuntimeAdmissionReceipt admission = {1, 1};
    Theron_V1DungeonHandoffReceipt receipt;
    Theron_V1Track02RawCueAdmissionFacts admission_facts;
    Theron_V1Track02RawCueAdmissionReceipt admission_receipt;
    Theron_V1DungeonHandoffIsoFacts iso_facts;
    const char *real_track02_path;
    unsigned char *real_track02;
    size_t real_track02_bytes;

    CHECK(raw != NULL);
    if (!raw) return 1;
    CHECK(theron_v1_track02_raw_bytes_match_md5(
        md5_vector, sizeof(md5_vector) - 1u,
        "900150983cd24fb0d6963f7d28e17f72"));
    CHECK(!theron_v1_track02_raw_bytes_match_md5(
        md5_vector, sizeof(md5_vector) - 1u,
        THERON_V1_TRACK02_MD5_US_BIN));
    CHECK(theron_v1_track02_variant_from_md5(THERON_V1_TRACK02_MD5_JP_BIN) ==
          THERON_V1_TRACK02_VARIANT_JP_BIN);
    CHECK(theron_v1_track02_variant_from_md5(THERON_V1_TRACK02_MD5_US_BIN) ==
          THERON_V1_TRACK02_VARIANT_US_BIN);
    CHECK(theron_v1_track02_variant_from_md5("not-a-track02-digest") ==
          THERON_V1_TRACK02_VARIANT_NONE);
    write_us_receipt_bytes(raw);
    facts = valid_facts(raw);
    facts.runtime_admission = &admission;
    admission_facts.runtime_admission = &admission;
    admission_facts.cue_track02_index01_observed = 1;
    admission_facts.cue_track02_index01_raw_sector = 225u;
    admission_facts.raw_bin_present = 1;
    admission_facts.raw_track02 = raw;
    admission_facts.raw_track02_bytes = RAW_BYTES;
    admission_facts.track02_md5 = THERON_V1_TRACK02_MD5_US_BIN;

    /* Anchor-shaped test bytes are not original media and must never select. */
    CHECK(!theron_v1_track02_raw_cue_admit(
        &admission_facts, &admission_receipt));
    CHECK(!admission_receipt.admitted);
    CHECK(!admission_receipt.no_fallback);
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    CHECK(!receipt.selected && !receipt.raw_track02_md5_verified);
    CHECK(receipt.header_width == 0u && receipt.header_height == 0u &&
          receipt.header_seed == 0u && receipt.header_identifier == 0u);

    admission_facts.raw_track02_bytes = 2048u;
    CHECK(!theron_v1_track02_raw_cue_admit(
        &admission_facts, &admission_receipt));
    admission_facts.raw_track02_bytes = RAW_BYTES;
    admission_facts.cue_track02_index01_observed = 0;
    CHECK(!theron_v1_track02_raw_cue_admit(
        &admission_facts, &admission_receipt));
    admission_facts.cue_track02_index01_observed = 1;
    admission_facts.cue_track02_index01_raw_sector = 224u;
    CHECK(!theron_v1_track02_raw_cue_admit(
        &admission_facts, &admission_receipt));
    admission_facts.cue_track02_index01_raw_sector = 225u;
    admission_facts.raw_bin_present = 0;
    CHECK(!theron_v1_track02_raw_cue_admit(
        &admission_facts, &admission_receipt));
    admission_facts.raw_bin_present = 1;

    facts.raw_track02_bytes = 0u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    facts.raw_track02_bytes = RAW_BYTES;
    admission.admitted = 0;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    admission.admitted = 1;
    facts.cue_track02_index01_raw_sector = 224u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    facts.cue_track02_index01_raw_sector = 225u;
    raw[US_DESCRIPTOR_OFFSET] = 0u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    write_us_receipt_bytes(raw);
    raw[US_CANDIDATE_OFFSET + 9u] = 0x27u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    raw[US_CANDIDATE_OFFSET + 9u] = 0x26u;
    raw[US_CANDIDATE_OFFSET + 3u] = 0x1au;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    raw[US_CANDIDATE_OFFSET + 3u] = 0x1bu;
    raw[US_CANDIDATE_OFFSET + 7u] = 0x39u;
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    raw[US_CANDIDATE_OFFSET + 7u] = 0x38u;
    facts.track02_md5 = "00000000000000000000000000000000";
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));

    /* This exact zero-filled 149-sector span models the known legacy stub
     * only as a negative control; it is never positive gameplay input. */
    CHECK(theron_v1_track02_raw_bytes_match_md5(
        raw, JP_ZERO_STUB_BYTES, "397039af02d50d15c70b74088eb8a1cb"));
    memset(&iso_facts, 0, sizeof(iso_facts));
    iso_facts.jp_cue_iso_track02 = raw;
    iso_facts.jp_cue_iso_track02_bytes = JP_ZERO_STUB_BYTES;
    iso_facts.track02_md5 = "397039af02d50d15c70b74088eb8a1cb";
    CHECK(!theron_v1_dungeon_handoff_select_initial_level_jp_cue_iso(
        &iso_facts, &receipt));
    CHECK(!receipt.selected && !receipt.iso_track02_md5_verified &&
          !receipt.raw_track02_md5_verified);

    /* The existing raw-BIN route remains independent from the ISO projection. */
    real_track02_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    real_track02 = read_raw_track02(real_track02_path, &real_track02_bytes);
    if (real_track02) {
        Theron_V1Track02Variant variant = THERON_V1_TRACK02_VARIANT_NONE;
        uint32_t cue_index01_sector = 0u;

        if (theron_v1_track02_raw_bytes_match_md5(
                real_track02, real_track02_bytes, THERON_V1_TRACK02_MD5_US_BIN)) {
            variant = THERON_V1_TRACK02_VARIANT_US_BIN;
            cue_index01_sector = 225u;
        } else if (theron_v1_track02_raw_bytes_match_md5(
                       real_track02, real_track02_bytes,
                       THERON_V1_TRACK02_MD5_JP_BIN)) {
            variant = theron_v1_track02_variant_from_md5(
                THERON_V1_TRACK02_MD5_JP_BIN);
            cue_index01_sector = 224u;
        }
        CHECK(variant != THERON_V1_TRACK02_VARIANT_NONE);
        if (variant != THERON_V1_TRACK02_VARIANT_NONE) {
            facts = valid_facts(real_track02);
            facts.raw_track02_bytes = real_track02_bytes;
            facts.track02_md5 = variant == THERON_V1_TRACK02_VARIANT_US_BIN ?
                THERON_V1_TRACK02_MD5_US_BIN : THERON_V1_TRACK02_MD5_JP_BIN;
            facts.cue_track02_index01_raw_sector = cue_index01_sector;
            admission_facts.raw_track02 = real_track02;
            admission_facts.raw_track02_bytes = real_track02_bytes;
            admission_facts.track02_md5 = facts.track02_md5;
            admission_facts.cue_track02_index01_raw_sector = cue_index01_sector;
            CHECK(theron_v1_track02_raw_cue_admit(
                &admission_facts, &admission_receipt));
            CHECK(admission_receipt.admitted);
            CHECK(admission_receipt.raw_bin_admitted);
            CHECK(admission_receipt.cue_index01_admitted);
            CHECK(admission_receipt.iso_image_blocked);
            CHECK(admission_receipt.no_fallback);
            CHECK(admission_receipt.raw_track02_variant == variant);
            CHECK(admission_receipt.cue_track02_index01_raw_sector ==
                  cue_index01_sector);
            CHECK(admission_receipt.raw_track02_bytes == real_track02_bytes);
            CHECK(admission_receipt.track02_md5 == facts.track02_md5);
            CHECK(strcmp(admission_receipt.status,
                         "raw_track02_bin_cue_admitted_iso_blocked_no_fallback") == 0);
            CHECK(theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
            CHECK(receipt.raw_track02_variant == variant);
            CHECK(receipt.adjacent_boundary_opaque);
            CHECK(receipt.route != NULL &&
                  strcmp(receipt.route, "raw_track02_initial_envelope") == 0);
        }
        free(real_track02);
    }

    /* Build the positive MODE1/2048 projection in memory from authentic JP
     * raw media only, then independently hash it again in the selector. */
    {
        size_t projection_bytes = 0u;
        int invalid_jp_source = 0;
        unsigned char *projection =
            project_jp_cue_iso_from_raw_bin(
                &projection_bytes, &invalid_jp_source);
        if (projection) {
            unsigned char *tampered = malloc(projection_bytes);
            iso_facts.jp_cue_iso_track02 = projection;
            iso_facts.jp_cue_iso_track02_bytes = projection_bytes;
            iso_facts.track02_md5 = "ceb02343868f80cec899e9b239aff2da";
            CHECK(!theron_v1_dungeon_handoff_select_initial_level_jp_cue_iso(
                &iso_facts, &receipt));
            CHECK(!receipt.selected && !receipt.iso_track02_md5_verified);

            iso_facts.track02_md5 = THERON_V1_TRACK02_MD5_JP_CUE_ISO;
            CHECK(theron_v1_dungeon_handoff_select_initial_level_jp_cue_iso(
                &iso_facts, &receipt));
            CHECK(receipt.selected && !receipt.runtime_route_consumed);
            CHECK(receipt.iso_track02_md5_verified);
            CHECK(receipt.iso_track02_bytes == projection_bytes);
            CHECK(receipt.iso_track02_md5 != NULL &&
                  strcmp(receipt.iso_track02_md5,
                         THERON_V1_TRACK02_MD5_JP_CUE_ISO) == 0);
            CHECK(!receipt.raw_track02_md5_verified);
            CHECK(receipt.raw_track02_variant ==
                  THERON_V1_TRACK02_VARIANT_NONE);
            CHECK(receipt.cue_track02_index01_raw_sector == 0u &&
                  receipt.track02_raw_sector == 0u &&
                  receipt.raw_sector_offset == 0u);
            CHECK(receipt.track02_iso_byte_offset ==
                  (0x700c84u / RAW_SECTOR_BYTES -
                   THERON_V1_TRACK02_JP_CUE_PREGAP_SECTORS) *
                      THERON_V1_TRACK02_MODE1_USER_DATA_BYTES +
                  (0x700c84u % RAW_SECTOR_BYTES -
                   THERON_V1_TRACK02_MODE1_HEADER_BYTES));
            CHECK(receipt.route != NULL && strcmp(receipt.route,
                  "jp_cue_iso_initial_envelope") == 0);

            if (tampered) {
                memcpy(tampered, projection, projection_bytes);
                tampered[0] ^= 1u;
                iso_facts.jp_cue_iso_track02 = tampered;
                CHECK(!theron_v1_dungeon_handoff_select_initial_level_jp_cue_iso(
                    &iso_facts, &receipt));
                CHECK(!receipt.selected && !receipt.iso_track02_md5_verified);
                free(tampered);
            } else {
                CHECK(0 && "allocate ISO hash-mutation rejection copy");
            }
            free(projection);
        } else {
            if (invalid_jp_source) {
                CHECK(0 && "explicit or present JP Track 02 source failed identity/projection checks");
            } else {
                puts("SKIP: authentic JP raw BIN unavailable for ISO projection");
                free(raw);
                return 77;
            }
        }
    }

    free(raw);
    return failures != 0;
}
