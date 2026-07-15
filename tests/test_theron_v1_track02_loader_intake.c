#include <stdio.h>
#include <string.h>

#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02.h"
#include "theron_v1_track02_loader_intake.h"

static int failures;

static uint32_t fnv1a32(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static Theron_V1Track02LoaderReadFacts valid_facts(void) {
    Theron_V1Track02LoaderReadFacts facts = {
        1, 1, THERON_TRACK02_VARIANT_US_BIN,
        THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_ENVELOPE_DESTINATION,
        THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES,
        1, 0x6e6d4d21u
    };
    return facts;
}

static Theron_V1Track02IsoLevelObjectReadFacts valid_iso_facts(
    const uint8_t *payload, size_t payload_bytes) {
    Theron_V1Track02IsoLevelObjectReadFacts facts;

    memset(&facts, 0, sizeof(facts));
    facts.authenticated_original_iso_capture = 1;
    facts.cue_declares_mode1_2048 = 1;
    facts.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    facts.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    facts.destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    facts.byte_count = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    facts.complete_payload_witness_verified = 1;
    facts.complete_payload_checksum = fnv1a32(payload, payload_bytes);
    facts.level_envelope_witness_verified = 1;
    facts.level_envelope_checksum =
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    facts.post_envelope_witness_verified = 1;
    facts.post_envelope_checksum =
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    return facts;
}

int main(void) {
    Theron_V1Track02LoaderReadFacts facts = valid_facts();
    Theron_V1Track02IsoLevelObjectReadFacts iso_facts;
    Theron_V1Track02LoaderIntakeReceipt receipt;
    Theron_V1Track02LoaderPayloadReceipt payload_receipt;
    Theron_V1Track02LoaderLevelEnvelopeReceipt level_envelope_receipt;
    Theron_V1Track02LoaderPostEnvelopeReceipt post_envelope_receipt;
    Theron_V1Track02IsoLevelObjectReceipt iso_receipt;
    uint8_t payload[THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES];
    uint8_t raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES * 2u];
    uint8_t iso_track02[THERON_TRACK02_RAW_USER_DATA_BYTES * 2u];
    uint32_t source_record = 0u;
    uint8_t source_byte = 0u;
    size_t i;

    for (i = 0u; i < sizeof(payload); ++i) {
        payload[i] = (uint8_t)(i * 37u + 11u);
    }
    facts.complete_payload_checksum = fnv1a32(payload, sizeof(payload));

    memset(raw_track02, 0, sizeof(raw_track02));
    memset(iso_track02, 0, sizeof(iso_track02));
    raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES +
                THERON_TRACK02_RAW_USER_DATA_OFFSET + 7u] = 0x5au;
    raw_track02[THERON_TRACK02_RAW_SECTOR_BYTES + 3u] = 0xa5u;
    iso_track02[THERON_TRACK02_RAW_USER_DATA_BYTES + 7u] = 0x6bu;

    CHECK(theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        raw_track02, sizeof(raw_track02), THERON_TRACK02_MD5_US_BIN, 3010u,
        THERON_TRACK02_RAW_USER_DATA_OFFSET + 7u, &source_record,
        &source_byte));
    CHECK(source_record == 1u && source_byte == 0x5au);
    CHECK(!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        raw_track02, sizeof(raw_track02), THERON_TRACK02_MD5_US_BIN, 3010u,
        3u, &source_record, &source_byte));
    CHECK(theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        iso_track02, sizeof(iso_track02), THERON_TRACK02_MD5_US_ISO, 3010u,
        7u, &source_record, &source_byte));
    CHECK(source_record == 1u && source_byte == 0x6bu);
    CHECK(!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        iso_track02, sizeof(iso_track02), THERON_TRACK02_MD5_US_ISO, 3010u,
        THERON_TRACK02_RAW_USER_DATA_BYTES, &source_record, &source_byte));
    CHECK(!theron_v1_raw_loader_trace_track02_byte_for_scsi_source(
        raw_track02, sizeof(raw_track02), "00000000000000000000000000000000",
        3010u, THERON_TRACK02_RAW_USER_DATA_OFFSET + 7u, &source_record,
        &source_byte));

    CHECK(theron_v1_track02_loader_intake_observe(&facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.track02_variant == THERON_TRACK02_VARIANT_US_BIN);
    CHECK(receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(receipt.record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(receipt.observed_destination == facts.destination);
    CHECK(receipt.observed_byte_count == facts.byte_count);
    CHECK(receipt.observed_payload_checksum == facts.complete_payload_checksum);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_observed_media_bound_payload_blocked") == 0);
    CHECK(theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(payload_receipt.handed_off && payload_receipt.no_fallback);
    CHECK(payload_receipt.track02_variant == receipt.track02_variant);
    CHECK(payload_receipt.record == receipt.record);
    CHECK(payload_receipt.destination == receipt.observed_destination);
    CHECK(payload_receipt.payload_bytes == sizeof(payload));
    CHECK(payload_receipt.payload_checksum == facts.complete_payload_checksum);
    CHECK(memcmp(payload_receipt.payload, payload, sizeof(payload)) == 0);
    CHECK(strcmp(payload_receipt.status,
                 "initial_envelope_payload_handoff_no_semantics") == 0);
    CHECK(theron_v1_track02_loader_intake_handoff_level_envelope(
        &payload_receipt,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES),
        &level_envelope_receipt));
    CHECK(level_envelope_receipt.handed_off &&
          level_envelope_receipt.no_fallback);
    CHECK(level_envelope_receipt.track02_variant == receipt.track02_variant);
    CHECK(level_envelope_receipt.record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(level_envelope_receipt.envelope_bytes ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt,
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES),
        &post_envelope_receipt));
    CHECK(post_envelope_receipt.handed_off && post_envelope_receipt.no_fallback);
    CHECK(post_envelope_receipt.track02_variant == receipt.track02_variant);
    CHECK(post_envelope_receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(post_envelope_receipt.record_user_data_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(post_envelope_receipt.byte_count ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(memcmp(post_envelope_receipt.bytes,
                 payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                 THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) == 0);
    CHECK(strcmp(post_envelope_receipt.status,
                 "initial_level_post_envelope_source_bytes_no_object_semantics") == 0);
    CHECK(!theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt, post_envelope_receipt.checksum ^ 1u,
        &post_envelope_receipt));
    CHECK(!post_envelope_receipt.handed_off && post_envelope_receipt.status == NULL);
    ++payload[0];
    CHECK(!theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(!payload_receipt.handed_off && payload_receipt.status == NULL);
    --payload[0];
    CHECK(!theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload) - 1u, &payload_receipt));
    CHECK(!payload_receipt.handed_off && payload_receipt.status == NULL);
    receipt.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    CHECK(!payload_receipt.handed_off && payload_receipt.status == NULL);
    receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    CHECK(theron_v1_track02_loader_intake_handoff_complete_payload(
        &receipt, payload, sizeof(payload), &payload_receipt));
    payload_receipt.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
        &payload_receipt,
        fnv1a32(payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES),
        &post_envelope_receipt));
    CHECK(!post_envelope_receipt.handed_off &&
          post_envelope_receipt.status == NULL);
    CHECK(!theron_v1_track02_loader_intake_handoff_level_envelope(
        &payload_receipt,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES,
        fnv1a32(payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES),
        &level_envelope_receipt));
    CHECK(!level_envelope_receipt.handed_off &&
          level_envelope_receipt.status == NULL);
    payload_receipt.track02_variant = THERON_TRACK02_VARIANT_US_BIN;

    iso_facts = valid_iso_facts(payload, sizeof(payload));
    CHECK(theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    CHECK(iso_receipt.handed_off && iso_receipt.no_fallback);
    CHECK(iso_receipt.original_iso_capture && iso_receipt.cue_mode1_2048);
    CHECK(iso_receipt.no_raw_bin_trace_borrowing &&
          iso_receipt.no_sector_conversion &&
          iso_receipt.no_synthetic_dungeon);
    CHECK(iso_receipt.track02_variant == THERON_TRACK02_VARIANT_US_ISO);
    CHECK(iso_receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(iso_receipt.destination == THERON_V1_INITIAL_ENVELOPE_DESTINATION);
    CHECK(iso_receipt.payload_bytes == sizeof(payload));
    CHECK(iso_receipt.payload_checksum == iso_facts.complete_payload_checksum);
    CHECK(iso_receipt.level_envelope_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(iso_receipt.level_envelope_bytes ==
          THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES);
    CHECK(memcmp(iso_receipt.level_envelope,
                 payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
                 THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES) == 0);
    CHECK(iso_receipt.post_envelope_offset ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET);
    CHECK(iso_receipt.post_envelope_bytes ==
          THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES);
    CHECK(memcmp(iso_receipt.post_envelope,
                 payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
                 THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) == 0);
    CHECK(strcmp(iso_receipt.status,
                 "iso_mode1_2048_initial_level_and_post_envelope_source_bytes_no_semantics") == 0);
    iso_facts.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    CHECK(!iso_receipt.handed_off && iso_receipt.status == NULL);
    iso_facts.track02_variant = THERON_TRACK02_VARIANT_JP_REV1_ISO;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.raw_bin_trace_borrowed = 1;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.sector_conversion_applied = 1;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.synthetic_dungeon_promoted = 1;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    iso_facts.level_envelope_checksum ^= 1u;
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload), &iso_receipt));
    iso_facts = valid_iso_facts(payload, sizeof(payload));
    CHECK(!theron_v1_track02_loader_intake_handoff_iso_level_object_record(
        &iso_facts, payload, sizeof(payload) - 1u, &iso_receipt));

    facts.authenticated_original_trace = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.authenticated_original_trace = 1;
    facts.later_than_stage2_transfer = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.later_than_stage2_transfer = 1;
    facts.track02_variant = THERON_TRACK02_VARIANT_US_ISO;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_variant = THERON_TRACK02_VARIANT_UNKNOWN;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    facts.track02_record = 0x04e0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = 0x04dfu;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    facts.record_user_data_offset = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.destination = 0x3900u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.destination = THERON_V1_INITIAL_ENVELOPE_DESTINATION;
    facts.byte_count = 1024u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.byte_count = THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES;
    facts.complete_payload_witness_verified = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.complete_payload_witness_verified = 1;
    facts.complete_payload_checksum = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.complete_payload_checksum = fnv1a32(payload, sizeof(payload));
    facts.byte_count = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));

    return failures != 0;
}
