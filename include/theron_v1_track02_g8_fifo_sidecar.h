#ifndef THERON_V1_TRACK02_G8_FIFO_SIDECAR_H
#define THERON_V1_TRACK02_G8_FIFO_SIDECAR_H

#include <stdint.h>
typedef struct {
    int valid;
    char trace_md5[33];
    char capture_file_md5[33];
    uint32_t capture_file_fnv1a, capture_row_count, capture_file_identity;
    uint32_t generation, lba, dispatch, source_offset, fifo_sequence, fingerprint;
    uint32_t first_fifo_sequence, last_fifo_sequence, capture_byte_count;
    uint32_t source_window_offset, source_window_bytes, sequence_window_identity;
    uint16_t dispatch_logical_pc;
    uint32_t dispatch_physical_pc;
    uint8_t dispatch_a, dispatch_x, dispatch_y;
    uint8_t cdb_opcode, cdb_sector_count, cdb[6];
    uint32_t cdb_lba, capture_cdb_identity;
    uint16_t reader_pc, logical_destination, writer_pc;
    uint32_t physical_destination, writer_physical_pc;
    uint8_t value;
} Theron_V1Track02G8FifoSidecarReceipt;
int theron_v1_track02_g8_fifo_sidecar_parse_file(const char*, Theron_V1Track02G8FifoSidecarReceipt*);
#endif
