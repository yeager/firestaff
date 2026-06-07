#include "dm1_v1_chest_destination_guard_pc34_compat.h"

#include <string.h>

typedef struct {
    int openChestThing;
    int leaderHandItem;
    int chestSlots[DM1_PC34_CHEST_DESTINATION_GUARD_SLOT_COUNT];
    int dungeonLinkCount;
    int squareFirstThing;
} DM1_V1_ChestDestinationGuardStatePc34;

static const DM1_V1_ChestDestinationGuardSpecPc34 s_spec = {
    "Source-locked contract gate only; not full real-asset chest/dungeon runtime parity.",
    DM1_PC34_CHEST_DESTINATION_GUARD_FIRST_SLOT,
    DM1_PC34_CHEST_DESTINATION_GUARD_LAST_SLOT,
    DM1_PC34_CHEST_DESTINATION_GUARD_SLOT_COUNT,
    DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE,
    DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_TELEPORTER,
    DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_FAKEWALL,
    DM1_PC34_CHEST_DESTINATION_GUARD_ELEMENT_CORRIDOR
};

static const DM1_V1_ChestDestinationGuardEvidencePc34 s_evidence = {
    "Contract-only in-memory guard; it does not load real party, CHEST, INVENTORY, map, sensor, or asset data.",
    "CHEST.C F0333:31-76 opens a chest and materializes only visible linked contents into G0425_aT_ChestSlots.",
    "CHEST.C F0334:113-132 closes an open chest, clears G0426_T_OpenChest, and relinks visible slots with CM1_MAPX_NOT_ON_A_SQUARE.",
    "CHAMPION.C F0302:688-710 routes C30..C37 chest slot clicks through G0425_aT_ChestSlots before leader-hand mutation.",
    "DUNGEON.C F0163:1796-1837 links to a map square only when P0289_i_MapX >= 0; CM1_MAPX_NOT_ON_A_SQUARE keeps relinks off-map.",
    "DUNGEON.C F0172:2522-2523,2651-2667,2683-2692 classifies fake-wall and teleporter squares for dungeon aspect logic.",
    "DEFS.H:1001,1007-1013,1031-1035,2886 defines M034_SQUARE_TYPE, corridor/teleporter/fakewall elements, masks, and CM1_MAPX_NOT_ON_A_SQUARE.",
    "Disjoint from the twelve named chest/inventory modules: this slice guards rejected dungeon-square destinations before any C537-C544 mutation."
};

static int hash_state(const DM1_V1_ChestDestinationGuardStatePc34* state)
{
    unsigned int hash = 2166136261u;
    int i;

    if (!state) {
        return 0;
    }

    hash = (hash ^ (unsigned int)state->openChestThing) * 16777619u;
    hash = (hash ^ (unsigned int)state->leaderHandItem) * 16777619u;
    hash = (hash ^ (unsigned int)state->dungeonLinkCount) * 16777619u;
    hash = (hash ^ (unsigned int)state->squareFirstThing) * 16777619u;
    for (i = 0; i < DM1_PC34_CHEST_DESTINATION_GUARD_SLOT_COUNT; ++i) {
        hash = (hash ^ (unsigned int)state->chestSlots[i]) * 16777619u;
    }
    return (int)(hash & 0x7fffffff);
}

static void init_guard_state(DM1_V1_ChestDestinationGuardStatePc34* state,
                             int openChestThing)
{
    memset(state, 0, sizeof(*state));
    state->openChestThing = openChestThing;
    state->leaderHandItem = DM1_PC34_CHEST_DESTINATION_GUARD_ITEM;
    state->squareFirstThing = DM1_PC34_CHEST_DESTINATION_GUARD_NONE;
    state->chestSlots[2] = DM1_PC34_CHEST_DESTINATION_GUARD_EXISTING_ITEM;
}

static int destination_is_internal(
    DM1_V1_ChestDestinationGuardDestinationPc34 destination,
    int mapX,
    int mapY)
{
    return destination == DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_INTERNAL &&
           mapX == DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE &&
           mapY == DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE;
}

static int attempt_chest_slot_place(
    DM1_V1_ChestDestinationGuardStatePc34* state,
    int chestSlotIndex,
    DM1_V1_ChestDestinationGuardDestinationPc34 destination,
    int mapX,
    int mapY)
{
    if (!state || chestSlotIndex < 0 ||
        chestSlotIndex >= DM1_PC34_CHEST_DESTINATION_GUARD_SLOT_COUNT) {
        return 0;
    }
    if (state->openChestThing == DM1_PC34_CHEST_DESTINATION_GUARD_CLOSED_CHEST ||
        state->leaderHandItem == DM1_PC34_CHEST_DESTINATION_GUARD_NONE) {
        return 0;
    }

    /* ReDMCSB CHEST.C F0334 lines 117-129 relinks visible chest contents by
     * passing CM1_MAPX_NOT_ON_A_SQUARE to DUNGEON.C F0163.  A C30..C37 chest
     * placement therefore must not also carry a dungeon-square destination,
     * especially one classified by DUNGEON.C F0172 as teleporter/fake-wall. */
    if (!destination_is_internal(destination, mapX, mapY)) {
        return 0;
    }

    state->chestSlots[chestSlotIndex] = state->leaderHandItem;
    state->leaderHandItem = DM1_PC34_CHEST_DESTINATION_GUARD_NONE;
    return 1;
}

static void run_attempt(
    DM1_V1_ChestDestinationGuardAttemptPc34* out,
    int openChestThing,
    DM1_V1_ChestDestinationGuardDestinationPc34 destination,
    int mapX,
    int mapY)
{
    DM1_V1_ChestDestinationGuardStatePc34 state;
    const int targetSlot = 0;

    init_guard_state(&state, openChestThing);
    out->stateHashBefore = hash_state(&state);
    out->result = attempt_chest_slot_place(&state, targetSlot, destination,
                                           mapX, mapY);
    out->rejected = !out->result;
    out->stateHashAfter = hash_state(&state);
    out->stateStable = out->stateHashBefore == out->stateHashAfter;
    out->leaderHandAfter = state.leaderHandItem;
    out->destinationSlotAfter = state.chestSlots[targetSlot];
    out->openChestThingAfter = state.openChestThing;
    out->dungeonLinkCountAfter = state.dungeonLinkCount;
    out->squareFirstThingAfter = state.squareFirstThing;
}

const DM1_V1_ChestDestinationGuardEvidencePc34*
M11_GameView_ChestDestinationGuardEvidencePc34(void)
{
    return &s_evidence;
}

const DM1_V1_ChestDestinationGuardSpecPc34*
M11_GameView_ChestDestinationGuardSpecPc34(void)
{
    return &s_spec;
}

int M11_GameView_ChestDestinationGuardRunPc34(
    DM1_V1_ChestDestinationGuardProbePc34* out)
{
    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    out->contract_only = 1;
    out->no_real_asset_data = 1;
    out->openChestThing = DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST;
    out->leaderHandItem = DM1_PC34_CHEST_DESTINATION_GUARD_ITEM;
    out->targetChestSlot = DM1_PC34_CHEST_DESTINATION_GUARD_FIRST_SLOT;
    out->occupiedSentinel = DM1_PC34_CHEST_DESTINATION_GUARD_EXISTING_ITEM;
    out->initialDungeonLinkCount = 0;
    out->initialSquareFirstThing = DM1_PC34_CHEST_DESTINATION_GUARD_NONE;

    run_attempt(&out->teleporterAttempt,
                DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST,
                DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_TELEPORTER,
                4, 14);
    run_attempt(&out->fakewallAttempt,
                DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST,
                DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_FAKEWALL,
                5, 14);
    run_attempt(&out->corridorSquareAttempt,
                DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST,
                DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_CORRIDOR,
                6, 14);
    run_attempt(&out->closedChestAttempt,
                DM1_PC34_CHEST_DESTINATION_GUARD_CLOSED_CHEST,
                DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_INTERNAL,
                DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE,
                DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE);
    run_attempt(&out->internalChestAttempt,
                DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST,
                DM1_V1_CHEST_DESTINATION_GUARD_DESTINATION_INTERNAL,
                DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE,
                DM1_PC34_CHEST_DESTINATION_GUARD_NOT_ON_SQUARE);

    out->internalStoredItem =
        out->internalChestAttempt.destinationSlotAfter ==
        DM1_PC34_CHEST_DESTINATION_GUARD_ITEM;
    out->internalLeaderHandEmpty =
        out->internalChestAttempt.leaderHandAfter ==
        DM1_PC34_CHEST_DESTINATION_GUARD_NONE;
    out->internalNoDungeonLink =
        out->internalChestAttempt.dungeonLinkCountAfter == 0 &&
        out->internalChestAttempt.squareFirstThingAfter ==
        DM1_PC34_CHEST_DESTINATION_GUARD_NONE;
    out->internalOpenChestStable =
        out->internalChestAttempt.openChestThingAfter ==
        DM1_PC34_CHEST_DESTINATION_GUARD_OPEN_CHEST;
    out->rejectedAttemptsStable =
        out->teleporterAttempt.stateStable &&
        out->fakewallAttempt.stateStable &&
        out->corridorSquareAttempt.stateStable &&
        out->closedChestAttempt.stateStable;

    return 1;
}
