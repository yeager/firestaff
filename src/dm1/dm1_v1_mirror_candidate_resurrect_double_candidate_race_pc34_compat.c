#include "dm1_v1_mirror_candidate_resurrect_double_candidate_race_pc34_compat.h"

#include <string.h>

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex <
               DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
}

static unsigned int ordinal_from_index(int championIndex)
{
    return (unsigned int)(championIndex + 1);
}

static int index_from_ordinal(unsigned int ordinal)
{
    if (ordinal == 0u) {
        return DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    }
    return (int)ordinal - 1;
}

static int alive_count(const Dm1V1MirrorCandidateRdcrStatePc34Compat *state)
{
    int championIndex;
    int count;

    count = 0;
    if (!state) {
        return 0;
    }
    for (championIndex = 0; championIndex < state->partyChampionCount;
         ++championIndex) {
        if (state->champions[championIndex].present &&
            state->champions[championIndex].currentHealth > 0) {
            ++count;
        }
    }
    return count;
}

static unsigned int pending_ordinal_at(
    const Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    int index)
{
    if (!state || index < 0 || index >= state->pendingCount ||
        index >= DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT) {
        return 0u;
    }
    return state->pendingOrdinals[index];
}

static void snapshot_step(
    Dm1V1MirrorCandidateRdcrStepPc34Compat *step,
    const Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
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
    step->leaderIndexBefore = DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    step->leaderIndexAfter = DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    step->activePanelChampionBefore =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    step->activePanelChampionAfter =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    if (!state) {
        return;
    }
    step->candidateOrdinalBefore = state->candidateChampionOrdinal;
    step->candidateOrdinalAfter = state->candidateChampionOrdinal;
    step->pendingCountBefore = state->pendingCount;
    step->pendingCountAfter = state->pendingCount;
    step->pendingHeadOrdinalBefore = pending_ordinal_at(state, 0);
    step->pendingHeadOrdinalAfter = pending_ordinal_at(state, 0);
    step->pendingTailOrdinalAfter = pending_ordinal_at(state, 1);
    step->leaderIndexBefore = state->leaderIndex;
    step->leaderIndexAfter = state->leaderIndex;
    step->partyChampionCountBefore = state->partyChampionCount;
    step->partyChampionCountAfter = state->partyChampionCount;
    step->activePanelChampionBefore = state->activePanelChampionIndex;
    step->activePanelChampionAfter = state->activePanelChampionIndex;
    step->aliveCountBefore = alive_count(state);
    step->aliveCountAfter = step->aliveCountBefore;
    step->panelDrawGenerationBefore = state->panel.drawGeneration;
    step->panelDrawGenerationAfter = state->panel.drawGeneration;
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        step->healthBefore[championIndex] =
            state->champions[championIndex].currentHealth;
        step->healthAfter[championIndex] =
            state->champions[championIndex].currentHealth;
    }
}

static void finish_step(
    Dm1V1MirrorCandidateRdcrStepPc34Compat *step,
    const Dm1V1MirrorCandidateRdcrStatePc34Compat *state)
{
    int championIndex;
    int seen[
        DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT];

    if (!step || !state) {
        return;
    }
    step->candidateOrdinalAfter = state->candidateChampionOrdinal;
    step->pendingCountAfter = state->pendingCount;
    step->pendingHeadOrdinalAfter = pending_ordinal_at(state, 0);
    step->pendingTailOrdinalAfter = pending_ordinal_at(state, 1);
    step->leaderIndexAfter = state->leaderIndex;
    step->partyChampionCountAfter = state->partyChampionCount;
    step->activePanelChampionAfter = state->activePanelChampionIndex;
    step->aliveCountAfter = alive_count(state);
    step->panelDrawGenerationAfter = state->panel.drawGeneration;
    step->firstResurrectedAlive =
        valid_champion_index(state->firstDeathChampionIndex) &&
        state->champions[state->firstDeathChampionIndex].currentHealth > 0;
    step->secondDeadStillDistinct =
        valid_champion_index(state->secondDeathChampionIndex) &&
        state->champions[state->secondDeathChampionIndex].currentHealth == 0 &&
        state->firstDeathChampionIndex != state->secondDeathChampionIndex;
    step->separatePendingCandidateRegistered =
        state->secondDeathRegisteredSeparatePending &&
        !state->secondDeathSilentlyAbsorbed;
    step->leaderSwitchPreserved =
        state->leaderIndex == 1 && state->leaderSwitchCount == 1 &&
        state->leaderDeathSwitchCount == 1;
    step->panelShowsPendingCandidate =
        (unsigned int)state->panel.activeCandidateOrdinal ==
            state->candidateChampionOrdinal &&
        state->panel.activeCandidateOrdinal != 0;
    memset(seen, 0, sizeof(seen));
    step->noChampionSlotDroppedOrMerged =
        state->partyChampionCount ==
            DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        step->healthAfter[championIndex] =
            state->champions[championIndex].currentHealth;
        if (!state->champions[championIndex].present ||
            state->champions[championIndex].portraitOrdinal < 20 ||
            state->champions[championIndex].portraitOrdinal >= 24) {
            step->noChampionSlotDroppedOrMerged = 0;
        } else {
            seen[state->champions[championIndex].portraitOrdinal - 20] = 1;
        }
    }
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        if (!seen[championIndex]) {
            step->noChampionSlotDroppedOrMerged = 0;
        }
    }
}

static void draw_all_champion_states(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state)
{
    int championIndex;

    for (championIndex = 0; championIndex < state->partyChampionCount;
         ++championIndex) {
        ++state->champions[championIndex].redrawStateCount;
    }
}

static void draw_c040_panel(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    int afterRace)
{
    int candidateIndex;
    int queueIndex;

    /* ReDMCSB: PANEL.C F0347 lines 1651-1656 closes chest state, then
     * dispatches G0299-owned C040 to F0346; F0346 lines 1624-1636 draws
     * C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE as M568. */
    ++state->f0334CloseChestRewriteCount;
    ++state->f0344PanelRedrawCount;
    ++state->f0345PanelRedrawCount;
    ++state->f0346DrawC040PanelCount;
    ++state->f0347DrawPanelDispatchCount;
    ++state->panel.drawGeneration;
    if (afterRace) {
        ++state->panelDrawsAfterRace;
    }
    state->g0426OpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    state->panel.panelOpen = 1;
    state->panel.panelContent =
        DM1_V1_MIRROR_CANDIDATE_RDCR_M568_PANEL_PC34_COMPAT;
    state->panel.panelGraphic =
        DM1_V1_MIRROR_CANDIDATE_RDCR_C040_GRAPHIC_PC34_COMPAT;
    state->panel.chestFirstSlot =
        DM1_V1_MIRROR_CANDIDATE_RDCR_C30_SLOT_CHEST_1_PC34_COMPAT;
    state->panel.chestSlotProbeCount =
        DM1_V1_MIRROR_CANDIDATE_RDCR_CHEST_SLOT_COUNT_PC34_COMPAT;
    state->panel.drawnQueueCount = state->pendingCount;
    for (queueIndex = 0;
         queueIndex <
             DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT;
         ++queueIndex) {
        state->panel.drawnCandidateOrdinals[queueIndex] =
            pending_ordinal_at(state, queueIndex);
    }
    state->panel.activeCandidateOrdinal = pending_ordinal_at(state, 0);
    if (state->panel.activeCandidateOrdinal == 0u &&
        state->consumedCount > 0) {
        state->panel.activeCandidateOrdinal =
            state->consumedOrdinals[state->consumedCount - 1];
    }
    if (state->panel.activeCandidateOrdinal != 0u) {
        state->panel.lastRaceCandidateOrdinal =
            state->panel.activeCandidateOrdinal;
    }
    candidateIndex = index_from_ordinal(state->panel.activeCandidateOrdinal);
    state->activePanelChampionIndex = candidateIndex;
    if (valid_champion_index(candidateIndex)) {
        ++state->champions[candidateIndex].redrawPanelCount;
    }
    if ((unsigned int)state->panel.activeCandidateOrdinal ==
        ordinal_from_index(state->secondDeathChampionIndex)) {
        ++state->panelDrawsWithSecondCandidate;
    }
    draw_all_champion_states(state);
}

static int push_pending_candidate(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    int championIndex)
{
    if (!state || !valid_champion_index(championIndex) ||
        state->pendingCount >=
            DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT) {
        return 0;
    }
    state->pendingOrdinals[state->pendingCount++] =
        ordinal_from_index(championIndex);
    state->candidateChampionOrdinal = state->pendingOrdinals[0];
    return 1;
}

static unsigned int pop_pending_candidate(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state)
{
    unsigned int ordinal;
    int queueIndex;

    if (!state || state->pendingCount <= 0) {
        return 0u;
    }
    ordinal = state->pendingOrdinals[0];
    for (queueIndex = 1; queueIndex < state->pendingCount; ++queueIndex) {
        state->pendingOrdinals[queueIndex - 1] =
            state->pendingOrdinals[queueIndex];
    }
    --state->pendingCount;
    state->pendingOrdinals[state->pendingCount] = 0u;
    if (state->consumedCount <
        DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT) {
        state->consumedOrdinals[state->consumedCount++] = ordinal;
    }
    state->candidateChampionOrdinal =
        state->pendingCount > 0 ? state->pendingOrdinals[0] : 0u;
    return ordinal;
}

static int register_death(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    int championIndex,
    int firstDeath,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    int previousLeader;

    if (!state || !valid_champion_index(championIndex) ||
        championIndex >= state->partyChampionCount ||
        state->champions[championIndex].currentHealth == 0) {
        return 0;
    }

    previousLeader = state->leaderIndex;
    state->champions[championIndex].currentHealth = 0;
    ++state->champions[championIndex].deathCount;
    ++state->f0280CandidateSetCount;
    ++state->f0302SlotWritebackGuardCount;
    if (!push_pending_candidate(state, championIndex)) {
        if (!firstDeath) {
            state->secondDeathSilentlyAbsorbed = 1;
        }
        return 0;
    }
    state->inventoryChampionOrdinal = state->candidateChampionOrdinal;

    /* ReDMCSB: REVIVE.C F0280 lines 124-132 admits a candidate only while
     * the leader hand is empty and the party has room; COMMAND.C F0359
     * lines 1985-1990 later routes M568/C040 clicks to F0282. */
    if (championIndex == previousLeader) {
        state->leaderIndex = 1;
        ++state->leaderSwitchCount;
        ++state->leaderDeathSwitchCount;
    } else if (state->leaderIndex == previousLeader) {
        ++state->nonLeaderDeathPreservedLeaderCount;
    }
    if (firstDeath) {
        state->firstDeathChampionIndex = championIndex;
    } else {
        state->secondDeathChampionIndex = championIndex;
        state->secondDeathRegisteredSeparatePending =
            state->pendingCount == 1 &&
            state->pendingOrdinals[0] == ordinal_from_index(championIndex) &&
            state->champions[state->firstDeathChampionIndex].currentHealth > 0;
    }
    draw_c040_panel(state, 0);
    if (step) {
        step->routeF0280 = 1;
        step->routeF0334 = 1;
        step->routeF0344 = 1;
        step->routeF0345 = 1;
        step->routeF0346 = 1;
        step->routeF0347 = 1;
        step->routeF0302 = 1;
    }
    return 1;
}

static int process_resurrect(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    int expectedChampionIndex,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    unsigned int ordinal;
    int championIndex;

    if (!state || !state->leaderEmptyHanded || state->pendingCount <= 0) {
        return 0;
    }

    /* ReDMCSB: COMMAND.C F0359 lines 1985-1990 requires an empty leader
     * hand before dispatching C160/C161/C162.  REVIVE.C F0282 lines 744-806
     * consumes G0305-1, clears G0299 at line 785, and rewrites the champion
     * instead of replacing another slot. */
    ++state->f0359C040DispatchCount;
    ++state->f0282ResurrectRouteCount;
    ++state->f0297PutLeaderHandCount;
    ++state->f0298RemoveLeaderHandCount;
    ++state->f0302SlotWritebackGuardCount;
    ordinal = pop_pending_candidate(state);
    championIndex = index_from_ordinal(ordinal);
    if (!valid_champion_index(championIndex) ||
        championIndex != expectedChampionIndex ||
        state->champions[championIndex].currentHealth != 0) {
        return 0;
    }
    state->champions[championIndex].currentHealth = 1;
    ++state->champions[championIndex].resurrectCount;
    state->inventoryChampionOrdinal = ordinal;
    if (championIndex == state->firstDeathChampionIndex) {
        state->clobberedFirstResurrectedChampion =
            state->champions[championIndex].portraitOrdinal !=
            20 + championIndex;
    }
    if (championIndex == state->secondDeathChampionIndex) {
        state->clobberedSecondDeadChampion =
            state->champions[championIndex].portraitOrdinal !=
            20 + championIndex;
    }
    if (step) {
        step->routeF0282 = 1;
        step->routeF0297 = 1;
        step->routeF0298 = 1;
        step->routeF0302 = 1;
        step->routeF0359 = 1;
    }
    return 1;
}

void DM1_V1_MirrorCandidateRdcr_InitPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state)
{
    int championIndex;
    int slotIndex;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->partyChampionCount =
        DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
    state->leaderIndex = 0;
    state->leaderEmptyHanded = 1;
    state->activePanelChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    state->firstDeathChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    state->secondDeathChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    state->g0426OpenChestThing =
        DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    for (slotIndex = 0;
         slotIndex <
             DM1_V1_MIRROR_CANDIDATE_RDCR_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        state->g0425ChestSlots[slotIndex] =
            DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT;
    }
    for (championIndex = 0;
         championIndex <
             DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        state->champions[championIndex].present = 1;
        state->champions[championIndex].currentHealth = 40 + championIndex;
        state->champions[championIndex].cell = championIndex;
        state->champions[championIndex].direction = 0;
        state->champions[championIndex].portraitOrdinal = 20 + championIndex;
        for (slotIndex = 0;
             slotIndex <
                 DM1_V1_MIRROR_CANDIDATE_RDCR_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            state->champions[championIndex].slots[slotIndex] =
                100 * championIndex + slotIndex;
        }
    }
}

int DM1_V1_MirrorCandidateRdcr_FirstDeathPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep)
{
    snapshot_step(outStep, state,
                  DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_FIRST_DEATH_PC34_COMPAT,
                  "STEP_FIRST_DEATH",
                  "REVIVE.C F0280:124-132; COMMAND.C F0359:1985-1990");
    if (!register_death(state, 0, 1, outStep)) {
        return 0;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRdcr_FirstResurrectPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep)
{
    snapshot_step(outStep, state,
                  DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_FIRST_RESURRECT_PC34_COMPAT,
                  "STEP_FIRST_RESURRECT",
                  "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    if (!process_resurrect(state, 0, outStep)) {
        return 0;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRdcr_SecondDeathPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep)
{
    snapshot_step(outStep, state,
                  DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_SECOND_DEATH_PC34_COMPAT,
                  "STEP_SECOND_DEATH",
                  "REVIVE.C F0280:124-132; PANEL.C F0344/F0345/F0346/F0347");
    if (!register_death(state, 2, 0, outStep)) {
        return 0;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRdcr_SecondResurrectPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep)
{
    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_SECOND_RESURRECT_PC34_COMPAT,
        "STEP_SECOND_RESURRECT",
        "REVIVE.C F0282:744-806; CHAMPION.C F0297/F0298/F0302");
    if (!process_resurrect(state, 2, outStep)) {
        return 0;
    }
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRdcr_PanelDrawAfterRacePc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep)
{
    snapshot_step(
        outStep, state,
        DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_PANEL_DRAW_AFTER_RACE_PC34_COMPAT,
        "STEP_PANEL_DRAW_AFTER_RACE",
        "PANEL.C F0344/F0345/F0346/F0347; CHEST.C F0333/F0334");
    if (!state) {
        return 0;
    }
    draw_c040_panel(state, 1);
    finish_step(outStep, state);
    return 1;
}

int DM1_V1_MirrorCandidateRdcr_DriveRegressionPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *steps,
    int stepCapacity)
{
    if (!state || !steps || stepCapacity < 5) {
        return 0;
    }
    DM1_V1_MirrorCandidateRdcr_InitPc34Compat(state);
    if (!DM1_V1_MirrorCandidateRdcr_FirstDeathPc34Compat(
            state, &steps[0])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRdcr_FirstResurrectPc34Compat(
            state, &steps[1])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRdcr_SecondDeathPc34Compat(
            state, &steps[2])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRdcr_SecondResurrectPc34Compat(
            state, &steps[3])) {
        return 0;
    }
    if (!DM1_V1_MirrorCandidateRdcr_PanelDrawAfterRacePc34Compat(
            state, &steps[4])) {
        return 0;
    }
    return 5;
}

const char *
DM1_V1_MirrorCandidateRdcr_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB anchors: COMMAND.C F0359:1985-1990 empty-hand M568/C040 "
           "dispatch to F0282; REVIVE.C F0282:744-806 resurrect command "
           "consumes the current candidate and clears G0299; REVIVE.C "
           "F0280:124-132 candidate add gate; CHEST.C F0333:30-67 and "
           "F0334:117-132 open/close chest slot rewrite; CHAMPION.C "
           "F0297:243-298 and F0298:270-298 leader-hand read/write; "
           "CHAMPION.C F0302:662-714 C030/C040 writeback guard; PANEL.C "
           "F0344/F0345 food-water redraw helpers and F0346/F0347 "
           "resurrect panel redraw; DEFS.H C30/G0425/G0426/M070/M516/C040.";
}
