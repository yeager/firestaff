#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>

/* The corpus audit identifies selector values but does not itself exercise a
 * runtime pose. This verifies that the first real, unbound ceiling selector
 * reaches the plan gate and remains no-draw. */
static int prove_missing_ceiling_selector_blocks(Nexus_V1_Engine *engine,
                                                 int *out_level,
                                                 int *out_x,
                                                 int *out_y,
                                                 int *out_selector)
{
    int level_index;

    if (out_level) *out_level = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (out_selector) *out_selector = -1;
    if (!engine) return 0;

    for (level_index = 0; level_index < 16; ++level_index) {
        int y;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE; ++x) {
                const Nexus_V1_DgnMaterialPlan *plan;
                const Nexus_V1_DgnRenderPlanReceipt *receipt;
                int selector = engine->current_level.ceiling_material_refs[y][x];

                if (selector < 0 || selector >= NEXUS_DMDF_MATERIAL_COUNT ||
                    engine->floor_materials.surfaces[selector].valid ||
                    engine->current_level.squares[y][x] == 0 ||
                    engine->current_level.floor_animation_ids[y][x] != 0xffU) {
                    continue;
                }
                nexus_v1_sync_dgn_runtime_pose(engine, level_index, x, y, 0);
                plan = nexus_v1_prepare_dgn_material_plan(engine, x, y, 0);
                receipt = &engine->dgn_material_plan.receipt;
                if (!plan && receipt->blocks_real_dgn_mesh_render &&
                    !receipt->fallback_visuals_permitted &&
                    receipt->missing_material_count > 0 &&
                    receipt->first_missing_material_id == (uint8_t)selector) {
                    if (out_level) *out_level = level_index;
                    if (out_x) *out_x = x;
                    if (out_y) *out_y = y;
                    if (out_selector) *out_selector = selector;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int prove_structure1f_direct_entries_reach_plan(Nexus_V1_Engine *engine,
                                                        int *out_level,
                                                        int *out_x,
                                                        int *out_y,
                                                        int *out_entry_count)
{
    int level_index;

    if (out_level) *out_level = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (out_entry_count) *out_entry_count = 0;
    if (!engine) return 0;

    for (level_index = 0; level_index < 16; ++level_index) {
        int entry_index;
        if (nexus_v1_load_level(engine, level_index) != 0) continue;
        for (entry_index = 0;
             entry_index < engine->current_level.structure1f_entry_count;
             ++entry_index) {
            const Nexus_V1_DgnStructure1FEntry *entry =
                &engine->current_level.structure1f_entries[entry_index];
            int direction;

            if (entry->family != NEXUS_V1_DGN_STRUCTURE1F_ITEMS &&
                entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
                entry->family != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) {
                continue;
            }
            if (entry->x >= NEXUS_MAX_MAP_SIZE || entry->y >= NEXUS_MAX_MAP_SIZE ||
                engine->current_level.squares[entry->y][entry->x] == 0 ||
                engine->current_level.floor_animation_ids[entry->y][entry->x] != 0xffU) {
                continue;
            }
            for (direction = 0; direction < 4; ++direction) {
                const Nexus_V1_DgnMaterialPlan *plan;
                const Nexus_V1_DgnRenderPlanReceipt *receipt;

                nexus_v1_sync_dgn_runtime_pose(engine, level_index,
                                                entry->x, entry->y, direction);
                plan = nexus_v1_prepare_dgn_material_plan(
                    engine, entry->x, entry->y, direction);
                receipt = &engine->dgn_material_plan.receipt;
                if (plan && receipt->plan_ready &&
                    !receipt->blocks_real_dgn_mesh_render &&
                    !receipt->fallback_visuals_permitted &&
                    receipt->structure1f_plan_direct_entry_count > 0) {
                    if (out_level) *out_level = level_index;
                    if (out_x) *out_x = entry->x;
                    if (out_y) *out_y = entry->y;
                    if (out_entry_count) {
                        *out_entry_count =
                            receipt->structure1f_plan_direct_entry_count;
                    }
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Skip-safe real-media probe. It reports typed Structure1B selectors,
 * Structure1G declarations, and the bounded Structure2 descriptor envelope;
 * opaque Structure2 bytes remain unexamined and never become materials. */
int main(int argc, char **argv) {
    Nexus_V1_Engine engine;
    Nexus_V1_DgnMaterialCorpusReceipt receipt;
    int missing_level;
    int missing_x;
    int missing_y;
    int missing_selector;
    int structure1f_level;
    int structure1f_x;
    int structure1f_y;
    int structure1f_entry_count;
    const char *root = argc > 1 ? argv[1] : NULL;

    memset(&engine, 0, sizeof(engine));
    memset(&receipt, 0, sizeof(receipt));
    if (nexus_v1_init(&engine, root) < 0) {
        puts("SKIP: Nexus Track 1 data unavailable");
        return 0;
    }
    (void)nexus_v1_inspect_dgn_material_corpus(&engine, &receipt);
    printf("LEV corpus: readable=%d parsed=%d geometry=%d expected=%d\n",
           receipt.readable_level_count, receipt.parsed_level_count,
           receipt.geometry_ready_level_count, receipt.expected_level_count);
    printf("Material refs: floor=%u/%u ceiling=%u/%u wall=%u/%u\n",
           receipt.floor_coverage.command_count,
           receipt.floor_coverage.unique_material_id_count,
           receipt.ceiling_coverage.command_count,
           receipt.ceiling_coverage.unique_material_id_count,
           receipt.wall_coverage.command_count,
           receipt.wall_coverage.unique_material_id_count);
    printf("Typed DGN: 1F levels=%d entries=%d; 1G present=%d valid=%d "
           "animations=%d sequences=%d images=%d gotos=%d "
           "animated-floors=%d bound=%d\n",
           receipt.structure1f_valid_level_count,
           receipt.structure1f_typed_entry_count,
           receipt.structure1g_present_level_count,
           receipt.structure1g_valid_level_count,
           receipt.structure1g_animated_texture_count,
           receipt.structure1g_sequence_count,
           receipt.structure1g_image_instruction_count,
           receipt.structure1g_goto_instruction_count,
           receipt.structure1g_floor_animation_cell_count,
           receipt.structure1g_floor_animation_bound_count);
    printf("Structure2: valid-levels=%d descriptors=%d 1G-first-bound=%d "
           "payload-envelopes=%d opaque-bytes=%d proven-material-levels=%d "
           "canonical-sources=%d bound-payloads=%d offsets=%d/%d/%d "
           "local-pattern-levels=%d\n",
           receipt.structure2_valid_level_count,
           receipt.structure2_texture_count,
           receipt.structure1g_structure2_first_image_bound_count,
           receipt.structure2_payload_envelope_valid_level_count,
           receipt.structure2_opaque_payload_byte_count,
           receipt.structure2_material_or_image_data_proven_level_count,
           receipt.structure2_canonical_source_verified_level_count,
           receipt.structure2_materialization_bound_level_count,
           receipt.structure2_nonzero_descriptor_offset_count,
           receipt.structure2_descriptor_offsets_in_opaque_payload_count,
           receipt.structure2_descriptor_offsets_outside_opaque_payload_count,
           receipt.structure2_local_payload_offset_pattern_level_count);
    printf("Structure3: declared=%d valid=%d bytes=%d nonzero=%d transitions=%d "
           "byte-runs=%d longest-byte-run=%d "
           "zero-blocks=%d nonzero-blocks=%d runs=%d longest-run=%d "
           "complete-model-relations=%d complete-transform-selectors=%d "
           "complete-face-selectors=%d "
           "complete-rotation-selectors=%d "
           "complete-face-rotation-pairs=%d "
           "complete-offset-pairs=%d "
           "complete-wall-payload-selectors=%d "
           "complete-wall-sensor-destinations=%d "
           "complete-wall-sensor-controls=%d "
           "complete-wall-sensor-model-rotation-pairs=%d "
           "complete-wall-decoration-model-rotation-pairs=%d "
           "complete-alcove-payload-selectors=%d "
           "complete-floor-sensor-controls=%d "
           "complete-floor-sensor-destinations=%d "
           "complete-floor-sensor-model-rotation-pairs=%d "
           "complete-floor-sensor-extent-pairs=%d "
           "complete-floor-decoration-payload-selectors=%d "
           "complete-floor-decoration-rotation-selectors=%d "
           "complete-floor-decoration-model-rotation-pairs=%d "
           "complete-floor-decoration-offset-pairs=%d "
           "complete-floor-decoration-control-extents=%d "
           "complete-item-attribute-pairs=%d "
           "complete-item-location-pairs=%d "
           "complete-item-coordinate-pairs=%d "
           "ordinal-block-disproven=%d "
           "ordinal-byte-run-disproven=%d "
           "ordinal-run-disproven=%d ordinal-zero=%d/%d/%d "
           "ordinal-one=%d/%d/%d\n",
           receipt.structure3_payload_declared_level_count,
           receipt.structure3_payload_valid_level_count,
           receipt.structure3_payload_byte_count,
           receipt.structure3_payload_nonzero_byte_count,
           receipt.structure3_payload_transition_count,
           receipt.structure3_nonzero_byte_run_count,
           receipt.structure3_longest_nonzero_byte_run,
           receipt.structure3_zero_block_count,
           receipt.structure3_nonzero_block_count,
           receipt.structure3_nonzero_block_run_count,
           receipt.structure3_longest_nonzero_block_run,
           receipt.structure3_model_reference_complete_level_count,
           receipt.structure1a_transform_selector_complete_level_count,
           receipt.structure1f_face_selector_complete_level_count,
           receipt.structure1f_rotation_selector_complete_level_count,
           receipt.structure1f_face_rotation_pair_complete_level_count,
           receipt.structure1f_offset_pair_complete_level_count,
           receipt.structure1f_wall_payload_selector_complete_level_count,
           receipt.structure1f_wall_sensor_destination_complete_level_count,
           receipt.structure1f_wall_sensor_control_selector_complete_level_count,
           receipt.structure1f_wall_sensor_model_rotation_pair_complete_level_count,
           receipt.structure1f_wall_decoration_model_rotation_pair_complete_level_count,
           receipt.structure1f_alcove_payload_selector_complete_level_count,
           receipt.structure1f_floor_sensor_control_selector_complete_level_count,
           receipt.structure1f_floor_sensor_destination_complete_level_count,
           receipt.structure1f_floor_sensor_model_rotation_pair_complete_level_count,
           receipt.structure1f_floor_sensor_extent_pair_complete_level_count,
           receipt.structure1f_floor_decoration_payload_selector_complete_level_count,
           receipt.structure1f_floor_decoration_rotation_selector_complete_level_count,
           receipt.structure1f_floor_decoration_model_rotation_pair_complete_level_count,
           receipt.structure1f_floor_decoration_offset_pair_complete_level_count,
           receipt.structure1f_floor_decoration_control_extent_complete_level_count,
           receipt.structure1f_item_attribute_pair_complete_level_count,
           receipt.structure1f_item_location_pair_complete_level_count,
           receipt.structure1f_item_coordinate_pair_complete_level_count,
           receipt.structure3_direct_block_ordinal_mapping_disproven_level_count,
           receipt.structure3_direct_byte_run_ordinal_mapping_disproven_level_count,
           receipt.structure3_direct_run_ordinal_mapping_disproven_level_count,
           receipt.structure3_zero_based_block_ordinal_mapping_disproven_level_count,
           receipt.structure3_zero_based_byte_run_ordinal_mapping_disproven_level_count,
           receipt.structure3_zero_based_run_ordinal_mapping_disproven_level_count,
           receipt.structure3_one_based_block_ordinal_mapping_disproven_level_count,
           receipt.structure3_one_based_byte_run_ordinal_mapping_disproven_level_count,
           receipt.structure3_one_based_run_ordinal_mapping_disproven_level_count);
    for (int level = 0; level < 16; ++level) {
        const Nexus_V1_DgnStructure3PayloadReceipt *payload =
            &receipt.structure3_payloads[level];
        if (!payload->declared) continue;
        printf("Structure3 LEV%02d: first-byte-run=%d+%d last-byte-run=%d+%d "
               "first-block-run=%d+%d last-block-run=%d+%d\n",
               level, payload->first_nonzero_byte_run_offset,
               payload->first_nonzero_byte_run_byte_count,
               payload->last_nonzero_byte_run_offset,
               payload->last_nonzero_byte_run_byte_count,
               payload->first_nonzero_block_run_start_block_index,
               payload->first_nonzero_block_run_block_count,
               payload->last_nonzero_block_run_start_block_index,
               payload->last_nonzero_block_run_block_count);
    }
    printf("Coverage: floor=%d ceiling=%d wall=%d mns-host=%d bpk-host=%d complete=%d\n",
           receipt.floor_coverage.covered, receipt.ceiling_coverage.covered,
           receipt.wall_coverage.covered,
           receipt.static_mns_host_route_complete,
           receipt.bpk_host_routes_complete,
           receipt.host_route_evidence_complete);
    printf("Selector surfaces: floor=%u/%u ceiling=%u/%u wall=%u/%u; "
           "first missing ceiling=%u wall=%u\n",
           receipt.floor_coverage.covered_unique_material_id_count,
           receipt.floor_coverage.unique_material_id_count,
           receipt.ceiling_coverage.covered_unique_material_id_count,
           receipt.ceiling_coverage.unique_material_id_count,
           receipt.wall_coverage.covered_unique_material_id_count,
           receipt.wall_coverage.unique_material_id_count,
           receipt.ceiling_coverage.first_missing_material_id,
           receipt.wall_coverage.first_missing_material_id);
    if (prove_missing_ceiling_selector_blocks(&engine, &missing_level,
                                              &missing_x, &missing_y,
                                              &missing_selector)) {
        printf("Fail-closed ceiling selector: level=%d cell=%d,%d id=%d\n",
               missing_level, missing_x, missing_y, missing_selector);
    } else {
        fprintf(stderr, "FAIL: no real missing ceiling selector reached the no-draw plan gate\n");
        nexus_v1_shutdown(&engine);
        return 1;
    }
    if (prove_structure1f_direct_entries_reach_plan(
            &engine, &structure1f_level, &structure1f_x, &structure1f_y,
            &structure1f_entry_count)) {
        printf("Structure1F plan provenance: level=%d cell=%d,%d entries=%d\n",
               structure1f_level, structure1f_x, structure1f_y,
               structure1f_entry_count);
    } else {
        fprintf(stderr, "FAIL: no direct Structure1F entry reached a real no-fallback DGN plan\n");
        nexus_v1_shutdown(&engine);
        return 1;
    }
    printf("Containers: floors present=%d format=%d identity=%d host=%d; "
           "walls present=%d format=%d identity=%d host=%d\n",
           receipt.floor_container.source_present,
           receipt.floor_container.format_valid,
           receipt.floor_container.identity_verified,
           receipt.floor_container.host_route_permitted,
           receipt.wall_container.source_present,
           receipt.wall_container.format_valid,
           receipt.wall_container.identity_verified,
           receipt.wall_container.host_route_permitted);
    nexus_v1_shutdown(&engine);
    return receipt.parsed_level_count == receipt.expected_level_count &&
           receipt.geometry_ready_level_count == receipt.expected_level_count &&
           receipt.static_mns_host_route_complete &&
           !receipt.bpk_host_routes_complete &&
           missing_selector == (int)receipt.ceiling_coverage.first_missing_material_id &&
           structure1f_entry_count > 0
        ? 0 : 1;
}
