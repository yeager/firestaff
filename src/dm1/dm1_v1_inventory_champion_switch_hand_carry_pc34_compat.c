#include "include/dm1/dm1_v1_inventory_champion_switch_hand_carry_pc34_compat.h"

#include <string.h>

/* Contract-only synthetic gate for changing the open inventory champion while
 * the global leader hand carries an item. This models the F0354 panel-state
 * path only; it does not claim real-asset or original-DOS pixel parity.
 */

static const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34 s_evidence = {
    "ReDMCSB PANEL.C:2267-2285 F0354_INVENTORY_DrawStatusBoxPortrait "
    "rejects dead champions and mouth/eye presses before panel or hand state "
    "mutates",
    "ReDMCSB PANEL.C:2299-2322 F0354 captures old G0423, clears it, calls "
    "F0334_INVENTORY_CloseChest, and redraws the old champion state",
    "ReDMCSB PANEL.C:2335-2352 F0354 close-inventory branch refreshes mouse, "
    "movement input, discards input, and draws floor/ceiling",
    "ReDMCSB PANEL.C:2363-2433 F0354 assigns the new G0423 ordinal, draws "
    "C00..C29 inventory slots, marks attributes, and redraws champion state",
    "ReDMCSB PANEL.C:2437-2447 F0354 updates mouse pointer bitmap, triggers "
    "input refresh, selects inventory secondary input, and discards input",
    "ReDMCSB CHEST.C:113-132 F0334_INVENTORY_CloseChest rewrites and clears "
    "the open G0426/G0425 chest state when F0354 leaves an old inventory, "
    "skipping empty G0425 slots while preserving the visible order",
    "ReDMCSB PANEL.C:2153-2158 F0352 and PANEL.C:2183-2190 F0353 read "
    "G4055_s_LeaderHandObject for hand-object drawing/name; F0354 itself "
    "does not move that global hand object",
    "Non-overlap: this covers champion-switch/close preservation of a carried "
    "leader-hand item across F0354. It does not cover C537-C544 slot masks, "
    "occupied-slot swap, status hand box routing, chest-open races, or mirror "
    "candidate select/click/cancel/deadzone/cancel-reselect."
};

static void capture_before(
    const DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state,
    int requestedChampionIndex,
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34* result)
{
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->requestedChampionIndex = requestedChampionIndex;
    result->oldInventoryOrdinalBefore = state->g0423InventoryChampionOrdinal;
    result->leaderHandThingBefore = state->g4055LeaderHandThing;
    result->openChestThingBefore = state->g0426OpenChestThing;
}

static void capture_after(
    const DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state,
    const DM1_V1_InventoryChampionSwitchHandCarryStatePc34* before,
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34* result)
{
    result->oldInventoryOrdinalAfter = before->g0423InventoryChampionOrdinal;
    result->targetOrdinalAfter = state->g0423InventoryChampionOrdinal;
    result->leaderHandThingAfter = state->g4055LeaderHandThing;
    result->leaderHandPreserved =
        result->leaderHandThingBefore == result->leaderHandThingAfter;
    result->openChestThingAfter = state->g0426OpenChestThing;
    result->chestClosed =
        result->openChestThingBefore != DM1_V1_ICSWHC_THING_NONE_PC34 &&
        result->openChestThingAfter == DM1_V1_ICSWHC_THING_NONE_PC34;
    result->chestCloseCountAfter = state->f0334ClosedCount;
    result->chestSlotsClearedAfterClose = 1;
    for (int i = 0; i < DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34; ++i) {
        result->closedChestTypes[i] = state->f0334ClosedTypes[i];
        if (state->g0425ChestSlots[i] != DM1_V1_ICSWHC_THING_NONE_PC34) {
            result->chestSlotsClearedAfterClose = 0;
        }
    }
    result->oldStatusDrawDelta =
        state->oldStatusDrawCount - before->oldStatusDrawCount;
    result->newStatusDrawDelta =
        state->newStatusDrawCount - before->newStatusDrawCount;
    result->slotDrawDelta = state->slotDrawCount - before->slotDrawCount;
    result->movementArrowsDrawDelta =
        state->movementArrowsDrawCount - before->movementArrowsDrawCount;
    result->floorCeilingDrawDelta =
        state->floorCeilingDrawCount - before->floorCeilingDrawCount;
    result->mousePointerBitmapUpdatedDelta =
        state->mousePointerBitmapUpdated - before->mousePointerBitmapUpdated;
    result->refreshMousePointerDelta =
        state->refreshMousePointerInMainLoop -
        before->refreshMousePointerInMainLoop;
    result->secondaryInputInventoryAfter = state->secondaryInputInventory;
    result->secondaryInputMovementAfter = state->secondaryInputMovement;
    result->discardInputDelta =
        state->discardInputCount - before->discardInputCount;
    result->stopWaitingForPlayerInputAfter =
        state->stopWaitingForPlayerInput;
}

static void close_open_chest_if_any(
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state)
{
    if (state->g0426OpenChestThing != DM1_V1_ICSWHC_THING_NONE_PC34) {
        int closedCount = 0;

        /* ReDMCSB CHEST.C F0334 lines 117-132 rewrites the container from
         * non-empty G0425 slots in visible order and clears each processed
         * slot before F0354 assigns the next G0423 inventory champion. */
        for (int i = 0; i < DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34; ++i) {
            state->f0334ClosedTypes[i] = DM1_V1_ICSWHC_THING_NONE_PC34;
        }
        for (int i = 0; i < DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34; ++i) {
            if (state->g0425ChestSlots[i] !=
                DM1_V1_ICSWHC_THING_NONE_PC34) {
                state->f0334ClosedTypes[closedCount++] =
                    state->g0425ChestSlots[i];
            }
            state->g0425ChestSlots[i] = DM1_V1_ICSWHC_THING_NONE_PC34;
        }
        state->f0334ClosedCount = closedCount;
        state->g0426OpenChestThing = DM1_V1_ICSWHC_THING_NONE_PC34;
        ++state->f0334CloseChestCount;
    }
}

const DM1_V1_InventoryChampionSwitchHandCarryEvidencePc34*
DM1_V1_InventoryChampionSwitchHandCarry_EvidencePc34(void)
{
    return &s_evidence;
}

void DM1_V1_InventoryChampionSwitchHandCarry_InitPc34(
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state,
    int openInventoryChampionIndex,
    int openChestThing)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->championCount = DM1_V1_ICSWHC_MAX_CHAMPIONS_PC34;
    for (i = 0; i < DM1_V1_ICSWHC_MAX_CHAMPIONS_PC34; ++i) {
        state->championHealth[i] = 100;
    }
    if (openInventoryChampionIndex >= 0 &&
        openInventoryChampionIndex < DM1_V1_ICSWHC_MAX_CHAMPIONS_PC34) {
        state->g0423InventoryChampionOrdinal =
            openInventoryChampionIndex + 1;
    }
    state->g4055LeaderHandThing = DM1_V1_ICSWHC_LEADER_HAND_THING_PC34;
    state->leaderHandWeight = 7;
    state->g0426OpenChestThing = openChestThing != 0 ?
        openChestThing : DM1_V1_ICSWHC_THING_NONE_PC34;
    for (i = 0; i < DM1_V1_ICSWHC_CHEST_SLOT_COUNT_PC34; ++i) {
        state->g0425ChestSlots[i] = DM1_V1_ICSWHC_THING_NONE_PC34;
        state->f0334ClosedTypes[i] = DM1_V1_ICSWHC_THING_NONE_PC34;
    }
    if (state->g0426OpenChestThing != DM1_V1_ICSWHC_THING_NONE_PC34) {
        state->g0425ChestSlots[0] =
            DM1_V1_ICSWHC_CHEST_ITEM_FIRST_PC34;
        state->g0425ChestSlots[2] =
            DM1_V1_ICSWHC_CHEST_ITEM_THIRD_PC34;
        state->g0425ChestSlots[7] =
            DM1_V1_ICSWHC_CHEST_ITEM_EIGHTH_PC34;
    }
}

int DM1_V1_InventoryChampionSwitchHandCarry_OpenPc34(
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34* state,
    int requestedChampionIndex,
    DM1_V1_InventoryChampionSwitchHandCarryResultPc34* outResult)
{
    DM1_V1_InventoryChampionSwitchHandCarryStatePc34 before;
    int targetChampionIndex = requestedChampionIndex;
    int oldOrdinal;

    if (!state || !outResult) {
        return 0;
    }
    before = *state;
    capture_before(state, requestedChampionIndex, outResult);

    if (targetChampionIndex != DM1_V1_ICSWHC_CLOSE_INVENTORY_PC34 &&
        (targetChampionIndex < 0 || targetChampionIndex >= state->championCount ||
         state->championHealth[targetChampionIndex] <= 0)) {
        outResult->rejectedDeadChampion = 1;
        capture_after(state, &before, outResult);
        return 0;
    }
    if (state->pressingMouthOrEye) {
        outResult->rejectedMouthEyePress = 1;
        capture_after(state, &before, outResult);
        return 0;
    }

    state->stopWaitingForPlayerInput = 1;
    oldOrdinal = state->g0423InventoryChampionOrdinal;
    if (targetChampionIndex + 1 == oldOrdinal) {
        targetChampionIndex = DM1_V1_ICSWHC_CLOSE_INVENTORY_PC34;
    }

    if (oldOrdinal != DM1_V1_ICSWHC_NO_INVENTORY_ORDINAL_PC34 &&
        targetChampionIndex != DM1_V1_ICSWHC_SPECIAL_INVENTORY_PC34) {
        state->g0423InventoryChampionOrdinal =
            DM1_V1_ICSWHC_NO_INVENTORY_ORDINAL_PC34;
        close_open_chest_if_any(state);
        ++state->oldStatusDrawCount;
        if (targetChampionIndex == DM1_V1_ICSWHC_CLOSE_INVENTORY_PC34) {
            ++state->refreshMousePointerInMainLoop;
            ++state->movementArrowsDrawCount;
            state->secondaryInputInventory = 0;
            state->secondaryInputMovement = 1;
            ++state->discardInputCount;
            ++state->floorCeilingDrawCount;
            outResult->acceptedClose = 1;
            capture_after(state, &before, outResult);
            return 1;
        }
    }

    state->g0423InventoryChampionOrdinal = targetChampionIndex + 1;
    state->slotDrawCount += DM1_V1_ICSWHC_SLOT_DRAW_COUNT_PC34;
    ++state->newStatusDrawCount;
    ++state->mousePointerBitmapUpdated;
    state->secondaryInputInventory = 1;
    state->secondaryInputMovement = 0;
    ++state->discardInputCount;
    outResult->acceptedSwitch = 1;
    capture_after(state, &before, outResult);
    return 1;
}
