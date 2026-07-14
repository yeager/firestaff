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
