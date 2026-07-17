#ifndef THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_INTAKE_H
#define THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_INTAKE_H

#include "theron_v1_track02_capture_artifact_importer.h"
#include "theron_v1_track02_level_object_descriptor_capture_intake.h"

typedef enum {
    THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_UNAVAILABLE = 0,
    THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_REJECTED,
    THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_READY
} Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeStatus;

/* A source-bound presentation witness. The identity fields name observed
 * capture outputs only; no bitmap bytes, palette entries, or decoded format
 * crosses this boundary. */
typedef struct {
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeStatus status;
    int direct_cue_bin_consumed;
    int coalesced_loader_trace_consumed;
    int replay_tail_consumed;
    int descriptor_chain_consumed;
    int palette_output_consumed;
    int bitmap_transfer_consumed;
    int opaque_presentation_only;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    char coalesced_trace_md5[33];
    uint32_t campaign_layout_epoch;
    uint32_t campaign_media_scan_epoch;
    uint32_t capture_target_plan_identity;
    uint32_t cd_read_record;
    uint16_t descriptor_selector;
    size_t descriptor_ordinal;
    uint32_t descriptor_source_hash;
    uint32_t descriptor_record;
    size_t loader_output_raw_offset;
    size_t loader_output_bytes;
    uint32_t loader_output_identity;
    uint32_t palette_output_identity;
    uint32_t bitmap_transfer_identity;
    size_t destination_offset;
    size_t destination_bytes;
    uint32_t destination_identity;
} Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt;

/* Attests the dungeon-handoff bitmap/palette capture row only after it agrees
 * with the current descriptor-selected record, direct media, replay, trace,
 * epochs, and external capture artifact. Missing artifacts are UNAVAILABLE.
 * A READY receipt remains presentation no-draw evidence. */
int theron_v1_track02_descriptor_bitmap_palette_capture_intake_admit(
    const Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt *descriptor,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02LaunchTraceIdentityReceipt *trace_identity,
    const Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *artifact,
    uint32_t campaign_layout_epoch,
    uint32_t campaign_media_scan_epoch,
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt *out);

#endif
