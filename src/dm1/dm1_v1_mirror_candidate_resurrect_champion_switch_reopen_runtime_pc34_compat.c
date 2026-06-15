#include "dm1_v1_mirror_candidate_resurrect_champion_switch_reopen_runtime_pc34_compat.h"

#include <string.h>

enum {
    STEP_SELECT_CANDIDATE_PC34_COMPAT = 1,
    STEP_FIRST_RESURRECT_HAND_ICON_PC34_COMPAT = 2,
    STEP_INVENTORY_CHAMPION_SWITCH_PC34_COMPAT = 3,
    STEP_REOPEN_DIFFERENT_CHAMPION_PC34_COMPAT = 4
};

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex <
               DM1_V1_MIRROR_CANDIDATE_RCSR_CHAMPION_COUNT_PC34_COMPAT;
}

static unsigned int ordinal_from_index(int championIndex)
{
    return (unsigned int)(championIndex + 1);
}

static int normalize_direction(int value)
{
    int normalized = value % 4;

    if (normalized < 0) {
        normalized += 4;
    }
    return normalized;
}

static void snapshot_step(
    Dm1V1MirrorCandidateRcsrStepPc34Compat *step,
    const Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int stepId,
    const char *name,
    const char *anchor)
{
    if (!step) {
        return;
    }
    memset(step, 0, sizeof(*step));
    step->stepId = stepId;
    step->name = name;
    step->redmcsbAnchor = anchor;
    if (!state) {
        step->panelChampionBefore =
            DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT;
        step->panelChampionAfter =
            DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT;
        return;
    }
    step->candidateOrdinalBefore = state->candidateChampionOrdinal;
    step->candidateOrdinalAfter = state->candidateChampionOrdinal;
    step->inventoryOrdinalBefore = state->inventoryChampionOrdinal;
    step->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    step->panelChampionBefore = state->activePanelChampionIndex;
    step->panelChampionAfter = state->activePanelChampionIndex;
}

static Dm1V1MirrorCandidateRcsrPanelChromePc34Compat make_c040_chrome(void)
{
    Dm1V1MirrorCandidateRcsrPanelChromePc34Compat chrome;

    memset(&chrome, 0, sizeof(chrome));
    chrome.panelContent = DM1_V1_MIRROR_CANDIDATE_RCSR_M568_PANEL_PC34_COMPAT;
    chrome.panelGraphic = DM1_V1_MIRROR_CANDIDATE_RCSR_C040_GRAPHIC_PC34_COMPAT;
    chrome.panelBoxLeft = 0;
    chrome.panelBoxTop = 0;
    chrome.panelBoxRight = 223;
    chrome.panelBoxBottom = 135;
    chrome.panelByteWidth = 72;
    chrome.transparentColor = 6;
    chrome.chestFirstSlot =
        DM1_V1_MIRROR_CANDIDATE_RCSR_C30_SLOT_CHEST_1_PC34_COMPAT;
    chrome.chestSlotProbeCount =
        DM1_V1_MIRROR_CANDIDATE_RCSR_CHEST_SLOT_COUNT_PC34_COMPAT;
    return chrome;
}

static int same_chrome(
    const Dm1V1MirrorCandidateRcsrPanelChromePc34Compat *left,
    const Dm1V1MirrorCandidateRcsrPanelChromePc34Compat *right)
{
    return left && right &&
           left->panelContent == right->panelContent &&
           left->panelGraphic == right->panelGraphic &&
           left->panelBoxLeft == right->panelBoxLeft &&
           left->panelBoxTop == right->panelBoxTop &&
           left->panelBoxRight == right->panelBoxRight &&
           left->panelBoxBottom == right->panelBoxBottom &&
           left->panelByteWidth == right->panelByteWidth &&
           left->transparentColor == right->transparentColor &&
           left->chestFirstSlot == right->chestFirstSlot &&
           left->chestSlotProbeCount == right->chestSlotProbeCount;
}

static void draw_all_champion_states(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state)
{
    int championIndex;

    ++state->f0293DrawAllChampionStatesCount;
    for (championIndex = 0; championIndex < state->partyChampionCount;
         ++championIndex) {
        ++state->champions[championIndex].redrawStateCount;
    }
}

static void draw_panel_chrome(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    int reopened)
{
    int slotIndex;

    ++state->f0346DrawC040PanelCount;
    if (reopened) {
        state->reopenedPanelChrome = make_c040_chrome();
    } else {
        state->firstPanelChrome = make_c040_chrome();
    }
    if (valid_champion_index(championIndex)) {
        ++state->champions[championIndex].redrawPanelChromeCount;
        for (slotIndex = 0;
             slotIndex < DM1_V1_MIRROR_CANDIDATE_RCSR_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            ++state->champions[championIndex].redrawSlotCount;
            ++state->f0291DrawSlotCount;
        }
    }
    ++state->f0296DrawChangedObjectIconsCount;
}

void DM1_V1_MirrorCandidateRcsr_InitPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state)
{
    int championIndex;
    int slotIndex;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->partyChampionCount = 3;
    state->partyDirection = 0;
    state->leaderEmptyHanded = 1;
    state->activePanelChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT;
    state->originalCandidateChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT;
    state->leaderIndex = 0;
    state->g0426OpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT;
    for (slotIndex = 0;
         slotIndex < DM1_V1_MIRROR_CANDIDATE_RCSR_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        state->g0425ChestSlots[slotIndex] =
            DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT;
    }
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RCSR_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        state->champions[championIndex].present = championIndex < 3;
        state->champions[championIndex].currentHealth =
            championIndex == 0 ? 0 : 40 + championIndex;
        state->champions[championIndex].cell = championIndex;
        state->champions[championIndex].direction = 0;
        state->champions[championIndex].portraitOrdinal = 10 + championIndex;
        for (slotIndex = 0;
             slotIndex < DM1_V1_MIRROR_CANDIDATE_RCSR_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            state->champions[championIndex].slots[slotIndex] =
                100 * championIndex + slotIndex;
        }
    }
}

int DM1_V1_MirrorCandidateRcsr_SelectCandidatePc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *outStep)
{
    snapshot_step(outStep, state, STEP_SELECT_CANDIDATE_PC34_COMPAT,
                  "select C040 candidate champion 0",
                  "REVIVE.C F0280:124-132");
    if (!state || !valid_champion_index(championIndex) ||
        championIndex >= state->partyChampionCount) {
        return 0;
    }
    state->candidateChampionOrdinal = ordinal_from_index(championIndex);
    state->inventoryChampionOrdinal = ordinal_from_index(championIndex);
    state->activePanelChampionIndex = championIndex;
    state->originalCandidateChampionIndex = championIndex;
    state->originalCandidateChampionOrdinal =
        state->candidateChampionOrdinal;
    ++state->f0280CandidateSetCount;
    draw_panel_chrome(state, championIndex, 0);
    draw_all_champion_states(state);
    if (outStep) {
        outStep->candidateOrdinalAfter = state->candidateChampionOrdinal;
        outStep->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
        outStep->panelChampionAfter = state->activePanelChampionIndex;
        outStep->routeF0293 = 1;
        outStep->panelChromeStable = 1;
    }
    return 1;
}

int DM1_V1_MirrorCandidateRcsr_ClickResurrectHandIconPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *outStep)
{
    int reopened;

    snapshot_step(outStep, state,
                  state && championIndex ==
                              state->originalCandidateChampionIndex
                      ? STEP_FIRST_RESURRECT_HAND_ICON_PC34_COMPAT
                      : STEP_REOPEN_DIFFERENT_CHAMPION_PC34_COMPAT,
                  "resurrect hand icon C040 route",
                  "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    if (!state || !valid_champion_index(championIndex) ||
        championIndex >= state->partyChampionCount || !state->leaderEmptyHanded) {
        return 0;
    }

    reopened = championIndex != state->originalCandidateChampionIndex;
    ++state->resurrectHandIconClickCount;
    ++state->f0359C040DispatchCount;
    ++state->f0282RouteCount;
    ++state->f0282ClearSkippedForHandIconCount;
    state->candidateChampionOrdinal = ordinal_from_index(championIndex);
    state->inventoryChampionOrdinal = ordinal_from_index(championIndex);
    state->activePanelChampionIndex = championIndex;
    draw_panel_chrome(state, championIndex, reopened);
    draw_all_champion_states(state);
    state->reopenedDifferentChampion =
        championIndex != state->originalCandidateChampionIndex;

    if (outStep) {
        outStep->candidateOrdinalAfter = state->candidateChampionOrdinal;
        outStep->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
        outStep->panelChampionAfter = state->activePanelChampionIndex;
        outStep->routeF0282 = 1;
        outStep->routeF0293 = 1;
        outStep->g0299Preserved =
            outStep->candidateOrdinalBefore == outStep->candidateOrdinalAfter;
        outStep->panelChromeStable =
            same_chrome(&state->firstPanelChrome, &state->reopenedPanelChrome);
    }
    return 1;
}

int DM1_V1_MirrorCandidateRcsr_SwitchInventoryChampionPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    int newPartyDirection,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *outStep)
{
    int championIterator;
    int oldDirection;
    int delta;

    snapshot_step(outStep, state, STEP_INVENTORY_CHAMPION_SWITCH_PC34_COMPAT,
                  "inventory champion switch to champion 2",
                  "PANEL.C F0354:2299-2322; CHAMPION.C F0284:93-131");
    if (!state || !valid_champion_index(championIndex) ||
        championIndex >= state->partyChampionCount) {
        return 0;
    }

    ++state->f0354ChampionSwitchCount;
    ++state->f0355InventoryToggleCount;
    oldDirection = state->partyDirection;
    newPartyDirection = normalize_direction(newPartyDirection);
    if (newPartyDirection != oldDirection) {
        delta = newPartyDirection - oldDirection;
        if (delta < 0) {
            delta += 4;
        }
        for (championIterator = 0;
             championIterator < state->partyChampionCount;
             ++championIterator) {
            state->champions[championIterator].cell =
                normalize_direction(state->champions[championIterator].cell +
                                    delta);
            state->champions[championIterator].direction =
                normalize_direction(
                    state->champions[championIterator].direction + delta);
        }
        state->partyDirection = newPartyDirection;
        ++state->f0284SetPartyDirectionCount;
        ++state->f0296DrawChangedObjectIconsCount;
    }
    state->inventoryChampionOrdinal = ordinal_from_index(championIndex);
    state->preservedAcrossSwitch =
        state->candidateChampionOrdinal ==
            state->originalCandidateChampionOrdinal &&
        state->candidateChampionOrdinal != 0u;
    draw_all_champion_states(state);

    if (outStep) {
        outStep->candidateOrdinalAfter = state->candidateChampionOrdinal;
        outStep->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
        outStep->panelChampionAfter = state->activePanelChampionIndex;
        outStep->routeF0284 = 1;
        outStep->routeF0293 = 1;
        outStep->routeF0354 = 1;
        outStep->g0299Preserved = state->preservedAcrossSwitch;
        outStep->panelChromeStable = 1;
    }
    return 1;
}

int DM1_V1_MirrorCandidateRcsr_DriveRegressionPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *steps,
    int stepCapacity)
{
    if (!state || !steps || stepCapacity < 4) {
        return 0;
    }
    DM1_V1_MirrorCandidateRcsr_InitPc34Compat(state);
    if (!DM1_V1_MirrorCandidateRcsr_SelectCandidatePc34Compat(
            state, 0, &steps[0])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcsr_ClickResurrectHandIconPc34Compat(
            state, 0, &steps[1])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcsr_SwitchInventoryChampionPc34Compat(
            state, 2, 1, &steps[2])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcsr_ClickResurrectHandIconPc34Compat(
            state, 2, &steps[3])) {
        return 0;
    }
    return 4;
}

const char *
DM1_V1_MirrorCandidateRcsr_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB anchors: REVIVE.C F0280:124-132 candidate state set; "
           "REVIVE.C F0282:744-806 candidate state clear, panel redraw; "
           "CHAMDRAW.C F0293:1117-1143 champion-state redraw; "
           "CHAMDRAW.C F0291/F0296:551-552,1249-1252 champion redraw + "
           "panel chrome; CHAMPION.C F0284:93-131 party direction rotate, "
           "leader-hand put; CHAMPION.C F0297:243-298 leader-hand put; "
           "CHAMPION.C F0302:662-714 occupied-slot click dispatch; "
           "COMMAND.C F0359:1985-1990 M568/C040 dispatch; "
           "PANEL.C F0344/F0345:1895-1944,1946-1999 per-cell highlight + "
           "panel click; PANEL.C F0354:2299-2322 panel scroll/champion-switch; "
           "PANEL.C F0352 pressing-eye route; "
           "DEFS.H C30/G0425/G0426/M070/M516/C040.";
}
