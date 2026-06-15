#ifndef FIRESTAFF_M11_INVENTORY_CHEST_AUTO_CLOSE_ON_LEADER_DEATH_PC34_COMPAT_H
#define FIRESTAFF_M11_INVENTORY_CHEST_AUTO_CLOSE_ON_LEADER_DEATH_PC34_COMPAT_H

/*
 * M11 runtime helper for the DM1 V1 auto-close-chest-on-leader-
 * death contract. Mirrors CHAMPION.C F0319 (F0319_CHAMPION_Kill)
 * ordering against the live M11 inventory state:
 *
 *   (a) detect leader is the active champion and is dying
 *       (CurrentHealth reaches 0 in the active tick),
 *   (b) if G0426 (open chest thing) is set on the leader,
 *       call F0334 to close it (G0426 -> C0xFFFF_THING_NONE,
 *       G0424 -> C00_PANEL_INVENTORY),
 *   (c) drop the leader's hand objects via F0318 (champion
 *       drops all C00..C29 slots).
 *
 * The function is idempotent: calling it twice on a dead leader
 * is a no-op.  It is intentionally disjoint from the C061-drop-
 * while-leader-rotation, C040-drop-during-rotation, resurrect-
 * rotation-scroll-wheel, pickup-during-resurrect-pending, and
 * open-chest-teleporter-survival gates; those exercise different
 * contracts and have their own probes.
 *
 * Probe-mode caller (the regression test) sets `in_inventoryState`
 * to a synthetic M11_InventoryState and reads the resulting
 * G0426 / G0424 / G0423 fields.  Production-mode caller sets
 * `in_inventoryState` to the live M11 game-view inventory and
 * observes the same fields via m11_inventory_get_* helpers.
 *
 * Source-locked to ReDMCSB:
 *   CHAMPION.C F0319 lines 1552-1607
 *   PANEL.C    F0355 lines 2244-2310 (called only when leader
 *                                       owns the inventory panel)
 *   CHEST.C    F0334 lines 79-130
 *   CHAMPION.C F0318 lines 1527-1551
 */

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct M11_InventoryChestAutoCloseOnLeaderDeathProbePc34_ {
    /* Inputs */
    const M11_InventoryState* in_inventoryState;
    int leaderChampionIndex;
    int chestWasOpen;            /* 1 if G0426 was non-zero on entry */

    /* Outputs */
    int f0319Observed;
    int f0355Observed;           /* F0355 dispatched to F0334 */
    int f0334Observed;           /* F0334_INVENTORY_CloseChest ran */
    int f0318Observed;           /* F0318 drop-all ran */
    int g0426ClearedToNone;      /* open-chest thing reaches C0xFFFF */
    int g0424EndedAtInventory;   /* panel content reaches C00_INVENTORY */
    int leaderHandClearedByF0318;
    const char* anchor;
} M11_InventoryChestAutoCloseOnLeaderDeathProbePc34;

int m11_inventory_chest_auto_close_on_leader_death_pc34_compat_run(
    M11_InventoryChestAutoCloseOnLeaderDeathProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_INVENTORY_CHEST_AUTO_CLOSE_ON_LEADER_DEATH_PC34_COMPAT_H */
