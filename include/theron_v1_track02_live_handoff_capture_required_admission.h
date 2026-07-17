#ifndef THERON_V1_TRACK02_LIVE_HANDOFF_CAPTURE_REQUIRED_ADMISSION_H
#define THERON_V1_TRACK02_LIVE_HANDOFF_CAPTURE_REQUIRED_ADMISSION_H

#include "theron_v1_track02_dynamic_cd_read_ownership.h"
#include "theron_v1_track02_g8_fifo_capture_binding.h"
#include "theron_v1_track02_live_loader_route_admission.h"
#include "theron_v1_track02_loader_output_record_admission.h"

/* Joins two independently observed loader records only as current no-draw
 * capture provenance. The initial loader-output record and the live dynamic
 * CD_READ record remain distinct; this receipt never exposes their bytes. */
typedef struct {
    int valid;
    int loader_output_record_consumed;
    int g8_fifo_capture_consumed;
    int live_handoff_consumed;
    int dynamic_cd_read_ownership_consumed;
    int capture_required_only;
    int no_draw_only;
    uint32_t lifecycle_scan_epoch;
    uint32_t capture_target_plan_identity;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char source_trace_md5[33];
    uint32_t loader_output_record;
    uint16_t loader_output_destination;
    uint32_t live_handoff_record;
    uint16_t live_handoff_destination;
    uint32_t g8_lba;
    uint32_t g8_capture_file_identity;
    int level_object_semantics_allowed;
    int bitmap_palette_admission_allowed;
    int pixel_decode_allowed;
    int dungeon_draw_allowed;
    int fallback_visuals_allowed;
} Theron_V1Track02LiveHandoffCaptureRequiredAdmissionReceipt;

int theron_v1_track02_live_handoff_capture_required_admit(
    const Theron_V1Track02LoaderOutputRecordAdmissionReceipt *loader_output,
    const Theron_V1Track02G8FifoCaptureBindingReceipt *g8_binding,
    const Theron_V1Track02LiveLoaderRouteAdmissionReceipt *live_handoff,
    const Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership,
    const Theron_V1Track02HandoffArtifactCorpusReceipt *artifact_corpus,
    uint32_t lifecycle_scan_epoch,
    Theron_V1Track02LiveHandoffCaptureRequiredAdmissionReceipt *out);

#endif
