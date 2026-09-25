#ifndef THERON_V1_MEDNAFEN_VDC_IO_TRACE_H
#define THERON_V1_MEDNAFEN_VDC_IO_TRACE_H

#include <stddef.h>
#include <stdint.h>

#define THERON_V1_VDC_IO_TRACE_PATH_CAPACITY 512
#define THERON_V1_VDC_IO_TRACE_LEGACY_WRITES 65536u
#define THERON_V1_VDC_IO_TRACE_MAX_WRITES 2097152u

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
    int physical_bus_marker_verified;
    int register_bounds_verified;
    int semantic_publication_allowed;
    int snapshot_boundary_verified;
    int source_sequence_verified;
    uint32_t write_count;
    uint32_t source_sequence_base;
    uint32_t first_source_sequence;
    uint32_t last_source_sequence;
    uint32_t first_timestamp;
    uint32_t last_timestamp;
    /* Compatibility names: these count observed timestamp runs/regressions,
     * not proven monotonic HuC6280 execution epochs. */
    uint32_t timestamp_epoch_count;
    uint32_t timestamp_reset_count;
    uint32_t snapshot_boundary_sequence;
    uint32_t snapshot_boundary_timestamp;
    uint32_t first_logical_address;
    uint32_t last_logical_address;
    uint32_t first_physical_address;
    uint32_t last_physical_address;
    uint32_t first_normalized_physical_address;
    uint32_t last_normalized_physical_address;
    uint32_t marked_physical_write_count;
    uint16_t first_writer_pc;
    uint16_t last_writer_pc;
    uint32_t first_writer_physical_pc;
    uint32_t last_writer_physical_pc;
    uint8_t last_a;
    uint8_t last_x;
    uint8_t last_y;
    char source_trace_path[THERON_V1_VDC_IO_TRACE_PATH_CAPACITY];
} Theron_V1VdcIoTraceReceipt;

typedef struct {
    uint32_t sequence;
    uint32_t timestamp;
    uint16_t logical_address;
    uint32_t raw_physical_address;
    uint32_t normalized_physical_address;
    uint8_t value;
    uint16_t writer_pc;
    uint32_t writer_physical_pc;
    uint8_t a;
    uint8_t x;
    uint8_t y;
} Theron_V1VdcIoWrite;

typedef struct {
    Theron_V1VdcIoTraceReceipt receipt;
    Theron_V1VdcIoWrite *writes;
    size_t write_count;
} Theron_V1VdcIoTrace;

typedef struct {
    uint32_t vwr_commit_count;
    uint32_t written_word_count;
    uint32_t matched_word_count;
    uint32_t mismatched_word_count;
    int atomic_snapshot_verified;
    int semantic_publication_allowed;
} Theron_V1VdcIoVramReplayReceipt;

int theron_v1_mednafen_vdc_io_trace_parse_file(
    const char *path,
    Theron_V1VdcIoTraceReceipt *out);

/* Retain every write only after the complete trace passes the same bounded
 * provenance checks as parse_file().  The caller owns the returned array and
 * must release it with theron_v1_mednafen_vdc_io_trace_free(). */
int theron_v1_mednafen_vdc_io_trace_load_file(
    const char *path,
    Theron_V1VdcIoTrace *out);

void theron_v1_mednafen_vdc_io_trace_free(Theron_V1VdcIoTrace *trace);

/* Replay only HuC6270 register/VWR transport and compare every word touched
 * by the bounded stream with its same-instant 64 KiB VRAM snapshot.  This is
 * a provenance check, not permission to publish dungeon or screen meaning. */
int theron_v1_mednafen_vdc_io_verify_vram_snapshot(
    const Theron_V1VdcIoTrace *trace,
    const char *vram_path,
    Theron_V1VdcIoVramReplayReceipt *out);

#endif
