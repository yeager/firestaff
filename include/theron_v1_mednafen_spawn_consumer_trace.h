#ifndef THERON_V1_MEDNAFEN_SPAWN_CONSUMER_TRACE_H
#define THERON_V1_MEDNAFEN_SPAWN_CONSUMER_TRACE_H

#include <stdint.h>

#define THERON_V1_SPAWN_CONSUMER_TRACE_PATH_CAPACITY 512

typedef enum {
    THERON_V1_SPAWN_CONSUMER_TRACE_UNAVAILABLE = 0,
    THERON_V1_SPAWN_CONSUMER_TRACE_REJECTED,
    THERON_V1_SPAWN_CONSUMER_TRACE_READY
} Theron_V1SpawnConsumerTraceStatus;

/* Execution provenance for the disassembly-bound US Track 02 spawn windows.
 * This receipt intentionally has no RNG value, creature type, spawn count or
 * return-value field: accepting the CPU edge is not semantic publication. */
typedef struct {
    Theron_V1SpawnConsumerTraceStatus status;
    int source_header_verified;
    int sequence_verified;
    int bank_coordinates_verified;
    int boundary_flags_verified;
    int target_5d64_seen;
    int target_5d6a_seen;
    int c96b_window_seen;
    int cc4c_window_seen;
    int semantic_publication_allowed;
    uint32_t read_count;
    uint32_t first_logical_address;
    uint32_t first_physical_address;
    uint32_t first_reader_pc;
    uint32_t first_reader_physical_pc;
    uint32_t last_logical_address;
    uint32_t last_physical_address;
    uint32_t last_reader_pc;
    uint32_t last_reader_physical_pc;
    char source_trace_path[THERON_V1_SPAWN_CONSUMER_TRACE_PATH_CAPACITY];
} Theron_V1SpawnConsumerTraceReceipt;

int theron_v1_mednafen_spawn_consumer_trace_parse_file(
    const char *path, Theron_V1SpawnConsumerTraceReceipt *out);

/* Strict register-sidecar provenance for the same disassembly windows.  The
 * register values are retained for later dynamic analysis, but this receipt
 * never turns them into an RNG result, creature record, or spawn action. */
typedef struct {
    Theron_V1SpawnConsumerTraceStatus status;
    int source_header_verified;
    int sequence_verified;
    int bank_coordinates_verified;
    int boundary_flags_verified;
    int c96b_window_seen;
    int cc4c_window_seen;
    int preconsumer_4644_seen;
    int helper_4667_seen;
    int semantic_publication_allowed;
    uint32_t sample_count;
    uint32_t first_pc;
    uint32_t last_pc;
    uint32_t first_physical_pc;
    uint32_t last_physical_pc;
    uint8_t last_a;
    uint8_t last_x;
    uint8_t last_y;
    uint8_t last_sp;
    uint8_t last_p;
    uint8_t last_mpr0;
    uint8_t last_b3;
    uint8_t last_b4;
    uint8_t last_b5;
    uint8_t last_b6;
    uint8_t last_b8;
    uint8_t last_ba;
    uint8_t last_bb;
    char source_trace_path[THERON_V1_SPAWN_CONSUMER_TRACE_PATH_CAPACITY];
} Theron_V1SpawnRegisterTraceReceipt;

int theron_v1_mednafen_spawn_register_trace_parse_file(
    const char *path, Theron_V1SpawnRegisterTraceReceipt *out);

#endif
