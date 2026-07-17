#include "nexus_v1_owner_material_capture_campaign.h"

#include <string.h>

static int campaign_witness_valid(
    const Nexus_V1_OwnerMaterialCaptureCampaignWitness *witness,
    Nexus_V1_OwnerMaterialCaptureCampaignCoverage *coverage)
{
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target;
    const Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *capture;
    const Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *trace;
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *owner;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;

    if (!witness || !coverage || !witness->target || !witness->capture ||
        !witness->trace) return 0;
    target = witness->target;
    capture = witness->capture;
    trace = witness->trace;
    owner = &target->owner_face_target;
    descriptor = &target->material_target.descriptor_target;
    if (!target->valid || target->level_index < 0 ||
        target->level_index >= (int)NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES ||
        !target->owner_face_source_bound || !target->static_material_source_bound ||
        target->owner_to_entry_mapping_proven || !target->no_draw_only ||
        target->fallback_visuals_permitted ||
        !target->blocks_real_dgn_mesh_render ||
        !target->material_target.source_bytes_fnv1a64 ||
        target->material_target.source_byte_count <= 0 || !capture->valid ||
        !capture->target_bound || !capture->owner_bound || !capture->face_bound ||
        !capture->descriptor_bound || !capture->candidates_bound ||
        !capture->payload_bounds_bound || !capture->payload_hash_bound ||
        !capture->trace_witness_bound || !capture->payload_opaque ||
        !capture->original_saturn_capture_verified || !capture->no_draw_only ||
        capture->fallback_visuals_permitted ||
        !capture->blocks_real_dgn_mesh_render ||
        capture->target_source_fnv1a64 != target->material_target.source_bytes_fnv1a64 ||
        capture->target_descriptor_fnv1a64 != descriptor->descriptor_bytes_fnv1a64 ||
        capture->target_face_row_fnv1a64 !=
            (uint64_t)owner->face_target.candidate.face_row_fnv1a32 ||
        !capture->capture_fnv1a64 || !capture->raw_trace_fnv1a64 ||
        !capture->raw_trace_byte_count ||
        trace->status != NEXUS_V1_OWNER_MATERIAL_TRACE_ADMITTED_OPAQUE ||
        trace->level_index != target->level_index ||
        trace->descriptor_index != descriptor->descriptor_index ||
        !trace->atomic_target_bound || !trace->owner_face_bound ||
        !trace->structure2_trace_admitted ||
        !trace->original_saturn_capture_verified || !trace->opaque_trace_admitted ||
        trace->decoder_permitted || !trace->no_draw_only ||
        trace->fallback_visuals_permitted ||
        !trace->blocks_real_dgn_mesh_render) return 0;
    memset(coverage, 0, sizeof(*coverage));
    coverage->valid = 1;
    coverage->level_index = (uint32_t)target->level_index;
    coverage->source_fnv1a64 = target->material_target.source_bytes_fnv1a64;
    coverage->descriptor_fnv1a64 = descriptor->descriptor_bytes_fnv1a64;
    coverage->face_row_fnv1a64 =
        (uint64_t)owner->face_target.candidate.face_row_fnv1a32;
    coverage->capture_fnv1a64 = capture->capture_fnv1a64;
    coverage->raw_trace_fnv1a64 = capture->raw_trace_fnv1a64;
    coverage->raw_trace_byte_count = capture->raw_trace_byte_count;
    coverage->opaque_original_capture_covered = 1;
    return 1;
}

int nexus_v1_owner_material_capture_campaign_admit(
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *input,
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt *out_receipt)
{
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt receipt;
    size_t index;
    size_t prior;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!input || !input->witnesses ||
        input->witness_count < NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MIN_WITNESSES ||
        input->witness_count > NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES) {
        *out_receipt = receipt;
        return 0;
    }
    for (index = 0U; index < input->witness_count; ++index) {
        Nexus_V1_OwnerMaterialCaptureCampaignCoverage *coverage =
            &receipt.witnesses[index];

        if (!campaign_witness_valid(&input->witnesses[index], coverage)) {
            *out_receipt = receipt;
            return 0;
        }
        for (prior = 0U; prior < index; ++prior) {
            const Nexus_V1_OwnerMaterialCaptureCampaignCoverage *previous =
                &receipt.witnesses[prior];

            if (previous->level_index == coverage->level_index ||
                previous->source_fnv1a64 == coverage->source_fnv1a64 ||
                previous->capture_fnv1a64 == coverage->capture_fnv1a64 ||
                previous->raw_trace_fnv1a64 == coverage->raw_trace_fnv1a64) {
                *out_receipt = receipt;
                return 0;
            }
        }
    }
    receipt.valid = 1;
    receipt.witness_count = (uint32_t)input->witness_count;
    receipt.imported_original_saturn_captures_bound = 1;
    receipt.opaque_evidence_only = 1;
    *out_receipt = receipt;
    return 1;
}
