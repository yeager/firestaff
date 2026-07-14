#include <stdio.h>
#include <stdlib.h>
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

static unsigned char *read_real_track02(const char *path, size_t *out_bytes) {
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
    Theron_V1Track02LoaderReadFacts facts = valid_facts();
    Theron_V1TraceProvenanceReceipt trace = {
        1, 1, "trace_accepted_runtime_admitted"
    };
    Theron_V1DungeonHandoffReceipt initial_envelope = {
        .selected = 1,
        .runtime_route_consumed = 1,
        .record = THERON_V1_INITIAL_ENVELOPE_RECORD,
        .record_user_data_offset =
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET,
        .envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES,
        .header_width = THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH,
        .header_height = THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT,
        .header_seed = THERON_V1_INITIAL_ENVELOPE_HEADER_SEED,
        .header_identifier = THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER,
        .cue_track02_index01_raw_sector = 225u,
        .track02_raw_sector = 3123u,
        .raw_sector_offset = 0x124u,
        .raw_track02_md5_verified = 1,
        .adjacent_boundary_opaque = 1,
        .route = "raw_track02_initial_envelope"
    };
    Theron_V1AuthenticatedTrack02LoaderReadFacts authenticated_facts = {
        &trace, 1, THERON_V1_INITIAL_ENVELOPE_RECORD,
        THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET, 0x3800u, 0x800u
    };
    Theron_V1Track02LoaderIntakeReceipt receipt;
    Theron_V1Track02LoaderIntakeReceipt decoded_receipt;
    const char *real_track02_path;
    unsigned char *real_track02;
    size_t real_track02_bytes;
    unsigned char *synthetic_raw;
    size_t synthetic_raw_bytes;

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

    /* Success remains available only for an operator-supplied canonical BIN. */
    real_track02_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    real_track02 = read_real_track02(real_track02_path, &real_track02_bytes);
    if (real_track02) {
        CHECK(theron_v1_track02_loader_intake_decode_initial_envelope(
            &receipt, &initial_envelope, real_track02, real_track02_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, &decoded_receipt));
        CHECK(decoded_receipt.payload_intake_admitted);
        CHECK(decoded_receipt.initial_envelope_decoded);
        CHECK(decoded_receipt.decoded_grid_row_count == 0x001bu);
        CHECK(decoded_receipt.decoded_grid_row_bytes == 0x0020u);
        CHECK(decoded_receipt.decoded_grid_first_row_hash == 0x4b97e3abu);
        CHECK(decoded_receipt.decoded_grid_last_row_hash == 0x0b2ae445u);
        free(real_track02);
    }

    /* A header-shaped local fixture cannot substitute for the canonical BIN. */
    synthetic_raw_bytes = ((size_t)initial_envelope.track02_raw_sector + 1u) *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES;
    synthetic_raw = calloc(1u, synthetic_raw_bytes);
    CHECK(synthetic_raw != NULL);
    if (synthetic_raw) {
        size_t raw_offset = (size_t)initial_envelope.track02_raw_sector *
            THERON_V1_TRACK02_RAW_SECTOR_BYTES + initial_envelope.raw_sector_offset;
        unsigned char header[] = {
            0x00, 0x20, 0x00, 0x1b, 0x01, 0x08,
            0xe9, 0x38, 0x00, 0x26, 0x01, 0x03
        };

        memcpy(synthetic_raw + raw_offset, header, sizeof(header));
        CHECK(!theron_v1_track02_loader_intake_decode_initial_envelope(
            &receipt, &initial_envelope, synthetic_raw, synthetic_raw_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, &decoded_receipt));
        CHECK(!decoded_receipt.payload_intake_admitted);
        CHECK(!decoded_receipt.initial_envelope_decoded);
        CHECK(decoded_receipt.status == NULL);
        CHECK(receipt.initial_envelope_source_bound);
        free(synthetic_raw);
    }

    initial_envelope.envelope_bytes = facts.byte_count + 1u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.envelope_bytes = THERON_V1_INITIAL_ENVELOPE_BYTES;
    initial_envelope.header_width = 0x001fu;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.header_width = THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH;
    initial_envelope.header_height = 0x001au;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.header_height = THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT;
    initial_envelope.header_seed = 0x0108e939u;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.header_seed = THERON_V1_INITIAL_ENVELOPE_HEADER_SEED;
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
    initial_envelope.raw_track02_md5_verified = 0;
    CHECK(!theron_v1_track02_loader_intake_bind_initial_envelope(
        &receipt, &initial_envelope, &receipt));
    initial_envelope.raw_track02_md5_verified = 1;

    CHECK(theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == authenticated_facts.track02_record);
    CHECK(receipt.record_user_data_offset ==
          authenticated_facts.record_user_data_offset);
    CHECK(receipt.observed_destination == authenticated_facts.destination);
    CHECK(receipt.observed_byte_count == authenticated_facts.byte_count);
    trace.runtime_admitted = 0;
    CHECK(!theron_v1_track02_loader_intake_observe_authenticated_trace(
        &authenticated_facts, &receipt));
    CHECK(!receipt.observed);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(receipt.record == 0u);
    CHECK(receipt.record_user_data_offset == 0u);
    CHECK(receipt.observed_destination == 0u);
    CHECK(receipt.observed_byte_count == 0u);
    CHECK(receipt.status == NULL);
    trace.runtime_admitted = 1;
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
