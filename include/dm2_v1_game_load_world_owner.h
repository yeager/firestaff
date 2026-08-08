#ifndef FIRESTAFF_DM2_V1_GAME_LOAD_WORLD_OWNER_H
#define FIRESTAFF_DM2_V1_GAME_LOAD_WORLD_OWNER_H

/*
 * dm2_v1_game_load_world_owner.h — owned File_header world for New Game.
 *
 * SKProject loads the dungeon structure before DM2_GAME_LOAD mutates the
 * DYN, champion and timer state.  This owner is that first durable boundary:
 * it copies only the hash-admitted File_header image and its exact DB pools
 * to RAM.  It is intentionally not a playable session.
 *
 * Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON (616-640),
 *         ::DM2_GAME_LOAD (1415-1546),
 *         SKULLWIN/c_record.cpp::DM2_GET_ADDRESS_OF_RECORD (28-57).
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* `prepared` means all source bytes have one RAM owner. `committed` is
     * deliberately zero until the later source-ordered DYN/hero/timer
     * transaction can publish every owner atomically. */
    int prepared;
    int committed;
    int current_map;
    uint32_t source_transaction_hash;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet record_pools;
    DM2_V1_BootNewGameTransactionReceipt transaction;
    /* Exact detached result of the source selection receipt.  This is not
     * installed in M11 or the runtime party state. */
    DM2_V1_Party selected_party;
} DM2_V1_GameLoadWorldOwner;

/* Build a source-owned, pre-selection world from one currently mounted,
 * hash-verified PC File_header/GDAT pair and exact mirror clicks.  It rejects
 * partial record graphs and never modifies profile-owned raw media. */
int dm2_v1_game_load_world_owner_init_new_game(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootProfile *profile,
    const DM2_V1_BootNewGamePartySelection *selections,
    int selection_count);

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner);
int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner);

#ifdef __cplusplus
}
#endif

#endif
