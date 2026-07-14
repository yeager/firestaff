#ifndef THERON_V1_TRACK02_LOADER_INTAKE_H
#define THERON_V1_TRACK02_LOADER_INTAKE_H

#include <stdint.h>

#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_trace_provenance.h"

/* This is an observation boundary, not a payload decoder. The destination and
 * byte count are retained only when an original later loader read reports
 * them; neither value is interpreted as a memory or record layout. */
typedef struct {
    int authenticated_original_trace;
    int later_than_stage2_transfer;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t byte_count;
} Theron_V1Track02LoaderReadFacts;

typedef struct {
    int observed;
    int payload_intake_admitted;
    int initial_envelope_source_bound;
    int initial_envelope_decoded;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t observed_destination;
    uint32_t observed_byte_count;
    uint16_t decoded_header_width;
    uint16_t decoded_header_height;
    uint32_t decoded_header_seed;
    uint16_t decoded_header_identifier;
    uint16_t decoded_header_extension;
    uint32_t decoded_grid_bytes;
    uint32_t decoded_grid_hash;
    uint16_t decoded_grid_row_count;
    uint16_t decoded_grid_row_bytes;
    uint32_t decoded_grid_raw_sector;
    uint32_t decoded_grid_raw_sector_offset;
    uint32_t decoded_grid_first_row_hash;
    uint32_t decoded_grid_last_row_hash;
    const char *status;
} Theron_V1Track02LoaderIntakeReceipt;

/* A coordinate handoff is restricted to the verified raw grid byte and its
 * physical Track 02 placement. It intentionally does not name or interpret
 * the byte as a cell, object, tile, visual, or runtime value. */
typedef struct {
    int handed_off;
    uint16_t raw_grid_x;
    uint16_t raw_grid_y;
    uint8_t raw_grid_byte;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridCoordinateReceipt;

/* This binds an observed read to the existing accepted-trace provenance
 * boundary.  The transfer facts remain opaque observations. */
typedef struct {
    const Theron_V1TraceProvenanceReceipt *trace_provenance;
    int later_than_stage2_transfer;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t byte_count;
} Theron_V1AuthenticatedTrack02LoaderReadFacts;

/* Accepts only a provenance-authenticated later read of the source-locked
 * initial envelope. It deliberately leaves payload intake blocked until
 * independent evidence establishes what the observed transfer means. */
int theron_v1_track02_loader_intake_observe(
    const Theron_V1Track02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Admits an observation only when it came through the existing authenticated
 * trace path. It does not promote the receipt to payload intake. */
int theron_v1_track02_loader_intake_observe_authenticated_trace(
    const Theron_V1AuthenticatedTrack02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Joins an authenticated later-read observation to the independently
 * source-verified initial-envelope receipt. The observed read must cover the
 * real envelope, but this remains a boundary binding rather than a decode. */
int theron_v1_track02_loader_intake_bind_initial_envelope(
    const Theron_V1Track02LoaderIntakeReceipt *observation,
    const Theron_V1DungeonHandoffReceipt *initial_envelope,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Decodes the source-bound initial envelope only from the complete, canonical
 * raw Track 02 image. The raw bytes are independently rehashed and must still
 * agree with the selected receipt and runtime-admitted loader observation.
 * It promotes the literal header, grid span, and its bounded raw-sector row
 * partition, but assigns no cell, object, visual, or post-grid-tail
 * semantics. */
int theron_v1_track02_loader_intake_decode_initial_envelope(
    const Theron_V1Track02LoaderIntakeReceipt *source_bound_receipt,
    const Theron_V1DungeonHandoffReceipt *initial_envelope,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Hands off one coordinate only after independently rehashing the canonical
 * raw BIN and rechecking the decoded envelope header and complete grid hash.
 * Invalid provenance, altered bytes, or coordinates outside the verified grid
 * produce no receipt. */
int theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint16_t raw_grid_x,
    uint16_t raw_grid_y,
    Theron_V1Track02RawGridCoordinateReceipt *out_receipt);

#endif
