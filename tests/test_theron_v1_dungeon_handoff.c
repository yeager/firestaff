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

int main(void) {
    static const unsigned char md5_vector[] = "abc";
    unsigned char *raw = calloc(1u, RAW_BYTES);
    Theron_V1DungeonHandoffFacts facts;
    Theron_V1RuntimeAdmissionReceipt admission = {1, 1};
    Theron_V1DungeonHandoffReceipt receipt;
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

    /* Anchor-shaped test bytes are not original media and must never select. */
    CHECK(!theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
    CHECK(!receipt.selected && !receipt.raw_track02_md5_verified);
    CHECK(receipt.header_width == 0u && receipt.header_height == 0u &&
          receipt.header_seed == 0u && receipt.header_identifier == 0u);

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

    /* Positive selection is available only for an operator-supplied raw BIN. */
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
            CHECK(theron_v1_dungeon_handoff_select_initial_level(&facts, &receipt));
            CHECK(receipt.raw_track02_variant == variant);
            CHECK(receipt.adjacent_boundary_opaque);
            CHECK(receipt.route != NULL &&
                  strcmp(receipt.route, "raw_track02_initial_envelope") == 0);
        }
        free(real_track02);
    }

    free(raw);
    return failures != 0;
}
