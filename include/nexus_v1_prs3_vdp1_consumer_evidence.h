#ifndef NEXUS_V1_PRS3_VDP1_CONSUMER_EVIDENCE_H
#define NEXUS_V1_PRS3_VDP1_CONSUMER_EVIDENCE_H

#include <stdint.h>

#include "nexus_v1_prs3_capture_trace_schema.h"
#include "nexus_v1_prs3_structure2_abi.h"

#define NEXUS_V1_PRS3_VDP1_CONSUMER_CAPTURE_TARGET_MAGIC \
    "FIRESTAFF_NEXUS_PRS3_ENTRY5_STRUCTURE2_VDP1_CONSUMER_TARGET_V1"

typedef enum {
    NEXUS_V1_PRS3_VDP1_CONSUMER_READY_BLOCKED = 0,
    NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_INPUT = 1,
    NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_ABI = 2,
    NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_EMULATOR = 3,
    NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_TRACE = 4,
    NEXUS_V1_PRS3_VDP1_CONSUMER_BLOCKED_CONSUMER_SEMANTICS = 5
} Nexus_V1_Prs3Vdp1ConsumerEvidenceStatus;

typedef struct {
    const Nexus_V1_Prs3Structure2AbiReceipt *abi;
    int retail_media_available;
    int saturn_emulator_available;
    int saturn_bios_available;
    int raw_trace_artifact_available;
    uint64_t raw_trace_fnv1a64;
    uint32_t raw_trace_size;
    int raw_trace_authenticated;
    int collector_manifest_only;
    int trace_binds_dm_bin;
    int trace_binds_menu_bpk_entry5;
    int trace_binds_lev00_structure2;
    int vdp1_command_observed;
    int vdp1_texture_window_observed;
    int be16_palette_application_observed;
    int structure2_descriptor_selection_observed;
    int texture_placement_observed;
    uint64_t prs3_header_span_fnv1a64;
    uint64_t prs3_bitmap_candidate_fnv1a64;
    uint32_t prs3_bitmap_candidate_offset;
    uint32_t prs3_bitmap_candidate_size;
    uint64_t palt_candidate_fnv1a64;
    uint32_t palt_candidate_size;
} Nexus_V1_Prs3Vdp1ConsumerEvidenceInput;

typedef struct {
    Nexus_V1_Prs3Vdp1ConsumerEvidenceStatus status;
    int capture_target_bound;
    int retail_media_available;
    int saturn_emulator_available;
    int saturn_bios_available;
    int requires_independent_trace;
    int rejects_collector_manifest_only;
    uint32_t prs3_entry_index;
    uint32_t prs3_stream_offset;
    uint32_t prs3_stream_size;
    uint32_t prs3_expected_output_bytes;
    uint32_t prs3_width;
    uint32_t prs3_height;
    uint32_t prs3_bpp;
    uint64_t prs3_output_fnv1a64;
    uint64_t palt_entries_fnv1a64;
    int palt_entries_are_be16;
    int structure2_descriptor_count;
    int structure2_image_anchor_count;
    int structure2_palette_anchor_count;
    int structure2_palette_absent_count;
    int trace_artifact_available;
    uint64_t raw_trace_fnv1a64;
    uint32_t raw_trace_size;
    int raw_trace_authenticated;
    int trace_binds_dm_bin;
    int trace_binds_menu_bpk_entry5;
    int trace_binds_lev00_structure2;
    int vdp1_command_observed;
    int vdp1_texture_window_observed;
    int be16_palette_application_observed;
    int structure2_descriptor_selection_observed;
    int texture_placement_observed;
    int candidate_spans_bound;
    uint64_t prs3_header_span_fnv1a64;
    uint64_t prs3_bitmap_candidate_fnv1a64;
    uint32_t prs3_bitmap_candidate_offset;
    uint32_t prs3_bitmap_candidate_size;
    uint64_t palt_candidate_fnv1a64;
    uint32_t palt_candidate_size;
    int pixel_format_proven;
    int palette_application_proven;
    int descriptor_semantics_proven;
    int texture_placement_proven;
    int can_submit_structure2_pixels;
    int can_submit_palette;
    int runtime_m11_handoff_permitted;
    int fallback_visuals_permitted;
    int no_draw_only;
} Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt;

int nexus_v1_prs3_vdp1_consumer_evidence_admit(
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceInput *input,
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *out_receipt);

/* Consume a parsed, asset-bound PRS3/VDP1 trace without inventing Structure2
 * placement or pixel semantics. A valid trace may reach only the existing
 * no-draw consumer-semantic blocker until those lanes are independently
 * observed. */
int nexus_v1_prs3_vdp1_consumer_evidence_admit_capture_binding(
    const Nexus_V1_Prs3Structure2AbiReceipt *abi,
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace,
    const Nexus_V1_Prs3Vdp1CaptureBindingReceipt *binding,
    uint64_t trace_fnv1a64, uint32_t trace_size,
    int original_saturn_capture_authenticated,
    Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *out_receipt);

const char *nexus_v1_prs3_vdp1_consumer_evidence_status_name(
    Nexus_V1_Prs3Vdp1ConsumerEvidenceStatus status);

#endif
