#include "theron_v1_track02_loader_intake.h"

#include <string.h>

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
        facts->byte_count == 0u) {
        return 0;
    }

    receipt.observed = 1;
    receipt.record = facts->track02_record;
    receipt.record_user_data_offset = facts->record_user_data_offset;
    receipt.observed_destination = facts->destination;
    receipt.observed_byte_count = facts->byte_count;
    receipt.status = "initial_envelope_loader_read_observed_payload_blocked";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_observe_authenticated_trace(
    const Theron_V1AuthenticatedTrack02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderReadFacts observation;

    if (!facts || !facts->trace_provenance ||
        !facts->trace_provenance->valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }

    observation.authenticated_original_trace = 1;
    observation.later_than_stage2_transfer = facts->later_than_stage2_transfer;
    observation.track02_record = facts->track02_record;
    observation.record_user_data_offset = facts->record_user_data_offset;
    observation.destination = facts->destination;
    observation.byte_count = facts->byte_count;
    return theron_v1_track02_loader_intake_observe(&observation, out_receipt);
}

int theron_v1_track02_loader_intake_bind_initial_envelope(
    const Theron_V1Track02LoaderIntakeReceipt *observation,
    const Theron_V1DungeonHandoffReceipt *initial_envelope,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderIntakeReceipt receipt;

    if (!observation || !initial_envelope || !out_receipt) return 0;
    receipt = *observation;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!receipt.observed || receipt.payload_intake_admitted ||
        receipt.record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        receipt.record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        !initial_envelope->selected ||
        !initial_envelope->runtime_route_consumed ||
        !initial_envelope->raw_track02_md5_verified ||
        initial_envelope->record != receipt.record ||
        initial_envelope->record_user_data_offset !=
            receipt.record_user_data_offset ||
        initial_envelope->envelope_bytes != THERON_V1_INITIAL_ENVELOPE_BYTES ||
        initial_envelope->header_identifier !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER ||
        initial_envelope->track02_raw_sector !=
            initial_envelope->cue_track02_index01_raw_sector + receipt.record ||
        initial_envelope->raw_sector_offset !=
            receipt.record_user_data_offset + THERON_V1_TRACK02_MODE1_HEADER_BYTES ||
        !initial_envelope->adjacent_boundary_opaque ||
        receipt.observed_byte_count < initial_envelope->envelope_bytes) {
        return 0;
    }

    receipt.initial_envelope_source_bound = 1;
    receipt.status = "initial_envelope_loader_read_source_bound_payload_blocked";
    *out_receipt = receipt;
    return 1;
}
