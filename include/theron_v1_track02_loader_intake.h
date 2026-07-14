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
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t observed_destination;
    uint32_t observed_byte_count;
    const char *status;
} Theron_V1Track02LoaderIntakeReceipt;

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

#endif
