#include "dm1_v1_mirror_candidate_resurrect_cross_candidate_clear_pc34_compat.h"

#include <string.h>

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex <
               DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
}

static unsigned int ordinal_from_index(int championIndex)
{
    return (unsigned int)(championIndex + 1);
}

static void snapshot_step(
    Dm1V1MirrorCandidateRccStepPc34Compat *step,
    const Dm1V1MirrorCandidateRccStatePc34Compat *state,
    int stepId,
    const char *name,
    const char *anchor)
{
    int championIndex;

    if (!step) {
        return;
    }
    memset(step, 0, sizeof(*step));
    step->stepId = stepId;
    step->name = name;
    step->redmcsbAnchor = anchor;
    step->panelOwnerBefore =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    step->panelOwnerAfter =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    if (!state) {
        return;
    }
    step->candidateOrdinalBefore = state->candidateChampionOrdinal;
    step->candidateOrdinalAfter = state->candidateChampionOrdinal;
    step->inventoryOrdinalBefore = state->inventoryChampionOrdinal;
    step->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    step->panelOwnerBefore = state->panel.ownerChampionIndex;
    step->panelOwnerAfter = state->panel.ownerChampionIndex;
    step->panelOpenBefore = state->panel.panelOpen;
    step->panelOpenAfter = state->panel.panelOpen;
    step->leaderIndexBefore = state->leaderIndex;
    step->leaderIndexAfter = state->leaderIndex;
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        step->healthBefore[championIndex] =
            state->champions[championIndex].currentHealth;
        step->healthAfter[championIndex] =
            state->champions[championIndex].currentHealth;
    }
}

static void finish_step(
    Dm1V1MirrorCandidateRccStepPc34Compat *step,
    const Dm1V1MirrorCandidateRccStatePc34Compat *state)
{
    int championIndex;

    if (!step || !state) {
        return;
    }
    step->candidateOrdinalAfter = state->candidateChampionOrdinal;
    step->inventoryOrdinalAfter = state->inventoryChampionOrdinal;
    step->panelOwnerAfter = state->panel.ownerChampionIndex;
    step->panelOpenAfter = state->panel.panelOpen;
    step->leaderIndexAfter = state->leaderIndex;
    step->staleBClosedWithoutResurrect =
        state->stalePanelClosedWithoutResurrect &&
        state->champions[1].resurrectCount == 0 &&
        state->bPanelEverResurrected == 0;
    step->freshAIsCurrentCandidate =
        state->candidateChampionOrdinal == ordinal_from_index(0) &&
        state->candidateOwnerChampionIndex == 0 &&
        state->panel.ownerChampionIndex == 0;
    step->leaderOwnsFreshA =
        state->leaderIndex == 0 &&
        state->leaderOwnedFreshCandidateThroughout;
    step->bStillDead = state->champions[1].currentHealth == 0;
    step->aResurrected = state->champions[0].currentHealth > 0 &&
                         state->champions[0].resurrectCount == 1;
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        step->healthAfter[championIndex] =
            state->champions[championIndex].currentHealth;
    }
}

static void draw_c040_panel(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    int championIndex,
    unsigned int candidateOrdinal)
{
    int slotIndex;

    /* ReDMCSB: PANEL.C F0347:1639-1656 dispatches G0299-owned C040 to
     * F0346; F0346:1619-1635 draws C040 as M568.  F0344/F0345:
     * 1493-1615 are the neighboring food/water panel redraw helpers that
     * must not steal ownership from the active candidate panel. */
    ++state->f0344FoodWaterBarRedrawCount;
    ++state->f0345FoodWaterPanelRedrawCount;
    ++state->f0346DrawC040PanelCount;
    ++state->f0347DrawPanelDispatchCount;
    ++state->panel.drawGeneration;
    state->panel.panelOpen = 1;
    state->panel.panelContent =
        DM1_V1_MIRROR_CANDIDATE_RCC_M568_PANEL_PC34_COMPAT;
    state->panel.panelGraphic =
        DM1_V1_MIRROR_CANDIDATE_RCC_C040_GRAPHIC_PC34_COMPAT;
    state->panel.ownerChampionIndex = championIndex;
    state->panel.candidateOrdinal = candidateOrdinal;
    state->panel.chestFirstSlot =
        DM1_V1_MIRROR_CANDIDATE_RCC_C30_SLOT_CHEST_1_PC34_COMPAT;
    state->panel.chestSlotProbeCount =
        DM1_V1_MIRROR_CANDIDATE_RCC_CHEST_SLOT_COUNT_PC34_COMPAT;
    state->activePanelChampionIndex = championIndex;
    if (valid_champion_index(championIndex)) {
        ++state->champions[championIndex].redrawPanelCount;
    }
    for (slotIndex = 0;
         slotIndex <
             DM1_V1_MIRROR_CANDIDATE_RCC_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        state->g0425ChestSlots[slotIndex] =
            DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    }
}

void DM1_V1_MirrorCandidateRcc_InitPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state)
{
    int championIndex;
    int slotIndex;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->partyChampionCount =
        DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
    state->leaderIndex = 0;
    state->leaderEmptyHanded = 1;
    state->candidateOwnerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->activePanelChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->stalePanelOwnerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->g0426OpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->panel.ownerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    for (slotIndex = 0;
         slotIndex <
             DM1_V1_MIRROR_CANDIDATE_RCC_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        state->g0425ChestSlots[slotIndex] =
            DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    }
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        state->champions[championIndex].present = 1;
        state->champions[championIndex].currentHealth = 40 + championIndex;
        state->champions[championIndex].maximumHealth = 80 + championIndex;
        state->champions[championIndex].cell = championIndex;
        state->champions[championIndex].direction = 0;
        state->champions[championIndex].portraitOrdinal = 20 + championIndex;
        for (slotIndex = 0;
             slotIndex <
                 DM1_V1_MIRROR_CANDIDATE_RCC_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            state->champions[championIndex].slots[slotIndex] =
                100 * championIndex + slotIndex;
        }
    }
}

int DM1_V1_MirrorCandidateRcc_SeedStaleBPanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep)
{
    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RCC_STEP_SEED_STALE_B_PANEL_PC34_COMPAT,
        "STEP_SEED_STALE_B_PANEL",
        "REVIVE.C F0280:124-132; PANEL.C F0346:1619-1635");
    if (!state || state->partyChampionCount < 2) {
        return 0;
    }

    /* The stale panel is deliberately synthetic: it is on screen for B, but
     * must be cleared before the fresh F0280 append for A becomes current. */
    state->champions[1].currentHealth = 0;
    state->champions[1].deathCount = 1;
    state->stalePanelOwnerChampionIndex = 1;
    state->staleCandidateOrdinal = ordinal_from_index(1);
    state->candidateChampionOrdinal = state->staleCandidateOrdinal;
    state->candidateOwnerChampionIndex = 1;
    state->inventoryChampionOrdinal = state->staleCandidateOrdinal;
    draw_c040_panel(state, 1, state->staleCandidateOrdinal);
    if (outStep) {
        outStep->routeF0280 = 1;
        outStep->routeF0344 = 1;
        outStep->routeF0345 = 1;
        outStep->routeF0346 = 1;
        outStep->routeF0347 = 1;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRcc_CloseStaleBPanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep)
{
    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RCC_STEP_CLOSE_STALE_B_PANEL_PC34_COMPAT,
        "STEP_CLOSE_STALE_B_PANEL",
        "REVIVE.C F0282:744-758 cancel; CHEST.C F0334:113-132");
    if (!state || state->panel.ownerChampionIndex != 1 ||
        state->panel.candidateOrdinal != ordinal_from_index(1)) {
        return 0;
    }

    /* ReDMCSB: REVIVE.C F0282:744-758 clears G0299 and decrements the
     * candidate slot on cancel.  This regression uses that rollback shape to
     * close stale B before A's fresh candidate can be resurrected. */
    ++state->f0282CancelRouteCount;
    ++state->f0334CloseChestRewriteCount;
    state->panel.panelOpen = 0;
    state->panel.closedWithoutResurrect = 1;
    ++state->panel.closeGeneration;
    state->panel.ownerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->panel.candidateOrdinal = 0;
    state->activePanelChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->candidateChampionOrdinal = 0;
    state->candidateOwnerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->inventoryChampionOrdinal = 0;
    state->staleCandidateFreed = 1;
    state->stalePanelClosedWithoutResurrect = 1;
    ++state->champions[1].candidateCloseCount;
    if (outStep) {
        outStep->routeF0282 = 1;
        outStep->routeF0334 = 1;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRcc_QueueFreshAPanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep)
{
    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RCC_STEP_QUEUE_FRESH_A_PANEL_PC34_COMPAT,
        "STEP_QUEUE_FRESH_A_PANEL",
        "REVIVE.C F0280:124-132; PANEL.C F0347:1639-1656");
    if (!state || !state->leaderEmptyHanded ||
        !state->stalePanelClosedWithoutResurrect ||
        state->champions[0].currentHealth == 0) {
        return 0;
    }

    /* ReDMCSB: REVIVE.C F0280:124-132 admits a candidate only with an empty
     * leader hand and room in M516.  F0302:677-684 blocks status-hand
     * routing while G0299 is live, preserving this fresh candidate owner. */
    state->champions[0].currentHealth = 0;
    state->champions[0].deathCount = 1;
    ++state->f0280CandidateSetCount;
    ++state->f0302SlotWritebackGuardCount;
    state->candidateChampionOrdinal = ordinal_from_index(0);
    state->candidateOwnerChampionIndex = 0;
    state->inventoryChampionOrdinal = ordinal_from_index(0);
    state->freshCandidateQueuedAfterStaleClose = 1;
    state->leaderOwnedFreshCandidateThroughout = state->leaderIndex == 0;
    draw_c040_panel(state, 0, state->candidateChampionOrdinal);
    if (outStep) {
        outStep->routeF0280 = 1;
        outStep->routeF0302 = 1;
        outStep->routeF0344 = 1;
        outStep->routeF0345 = 1;
        outStep->routeF0346 = 1;
        outStep->routeF0347 = 1;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRcc_ResurrectAPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep)
{
    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RCC_STEP_RESURRECT_A_PC34_COMPAT,
        "STEP_RESURRECT_A",
        "COMMAND.C F0359:1985-1990; REVIVE.C F0282:785-806");
    if (!state || !state->leaderEmptyHanded ||
        state->candidateChampionOrdinal != ordinal_from_index(0) ||
        state->candidateOwnerChampionIndex != 0 ||
        state->panel.ownerChampionIndex != 0) {
        return 0;
    }

    /* ReDMCSB: COMMAND.C F0359:1985-1990 dispatches C160 from M568 only
     * when the leader hand is empty.  REVIVE.C F0282:785-806 clears G0299,
     * unlinks old possessions, and commits the candidate champion in place. */
    ++state->f0359C040DispatchCount;
    ++state->f0282ResurrectRouteCount;
    ++state->f0297PutLeaderHandCount;
    ++state->f0298RemoveLeaderHandCount;
    ++state->f0300SlotClearCount;
    ++state->f0301SlotWriteCount;
    ++state->f0302SlotWritebackGuardCount;
    state->champions[0].currentHealth = state->champions[0].maximumHealth / 2;
    ++state->champions[0].resurrectCount;
    state->candidateChampionOrdinal = 0;
    state->candidateOwnerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->inventoryChampionOrdinal = ordinal_from_index(0);
    state->panel.panelOpen = 0;
    state->panel.closedWithoutResurrect = 0;
    ++state->panel.closeGeneration;
    state->activePanelChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    if (outStep) {
        outStep->routeF0282 = 1;
        outStep->routeF0297 = 1;
        outStep->routeF0298 = 1;
        outStep->routeF0300 = 1;
        outStep->routeF0301 = 1;
        outStep->routeF0302 = 1;
        outStep->routeF0359 = 1;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRcc_SettlePanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep)
{
    int championIndex;

    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RCC_STEP_SETTLE_PANEL_PC34_COMPAT,
        "STEP_SETTLE_PANEL",
        "CHEST.C F0333:30-67; CHAMPION.C F0284:93-131");
    if (!state || state->champions[0].resurrectCount != 1 ||
        !state->stalePanelClosedWithoutResurrect) {
        return 0;
    }

    ++state->f0333OpenChestRouteCount;
    ++state->f0334CloseChestRewriteCount;
    state->g0426OpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->panel.ownerChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT;
    state->panel.candidateOrdinal = 0;
    state->panel.panelOpen = 0;
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        ++state->champions[championIndex].redrawStateCount;
    }
    if (outStep) {
        outStep->routeF0333 = 1;
        outStep->routeF0334 = 1;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRcc_DriveRegressionPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *steps,
    int stepCapacity)
{
    if (!state || !steps || stepCapacity < 5) {
        return 0;
    }
    DM1_V1_MirrorCandidateRcc_InitPc34Compat(state);
    if (!DM1_V1_MirrorCandidateRcc_SeedStaleBPanelPc34Compat(
            state, &steps[0])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcc_CloseStaleBPanelPc34Compat(
            state, &steps[1])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcc_QueueFreshAPanelPc34Compat(
            state, &steps[2])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcc_ResurrectAPc34Compat(
            state, &steps[3])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRcc_SettlePanelPc34Compat(
            state, &steps[4])) {
        return 0;
    }
    return 5;
}

const char *
DM1_V1_MirrorCandidateRcc_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB anchors: COMMAND.C F0359:1985-1990 empty-hand M568/C040 "
           "dispatch; REVIVE.C F0280:124-132 candidate admission; "
           "REVIVE.C F0282:744-806 cancel/confirm candidate clear and "
           "resurrect commit; CHAMPION.C F0284:93-131 party direction "
           "preserves champion identity; CHAMPION.C F0297:243-298, "
           "F0298:270-298, F0300:511-515, F0301:606-614, and "
           "F0302:662-714 leader-hand/slot writeback guards; CHEST.C "
           "F0333:30-67 and F0334:113-132 panel/chest close rewrite; "
           "PANEL.C F0344:1493-1561, F0345:1563-1615, F0346:1619-1635, "
           "and F0347:1639-1656 panel redraw; DEFS.H:338/2088/2200/3002 "
           "C160/C10/C040/M568 plus C30/G0425/G0426/M070/M516 symbols.";
}
