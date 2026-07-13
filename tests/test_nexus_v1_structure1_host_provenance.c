#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(c, m) do { if (!(c)) { printf("FAIL: %s\n", m); ++failures; } } while (0)

static void make_direct_level(Nexus_V1_Level *level)
{
    memset(level, 0, sizeof(*level));
    level->width = NEXUS_MAX_MAP_SIZE;
    level->height = NEXUS_MAX_MAP_SIZE;
    level->geometry_info.structure1f_declared = 1;
    level->geometry_info.structure1f_valid = 1;
    level->geometry_info.structure1f_total_entry_count = 1;
    level->structure1f_entry_count = 1;
    level->structure1f_entries[0].family = NEXUS_V1_DGN_STRUCTURE1F_ITEMS;
    level->structure1f_entries[0].x = 11;
    level->structure1f_entries[0].y = 29;
}

int main(void)
{
    Nexus_V1_Level level;
    Nexus_V1_DgnStructure1HostProvenanceReceipt receipt;

    make_direct_level(&level);
    CHECK(nexus_v1_level_dgn_structure1_host_provenance_receipt(
              &level, &receipt) == 0 &&
          receipt.status == NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_DIRECT &&
          receipt.can_prepare_runtime_dgn &&
          !receipt.blocks_real_dgn_mesh_render &&
          receipt.structure1f_spatial.direct_coordinate_entry_count == 1 &&
          receipt.structure1a_boundary.entry_count == 0,
          "verified direct Structure1F records reach the host without inferred semantics");

    level.structure1f_entries[0].family = NEXUS_V1_DGN_STRUCTURE1F_ALCOVES;
    level.structure1f_entries[0].structure1a_index = 4;
    CHECK(nexus_v1_level_dgn_structure1_host_provenance_receipt(
              &level, &receipt) == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION &&
          !receipt.can_prepare_runtime_dgn &&
          receipt.blocks_real_dgn_mesh_render &&
          receipt.structure1a_boundary.valid &&
          receipt.structure1a_boundary.entry_count == 1 &&
          strcmp(nexus_v1_dgn_structure1_host_provenance_status_name(
                     receipt.status), "blocked-structure1a-relation") == 0,
          "Structure1A-indexed records fail closed before host rendering");

    level.structure1a_table_valid = 1;
    level.structure1a_model_count = 5;
    level.structure1a_owner_ref_valid[9][7] = 1;
    level.structure1a_owner_refs[9][7] = 4;
    level.structure1f_entries[0].structure1a_relation_valid = 1;
    level.structure1f_entries[0].structure1a_owner_x = 7;
    level.structure1f_entries[0].structure1a_owner_y = 9;
    level.structure1f_entries[0].structure1a_structure3_model_index = 2;
    CHECK(nexus_v1_level_dgn_structure1_host_provenance_receipt(
              &level, &receipt) == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_RESOLVED_STRUCTURE1A &&
          receipt.can_prepare_runtime_dgn &&
          !receipt.blocks_real_dgn_mesh_render &&
          receipt.structure1a_relation.complete &&
          receipt.structure1a_relation.resolved_entry_count == 1 &&
          strcmp(nexus_v1_dgn_structure1_host_provenance_status_name(
                     receipt.status), "ready-resolved-structure1a") == 0,
          "only a complete Structure1A owner/model relation advances the host gate");

    level.structure1f_entries[0].structure1a_relation_valid = 0;
    CHECK(nexus_v1_level_dgn_structure1_host_provenance_receipt(
              &level, &receipt) == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION &&
          !receipt.can_prepare_runtime_dgn &&
          !receipt.structure1a_relation.complete,
          "an incomplete Structure1A relation remains fail closed at the host gate");

    make_direct_level(&level);
    level.geometry_info.structure1f_valid = 0;
    CHECK(nexus_v1_level_dgn_structure1_host_provenance_receipt(
              &level, &receipt) == 0 &&
          receipt.status ==
              NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1F_LAYOUT &&
          !receipt.can_prepare_runtime_dgn &&
          receipt.blocks_real_dgn_mesh_render,
          "declared malformed Structure1F records fail closed before host rendering");

    if (failures) return 1;
    puts("test_nexus_v1_structure1_host_provenance: PASS");
    return 0;
}
