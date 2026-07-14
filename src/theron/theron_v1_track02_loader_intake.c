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
