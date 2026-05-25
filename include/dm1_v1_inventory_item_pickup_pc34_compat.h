#ifndef FIRESTAFF_DM1_V1_INVENTORY_ITEM_PICKUP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_ITEM_PICKUP_PC34_COMPAT_H

#include <stdint.h>
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * dm1_v1_inventory_item_pickup_pc34_compat.c:m11_inventory_can_pickup_active_champion:1
 *
 * Pre-flight check: active champion must have at least one free inventory slot.
 * Mirrors m11_find_empty_slot() in m11_game_view.c.
 * Source: CHAMPION.C:694 F0302 empty-slot guard
 *         (slot empty -> continue; no free slot -> reject).
 *         CLIKVIEW.C F0373: grab only when leader hand empty AND free slot exists.
 *
 * Argument:
 *   champ  - ChampionState_Compat* active champion (state->world.party.champions[idx])
 *
 * Returns: 1 if free slot exists, 0 if inventory full.
 */
int m11_inventory_can_pickup_active_champion(const struct ChampionState_Compat* champ);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INVENTORY_ITEM_PICKUP_PC34_COMPAT_H */
