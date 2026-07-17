#include "nexus_v1_owner_material_capture_admission.h"

#include <string.h>

static uint32_t capture_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24U) | ((uint32_t)bytes[1] << 16U) |
        ((uint32_t)bytes[2] << 8U) | bytes[3];
}

static uint64_t capture_be64(const uint8_t *bytes)
{
    uint64_t value = 0U;
    size_t index;

    for (index = 0U; index < 8U; ++index) value = (value << 8U) | bytes[index];
    return value;
}

static uint64_t capture_fnv1a64(const uint8_t *bytes, size_t byte_count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int nexus_v1_owner_material_capture_admit(
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *out_receipt)
{
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt receipt;
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *owner;
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;
    uint32_t payload_offset;
    uint32_t payload_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!target || !target->valid || !target->owner_face_source_bound ||
        !target->static_material_source_bound ||
        target->owner_to_entry_mapping_proven ||
        !target->capture_producer_required ||
        !target->original_saturn_capture_required || !target->no_draw_only ||
        target->fallback_visuals_permitted ||
        !target->blocks_real_dgn_mesh_render || !capture_bytes ||
        capture_byte_count < NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES ||
        memcmp(capture_bytes, NEXUS_V1_OWNER_MATERIAL_CAPTURE_MAGIC, 8U) != 0 ||
        capture_be32(capture_bytes + 8U) != NEXUS_V1_OWNER_MATERIAL_CAPTURE_VERSION ||
        capture_be32(capture_bytes + 12U) != NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    owner = &target->owner_face_target;
    descriptor = &target->material_target.descriptor_target;
    receipt.target_bound = capture_be64(capture_bytes + 16U) ==
            (uint64_t)(uint32_t)target->level_index &&
        capture_be64(capture_bytes + 24U) == target->material_target.source_bytes_fnv1a64 &&
        capture_be64(capture_bytes + 32U) ==
            (uint64_t)(uint32_t)target->material_target.source_byte_count;
    receipt.owner_bound = capture_be64(capture_bytes + 40U) ==
            (uint64_t)(uint32_t)owner->owner_x &&
        capture_be64(capture_bytes + 48U) == (uint64_t)(uint32_t)owner->owner_y &&
        capture_be64(capture_bytes + 56U) ==
            (uint64_t)(uint32_t)owner->structure1f_entry_index &&
        capture_be64(capture_bytes + 64U) == owner->structure1a_index;
    receipt.face_bound = capture_be64(capture_bytes + 72U) ==
            owner->face_target.candidate.entry_index &&
        capture_be64(capture_bytes + 80U) == owner->face_target.candidate.face_ordinal &&
        capture_be64(capture_bytes + 88U) ==
            owner->face_target.candidate.face_row_fnv1a32;
    receipt.descriptor_bound = capture_be64(capture_bytes + 96U) ==
            (uint64_t)(uint32_t)descriptor->descriptor_index &&
        capture_be64(capture_bytes + 104U) == descriptor->descriptor_bytes_fnv1a64;
    receipt.candidates_bound = capture_be64(capture_bytes + 112U) ==
            descriptor->image_payload_candidate_fnv1a64 &&
        capture_be64(capture_bytes + 120U) ==
            descriptor->palette_payload_candidate_fnv1a64;
    payload_offset = capture_be32(capture_bytes + 128U);
    payload_length = capture_be32(capture_bytes + 132U);
    receipt.payload_bounds_bound =
        payload_offset == NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES &&
        payload_length > 0U && payload_offset <= capture_byte_count &&
        payload_length <= capture_byte_count - payload_offset &&
        payload_offset + (size_t)payload_length == capture_byte_count;
    receipt.payload_hash_bound = receipt.payload_bounds_bound &&
        capture_be64(capture_bytes + 136U) ==
            capture_fnv1a64(capture_bytes + payload_offset, payload_length);
    receipt.raw_trace_fnv1a64 = capture_be64(capture_bytes + 144U);
    receipt.raw_trace_byte_count = capture_be64(capture_bytes + 152U);
    receipt.trace_witness_bound = receipt.raw_trace_fnv1a64 != 0U &&
        receipt.raw_trace_byte_count != 0U;
    if (!receipt.target_bound || !receipt.owner_bound || !receipt.face_bound ||
        !receipt.descriptor_bound || !receipt.candidates_bound ||
        !receipt.payload_hash_bound || !receipt.trace_witness_bound) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.capture_fnv1a64 = capture_fnv1a64(capture_bytes, capture_byte_count);
    receipt.capture_byte_count = capture_byte_count;
    receipt.target_source_fnv1a64 = target->material_target.source_bytes_fnv1a64;
    receipt.target_descriptor_fnv1a64 = descriptor->descriptor_bytes_fnv1a64;
    receipt.target_face_row_fnv1a64 =
        owner->face_target.candidate.face_row_fnv1a32;
    receipt.payload_offset = payload_offset;
    receipt.payload_length = payload_length;
    receipt.payload_fnv1a64 = capture_be64(capture_bytes + 136U);
    receipt.payload_opaque = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_owner_material_capture_adjudicate(
    const Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *first,
    const Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *second,
    Nexus_V1_OwnerMaterialCaptureAdjudicationReceipt *out_receipt)
{
    Nexus_V1_OwnerMaterialCaptureAdjudicationReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!first || !second || !first->valid || !second->valid ||
        !first->payload_opaque || !second->payload_opaque ||
        !first->no_draw_only || !second->no_draw_only ||
        !first->blocks_real_dgn_mesh_render ||
        !second->blocks_real_dgn_mesh_render ||
        !first->target_source_fnv1a64 || !first->target_descriptor_fnv1a64 ||
        !first->target_face_row_fnv1a64 ||
        first->target_source_fnv1a64 != second->target_source_fnv1a64 ||
        first->target_descriptor_fnv1a64 != second->target_descriptor_fnv1a64 ||
        first->target_face_row_fnv1a64 != second->target_face_row_fnv1a64 ||
        first->capture_fnv1a64 == second->capture_fnv1a64 ||
        first->raw_trace_fnv1a64 == second->raw_trace_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.target_bound = 1;
    receipt.independent_captures_bound = 1;
    receipt.opaque_evidence_only = 1;
    *out_receipt = receipt;
    return 1;
}
