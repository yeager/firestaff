#include "firestaff/dm1/v1/mirror/dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_pc34_compat.h"

#include <string.h>

enum {
    kPanelFoodWater = 1,
    kPanelChest = 6,
    kPanelCandidate = 7,
    kGraphicFoodWaterIconPage = 45,
    kGraphicResurrectPanel = 40,
    kCommandC018Close = 18,
    kCommandC503CloseButton = 503,
    kCommandC144Eye = 144,
    kLeaderIndex = 0,
    kInventoryChampionOrdinal = 1,
    kSourceSlot = 2,
    kOpenChestThing = 0x6420,
    kSourceChestThing = 0x7330,
    kFoodThing = 0x0451,
    kWaterThing = 0x0452,
    kPendingCandidateOrdinal = 4
};

/*
 * ReDMCSB: CHEST.C F0333:30-67/F0334:113-132, CHAMPION.C
 * F0297:243-298/F0298:270-298/F0300:511-515/F0301:606-614/
 * F0302:662-714, PANEL.C F0344:1493-1561/F0345:1563-1616/
 * F0354:2299-2352, REVIVE.C F0280:124-132/F0282:744-806,
 * COMMAND.C F0359:1985-1990, and DEFS.H C30/G0425/G0426/M070/M516/
 * C040/C045/M565/M568 bind this contract-only, asset-free regression.
 */
static const Dm1V1MirrorCandidateC045FoodWaterCloseEvidencePc34Compat
    kEvidence = {
        "CHEST.C F0333:30-67 materializes the chest-bound C30/G0425 slot",
        "CHEST.C F0334:113-132 closes G0426 and rewrites the visible chain",
        "CHAMPION.C F0297:243-298, F0298:270-298, F0300:511-515, "
        "F0301:606-614, F0302:662-714 own leader hand and C30/G0425 slots",
        "PANEL.C F0344:1493-1561 and F0345:1563-1616 draw/read food/water",
        "PANEL.C F0354:2299-2352 closes inventory/panel via C503/C018",
        "REVIVE.C F0280:124-132 only publishes C040 when the hand is empty",
        "REVIVE.C F0282:744-806 is the C040 candidate panel path not entered",
        "COMMAND.C F0359:1985-1990 dispatches M568/C040 only for candidate",
        "DEFS.H:2200 C040, 2205 C045, 3005 M565, 3008 M568, "
        "5876-5881 G0423/G0425/G0426, C30/M070/M516",
        "Disjoint from mirror_candidate_c040_*, resurrect_*, c545_*, "
        "chest_close_*, full_chain_*, no_pending_resurrect_*, "
        "open_then_reselect_*, and scroll_pickup_*: this gate covers "
        "C144 eye -> C045 food/water close from a chest-bound C30 slot."
    };

static const char kSourceEvidence[] =
    "CHEST.C F0333:30-67 materializes G0425 from a chest chain; "
    "CHEST.C F0334:113-132 clears G0426 and relinks only non-empty G0425. "
    "CHAMPION.C F0297:243-298/F0298:270-298/F0300:511-515/"
    "F0301:606-614/F0302:662-714 own leader-hand and C30/G0425 slot "
    "mutation. PANEL.C F0344:1493-1561 and F0345:1563-1616 draw/read "
    "food/water bars after closing the chest; PANEL.C F0354:2299-2352 "
    "handles close-button C503/C018 territory. REVIVE.C F0280:124-132 "
    "and F0282:744-806 are the C040 path, which this C045 close must not "
    "enter. COMMAND.C F0359:1985-1990 dispatches M568/C040 only for the "
    "candidate panel. DEFS.H:2200 C040, 2205 C045, 3005 M565, 3008 M568, "
    "5876-5881 G0423/G0425/G0426 plus C30/M070/M516.";

static uint32_t fnv_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static uint32_t hash_state(
    const Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = fnv_step(hash, (unsigned int)state->openChestThing);
    hash = fnv_step(hash, (unsigned int)state->championSwitchC30Thing);
    hash = fnv_step(hash, (unsigned int)state->panelContent);
    hash = fnv_step(hash, (unsigned int)state->panelGraphic);
    hash = fnv_step(hash, (unsigned int)state->panelOpen);
    hash = fnv_step(hash, (unsigned int)state->c040ResurrectPendingOrdinal);
    hash = fnv_step(hash, (unsigned int)state->c040PanelOpened);
    hash = fnv_step(hash, (unsigned int)state->f0282Entered);
    hash = fnv_step(hash, (unsigned int)state->preservedFoodBeforeClose);
    hash = fnv_step(hash, (unsigned int)state->preservedWaterBeforeClose);
    hash = fnv_step(hash, (unsigned int)state->consumedFoodAfterClose);
    hash = fnv_step(hash, (unsigned int)state->consumedWaterAfterClose);
    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        hash = fnv_step(hash, (unsigned int)state->sourceChestChain[i]);
        hash = fnv_step(hash, (unsigned int)state->g0425Slots[i]);
    }
    return hash;
}

static void seed_slots(uint16_t slots[])
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        slots[i] = (uint16_t)(0x5100u + (unsigned int)i);
    }
    slots[kSourceSlot] = (uint16_t)kFoodThing;
    slots[5] = (uint16_t)kWaterThing;
}

static void copy_slots(uint16_t dst[], const uint16_t src[])
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        dst[i] = src[i];
    }
}

static int all_g0425_clear(
    const Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    int i;

    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        if (state->g0425Slots[i] != DM1_V1_MC_C045_FW_NONE_PC34) {
            return 0;
        }
    }
    return 1;
}

void dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_init_pc34(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->leaderIndex = kLeaderIndex;
    state->sourceChestSlotIndex = kSourceSlot;
    state->openChestThing = (uint16_t)kOpenChestThing;
    state->sourceChestThing = (uint16_t)kSourceChestThing;
    state->foodThing = (uint16_t)kFoodThing;
    state->waterThing = (uint16_t)kWaterThing;
    seed_slots(state->sourceChestChain);
    copy_slots(state->g0425Slots, state->sourceChestChain);
    state->championSwitchC30Thing = DM1_V1_MC_C045_FW_NONE_PC34;
    state->championSwitchSourceSlot = -1;
    state->c018CloseCommand = kCommandC018Close;
    state->panelContent = kPanelChest;
    state->panelGraphic = 25;
    state->panelOpen = 1;
    state->c040ResurrectPendingOrdinal = kPendingCandidateOrdinal;
    state->c040PanelOpened = 0;
    state->f0280CandidateGateChecked = 1;
    state->f0333OpenCount = 1;
    state->preservedFoodBeforeClose = 1536;
    state->preservedWaterBeforeClose = 2048;
    state->consumedFoodAfterClose = state->preservedFoodBeforeClose;
    state->consumedWaterAfterClose = state->preservedWaterBeforeClose;
    for (i = 0; i < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34; ++i) {
        if (state->g0425Slots[i] == 0u) {
            state->g0425Slots[i] = DM1_V1_MC_C045_FW_NONE_PC34;
        }
    }
    state->preHash = hash_state(state);
}

static int ready(
    const Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    return state && state->contractOnly &&
           state->openChestThing != DM1_V1_MC_C045_FW_NONE_PC34 &&
           state->sourceChestSlotIndex >= 0 &&
           state->sourceChestSlotIndex < DM1_V1_MC_C045_FW_SLOT_COUNT_PC34 &&
           state->sourceChestChain[state->sourceChestSlotIndex] ==
               state->foodThing &&
           state->g0425Slots[state->sourceChestSlotIndex] == state->foodThing &&
           state->panelContent == kPanelChest &&
           state->c040ResurrectPendingOrdinal == kPendingCandidateOrdinal &&
           state->c040PanelOpened == 0 && state->f0282Entered == 0;
}

static int open_food_from_c144(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    if (!ready(state)) {
        return 0;
    }
    ++state->c144EyeDispatches;
    ++state->f0334CloseCount;
    ++state->f0300RemoveSlotCount;
    state->championSwitchC30Thing =
        state->g0425Slots[state->sourceChestSlotIndex];
    state->championSwitchSourceSlot = state->sourceChestSlotIndex;
    state->sourceChestChain[state->sourceChestSlotIndex] =
        DM1_V1_MC_C045_FW_NONE_PC34;
    state->g0425Slots[state->sourceChestSlotIndex] =
        DM1_V1_MC_C045_FW_NONE_PC34;
    state->openChestThing = DM1_V1_MC_C045_FW_NONE_PC34;
    state->panelContent = kPanelFoodWater;
    state->panelGraphic = kGraphicFoodWaterIconPage;
    state->panelOpen = 1;
    ++state->f0345FoodWaterDrawCount;
    state->f0344BarReadCount += 2;
    state->openHash = hash_state(state);
    return 1;
}

static int close_c045_panel(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    if (!state || state->panelContent != kPanelFoodWater ||
        state->panelGraphic != kGraphicFoodWaterIconPage ||
        state->championSwitchC30Thing != state->foodThing ||
        state->championSwitchSourceSlot != state->sourceChestSlotIndex) {
        return 0;
    }
    ++state->c503CloseDispatches;
    ++state->f0354CloseCount;
    ++state->f0301AddSlotCount;
    state->sourceChestChain[state->sourceChestSlotIndex] =
        state->championSwitchC30Thing;
    state->g0425Slots[state->sourceChestSlotIndex] =
        state->championSwitchC30Thing;
    state->championSwitchC30Thing = DM1_V1_MC_C045_FW_NONE_PC34;
    state->championSwitchSourceSlot = -1;
    state->closeReleasedC30ToChestChain = 1;
    state->panelOpen = 0;
    state->panelContent = 0;
    state->panelGraphic = 0;
    state->closeHash = hash_state(state);
    return 1;
}

static int consume_after_close(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state)
{
    if (!state || state->panelOpen || !state->closeReleasedC30ToChestChain ||
        state->sourceChestChain[state->sourceChestSlotIndex] !=
            state->foodThing) {
        return 0;
    }
    state->consumedFoodAfterClose = state->preservedFoodBeforeClose - 384;
    state->consumedWaterAfterClose = state->preservedWaterBeforeClose;
    ++state->f0297PutLeaderHandCount;
    ++state->f0298RemoveLeaderHandCount;
    state->consumeHash = hash_state(state);
    return 1;
}

static int guard_rejects(
    const Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *base,
    int kind)
{
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat probe;

    probe = *base;
    if (kind == 0) {
        probe.openChestThing = DM1_V1_MC_C045_FW_NONE_PC34;
    } else if (kind == 1) {
        probe.g0425Slots[probe.sourceChestSlotIndex] =
            DM1_V1_MC_C045_FW_NONE_PC34;
    } else {
        probe.panelContent = kPanelCandidate;
    }
    return !open_food_from_c144(&probe);
}

int dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_run_pc34(
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat *state,
    Dm1V1MirrorCandidateC045FoodWaterCloseResultPc34Compat *result)
{
    Dm1V1MirrorCandidateC045FoodWaterCloseStatePc34Compat base;
    int opened;
    int closed;
    int consumed;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    base = *state;
    opened = open_food_from_c144(state);
    closed = close_c045_panel(state);
    consumed = consume_after_close(state);

    result->openedByC144Eye = opened && state->c144EyeDispatches == 1;
    result->openedC045FoodWater =
        opened && state->openHash != base.preHash &&
        base.c040ResurrectPendingOrdinal == state->c040ResurrectPendingOrdinal;
    result->noC040OnOpen =
        opened && state->c040PanelOpened == 0 &&
        state->panelGraphic != kGraphicResurrectPanel &&
        state->f0359C040DispatchCount == 0;
    result->noF0282OnC045Close = closed && state->f0282Entered == 0;
    result->closeFromChestBoundState =
        closed && base.openChestThing == kOpenChestThing &&
        state->f0334CloseCount == base.f0334CloseCount + 1;
    result->closeCommandC503C018 =
        closed && state->c503CloseDispatches == 1 &&
        state->c018CloseCommand == kCommandC018Close;
    result->releasedC30BackToSourceSlot =
        closed && state->closeReleasedC30ToChestChain &&
        state->championSwitchC30Thing == DM1_V1_MC_C045_FW_NONE_PC34;
    result->sourceChainRestored =
        closed && state->sourceChestChain[kSourceSlot] == state->foodThing;
    result->g0425SlotsClearedAfterClose = closed && !all_g0425_clear(state);
    result->foodWaterPanelStatePreserved =
        closed && state->preservedFoodBeforeClose == 1536 &&
        state->preservedWaterBeforeClose == 2048;
    result->consumptionReadPreservedFood =
        consumed && state->consumedFoodAfterClose == 1152;
    result->consumptionReadPreservedWater =
        consumed && state->consumedWaterAfterClose == 2048;
    result->foodAfterConsumption = state->consumedFoodAfterClose;
    result->waterAfterConsumption = state->consumedWaterAfterClose;
    result->c040PendingStillPending =
        state->c040ResurrectPendingOrdinal == kPendingCandidateOrdinal;
    result->c040PanelStillClosed = state->c040PanelOpened == 0;
    result->chestClosedBeforeFoodWaterDraw =
        state->f0334CloseCount == 1 && state->f0345FoodWaterDrawCount == 1;
    result->championSwitchSlotWasTransient =
        state->f0300RemoveSlotCount == 1 && state->f0301AddSlotCount == 1;
    result->disjointFromC040CandidatePath =
        state->f0282Entered == 0 && state->f0359C040DispatchCount == 0 &&
        state->panelGraphic != kGraphicResurrectPanel;
    result->guardRejectsInvalidChest = guard_rejects(&base, 0);
    result->guardRejectsNoFoodThing = guard_rejects(&base, 1);
    result->guardRejectsWrongPanel = guard_rejects(&base, 2);
    result->restoredThing = state->sourceChestChain[kSourceSlot];
    copy_slots(result->restoredChain, state->sourceChestChain);
    copy_slots(result->g0425AfterClose, state->g0425Slots);
    result->hash = fnv_step(state->consumeHash, (unsigned int)closed);
    result->accepted =
        result->openedByC144Eye && result->openedC045FoodWater &&
        result->noC040OnOpen && result->noF0282OnC045Close &&
        result->closeFromChestBoundState && result->closeCommandC503C018 &&
        result->releasedC30BackToSourceSlot &&
        result->sourceChainRestored && result->foodWaterPanelStatePreserved &&
        result->consumptionReadPreservedFood &&
        result->consumptionReadPreservedWater &&
        result->c040PendingStillPending && result->c040PanelStillClosed &&
        result->chestClosedBeforeFoodWaterDraw &&
        result->championSwitchSlotWasTransient &&
        result->disjointFromC040CandidatePath &&
        result->guardRejectsInvalidChest && result->guardRejectsNoFoodThing &&
        result->guardRejectsWrongPanel && result->hash != 0u;
    return result->accepted;
}

const Dm1V1MirrorCandidateC045FoodWaterCloseEvidencePc34Compat *
dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_evidence_pc34(void)
{
    return &kEvidence;
}

const char *
dm1_v1_mirror_candidate_c045_food_water_close_no_candidate_source_evidence_pc34(void)
{
    return kSourceEvidence;
}
