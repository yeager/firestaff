#ifndef THERON_V1_TRACK02_LOADER_OUTPUT_RECORD_ADMISSION_H
#define THERON_V1_TRACK02_LOADER_OUTPUT_RECORD_ADMISSION_H

#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02_handoff_artifact_corpus.h"

/* Bounded source-owned slices of the one fully disassembled later $e009
 * record. `bitmap_boundary_proven` remains zero: no original observation
 * identifies a bitmap subrange in this loader output. */
typedef struct {
    int valid;
    int original_loader_output_consumed;
    int envelope_header_fields_proven;
    int level_boundary_proven;
    int bitmap_boundary_proven;
    int object_continuation_boundary_proven;
    int no_draw_only;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    uint32_t record;
    uint16_t destination;
    size_t loader_output_bytes;
    uint32_t loader_output_checksum;
    uint16_t header_width;
    uint16_t header_height;
    uint32_t header_seed;
    uint16_t header_level_index;
    uint16_t header_extension_be;
    size_t level_offset;
    size_t level_bytes;
    uint32_t level_checksum;
    size_t object_continuation_offset;
    size_t object_continuation_bytes;
    uint32_t object_continuation_checksum;
    int level_semantics_allowed;
    int header_level_identifier_semantics_allowed;
    int object_semantics_allowed;
    int bitmap_semantics_allowed;
    int pixel_decode_allowed;
    int render_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LoaderOutputRecordAdmissionReceipt;

/* Accepts only the manifest-bound, original 0x0b52 $e009 output already
 * proven by the Stage-3 descriptor chain and current artifact corpus. */
int theron_v1_track02_loader_output_record_admit(
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *handoff,
    const Theron_V1Track02HandoffArtifactCorpusReceipt *artifact_corpus,
    const Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *sector_corpus,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const char *expected_source_trace_md5,
    Theron_V1Track02LoaderOutputRecordAdmissionReceipt *out);

#endif
