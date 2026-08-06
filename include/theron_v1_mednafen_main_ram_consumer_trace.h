#ifndef THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_H
#define THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_H

#include <stdint.h>

#define THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_PATH_CAPACITY 512

typedef enum {
    THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_UNAVAILABLE = 0,
    THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_REJECTED,
    THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_READY
} Theron_V1MednafenMainRamConsumerTraceStatus;

/* Receipt for real Mednafen reads made by code executing in the Track 02
 * consumer bank. This records execution provenance only. It deliberately
 * does not publish the bytes as `$2600` data or as object/level records. */
typedef struct {
    Theron_V1MednafenMainRamConsumerTraceStatus status;
    int source_trace_md5_verified;
    int source_header_verified;
    int bank_coordinates_verified;
    int target_2600_bytes_present;
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
    char source_trace_path[THERON_V1_MEDNAFEN_MAIN_RAM_CONSUMER_TRACE_PATH_CAPACITY];
    char source_trace_md5[33];
} Theron_V1MednafenMainRamConsumerTraceReceipt;

int theron_v1_mednafen_main_ram_consumer_trace_parse_file(
    const char *path,
    Theron_V1MednafenMainRamConsumerTraceReceipt *out);

#endif
