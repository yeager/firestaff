#include "theron_v1_track02_loader_intake.h"

#include <string.h>

enum {
    TQR_INITIAL_ENVELOPE_HEADER_BYTES = 12u,
    TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR = 3122u,
    TQR_US_INITIAL_ENVELOPE_RAW_SECTOR = 3123u,
    TQR_JP_CUE_INDEX01_RAW_SECTOR = 224u,
    TQR_US_CUE_INDEX01_RAW_SECTOR = 225u,
    TQR_MODE1_USER_DATA_BYTES = 2048u
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

static int valid_later_read_raw_media_gate_receipt(
    const Theron_V1Track02LaterReadRawMediaGateReceipt *gate);
static int valid_object_dungeon_layout_bytes_pair_receipt(
    const Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *pair);

static int valid_raw_grid_receipt(const Theron_V1Track02RawGridReceipt *grid) {
    if (!grid || !grid->handed_off || !grid->authenticated_v3_trace ||
        grid->raw_grid_width != THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        grid->raw_grid_height != THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        grid->raw_grid_bytes !=
            (uint32_t)THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH *
                THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        grid->raw_grid_hash == 0u ||
        hash_bytes(grid->raw_grid, grid->raw_grid_bytes) !=
            grid->raw_grid_hash ||
        (grid->raw_track02_sector != TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR &&
         grid->raw_track02_sector != TQR_US_INITIAL_ENVELOPE_RAW_SECTOR) ||
        grid->raw_sector_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET +
                THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                TQR_INITIAL_ENVELOPE_HEADER_BYTES ||
        grid->raw_track02_offset !=
            grid->raw_track02_sector * THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                grid->raw_sector_offset ||
        !grid->status ||
        strcmp(grid->status,
               "initial_envelope_raw_grid_handoff_no_semantics") != 0) {
        return 0;
    }
    return 1;
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
        !decoded_receipt->authenticated_v3_trace ||
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
    receipt.authenticated_v3_trace = 1;
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
        !decoded_receipt->authenticated_v3_trace ||
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
    receipt.authenticated_v3_trace = 1;
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
    receipt.authenticated_v3_trace = 1;
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
    receipt.authenticated_v3_trace = 1;
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

int theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridObjectTableProjectionReceipt *out_receipt) {
    Theron_V1Track02RawGridObjectTableProjectionReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.projection_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.no_fallback = 1;
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_raw_grid_object_table_projection_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_block_object_table_read_claim(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *candidate_read,
    Theron_V1Track02ObjectTableReadBlockReceipt *out_receipt) {
    Theron_V1Track02ObjectTableReadBlockReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.object_table_read_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.separate_object_table_record_required = 1;
    receipt.later_loader_read_required = 1;
    receipt.no_fallback = 1;
    receipt.startup_grid_record =
        grid->raw_track02_sector -
        (grid->raw_track02_sector == TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR ?
         224u : 225u);
    if (candidate_read) {
        receipt.candidate_read_seen = 1;
        receipt.candidate_read_authenticated =
            candidate_read->authenticated_original_trace &&
            candidate_read->later_than_stage2_transfer;
        receipt.candidate_record = candidate_read->track02_record;
        receipt.candidate_record_user_data_offset =
            candidate_read->record_user_data_offset;
        receipt.candidate_destination = candidate_read->destination;
        receipt.candidate_byte_count = candidate_read->byte_count;
        receipt.startup_record_rejected_as_object_table =
            candidate_read->track02_record == THERON_V1_INITIAL_ENVELOPE_RECORD &&
            candidate_read->record_user_data_offset ==
                THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    }
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_object_table_read_claim_blocked_separate_later_read_required_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridBitmapRouteReceipt *out_receipt) {
    Theron_V1Track02RawGridBitmapRouteReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.bitmap_route_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.no_fallback = 1;
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_raw_grid_bitmap_route_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_block_bitmap_read_claim(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *candidate_read,
    Theron_V1Track02BitmapReadBlockReceipt *out_receipt) {
    Theron_V1Track02BitmapReadBlockReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.bitmap_read_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.separate_bitmap_record_required = 1;
    receipt.later_loader_read_required = 1;
    receipt.palette_binding_required = 1;
    receipt.no_fallback_visual = 1;
    receipt.startup_grid_record =
        grid->raw_track02_sector -
        (grid->raw_track02_sector == TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR ?
         224u : 225u);
    if (candidate_read) {
        receipt.candidate_read_seen = 1;
        receipt.candidate_read_authenticated =
            candidate_read->authenticated_original_trace &&
            candidate_read->later_than_stage2_transfer;
        receipt.candidate_record = candidate_read->track02_record;
        receipt.candidate_record_user_data_offset =
            candidate_read->record_user_data_offset;
        receipt.candidate_destination = candidate_read->destination;
        receipt.candidate_byte_count = candidate_read->byte_count;
        receipt.startup_record_rejected_as_bitmap =
            candidate_read->track02_record == THERON_V1_INITIAL_ENVELOPE_RECORD &&
            candidate_read->record_user_data_offset ==
                THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    }
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_bitmap_read_claim_blocked_separate_later_read_palette_required_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_admit_raw_grid_level_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridLevelRouteReceipt *out_receipt) {
    Theron_V1Track02RawGridLevelRouteReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.level_route_admitted = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.bitmap_route_blocked = 1;
    receipt.object_route_blocked = 1;
    receipt.no_fallback = 1;
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_raw_grid_level_route_bitmap_object_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridDungeonRouteReceipt *out_receipt) {
    Theron_V1Track02RawGridDungeonRouteReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.dungeon_route_admitted = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.bitmap_route_blocked = 1;
    receipt.object_route_blocked = 1;
    receipt.no_fallback = 1;
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_raw_grid_dungeon_route_bitmap_object_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridDungeonRecordEvidenceReceipt *out_receipt) {
    Theron_V1Track02RawGridDungeonRecordEvidenceReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.dungeon_record_blocked = 1;
    receipt.object_table_record_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.later_loader_read_required = 1;
    receipt.no_fallback = 1;
    receipt.expected_dungeon_record = THERON_V1_INITIAL_ENVELOPE_RECORD;
    receipt.observed_raw_grid_record =
        grid->raw_track02_sector -
        (grid->raw_track02_sector == TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR ?
         224u : 225u);
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_raw_grid_dungeon_record_object_table_blocked_later_read_required_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_block_dungeon_read_claim(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *candidate_read,
    Theron_V1Track02DungeonReadBlockReceipt *out_receipt) {
    Theron_V1Track02DungeonReadBlockReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.dungeon_read_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.separate_dungeon_record_required = 1;
    receipt.later_loader_read_required = 1;
    receipt.grammar_binding_required = 1;
    receipt.no_fallback_dungeon = 1;
    receipt.startup_grid_record =
        grid->raw_track02_sector -
        (grid->raw_track02_sector == TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR ?
         224u : 225u);
    if (candidate_read) {
        receipt.candidate_read_seen = 1;
        receipt.candidate_read_authenticated =
            candidate_read->authenticated_original_trace &&
            candidate_read->later_than_stage2_transfer;
        receipt.candidate_record = candidate_read->track02_record;
        receipt.candidate_record_user_data_offset =
            candidate_read->record_user_data_offset;
        receipt.candidate_destination = candidate_read->destination;
        receipt.candidate_byte_count = candidate_read->byte_count;
        receipt.startup_record_rejected_as_dungeon =
            candidate_read->track02_record == THERON_V1_INITIAL_ENVELOPE_RECORD &&
            candidate_read->record_user_data_offset ==
                THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
    }
    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_dungeon_read_claim_blocked_separate_later_read_grammar_required_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *object_read,
    const Theron_V1Track02LoaderReadFacts *dungeon_read,
    Theron_V1Track02ObjectDungeonHandoffGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonHandoffGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !valid_raw_grid_receipt(grid)) {
        return 0;
    }

    receipt.handoff_blocked = 1;
    receipt.authenticated_v3_trace = 1;
    receipt.object_decoder_binding_required = 1;
    receipt.dungeon_grammar_binding_required = 1;
    receipt.object_handoff_blocked = 1;
    receipt.dungeon_handoff_blocked = 1;
    receipt.no_fallback_visuals = 1;
    receipt.no_synthetic_handoff = 1;
    receipt.startup_grid_record =
        grid->raw_track02_sector -
        (grid->raw_track02_sector == TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR ?
         224u : 225u);

    if (object_read) {
        receipt.object_read_seen = 1;
        receipt.object_read_authenticated =
            object_read->authenticated_original_trace &&
            object_read->later_than_stage2_transfer;
        receipt.object_candidate_record = object_read->track02_record;
        receipt.object_candidate_record_user_data_offset =
            object_read->record_user_data_offset;
        receipt.object_candidate_destination = object_read->destination;
        receipt.object_candidate_byte_count = object_read->byte_count;
        receipt.object_startup_record_rejected =
            object_read->track02_record == THERON_V1_INITIAL_ENVELOPE_RECORD &&
            object_read->record_user_data_offset ==
                THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
        receipt.object_later_read_proven =
            receipt.object_read_authenticated &&
            !receipt.object_startup_record_rejected &&
            object_read->byte_count != 0u;
    }

    if (dungeon_read) {
        receipt.dungeon_read_seen = 1;
        receipt.dungeon_read_authenticated =
            dungeon_read->authenticated_original_trace &&
            dungeon_read->later_than_stage2_transfer;
        receipt.dungeon_candidate_record = dungeon_read->track02_record;
        receipt.dungeon_candidate_record_user_data_offset =
            dungeon_read->record_user_data_offset;
        receipt.dungeon_candidate_destination = dungeon_read->destination;
        receipt.dungeon_candidate_byte_count = dungeon_read->byte_count;
        receipt.dungeon_startup_record_rejected =
            dungeon_read->track02_record == THERON_V1_INITIAL_ENVELOPE_RECORD &&
            dungeon_read->record_user_data_offset ==
                THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET;
        receipt.dungeon_later_read_proven =
            receipt.dungeon_read_authenticated &&
            !receipt.dungeon_startup_record_rejected &&
            dungeon_read->byte_count != 0u;
    }

    receipt.raw_grid_width = grid->raw_grid_width;
    receipt.raw_grid_height = grid->raw_grid_height;
    receipt.raw_grid_bytes = grid->raw_grid_bytes;
    receipt.raw_grid_hash = grid->raw_grid_hash;
    receipt.raw_track02_sector = grid->raw_track02_sector;
    receipt.raw_sector_offset = grid->raw_sector_offset;
    receipt.raw_track02_offset = grid->raw_track02_offset;
    receipt.status =
        "initial_envelope_object_dungeon_handoff_blocked_later_reads_decoder_grammar_required_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_admit_later_cd_record_read(
    const Theron_V1Track02RawCueAdmissionReceipt *raw_cue,
    const Theron_V1Track02LoaderReadFacts *read,
    Theron_V1Track02LaterReadCdRecordReceipt *out_receipt) {
    Theron_V1Track02LaterReadCdRecordReceipt receipt = {0};
    uint32_t cue_index01_sector;
    uint32_t raw_sector_offset;
    size_t raw_track02_offset;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !raw_cue || !read || !raw_cue->admitted ||
        !raw_cue->raw_bin_admitted || !raw_cue->cue_index01_admitted ||
        !raw_cue->iso_image_blocked || !raw_cue->no_fallback ||
        !raw_cue->track02_md5 ||
        !read->authenticated_original_trace ||
        !read->later_than_stage2_transfer ||
        read->byte_count == 0u ||
        read->record_user_data_offset >= TQR_MODE1_USER_DATA_BYTES ||
        read->byte_count >
            TQR_MODE1_USER_DATA_BYTES - read->record_user_data_offset) {
        return 0;
    }

    if (raw_cue->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN &&
        raw_cue->cue_track02_index01_raw_sector ==
            TQR_JP_CUE_INDEX01_RAW_SECTOR &&
        strcmp(raw_cue->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0) {
        cue_index01_sector = TQR_JP_CUE_INDEX01_RAW_SECTOR;
    } else if (raw_cue->raw_track02_variant ==
                   THERON_V1_TRACK02_VARIANT_US_BIN &&
               raw_cue->cue_track02_index01_raw_sector ==
                   TQR_US_CUE_INDEX01_RAW_SECTOR &&
               strcmp(raw_cue->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) ==
                   0) {
        cue_index01_sector = TQR_US_CUE_INDEX01_RAW_SECTOR;
    } else {
        return 0;
    }

    raw_sector_offset = THERON_V1_TRACK02_MODE1_HEADER_BYTES +
        read->record_user_data_offset;
    if (read->track02_record == THERON_V1_INITIAL_ENVELOPE_RECORD &&
        read->record_user_data_offset ==
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET) {
        receipt.startup_record_rejected = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    raw_track02_offset = (size_t)(cue_index01_sector + read->track02_record) *
        THERON_V1_TRACK02_RAW_SECTOR_BYTES + raw_sector_offset;
    if (raw_track02_offset > raw_cue->raw_track02_bytes ||
        read->byte_count > raw_cue->raw_track02_bytes - raw_track02_offset) {
        return 0;
    }

    receipt.cd_record_read_proven = 1;
    receipt.raw_cue_admission_consumed = 1;
    receipt.authenticated_later_loader_read = 1;
    receipt.object_semantics_blocked = 1;
    receipt.dungeon_semantics_blocked = 1;
    receipt.decoder_binding_required = 1;
    receipt.grammar_binding_required = 1;
    receipt.no_fallback_visuals = 1;
    receipt.no_synthetic_handoff = 1;
    receipt.raw_track02_variant = raw_cue->raw_track02_variant;
    receipt.track02_record = read->track02_record;
    receipt.record_user_data_offset = read->record_user_data_offset;
    receipt.destination = read->destination;
    receipt.byte_count = read->byte_count;
    receipt.raw_track02_sector = cue_index01_sector + read->track02_record;
    receipt.raw_sector_offset = raw_sector_offset;
    receipt.raw_track02_offset = (uint32_t)raw_track02_offset;
    receipt.track02_md5 = raw_cue->track02_md5;
    receipt.status =
        "later_loader_cd_record_read_proven_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_raw_cue_admission_receipt(
    const Theron_V1Track02RawCueAdmissionReceipt *raw_cue) {
    if (!raw_cue || !raw_cue->admitted || !raw_cue->raw_bin_admitted ||
        !raw_cue->cue_index01_admitted || !raw_cue->iso_image_blocked ||
        !raw_cue->no_fallback || raw_cue->raw_track02_bytes == 0u ||
        raw_cue->raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        !raw_cue->track02_md5 || !raw_cue->status ||
        strcmp(raw_cue->status,
               "raw_track02_bin_cue_admitted_iso_blocked_no_fallback") != 0) {
        return 0;
    }

    if (raw_cue->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return raw_cue->cue_track02_index01_raw_sector ==
                TQR_JP_CUE_INDEX01_RAW_SECTOR &&
            strcmp(raw_cue->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (raw_cue->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return raw_cue->cue_track02_index01_raw_sector ==
                TQR_US_CUE_INDEX01_RAW_SECTOR &&
            strcmp(raw_cue->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_gate_later_read_raw_media(
    const Theron_V1Track02RawCueAdmissionReceipt *raw_cue,
    Theron_V1Track02LaterReadRawMediaGateReceipt *out_receipt) {
    Theron_V1Track02LaterReadRawMediaGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt) return 0;

    receipt.gate_evaluated = 1;
    receipt.canonical_raw_bin_required = 1;
    receipt.iso_image_blocked = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (!raw_cue) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "later_loader_raw_track02_media_missing_handoff_blocked_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_raw_cue_admission_receipt(raw_cue)) {
        return 0;
    }

    receipt.raw_media_bound = 1;
    receipt.raw_cue_admission_consumed = 1;
    receipt.canonical_raw_bin_present = 1;
    receipt.raw_track02_variant = raw_cue->raw_track02_variant;
    receipt.cue_track02_index01_raw_sector =
        raw_cue->cue_track02_index01_raw_sector;
    receipt.raw_track02_bytes = raw_cue->raw_track02_bytes;
    receipt.track02_md5 = raw_cue->track02_md5;
    receipt.status =
        "later_loader_raw_track02_media_bound_handoff_still_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_later_cd_record_receipt(
    const Theron_V1Track02LaterReadCdRecordReceipt *cd_record) {
    if (!cd_record ||
        !cd_record->cd_record_read_proven ||
        !cd_record->raw_cue_admission_consumed ||
        !cd_record->authenticated_later_loader_read ||
        cd_record->startup_record_rejected ||
        !cd_record->object_semantics_blocked ||
        !cd_record->dungeon_semantics_blocked ||
        !cd_record->decoder_binding_required ||
        !cd_record->grammar_binding_required ||
        !cd_record->no_fallback_visuals ||
        !cd_record->no_synthetic_handoff ||
        cd_record->byte_count == 0u ||
        cd_record->record_user_data_offset >= TQR_MODE1_USER_DATA_BYTES ||
        cd_record->byte_count >
            TQR_MODE1_USER_DATA_BYTES - cd_record->record_user_data_offset ||
        cd_record->raw_sector_offset !=
            THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                cd_record->record_user_data_offset ||
        cd_record->raw_track02_offset !=
            cd_record->raw_track02_sector *
                THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                cd_record->raw_sector_offset ||
        !cd_record->track02_md5 ||
        !cd_record->status ||
        strcmp(cd_record->status,
               "later_loader_cd_record_read_proven_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (cd_record->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(cd_record->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0 &&
            cd_record->raw_track02_sector ==
                TQR_JP_CUE_INDEX01_RAW_SECTOR + cd_record->track02_record;
    }
    if (cd_record->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(cd_record->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0 &&
            cd_record->raw_track02_sector ==
                TQR_US_CUE_INDEX01_RAW_SECTOR + cd_record->track02_record;
    }
    return 0;
}

int theron_v1_track02_loader_intake_bind_object_dungeon_loader_read_table(
    const Theron_V1Track02LaterReadRawMediaGateReceipt *raw_media_gate,
    const Theron_V1Track02LaterReadCdRecordReceipt *object_read,
    const Theron_V1Track02LaterReadCdRecordReceipt *dungeon_read,
    Theron_V1Track02ObjectDungeonLoaderReadTableReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonLoaderReadTableReceipt receipt = {0};
    size_t object_begin, object_end, dungeon_begin, dungeon_end;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_later_read_raw_media_gate_receipt(raw_media_gate)) {
        return 0;
    }

    receipt.raw_media_gate_consumed = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_handoff = 1;

    if (raw_media_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_loader_read_table_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_later_cd_record_receipt(object_read) ||
        !valid_later_cd_record_receipt(dungeon_read) ||
        object_read->raw_track02_variant != raw_media_gate->raw_track02_variant ||
        dungeon_read->raw_track02_variant != raw_media_gate->raw_track02_variant ||
        strcmp(object_read->track02_md5, raw_media_gate->track02_md5) != 0 ||
        strcmp(dungeon_read->track02_md5, raw_media_gate->track02_md5) != 0 ||
        object_read->track02_record == dungeon_read->track02_record) {
        return 0;
    }

    object_begin = object_read->raw_track02_offset;
    object_end = object_begin + object_read->byte_count;
    dungeon_begin = dungeon_read->raw_track02_offset;
    dungeon_end = dungeon_begin + dungeon_read->byte_count;
    if (object_end < object_begin || dungeon_end < dungeon_begin ||
        object_end > raw_media_gate->raw_track02_bytes ||
        dungeon_end > raw_media_gate->raw_track02_bytes ||
        !(object_end <= dungeon_begin || dungeon_end <= object_begin)) {
        return 0;
    }

    receipt.table_bound = 1;
    receipt.object_cd_record_consumed = 1;
    receipt.dungeon_cd_record_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.distinct_records = 1;
    receipt.non_overlapping_raw_windows = 1;
    receipt.raw_track02_variant = raw_media_gate->raw_track02_variant;
    receipt.object_track02_record = object_read->track02_record;
    receipt.object_record_user_data_offset =
        object_read->record_user_data_offset;
    receipt.object_destination = object_read->destination;
    receipt.object_byte_count = object_read->byte_count;
    receipt.object_raw_track02_offset = object_read->raw_track02_offset;
    receipt.dungeon_track02_record = dungeon_read->track02_record;
    receipt.dungeon_record_user_data_offset =
        dungeon_read->record_user_data_offset;
    receipt.dungeon_destination = dungeon_read->destination;
    receipt.dungeon_byte_count = dungeon_read->byte_count;
    receipt.dungeon_raw_track02_offset = dungeon_read->raw_track02_offset;
    receipt.track02_md5 = raw_media_gate->track02_md5;
    receipt.status =
        "object_dungeon_loader_read_table_bound_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_bind_later_read_layout(
    const Theron_V1Track02LaterReadCdRecordReceipt *cd_record,
    Theron_V1Track02LayoutRole role,
    Theron_V1Track02LaterReadLayoutReceipt *out_receipt) {
    Theron_V1Track02LaterReadLayoutReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !cd_record ||
        !cd_record->cd_record_read_proven ||
        !cd_record->raw_cue_admission_consumed ||
        !cd_record->authenticated_later_loader_read ||
        cd_record->startup_record_rejected ||
        !cd_record->object_semantics_blocked ||
        !cd_record->dungeon_semantics_blocked ||
        !cd_record->decoder_binding_required ||
        !cd_record->grammar_binding_required ||
        !cd_record->no_fallback_visuals ||
        !cd_record->no_synthetic_handoff ||
        cd_record->byte_count == 0u ||
        cd_record->record_user_data_offset >= TQR_MODE1_USER_DATA_BYTES ||
        cd_record->byte_count >
            TQR_MODE1_USER_DATA_BYTES -
                cd_record->record_user_data_offset ||
        cd_record->raw_sector_offset !=
            THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                cd_record->record_user_data_offset ||
        cd_record->raw_track02_offset !=
            cd_record->raw_track02_sector *
                THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                cd_record->raw_sector_offset ||
        !cd_record->track02_md5 ||
        !cd_record->status ||
        strcmp(cd_record->status,
               "later_loader_cd_record_read_proven_semantics_blocked_no_fallback") != 0 ||
        (role != THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE &&
         role != THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD)) {
        return 0;
    }

    if (cd_record->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        if (strcmp(cd_record->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0 ||
            cd_record->raw_track02_sector !=
                TQR_JP_CUE_INDEX01_RAW_SECTOR + cd_record->track02_record) {
            return 0;
        }
    } else if (cd_record->raw_track02_variant ==
                   THERON_V1_TRACK02_VARIANT_US_BIN) {
        if (strcmp(cd_record->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0 ||
            cd_record->raw_track02_sector !=
                TQR_US_CUE_INDEX01_RAW_SECTOR + cd_record->track02_record) {
            return 0;
        }
    } else {
        return 0;
    }

    receipt.layout_window_bound = 1;
    receipt.cd_record_read_consumed = 1;
    receipt.object_layout_bound =
        role == THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE;
    receipt.dungeon_layout_bound =
        role == THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD;
    receipt.parser_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.decoder_or_grammar_required = 1;
    receipt.no_synthetic_layout = 1;
    receipt.role = role;
    receipt.raw_track02_variant = cd_record->raw_track02_variant;
    receipt.track02_record = cd_record->track02_record;
    receipt.record_user_data_offset = cd_record->record_user_data_offset;
    receipt.destination = cd_record->destination;
    receipt.layout_bytes = cd_record->byte_count;
    receipt.raw_track02_sector = cd_record->raw_track02_sector;
    receipt.raw_sector_offset = cd_record->raw_sector_offset;
    receipt.raw_track02_offset = cd_record->raw_track02_offset;
    receipt.track02_md5 = cd_record->track02_md5;
    receipt.status =
        "later_loader_layout_window_bound_parser_runtime_render_blocked";
    *out_receipt = receipt;
    return 1;
}

static int valid_later_read_layout_receipt(
    const Theron_V1Track02LaterReadLayoutReceipt *layout,
    Theron_V1Track02LayoutRole role) {
    if (!layout ||
        !layout->layout_window_bound ||
        !layout->cd_record_read_consumed ||
        !layout->parser_semantics_blocked ||
        !layout->runtime_handoff_blocked ||
        !layout->rendering_blocked ||
        !layout->fallback_visuals_blocked ||
        !layout->decoder_or_grammar_required ||
        !layout->no_synthetic_layout ||
        layout->role != role ||
        layout->object_layout_bound !=
            (role == THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE) ||
        layout->dungeon_layout_bound !=
            (role == THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD) ||
        layout->layout_bytes == 0u ||
        layout->record_user_data_offset >= TQR_MODE1_USER_DATA_BYTES ||
        layout->layout_bytes >
            TQR_MODE1_USER_DATA_BYTES - layout->record_user_data_offset ||
        layout->raw_sector_offset !=
            THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                layout->record_user_data_offset ||
        layout->raw_track02_offset !=
            layout->raw_track02_sector * THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                layout->raw_sector_offset ||
        !layout->track02_md5 ||
        !layout->status ||
        strcmp(layout->status,
               "later_loader_layout_window_bound_parser_runtime_render_blocked") != 0) {
        return 0;
    }

    if (layout->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(layout->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0 &&
            layout->raw_track02_sector ==
                TQR_JP_CUE_INDEX01_RAW_SECTOR + layout->track02_record;
    }
    if (layout->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(layout->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0 &&
            layout->raw_track02_sector ==
                TQR_US_CUE_INDEX01_RAW_SECTOR + layout->track02_record;
    }
    return 0;
}

static int valid_object_dungeon_loader_read_table_receipt(
    const Theron_V1Track02ObjectDungeonLoaderReadTableReceipt *table) {
    size_t object_begin, object_end, dungeon_begin, dungeon_end;

    if (!table ||
        !table->raw_media_gate_consumed ||
        !table->parser_semantics_blocked ||
        !table->dungeon_grammar_blocked ||
        !table->runtime_handoff_blocked ||
        !table->rendering_blocked ||
        !table->fallback_visuals_blocked ||
        !table->no_synthetic_handoff ||
        !table->status) {
        return 0;
    }

    if (table->raw_media_missing_blocked) {
        return !table->table_bound &&
            !table->object_cd_record_consumed &&
            !table->dungeon_cd_record_consumed &&
            table->track02_md5 == NULL &&
            strcmp(table->status,
                   "object_dungeon_loader_read_table_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!table->table_bound ||
        !table->object_cd_record_consumed ||
        !table->dungeon_cd_record_consumed ||
        !table->same_track02_media ||
        !table->distinct_records ||
        !table->non_overlapping_raw_windows ||
        table->object_byte_count == 0u ||
        table->dungeon_byte_count == 0u ||
        !table->track02_md5 ||
        strcmp(table->status,
               "object_dungeon_loader_read_table_bound_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (table->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        if (strcmp(table->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0) {
            return 0;
        }
    } else if (table->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        if (strcmp(table->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) {
            return 0;
        }
    } else {
        return 0;
    }

    object_begin = table->object_raw_track02_offset;
    object_end = object_begin + table->object_byte_count;
    dungeon_begin = table->dungeon_raw_track02_offset;
    dungeon_end = dungeon_begin + table->dungeon_byte_count;
    return object_end >= object_begin && dungeon_end >= dungeon_begin &&
        (object_end <= dungeon_begin || dungeon_end <= object_begin);
}

int theron_v1_track02_loader_intake_bind_read_table_to_layouts(
    const Theron_V1Track02ObjectDungeonLoaderReadTableReceipt *read_table,
    const Theron_V1Track02LaterReadLayoutReceipt *object_layout,
    const Theron_V1Track02LaterReadLayoutReceipt *dungeon_layout,
    Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_object_dungeon_loader_read_table_receipt(read_table)) {
        return 0;
    }

    receipt.read_table_consumed = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_layout = 1;

    if (read_table->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_read_table_layout_binding_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_later_read_layout_receipt(
            object_layout, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE) ||
        !valid_later_read_layout_receipt(
            dungeon_layout, THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD) ||
        object_layout->raw_track02_variant != read_table->raw_track02_variant ||
        dungeon_layout->raw_track02_variant != read_table->raw_track02_variant ||
        strcmp(object_layout->track02_md5, read_table->track02_md5) != 0 ||
        strcmp(dungeon_layout->track02_md5, read_table->track02_md5) != 0 ||
        object_layout->track02_record != read_table->object_track02_record ||
        object_layout->record_user_data_offset !=
            read_table->object_record_user_data_offset ||
        object_layout->destination != read_table->object_destination ||
        object_layout->layout_bytes != read_table->object_byte_count ||
        object_layout->raw_track02_offset !=
            read_table->object_raw_track02_offset ||
        dungeon_layout->track02_record != read_table->dungeon_track02_record ||
        dungeon_layout->record_user_data_offset !=
            read_table->dungeon_record_user_data_offset ||
        dungeon_layout->destination != read_table->dungeon_destination ||
        dungeon_layout->layout_bytes != read_table->dungeon_byte_count ||
        dungeon_layout->raw_track02_offset !=
            read_table->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.binding_bound = 1;
    receipt.object_layout_consumed = 1;
    receipt.dungeon_layout_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.destinations_preserved = 1;
    receipt.layout_windows_match_reads = 1;
    receipt.raw_track02_variant = read_table->raw_track02_variant;
    receipt.object_track02_record = read_table->object_track02_record;
    receipt.object_record_user_data_offset =
        read_table->object_record_user_data_offset;
    receipt.object_destination = read_table->object_destination;
    receipt.object_layout_bytes = read_table->object_byte_count;
    receipt.object_raw_track02_offset = read_table->object_raw_track02_offset;
    receipt.dungeon_track02_record = read_table->dungeon_track02_record;
    receipt.dungeon_record_user_data_offset =
        read_table->dungeon_record_user_data_offset;
    receipt.dungeon_destination = read_table->dungeon_destination;
    receipt.dungeon_layout_bytes = read_table->dungeon_byte_count;
    receipt.dungeon_raw_track02_offset = read_table->dungeon_raw_track02_offset;
    receipt.track02_md5 = read_table->track02_md5;
    receipt.status =
        "object_dungeon_read_table_layout_binding_bound_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
    const Theron_V1Track02LaterReadLayoutReceipt *object_layout,
    const Theron_V1Track02LaterReadLayoutReceipt *dungeon_layout,
    Theron_V1Track02ObjectDungeonLayoutPairReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonLayoutPairReceipt receipt = {0};
    size_t object_begin, object_end, dungeon_begin, dungeon_end;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_later_read_layout_receipt(
            object_layout, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE) ||
        !valid_later_read_layout_receipt(
            dungeon_layout, THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD) ||
        object_layout->raw_track02_variant !=
            dungeon_layout->raw_track02_variant ||
        strcmp(object_layout->track02_md5, dungeon_layout->track02_md5) != 0 ||
        object_layout->track02_record == dungeon_layout->track02_record) {
        return 0;
    }

    object_begin = object_layout->raw_track02_offset;
    object_end = object_begin + object_layout->layout_bytes;
    dungeon_begin = dungeon_layout->raw_track02_offset;
    dungeon_end = dungeon_begin + dungeon_layout->layout_bytes;
    if (object_end < object_begin || dungeon_end < dungeon_begin ||
        !(object_end <= dungeon_begin || dungeon_end <= object_begin)) {
        return 0;
    }

    receipt.layout_pair_bound = 1;
    receipt.object_layout_consumed = 1;
    receipt.dungeon_layout_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.distinct_records = 1;
    receipt.non_overlapping_windows = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_layout = 1;
    receipt.raw_track02_variant = object_layout->raw_track02_variant;
    receipt.object_track02_record = object_layout->track02_record;
    receipt.object_record_user_data_offset =
        object_layout->record_user_data_offset;
    receipt.object_layout_bytes = object_layout->layout_bytes;
    receipt.object_raw_track02_offset = object_layout->raw_track02_offset;
    receipt.dungeon_track02_record = dungeon_layout->track02_record;
    receipt.dungeon_record_user_data_offset =
        dungeon_layout->record_user_data_offset;
    receipt.dungeon_layout_bytes = dungeon_layout->layout_bytes;
    receipt.dungeon_raw_track02_offset = dungeon_layout->raw_track02_offset;
    receipt.track02_md5 = object_layout->track02_md5;
    receipt.status =
        "object_dungeon_layout_pair_bound_nonoverlap_render_blocked";
    *out_receipt = receipt;
    return 1;
}

static int valid_read_table_layout_binding_receipt(
    const Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt *binding) {
    if (!binding ||
        !binding->read_table_consumed ||
        !binding->parser_semantics_blocked ||
        !binding->dungeon_grammar_blocked ||
        !binding->runtime_handoff_blocked ||
        !binding->rendering_blocked ||
        !binding->fallback_visuals_blocked ||
        !binding->no_synthetic_layout ||
        !binding->status) {
        return 0;
    }

    if (binding->raw_media_missing_blocked) {
        return !binding->binding_bound &&
            !binding->object_layout_consumed &&
            !binding->dungeon_layout_consumed &&
            binding->track02_md5 == NULL &&
            strcmp(binding->status,
                   "object_dungeon_read_table_layout_binding_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!binding->binding_bound ||
        !binding->object_layout_consumed ||
        !binding->dungeon_layout_consumed ||
        !binding->same_track02_media ||
        !binding->destinations_preserved ||
        !binding->layout_windows_match_reads ||
        binding->object_layout_bytes == 0u ||
        binding->dungeon_layout_bytes == 0u ||
        !binding->track02_md5 ||
        strcmp(binding->status,
               "object_dungeon_read_table_layout_binding_bound_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (binding->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(binding->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (binding->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(binding->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

static int valid_object_dungeon_layout_pair_receipt(
    const Theron_V1Track02ObjectDungeonLayoutPairReceipt *pair) {
    size_t object_begin, object_end, dungeon_begin, dungeon_end;

    if (!pair ||
        !pair->layout_pair_bound ||
        !pair->object_layout_consumed ||
        !pair->dungeon_layout_consumed ||
        !pair->same_track02_media ||
        !pair->distinct_records ||
        !pair->non_overlapping_windows ||
        !pair->parser_semantics_blocked ||
        !pair->runtime_handoff_blocked ||
        !pair->rendering_blocked ||
        !pair->fallback_visuals_blocked ||
        !pair->no_synthetic_layout ||
        pair->object_layout_bytes == 0u ||
        pair->dungeon_layout_bytes == 0u ||
        !pair->track02_md5 ||
        !pair->status ||
        strcmp(pair->status,
               "object_dungeon_layout_pair_bound_nonoverlap_render_blocked") != 0) {
        return 0;
    }

    if (pair->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        if (strcmp(pair->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0) {
            return 0;
        }
    } else if (pair->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        if (strcmp(pair->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) {
            return 0;
        }
    } else {
        return 0;
    }

    object_begin = pair->object_raw_track02_offset;
    object_end = object_begin + pair->object_layout_bytes;
    dungeon_begin = pair->dungeon_raw_track02_offset;
    dungeon_end = dungeon_begin + pair->dungeon_layout_bytes;
    return object_end >= object_begin && dungeon_end >= dungeon_begin &&
        (object_end <= dungeon_begin || dungeon_end <= object_begin);
}

int theron_v1_track02_loader_intake_bridge_read_layout_binding_to_layout_pair(
    const Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt
        *read_layout_binding,
    const Theron_V1Track02ObjectDungeonLayoutPairReceipt *layout_pair,
    Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_read_table_layout_binding_receipt(read_layout_binding)) {
        return 0;
    }

    receipt.read_layout_binding_consumed = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_layout = 1;

    if (read_layout_binding->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_read_layout_pair_bridge_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_object_dungeon_layout_pair_receipt(layout_pair) ||
        layout_pair->raw_track02_variant !=
            read_layout_binding->raw_track02_variant ||
        strcmp(layout_pair->track02_md5,
               read_layout_binding->track02_md5) != 0 ||
        layout_pair->object_track02_record !=
            read_layout_binding->object_track02_record ||
        layout_pair->object_record_user_data_offset !=
            read_layout_binding->object_record_user_data_offset ||
        layout_pair->object_layout_bytes !=
            read_layout_binding->object_layout_bytes ||
        layout_pair->object_raw_track02_offset !=
            read_layout_binding->object_raw_track02_offset ||
        layout_pair->dungeon_track02_record !=
            read_layout_binding->dungeon_track02_record ||
        layout_pair->dungeon_record_user_data_offset !=
            read_layout_binding->dungeon_record_user_data_offset ||
        layout_pair->dungeon_layout_bytes !=
            read_layout_binding->dungeon_layout_bytes ||
        layout_pair->dungeon_raw_track02_offset !=
            read_layout_binding->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.bridge_bound = 1;
    receipt.layout_pair_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.read_windows_preserved = 1;
    receipt.non_overlapping_windows = 1;
    receipt.raw_track02_variant = read_layout_binding->raw_track02_variant;
    receipt.object_track02_record = read_layout_binding->object_track02_record;
    receipt.object_record_user_data_offset =
        read_layout_binding->object_record_user_data_offset;
    receipt.object_destination = read_layout_binding->object_destination;
    receipt.object_layout_bytes = read_layout_binding->object_layout_bytes;
    receipt.object_raw_track02_offset =
        read_layout_binding->object_raw_track02_offset;
    receipt.dungeon_track02_record =
        read_layout_binding->dungeon_track02_record;
    receipt.dungeon_record_user_data_offset =
        read_layout_binding->dungeon_record_user_data_offset;
    receipt.dungeon_destination = read_layout_binding->dungeon_destination;
    receipt.dungeon_layout_bytes = read_layout_binding->dungeon_layout_bytes;
    receipt.dungeon_raw_track02_offset =
        read_layout_binding->dungeon_raw_track02_offset;
    receipt.track02_md5 = read_layout_binding->track02_md5;
    receipt.status =
        "object_dungeon_read_layout_pair_bridge_bound_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_handoff_later_layout_bytes(
    const Theron_V1Track02LaterReadLayoutReceipt *layout,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint8_t *out_bytes,
    size_t out_capacity,
    Theron_V1Track02LaterReadLayoutBytesReceipt *out_receipt) {
    Theron_V1Track02LaterReadLayoutBytesReceipt receipt = {0};
    size_t raw_end;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !layout || !raw_track02 || !raw_track02_md5 ||
        !out_bytes ||
        (layout->role != THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE &&
         layout->role != THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD) ||
        !valid_later_read_layout_receipt(layout, layout->role) ||
        strcmp(raw_track02_md5, layout->track02_md5) != 0 ||
        raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        !theron_v1_track02_raw_bytes_match_md5(raw_track02, raw_track02_bytes,
                                                raw_track02_md5) ||
        layout->layout_bytes > out_capacity ||
        layout->raw_track02_offset > raw_track02_bytes) {
        return 0;
    }

    raw_end = (size_t)layout->raw_track02_offset + layout->layout_bytes;
    if (raw_end < layout->raw_track02_offset || raw_end > raw_track02_bytes) {
        return 0;
    }

    memcpy(out_bytes, raw_track02 + layout->raw_track02_offset,
           layout->layout_bytes);
    receipt.bytes_handed_off = 1;
    receipt.layout_receipt_consumed = 1;
    receipt.object_layout_bytes =
        layout->role == THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE;
    receipt.dungeon_layout_bytes =
        layout->role == THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD;
    receipt.exact_source_bytes = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;
    receipt.role = layout->role;
    receipt.raw_track02_variant = layout->raw_track02_variant;
    receipt.track02_record = layout->track02_record;
    receipt.record_user_data_offset = layout->record_user_data_offset;
    receipt.layout_bytes = layout->layout_bytes;
    receipt.layout_hash = hash_bytes(out_bytes, layout->layout_bytes);
    receipt.raw_track02_sector = layout->raw_track02_sector;
    receipt.raw_sector_offset = layout->raw_sector_offset;
    receipt.raw_track02_offset = layout->raw_track02_offset;
    receipt.track02_md5 = layout->track02_md5;
    receipt.status =
        "later_loader_layout_bytes_handoff_opaque_render_blocked";
    *out_receipt = receipt;
    return 1;
}

static int valid_later_read_layout_bytes_receipt(
    const Theron_V1Track02LaterReadLayoutBytesReceipt *bytes,
    Theron_V1Track02LayoutRole role) {
    if (!bytes ||
        !bytes->bytes_handed_off ||
        !bytes->layout_receipt_consumed ||
        !bytes->exact_source_bytes ||
        !bytes->parser_semantics_blocked ||
        !bytes->runtime_handoff_blocked ||
        !bytes->rendering_blocked ||
        !bytes->fallback_visuals_blocked ||
        !bytes->no_synthetic_bytes ||
        bytes->role != role ||
        bytes->object_layout_bytes !=
            (role == THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE) ||
        bytes->dungeon_layout_bytes !=
            (role == THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD) ||
        bytes->layout_bytes == 0u ||
        bytes->layout_hash == 0u ||
        bytes->record_user_data_offset >= TQR_MODE1_USER_DATA_BYTES ||
        bytes->layout_bytes >
            TQR_MODE1_USER_DATA_BYTES - bytes->record_user_data_offset ||
        bytes->raw_sector_offset !=
            THERON_V1_TRACK02_MODE1_HEADER_BYTES +
                bytes->record_user_data_offset ||
        bytes->raw_track02_offset !=
            bytes->raw_track02_sector * THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                bytes->raw_sector_offset ||
        !bytes->track02_md5 ||
        !bytes->status ||
        strcmp(bytes->status,
               "later_loader_layout_bytes_handoff_opaque_render_blocked") != 0) {
        return 0;
    }

    if (bytes->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(bytes->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0 &&
            bytes->raw_track02_sector ==
                TQR_JP_CUE_INDEX01_RAW_SECTOR + bytes->track02_record;
    }
    if (bytes->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(bytes->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0 &&
            bytes->raw_track02_sector ==
                TQR_US_CUE_INDEX01_RAW_SECTOR + bytes->track02_record;
    }
    return 0;
}

int theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
    const Theron_V1Track02ObjectDungeonLayoutPairReceipt *layout_pair,
    const Theron_V1Track02LaterReadLayoutBytesReceipt *object_bytes,
    const Theron_V1Track02LaterReadLayoutBytesReceipt *dungeon_bytes,
    Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt receipt = {0};
    size_t object_begin, object_end, dungeon_begin, dungeon_end;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !layout_pair ||
        !layout_pair->layout_pair_bound ||
        !layout_pair->object_layout_consumed ||
        !layout_pair->dungeon_layout_consumed ||
        !layout_pair->same_track02_media ||
        !layout_pair->distinct_records ||
        !layout_pair->non_overlapping_windows ||
        !layout_pair->parser_semantics_blocked ||
        !layout_pair->runtime_handoff_blocked ||
        !layout_pair->rendering_blocked ||
        !layout_pair->fallback_visuals_blocked ||
        !layout_pair->no_synthetic_layout ||
        !layout_pair->track02_md5 ||
        !layout_pair->status ||
        strcmp(layout_pair->status,
               "object_dungeon_layout_pair_bound_nonoverlap_render_blocked") != 0 ||
        !valid_later_read_layout_bytes_receipt(
            object_bytes, THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE) ||
        !valid_later_read_layout_bytes_receipt(
            dungeon_bytes, THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD) ||
        object_bytes->raw_track02_variant != layout_pair->raw_track02_variant ||
        dungeon_bytes->raw_track02_variant != layout_pair->raw_track02_variant ||
        strcmp(object_bytes->track02_md5, layout_pair->track02_md5) != 0 ||
        strcmp(dungeon_bytes->track02_md5, layout_pair->track02_md5) != 0 ||
        object_bytes->track02_record != layout_pair->object_track02_record ||
        object_bytes->layout_bytes != layout_pair->object_layout_bytes ||
        object_bytes->raw_track02_offset !=
            layout_pair->object_raw_track02_offset ||
        dungeon_bytes->track02_record != layout_pair->dungeon_track02_record ||
        dungeon_bytes->layout_bytes != layout_pair->dungeon_layout_bytes ||
        dungeon_bytes->raw_track02_offset !=
            layout_pair->dungeon_raw_track02_offset) {
        return 0;
    }

    object_begin = object_bytes->raw_track02_offset;
    object_end = object_begin + object_bytes->layout_bytes;
    dungeon_begin = dungeon_bytes->raw_track02_offset;
    dungeon_end = dungeon_begin + dungeon_bytes->layout_bytes;
    if (object_end < object_begin || dungeon_end < dungeon_begin ||
        !(object_end <= dungeon_begin || dungeon_end <= object_begin)) {
        return 0;
    }

    receipt.byte_pair_bound = 1;
    receipt.layout_pair_consumed = 1;
    receipt.object_bytes_consumed = 1;
    receipt.dungeon_bytes_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.non_overlapping_windows = 1;
    receipt.exact_source_bytes = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;
    receipt.raw_track02_variant = layout_pair->raw_track02_variant;
    receipt.object_track02_record = object_bytes->track02_record;
    receipt.object_layout_bytes = object_bytes->layout_bytes;
    receipt.object_layout_hash = object_bytes->layout_hash;
    receipt.object_raw_track02_offset = object_bytes->raw_track02_offset;
    receipt.dungeon_track02_record = dungeon_bytes->track02_record;
    receipt.dungeon_layout_bytes = dungeon_bytes->layout_bytes;
    receipt.dungeon_layout_hash = dungeon_bytes->layout_hash;
    receipt.dungeon_raw_track02_offset = dungeon_bytes->raw_track02_offset;
    receipt.track02_md5 = layout_pair->track02_md5;
    receipt.status =
        "object_dungeon_layout_bytes_pair_bound_opaque_render_blocked";
    *out_receipt = receipt;
    return 1;
}

static int valid_read_layout_pair_bridge_receipt(
    const Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt *bridge) {
    if (!bridge ||
        !bridge->read_layout_binding_consumed ||
        !bridge->parser_semantics_blocked ||
        !bridge->dungeon_grammar_blocked ||
        !bridge->runtime_handoff_blocked ||
        !bridge->rendering_blocked ||
        !bridge->fallback_visuals_blocked ||
        !bridge->no_synthetic_layout ||
        !bridge->status) {
        return 0;
    }

    if (bridge->raw_media_missing_blocked) {
        return !bridge->bridge_bound &&
            !bridge->layout_pair_consumed &&
            bridge->track02_md5 == NULL &&
            strcmp(bridge->status,
                   "object_dungeon_read_layout_pair_bridge_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!bridge->bridge_bound ||
        !bridge->layout_pair_consumed ||
        !bridge->same_track02_media ||
        !bridge->read_windows_preserved ||
        !bridge->non_overlapping_windows ||
        bridge->object_layout_bytes == 0u ||
        bridge->dungeon_layout_bytes == 0u ||
        !bridge->track02_md5 ||
        strcmp(bridge->status,
               "object_dungeon_read_layout_pair_bridge_bound_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (bridge->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(bridge->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (bridge->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(bridge->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_bridge_read_layout_pair_to_bytes(
    const Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt
        *read_layout_pair_bridge,
    const Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *byte_pair,
    Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_read_layout_pair_bridge_receipt(read_layout_pair_bridge)) {
        return 0;
    }

    receipt.read_layout_pair_bridge_consumed = 1;
    receipt.parser_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (read_layout_pair_bridge->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_read_to_bytes_bridge_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_object_dungeon_layout_bytes_pair_receipt(byte_pair) ||
        byte_pair->raw_track02_variant !=
            read_layout_pair_bridge->raw_track02_variant ||
        strcmp(byte_pair->track02_md5,
               read_layout_pair_bridge->track02_md5) != 0 ||
        byte_pair->object_track02_record !=
            read_layout_pair_bridge->object_track02_record ||
        byte_pair->object_layout_bytes !=
            read_layout_pair_bridge->object_layout_bytes ||
        byte_pair->object_raw_track02_offset !=
            read_layout_pair_bridge->object_raw_track02_offset ||
        byte_pair->dungeon_track02_record !=
            read_layout_pair_bridge->dungeon_track02_record ||
        byte_pair->dungeon_layout_bytes !=
            read_layout_pair_bridge->dungeon_layout_bytes ||
        byte_pair->dungeon_raw_track02_offset !=
            read_layout_pair_bridge->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.bridge_bound = 1;
    receipt.byte_pair_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.source_windows_preserved = 1;
    receipt.byte_hashes_recorded = 1;
    receipt.raw_track02_variant = read_layout_pair_bridge->raw_track02_variant;
    receipt.object_track02_record =
        read_layout_pair_bridge->object_track02_record;
    receipt.object_layout_bytes =
        read_layout_pair_bridge->object_layout_bytes;
    receipt.object_layout_hash = byte_pair->object_layout_hash;
    receipt.object_raw_track02_offset =
        read_layout_pair_bridge->object_raw_track02_offset;
    receipt.dungeon_track02_record =
        read_layout_pair_bridge->dungeon_track02_record;
    receipt.dungeon_layout_bytes =
        read_layout_pair_bridge->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = byte_pair->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        read_layout_pair_bridge->dungeon_raw_track02_offset;
    receipt.track02_md5 = read_layout_pair_bridge->track02_md5;
    receipt.status =
        "object_dungeon_read_to_bytes_bridge_bound_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_later_read_raw_media_gate_receipt(
    const Theron_V1Track02LaterReadRawMediaGateReceipt *gate) {
    if (!gate || !gate->gate_evaluated ||
        !gate->canonical_raw_bin_required ||
        !gate->iso_image_blocked ||
        !gate->parser_semantics_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->raw_media_bound &&
            !gate->raw_cue_admission_consumed &&
            !gate->canonical_raw_bin_present &&
            gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_NONE &&
            gate->raw_track02_bytes == 0u &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "later_loader_raw_track02_media_missing_handoff_blocked_no_fallback") == 0;
    }

    if (!gate->raw_media_bound ||
        !gate->raw_cue_admission_consumed ||
        !gate->canonical_raw_bin_present ||
        gate->raw_track02_bytes == 0u ||
        gate->raw_track02_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "later_loader_raw_track02_media_bound_handoff_still_blocked_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return gate->cue_track02_index01_raw_sector ==
                TQR_JP_CUE_INDEX01_RAW_SECTOR &&
            strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return gate->cue_track02_index01_raw_sector ==
                TQR_US_CUE_INDEX01_RAW_SECTOR &&
            strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

static int valid_object_dungeon_layout_bytes_pair_receipt(
    const Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *pair) {
    size_t object_begin, object_end, dungeon_begin, dungeon_end;

    if (!pair ||
        !pair->byte_pair_bound ||
        !pair->layout_pair_consumed ||
        !pair->object_bytes_consumed ||
        !pair->dungeon_bytes_consumed ||
        !pair->same_track02_media ||
        !pair->non_overlapping_windows ||
        !pair->exact_source_bytes ||
        !pair->parser_semantics_blocked ||
        !pair->runtime_handoff_blocked ||
        !pair->rendering_blocked ||
        !pair->fallback_visuals_blocked ||
        !pair->no_synthetic_bytes ||
        pair->object_layout_bytes == 0u ||
        pair->object_layout_hash == 0u ||
        pair->dungeon_layout_bytes == 0u ||
        pair->dungeon_layout_hash == 0u ||
        !pair->track02_md5 ||
        !pair->status ||
        strcmp(pair->status,
               "object_dungeon_layout_bytes_pair_bound_opaque_render_blocked") != 0) {
        return 0;
    }

    if (pair->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        if (strcmp(pair->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0) {
            return 0;
        }
    } else if (pair->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        if (strcmp(pair->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) {
            return 0;
        }
    } else {
        return 0;
    }

    object_begin = pair->object_raw_track02_offset;
    object_end = object_begin + pair->object_layout_bytes;
    dungeon_begin = pair->dungeon_raw_track02_offset;
    dungeon_end = dungeon_begin + pair->dungeon_layout_bytes;
    return object_end >= object_begin && dungeon_end >= dungeon_begin &&
        (object_end <= dungeon_begin || dungeon_end <= object_begin);
}

int theron_v1_track02_loader_intake_gate_object_dungeon_decoder_bytes(
    const Theron_V1Track02LaterReadRawMediaGateReceipt *raw_media_gate,
    const Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *byte_pair,
    Theron_V1Track02ObjectDungeonDecoderGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonDecoderGateReceipt receipt = {0};
    size_t object_end, dungeon_end;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_later_read_raw_media_gate_receipt(raw_media_gate)) {
        return 0;
    }

    receipt.gate_evaluated = 1;
    receipt.raw_media_gate_consumed = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (raw_media_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_decoder_bytes_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_object_dungeon_layout_bytes_pair_receipt(byte_pair) ||
        byte_pair->raw_track02_variant != raw_media_gate->raw_track02_variant ||
        strcmp(byte_pair->track02_md5, raw_media_gate->track02_md5) != 0) {
        return 0;
    }

    object_end = (size_t)byte_pair->object_raw_track02_offset +
        byte_pair->object_layout_bytes;
    dungeon_end = (size_t)byte_pair->dungeon_raw_track02_offset +
        byte_pair->dungeon_layout_bytes;
    if (object_end > raw_media_gate->raw_track02_bytes ||
        dungeon_end > raw_media_gate->raw_track02_bytes) {
        return 0;
    }

    receipt.byte_pair_consumed = 1;
    receipt.source_bytes_ready = 1;
    receipt.raw_track02_variant = byte_pair->raw_track02_variant;
    receipt.object_track02_record = byte_pair->object_track02_record;
    receipt.object_layout_bytes = byte_pair->object_layout_bytes;
    receipt.object_layout_hash = byte_pair->object_layout_hash;
    receipt.object_raw_track02_offset = byte_pair->object_raw_track02_offset;
    receipt.dungeon_track02_record = byte_pair->dungeon_track02_record;
    receipt.dungeon_layout_bytes = byte_pair->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = byte_pair->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset = byte_pair->dungeon_raw_track02_offset;
    receipt.track02_md5 = byte_pair->track02_md5;
    receipt.status =
        "object_dungeon_decoder_bytes_source_ready_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static uint32_t mix_u32(uint32_t hash, uint32_t value) {
    hash ^= value & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 8u) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 16u) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 24u) & 0xffu;
    hash *= 16777619u;
    return hash;
}

static int valid_object_dungeon_decoder_gate_receipt(
    const Theron_V1Track02ObjectDungeonDecoderGateReceipt *gate) {
    if (!gate || !gate->gate_evaluated ||
        !gate->raw_media_gate_consumed ||
        !gate->decoder_semantics_blocked ||
        !gate->dungeon_grammar_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->byte_pair_consumed &&
            !gate->source_bytes_ready &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "object_dungeon_decoder_bytes_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!gate->byte_pair_consumed ||
        !gate->source_bytes_ready ||
        gate->object_layout_bytes == 0u ||
        gate->object_layout_hash == 0u ||
        gate->dungeon_layout_bytes == 0u ||
        gate->dungeon_layout_hash == 0u ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "object_dungeon_decoder_bytes_source_ready_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

static int valid_read_to_bytes_bridge_receipt(
    const Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt *bridge) {
    if (!bridge ||
        !bridge->read_layout_pair_bridge_consumed ||
        !bridge->parser_semantics_blocked ||
        !bridge->dungeon_grammar_blocked ||
        !bridge->runtime_handoff_blocked ||
        !bridge->rendering_blocked ||
        !bridge->fallback_visuals_blocked ||
        !bridge->no_synthetic_bytes ||
        !bridge->status) {
        return 0;
    }

    if (bridge->raw_media_missing_blocked) {
        return !bridge->bridge_bound &&
            !bridge->byte_pair_consumed &&
            bridge->track02_md5 == NULL &&
            strcmp(bridge->status,
                   "object_dungeon_read_to_bytes_bridge_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!bridge->bridge_bound ||
        !bridge->byte_pair_consumed ||
        !bridge->same_track02_media ||
        !bridge->source_windows_preserved ||
        !bridge->byte_hashes_recorded ||
        bridge->object_layout_bytes == 0u ||
        bridge->object_layout_hash == 0u ||
        bridge->dungeon_layout_bytes == 0u ||
        bridge->dungeon_layout_hash == 0u ||
        !bridge->track02_md5 ||
        strcmp(bridge->status,
               "object_dungeon_read_to_bytes_bridge_bound_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (bridge->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(bridge->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (bridge->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(bridge->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_bind_read_to_bytes_to_decoder_gate(
    const Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt
        *read_to_bytes_bridge,
    const Theron_V1Track02ObjectDungeonDecoderGateReceipt *decoder_gate,
    Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_read_to_bytes_bridge_receipt(read_to_bytes_bridge)) {
        return 0;
    }

    receipt.read_to_bytes_bridge_consumed = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (read_to_bytes_bridge->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_read_to_decoder_gate_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_object_dungeon_decoder_gate_receipt(decoder_gate) ||
        decoder_gate->raw_media_missing_blocked ||
        decoder_gate->raw_track02_variant !=
            read_to_bytes_bridge->raw_track02_variant ||
        strcmp(decoder_gate->track02_md5,
               read_to_bytes_bridge->track02_md5) != 0 ||
        decoder_gate->object_track02_record !=
            read_to_bytes_bridge->object_track02_record ||
        decoder_gate->object_layout_bytes !=
            read_to_bytes_bridge->object_layout_bytes ||
        decoder_gate->object_layout_hash !=
            read_to_bytes_bridge->object_layout_hash ||
        decoder_gate->object_raw_track02_offset !=
            read_to_bytes_bridge->object_raw_track02_offset ||
        decoder_gate->dungeon_track02_record !=
            read_to_bytes_bridge->dungeon_track02_record ||
        decoder_gate->dungeon_layout_bytes !=
            read_to_bytes_bridge->dungeon_layout_bytes ||
        decoder_gate->dungeon_layout_hash !=
            read_to_bytes_bridge->dungeon_layout_hash ||
        decoder_gate->dungeon_raw_track02_offset !=
            read_to_bytes_bridge->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.gate_bound = 1;
    receipt.decoder_gate_consumed = 1;
    receipt.source_bytes_ready = 1;
    receipt.same_track02_media = 1;
    receipt.source_windows_preserved = 1;
    receipt.byte_hashes_preserved = 1;
    receipt.raw_track02_variant = read_to_bytes_bridge->raw_track02_variant;
    receipt.object_track02_record =
        read_to_bytes_bridge->object_track02_record;
    receipt.object_layout_bytes = read_to_bytes_bridge->object_layout_bytes;
    receipt.object_layout_hash = read_to_bytes_bridge->object_layout_hash;
    receipt.object_raw_track02_offset =
        read_to_bytes_bridge->object_raw_track02_offset;
    receipt.dungeon_track02_record =
        read_to_bytes_bridge->dungeon_track02_record;
    receipt.dungeon_layout_bytes = read_to_bytes_bridge->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = read_to_bytes_bridge->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        read_to_bytes_bridge->dungeon_raw_track02_offset;
    receipt.track02_md5 = read_to_bytes_bridge->track02_md5;
    receipt.status =
        "object_dungeon_read_to_decoder_gate_bound_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_record_object_dungeon_predecode_evidence(
    const Theron_V1Track02ObjectDungeonDecoderGateReceipt *decoder_gate,
    Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt receipt = {0};
    size_t object_begin, object_end, dungeon_begin, dungeon_end;
    size_t span_begin, span_end;
    uint32_t evidence_hash = 2166136261u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_object_dungeon_decoder_gate_receipt(decoder_gate)) {
        return 0;
    }

    receipt.evidence_recorded = 1;
    receipt.decoder_gate_consumed = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (decoder_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_predecode_evidence_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    object_begin = decoder_gate->object_raw_track02_offset;
    object_end = object_begin + decoder_gate->object_layout_bytes;
    dungeon_begin = decoder_gate->dungeon_raw_track02_offset;
    dungeon_end = dungeon_begin + decoder_gate->dungeon_layout_bytes;
    if (object_end < object_begin || dungeon_end < dungeon_begin ||
        !(object_end <= dungeon_begin || dungeon_end <= object_begin)) {
        return 0;
    }

    receipt.source_bytes_ready = 1;
    receipt.same_track02_media = 1;
    receipt.non_overlapping_windows = 1;
    receipt.object_window_before_dungeon = object_end <= dungeon_begin;
    receipt.dungeon_window_before_object = dungeon_end <= object_begin;
    receipt.raw_track02_variant = decoder_gate->raw_track02_variant;
    receipt.object_track02_record = decoder_gate->object_track02_record;
    receipt.object_layout_bytes = decoder_gate->object_layout_bytes;
    receipt.object_layout_hash = decoder_gate->object_layout_hash;
    receipt.object_raw_track02_offset = decoder_gate->object_raw_track02_offset;
    receipt.dungeon_track02_record = decoder_gate->dungeon_track02_record;
    receipt.dungeon_layout_bytes = decoder_gate->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = decoder_gate->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset = decoder_gate->dungeon_raw_track02_offset;
    if (receipt.object_window_before_dungeon) {
        receipt.gap_bytes = (uint32_t)(dungeon_begin - object_end);
        span_begin = object_begin;
        span_end = dungeon_end;
    } else {
        receipt.gap_bytes = (uint32_t)(object_begin - dungeon_end);
        span_begin = dungeon_begin;
        span_end = object_end;
    }
    receipt.total_span_bytes = (uint32_t)(span_end - span_begin);
    evidence_hash = mix_u32(evidence_hash, receipt.object_track02_record);
    evidence_hash = mix_u32(evidence_hash, receipt.object_raw_track02_offset);
    evidence_hash = mix_u32(evidence_hash, receipt.object_layout_bytes);
    evidence_hash = mix_u32(evidence_hash, receipt.object_layout_hash);
    evidence_hash = mix_u32(evidence_hash, receipt.dungeon_track02_record);
    evidence_hash = mix_u32(evidence_hash, receipt.dungeon_raw_track02_offset);
    evidence_hash = mix_u32(evidence_hash, receipt.dungeon_layout_bytes);
    evidence_hash = mix_u32(evidence_hash, receipt.dungeon_layout_hash);
    evidence_hash = mix_u32(evidence_hash, receipt.gap_bytes);
    evidence_hash = mix_u32(evidence_hash, receipt.total_span_bytes);
    receipt.predecode_evidence_hash = evidence_hash;
    receipt.track02_md5 = decoder_gate->track02_md5;
    receipt.status =
        "object_dungeon_predecode_evidence_recorded_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_read_to_decoder_gate_receipt(
    const Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt *gate) {
    if (!gate ||
        !gate->read_to_bytes_bridge_consumed ||
        !gate->decoder_semantics_blocked ||
        !gate->dungeon_grammar_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->gate_bound &&
            !gate->decoder_gate_consumed &&
            !gate->source_bytes_ready &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "object_dungeon_read_to_decoder_gate_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!gate->gate_bound ||
        !gate->decoder_gate_consumed ||
        !gate->source_bytes_ready ||
        !gate->same_track02_media ||
        !gate->source_windows_preserved ||
        !gate->byte_hashes_preserved ||
        gate->object_layout_bytes == 0u ||
        gate->object_layout_hash == 0u ||
        gate->dungeon_layout_bytes == 0u ||
        gate->dungeon_layout_hash == 0u ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "object_dungeon_read_to_decoder_gate_bound_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

static int valid_predecode_evidence_receipt(
    const Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt *evidence) {
    if (!evidence ||
        !evidence->evidence_recorded ||
        !evidence->decoder_gate_consumed ||
        !evidence->decoder_semantics_blocked ||
        !evidence->dungeon_grammar_blocked ||
        !evidence->runtime_handoff_blocked ||
        !evidence->rendering_blocked ||
        !evidence->fallback_visuals_blocked ||
        !evidence->no_synthetic_bytes ||
        !evidence->status) {
        return 0;
    }

    if (evidence->raw_media_missing_blocked) {
        return !evidence->source_bytes_ready &&
            evidence->track02_md5 == NULL &&
            strcmp(evidence->status,
                   "object_dungeon_predecode_evidence_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!evidence->source_bytes_ready ||
        !evidence->same_track02_media ||
        !evidence->non_overlapping_windows ||
        (evidence->object_window_before_dungeon ==
         evidence->dungeon_window_before_object) ||
        evidence->object_layout_bytes == 0u ||
        evidence->object_layout_hash == 0u ||
        evidence->dungeon_layout_bytes == 0u ||
        evidence->dungeon_layout_hash == 0u ||
        evidence->total_span_bytes !=
            evidence->object_layout_bytes + evidence->dungeon_layout_bytes +
                evidence->gap_bytes ||
        evidence->predecode_evidence_hash == 0u ||
        !evidence->track02_md5 ||
        strcmp(evidence->status,
               "object_dungeon_predecode_evidence_recorded_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (evidence->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(evidence->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (evidence->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(evidence->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_gate_object_dungeon_post_predecode(
    const Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt
        *read_to_decoder_gate,
    const Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt
        *predecode_evidence,
    Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_read_to_decoder_gate_receipt(read_to_decoder_gate)) {
        return 0;
    }

    receipt.read_to_decoder_gate_consumed = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (read_to_decoder_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_post_predecode_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_predecode_evidence_receipt(predecode_evidence) ||
        predecode_evidence->raw_track02_variant !=
            read_to_decoder_gate->raw_track02_variant ||
        strcmp(predecode_evidence->track02_md5,
               read_to_decoder_gate->track02_md5) != 0 ||
        predecode_evidence->object_track02_record !=
            read_to_decoder_gate->object_track02_record ||
        predecode_evidence->object_layout_bytes !=
            read_to_decoder_gate->object_layout_bytes ||
        predecode_evidence->object_layout_hash !=
            read_to_decoder_gate->object_layout_hash ||
        predecode_evidence->object_raw_track02_offset !=
            read_to_decoder_gate->object_raw_track02_offset ||
        predecode_evidence->dungeon_track02_record !=
            read_to_decoder_gate->dungeon_track02_record ||
        predecode_evidence->dungeon_layout_bytes !=
            read_to_decoder_gate->dungeon_layout_bytes ||
        predecode_evidence->dungeon_layout_hash !=
            read_to_decoder_gate->dungeon_layout_hash ||
        predecode_evidence->dungeon_raw_track02_offset !=
            read_to_decoder_gate->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.readiness_bound = 1;
    receipt.predecode_evidence_consumed = 1;
    receipt.topology_ready = 1;
    receipt.same_track02_media = 1;
    receipt.source_windows_preserved = 1;
    receipt.byte_hashes_preserved = 1;
    receipt.topology_hash_preserved = 1;
    receipt.raw_track02_variant = read_to_decoder_gate->raw_track02_variant;
    receipt.object_track02_record =
        read_to_decoder_gate->object_track02_record;
    receipt.object_layout_bytes = read_to_decoder_gate->object_layout_bytes;
    receipt.object_layout_hash = read_to_decoder_gate->object_layout_hash;
    receipt.object_raw_track02_offset =
        read_to_decoder_gate->object_raw_track02_offset;
    receipt.dungeon_track02_record =
        read_to_decoder_gate->dungeon_track02_record;
    receipt.dungeon_layout_bytes = read_to_decoder_gate->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = read_to_decoder_gate->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        read_to_decoder_gate->dungeon_raw_track02_offset;
    receipt.gap_bytes = predecode_evidence->gap_bytes;
    receipt.total_span_bytes = predecode_evidence->total_span_bytes;
    receipt.predecode_evidence_hash =
        predecode_evidence->predecode_evidence_hash;
    receipt.track02_md5 = read_to_decoder_gate->track02_md5;
    receipt.status =
        "object_dungeon_post_predecode_topology_ready_semantics_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_post_predecode_gate_receipt(
    const Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt *gate) {
    if (!gate ||
        !gate->read_to_decoder_gate_consumed ||
        !gate->decoder_semantics_blocked ||
        !gate->dungeon_grammar_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->readiness_bound &&
            !gate->predecode_evidence_consumed &&
            !gate->topology_ready &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "object_dungeon_post_predecode_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!gate->readiness_bound ||
        !gate->predecode_evidence_consumed ||
        !gate->topology_ready ||
        !gate->same_track02_media ||
        !gate->source_windows_preserved ||
        !gate->byte_hashes_preserved ||
        !gate->topology_hash_preserved ||
        gate->object_layout_bytes == 0u ||
        gate->object_layout_hash == 0u ||
        gate->dungeon_layout_bytes == 0u ||
        gate->dungeon_layout_hash == 0u ||
        gate->predecode_evidence_hash == 0u ||
        gate->total_span_bytes !=
            gate->object_layout_bytes + gate->dungeon_layout_bytes +
                gate->gap_bytes ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "object_dungeon_post_predecode_topology_ready_semantics_blocked_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

static int valid_initial_level_handoff_for_variant(
    const Theron_V1DungeonHandoffReceipt *initial_level,
    Theron_V1Track02Variant variant,
    const char *track02_md5) {
    uint32_t expected_cue, expected_sector;

    if (!initial_level || !track02_md5 ||
        !initial_level->selected ||
        !initial_level->runtime_route_consumed ||
        initial_level->record != THERON_V1_INITIAL_ENVELOPE_RECORD ||
        initial_level->record_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        initial_level->envelope_bytes != THERON_V1_INITIAL_ENVELOPE_BYTES ||
        initial_level->header_width !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        initial_level->header_height !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        initial_level->header_seed !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_SEED ||
        initial_level->header_identifier !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_IDENTIFIER ||
        initial_level->raw_sector_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET +
                THERON_V1_TRACK02_MODE1_HEADER_BYTES ||
        !initial_level->raw_track02_md5_verified ||
        !initial_level->adjacent_boundary_opaque ||
        !initial_level->route ||
        strcmp(initial_level->route, "raw_track02_initial_envelope") != 0 ||
        initial_level->raw_track02_variant != variant) {
        return 0;
    }

    if (variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        if (strcmp(track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) != 0) {
            return 0;
        }
        expected_cue = TQR_JP_CUE_INDEX01_RAW_SECTOR;
        expected_sector = TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR;
    } else if (variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        if (strcmp(track02_md5, THERON_V1_TRACK02_MD5_US_BIN) != 0) {
            return 0;
        }
        expected_cue = TQR_US_CUE_INDEX01_RAW_SECTOR;
        expected_sector = TQR_US_INITIAL_ENVELOPE_RAW_SECTOR;
    } else {
        return 0;
    }

    return initial_level->cue_track02_index01_raw_sector == expected_cue &&
        initial_level->track02_raw_sector == expected_sector;
}

int theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
    const Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt
        *post_predecode_gate,
    const Theron_V1DungeonHandoffReceipt *initial_level,
    Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_post_predecode_gate_receipt(post_predecode_gate)) {
        return 0;
    }

    receipt.post_predecode_gate_consumed = 1;
    receipt.object_records_blocked = 1;
    receipt.dungeon_records_blocked = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.dungeon_grammar_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (post_predecode_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_level_handoff_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_initial_level_handoff_for_variant(
            initial_level, post_predecode_gate->raw_track02_variant,
            post_predecode_gate->track02_md5)) {
        return 0;
    }

    receipt.level_handoff_bound = 1;
    receipt.initial_level_handoff_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.initial_level_source_locked = 1;
    receipt.initial_level_boundary_opaque = 1;
    receipt.topology_evidence_preserved = 1;
    receipt.raw_track02_variant = post_predecode_gate->raw_track02_variant;
    receipt.initial_level_track02_record = initial_level->record;
    receipt.initial_level_user_data_offset =
        initial_level->record_user_data_offset;
    receipt.initial_level_raw_track02_sector =
        initial_level->track02_raw_sector;
    receipt.initial_level_raw_sector_offset =
        initial_level->raw_sector_offset;
    receipt.initial_level_width = initial_level->header_width;
    receipt.initial_level_height = initial_level->header_height;
    receipt.object_track02_record =
        post_predecode_gate->object_track02_record;
    receipt.object_layout_bytes = post_predecode_gate->object_layout_bytes;
    receipt.object_layout_hash = post_predecode_gate->object_layout_hash;
    receipt.object_raw_track02_offset =
        post_predecode_gate->object_raw_track02_offset;
    receipt.dungeon_track02_record =
        post_predecode_gate->dungeon_track02_record;
    receipt.dungeon_layout_bytes = post_predecode_gate->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = post_predecode_gate->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        post_predecode_gate->dungeon_raw_track02_offset;
    receipt.gap_bytes = post_predecode_gate->gap_bytes;
    receipt.total_span_bytes = post_predecode_gate->total_span_bytes;
    receipt.predecode_evidence_hash =
        post_predecode_gate->predecode_evidence_hash;
    receipt.track02_md5 = post_predecode_gate->track02_md5;
    receipt.status =
        "object_dungeon_level_handoff_bound_topology_preserved_runtime_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_level_handoff_gate_receipt(
    const Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt *gate) {
    if (!gate ||
        !gate->post_predecode_gate_consumed ||
        !gate->object_records_blocked ||
        !gate->dungeon_records_blocked ||
        !gate->decoder_semantics_blocked ||
        !gate->dungeon_grammar_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->level_handoff_bound &&
            !gate->initial_level_handoff_consumed &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "object_dungeon_level_handoff_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!gate->level_handoff_bound ||
        !gate->initial_level_handoff_consumed ||
        !gate->same_track02_media ||
        !gate->initial_level_source_locked ||
        !gate->initial_level_boundary_opaque ||
        !gate->topology_evidence_preserved ||
        gate->initial_level_track02_record !=
            THERON_V1_INITIAL_ENVELOPE_RECORD ||
        gate->initial_level_user_data_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET ||
        gate->initial_level_raw_sector_offset !=
            THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET +
                THERON_V1_TRACK02_MODE1_HEADER_BYTES ||
        gate->initial_level_width !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH ||
        gate->initial_level_height !=
            THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT ||
        gate->object_layout_bytes == 0u ||
        gate->object_layout_hash == 0u ||
        gate->dungeon_layout_bytes == 0u ||
        gate->dungeon_layout_hash == 0u ||
        gate->predecode_evidence_hash == 0u ||
        gate->total_span_bytes !=
            gate->object_layout_bytes + gate->dungeon_layout_bytes +
                gate->gap_bytes ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "object_dungeon_level_handoff_bound_topology_preserved_runtime_blocked_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return gate->initial_level_raw_track02_sector ==
                TQR_JP_INITIAL_ENVELOPE_RAW_SECTOR &&
            strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return gate->initial_level_raw_track02_sector ==
                TQR_US_INITIAL_ENVELOPE_RAW_SECTOR &&
            strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
    const Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt
        *level_handoff_gate,
    Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_level_handoff_gate_receipt(level_handoff_gate)) {
        return 0;
    }

    receipt.grammar_gate_evaluated = 1;
    receipt.level_handoff_gate_consumed = 1;
    receipt.object_table_grammar_required = 1;
    receipt.dungeon_record_grammar_required = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (level_handoff_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_grammar_admission_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    receipt.source_topology_ready = 1;
    receipt.same_track02_media = 1;
    receipt.original_cd_read_evidence_preserved = 1;
    receipt.topology_evidence_preserved = 1;
    receipt.raw_track02_variant = level_handoff_gate->raw_track02_variant;
    receipt.object_track02_record =
        level_handoff_gate->object_track02_record;
    receipt.object_layout_bytes = level_handoff_gate->object_layout_bytes;
    receipt.object_layout_hash = level_handoff_gate->object_layout_hash;
    receipt.object_raw_track02_offset =
        level_handoff_gate->object_raw_track02_offset;
    receipt.dungeon_track02_record =
        level_handoff_gate->dungeon_track02_record;
    receipt.dungeon_layout_bytes = level_handoff_gate->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = level_handoff_gate->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        level_handoff_gate->dungeon_raw_track02_offset;
    receipt.gap_bytes = level_handoff_gate->gap_bytes;
    receipt.total_span_bytes = level_handoff_gate->total_span_bytes;
    receipt.predecode_evidence_hash =
        level_handoff_gate->predecode_evidence_hash;
    receipt.track02_md5 = level_handoff_gate->track02_md5;
    receipt.status =
        "object_dungeon_grammar_admission_blocked_original_grammar_witness_required_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_grammar_admission_gate_receipt(
    const Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt *gate) {
    if (!gate ||
        !gate->grammar_gate_evaluated ||
        !gate->level_handoff_gate_consumed ||
        !gate->object_table_grammar_required ||
        !gate->dungeon_record_grammar_required ||
        gate->object_table_grammar_admitted ||
        gate->dungeon_record_grammar_admitted ||
        !gate->decoder_semantics_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->source_topology_ready &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "object_dungeon_grammar_admission_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!gate->source_topology_ready ||
        !gate->same_track02_media ||
        !gate->original_cd_read_evidence_preserved ||
        !gate->topology_evidence_preserved ||
        gate->object_layout_bytes == 0u ||
        gate->object_layout_hash == 0u ||
        gate->dungeon_layout_bytes == 0u ||
        gate->dungeon_layout_hash == 0u ||
        gate->predecode_evidence_hash == 0u ||
        gate->total_span_bytes !=
            gate->object_layout_bytes + gate->dungeon_layout_bytes +
                gate->gap_bytes ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "object_dungeon_grammar_admission_blocked_original_grammar_witness_required_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
    const Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt
        *grammar_gate,
    const Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt
        *read_layout_binding,
    Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_grammar_admission_gate_receipt(grammar_gate)) {
        return 0;
    }

    receipt.grammar_gate_consumed = 1;
    receipt.object_table_grammar_required = 1;
    receipt.dungeon_record_grammar_required = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (grammar_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_grammar_read_evidence_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!valid_read_table_layout_binding_receipt(read_layout_binding) ||
        read_layout_binding->raw_media_missing_blocked ||
        read_layout_binding->raw_track02_variant !=
            grammar_gate->raw_track02_variant ||
        strcmp(read_layout_binding->track02_md5,
               grammar_gate->track02_md5) != 0 ||
        read_layout_binding->object_track02_record !=
            grammar_gate->object_track02_record ||
        read_layout_binding->object_layout_bytes !=
            grammar_gate->object_layout_bytes ||
        read_layout_binding->object_raw_track02_offset !=
            grammar_gate->object_raw_track02_offset ||
        read_layout_binding->dungeon_track02_record !=
            grammar_gate->dungeon_track02_record ||
        read_layout_binding->dungeon_layout_bytes !=
            grammar_gate->dungeon_layout_bytes ||
        read_layout_binding->dungeon_raw_track02_offset !=
            grammar_gate->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.read_evidence_bound = 1;
    receipt.read_layout_binding_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.original_cd_read_destinations_preserved = 1;
    receipt.layout_windows_preserved = 1;
    receipt.topology_evidence_preserved = 1;
    receipt.raw_track02_variant = grammar_gate->raw_track02_variant;
    receipt.object_track02_record = grammar_gate->object_track02_record;
    receipt.object_record_user_data_offset =
        read_layout_binding->object_record_user_data_offset;
    receipt.object_destination = read_layout_binding->object_destination;
    receipt.object_layout_bytes = grammar_gate->object_layout_bytes;
    receipt.object_layout_hash = grammar_gate->object_layout_hash;
    receipt.object_raw_track02_offset =
        grammar_gate->object_raw_track02_offset;
    receipt.dungeon_track02_record = grammar_gate->dungeon_track02_record;
    receipt.dungeon_record_user_data_offset =
        read_layout_binding->dungeon_record_user_data_offset;
    receipt.dungeon_destination = read_layout_binding->dungeon_destination;
    receipt.dungeon_layout_bytes = grammar_gate->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = grammar_gate->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        grammar_gate->dungeon_raw_track02_offset;
    receipt.predecode_evidence_hash =
        grammar_gate->predecode_evidence_hash;
    receipt.track02_md5 = grammar_gate->track02_md5;
    receipt.status =
        "object_dungeon_grammar_read_evidence_bound_original_loader_reads_grammar_blocked_no_fallback";
    *out_receipt = receipt;
    return 1;
}

static int valid_grammar_read_evidence_gate_receipt(
    const Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt *gate) {
    if (!gate ||
        !gate->grammar_gate_consumed ||
        !gate->object_table_grammar_required ||
        !gate->dungeon_record_grammar_required ||
        gate->object_table_grammar_admitted ||
        gate->dungeon_record_grammar_admitted ||
        !gate->decoder_semantics_blocked ||
        !gate->runtime_handoff_blocked ||
        !gate->rendering_blocked ||
        !gate->fallback_visuals_blocked ||
        !gate->no_synthetic_bytes ||
        !gate->status) {
        return 0;
    }

    if (gate->raw_media_missing_blocked) {
        return !gate->read_evidence_bound &&
            !gate->read_layout_binding_consumed &&
            gate->track02_md5 == NULL &&
            strcmp(gate->status,
                   "object_dungeon_grammar_read_evidence_blocked_missing_raw_media_no_fallback") == 0;
    }

    if (!gate->read_evidence_bound ||
        !gate->read_layout_binding_consumed ||
        !gate->same_track02_media ||
        !gate->original_cd_read_destinations_preserved ||
        !gate->layout_windows_preserved ||
        !gate->topology_evidence_preserved ||
        gate->object_layout_bytes == 0u ||
        gate->object_layout_hash == 0u ||
        gate->dungeon_layout_bytes == 0u ||
        gate->dungeon_layout_hash == 0u ||
        gate->predecode_evidence_hash == 0u ||
        !gate->track02_md5 ||
        strcmp(gate->status,
               "object_dungeon_grammar_read_evidence_bound_original_loader_reads_grammar_blocked_no_fallback") != 0) {
        return 0;
    }

    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_JP_BIN) == 0;
    }
    if (gate->raw_track02_variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        return strcmp(gate->track02_md5, THERON_V1_TRACK02_MD5_US_BIN) == 0;
    }
    return 0;
}

int theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
    const Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt
        *grammar_read_gate,
    const Theron_V1Track02ObjectDungeonParserGrammarWitnessFacts *witness,
    Theron_V1Track02ObjectDungeonParserGrammarWitnessReceipt *out_receipt) {
    Theron_V1Track02ObjectDungeonParserGrammarWitnessReceipt receipt = {0};

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt ||
        !valid_grammar_read_evidence_gate_receipt(grammar_read_gate)) {
        return 0;
    }

    receipt.grammar_read_evidence_consumed = 1;
    receipt.object_table_fields_blocked = 1;
    receipt.dungeon_record_fields_blocked = 1;
    receipt.decoder_semantics_blocked = 1;
    receipt.runtime_handoff_blocked = 1;
    receipt.rendering_blocked = 1;
    receipt.fallback_visuals_blocked = 1;
    receipt.no_synthetic_bytes = 1;

    if (grammar_read_gate->raw_media_missing_blocked) {
        receipt.raw_media_missing_blocked = 1;
        receipt.status =
            "object_dungeon_parser_witness_blocked_missing_raw_media_no_fallback";
        *out_receipt = receipt;
        return 1;
    }

    if (!witness ||
        !witness->original_loader_trace ||
        !witness->original_parser_trace ||
        !witness->object_table_parser_entered ||
        !witness->dungeon_record_parser_entered ||
        !witness->no_fallback_visuals ||
        !witness->no_synthetic_bytes ||
        witness->raw_track02_variant != grammar_read_gate->raw_track02_variant ||
        !witness->track02_md5 ||
        strcmp(witness->track02_md5, grammar_read_gate->track02_md5) != 0 ||
        witness->object_track02_record !=
            grammar_read_gate->object_track02_record ||
        witness->object_record_user_data_offset !=
            grammar_read_gate->object_record_user_data_offset ||
        witness->object_destination != grammar_read_gate->object_destination ||
        witness->object_byte_count !=
            grammar_read_gate->object_layout_bytes ||
        witness->object_raw_track02_offset !=
            grammar_read_gate->object_raw_track02_offset ||
        witness->dungeon_track02_record !=
            grammar_read_gate->dungeon_track02_record ||
        witness->dungeon_record_user_data_offset !=
            grammar_read_gate->dungeon_record_user_data_offset ||
        witness->dungeon_destination !=
            grammar_read_gate->dungeon_destination ||
        witness->dungeon_byte_count !=
            grammar_read_gate->dungeon_layout_bytes ||
        witness->dungeon_raw_track02_offset !=
            grammar_read_gate->dungeon_raw_track02_offset) {
        return 0;
    }

    receipt.parser_witness_bound = 1;
    receipt.original_loader_trace_consumed = 1;
    receipt.original_parser_trace_consumed = 1;
    receipt.same_track02_media = 1;
    receipt.object_table_parser_witnessed = 1;
    receipt.dungeon_record_parser_witnessed = 1;
    receipt.object_table_grammar_admitted = 1;
    receipt.dungeon_record_grammar_admitted = 1;
    receipt.raw_track02_variant = grammar_read_gate->raw_track02_variant;
    receipt.object_track02_record = grammar_read_gate->object_track02_record;
    receipt.object_record_user_data_offset =
        grammar_read_gate->object_record_user_data_offset;
    receipt.object_destination = grammar_read_gate->object_destination;
    receipt.object_layout_bytes = grammar_read_gate->object_layout_bytes;
    receipt.object_layout_hash = grammar_read_gate->object_layout_hash;
    receipt.object_raw_track02_offset =
        grammar_read_gate->object_raw_track02_offset;
    receipt.dungeon_track02_record = grammar_read_gate->dungeon_track02_record;
    receipt.dungeon_record_user_data_offset =
        grammar_read_gate->dungeon_record_user_data_offset;
    receipt.dungeon_destination = grammar_read_gate->dungeon_destination;
    receipt.dungeon_layout_bytes = grammar_read_gate->dungeon_layout_bytes;
    receipt.dungeon_layout_hash = grammar_read_gate->dungeon_layout_hash;
    receipt.dungeon_raw_track02_offset =
        grammar_read_gate->dungeon_raw_track02_offset;
    receipt.predecode_evidence_hash =
        grammar_read_gate->predecode_evidence_hash;
    receipt.track02_md5 = grammar_read_gate->track02_md5;
    receipt.status =
        "object_dungeon_parser_witness_bound_grammar_proven_fields_runtime_render_blocked";
    *out_receipt = receipt;
    return 1;
}

int theron_v1_track02_loader_intake_observe_authenticated_trace(
    const Theron_V1AuthenticatedTrack02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt) {
    Theron_V1Track02LoaderReadFacts observation;
    int observed;

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
    observed = theron_v1_track02_loader_intake_observe(&observation,
                                                       out_receipt);
    if (observed) {
        out_receipt->authenticated_v3_trace = 1;
    }
    return observed;
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
