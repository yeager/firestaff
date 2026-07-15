#include "theron_v1_track02_loader_intake.h"

#include <string.h>

static uint32_t theron_v1_track02_loader_intake_fnv1a32(
    const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes && byte_count != 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int theron_v1_track02_loader_intake_observe(
    const Theron_V1Track02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderIntakeReceipt receipt = {0};

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
    }
    if (!facts || !out_receipt || !facts->authenticated_original_trace ||
        !facts->later_than_stage2_transfer ||
        (facts->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         facts->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        facts->track02_record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        facts->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        facts->destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        facts->byte_count != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        !facts->complete_payload_witness_verified ||
        facts->complete_payload_checksum == 0u) {
        return 0;
    }

    receipt.observed = 1;
    receipt.track02_variant = facts->track02_variant;
    receipt.record = facts->track02_record;
    receipt.record_user_data_offset = facts->record_user_data_offset;
    receipt.observed_destination = facts->destination;
    receipt.observed_byte_count = facts->byte_count;
    receipt.observed_payload_checksum = facts->complete_payload_checksum;
    receipt.status =
        "initial_envelope_loader_read_observed_media_bound_payload_blocked";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_complete_payload(
    const Theron_V1Track02LoaderIntakeReceipt *intake,
    const uint8_t *payload,
    size_t payload_bytes,
    Theron_V1Track02LoaderPayloadReceipt *out_receipt) {
    Theron_V1Track02LoaderPayloadReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!intake || !payload || !out_receipt || !intake->observed ||
        intake->payload_intake_admitted ||
        (intake->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         intake->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        intake->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        intake->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        intake->observed_destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        intake->observed_byte_count != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        intake->observed_payload_checksum == 0u ||
        payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        theron_v1_track02_loader_intake_fnv1a32(payload, payload_bytes) !=
            intake->observed_payload_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.track02_variant = intake->track02_variant;
    receipt.record = intake->record;
    receipt.record_user_data_offset = intake->record_user_data_offset;
    receipt.destination = intake->observed_destination;
    receipt.payload_bytes = (uint32_t)payload_bytes;
    receipt.payload_checksum = intake->observed_payload_checksum;
    memcpy(receipt.payload, payload, payload_bytes);
    receipt.status = "initial_envelope_payload_handoff_no_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_level_envelope(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    uint32_t record_user_data_offset,
    uint32_t envelope_bytes,
    uint32_t envelope_checksum,
    Theron_V1Track02LoaderLevelEnvelopeReceipt *out_receipt) {
    Theron_V1Track02LoaderLevelEnvelopeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!payload || !out_receipt || !payload->handed_off ||
        !payload->no_fallback ||
        (payload->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        payload->payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        record_user_data_offset != THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        envelope_bytes != THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES ||
        envelope_checksum == 0u ||
        record_user_data_offset > payload->payload_bytes ||
        envelope_bytes > payload->payload_bytes - record_user_data_offset ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload->payload + record_user_data_offset, envelope_bytes) !=
            envelope_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.track02_variant = payload->track02_variant;
    receipt.record = payload->record;
    receipt.record_user_data_offset = record_user_data_offset;
    receipt.envelope_bytes = envelope_bytes;
    receipt.envelope_checksum = envelope_checksum;
    memcpy(receipt.envelope, payload->payload + record_user_data_offset,
           envelope_bytes);
    receipt.status = "initial_level_record_slice_handoff_no_object_or_visual_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    uint32_t post_envelope_checksum,
    Theron_V1Track02LoaderPostEnvelopeReceipt *out_receipt) {
    Theron_V1Track02LoaderPostEnvelopeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!payload || !out_receipt || !payload->handed_off ||
        !payload->no_fallback ||
        (payload->track02_variant != THERON_TRACK02_VARIANT_JP_BIN &&
         payload->track02_variant != THERON_TRACK02_VARIANT_US_BIN) ||
        payload->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        payload->payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        post_envelope_checksum == 0u ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload->payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) !=
            post_envelope_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.track02_variant = payload->track02_variant;
    receipt.record = payload->record;
    receipt.record_user_data_offset =
        THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    receipt.byte_count = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    receipt.checksum = post_envelope_checksum;
    memcpy(receipt.bytes,
           payload->payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
           sizeof(receipt.bytes));
    receipt.status =
        "initial_level_post_envelope_source_bytes_no_object_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_iso_level_object_record(
    const Theron_V1Track02IsoLevelObjectReadFacts *facts,
    const uint8_t *payload,
    size_t payload_bytes,
    Theron_V1Track02IsoLevelObjectReceipt *out_receipt) {
    Theron_V1Track02IsoLevelObjectReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!facts || !payload || !out_receipt ||
        !facts->authenticated_original_iso_capture ||
        !facts->cue_declares_mode1_2048 ||
        facts->raw_bin_trace_borrowed ||
        facts->sector_conversion_applied ||
        facts->synthetic_dungeon_promoted ||
        facts->track02_variant != THERON_TRACK02_VARIANT_US_ISO ||
        facts->track02_record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        facts->destination != THERON_V1_INITIAL_ENVELOPE_DESTINATION ||
        facts->byte_count != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        !facts->complete_payload_witness_verified ||
        !facts->level_envelope_witness_verified ||
        !facts->post_envelope_witness_verified ||
        facts->complete_payload_checksum == 0u ||
        facts->level_envelope_checksum == 0u ||
        facts->post_envelope_checksum == 0u ||
        payload_bytes != THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES ||
        theron_v1_track02_loader_intake_fnv1a32(payload, payload_bytes) !=
            facts->complete_payload_checksum ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
            THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES) !=
            facts->level_envelope_checksum ||
        theron_v1_track02_loader_intake_fnv1a32(
            payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
            THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES) !=
            facts->post_envelope_checksum) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.no_fallback = 1;
    receipt.original_iso_capture = 1;
    receipt.cue_mode1_2048 = 1;
    receipt.no_raw_bin_trace_borrowing = 1;
    receipt.no_sector_conversion = 1;
    receipt.no_synthetic_dungeon = 1;
    receipt.track02_variant = facts->track02_variant;
    receipt.record = facts->track02_record;
    receipt.destination = facts->destination;
    receipt.payload_bytes = facts->byte_count;
    receipt.payload_checksum = facts->complete_payload_checksum;
    receipt.level_envelope_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    receipt.level_envelope_bytes = THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES;
    receipt.level_envelope_checksum = facts->level_envelope_checksum;
    memcpy(receipt.level_envelope,
           payload + THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
           sizeof(receipt.level_envelope));
    receipt.post_envelope_offset = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET;
    receipt.post_envelope_bytes = THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES;
    receipt.post_envelope_checksum = facts->post_envelope_checksum;
    memcpy(receipt.post_envelope,
           payload + THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET,
           sizeof(receipt.post_envelope));
    receipt.status =
        "iso_mode1_2048_initial_level_and_post_envelope_source_bytes_no_semantics";
    *out_receipt = receipt;
    return 1;
}
