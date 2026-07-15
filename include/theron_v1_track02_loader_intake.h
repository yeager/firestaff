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
    int authenticated_v3_trace;
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
    int authenticated_v3_trace;
    uint16_t raw_grid_x;
    uint16_t raw_grid_y;
    uint8_t raw_grid_byte;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridCoordinateReceipt;

/* A row handoff retains one complete verified source row as opaque bytes.
 * Neither the row nor its contents acquire dungeon, cell, object, tile, or
 * visual semantics at this boundary. */
typedef struct {
    int handed_off;
    int authenticated_v3_trace;
    uint16_t raw_grid_y;
    uint16_t raw_grid_bytes;
    uint8_t raw_grid_row[THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH];
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    uint32_t raw_grid_row_hash;
    const char *status;
} Theron_V1Track02RawGridRowReceipt;

/* A complete source-verified initial grid for the downstream dungeon handoff.
 * The bytes are atomic and opaque: this boundary assigns no dungeon, cell,
 * object, tile, visual, or runtime semantics to them. */
typedef struct {
    int handed_off;
    int authenticated_v3_trace;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint8_t raw_grid[THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH *
                     THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT];
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    uint32_t raw_grid_hash;
    const char *status;
} Theron_V1Track02RawGridReceipt;

/* A runtime consumer receives the exact source-verified startup grid.  The
 * bytes are read-only and remain deliberately unclassified: the loader trace
 * proves their transfer, not cell, object, tile, palette, or visual meaning.
 * A consumer must explicitly accept the receipt; rejection leaves the caller
 * without a route rather than enabling any generated substitute. */
typedef int (*Theron_V1Track02RawGridConsumer)(
    const Theron_V1Track02RawGridReceipt *grid,
    void *context);

typedef struct {
    int delivered;
    int authenticated_v3_trace;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridRuntimeReceipt;

/* The initial-grid handoff is not an object-table handoff. This receipt lets
 * callers record that they reached verified raw bytes while still refusing to
 * project those bytes into objects, triggers, monsters, or fallback visuals. */
typedef struct {
    int projection_blocked;
    int authenticated_v3_trace;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridObjectTableProjectionReceipt;

/* A narrow level route for the verified startup grid. This is the first
 * dungeon-facing positive handoff from Track 02, but it still admits only the
 * source-owned raw grid. Bitmap and object routes stay blocked until their
 * own original-data receipts exist. */
typedef struct {
    int level_route_admitted;
    int authenticated_v3_trace;
    int bitmap_route_blocked;
    int object_route_blocked;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridLevelRouteReceipt;

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

/* Hands off one complete source-verified grid row after independently
 * rehashing the canonical raw BIN and rechecking its literal envelope and
 * complete-grid receipt. The returned bytes remain opaque. */
int theron_v1_track02_loader_intake_handoff_raw_grid_row(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint16_t raw_grid_y,
    Theron_V1Track02RawGridRowReceipt *out_receipt);

/* Hands off the complete literal grid only after independently rehashing the
 * canonical raw BIN and requiring the exact source-locked 32x27 receipt.
 * This is the atomic raw-data boundary for a later dungeon consumer; no
 * fallback visual or inferred semantic route is enabled here. */
int theron_v1_track02_loader_intake_handoff_raw_grid(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02RawGridReceipt *out_receipt);

/* Delivers the complete, rehashed initial grid to a caller-owned runtime
 * consumer. This is the final original-media boundary before a future
 * evidence-backed dungeon decoder: it never creates a level, object list, or
 * visual fallback, and a rejecting consumer leaves no successful receipt. */
int theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02RawGridConsumer consumer,
    void *consumer_context,
    Theron_V1Track02RawGridRuntimeReceipt *out_receipt);

/* Explicitly blocks object-table projection from the verified startup grid.
 * A successful receipt is a negative handoff: it proves no object route or
 * substitute visual was admitted from these bytes. */
int theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridObjectTableProjectionReceipt *out_receipt);

/* Admits only the source-verified raw grid as a level route. The paired
 * bitmap and object routes are explicitly unavailable with no fallback. */
int theron_v1_track02_loader_intake_admit_raw_grid_level_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridLevelRouteReceipt *out_receipt);

#endif
