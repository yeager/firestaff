#ifndef FIRESTAFF_DM1_V1_CHEST_AUTO_CLOSE_ON_LEADER_DEATH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_AUTO_CLOSE_ON_LEADER_DEATH_PC34_COMPAT_H

/*
 * DM1 V1 runtime regression gate: the inventory champion (who is also the
 * party leader by definition) takes a fatal hit while G0426 / G0425 are
 * live; CHAMPION.C F0319 lines 1552-1607 (F0319_CHAMPION_Kill) must:
 *   (a) drop CurrentHealth to 0,
 *   (b) clear G0333 (pressing-mouth) and G0331 (pressing-eye) if they
 *       are set,
 *   (c) call PANEL.C F0355 lines 2244-2310 (F0355_INVENTORY_Toggle_CPSE)
 *       with C04_CHAMPION_CLOSE_INVENTORY, which routes through
 *       CHEST.C F0334 lines 79-130 (F0334_INVENTORY_CloseChest) to
 *       rewire the live G0425 chain to the chest's Slot list and
 *       clear G0426,
 *   (d) call CHAMPION.C F0318 lines 1527-1551 (F0318_CHAMPION_DropAllObjects)
 *       which drops every C00..C29 slot object to the floor.
 *
 * The contract pins: G0426 reaches C0xFFFF_THING_NONE; G0425 slots
 * reach C0xFFFF_THING_NONE (because the close rewires them out into the
 * container's Slot list, not into the C30..C37 array); G0423 reaches
 * the CM1_CHAMPION_NONE sentinel; G0424 lands at C00_PANEL_INVENTORY
 * after the close; the leader's C00/C01 hand bytes are byte-stable
 * (the death-clear must not corrupt them before the drop); and
 * M516_CHAMPIONS[leader].CurrentHealth == 0.
 *
 * The leader hand is permitted to remain non-empty after death because
 * F0318 will drop it on the very next drop cycle; the gate asserts
 * that the post-drop reachability state is consistent (hand bytes
 * reach the empty sentinel exactly once F0318 has run, and only after
 * F0318 has run).
 *
 * ReDMCSB anchors:
 * - CHAMPION.C F0319:1552-1607 (F0319_CHAMPION_Kill) sets
 *   CurrentHealth = 0 and dispatches the inventory close + drop.
 * - CHAMPION.C F0318:1527-1551 (F0318_CHAMPION_DropAllObjects) drops
 *   every C00..C29 slot to the leader's current Cell.
 * - PANEL.C F0355:2244-2310 (F0355_INVENTORY_Toggle_CPSE) closes the
 *   inventory panel when called with C04_CHAMPION_CLOSE_INVENTORY and
 *   the dying champion is the inventory champion.
 * - PANEL.C F0355:2268-2275 (death short-circuit) returns early when
 *   the champion is dead and not closing inventory.
 * - PANEL.C F0355:2318-2322 (F0334 call) is the one place that mutates
 *   G0426 to C0xFFFF_THING_NONE outside the open path.
 * - CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest) clears G0426 and
 *   rewires G0425_aT_ChestSlots into the container Slot list.
 * - CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest) is asserted
 *   as a negative no-open anchor (no reopen happens during the death).
 * - CHAMPION.C F0297/F0298:243-298 own the global leader-hand object
 *   (asserted as a no-leader-hand-mutate anchor during the death
 *   transition because F0318 runs after F0334).
 * - CHAMPION.C F0300/F0301:511-614 are the C00..C29 get/put object
 *   primitives F0318 uses.
 * - COMMAND.C F0380:2045-2184 is asserted as a negative no-queue-drain
 *   anchor: the death path does not require a queued command.
 * - DEFS.H C00..C29, C30..C37, C038, C040, C04_CHAMPION_CLOSE_INVENTORY,
 *   C10_COLOR_FLESH, G0299, G0331, G0333, G0423, G0424, G0425, G0426,
 *   M516_CHAMPIONS[].CurrentHealth/Load/Slots.
 *
 * Contract-only, deterministic, no game data, no GRAPHICS.DAT, no
 * DUNGEON.DAT, no real-asset pixels.
 */

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_ACLD_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_CHEST_ACLD_LEADER_PC34 = 0,
    DM1_V1_CHEST_ACLD_DEAD_LEADER_PC34 = 0,
    DM1_V1_CHEST_ACLD_NEXT_LEADER_PC34 = 1,
    DM1_V1_CHEST_ACLD_PARTY_CHAMPION_PC34 = 2,
    DM1_V1_CHEST_ACLD_IDLE_CHAMPION_PC34 = 3,
    DM1_V1_CHEST_ACLD_SLOT_COUNT_PC34 = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_V1_CHEST_ACLD_LEADER_HEALTH_BEFORE_PC34 = 88,
    DM1_V1_CHEST_ACLD_FATAL_DAMAGE_PC34 = 88,
    DM1_V1_CHEST_ACLD_LEADER_HEALTH_AFTER_PC34 = 0,
    DM1_V1_CHEST_ACLD_CHEST_THING_PC34 = 0x6A61,
    DM1_V1_CHEST_ACLD_LEADER_HAND_ITEM_PC34 = 0xC061,
    DM1_V1_CHEST_ACLD_LEADER_ACTION_ITEM_PC34 = 0x6101,
    DM1_V1_CHEST_ACLD_PANEL_CHEST_PC34 = DM1_PC34_PANEL_CHEST,
    DM1_V1_CHEST_ACLD_PANEL_INVENTORY_PC34 = DM1_PC34_PANEL_INVENTORY,
    DM1_V1_CHEST_ACLD_PANEL_RESURRECT_PC34 = DM1_PC34_PANEL_RESURRECT_REINCARNATE,
    DM1_V1_CHEST_ACLD_NONE_CHAMPION_PC34 = -1,
    DM1_V1_CHEST_ACLD_NO_THING_PC34 = 0,
    DM1_V1_CHEST_ACLD_C00_READY_HAND_PC34 = 0,
    DM1_V1_CHEST_ACLD_C01_ACTION_HAND_PC34 = 1,
    DM1_V1_CHEST_ACLD_C29_LAST_BODY_SLOT_PC34 = 29,
    DM1_V1_CHEST_ACLD_DETERMINISTIC_SEED_PC34 = 0xF0319AC1u
};

typedef enum {
    DM1_V1_CHEST_ACLD_STEP_OPEN_CHEST_PC34 = 0,
    DM1_V1_CHEST_ACLD_STEP_PRESSING_EYE_PC34 = 1,
    DM1_V1_CHEST_ACLD_STEP_F0319_KILL_PC34 = 2,
    DM1_V1_CHEST_ACLD_STEP_F0355_CLOSE_PC34 = 3,
    DM1_V1_CHEST_ACLD_STEP_F0334_REWIRE_PC34 = 4,
    DM1_V1_CHEST_ACLD_STEP_F0318_DROP_PC34 = 5,
    DM1_V1_CHEST_ACLD_STEP_ASSERT_STABLE_PC34 = 6
} DM1_V1_ChestAutoCloseOnLeaderDeathStepPc34;

typedef struct {
    const char* contractMarker;
    const char* f0319KillAnchor;
    const char* f0318DropAnchor;
    const char* f0355ToggleAnchor;
    const char* f0355DeathShortCircuitAnchor;
    const char* f0334CloseAnchor;
    const char* f0333OpenAnchor;
    const char* f0297HandAnchor;
    const char* f0298HandAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0380QueueAnchor;
    const char* defsAnchor;
    const char* disjointness;
    uint32_t deterministicSeed;
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
} DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int noGraphicsDatLoad;
    int noDungeonDatLoad;
    int noRealAssetPixels;
    int runtimeRegression;

    uint32_t deterministicSeed;
    uint32_t deterministicHash;
    int stepTrace[7];
    int stepCount;

    int leaderHealthBefore;
    int leaderHealthAfter;
    int fatalDamageApplied;
    int leaderCurrentHealthCleared;
    int f0318RanAfterF0334;
    int f0318DropCount;

    int g0426Before;
    int g0426AfterF0319;
    int g0426AfterF0334;
    int g0426Final;
    int g0426ClearedByF0334;
    int g0425AllSlotsClearedByF0334;
    int g0425VisibleCountBefore;
    int g0425VisibleCountAfterF0334;
    int containerSlotHeadBefore;
    int containerSlotHeadAfterF0334;

    int g0423Before;
    int g0423AfterF0319;
    int g0423Final;
    int g0423ClearedToNone;
    int g0424Before;
    int g0424AfterF0355;
    int g0424Final;
    int g0424EndedAtInventory;

    int f0319Observed;
    int f0318Observed;
    int f0355Observed;
    int f0334Observed;
    int f0333NotReopened;
    int f0077BracketedF0319;
    int f0078BracketedF0319;

    int leaderHandItemBefore;
    int leaderHandItemAfterF0319BeforeF0318;
    int leaderHandItemAfterF0318;
    int leaderActionHandItemBefore;
    int leaderActionHandItemAfterF0319BeforeF0318;
    int leaderActionHandItemAfterF0318;
    int leaderHandByteStableAcrossF0319;
    int leaderHandClearedByF0318;

    int pressingEyeBefore;
    int pressingEyeAfterF0319;
    int pressingEyeClearedByF0319;
    int pressingMouthBefore;
    int pressingMouthAfterF0319;
    int pressingMouthClearedByF0319;

    int deadLeaderHandObjectDropped;
    int deadLeaderActionObjectDropped;
    int deadLeaderDropIterationCount;

    int f0297LeaderHandPutDuringDeath;
    int f0298LeaderHandRemovedDuringDeath;
    int f0300RemoveFromC30;
    int f0301AddToC30;
    int f0355LeaderHandEmptyGuardSatisfied;
    int f0380QueueDepthAtDeath;
    int f0380QueueDidNotDrain;

    int noPassC061DropDuringRotation;
    int noPassC040DropDuringRotation;
    int noPassResurrectRotationScrollWheel;
    int noPassPickupDuringResurrectPending;
    int noPassCloseWhileCandidateOpenReopen;
    int noPassTeleporterSurvivalOpenG0426;
} DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34;

const char*
dm1_v1_chest_auto_close_on_leader_death_source_evidence_pc34(void);
const DM1_V1_ChestAutoCloseOnLeaderDeathSpecPc34*
dm1_v1_chest_auto_close_on_leader_death_spec_pc34(void);
int dm1_v1_chest_auto_close_on_leader_death_run_pc34(
    DM1_V1_ChestAutoCloseOnLeaderDeathProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_AUTO_CLOSE_ON_LEADER_DEATH_PC34_COMPAT_H */
