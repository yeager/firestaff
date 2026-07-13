#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) \
    do { if (!(condition)) { fprintf(stderr, "FAIL: %s\\n", message); ++failures; } } while (0)

static void make_source(Nexus_V1_Level *level,
                        Nexus_V1_DgnStructure1FStructure1ACommandSource *source)
{
    Nexus_V1_DgnStructure1FEntry *entry;

    memset(level, 0, sizeof(*level));
    memset(source, 0, sizeof(*source));
    level->structure1f_entry_count = 1;
    level->structure1a_table_valid = 1;
    level->structure1a_model_count = 1;
    level->structure1a_models[0].kind = 0x6aU;
    level->structure1a_models[0].structure3_model_index = 5U;
    level->structure1a_models[0].z_rotation = 2U;
    level->structure3_payload.declared = 1;
    level->structure3_payload.valid = 1;
    level->structure3_payload.block_count = 4;
    level->structure3_payload.complete_block_count = 4;
    level->structure3_payload.byte_size = 4 * NEXUS_DGN_BLOCK_SIZE;

    entry = &level->structure1f_entries[0];
    entry->family = NEXUS_V1_DGN_STRUCTURE1F_ALCOVES;
    entry->tag = 0x20U;
    entry->face = 3U;
    entry->structure1a_index = 0U;
    entry->structure1a_relation_valid = 1;
    entry->structure1a_owner_x = 7;
    entry->structure1a_owner_y = 9;
    entry->structure1a_structure3_model_index = 5U;
    entry->structure1a_z_rotation = 2U;

    source->command_index = 4;
    source->entry_index = 0;
    source->entry = *entry;
    source->owner_x = 7;
    source->owner_y = 9;
    source->structure3_model_index = 5U;
    source->z_rotation = 2U;
}

int main(void)
{
    Nexus_V1_Level level;
    Nexus_V1_DgnStructure1FStructure1ACommandSource source;
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate candidate;
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt receipt;

    make_source(&level, &source);
    CHECK(nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
              &level, &source, 1, &candidate, 1, &receipt) == 0 &&
          receipt.complete && receipt.topology_candidate_count == 1 &&
          receipt.structure1f_binding_count == 1 &&
          receipt.structure1a_row_binding_count == 1 &&
          receipt.structure1a_model_rotation_binding_count == 1 &&
          candidate.structure1f_structure1a_index == 0U &&
          candidate.structure1a_kind == 0x6aU &&
          candidate.structure1a_row_binding_proven &&
          !candidate.structure1a_kind_semantics_proven &&
          candidate.structure1a_structure3_model_index == 5U &&
          candidate.structure1a_z_rotation == 2U &&
          candidate.structure1a_model_rotation_binding_proven &&
          !candidate.structure1a_model_rotation_semantics_proven &&
          !candidate.model_ordinal_proven && !candidate.face_semantics_proven &&
          !candidate.draw_authorized,
          "Structure1F index retains the exact raw Structure1A row without authorizing draw semantics");

    level.structure1a_models[0].structure3_model_index = 6U;
    CHECK(nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
              &level, &source, 1, &candidate, 1, &receipt) == 0 &&
          !receipt.complete && receipt.topology_candidate_count == 0 &&
          receipt.structure1a_row_binding_count == 0 &&
          receipt.blocked_invalid_source_count == 1,
          "a Structure1A row whose raw model byte no longer matches the source fails closed");

    make_source(&level, &source);
    level.structure1a_models[0].z_rotation = 3U;
    CHECK(nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
              &level, &source, 1, &candidate, 1, &receipt) == 0 &&
          !receipt.complete && receipt.topology_candidate_count == 0 &&
          receipt.structure1a_model_rotation_binding_count == 0 &&
          receipt.blocked_invalid_source_count == 1,
          "a Structure1A row whose raw rotation byte no longer matches the source fails closed");

    if (failures) return 1;
    puts("test_nexus_v1_structure1a_structure3_row_receipt: PASS");
    return 0;
}
