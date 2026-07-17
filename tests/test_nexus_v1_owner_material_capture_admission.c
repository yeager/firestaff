#include "nexus_v1_launcher.h"
#include "nexus_v1_owner_material_capture_artifact_preflight.h"

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
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt route;
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt resumed;
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt trace;
    uint8_t capture[NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES + 4U];
    uint8_t raw_trace[128U];

    build_target(&target);
    memset(raw_trace, 0x5a, sizeof(raw_trace));
    build_capture(capture, &target);
    be64(capture, 144U, fnv1a64(raw_trace, sizeof(raw_trace)));
    if (!nexus_v1_owner_material_capture_admit(&target, capture, sizeof(capture),
                                                &receipt) || !receipt.valid ||
        !receipt.target_bound || !receipt.owner_bound || !receipt.face_bound ||
        !receipt.descriptor_bound || !receipt.candidates_bound ||
        !receipt.payload_opaque || !receipt.no_draw_only ||
        receipt.owner_mapping_proven || receipt.mesh_semantics_permitted ||
        receipt.texture_semantics_permitted || receipt.decoder_permitted) return 1;
    {
        Nexus_V1_OwnerMaterialCaptureCampaignReceipt campaign;
        Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput preflight_input;
        Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt preflight;

        memset(&campaign, 0, sizeof(campaign));
        campaign.valid = 1;
        campaign.witness_count = 2U;
        campaign.imported_original_saturn_captures_bound = 1;
        campaign.opaque_evidence_only = 1;
        campaign.no_draw_only = 1;
        campaign.blocks_real_dgn_mesh_render = 1;
        campaign.witnesses[0].valid = 1;
        campaign.witnesses[0].opaque_original_capture_covered = 1;
        campaign.witnesses[0].level_index = (uint32_t)target.level_index;
        campaign.witnesses[0].source_fnv1a64 =
            target.material_target.source_bytes_fnv1a64;
        campaign.witnesses[0].descriptor_fnv1a64 =
            target.material_target.descriptor_target.descriptor_bytes_fnv1a64;
        campaign.witnesses[0].face_row_fnv1a64 =
            target.owner_face_target.face_target.candidate.face_row_fnv1a32;
        campaign.witnesses[0].capture_fnv1a64 = receipt.capture_fnv1a64;
        campaign.witnesses[0].raw_trace_fnv1a64 = receipt.raw_trace_fnv1a64;
        campaign.witnesses[0].raw_trace_byte_count = receipt.raw_trace_byte_count;
        memset(&trace, 0, sizeof(trace));
        trace.status = NEXUS_V1_OWNER_MATERIAL_TRACE_ADMITTED_OPAQUE;
        trace.level_index = target.level_index;
        trace.descriptor_index = target.material_target.descriptor_target.descriptor_index;
        trace.atomic_target_bound = trace.owner_face_bound = 1;
        trace.structure2_trace_admitted = trace.original_saturn_capture_verified = 1;
        trace.opaque_trace_admitted = trace.no_draw_only = 1;
        trace.blocks_real_dgn_mesh_render = 1;
        memset(&preflight_input, 0, sizeof(preflight_input));
        preflight_input.campaign = &campaign;
        preflight_input.target = &target;
        preflight_input.capture_bytes = capture;
        preflight_input.capture_byte_count = sizeof(capture);
        preflight_input.raw_trace = raw_trace;
        preflight_input.raw_trace_byte_count = sizeof(raw_trace);
        preflight_input.trace = &trace;
        if (!nexus_v1_owner_material_capture_artifact_preflight(
                &preflight_input, &preflight) || !preflight.valid ||
            !preflight.campaign_row_bound || !preflight.artifact_bound ||
            !preflight.raw_trace_bound || !preflight.engine_trace_bound ||
            !preflight.no_draw_only || preflight.decoder_permitted) return 1;
        raw_trace[0] ^= 1U;
        if (nexus_v1_owner_material_capture_artifact_preflight(
                &preflight_input, &preflight) || preflight.valid ||
            !preflight.no_draw_only) return 1;
        raw_trace[0] ^= 1U;
    }
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
    build_capture(capture, &target);
    if (!nexus_v1_owner_material_capture_admit(&target, capture, sizeof(capture),
                                                &receipt) || !receipt.valid) return 1;
    memset(&route, 0, sizeof(route));
    route.valid = 1;
    route.capture_required = 1;
    route.operator_only = 1;
    route.no_draw_only = 1;
    route.blocks_real_dgn_mesh_render = 1;
    route.bios_region = NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_US;
    strcpy(route.bios_sha256,
           "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f");
    strcpy(route.disc_sha256,
           "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
    route.target = target;
    if (nexus_v1_launcher_resume_m12_m11_owner_material_capture(
            NULL, &route, capture, sizeof(capture), &resumed) || resumed.valid ||
        !resumed.capture_required || !resumed.no_draw_only ||
        !resumed.blocks_real_dgn_mesh_render) return 1;
    route.owner_mapping_proven = 0;
    if (nexus_v1_launcher_import_m12_m11_owner_material_capture(
            NULL, &route, capture, sizeof(capture), "target=opaque\n", 14U,
            capture + NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES, 4U, 0,
            &trace, &resumed) || resumed.valid || !resumed.capture_required ||
        !resumed.no_draw_only || !trace.no_draw_only ||
        !trace.blocks_real_dgn_mesh_render) return 1;
    {
        Nexus_V1_OwnerMaterialCaptureAdmissionReceipt second = receipt;
        Nexus_V1_OwnerMaterialCaptureAdjudicationReceipt adjudication;
        second.capture_fnv1a64 ^= UINT64_C(1);
        second.raw_trace_fnv1a64 ^= UINT64_C(1);
        if (!nexus_v1_owner_material_capture_adjudicate(
                &receipt, &second, &adjudication) || !adjudication.valid ||
            !adjudication.independent_captures_bound ||
            !adjudication.opaque_evidence_only || !adjudication.no_draw_only ||
            adjudication.owner_mapping_proven ||
            adjudication.mesh_semantics_permitted ||
            adjudication.texture_semantics_permitted ||
            adjudication.decoder_permitted) return 1;
        second.raw_trace_fnv1a64 = receipt.raw_trace_fnv1a64;
        if (nexus_v1_owner_material_capture_adjudicate(
                &receipt, &second, &adjudication) || adjudication.valid ||
            !adjudication.no_draw_only) return 1;
    }
    route.owner_mapping_proven = 1;
    if (nexus_v1_launcher_resume_m12_m11_owner_material_capture(
            NULL, &route, capture, sizeof(capture), &resumed) || resumed.valid ||
        !resumed.capture_required || !resumed.no_draw_only ||
        !resumed.blocks_real_dgn_mesh_render) return 1;
    puts("owner material capture admission: PASS");
    return 0;
}
