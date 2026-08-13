#ifndef THERON_V1_MEDNAFEN_VDC_IO_TRACE_H
#define THERON_V1_MEDNAFEN_VDC_IO_TRACE_H

#include <stdint.h>

#define THERON_V1_VDC_IO_TRACE_PATH_CAPACITY 512
#define THERON_V1_VDC_IO_TRACE_MAX_WRITES 65536u

typedef enum {
    THERON_V1_VDC_IO_TRACE_UNAVAILABLE = 0,
    THERON_V1_VDC_IO_TRACE_REJECTED,
    THERON_V1_VDC_IO_TRACE_READY
} Theron_V1VdcIoTraceStatus;

/* Provenance for side-effect-free writes observed at the original PCE VDC
 * port. This receipt proves an original CPU→VDC producer, not the source
 * bytes, BAT meaning, text consumer, square mapping or presented screen. */
typedef struct {
    Theron_V1VdcIoTraceStatus status;
    int source_header_verified;
    int sequence_verified;
    int timestamp_verified;
    int address_bounds_verified;
    int register_bounds_verified;
    int semantic_publication_allowed;
    uint32_t write_count;
    uint32_t first_timestamp;
    uint32_t last_timestamp;
    uint32_t first_logical_address;
    uint32_t last_logical_address;
    uint32_t first_physical_address;
    uint32_t last_physical_address;
    uint16_t first_writer_pc;
    uint16_t last_writer_pc;
    uint32_t first_writer_physical_pc;
    uint32_t last_writer_physical_pc;
    uint8_t last_a;
    uint8_t last_x;
    uint8_t last_y;
    char source_trace_path[THERON_V1_VDC_IO_TRACE_PATH_CAPACITY];
} Theron_V1VdcIoTraceReceipt;

int theron_v1_mednafen_vdc_io_trace_parse_file(
    const char *path,
    Theron_V1VdcIoTraceReceipt *out);

#endif
