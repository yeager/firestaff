#include "nexus_v1_launcher.h"

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
        bytes[offset + (size_t)(7 - index)] =
            (uint8_t)(value >> (index * 8));
    }
}

static void build_artifact(
    uint8_t *bytes, size_t byte_count,
    const Nexus_V1_OwnerMaterialCaptureCampaignReceipt *campaign)
{
    size_t index;

    memset(bytes, 0, byte_count);
    memcpy(bytes, NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_MAGIC, 8U);
    be32(bytes, 8U, NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_VERSION);
    be32(bytes, 12U,
         NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES);
    be32(bytes, 16U, campaign->witness_count);
    be32(bytes, 20U,
         NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES);
    for (index = 0U; index < campaign->witness_count; ++index) {
        const Nexus_V1_OwnerMaterialCaptureCampaignCoverage *coverage =
            &campaign->witnesses[index];
        size_t offset = NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES +
            index * NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES;

        be64(bytes, offset, coverage->level_index);
        be64(bytes, offset + 8U, coverage->source_fnv1a64);
        be64(bytes, offset + 16U, coverage->descriptor_fnv1a64);
        be64(bytes, offset + 24U, coverage->face_row_fnv1a64);
        be64(bytes, offset + 32U, coverage->capture_fnv1a64);
        be64(bytes, offset + 40U, coverage->raw_trace_fnv1a64);
        be64(bytes, offset + 48U, coverage->raw_trace_byte_count);
    }
    be64(bytes, 24U, fnv1a64(
        bytes + NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES,
        byte_count - NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES));
}

static void target_fixture(
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target,
    int level_index, uint64_t seed)
{
    memset(target, 0, sizeof(*target));
    target->valid = 1;
    target->level_index = level_index;
    target->owner_face_source_bound = 1;
    target->static_material_source_bound = 1;
    target->capture_producer_required = 1;
    target->original_saturn_capture_required = 1;
    target->no_draw_only = 1;
    target->blocks_real_dgn_mesh_render = 1;
    target->material_target.source_bytes_fnv1a64 = seed + UINT64_C(0x1000);
    target->material_target.source_byte_count = 4096;
    target->owner_face_target.owner_x = level_index + 1;
    target->owner_face_target.owner_y = level_index + 2;
    target->owner_face_target.structure1f_entry_index = level_index + 3;
    target->owner_face_target.structure1a_index = (uint32_t)(level_index + 4);
    target->owner_face_target.face_target.candidate.entry_index =
        (uint32_t)(level_index + 5);
    target->owner_face_target.face_target.candidate.face_ordinal =
        (uint32_t)(level_index + 6);
    target->owner_face_target.face_target.candidate.face_row_fnv1a32 =
        (uint32_t)(seed + 7U);
    target->material_target.descriptor_target.descriptor_index = level_index + 8;
    target->material_target.descriptor_target.descriptor_bytes_fnv1a64 =
        seed + UINT64_C(0x2000);
    target->material_target.descriptor_target.image_payload_candidate_fnv1a64 =
        seed + UINT64_C(0x3000);
    target->material_target.descriptor_target.palette_payload_candidate_fnv1a64 =
        seed + UINT64_C(0x4000);
}

static void receipt_fixture(
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target,
    uint64_t seed, Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *capture,
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *trace)
{
    memset(capture, 0, sizeof(*capture));
    capture->valid = 1;
    capture->target_bound = 1;
    capture->owner_bound = 1;
    capture->face_bound = 1;
    capture->descriptor_bound = 1;
    capture->candidates_bound = 1;
    capture->payload_bounds_bound = 1;
    capture->payload_hash_bound = 1;
    capture->trace_witness_bound = 1;
    capture->payload_opaque = 1;
    capture->original_saturn_capture_verified = 1;
    capture->no_draw_only = 1;
    capture->blocks_real_dgn_mesh_render = 1;
    capture->target_source_fnv1a64 = target->material_target.source_bytes_fnv1a64;
    capture->target_descriptor_fnv1a64 =
        target->material_target.descriptor_target.descriptor_bytes_fnv1a64;
    capture->target_face_row_fnv1a64 =
        target->owner_face_target.face_target.candidate.face_row_fnv1a32;
    capture->capture_fnv1a64 = seed + UINT64_C(0x5000);
    capture->raw_trace_fnv1a64 = seed + UINT64_C(0x6000);
    capture->raw_trace_byte_count = 128U;
    memset(trace, 0, sizeof(*trace));
    trace->status = NEXUS_V1_OWNER_MATERIAL_TRACE_ADMITTED_OPAQUE;
    trace->level_index = target->level_index;
    trace->descriptor_index =
        target->material_target.descriptor_target.descriptor_index;
    trace->atomic_target_bound = 1;
    trace->owner_face_bound = 1;
    trace->structure2_trace_admitted = 1;
    trace->original_saturn_capture_verified = 1;
    trace->opaque_trace_admitted = 1;
    trace->no_draw_only = 1;
    trace->blocks_real_dgn_mesh_render = 1;
}

int main(void)
{
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget targets[2];
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt captures[2];
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt traces[2];
    Nexus_V1_OwnerMaterialCaptureCampaignWitness witnesses[2];
    Nexus_V1_OwnerMaterialCaptureCampaignInput input;
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt receipt;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt route;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt resumed;
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt witness_route;
    Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt multi_route;
    Nexus_V1_LauncherM11MultiWitnessDungeonCaptureStartReceipt start;
    Nexus_V1_Engine engine;
    uint32_t selected_indices[2] = { 0U, 1U };
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt artifact_receipt;
    uint8_t artifact[NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES +
        2U * NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES];
    const char *bios_sha256 =
        "96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f";
    const char *disc_sha256 =
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";

    target_fixture(&targets[0], 2, UINT64_C(0x100000));
    target_fixture(&targets[1], 7, UINT64_C(0x200000));
    receipt_fixture(&targets[0], UINT64_C(0x100000), &captures[0], &traces[0]);
    receipt_fixture(&targets[1], UINT64_C(0x200000), &captures[1], &traces[1]);
    witnesses[0].target = &targets[0];
    witnesses[0].capture = &captures[0];
    witnesses[0].trace = &traces[0];
    witnesses[1].target = &targets[1];
    witnesses[1].capture = &captures[1];
    witnesses[1].trace = &traces[1];
    input.witnesses = witnesses;
    input.witness_count = 2U;
    if (!nexus_v1_owner_material_capture_campaign_admit(&input, &receipt) ||
        !receipt.valid || receipt.witness_count != 2U ||
        !receipt.imported_original_saturn_captures_bound ||
        !receipt.opaque_evidence_only || !receipt.no_draw_only ||
        receipt.owner_mapping_proven || receipt.mesh_semantics_permitted ||
        receipt.texture_semantics_permitted || receipt.decoder_permitted ||
        receipt.witnesses[0].level_index != 2U ||
        receipt.witnesses[1].level_index != 7U) return 1;
    if (!nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
            2U, bios_sha256, NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_US,
            disc_sha256, &route) || !route.valid || !route.capture_required ||
        !route.no_draw_only ||
        !nexus_v1_launcher_admit_m12_owner_material_capture_witness_required(
            &route, 1U, &witness_route) || !witness_route.valid ||
        !witness_route.capture_required || !witness_route.no_draw_only ||
        witness_route.decoder_permitted ||
        nexus_v1_launcher_admit_m12_owner_material_capture_witness_required(
            &route, 2U, &witness_route) || witness_route.valid ||
        !witness_route.capture_required || !witness_route.no_draw_only ||
        !nexus_v1_launcher_admit_m12_owner_material_capture_multi_witness_required(
            &route, selected_indices, 2U, &multi_route) || !multi_route.valid ||
        multi_route.witness_count != 2U || !multi_route.capture_required ||
        !multi_route.no_draw_only || multi_route.decoder_permitted ||
        multi_route.witness_indices[0] != 0U || multi_route.witness_indices[1] != 1U ||
        !nexus_v1_launcher_import_m12_owner_material_capture_campaign(
            &route, &input, &resumed) || !resumed.valid ||
        !resumed.captures_imported || !resumed.resume_ready ||
        resumed.capture_required || !resumed.campaign.valid ||
        !resumed.no_draw_only || resumed.decoder_permitted) return 1;
    memset(&engine, 0, sizeof(engine));
    engine.game.current_level = 2;
    if (!nexus_v1_launcher_admit_m11_multi_witness_dungeon_capture_start(
            &engine, &multi_route, &input, &start) || !start.valid ||
        !start.selected_witness || !start.capture_required || !start.no_draw_only ||
        start.witness_index != 0U || !start.capture_target_bound ||
        start.capture_target.material_target.source_bytes_fnv1a64 !=
            targets[0].material_target.source_bytes_fnv1a64 ||
        start.decoder_permitted) return 1;
    targets[0].material_target.descriptor_target.descriptor_bytes_fnv1a64 ^= UINT64_C(1);
    if (nexus_v1_launcher_admit_m11_multi_witness_dungeon_capture_start(
            &engine, &multi_route, &input, &start) || start.valid ||
        start.capture_target_bound || !start.no_draw_only) return 1;
    targets[0].material_target.descriptor_target.descriptor_bytes_fnv1a64 ^= UINT64_C(1);
    engine.game.current_level = 5;
    if (nexus_v1_launcher_admit_m11_multi_witness_dungeon_capture_start(
            &engine, &multi_route, &input, &start) || start.valid ||
        start.selected_witness || start.capture_required || !start.no_draw_only) return 1;
    selected_indices[1] = 0U;
    if (nexus_v1_launcher_admit_m12_owner_material_capture_multi_witness_required(
            &route, selected_indices, 2U, &multi_route) || multi_route.valid ||
        !multi_route.capture_required || !multi_route.no_draw_only) return 1;
    selected_indices[1] = 2U;
    if (nexus_v1_launcher_admit_m12_owner_material_capture_multi_witness_required(
            &route, selected_indices, 2U, &multi_route) || multi_route.valid ||
        !multi_route.capture_required || !multi_route.no_draw_only) return 1;
    selected_indices[1] = 1U;
    build_artifact(artifact, sizeof(artifact), &receipt);
    if (!nexus_v1_launcher_import_m12_owner_material_capture_campaign_artifact(
            &route, artifact, sizeof(artifact), &input, &artifact_receipt,
            &resumed) || !artifact_receipt.valid ||
        !artifact_receipt.campaign_bound || !artifact_receipt.rows_bounds_bound ||
        !artifact_receipt.rows_hash_bound || !artifact_receipt.no_draw_only ||
        artifact_receipt.decoder_permitted || !resumed.resume_ready ||
        !resumed.no_draw_only) return 1;
    artifact[NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES + 8U] ^= 1U;
    if (nexus_v1_launcher_import_m12_owner_material_capture_campaign_artifact(
            &route, artifact, sizeof(artifact), &input, &artifact_receipt,
            &resumed) || artifact_receipt.valid || !resumed.valid ||
        !resumed.capture_required || !resumed.no_draw_only) return 1;
    route.expected_witness_count = 3U;
    if (nexus_v1_launcher_import_m12_owner_material_capture_campaign(
            &route, &input, &resumed) || resumed.valid ||
        !resumed.capture_required || !resumed.no_draw_only) return 1;
    if (nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
            1U, bios_sha256, NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_US,
            disc_sha256, &route) || route.valid || !route.capture_required ||
        !route.no_draw_only) return 1;
    targets[1].level_index = targets[0].level_index;
    if (nexus_v1_owner_material_capture_campaign_admit(&input, &receipt) ||
        receipt.valid || !receipt.no_draw_only) return 1;
    target_fixture(&targets[1], 7, UINT64_C(0x200000));
    captures[1].target_descriptor_fnv1a64 ^= UINT64_C(1);
    if (nexus_v1_owner_material_capture_campaign_admit(&input, &receipt) ||
        receipt.valid || !receipt.no_draw_only) return 1;
    receipt_fixture(&targets[1], UINT64_C(0x200000), &captures[1], &traces[1]);
    captures[1].raw_trace_fnv1a64 = captures[0].raw_trace_fnv1a64;
    if (nexus_v1_owner_material_capture_campaign_admit(&input, &receipt) ||
        receipt.valid || !receipt.no_draw_only) return 1;
    puts("owner material capture campaign: PASS");
    return 0;
}
