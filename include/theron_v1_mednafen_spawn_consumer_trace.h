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
    int spawn_entry_b0e5_seen;
    int preconsumer_4644_seen;
    int helper_4667_seen;
    /* THQUEST.ASM L4667 takes its RAM-loaded RNG path only when
     * ($B3 & $07) == $04. Seeing the helper entry alone is not evidence
     * that this branch ran or that a returned random value was consumed. */
    int helper_4667_special_branch_seen;
    int semantic_publication_allowed;
    uint32_t sample_count;
    uint32_t first_pc;
    uint32_t last_pc;
    uint32_t first_physical_pc;
    uint32_t last_physical_pc;
    uint8_t last_mpr_pc;
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

/* Authenticated execution-window receipt for a real register sidecar. This
 * admits less than the strict parser above: observing both disassembly bodies
 * is useful evidence, but without the $4644 preconsumer and $4667 helper
 * edges it is not a runtime RNG/spawn contract. */
int theron_v1_mednafen_spawn_register_trace_parse_execution_window_file(
    const char *path, Theron_V1SpawnRegisterTraceReceipt *out);

/* Raw execution-window provenance for the two RAM-loaded RNG consumers
 * identified by the US disassembly.  The sidecar records register/RAM
 * snapshots only; it does not identify a returned random value, caller,
 * or semantic consumer. The stack-derived return boundary is retained as
 * provenance and is not itself semantic publication. */
typedef struct {
    Theron_V1SpawnConsumerTraceStatus status;
    int source_header_verified;
    int sequence_verified;
    int step_verified;
    int physical_pc_bounds_verified;
    int boundary_flags_verified;
    int target_5d64_seen;
    int target_5d6a_seen;
    int complete_window_seen;
    int return_boundary_seen;
    int semantic_publication_allowed;
    uint32_t sample_count;
    uint32_t window_count;
    uint32_t first_sequence;
    uint32_t last_sequence;
    uint32_t first_step;
    uint32_t last_step;
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
    uint8_t last_entry_sp;
    uint16_t last_return_pc;
    char source_trace_path[THERON_V1_SPAWN_CONSUMER_TRACE_PATH_CAPACITY];
} Theron_V1RngConsumerTraceReceipt;

int theron_v1_mednafen_rng_consumer_trace_parse_file(
    const char *path, Theron_V1RngConsumerTraceReceipt *out);

/* Correlates the two sidecars emitted by one instrumented run.  This is a
 * stronger capture handoff than either parser alone, but it deliberately
 * remains non-semantic: the dynamic RAM-loaded callees and return ownership
 * are not identified by window provenance. */
typedef struct {
    int ready;
    int consumer_ready;
    int registers_ready;
    int source_windows_paired;
    int dynamic_return_contract_verified;
    int semantic_publication_allowed;
    uint32_t consumer_read_count;
    uint32_t register_sample_count;
} Theron_V1SpawnCaptureCorrelationReceipt;

int theron_v1_mednafen_spawn_capture_correlate_files(
    const char *consumer_path,
    const char *register_path,
    Theron_V1SpawnCaptureCorrelationReceipt *out);

#endif
