/* Source-owned File_header world materialisation for DM2 New Game. */

#include "dm2_v1_game_load_world_owner.h"

#include <string.h>

static int dm2_v1_game_load_owner_validate_possessions(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    int i;

    if (!owner) return 0;
    for (i = 0; i < owner->transaction.possessions.placed_item_count; ++i) {
        const DM2_V1_BootNewGamePossession *possession =
            &owner->transaction.possessions.possessions[i];
        if (possession->source_object_id == 0u ||
            possession->equipped_record_id !=
                (uint16_t)(possession->source_object_id & 0x3fffu) ||
            dm2_v1_record_pool_address(&owner->record_pools,
                (int16_t)possession->source_object_id) == NULL) {
            return 0;
        }
    }
    return 1;
}

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner)
{
    if (!owner) return;
    dm2_v1_record_pool_set_free(&owner->record_pools);
    dm2_v1_dungeon_free(&owner->dungeon);
    memset(owner, 0, sizeof(*owner));
}

int dm2_v1_game_load_world_owner_init_new_game(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootProfile *profile,
    const DM2_V1_BootNewGamePartySelection *selections,
    int selection_count)
{
    const DM2_V1_DungeonData *source;
    DM2_V1_GameLoadWorldOwner candidate;

    if (!owner) return 0;
    /* Constructor callers pass a fresh/zeroed owner.  Do not inspect a
     * possibly uninitialised caller buffer here; reinitialising an existing
     * owner is explicitly free-then-init so no source RAM owner leaks. */
    memset(owner, 0, sizeof(*owner));
    memset(&candidate, 0, sizeof(candidate));
    if (!profile || !profile->assets_verified || !profile->dungeon_data ||
        !selections || selection_count <= 0 ||
        selection_count > DM2_MAX_HEROES ||
        !dm2_v1_boot_new_game_transaction_receipt(
            profile, selections, selection_count, &candidate.transaction) ||
        !candidate.transaction.valid ||
        !candidate.transaction.incomplete_game_load) {
        return 0;
    }
    source = (const DM2_V1_DungeonData *)profile->dungeon_data;
    if (!source->raw_data || source->raw_size <= 0 ||
        !source->record_graph_complete ||
        candidate.transaction.entrance.map < 0 ||
        candidate.transaction.entrance.map >= source->level_count ||
        dm2_v1_dungeon_load(&candidate.dungeon, source->raw_data,
                            source->raw_size) != 0 ||
        !candidate.dungeon.record_graph_complete ||
        !candidate.dungeon.initial_party_pose_valid ||
        candidate.dungeon.initial_party_x != candidate.transaction.entrance.x ||
        candidate.dungeon.initial_party_y != candidate.transaction.entrance.y ||
        candidate.dungeon.initial_party_dir != candidate.transaction.entrance.direction ||
        !dm2_v1_record_pool_set_init_from_dungeon(&candidate.record_pools,
                                                   &candidate.dungeon) ||
        !candidate.record_pools.valid ||
        !candidate.record_pools.record_graph_complete) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }

    candidate.current_map = candidate.transaction.entrance.map;
    candidate.selected_party = candidate.transaction.possessions.projected_party;
    candidate.source_transaction_hash = candidate.transaction.transaction_hash;
    if (candidate.selected_party.heros_in_party != selection_count ||
        candidate.source_transaction_hash == 0u ||
        !dm2_v1_game_load_owner_validate_possessions(&candidate)) {
        dm2_v1_game_load_world_owner_free(&candidate);
        return 0;
    }
    candidate.prepared = 1;
    candidate.committed = 0;
    *owner = candidate;
    return 1;
}

int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    return owner != NULL && owner->prepared && !owner->committed &&
        owner->dungeon.raw_data != NULL && owner->dungeon.raw_size > 0 &&
        owner->record_pools.valid && owner->record_pools.record_graph_complete &&
        owner->source_transaction_hash != 0u;
}
