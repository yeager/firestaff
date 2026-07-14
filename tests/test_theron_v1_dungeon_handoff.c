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

int main(void) {
    static const unsigned char md5_vector[] = "abc";
    unsigned char *raw = calloc(1u, RAW_BYTES);
    Theron_V1DungeonHandoffFacts facts;
    Theron_V1RuntimeAdmissionReceipt admission = {1, 1};
    Theron_V1DungeonHandoffReceipt receipt;

    CHECK(raw != NULL);
    if (!raw) return 1;
    CHECK(theron_v1_track02_raw_bytes_match_md5(
        md5_vector, sizeof(md5_vector) - 1u,
        "900150983cd24fb0d6963f7d28e17f72"));
    CHECK(!theron_v1_track02_raw_bytes_match_md5(
        md5_vector, sizeof(md5_vector) - 1u,
        THERON_V1_TRACK02_MD5_US_BIN));
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

    free(raw);
    return failures != 0;
}
