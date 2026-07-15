#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02_loader_intake.h"

static int failures;

typedef struct {
    int calls;
    uint32_t grid_hash;
    uint32_t grid_bytes;
    uint8_t first_byte;
    uint8_t last_byte;
    int accept;
} RuntimeConsumerCapture;

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

static int capture_runtime_grid(const Theron_V1Track02RawGridReceipt *grid,
                                void *context) {
    RuntimeConsumerCapture *capture = context;

    if (!grid || !capture) return 0;
    ++capture->calls;
    capture->grid_hash = grid->raw_grid_hash;
    capture->grid_bytes = grid->raw_grid_bytes;
    capture->first_byte = grid->raw_grid[0];
    capture->last_byte = grid->raw_grid[grid->raw_grid_bytes - 1u];
    return capture->accept;
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
    Theron_V1Track02RawGridCoordinateReceipt coordinate_receipt;
    Theron_V1Track02RawGridRowReceipt row_receipt;
    Theron_V1Track02RawGridReceipt grid_receipt;
    Theron_V1Track02RawGridRuntimeReceipt runtime_receipt;
    Theron_V1Track02RawGridObjectTableProjectionReceipt object_projection;
    Theron_V1Track02RawGridBitmapRouteReceipt bitmap_route;
    Theron_V1Track02RawGridLevelRouteReceipt level_route;
    Theron_V1Track02RawGridDungeonRouteReceipt dungeon_route;
    const char *real_track02_path;
    const char *real_track02_md5 = NULL;
    unsigned char *real_track02;
    size_t real_track02_bytes;
    unsigned char *synthetic_raw;
    size_t synthetic_raw_bytes;

    CHECK(theron_v1_track02_loader_intake_observe(&facts, &receipt));
    CHECK(receipt.observed);
    CHECK(!receipt.authenticated_v3_trace);
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
    CHECK(!receipt.authenticated_v3_trace);
    CHECK(!receipt.payload_intake_admitted);
    CHECK(strcmp(receipt.status,
                 "initial_envelope_loader_read_source_bound_payload_blocked") == 0);

    /* Success remains available only for an operator-supplied canonical BIN. */
    real_track02_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    real_track02 = read_real_track02(real_track02_path, &real_track02_bytes);
    if (real_track02) {
        RuntimeConsumerCapture consumer = {0};
        int is_us = theron_v1_track02_raw_bytes_match_md5(
            real_track02, real_track02_bytes, THERON_V1_TRACK02_MD5_US_BIN);

        real_track02_md5 = is_us ? THERON_V1_TRACK02_MD5_US_BIN :
            THERON_V1_TRACK02_MD5_JP_BIN;
        CHECK(theron_v1_track02_raw_bytes_match_md5(
            real_track02, real_track02_bytes, real_track02_md5));
        initial_envelope.cue_track02_index01_raw_sector = is_us ? 225u : 224u;
        initial_envelope.track02_raw_sector = is_us ? 3123u : 3122u;
        CHECK(theron_v1_track02_loader_intake_observe_authenticated_trace(
            &authenticated_facts, &receipt));
        CHECK(receipt.authenticated_v3_trace);
        CHECK(theron_v1_track02_loader_intake_bind_initial_envelope(
            &receipt, &initial_envelope, &receipt));
        CHECK(receipt.authenticated_v3_trace);
        CHECK(theron_v1_track02_loader_intake_decode_initial_envelope(
            &receipt, &initial_envelope, real_track02, real_track02_bytes,
            real_track02_md5, &decoded_receipt));
        CHECK(decoded_receipt.authenticated_v3_trace);
        CHECK(decoded_receipt.payload_intake_admitted);
        CHECK(decoded_receipt.initial_envelope_decoded);
        CHECK(decoded_receipt.decoded_grid_row_count == 0x001bu);
        CHECK(decoded_receipt.decoded_grid_row_bytes == 0x0020u);
        CHECK(decoded_receipt.decoded_grid_raw_sector ==
              initial_envelope.track02_raw_sector);
        CHECK(decoded_receipt.decoded_grid_raw_sector_offset == 0x130u);
        if (is_us) {
            CHECK(decoded_receipt.decoded_grid_first_row_hash == 0x4b97e3abu);
            CHECK(decoded_receipt.decoded_grid_last_row_hash == 0x0b2ae445u);
        }
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, &grid_receipt));
        CHECK(grid_receipt.handed_off);
        CHECK(grid_receipt.authenticated_v3_trace);
        CHECK(grid_receipt.raw_grid_width == 0x0020u);
        CHECK(grid_receipt.raw_grid_height == 0x001bu);
        CHECK(grid_receipt.raw_grid_bytes == 0x0360u);
        if (is_us) {
            CHECK(grid_receipt.raw_grid[0] == 0x84u);
            CHECK(grid_receipt.raw_grid[31] == 0x56u);
            CHECK(grid_receipt.raw_grid[0x035fu] == 0u);
        }
        CHECK(grid_receipt.raw_track02_sector == initial_envelope.track02_raw_sector);
        CHECK(grid_receipt.raw_sector_offset == 0x130u);
        CHECK(grid_receipt.raw_track02_offset == 0x7015c0u);
        CHECK(grid_receipt.raw_grid_hash == decoded_receipt.decoded_grid_hash);
        CHECK(strcmp(grid_receipt.status,
                     "initial_envelope_raw_grid_handoff_no_semantics") == 0);
        CHECK(theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
            &grid_receipt, &object_projection));
        CHECK(object_projection.projection_blocked);
        CHECK(object_projection.authenticated_v3_trace);
        CHECK(object_projection.no_fallback);
        CHECK(object_projection.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(object_projection.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(object_projection.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(object_projection.status,
                     "initial_envelope_raw_grid_object_table_projection_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
            &grid_receipt, &bitmap_route));
        CHECK(bitmap_route.bitmap_route_blocked);
        CHECK(bitmap_route.authenticated_v3_trace);
        CHECK(bitmap_route.no_fallback);
        CHECK(bitmap_route.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(bitmap_route.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(bitmap_route.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(bitmap_route.status,
                     "initial_envelope_raw_grid_bitmap_route_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_admit_raw_grid_level_route(
            &grid_receipt, &level_route));
        CHECK(level_route.level_route_admitted);
        CHECK(level_route.authenticated_v3_trace);
        CHECK(level_route.bitmap_route_blocked);
        CHECK(level_route.object_route_blocked);
        CHECK(level_route.no_fallback);
        CHECK(level_route.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(level_route.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(level_route.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(level_route.status,
                     "initial_envelope_raw_grid_level_route_bitmap_object_blocked_no_fallback") == 0);
        CHECK(theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
            &grid_receipt, &dungeon_route));
        CHECK(dungeon_route.dungeon_route_admitted);
        CHECK(dungeon_route.authenticated_v3_trace);
        CHECK(dungeon_route.bitmap_route_blocked);
        CHECK(dungeon_route.object_route_blocked);
        CHECK(dungeon_route.no_fallback);
        CHECK(dungeon_route.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(dungeon_route.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(dungeon_route.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(dungeon_route.status,
                     "initial_envelope_raw_grid_dungeon_route_bitmap_object_blocked_no_fallback") == 0);
        ++grid_receipt.raw_grid[0];
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
            &grid_receipt, &bitmap_route));
        CHECK(!bitmap_route.bitmap_route_blocked);
        CHECK(!bitmap_route.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_level_route(
            &grid_receipt, &level_route));
        CHECK(!level_route.level_route_admitted);
        CHECK(!level_route.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
            &grid_receipt, &dungeon_route));
        CHECK(!dungeon_route.dungeon_route_admitted);
        CHECK(!dungeon_route.no_fallback);
        --grid_receipt.raw_grid[0];
        consumer.accept = 1;
        CHECK(theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, capture_runtime_grid, &consumer,
            &runtime_receipt));
        CHECK(runtime_receipt.delivered && runtime_receipt.no_fallback);
        CHECK(runtime_receipt.authenticated_v3_trace);
        CHECK(runtime_receipt.raw_grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(runtime_receipt.raw_grid_hash == grid_receipt.raw_grid_hash);
        CHECK(runtime_receipt.raw_track02_offset == grid_receipt.raw_track02_offset);
        CHECK(strcmp(runtime_receipt.status,
                     "initial_envelope_raw_grid_delivered_no_fallback") == 0);
        CHECK(consumer.calls == 1);
        CHECK(consumer.grid_hash == grid_receipt.raw_grid_hash);
        CHECK(consumer.grid_bytes == grid_receipt.raw_grid_bytes);
        CHECK(consumer.first_byte == grid_receipt.raw_grid[0]);
        CHECK(consumer.last_byte == grid_receipt.raw_grid[
            grid_receipt.raw_grid_bytes - 1u]);
        consumer.accept = 0;
        CHECK(!theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, capture_runtime_grid, &consumer,
            &runtime_receipt));
        CHECK(!runtime_receipt.delivered && !runtime_receipt.no_fallback);
        CHECK(runtime_receipt.status == NULL);
        CHECK(consumer.calls == 2);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, &row_receipt));
        CHECK(row_receipt.handed_off);
        CHECK(row_receipt.raw_grid_y == 0u);
        CHECK(row_receipt.raw_grid_bytes == 0x20u);
        if (is_us) {
            CHECK(row_receipt.raw_grid_row[0] == 0x84u);
            CHECK(row_receipt.raw_grid_row[31] == 0x56u);
        }
        CHECK(row_receipt.raw_track02_sector == initial_envelope.track02_raw_sector);
        CHECK(row_receipt.raw_sector_offset == 0x130u);
        CHECK(row_receipt.raw_track02_offset == 0x7015c0u);
        CHECK(row_receipt.raw_grid_row_hash == 0x4b97e3abu);
        CHECK(strcmp(row_receipt.status,
                     "initial_envelope_raw_grid_row_handoff_no_semantics") == 0);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 26u, &row_receipt));
        CHECK(row_receipt.raw_sector_offset == 0x470u);
        if (is_us) {
            CHECK(row_receipt.raw_grid_row[0] == 0u);
            CHECK(row_receipt.raw_grid_row_hash == 0x0b2ae445u);
        }
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 27u, &row_receipt));
        CHECK(!row_receipt.handed_off);
        CHECK(row_receipt.status == NULL);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 0u, &coordinate_receipt));
        CHECK(coordinate_receipt.handed_off);
        if (is_us) CHECK(coordinate_receipt.raw_grid_byte == 0x84u);
        CHECK(coordinate_receipt.raw_track02_sector ==
              initial_envelope.track02_raw_sector);
        CHECK(coordinate_receipt.raw_sector_offset == 0x130u);
        CHECK(coordinate_receipt.raw_track02_offset == 0x7015c0u);
        CHECK(strcmp(coordinate_receipt.status,
                     "initial_envelope_raw_grid_coordinate_handoff_no_semantics") == 0);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 31u, 0u, &coordinate_receipt));
        if (is_us) CHECK(coordinate_receipt.raw_grid_byte == 0x56u);
        CHECK(coordinate_receipt.raw_sector_offset == 0x14fu);
        CHECK(theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 26u, &coordinate_receipt));
        if (is_us) CHECK(coordinate_receipt.raw_grid_byte == 0u);
        CHECK(coordinate_receipt.raw_sector_offset == 0x470u);
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 32u, 0u, &coordinate_receipt));
        CHECK(!coordinate_receipt.handed_off);
        CHECK(coordinate_receipt.status == NULL);
        ++decoded_receipt.decoded_grid_hash;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 0u, &coordinate_receipt));
        CHECK(!coordinate_receipt.handed_off);
        CHECK(coordinate_receipt.status == NULL);
        --decoded_receipt.decoded_grid_hash;
        ++decoded_receipt.decoded_grid_row_bytes;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, &row_receipt));
        CHECK(!row_receipt.handed_off);
        CHECK(row_receipt.status == NULL);
        --decoded_receipt.decoded_grid_row_bytes;
        ++decoded_receipt.decoded_grid_row_count;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, &grid_receipt));
        CHECK(!grid_receipt.handed_off);
        CHECK(grid_receipt.status == NULL);
        --decoded_receipt.decoded_grid_row_count;
        ++decoded_receipt.decoded_grid_raw_sector;
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
            &decoded_receipt, real_track02, real_track02_bytes,
            real_track02_md5, 0u, 0u, &coordinate_receipt));
        CHECK(!coordinate_receipt.handed_off);
        CHECK(coordinate_receipt.status == NULL);
        free(real_track02);
        initial_envelope.cue_track02_index01_raw_sector = 225u;
        initial_envelope.track02_raw_sector = 3123u;
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
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid_row(
            &receipt, synthetic_raw, synthetic_raw_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, 0u, &row_receipt));
        CHECK(!row_receipt.handed_off);
        CHECK(row_receipt.status == NULL);
        CHECK(!theron_v1_track02_loader_intake_handoff_raw_grid(
            &receipt, synthetic_raw, synthetic_raw_bytes,
            THERON_V1_TRACK02_MD5_US_BIN, &grid_receipt));
        CHECK(!grid_receipt.handed_off);
        CHECK(grid_receipt.status == NULL);
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
            &grid_receipt, &object_projection));
        CHECK(!object_projection.projection_blocked);
        CHECK(!object_projection.no_fallback);
        CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
            &grid_receipt, &bitmap_route));
        CHECK(!bitmap_route.bitmap_route_blocked);
        CHECK(!bitmap_route.no_fallback);
        CHECK(receipt.initial_envelope_source_bound);
        free(synthetic_raw);
    }

    memset(&grid_receipt, 0, sizeof(grid_receipt));
    grid_receipt.handed_off = 1;
    grid_receipt.authenticated_v3_trace = 0;
    grid_receipt.raw_grid_width = THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH;
    grid_receipt.raw_grid_height = THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT;
    grid_receipt.raw_grid_bytes =
        THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH *
        THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT;
    grid_receipt.raw_grid_hash = 0x12345678u;
    grid_receipt.raw_track02_sector = initial_envelope.track02_raw_sector;
    grid_receipt.raw_sector_offset = 0x130u;
    grid_receipt.raw_track02_offset = 0x7015c0u;
    grid_receipt.status = "initial_envelope_raw_grid_handoff_no_semantics";
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_level_route(
        &grid_receipt, &level_route));
    CHECK(!level_route.level_route_admitted);
    CHECK(!level_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
        &grid_receipt, &dungeon_route));
    CHECK(!dungeon_route.dungeon_route_admitted);
    CHECK(!dungeon_route.no_fallback);
    grid_receipt.authenticated_v3_trace = 1;
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_level_route(
        &grid_receipt, &level_route));
    CHECK(!level_route.level_route_admitted);
    CHECK(!level_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
        &grid_receipt, &dungeon_route));
    CHECK(!dungeon_route.dungeon_route_admitted);
    CHECK(!dungeon_route.no_fallback);
    grid_receipt.status = NULL;
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    grid_receipt.status = "initial_envelope_raw_grid_handoff_no_semantics";
    grid_receipt.raw_grid_hash = 0u;
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
        &grid_receipt, &object_projection));
    CHECK(!object_projection.projection_blocked);
    CHECK(!object_projection.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
        &grid_receipt, &bitmap_route));
    CHECK(!bitmap_route.bitmap_route_blocked);
    CHECK(!bitmap_route.no_fallback);
    CHECK(!theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
        &grid_receipt, &dungeon_route));
    CHECK(!dungeon_route.dungeon_route_admitted);
    CHECK(!dungeon_route.no_fallback);

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
    CHECK(receipt.authenticated_v3_trace);
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
