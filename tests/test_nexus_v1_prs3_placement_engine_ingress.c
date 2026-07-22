#include "nexus_v1_engine.h"
#include "nexus_v1_prs3_dgn_placement_adapter.h"
#include "nexus_v1_structure1f_placement_binding.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_Engine engine;
    Nexus_V1_Prs3DgnPlacementAdapterReceipt receipt;
    Nexus_V1_Structure1FPlacementBindingReceipt structure1f;
    Nexus_V1_ExternalStructure1FPlacementReceipt external_receipt;
    uint32_t invalidations;

    memset(&engine, 0, sizeof(engine));
    memset(&receipt, 0, sizeof(receipt));
    memset(&structure1f, 0, sizeof(structure1f));
    memset(&external_receipt, 0, sizeof(external_receipt));
    engine.level_loaded = 1;
    engine.game.current_level = 3;
    engine.current_level_structure2_source.loaded_dgn_fnv1a64 = 0x1234U;
    receipt.valid = receipt.trace_bound = receipt.dgn_source_bound = 1;
    receipt.descriptor_envelope_bound = receipt.command_coordinates_bound = 1;
    receipt.no_draw_only = receipt.blocks_real_dgn_mesh_render = 1;
    receipt.trace_fnv1a64 = 0x77U;
    receipt.trace_size = 8U;
    receipt.dgn_fnv1a64 = 0x1234U;
    receipt.descriptor_index = 2U;
    receipt.descriptor_fnv1a64 = 0x99U;
    receipt.frame_sequence = 0x44U;
    receipt.command_sequence = 0x45U;
    receipt.dgn_placement_observed = 1;
    receipt.prs3_header_span_fnv1a64 = 0x11U;
    receipt.prs3_bitmap_candidate_fnv1a64 = 0x22U;
    receipt.prs3_bitmap_candidate_offset = 4U;
    receipt.prs3_bitmap_candidate_size = 8U;
    receipt.palt_candidate_fnv1a64 = 0x33U;
    receipt.palt_candidate_size = 512U;
    if (!nexus_v1_engine_set_external_prs3_replay_placement_receipt(
            &engine, 1U, 0x77U, 0x1234U, 0x22U, &receipt) ||
        !engine.external_prs3_placement_valid ||
        engine.external_prs3_placement_route_epoch != 1U ||
        engine.external_prs3_placement_level != 3 ||
        engine.external_prs3_placement_dgn_fnv1a64 != 0x1234U ||
        engine.external_prs3_placement_descriptor_index != 2U ||
        engine.external_prs3_placement_frame_sequence != 0x44U ||
        engine.external_prs3_placement_command_sequence != 0x45U ||
        engine.external_prs3_placement_descriptor_fnv1a64 != 0x99U ||
        engine.external_prs3_placement_trace_fnv1a64 != 0x77U ||
        engine.external_prs3_placement_header_fnv1a64 != 0x11U ||
        engine.external_prs3_placement_bitmap_fnv1a64 != 0x22U ||
        engine.external_prs3_placement_bitmap_offset != 4U ||
        engine.external_prs3_placement_bitmap_size != 8U ||
        engine.external_prs3_placement_palt_fnv1a64 != 0x33U ||
        engine.external_prs3_placement_palt_size != 512U) return 1;
    if (nexus_v1_engine_set_external_prs3_replay_placement_receipt(
            &engine, 1U, 0x77U, 0x1234U, 0x22U, &receipt) ||
        engine.external_prs3_placement_valid ||
        engine.external_prs3_placement_route_epoch) return 1;
    if (!nexus_v1_engine_set_external_prs3_replay_placement_receipt(
            &engine, 2U, 0x77U, 0x1234U, 0x22U, &receipt)) return 1;
    engine.dgn_material_plan.valid = 1;
    invalidations = engine.dgn_material_plan.invalidation_count;
    if (nexus_v1_engine_set_external_prs3_replay_placement_receipt(
            &engine, 3U, 0x78U, 0x1234U, 0x22U, &receipt) ||
        engine.external_prs3_placement_valid ||
        engine.external_prs3_placement_route_epoch ||
        engine.dgn_material_plan.valid ||
        engine.dgn_material_plan.invalidation_count != invalidations + 1U) return 1;
    if (nexus_v1_engine_set_external_prs3_replay_placement_receipt(
            &engine, 2U, 0x77U, 0x1234U, 0x22U, &receipt)) return 1;
    if (!nexus_v1_engine_set_external_prs3_replay_placement_receipt(
            &engine, 4U, 0x77U, 0x1234U, 0x22U, &receipt)) return 1;
    structure1f.valid = structure1f.placement_observed =
        structure1f.no_draw_only = structure1f.blocks_real_dgn_mesh_render = 1;
    structure1f.dgn_fnv1a64 = 0x1234U;
    structure1f.descriptor_index = 2U;
    structure1f.frame_sequence = 0x44U;
    structure1f.command_sequence = 0x45U;
    structure1f.descriptor_index = 3U;
    if (nexus_v1_engine_set_external_structure1f_placement_binding(
            &engine, &structure1f) ||
        engine.external_structure1f_placement_valid) return 1;
    structure1f.descriptor_index = 2U;
    structure1f.descriptor_fnv1a64 = 0x99U;
    engine.dgn_material_plan.valid = 1;
    invalidations = engine.dgn_material_plan.invalidation_count;
    if (!nexus_v1_engine_set_external_structure1f_placement_binding(
            &engine, &structure1f) ||
        !engine.external_structure1f_placement_valid ||
        engine.external_structure1f_placement_descriptor_fnv1a64 != 0x99U ||
        engine.dgn_material_plan.valid ||
        engine.dgn_material_plan.invalidation_count != invalidations + 1U) {
        return 1;
    }
    if (!nexus_v1_current_external_structure1f_placement_receipt(
            &engine, &external_receipt) ||
        !external_receipt.valid || !external_receipt.no_draw_only ||
        !external_receipt.blocks_real_dgn_mesh_render ||
        external_receipt.fallback_visuals_permitted ||
        external_receipt.dgn_fnv1a64 != 0x1234U ||
        external_receipt.descriptor_index != 2U ||
        external_receipt.frame_sequence != 0x44U ||
        external_receipt.command_sequence != 0x45U ||
        external_receipt.descriptor_fnv1a64 != 0x99U) return 1;
    structure1f.command_sequence = 0x46U;
    engine.dgn_material_plan.valid = 1;
    invalidations = engine.dgn_material_plan.invalidation_count;
    if (nexus_v1_engine_set_external_structure1f_placement_binding(
            &engine, &structure1f) ||
        engine.external_structure1f_placement_valid ||
        engine.external_structure1f_placement_dgn_fnv1a64 ||
        engine.external_structure1f_placement_descriptor_index ||
        engine.external_structure1f_placement_frame_sequence ||
        engine.external_structure1f_placement_command_sequence ||
        engine.external_structure1f_placement_descriptor_fnv1a64 ||
        engine.dgn_material_plan.valid ||
        engine.dgn_material_plan.invalidation_count != invalidations + 1U) return 1;
    if (nexus_v1_current_external_structure1f_placement_receipt(
            &engine, &external_receipt) || external_receipt.valid ||
        external_receipt.no_draw_only ||
        external_receipt.blocks_real_dgn_mesh_render ||
        external_receipt.fallback_visuals_permitted ||
        external_receipt.dgn_fnv1a64 || external_receipt.descriptor_index ||
        external_receipt.frame_sequence || external_receipt.command_sequence ||
        external_receipt.descriptor_fnv1a64) return 1;
    structure1f.command_sequence = 0x45U;

    nexus_v1_sync_dgn_runtime_pose(&engine, 4, 1, 2, 3);
    if (engine.external_prs3_placement_valid ||
        engine.external_prs3_placement_dgn_fnv1a64 ||
        engine.external_prs3_placement_descriptor_index ||
        engine.external_prs3_placement_frame_sequence ||
        engine.external_prs3_placement_command_sequence ||
        engine.external_prs3_placement_trace_fnv1a64 ||
        engine.external_prs3_placement_header_fnv1a64 ||
        engine.external_prs3_placement_bitmap_fnv1a64 ||
        engine.external_prs3_placement_bitmap_offset ||
        engine.external_prs3_placement_bitmap_size ||
        engine.external_prs3_placement_palt_fnv1a64 ||
        engine.external_prs3_placement_palt_size ||
        engine.external_structure1f_placement_valid ||
        engine.external_structure1f_placement_dgn_fnv1a64 ||
        engine.external_structure1f_placement_descriptor_index ||
        engine.external_structure1f_placement_frame_sequence ||
        engine.external_structure1f_placement_command_sequence ||
        engine.external_structure1f_placement_descriptor_fnv1a64) return 1;
    engine.game.current_level = 3;
    engine.current_level_structure2_source.loaded_dgn_fnv1a64 = 0x9999U;
    if (engine.external_prs3_placement_dgn_fnv1a64 ==
        engine.current_level_structure2_source.loaded_dgn_fnv1a64) return 1;
    receipt.dgn_fnv1a64 = 0x9999U;
    if (!nexus_v1_engine_set_external_prs3_placement_receipt(&engine, &receipt)) return 1;
    structure1f.dgn_fnv1a64 = 0x9999U;
    structure1f.descriptor_index = 2U;
    structure1f.frame_sequence = 0x44U;
    structure1f.command_sequence = 0x45U;
    structure1f.descriptor_fnv1a64 = 0x99U;
    if (!nexus_v1_engine_set_external_structure1f_placement_binding(
            &engine, &structure1f) ||
        !engine.external_structure1f_placement_valid) return 1;
    receipt.fallback_visuals_permitted = 1;
    if (nexus_v1_engine_set_external_prs3_placement_receipt(&engine, &receipt)) return 1;
    if (engine.external_prs3_placement_valid ||
        engine.external_prs3_placement_dgn_fnv1a64 ||
        engine.external_prs3_placement_descriptor_index ||
        engine.external_prs3_placement_frame_sequence ||
        engine.external_prs3_placement_command_sequence ||
        engine.external_prs3_placement_trace_fnv1a64 ||
        engine.external_prs3_placement_header_fnv1a64 ||
        engine.external_prs3_placement_bitmap_fnv1a64 ||
        engine.external_prs3_placement_bitmap_offset ||
        engine.external_prs3_placement_bitmap_size ||
        engine.external_prs3_placement_palt_fnv1a64 ||
        engine.external_prs3_placement_palt_size ||
        engine.external_structure1f_placement_valid ||
        engine.external_structure1f_placement_dgn_fnv1a64 ||
        engine.external_structure1f_placement_descriptor_index ||
        engine.external_structure1f_placement_frame_sequence ||
        engine.external_structure1f_placement_command_sequence ||
        engine.external_structure1f_placement_descriptor_fnv1a64) return 1;
    receipt.fallback_visuals_permitted = 0;
    receipt.frame_sequence = 0U;
    if (nexus_v1_engine_set_external_prs3_placement_receipt(&engine, &receipt) ||
        engine.external_prs3_placement_valid) return 1;
    receipt.frame_sequence = 0x44U;
    receipt.command_sequence = 0U;
    if (nexus_v1_engine_set_external_prs3_placement_receipt(&engine, &receipt) ||
        engine.external_prs3_placement_valid) return 1;
    receipt.command_sequence = 0x45U;
    receipt.palt_candidate_fnv1a64 = 0U;
    if (nexus_v1_engine_set_external_prs3_placement_receipt(&engine, &receipt)) return 1;
    puts("nexus prs3 placement engine ingress: PASS");
    return 0;
}
