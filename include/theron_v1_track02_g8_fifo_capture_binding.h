#ifndef THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_H
#define THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_H

#include <stdint.h>

#include "theron_v1_track02_g8_fifo_sidecar.h"
#include "theron_v1_track02_handoff_artifact_corpus.h"

typedef enum {
    THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_UNAVAILABLE = 0,
    THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_REJECTED,
    THERON_V1_TRACK02_G8_FIFO_CAPTURE_BINDING_CAPTURE_REQUIRED
} Theron_V1Track02G8FifoCaptureBindingStatus;

/* Immutable provenance join for the observed G8 FIFO row. It retains only
 * identities and metadata; no FIFO bytes, route admission, decode, or draw. */
typedef struct {
    Theron_V1Track02G8FifoCaptureBindingStatus status;
    int sidecar_consumed;
    int artifact_corpus_source_trace_md5_consumed;
    int m11_lifecycle_source_trace_md5_consumed;
    int capture_required_only;
    int no_draw_only;
    uint32_t lifecycle_scan_epoch;
    uint32_t generation;
    uint32_t lba;
    uint32_t dispatch;
    uint32_t fifo_sequence;
    uint32_t fingerprint;
    uint32_t capture_target_plan_identity;
    uint32_t source_offset;
    uint32_t first_fifo_sequence;
    uint32_t last_fifo_sequence;
    uint32_t capture_byte_count;
    uint32_t source_window_offset;
    uint32_t source_window_bytes;
    uint32_t sequence_window_identity;
    uint16_t dispatch_logical_pc;
    uint32_t dispatch_physical_pc;
    uint8_t dispatch_a;
    uint8_t dispatch_x;
    uint8_t dispatch_y;
    uint8_t cdb_opcode;
    uint8_t cdb_sector_count;
    uint8_t cdb[6];
    uint32_t cdb_lba;
    uint32_t capture_cdb_identity;
    uint16_t reader_pc;
    uint16_t logical_destination;
    uint32_t physical_destination;
    uint16_t writer_pc;
    uint32_t writer_physical_pc;
    uint8_t value;
    char sidecar_trace_md5[33];
    char capture_file_md5[33];
    uint32_t capture_file_fnv1a;
    uint32_t capture_row_count;
    uint32_t capture_file_identity;
    char source_trace_md5[33];
    char artifact_bundle_md5[33];
} Theron_V1Track02G8FifoCaptureBindingReceipt;

int theron_v1_track02_g8_fifo_capture_binding_bind(
    const Theron_V1Track02G8FifoSidecarReceipt* sidecar,
    const Theron_V1Track02HandoffArtifactCorpusReceipt* artifact_corpus,
    const char* m11_lifecycle_source_trace_md5,
    uint32_t lifecycle_scan_epoch,
    Theron_V1Track02G8FifoCaptureBindingReceipt* out);

int theron_v1_track02_g8_fifo_capture_binding_matches_lifecycle(
    const Theron_V1Track02G8FifoCaptureBindingReceipt* receipt,
    const Theron_V1Track02HandoffArtifactCorpusReceipt* artifact_corpus,
    const char* m11_lifecycle_source_trace_md5,
    uint32_t lifecycle_scan_epoch);

#endif
