#ifndef THERON_V1_DUNGEON_HANDOFF_H
#define THERON_V1_DUNGEON_HANDOFF_H

#include <stdint.h>

#include "theron_v1_runtime_admission.h"

#define THERON_V1_INITIAL_LEVEL_RECORD 0x0b52u
#define THERON_V1_INITIAL_LEVEL_USER_DATA_OFFSET 0x114u
#define THERON_V1_INITIAL_LEVEL_ENVELOPE_BYTES 0x36cu
#define THERON_V1_INITIAL_LEVEL_IDENTIFIER 0x0026u

typedef struct {
    const Theron_V1RuntimeAdmissionReceipt *runtime_admission;
    int track02_hash_verified;
    int initial_level_envelope_verified;
    int adjacent_boundary_unparsed;
    uint32_t record;
    uint32_t user_data_offset;
    uint32_t envelope_bytes;
    uint16_t level_identifier;
} Theron_V1DungeonHandoffFacts;

typedef struct {
    int selected;
    int runtime_route_consumed;
    uint32_t record;
    uint32_t user_data_offset;
    uint32_t envelope_bytes;
    uint16_t level_identifier;
    const char *route;
} Theron_V1DungeonHandoffReceipt;

/* Selects only the hash-verified initial-level envelope. It exposes no level
 * bytes or grammar, and requires the neighboring source span to remain opaque. */
int theron_v1_dungeon_handoff_select_initial_level(
    const Theron_V1DungeonHandoffFacts *facts,
    Theron_V1DungeonHandoffReceipt *out_receipt);

#endif
