#ifndef THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_H
#define THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_H

#include <stdint.h>

#define THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_PATH_CAPACITY 512

typedef enum {
    THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_UNAVAILABLE = 0,
    THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_REJECTED,
    THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_READY
} Theron_V1MednafenMainRamTraceStatus;

/* Opaque receipt for an observed Main-RAM loader trace. It proves transfer
 * control/data coordinates only; it never treats the transfer as `$2600`
 * code or as a level/object record. */
typedef struct {
    Theron_V1MednafenMainRamTraceStatus status;
    int source_trace_md5_verified;
    int source_header_verified;
    int transfer_coordinates_verified;
    int target_2600_bytes_present;
    int semantic_publication_allowed;
    uint32_t block_transfer_count;
    uint32_t rts_count;
    uint32_t post_rts_count;
    uint32_t first_logical_pc;
    uint32_t first_physical_pc;
    uint32_t first_source;
    uint32_t first_destination;
    uint32_t first_length;
    char source_trace_path[THERON_V1_MEDNAFEN_MAIN_RAM_TRACE_PATH_CAPACITY];
    char source_trace_md5[33];
} Theron_V1MednafenMainRamTraceReceipt;

/* Parse a real Mednafen Main-RAM loader sidecar without copying RAM bytes. */
int theron_v1_mednafen_main_ram_trace_parse_file(
    const char *path,
    Theron_V1MednafenMainRamTraceReceipt *out);

#endif
