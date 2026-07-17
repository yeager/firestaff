#include "nexus_v1_owner_material_capture_artifact_preflight.h"

#include <string.h>

static uint64_t preflight_fnv1a64(const uint8_t *bytes, size_t byte_count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int nexus_v1_owner_material_capture_artifact_preflight(
    const Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput *input,
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt *out_receipt)
{
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt receipt;
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt capture;
    const Nexus_V1_OwnerMaterialCaptureCampaignCoverage *coverage;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    memset(&capture, 0, sizeof(capture));
    if (!input || !input->campaign || !input->campaign->valid ||
        !input->campaign->opaque_evidence_only || !input->campaign->no_draw_only ||
        input->campaign->fallback_visuals_permitted ||
        !input->campaign->blocks_real_dgn_mesh_render ||
        input->witness_index >= input->campaign->witness_count || !input->target ||
        !input->raw_trace || input->raw_trace_byte_count == 0U || !input->trace ||
        !nexus_v1_owner_material_capture_admit(input->target, input->capture_bytes,
                                               input->capture_byte_count, &capture)) {
        *out_receipt = receipt;
        return 0;
    }
    coverage = &input->campaign->witnesses[input->witness_index];
    descriptor = &input->target->material_target.descriptor_target;
    receipt.artifact_bound = capture.valid && capture.payload_opaque &&
        capture.no_draw_only && !capture.fallback_visuals_permitted &&
        capture.blocks_real_dgn_mesh_render;
    receipt.raw_trace_bound = input->raw_trace_byte_count == capture.raw_trace_byte_count &&
        preflight_fnv1a64(input->raw_trace, input->raw_trace_byte_count) ==
            capture.raw_trace_fnv1a64;
    receipt.campaign_row_bound = coverage->valid &&
        coverage->level_index == (uint32_t)input->target->level_index &&
        coverage->source_fnv1a64 == input->target->material_target.source_bytes_fnv1a64 &&
        coverage->descriptor_fnv1a64 == descriptor->descriptor_bytes_fnv1a64 &&
        coverage->face_row_fnv1a64 == (uint64_t)input->target->owner_face_target
            .face_target.candidate.face_row_fnv1a32 &&
        coverage->capture_fnv1a64 == capture.capture_fnv1a64 &&
        coverage->raw_trace_fnv1a64 == capture.raw_trace_fnv1a64 &&
        coverage->raw_trace_byte_count == capture.raw_trace_byte_count;
    receipt.engine_trace_bound = input->trace->status ==
            NEXUS_V1_OWNER_MATERIAL_TRACE_ADMITTED_OPAQUE &&
        input->trace->level_index == input->target->level_index &&
        input->trace->descriptor_index == descriptor->descriptor_index &&
        input->trace->atomic_target_bound && input->trace->owner_face_bound &&
        input->trace->structure2_trace_admitted &&
        input->trace->original_saturn_capture_verified &&
        input->trace->opaque_trace_admitted && !input->trace->decoder_permitted &&
        input->trace->no_draw_only && !input->trace->fallback_visuals_permitted &&
        input->trace->blocks_real_dgn_mesh_render;
    if (!receipt.artifact_bound || !receipt.raw_trace_bound ||
        !receipt.campaign_row_bound || !receipt.engine_trace_bound) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.witness_index = input->witness_index;
    receipt.capture_fnv1a64 = capture.capture_fnv1a64;
    receipt.raw_trace_fnv1a64 = capture.raw_trace_fnv1a64;
    receipt.raw_trace_byte_count = capture.raw_trace_byte_count;
    receipt.opaque_evidence_only = 1;
    *out_receipt = receipt;
    return 1;
}
