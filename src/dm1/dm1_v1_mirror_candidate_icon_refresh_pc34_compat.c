#include "dm1_v1_mirror_candidate_icon_refresh_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract-only slice:
 * CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182,
 * CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262,
 * CHAMPION.C F0285_CHAMPION_GetIndexInCell:180-210,
 * COMMAND.C F0380 command candidate gates:2158-2182 and 2302-2311,
 * DEFS.H F0295/F0296/F0297/F0298 prototypes:7915-7931.
 */

enum {
    kCandidateOrdinal = 3,
    kInventoryOrdinal = 2,
    kPartyChampionCount = 3,
    kC040PanelOpen = 1,
    kLeaderIndex = 0,
    kLeaderHandThing = 0x0420,
    kLeaderHandCurrentIcon = 12,
    kLeaderHandObjectIcon = 148,
    kMousePointerPreviouslyHidden = 1,
    kSlotCurrentLow = 7,
    kSlotObjectPotion = 149,
    kSlotCurrentPotion = 148,
    kSlotObjectEmptyFlask = 195,
    kSlotCurrentEmptyFlask = 195,
    kImmutableIcon = 40
};

static const Dm1V1MirrorCandidateIconRefreshEvidencePc34Compat
    s_evidence = {
        "CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
        "CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262",
        "CHAMPION.C F0285_CHAMPION_GetIndexInCell:180-210",
        "COMMAND.C F0380 candidate gates:2158-2182,2302-2311",
        "DEFS.H F0295/F0296/F0297/F0298 prototypes:7915-7931",
        "contract-only in-memory icon slotbox snapshot; no real party, chest, "
        "inventory, savegame, or asset data is loaded or claimed",
        "disjoint from click_cancel_front_cell, close_button_pc34, "
        "reincarnate ProcessPanelCommand, resurrect ProcessResurrect, "
        "resurrect ProcessStatusBoxClick, champion mirror ProcessStatusBoxClick"
    };

static int mutable_icon_index(int iconIndex)
{
    return (iconIndex >= 0 && iconIndex < 32) ||
           (iconIndex >= 148 && iconIndex <= 163) ||
           iconIndex == 195;
}

static void slot_init(Dm1V1MirrorCandidateIconSlotPc34Compat *slot,
                      int currentIcon,
                      int objectIcon)
{
    slot->currentIcon = currentIcon;
    slot->objectIcon = objectIcon;
    slot->changed = 0;
}

static void fill_fixture(Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state)
{
    int championIndex;
    int handIndex;
    int slotIndex;

    memset(state, 0, sizeof(*state));
    state->candidateChampionOrdinal = kCandidateOrdinal;
    state->partyChampionCount = kPartyChampionCount;
    state->c040PanelOpen = kC040PanelOpen;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThingOrdinal = kLeaderHandThing;
    state->leaderHandCurrentIcon = kLeaderHandCurrentIcon;
    state->leaderHandObjectIcon = kLeaderHandObjectIcon;
    state->leaderHandPointerIcon = kLeaderHandCurrentIcon;
    state->mousePointerHiddenForChangedIcon = kMousePointerPreviouslyHidden;

    for (championIndex = 0; championIndex <
             DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        for (handIndex = 0; handIndex <
                 DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_HAND_SLOTS_PC34_COMPAT;
             ++handIndex) {
            slot_init(&state->partyHandSlots[championIndex][handIndex],
                      kSlotCurrentLow + championIndex + handIndex,
                      kSlotObjectPotion + championIndex + handIndex);
        }
    }
    for (slotIndex = 0; slotIndex <
             DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_INVENTORY_SLOTS_PC34_COMPAT;
         ++slotIndex) {
        slot_init(&state->inventorySlots[slotIndex],
                  slotIndex == 1 ? kSlotCurrentPotion : kSlotCurrentLow,
                  slotIndex == 1 ? kSlotObjectEmptyFlask :
                      kSlotObjectPotion + slotIndex);
    }
    for (slotIndex = 0; slotIndex <
             DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHEST_SLOTS_PC34_COMPAT;
         ++slotIndex) {
        slot_init(&state->chestSlots[slotIndex],
                  slotIndex == 0 ? kSlotCurrentEmptyFlask : kSlotCurrentLow,
                  slotIndex == 0 ? kSlotCurrentEmptyFlask :
                      kSlotObjectPotion + slotIndex);
    }
}

void DM1_V1_MirrorCandidateIconRefresh_InitSuppressedPc34Compat(
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    fill_fixture(state);
    state->inventoryChampionOrdinal = 0u;
}

void DM1_V1_MirrorCandidateIconRefresh_InitInventoryOpenPc34Compat(
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    fill_fixture(state);
    state->inventoryChampionOrdinal = kInventoryOrdinal;
}

static void result_begin(
    const Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state,
    Dm1V1MirrorCandidateIconRefreshResultPc34Compat *result)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->contractOnly = 1;
    result->mutableIconLowRange = mutable_icon_index(0) &&
        mutable_icon_index(31);
    result->mutableIconWeaponBoundaryRejected = !mutable_icon_index(32);
    result->mutableIconPotionRange = mutable_icon_index(148) &&
        mutable_icon_index(163);
    result->mutableIconEmptyFlask = mutable_icon_index(195);
    result->immutableIconRejected = !mutable_icon_index(kImmutableIcon);
    if (!state) {
        return;
    }
    result->candidateOrdinalBefore = (int)state->candidateChampionOrdinal;
    result->candidateOrdinalAfter = result->candidateOrdinalBefore;
    result->inventoryOrdinalBefore = (int)state->inventoryChampionOrdinal;
    result->inventoryOrdinalAfter = result->inventoryOrdinalBefore;
    result->c040PanelBefore = state->c040PanelOpen;
    result->c040PanelAfter = result->c040PanelBefore;
    result->leaderHandCurrentIconBefore = state->leaderHandCurrentIcon;
    result->leaderHandCurrentIconAfter = state->leaderHandCurrentIcon;
    result->leaderHandPointerIconBefore = state->leaderHandPointerIcon;
    result->leaderHandPointerIconAfter = state->leaderHandPointerIcon;
    result->leaderHandNameDrawCountBefore = state->leaderHandNameDrawCount;
    result->leaderHandNameDrawCountAfter = state->leaderHandNameDrawCount;
    result->mousePointerHiddenBefore = state->mousePointerHiddenForChangedIcon;
    result->mousePointerHiddenAfter = state->mousePointerHiddenForChangedIcon;
    result->mouseScreenUpdatePairsBefore = state->mouseScreenUpdatePairs;
    result->mouseScreenUpdatePairsAfter = state->mouseScreenUpdatePairs;
    result->partyStatusSlotRefreshCountBefore =
        state->partyStatusSlotRefreshCount;
    result->partyStatusSlotRefreshCountAfter =
        state->partyStatusSlotRefreshCount;
    result->partyActionIconDrawCountBefore = state->partyActionIconDrawCount;
    result->partyActionIconDrawCountAfter = state->partyActionIconDrawCount;
    result->inventorySlotRefreshCountBefore = state->inventorySlotRefreshCount;
    result->inventorySlotRefreshCountAfter = state->inventorySlotRefreshCount;
    result->chestSlotRefreshCountBefore = state->chestSlotRefreshCount;
    result->chestSlotRefreshCountAfter = state->chestSlotRefreshCount;
    result->viewportDrawCountBefore = state->viewportDrawCount;
    result->viewportDrawCountAfter = state->viewportDrawCount;
    result->earlyReturnCountBefore = state->earlyReturnCount;
    result->earlyReturnCountAfter = state->earlyReturnCount;
    result->partyLoopVisitsBefore = state->partyLoopVisits;
    result->partyLoopVisitsAfter = state->partyLoopVisits;
    result->inventoryLoopVisitsBefore = state->inventoryLoopVisits;
    result->inventoryLoopVisitsAfter = state->inventoryLoopVisits;
    result->chestLoopVisitsBefore = state->chestLoopVisits;
    result->chestLoopVisitsAfter = state->chestLoopVisits;
    result->candidateOrdinalClearedCountBefore =
        state->candidateOrdinalClearedCount;
    result->candidateOrdinalClearedCountAfter =
        state->candidateOrdinalClearedCount;
    result->panelClearedCountBefore = state->panelClearedCount;
    result->panelClearedCountAfter = state->panelClearedCount;
    result->commandQueueMutationCountBefore = state->commandQueueMutationCount;
    result->commandQueueMutationCountAfter = state->commandQueueMutationCount;
}

static void result_finish(
    const Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state,
    Dm1V1MirrorCandidateIconRefreshResultPc34Compat *result)
{
    if (!state || !result) {
        return;
    }
    result->candidateOrdinalAfter = (int)state->candidateChampionOrdinal;
    result->inventoryOrdinalAfter = (int)state->inventoryChampionOrdinal;
    result->c040PanelAfter = state->c040PanelOpen;
    result->leaderHandCurrentIconAfter = state->leaderHandCurrentIcon;
    result->leaderHandPointerIconAfter = state->leaderHandPointerIcon;
    result->leaderHandNameDrawCountAfter = state->leaderHandNameDrawCount;
    result->mousePointerHiddenAfter = state->mousePointerHiddenForChangedIcon;
    result->mouseScreenUpdatePairsAfter = state->mouseScreenUpdatePairs;
    result->partyStatusSlotRefreshCountAfter =
        state->partyStatusSlotRefreshCount;
    result->partyActionIconDrawCountAfter = state->partyActionIconDrawCount;
    result->inventorySlotRefreshCountAfter = state->inventorySlotRefreshCount;
    result->chestSlotRefreshCountAfter = state->chestSlotRefreshCount;
    result->viewportDrawCountAfter = state->viewportDrawCount;
    result->earlyReturnCountAfter = state->earlyReturnCount;
    result->partyLoopVisitsAfter = state->partyLoopVisits;
    result->inventoryLoopVisitsAfter = state->inventoryLoopVisits;
    result->chestLoopVisitsAfter = state->chestLoopVisits;
    result->candidateOrdinalClearedCountAfter =
        state->candidateOrdinalClearedCount;
    result->panelClearedCountAfter = state->panelClearedCount;
    result->commandQueueMutationCountAfter = state->commandQueueMutationCount;
}

static int refresh_slot_icon(
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state,
    Dm1V1MirrorCandidateIconSlotPc34Compat *slot,
    int slotBoxIndex)
{
    if (!mutable_icon_index(slot->currentIcon) ||
        slot->currentIcon == slot->objectIcon) {
        return 0;
    }
    if (slotBoxIndex < 8 && !state->mousePointerHiddenForChangedIcon) {
        state->mousePointerHiddenForChangedIcon = 1;
        ++state->mouseScreenUpdatePairs;
    }
    slot->currentIcon = slot->objectIcon;
    slot->changed = 1;
    return 1;
}

int DM1_V1_MirrorCandidateIconRefresh_DrawChangedObjectIconsPc34Compat(
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat *state,
    Dm1V1MirrorCandidateIconRefreshResultPc34Compat *outResult)
{
    int championIndex;
    int handIndex;
    int slotIndex;
    int drawViewport = 0;

    result_begin(state, outResult);
    if (!state || !outResult) {
        return 0;
    }

    /* ReDMCSB: CHAMDRAW.C F0296:1210-1213 returns before any icon probes
     * when G0299 is set and G0423 inventory ordinal is zero. */
    if (state->candidateChampionOrdinal != 0u &&
        state->inventoryChampionOrdinal == 0u) {
        ++state->earlyReturnCount;
        outResult->suppressedByCandidateWithoutInventory = 1;
        result_finish(state, outResult);
        return 0;
    }

    outResult->processedWithInventoryOpen = state->inventoryChampionOrdinal != 0u;
    state->mousePointerHiddenForChangedIcon = 0;

    if (mutable_icon_index(state->leaderHandCurrentIcon) &&
        state->leaderHandCurrentIcon != state->leaderHandObjectIcon) {
        state->leaderHandCurrentIcon = state->leaderHandObjectIcon;
        state->leaderHandPointerIcon = state->leaderHandObjectIcon;
        ++state->leaderHandIconRefreshCount;
        ++state->leaderHandNameDrawCount;
        state->mousePointerHiddenForChangedIcon = 1;
        ++state->mouseScreenUpdatePairs;
    }

    for (championIndex = 0; championIndex < state->partyChampionCount;
         ++championIndex) {
        if ((unsigned int)(championIndex + 1) ==
            state->inventoryChampionOrdinal) {
            continue;
        }
        for (handIndex = 0; handIndex <
                 DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_HAND_SLOTS_PC34_COMPAT;
             ++handIndex) {
            ++state->partyLoopVisits;
            if (refresh_slot_icon(state,
                                  &state->partyHandSlots[championIndex][handIndex],
                                  championIndex * 2 + handIndex)) {
                ++state->partyStatusSlotRefreshCount;
                if (handIndex == 1) {
                    ++state->partyActionIconDrawCount;
                }
            }
        }
    }

    if (state->inventoryChampionOrdinal != 0u) {
        for (slotIndex = 0; slotIndex <
                 DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_INVENTORY_SLOTS_PC34_COMPAT;
             ++slotIndex) {
            ++state->inventoryLoopVisits;
            if (refresh_slot_icon(state,
                                  &state->inventorySlots[slotIndex],
                                  8 + slotIndex)) {
                ++state->inventorySlotRefreshCount;
                drawViewport = 1;
                if (slotIndex == 1) {
                    ++state->partyActionIconDrawCount;
                }
            }
        }
        for (slotIndex = 0; slotIndex <
                 DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHEST_SLOTS_PC34_COMPAT;
             ++slotIndex) {
            ++state->chestLoopVisits;
            if (refresh_slot_icon(state,
                                  &state->chestSlots[slotIndex],
                                  38 + slotIndex)) {
                ++state->chestSlotRefreshCount;
                drawViewport = 1;
            }
        }
        if (drawViewport) {
            ++state->viewportDrawCount;
        }
    }

    result_finish(state, outResult);
    return 1;
}

Dm1V1MirrorCandidateIconRefreshProbePc34Compat
DM1_V1_MirrorCandidateIconRefresh_ProbePc34Compat(void)
{
    Dm1V1MirrorCandidateIconRefreshProbePc34Compat probe;
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat state;

    memset(&probe, 0, sizeof(probe));
    probe.evidence = &s_evidence;
    DM1_V1_MirrorCandidateIconRefresh_InitSuppressedPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateIconRefresh_DrawChangedObjectIconsPc34Compat(
        &state,
        &probe.suppressed);
    DM1_V1_MirrorCandidateIconRefresh_InitInventoryOpenPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateIconRefresh_DrawChangedObjectIconsPc34Compat(
        &state,
        &probe.inventoryOpen);
    return probe;
}

const Dm1V1MirrorCandidateIconRefreshEvidencePc34Compat *
DM1_V1_MirrorCandidateIconRefresh_EvidencePc34Compat(void)
{
    return &s_evidence;
}
