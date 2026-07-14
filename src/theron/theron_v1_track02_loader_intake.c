#include "theron_v1_track02_loader_intake.h"

#include <string.h>

enum {
    TQR_INITIAL_ENVELOPE_HEADER_BYTES = 12u
};

static uint16_t read_be16(const uint8_t *bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes) {
    return ((uint32_t)bytes[0] << 24u) | ((uint32_t)bytes[1] << 16u) |
        ((uint32_t)bytes[2] << 8u) | bytes[3];
}

static uint32_t hash_bytes(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;

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

int theron_v1_track02_loader_intake_decode_initial_envelope(
    const Theron_V1Track02LoaderIntakeReceipt *source_bound_receipt,
    const Theron_V1DungeonHandoffReceipt *initial_envelope,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderIntakeReceipt receipt;
    size_t raw_offset;
    const uint8_t *envelope;
    uint16_t grid_width;
    uint16_t grid_height;
    uint32_t grid_bytes;
    uint32_t grid_raw_sector_offset;

    if (!source_bound_receipt || !initial_envelope || !out_receipt) return 0;
    receipt = *source_bound_receipt;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!receipt.observed || !receipt.initial_envelope_source_bound ||
        receipt.payload_intake_admitted || receipt.initial_envelope_decoded ||
        !raw_track02 || !raw_track02_md5 ||
        raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        (strcmp(raw_track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0 &&
         strcmp(raw_track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) ||
        !theron_v1_track02_raw_bytes_match_md5(raw_track02,
                                                raw_track02_bytes,
                                                raw_track02_md5) ||
        !initial_envelope->selected ||
        !initial_envelope->runtime_route_consumed ||
        !initial_envelope->raw_track02_md5_verified ||
        initial_envelope->record != receipt.record ||
        initial_envelope->record_user_data_offset !=
            receipt.record_user_data_offset ||
        initial_envelope->envelope_bytes != THERON_V1_INITIAL_ENVELOPE_BYTES ||
        initial_envelope->track02_raw_sector <
            initial_envelope->cue_track02_index01_raw_sector ||
        initial_envelope->track02_raw_sector -
            initial_envelope->cue_track02_index01_raw_sector != receipt.record ||
        initial_envelope->raw_sector_offset !=
            receipt.record_user_data_offset + THERON_V1_TRACK02_MODE1_HEADER_BYTES ||
        receipt.observed_byte_count < initial_envelope->envelope_bytes) {
        return 0;
    }

    raw_offset = (size_t)initial_envelope->track02_raw_sector *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES + initial_envelope->raw_sector_offset;
    if (raw_offset > raw_track02_bytes ||
        initial_envelope->envelope_bytes > raw_track02_bytes - raw_offset ||
        initial_envelope->envelope_bytes < TQR_INITIAL_ENVELOPE_HEADER_BYTES) {
        return 0;
    }

    envelope = raw_track02 + raw_offset;
    grid_width = read_be16(envelope);
    grid_height = read_be16(envelope + 2u);
    grid_bytes = (uint32_t)grid_width * grid_height;
    if (initial_envelope->raw_sector_offset >
            THERON_V1_TRACK02_RAW_SECTOR_BYTES -
                TQR_INITIAL_ENVELOPE_HEADER_BYTES) {
        return 0;
    }
    grid_raw_sector_offset = initial_envelope->raw_sector_offset +
        TQR_INITIAL_ENVELOPE_HEADER_BYTES;
    if (grid_width != initial_envelope->header_width ||
        grid_height != initial_envelope->header_height ||
        read_be32(envelope + 4u) != initial_envelope->header_seed ||
        read_be16(envelope + 8u) != initial_envelope->header_identifier ||
        grid_bytes != initial_envelope->envelope_bytes -
            TQR_INITIAL_ENVELOPE_HEADER_BYTES ||
        grid_bytes > THERON_V1_TRACK02_RAW_SECTOR_BYTES -
            grid_raw_sector_offset) {
        return 0;
    }

    receipt.payload_intake_admitted = 1;
    receipt.initial_envelope_decoded = 1;
    receipt.decoded_header_width = read_be16(envelope);
    receipt.decoded_header_height = read_be16(envelope + 2u);
    receipt.decoded_header_seed = read_be32(envelope + 4u);
    receipt.decoded_header_identifier = read_be16(envelope + 8u);
    receipt.decoded_header_extension = read_be16(envelope + 10u);
    receipt.decoded_grid_bytes = grid_bytes;
    receipt.decoded_grid_hash = hash_bytes(envelope + TQR_INITIAL_ENVELOPE_HEADER_BYTES,
                                           grid_bytes);
    receipt.decoded_grid_row_count = grid_height;
    receipt.decoded_grid_row_bytes = grid_width;
    receipt.decoded_grid_raw_sector = initial_envelope->track02_raw_sector;
    receipt.decoded_grid_raw_sector_offset = grid_raw_sector_offset;
    receipt.decoded_grid_first_row_hash = hash_bytes(
        envelope + TQR_INITIAL_ENVELOPE_HEADER_BYTES, grid_width);
    receipt.decoded_grid_last_row_hash = hash_bytes(
        envelope + TQR_INITIAL_ENVELOPE_HEADER_BYTES +
            ((size_t)(grid_height - 1u) * grid_width),
        grid_width);
    receipt.status = "initial_envelope_loader_read_decoded_no_semantic_handoff";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint16_t raw_grid_x,
    uint16_t raw_grid_y,
    Theron_V1Track02RawGridCoordinateReceipt *out_receipt) {
    size_t grid_raw_offset;
    size_t coordinate_raw_offset;
    const uint8_t *envelope;
    const uint8_t *grid;
    uint32_t cue_index01_raw_sector;
    Theron_V1Track02RawGridCoordinateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!decoded_receipt || !raw_track02 || !raw_track02_md5 || !out_receipt ||
        !decoded_receipt->observed ||
        !decoded_receipt->payload_intake_admitted ||
        !decoded_receipt->initial_envelope_source_bound ||
        !decoded_receipt->initial_envelope_decoded ||
        decoded_receipt->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        decoded_receipt->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        decoded_receipt->decoded_header_width !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_header_height !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        decoded_receipt->decoded_header_seed !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_SEED ||
        decoded_receipt->decoded_header_identifier !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER ||
        decoded_receipt->decoded_grid_row_count !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        decoded_receipt->decoded_grid_row_bytes !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_grid_bytes !=
            (uint32_t)THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT *
                THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_grid_raw_sector_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET +
                THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                TQR_INITIAL_ENVELOPE_HEADER_BYTES ||
        raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        (strcmp(raw_track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0 &&
         strcmp(raw_track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) ||
        !theron_v1_track02_raw_bytes_match_md5(raw_track02,
                                                raw_track02_bytes,
                                                raw_track02_md5) ||
        raw_grid_x >= decoded_receipt->decoded_grid_row_bytes ||
        raw_grid_y >= decoded_receipt->decoded_grid_row_count) {
        return 0;
    }

    cue_index01_raw_sector = strcmp(raw_track02_md5,
        THERON_V1_TRACK02_MD5_JP_BIN) == 0 ? 224u : 225u;
    if (decoded_receipt->decoded_grid_raw_sector !=
            cue_index01_raw_sector + decoded_receipt->record) {
        return 0;
    }

    grid_raw_offset = (size_t)decoded_receipt->decoded_grid_raw_sector *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES +
        decoded_receipt->decoded_grid_raw_sector_offset;
    if (grid_raw_offset < TQR_INITIAL_ENVELOPE_HEADER_BYTES ||
        grid_raw_offset > raw_track02_bytes ||
        decoded_receipt->decoded_grid_bytes >
            raw_track02_bytes - grid_raw_offset) {
        return 0;
    }

    envelope = raw_track02 + grid_raw_offset - TQR_INITIAL_ENVELOPE_HEADER_BYTES;
    grid = raw_track02 + grid_raw_offset;
    if (read_be16(envelope) != decoded_receipt->decoded_header_width ||
        read_be16(envelope + 2u) != decoded_receipt->decoded_header_height ||
        read_be32(envelope + 4u) != decoded_receipt->decoded_header_seed ||
        read_be16(envelope + 8u) != decoded_receipt->decoded_header_identifier ||
        read_be16(envelope + 10u) != decoded_receipt->decoded_header_extension ||
        hash_bytes(grid, decoded_receipt->decoded_grid_bytes) !=
            decoded_receipt->decoded_grid_hash) {
        return 0;
    }

    coordinate_raw_offset = grid_raw_offset +
        (size_t)raw_grid_y * decoded_receipt->decoded_grid_row_bytes + raw_grid_x;
    receipt.handed_off = 1;
    receipt.raw_grid_x = raw_grid_x;
    receipt.raw_grid_y = raw_grid_y;
    receipt.raw_grid_byte = raw_track02[coordinate_raw_offset];
    receipt.raw_track02_sector = (uint32_t)(coordinate_raw_offset /
        THERON_V1_TRACK02_RAW_SECTOR_BYTES);
    receipt.raw_sector_offset = (uint32_t)(coordinate_raw_offset %
        THERON_V1_TRACK02_RAW_SECTOR_BYTES);
    receipt.raw_track02_offset = (uint32_t)coordinate_raw_offset;
    receipt.status = "initial_envelope_raw_grid_coordinate_handoff_no_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_raw_grid_row(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint16_t raw_grid_y,
    Theron_V1Track02RawGridRowReceipt *out_receipt) {
    size_t grid_raw_offset;
    size_t row_raw_offset;
    const uint8_t *envelope;
    const uint8_t *grid;
    uint32_t cue_index01_raw_sector;
    Theron_V1Track02RawGridRowReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!decoded_receipt || !raw_track02 || !raw_track02_md5 || !out_receipt ||
        !decoded_receipt->observed ||
        !decoded_receipt->payload_intake_admitted ||
        !decoded_receipt->initial_envelope_source_bound ||
        !decoded_receipt->initial_envelope_decoded ||
        decoded_receipt->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        decoded_receipt->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        decoded_receipt->decoded_header_width !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_header_height !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        decoded_receipt->decoded_header_seed !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_SEED ||
        decoded_receipt->decoded_header_identifier !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER ||
        decoded_receipt->decoded_grid_row_count !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        decoded_receipt->decoded_grid_row_bytes !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_grid_bytes !=
            (uint32_t)THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT *
                THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_grid_raw_sector_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET +
                THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                TQR_INITIAL_ENVELOPE_HEADER_BYTES ||
        raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        (strcmp(raw_track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0 &&
         strcmp(raw_track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) ||
        !theron_v1_track02_raw_bytes_match_md5(raw_track02,
                                                raw_track02_bytes,
                                                raw_track02_md5) ||
        raw_grid_y >= decoded_receipt->decoded_grid_row_count) {
        return 0;
    }

    cue_index01_raw_sector = strcmp(raw_track02_md5,
        THERON_V1_TRACK02_MD5_JP_BIN) == 0 ? 224u : 225u;
    if (decoded_receipt->decoded_grid_raw_sector !=
            cue_index01_raw_sector + decoded_receipt->record) {
        return 0;
    }

    grid_raw_offset = (size_t)decoded_receipt->decoded_grid_raw_sector *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES +
        decoded_receipt->decoded_grid_raw_sector_offset;
    if (grid_raw_offset < TQR_INITIAL_ENVELOPE_HEADER_BYTES ||
        grid_raw_offset > raw_track02_bytes ||
        decoded_receipt->decoded_grid_bytes >
            raw_track02_bytes - grid_raw_offset) {
        return 0;
    }

    envelope = raw_track02 + grid_raw_offset - TQR_INITIAL_ENVELOPE_HEADER_BYTES;
    grid = raw_track02 + grid_raw_offset;
    if (read_be16(envelope) != decoded_receipt->decoded_header_width ||
        read_be16(envelope + 2u) != decoded_receipt->decoded_header_height ||
        read_be32(envelope + 4u) != decoded_receipt->decoded_header_seed ||
        read_be16(envelope + 8u) != decoded_receipt->decoded_header_identifier ||
        read_be16(envelope + 10u) != decoded_receipt->decoded_header_extension ||
        hash_bytes(grid, decoded_receipt->decoded_grid_bytes) !=
            decoded_receipt->decoded_grid_hash) {
        return 0;
    }

    row_raw_offset = grid_raw_offset +
        (size_t)raw_grid_y * decoded_receipt->decoded_grid_row_bytes;
    receipt.handed_off = 1;
    receipt.raw_grid_y = raw_grid_y;
    receipt.raw_grid_bytes = decoded_receipt->decoded_grid_row_bytes;
    memcpy(receipt.raw_grid_row, raw_track02 + row_raw_offset,
           receipt.raw_grid_bytes);
    receipt.raw_track02_sector = (uint32_t)(row_raw_offset /
        THERON_V1_TRACK02_RAW_SECTOR_BYTES);
    receipt.raw_sector_offset = (uint32_t)(row_raw_offset %
        THERON_V1_TRACK02_RAW_SECTOR_BYTES);
    receipt.raw_track02_offset = (uint32_t)row_raw_offset;
    receipt.raw_grid_row_hash = hash_bytes(receipt.raw_grid_row,
                                           receipt.raw_grid_bytes);
    receipt.status = "initial_envelope_raw_grid_row_handoff_no_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_raw_grid(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02RawGridReceipt *out_receipt) {
    Theron_V1Track02RawGridRowReceipt row;
    Theron_V1Track02RawGridReceipt receipt = {0};
    uint16_t y;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!decoded_receipt || !out_receipt ||
        decoded_receipt->decoded_grid_row_count !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        decoded_receipt->decoded_grid_row_bytes !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        decoded_receipt->decoded_grid_bytes !=
            (uint32_t)THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT *
                THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        !theron_v1_track02_loader_intake_handoff_raw_grid_row(
            decoded_receipt, raw_track02, raw_track02_bytes, raw_track02_md5,
            0u, &row)) {
        return 0;
    }

    receipt.handed_off = 1;
    receipt.raw_grid_width = decoded_receipt->decoded_grid_row_bytes;
    receipt.raw_grid_height = decoded_receipt->decoded_grid_row_count;
    receipt.raw_grid_bytes = decoded_receipt->decoded_grid_bytes;
    receipt.raw_track02_sector = row.raw_track02_sector;
    receipt.raw_sector_offset = row.raw_sector_offset;
    receipt.raw_track02_offset = row.raw_track02_offset;
    memcpy(receipt.raw_grid, row.raw_grid_row, row.raw_grid_bytes);
    for (y = 1u; y < receipt.raw_grid_height; ++y) {
        if (!theron_v1_track02_loader_intake_handoff_raw_grid_row(
                decoded_receipt, raw_track02, raw_track02_bytes,
                raw_track02_md5, y, &row) ||
            row.raw_grid_bytes != receipt.raw_grid_width ||
            row.raw_track02_offset != receipt.raw_track02_offset +
                (uint32_t)y * receipt.raw_grid_width) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
        memcpy(receipt.raw_grid + (size_t)y * receipt.raw_grid_width,
               row.raw_grid_row, row.raw_grid_bytes);
    }
    receipt.raw_grid_hash = hash_bytes(receipt.raw_grid, receipt.raw_grid_bytes);
    if (receipt.raw_grid_hash != decoded_receipt->decoded_grid_hash) {
        return 0;
    }
    receipt.status = "initial_envelope_raw_grid_handoff_no_semantics";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02RawGridConsumer consumer,
    void *consumer_context,
    Theron_V1Track02RawGridRuntimeReceipt *out_receipt) {
    Theron_V1Track02RawGridReceipt grid;
    Theron_V1Track02RawGridRuntimeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!decoded_receipt || !raw_track02 || !raw_track02_md5 || !consumer ||
        !out_receipt ||
        !theron_v1_track02_loader_intake_handoff_raw_grid(
            decoded_receipt, raw_track02, raw_track02_bytes, raw_track02_md5,
            &grid)) {
        return 0;
    }

    /* The consumer sees only the exact rehashed grid receipt. It cannot cause
     * a fallback route: refusal leaves this entrypoint with a zero receipt. */
    if (!consumer(&grid, consumer_context)) return 0;

    receipt.delivered = 1;
    receipt.no_fallback = 1;
    receipt.raw_grid_width = grid.raw_grid_width;
    receipt.raw_grid_height = grid.raw_grid_height;
    receipt.raw_grid_bytes = grid.raw_grid_bytes;
    receipt.raw_grid_hash = grid.raw_grid_hash;
    receipt.raw_track02_sector = grid.raw_track02_sector;
    receipt.raw_sector_offset = grid.raw_sector_offset;
    receipt.raw_track02_offset = grid.raw_track02_offset;
    receipt.status = "initial_envelope_raw_grid_delivered_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_observe_authenticated_trace(
    const Theron_V1AuthenticatedTrack02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderReadFacts observation;

    if (!facts || !facts->trace_provenance ||
        !facts->trace_provenance->valid ||
        !facts->trace_provenance->runtime_admitted) {
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
        initial_envelope->header_width !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        initial_envelope->header_height !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        initial_envelope->header_seed !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_SEED ||
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
