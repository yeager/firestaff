/* Emits one dual-source Nexus capture target: a visible Structure1F owner
 * row and an independently selected Structure3 face. It records correlation
 * work for an external Saturn trace and never selects a mesh or draws. */

#include "nexus_v1_engine.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static int parse_nonnegative(const char *text, int *out_value)
{
    char *end = NULL;
    long value;

    if (!text || !out_value) return 0;
    errno = 0;
    value = strtol(text, &end, 0);
    if (errno != 0 || end == text || *end != '\0' || value < 0 ||
        value > 0x7fffffffL) return 0;
    *out_value = (int)value;
    return 1;
}

int main(int argc, char **argv)
{
    Nexus_V1_Engine engine;
    Nexus_V1_DgnStructure2SourceReceipt source_receipt;
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt plan_receipt;
    Nexus_V1_DgnStructure1FStructure1ACommandSource owner_sources[
        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt owner_receipt;
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate candidates[
        NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt candidate_receipt;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt target;
    const Nexus_V1_DgnStructure1AStructure3TopologyCandidate *selected = NULL;
    int level_index;
    int party_x;
    int party_y;
    int party_dir;
    int structure1f_entry_index;
    int structure3_entry_index;
    int face_ordinal;
    int candidate_index;
    int written = 0;

    if (argc != 10 || !parse_nonnegative(argv[2], &level_index) ||
        !parse_nonnegative(argv[3], &party_x) ||
        !parse_nonnegative(argv[4], &party_y) ||
        !parse_nonnegative(argv[5], &party_dir) ||
        !parse_nonnegative(argv[6], &structure1f_entry_index) ||
        !parse_nonnegative(argv[7], &structure3_entry_index) ||
        !parse_nonnegative(argv[8], &face_ordinal) || level_index > 15 ||
        party_x >= NEXUS_MAX_MAP_SIZE || party_y >= NEXUS_MAX_MAP_SIZE ||
        party_dir > 3) {
        fprintf(stderr,
                "usage: %s DATA_DIR LEVEL X Y DIR STRUCTURE1F_ENTRY STRUCTURE3_ENTRY FACE OUTPUT\n",
                argv[0]);
        return 2;
    }
    if (nexus_v1_init(&engine, argv[1]) != 0 ||
        nexus_v1_load_level(&engine, level_index) != 0 ||
        nexus_v1_current_level_structure2_source_receipt(
            &engine, &source_receipt) != 0 ||
        !source_receipt.canonical_hash_verified || !source_receipt.loaded_bytes_bound ||
        nexus_v1_level_build_dgn_view_render_plan(
            &engine.current_level, party_x, party_y, party_dir, commands,
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &plan_receipt) != 0 ||
        nexus_v1_dgn_bind_structure1a_owned_cell_sources(
            &engine.current_level, commands, plan_receipt.command_count,
            owner_sources, NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
            &owner_receipt) != 0 ||
        nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
            &engine.current_level, owner_sources,
            owner_receipt.floor_command_source_count, candidates,
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &candidate_receipt) != 0) {
        fprintf(stderr, "canonical Structure1F owner route unavailable\n");
        nexus_v1_shutdown(&engine);
        return 1;
    }
    for (candidate_index = 0;
         candidate_index < candidate_receipt.topology_candidate_count;
         ++candidate_index) {
        if (candidates[candidate_index].entry_index == structure1f_entry_index) {
            selected = &candidates[candidate_index];
            break;
        }
    }
    if (selected && nexus_v1_dgn_structure1a_structure3_capture_target_build(
            &engine.current_level, engine.current_level_dgn_data,
            engine.current_level_dgn_size, level_index,
            source_receipt.canonical_hash_verified, selected,
            (uint32_t)structure3_entry_index, (uint32_t)face_ordinal,
            &target)) {
        written = nexus_v1_dgn_structure1a_structure3_capture_target_write(
            argv[9], &target);
    }
    printf("canonical_dgn_bound=%d\n", source_receipt.canonical_hash_verified);
    printf("visible_owner_sources=%d\n", owner_receipt.floor_command_source_count);
    printf("topology_candidates=%d\n", candidate_receipt.topology_candidate_count);
    printf("structure1f_owner_target_written=%d\n", written ? 1 : 0);
    printf("structure3_entry_mapping_proven=0\n");
    printf("original_saturn_capture_required=1\n");
    printf("no_draw_only=1\n");
    nexus_v1_shutdown(&engine);
    return written ? 0 : 1;
}
