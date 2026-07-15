#ifndef THERON_V1_TRACK02_LOADER_INTAKE_H
#define THERON_V1_TRACK02_LOADER_INTAKE_H

#include <stddef.h>
#include <stdint.h>

#include "theron_v1_track02.h"

/* The original loader trace identifies this later Track 02 envelope before
 * its payload format is known. Keep the physical record facts here rather
 * than depending on an unimplemented dungeon-handoff API. */
#define THERON_V1_INITIAL_ENVELOPE_RECORD 0x0b52u
#define THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET 0x114u
#define THERON_V1_INITIAL_ENVELOPE_DESTINATION 0x3800u
#define THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES 2048u
#define THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES (12u + 32u * 27u)
#define THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET \
    (THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET + \
     THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES)
#define THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES \
    (THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES - \
     THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_OFFSET)

/* This is an observation boundary, not a payload decoder. The destination and
 * byte count are retained only when an original later loader read reports
 * them; neither value is interpreted as a memory or record layout. */
typedef struct {
    int authenticated_original_trace;
    int later_than_stage2_transfer;
    Theron_Track02Variant track02_variant;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t byte_count;
    int complete_payload_witness_verified;
    uint32_t complete_payload_checksum;
} Theron_V1Track02LoaderReadFacts;

/* Independent MODE1/2048 ISO capture for the same first level sector. This
 * is deliberately not accepted by the raw-BIN loader intake: an ISO handoff
 * must prove its own CUE/user-data route and must not borrow raw-sector trace
 * coordinates, apply a 2352->2048 conversion, or promote a synthetic dungeon. */
typedef struct {
    int authenticated_original_iso_capture;
    int cue_declares_mode1_2048;
    int raw_bin_trace_borrowed;
    int sector_conversion_applied;
    int synthetic_dungeon_promoted;
    Theron_Track02Variant track02_variant;
    uint32_t track02_record;
    uint32_t destination;
    uint32_t byte_count;
    int complete_payload_witness_verified;
    uint32_t complete_payload_checksum;
    int level_envelope_witness_verified;
    uint32_t level_envelope_checksum;
    int post_envelope_witness_verified;
    uint32_t post_envelope_checksum;
} Theron_V1Track02IsoLevelObjectReadFacts;

typedef struct {
    int observed;
    int payload_intake_admitted;
    Theron_Track02Variant track02_variant;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t observed_destination;
    uint32_t observed_byte_count;
    uint32_t observed_payload_checksum;
    const char *status;
} Theron_V1Track02LoaderIntakeReceipt;

/* The observed later $e009 read is one complete MODE1 user-data sector.  Keep
 * its bytes together with the checked loader coordinates so a runtime caller
 * can consume the original payload without reopening a generated-data route.
 * This is intentionally an opaque transfer boundary: no dungeon, object,
 * bitmap, palette, or transition grammar is assigned here. */
typedef struct {
    int handed_off;
    int no_fallback;
    Theron_Track02Variant track02_variant;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t payload_bytes;
    uint32_t payload_checksum;
    uint8_t payload[THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES];
    const char *status;
} Theron_V1Track02LoaderPayloadReceipt;

/* Exact record-local slice of the witnessed payload that is already proven
 * to be the initial level envelope. It is a byte handoff only: callers still
 * need independently source-locked level-loader evidence before treating it
 * as a runtime route. */
typedef struct {
    int handed_off;
    int no_fallback;
    Theron_Track02Variant track02_variant;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t envelope_bytes;
    uint32_t envelope_checksum;
    uint8_t envelope[THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES];
    const char *status;
} Theron_V1Track02LoaderLevelEnvelopeReceipt;

/* The remaining bytes in the same original sector after the proven level
 * envelope. Their ownership and grammar are not established: the name avoids
 * calling them an object table. They are copied only after the authenticated
 * full-sector handoff and the raw-media boundary agree exactly. */
typedef struct {
    int handed_off;
    int no_fallback;
    Theron_Track02Variant track02_variant;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t byte_count;
    uint32_t checksum;
    uint8_t bytes[THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES];
    const char *status;
} Theron_V1Track02LoaderPostEnvelopeReceipt;

/* Byte-faithful ISO handoff for the first level sector. The post-envelope
 * span is retained only as opaque source bytes; object/bitmap semantics stay
 * blocked until an original ISO loader consumer proves them. */
typedef struct {
    int handed_off;
    int no_fallback;
    int original_iso_capture;
    int cue_mode1_2048;
    int no_raw_bin_trace_borrowing;
    int no_sector_conversion;
    int no_synthetic_dungeon;
    Theron_Track02Variant track02_variant;
    uint32_t record;
    uint32_t destination;
    uint32_t payload_bytes;
    uint32_t payload_checksum;
    uint32_t level_envelope_offset;
    uint32_t level_envelope_bytes;
    uint32_t level_envelope_checksum;
    uint8_t level_envelope[THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES];
    uint32_t post_envelope_offset;
    uint32_t post_envelope_bytes;
    uint32_t post_envelope_checksum;
    uint8_t post_envelope[THERON_V1_INITIAL_LEVEL_POST_ENVELOPE_BYTES];
    const char *status;
} Theron_V1Track02IsoLevelObjectReceipt;

/* Accepts only a provenance-authenticated later read of the source-locked
 * initial envelope. It deliberately leaves payload intake blocked until
 * authenticated trace must also retain the observed $3800 one-sector payload
 * witness. Intake stays blocked until independent evidence establishes what
 * the observed transfer means. */
int theron_v1_track02_loader_intake_observe(
    const Theron_V1Track02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Copies only the exact, complete payload witnessed by the original loader.
 * The input receipt and byte checksum must agree; a mismatch leaves the
 * output empty.  This does not decode or classify the copied bytes. */
int theron_v1_track02_loader_intake_handoff_complete_payload(
    const Theron_V1Track02LoaderIntakeReceipt *intake,
    const uint8_t *payload,
    size_t payload_bytes,
    Theron_V1Track02LoaderPayloadReceipt *out_receipt);

/* Copies a source-locked subrange from the witnessed original sector. The
 * caller supplies the independently decoded record coordinate and checksum;
 * malformed, out-of-range, or changed bytes fail closed. */
int theron_v1_track02_loader_intake_handoff_level_envelope(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    uint32_t record_user_data_offset,
    uint32_t envelope_bytes,
    uint32_t envelope_checksum,
    Theron_V1Track02LoaderLevelEnvelopeReceipt *out_receipt);

/* Preserves the source-locked tail immediately following the initial level
 * envelope. This is an opaque byte handoff, not an object-record decoder. */
int theron_v1_track02_loader_intake_handoff_initial_level_post_envelope(
    const Theron_V1Track02LoaderPayloadReceipt *payload,
    uint32_t post_envelope_checksum,
    Theron_V1Track02LoaderPostEnvelopeReceipt *out_receipt);

/* Copies the first MODE1/2048 ISO level-sector bytes only after a separate
 * ISO capture proves the record, destination, and exact payload checksum.
 * Raw-BIN variants, sector-converted payloads, borrowed raw traces, and
 * synthetic dungeon promotion all fail closed. */
int theron_v1_track02_loader_intake_handoff_iso_level_object_record(
    const Theron_V1Track02IsoLevelObjectReadFacts *facts,
    const uint8_t *payload,
    size_t payload_bytes,
    Theron_V1Track02IsoLevelObjectReceipt *out_receipt);

#endif
