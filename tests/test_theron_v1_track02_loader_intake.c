#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_loader_intake.h"

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static Theron_V1Track02LoaderReadFacts valid_facts(void) {
    Theron_V1Track02LoaderReadFacts facts = {
        1, 1, THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        0x3800u, 0x800u
    };
    return facts;
}

int main(void) {
    Theron_V1Track02LoaderReadFacts facts = valid_facts();
    Theron_V1TraceProvenanceReceipt trace = {
        1, 0, "trace_accepted_runtime_unavailable"
    };
    Theron_V1DungeonHandoffReceipt initial_envelope = {
        .selected = 1,
        .runtime_route_consumed = 1,
        .record = THERON_V1_INITIAL_ENVELOPE_RECORD,
        .record_user_data_offset =
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        .envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES,
        .header_identifier = THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER,
        .cue_track02_index01_raw_sector = 225u,
        .track02_raw_sector = 3123u,
        .raw_sector_offset = 0x124u,
        .adjacent_boundary_opaque = 1,
        .route = "raw_track02_initial_envelope"
    };
    Theron_V1AuthenticatedTrack02LoaderReadFacts authenticated_facts = {
        &trace, 1, THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET, 0x3800u, 0x800u
    };
    Theron_V1Track02LoaderIntakeReceipt receipt;

    CHECK(theron_v1_track02_loader_intake_observe(&facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == THERON_V1_INITIAL_ENVELOPE_RECORD);
    CHECK(receipt.record_user_data_offset ==
          THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET);
    CHECK(receipt.observed_destination == facts.destination);
    CHECK(receipt.observed_byte_count == facts.byte_count);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_observed_payload_blocked") == 0);
    CHECK(theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    CHECK(receipt.initial_envelope_source_bound);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_source_bound_payload_blocked") == 0);

    initial_envelope.envelope_bytes = facts.byte_count + 1u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES;
    initial_envelope.record = 0x04e0u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    initial_envelope.track02_raw_sector = 3122u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.track02_raw_sector = 3123u;
    initial_envelope.raw_sector_offset = 0x123u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.raw_sector_offset = 0x124u;

    CHECK(theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == authenticated_facts.track02_record);
    CHECK(receipt.record_user_data_offset ==
          authenticated_facts.record_user_data_offset);
    CHECK(receipt.observed_destination == authenticated_facts.destination);
    CHECK(receipt.observed_byte_count == authenticated_facts.byte_count);
    trace.valid = 0;
    CHECK(!theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(!receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == 0u);
    CHECK(receipt.record_user_data_offset == 0u);
    CHECK(receipt.observed_destination == 0u);
    CHECK(receipt.observed_byte_count == 0u);
    CHECK(receipt.status == NULL);
    trace.valid = 1;
    authenticated_facts.byte_count = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    authenticated_facts.byte_count = 0x800u;

    facts.authenticated_original_trace = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.authenticated_original_trace = 1;
    facts.later_than_stage2_transfer = 0;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.later_than_stage2_transfer = 1;
    facts.track02_record = 0x04e0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = 0x04dfu;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.track02_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    facts.record_user_data_offset = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));
    facts.record_user_data_offset =
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    facts.byte_count = 0u;
    CHECK(!theron_v1_track02_loader_intake_observe(&facts, &receipt));

    return failures != 0;
}
