#ifndef THERON_V1_TRACK02_LOADER_INTAKE_H
#define THERON_V1_TRACK02_LOADER_INTAKE_H

#include <stdint.h>

/* The original loader trace identifies this later Track 02 envelope before
 * its payload format is known. Keep the physical record facts here rather
 * than depending on an unimplemented dungeon-handoff API. */
#define THERON_V1_INITIAL_ENVELOPE_RECORD 0x0b52u
#define THERON_V1_INITIAL_ENVELOPE_RECORD_USER_DATA_OFFSET 0x114u
#define THERON_V1_INITIAL_ENVELOPE_DESTINATION 0x3800u
#define THERON_V1_INITIAL_ENVELOPE_PAYLOAD_BYTES 2048u

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
    int complete_payload_witness_verified;
    uint32_t complete_payload_checksum;
} Theron_V1Track02LoaderReadFacts;

typedef struct {
    int observed;
    int payload_intake_admitted;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t observed_destination;
    uint32_t observed_byte_count;
    uint32_t observed_payload_checksum;
    const char *status;
} Theron_V1Track02LoaderIntakeReceipt;

/* Accepts only a provenance-authenticated later read of the source-locked
 * initial envelope. It deliberately leaves payload intake blocked until
 * authenticated trace must also retain the observed $3800 one-sector payload
 * witness. Intake stays blocked until independent evidence establishes what
 * the observed transfer means. */
int theron_v1_track02_loader_intake_observe(
    const Theron_V1Track02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

#endif
