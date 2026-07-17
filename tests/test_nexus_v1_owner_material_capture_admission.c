#include "nexus_v1_owner_material_capture_admission.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void be32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)(value >> 24U);
    bytes[offset + 1U] = (uint8_t)(value >> 16U);
    bytes[offset + 2U] = (uint8_t)(value >> 8U);
    bytes[offset + 3U] = (uint8_t)value;
}

static void be64(uint8_t *bytes, size_t offset, uint64_t value)
{
    int index;
    for (index = 7; index >= 0; --index) {
        bytes[offset + (size_t)(7 - index)] = (uint8_t)(value >> (index * 8));
    }
}

static void build_target(Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target)
{
    memset(target, 0, sizeof(*target));
    target->valid = 1;
    target->level_index = 4;
    target->owner_face_source_bound = 1;
    target->static_material_source_bound = 1;
    target->capture_producer_required = 1;
    target->original_saturn_capture_required = 1;
    target->no_draw_only = 1;
    target->blocks_real_dgn_mesh_render = 1;
    target->material_target.source_bytes_fnv1a64 = UINT64_C(0x1020304050607080);
    target->material_target.source_byte_count = 4096;
    target->owner_face_target.owner_x = 12;
    target->owner_face_target.owner_y = 34;
    target->owner_face_target.structure1f_entry_index = 56;
    target->owner_face_target.structure1a_index = 78;
    target->owner_face_target.face_target.candidate.entry_index = 90;
    target->owner_face_target.face_target.candidate.face_ordinal = 3;
    target->owner_face_target.face_target.candidate.face_row_fnv1a32 = 0x11223344U;
    target->material_target.descriptor_target.descriptor_index = 5;
    target->material_target.descriptor_target.descriptor_bytes_fnv1a64 =
        UINT64_C(0x2233445566778899);
    target->material_target.descriptor_target.image_payload_candidate_fnv1a64 =
        UINT64_C(0x33445566778899aa);
    target->material_target.descriptor_target.palette_payload_candidate_fnv1a64 =
        UINT64_C(0x445566778899aabb);
}

static void build_capture(uint8_t *capture,
                          const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target)
{
    const uint8_t payload[] = { 0x31U, 0x47U, 0x9aU, 0xc2U };
    memset(capture, 0, NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES + sizeof(payload));
    memcpy(capture, NEXUS_V1_OWNER_MATERIAL_CAPTURE_MAGIC, 8U);
    be32(capture, 8U, NEXUS_V1_OWNER_MATERIAL_CAPTURE_VERSION);
    be32(capture, 12U, NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES);
    be64(capture, 16U, (uint64_t)target->level_index);
    be64(capture, 24U, target->material_target.source_bytes_fnv1a64);
    be64(capture, 32U, (uint64_t)target->material_target.source_byte_count);
    be64(capture, 40U, (uint64_t)target->owner_face_target.owner_x);
    be64(capture, 48U, (uint64_t)target->owner_face_target.owner_y);
    be64(capture, 56U, (uint64_t)target->owner_face_target.structure1f_entry_index);
    be64(capture, 64U, target->owner_face_target.structure1a_index);
    be64(capture, 72U, target->owner_face_target.face_target.candidate.entry_index);
    be64(capture, 80U, target->owner_face_target.face_target.candidate.face_ordinal);
    be64(capture, 88U, target->owner_face_target.face_target.candidate.face_row_fnv1a32);
    be64(capture, 96U, target->material_target.descriptor_target.descriptor_index);
    be64(capture, 104U, target->material_target.descriptor_target.descriptor_bytes_fnv1a64);
    be64(capture, 112U, target->material_target.descriptor_target.image_payload_candidate_fnv1a64);
    be64(capture, 120U, target->material_target.descriptor_target.palette_payload_candidate_fnv1a64);
    be32(capture, 128U, NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES);
    be32(capture, 132U, sizeof(payload));
    memcpy(capture + NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES, payload, sizeof(payload));
    be64(capture, 136U, fnv1a64(payload, sizeof(payload)));
    be64(capture, 144U, UINT64_C(0x5566778899aabbcc));
    be64(capture, 152U, 128U);
}

int main(void)
{
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget target;
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt receipt;
    uint8_t capture[NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES + 4U];

    build_target(&target);
    build_capture(capture, &target);
    if (!nexus_v1_owner_material_capture_admit(&target, capture, sizeof(capture),
                                                &receipt) || !receipt.valid ||
        !receipt.target_bound || !receipt.owner_bound || !receipt.face_bound ||
        !receipt.descriptor_bound || !receipt.candidates_bound ||
        !receipt.payload_opaque || !receipt.no_draw_only ||
        receipt.owner_mapping_proven || receipt.mesh_semantics_permitted ||
        receipt.texture_semantics_permitted || receipt.decoder_permitted) return 1;
    capture[56U] ^= 1U;
    if (nexus_v1_owner_material_capture_admit(&target, capture, sizeof(capture), &receipt) ||
        receipt.valid || !receipt.no_draw_only) return 1;
    capture[56U] ^= 1U;
    capture[112U] ^= 1U;
    if (nexus_v1_owner_material_capture_admit(&target, capture, sizeof(capture), &receipt) ||
        receipt.valid || !receipt.no_draw_only) return 1;
    capture[112U] ^= 1U;
    capture[NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES] ^= 1U;
    if (nexus_v1_owner_material_capture_admit(&target, capture, sizeof(capture), &receipt) ||
        receipt.valid || !receipt.no_draw_only) return 1;
    puts("owner material capture admission: PASS");
    return 0;
}
