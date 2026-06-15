#include "dm1_v1_mirror_candidate_resurrect_double_candidate_race_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gAssertions;
static int gFailures;

#define CHECK_EQ(actual, expected, msg) do { \
    int actualValue = (int)(actual); \
    int expectedValue = (int)(expected); \
    ++gAssertions; \
    if (actualValue != expectedValue) { \
        ++gFailures; \
        printf("FAIL: %s actual=%d expected=%d\n", \
               (msg), actualValue, expectedValue); \
    } \
} while (0)

#define CHECK_NE(actual, unexpected, msg) do { \
    int actualValue = (int)(actual); \
    int unexpectedValue = (int)(unexpected); \
    ++gAssertions; \
    if (actualValue == unexpectedValue) { \
        ++gFailures; \
        printf("FAIL: %s actual=%d unexpected=%d\n", \
               (msg), actualValue, unexpectedValue); \
    } \
} while (0)

#define CHECK_CONTAINS(haystack, needle, msg) do { \
    ++gAssertions; \
    if ((haystack) == 0 || strstr((haystack), (needle)) == 0) { \
        ++gFailures; \
        printf("FAIL: %s missing='%s'\n", (msg), (needle)); \
    } \
} while (0)

static void check_source_evidence(void)
{
    const char *evidence =
        DM1_V1_MirrorCandidateRdcr_SourceEvidencePc34Compat();

    CHECK_CONTAINS(evidence, "COMMAND.C F0359:1985-1990",
                   "source evidence cites C040 command dispatch");
    CHECK_CONTAINS(evidence, "REVIVE.C F0282:744-806",
                   "source evidence cites resurrect command");
    CHECK_CONTAINS(evidence, "REVIVE.C F0280:124-132",
                   "source evidence cites candidate state machine");
    CHECK_CONTAINS(evidence, "CHEST.C F0333:30-67",
                   "source evidence cites chest open rewrite");
    CHECK_CONTAINS(evidence, "F0334:117-132",
                   "source evidence cites chest close rewrite");
    CHECK_CONTAINS(evidence, "CHAMPION.C F0297:243-298",
                   "source evidence cites leader-hand put");
    CHECK_CONTAINS(evidence, "F0298:270-298",
                   "source evidence cites leader-hand remove");
    CHECK_CONTAINS(evidence, "CHAMPION.C F0302:662-714",
                   "source evidence cites C030/C040 writeback guard");
    CHECK_CONTAINS(evidence, "PANEL.C F0344/F0345",
                   "source evidence cites panel redraw helpers");
    CHECK_CONTAINS(evidence, "F0346/F0347",
                   "source evidence cites C040 panel redraw");
    CHECK_CONTAINS(evidence, "DEFS.H C30/G0425/G0426/M070/M516/C040",
                   "source evidence cites defs symbols");
}

static void check_step_shape(
    const Dm1V1MirrorCandidateRdcrStepPc34Compat *steps,
    int stepCount)
{
    CHECK_EQ(stepCount, 5, "driver emits five explicit state-machine steps");
    CHECK_EQ(steps[0].stepId,
             DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_FIRST_DEATH_PC34_COMPAT,
             "step0 is STEP_FIRST_DEATH");
    CHECK_EQ(steps[1].stepId,
             DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_FIRST_RESURRECT_PC34_COMPAT,
             "step1 is STEP_FIRST_RESURRECT");
    CHECK_EQ(steps[2].stepId,
             DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_SECOND_DEATH_PC34_COMPAT,
             "step2 is STEP_SECOND_DEATH");
    CHECK_EQ(steps[3].stepId,
             DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_SECOND_RESURRECT_PC34_COMPAT,
             "step3 is STEP_SECOND_RESURRECT");
    CHECK_EQ(steps[4].stepId,
             DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_PANEL_DRAW_AFTER_RACE_PC34_COMPAT,
             "step4 is STEP_PANEL_DRAW_AFTER_RACE");
    CHECK_CONTAINS(steps[0].name, "FIRST_DEATH",
                   "step0 name pins first death");
    CHECK_CONTAINS(steps[1].name, "FIRST_RESURRECT",
                   "step1 name pins first resurrect");
    CHECK_CONTAINS(steps[2].name, "SECOND_DEATH",
                   "step2 name pins second death");
    CHECK_CONTAINS(steps[3].name, "SECOND_RESURRECT",
                   "step3 name pins second resurrect");
    CHECK_CONTAINS(steps[4].name, "PANEL_DRAW_AFTER_RACE",
                   "step4 name pins panel redraw after race");
}

static void check_first_death(
    const Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 0,
             "first death starts with no candidate");
    CHECK_EQ(step->candidateOrdinalAfter, 1,
             "first death queues champion 0 as candidate ordinal 1");
    CHECK_EQ(step->pendingCountBefore, 0,
             "first death starts with empty pending queue");
    CHECK_EQ(step->pendingCountAfter, 1,
             "first death adds one pending candidate");
    CHECK_EQ(step->pendingHeadOrdinalAfter, 1,
             "first death pending head is champion 0");
    CHECK_EQ(step->leaderIndexBefore, 0,
             "first death starts with champion 0 as leader");
    CHECK_EQ(step->leaderIndexAfter, 1,
             "first death switches leader to champion 1");
    CHECK_EQ(step->partyChampionCountBefore, 4,
             "first death starts with four champion slots");
    CHECK_EQ(step->partyChampionCountAfter, 4,
             "first death keeps four champion slots");
    CHECK_EQ(step->aliveCountBefore, 4,
             "first death starts with four live champions");
    CHECK_EQ(step->aliveCountAfter, 3,
             "first death leaves exactly one dead champion");
    CHECK_EQ(step->healthBefore[0], 40,
             "champion 0 health before first death");
    CHECK_EQ(step->healthAfter[0], 0,
             "champion 0 health after first death");
    CHECK_EQ(step->healthAfter[1], 41,
             "champion 1 remains alive after first death");
    CHECK_EQ(step->routeF0280, 1,
             "first death routes through F0280 candidate add");
    CHECK_EQ(step->routeF0346, 1,
             "first death redraws C040 panel");
    CHECK_EQ(step->routeF0347, 1,
             "first death dispatches inventory panel redraw");
    CHECK_EQ(step->routeF0334, 1,
             "first death keeps chest rewrite path closed");
    CHECK_EQ(step->panelShowsPendingCandidate, 1,
             "first death panel shows queued pending candidate");
    CHECK_EQ(step->noChampionSlotDroppedOrMerged, 1,
             "first death does not drop or merge champion slots");
}

static void check_first_resurrect(
    const Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 1,
             "first resurrect starts with champion 0 pending");
    CHECK_EQ(step->candidateOrdinalAfter, 0,
             "first resurrect consumes the only pending candidate");
    CHECK_EQ(step->pendingCountBefore, 1,
             "first resurrect starts with one pending candidate");
    CHECK_EQ(step->pendingCountAfter, 0,
             "first resurrect clears pending queue");
    CHECK_EQ(step->pendingHeadOrdinalBefore, 1,
             "first resurrect consumes champion 0");
    CHECK_EQ(step->leaderIndexBefore, 1,
             "first resurrect starts after leader switch to champion 1");
    CHECK_EQ(step->leaderIndexAfter, 1,
             "first resurrect preserves switched leader");
    CHECK_EQ(step->aliveCountBefore, 3,
             "first resurrect starts with champion 0 dead");
    CHECK_EQ(step->aliveCountAfter, 4,
             "first resurrect restores four live champions");
    CHECK_EQ(step->healthBefore[0], 0,
             "champion 0 is dead before first resurrect");
    CHECK_EQ(step->healthAfter[0], 1,
             "champion 0 is alive after first resurrect");
    CHECK_EQ(step->healthAfter[2], 42,
             "future second-death champion remains untouched");
    CHECK_EQ(step->routeF0282, 1,
             "first resurrect routes through F0282");
    CHECK_EQ(step->routeF0297, 1,
             "first resurrect preserves leader-hand writeback");
    CHECK_EQ(step->routeF0298, 1,
             "first resurrect preserves leader-hand readback");
    CHECK_EQ(step->routeF0302, 1,
             "first resurrect preserves C030/C040 writeback guard");
    CHECK_EQ(step->routeF0359, 1,
             "first resurrect routes from C040 command dispatch");
    CHECK_EQ(step->firstResurrectedAlive, 1,
             "first resurrected champion is alive before the race");
    CHECK_EQ(step->leaderSwitchPreserved, 1,
             "first resurrect preserves leader-switch behavior");
    CHECK_EQ(step->noChampionSlotDroppedOrMerged, 1,
             "first resurrect keeps champion slots distinct");
}

static void check_second_death(
    const Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 0,
             "second death starts after first pending consumed");
    CHECK_EQ(step->candidateOrdinalAfter, 3,
             "second death queues champion 2 as candidate ordinal 3");
    CHECK_EQ(step->pendingCountBefore, 0,
             "second death starts with no pending candidates");
    CHECK_EQ(step->pendingCountAfter, 1,
             "second death registers a new pending candidate");
    CHECK_EQ(step->pendingHeadOrdinalAfter, 3,
             "second death pending head is champion 2");
    CHECK_EQ(step->leaderIndexBefore, 1,
             "second death starts with champion 1 leader");
    CHECK_EQ(step->leaderIndexAfter, 1,
             "second death preserves leader for non-leader death");
    CHECK_EQ(step->partyChampionCountAfter, 4,
             "second death keeps four champion slots");
    CHECK_EQ(step->aliveCountBefore, 4,
             "second death starts after first champion is alive");
    CHECK_EQ(step->aliveCountAfter, 3,
             "second death leaves one dead champion");
    CHECK_EQ(step->healthBefore[0], 1,
             "first resurrected champion is alive before second death");
    CHECK_EQ(step->healthAfter[0], 1,
             "second death must not clobber first resurrected champion");
    CHECK_EQ(step->healthBefore[2], 42,
             "champion 2 is alive before second death");
    CHECK_EQ(step->healthAfter[2], 0,
             "champion 2 is dead after second death");
    CHECK_EQ(step->routeF0280, 1,
             "second death routes through a fresh F0280 candidate add");
    CHECK_EQ(step->routeF0346, 1,
             "second death redraws C040 for the new pending candidate");
    CHECK_EQ(step->routeF0347, 1,
             "second death dispatches panel redraw");
    CHECK_EQ(step->firstResurrectedAlive, 1,
             "second death sees first resurrected champion alive");
    CHECK_EQ(step->secondDeadStillDistinct, 1,
             "second death keeps the second dead champion distinct");
    CHECK_EQ(step->separatePendingCandidateRegistered, 1,
             "second death is not silently absorbed");
    CHECK_EQ(step->panelShowsPendingCandidate, 1,
             "second death makes the new candidate visible in C040 state");
    CHECK_EQ(step->leaderSwitchPreserved, 1,
             "second death preserves leader-switch behavior");
    CHECK_EQ(step->noChampionSlotDroppedOrMerged, 1,
             "second death does not merge champion slots");
}

static void check_second_resurrect(
    const Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 3,
             "second resurrect starts with champion 2 pending");
    CHECK_EQ(step->candidateOrdinalAfter, 0,
             "second resurrect consumes second pending candidate");
    CHECK_EQ(step->pendingCountBefore, 1,
             "second resurrect starts with one pending candidate");
    CHECK_EQ(step->pendingCountAfter, 0,
             "second resurrect clears the second pending candidate");
    CHECK_EQ(step->pendingHeadOrdinalBefore, 3,
             "second resurrect consumes champion 2 ordinal");
    CHECK_EQ(step->leaderIndexBefore, 1,
             "second resurrect starts with preserved leader");
    CHECK_EQ(step->leaderIndexAfter, 1,
             "second resurrect keeps preserved leader");
    CHECK_EQ(step->aliveCountBefore, 3,
             "second resurrect starts with champion 2 dead");
    CHECK_EQ(step->aliveCountAfter, 4,
             "second resurrect restores all four champions alive");
    CHECK_EQ(step->healthAfter[0], 1,
             "second resurrect leaves first resurrected champion alive");
    CHECK_EQ(step->healthBefore[2], 0,
             "champion 2 is dead before second resurrect");
    CHECK_EQ(step->healthAfter[2], 1,
             "champion 2 is alive after second resurrect");
    CHECK_EQ(step->routeF0282, 1,
             "second resurrect routes through F0282");
    CHECK_EQ(step->routeF0297, 1,
             "second resurrect preserves leader-hand writeback");
    CHECK_EQ(step->routeF0298, 1,
             "second resurrect preserves leader-hand readback");
    CHECK_EQ(step->routeF0302, 1,
             "second resurrect preserves slot writeback guard");
    CHECK_EQ(step->routeF0359, 1,
             "second resurrect routes from C040 command dispatch");
    CHECK_EQ(step->firstResurrectedAlive, 1,
             "second resurrect keeps first champion alive");
    CHECK_EQ(step->leaderSwitchPreserved, 1,
             "second resurrect preserves leader-switch behavior");
    CHECK_EQ(step->noChampionSlotDroppedOrMerged, 1,
             "second resurrect keeps champion slots distinct");
}

static void check_panel_draw_after_race(
    const Dm1V1MirrorCandidateRdcrStepPc34Compat *step)
{
    CHECK_EQ(step->pendingCountBefore, 0,
             "post-race panel draw starts after both candidates consumed");
    CHECK_EQ(step->pendingCountAfter, 0,
             "post-race panel draw does not add a phantom candidate");
    CHECK_EQ(step->leaderIndexBefore, 1,
             "post-race panel draw starts with preserved leader");
    CHECK_EQ(step->leaderIndexAfter, 1,
             "post-race panel draw preserves leader");
    CHECK_EQ(step->aliveCountBefore, 4,
             "post-race panel draw starts with all champions alive");
    CHECK_EQ(step->aliveCountAfter, 4,
             "post-race panel draw keeps all champions alive");
    CHECK_EQ(step->healthAfter[0], 1,
             "post-race panel draw keeps first resurrected champion alive");
    CHECK_EQ(step->healthAfter[2], 1,
             "post-race panel draw keeps second resurrected champion alive");
    CHECK_EQ(step->routeF0346, 0,
             "post-race step records redraw through state counters");
    CHECK_EQ(step->firstResurrectedAlive, 1,
             "post-race panel draw still sees first champion alive");
    CHECK_EQ(step->leaderSwitchPreserved, 1,
             "post-race panel draw preserves leader switch");
    CHECK_EQ(step->noChampionSlotDroppedOrMerged, 1,
             "post-race panel draw keeps all champion slots");
    CHECK_EQ(step->panelDrawGenerationAfter,
             step->panelDrawGenerationBefore + 1,
             "post-race panel draw advances panel generation once");
}

static void check_final_state(
    const Dm1V1MirrorCandidateRdcrStatePc34Compat *state)
{
    int championIndex;
    int slotIndex;

    CHECK_EQ(state->partyChampionCount, 4,
             "final party pool still contains four champion slots");
    CHECK_EQ(state->leaderIndex, 1,
             "final leader remains champion 1 after leader-switch preservation");
    CHECK_EQ(state->leaderSwitchCount, 1,
             "exactly one leader switch occurred");
    CHECK_EQ(state->leaderDeathSwitchCount, 1,
             "leader switch was caused by first leader death");
    CHECK_EQ(state->nonLeaderDeathPreservedLeaderCount, 1,
             "non-leader second death preserved leader");
    CHECK_EQ(state->candidateChampionOrdinal, 0,
             "final candidate ordinal is clear after both resurrects");
    CHECK_EQ(state->inventoryChampionOrdinal, 3,
             "final inventory ordinal records second resurrected champion");
    CHECK_EQ(state->pendingCount, 0,
             "final pending queue is empty");
    CHECK_EQ(state->consumedCount, 2,
             "two pending candidates were consumed in order");
    CHECK_EQ(state->consumedOrdinals[0], 1,
             "first consumed candidate was champion 0");
    CHECK_EQ(state->consumedOrdinals[1], 3,
             "second consumed candidate was champion 2");
    CHECK_EQ(state->firstDeathChampionIndex, 0,
             "first death champion index");
    CHECK_EQ(state->secondDeathChampionIndex, 2,
             "second death champion index");
    CHECK_NE(state->firstDeathChampionIndex, state->secondDeathChampionIndex,
             "two deaths use distinct champion slots");
    CHECK_EQ(state->secondDeathRegisteredSeparatePending, 1,
             "second death registered a separate pending candidate");
    CHECK_EQ(state->secondDeathSilentlyAbsorbed, 0,
             "second death was not silently absorbed");
    CHECK_EQ(state->clobberedFirstResurrectedChampion, 0,
             "resurrect command did not clobber first resurrected champion");
    CHECK_EQ(state->clobberedSecondDeadChampion, 0,
             "resurrect command did not clobber second dead champion");
    CHECK_EQ(state->f0280CandidateSetCount, 2,
             "two F0280 candidate registrations occurred");
    CHECK_EQ(state->f0282ResurrectRouteCount, 2,
             "two F0282 resurrect routes occurred");
    CHECK_EQ(state->f0297PutLeaderHandCount, 2,
             "two leader-hand writeback observations occurred");
    CHECK_EQ(state->f0298RemoveLeaderHandCount, 2,
             "two leader-hand readback observations occurred");
    CHECK_EQ(state->f0302SlotWritebackGuardCount, 4,
             "slot writeback guard covered deaths and resurrects");
    CHECK_EQ(state->f0334CloseChestRewriteCount, 3,
             "panel redraw closed/reconciled chest state three times");
    CHECK_EQ(state->f0344PanelRedrawCount, 3,
             "F0344 redraw helper count");
    CHECK_EQ(state->f0345PanelRedrawCount, 3,
             "F0345 redraw helper count");
    CHECK_EQ(state->f0346DrawC040PanelCount, 3,
             "C040 panel drawn for first death, second death, and final redraw");
    CHECK_EQ(state->f0347DrawPanelDispatchCount, 3,
             "F0347 dispatch count");
    CHECK_EQ(state->f0359C040DispatchCount, 2,
             "two C040 resurrect command dispatches");
    CHECK_EQ(state->panelDrawsWithSecondCandidate, 2,
             "C040 panel drew the second candidate while pending and after race");
    CHECK_EQ(state->panelDrawsAfterRace, 1,
             "one explicit post-race panel redraw occurred");
    CHECK_EQ(state->g0426OpenChestThing, -1,
             "G0426 open chest remains none");
    CHECK_EQ(state->panel.panelOpen, 1,
             "C040 panel remains open across the race");
    CHECK_EQ(state->panel.panelContent, 568,
             "panel content is M568");
    CHECK_EQ(state->panel.panelGraphic, 40,
             "panel graphic is C040");
    CHECK_EQ(state->panel.activeCandidateOrdinal, 3,
             "post-race panel retains the second race candidate as active draw");
    CHECK_EQ(state->panel.lastRaceCandidateOrdinal, 3,
             "last race candidate ordinal is champion 2");
    CHECK_EQ(state->panel.drawGeneration, 3,
             "panel draw generation count");
    CHECK_EQ(state->panel.chestFirstSlot, 30,
             "panel records C30 chest first slot");
    CHECK_EQ(state->panel.chestSlotProbeCount, 8,
             "panel probes eight G0425 chest slots");
    for (slotIndex = 0;
         slotIndex < DM1_V1_MIRROR_CANDIDATE_RDCR_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        CHECK_EQ(state->g0425ChestSlots[slotIndex], -1,
                 "G0425 chest slot remains none");
    }
    for (championIndex = 0;
         championIndex < DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        CHECK_EQ(state->champions[championIndex].present, 1,
                 "champion slot remains present");
        CHECK_EQ(state->champions[championIndex].portraitOrdinal,
                 20 + championIndex,
                 "champion portrait identity remains distinct");
        CHECK_EQ(state->champions[championIndex].redrawStateCount, 3,
                 "each champion receives three panel-state redraws");
        for (slotIndex = 0;
             slotIndex < DM1_V1_MIRROR_CANDIDATE_RDCR_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            CHECK_EQ(state->champions[championIndex].slots[slotIndex],
                     100 * championIndex + slotIndex,
                     "champion C00..C29 slots remain identity-stable");
        }
    }
    CHECK_EQ(state->champions[0].deathCount, 1,
             "champion 0 death count");
    CHECK_EQ(state->champions[0].resurrectCount, 1,
             "champion 0 resurrect count");
    CHECK_EQ(state->champions[0].currentHealth, 1,
             "champion 0 final health alive");
    CHECK_EQ(state->champions[2].deathCount, 1,
             "champion 2 death count");
    CHECK_EQ(state->champions[2].resurrectCount, 1,
             "champion 2 resurrect count");
    CHECK_EQ(state->champions[2].currentHealth, 1,
             "champion 2 final health alive");
    CHECK_EQ(state->champions[1].deathCount, 0,
             "champion 1 never dies");
    CHECK_EQ(state->champions[3].deathCount, 0,
             "champion 3 never dies");
}

int main(void)
{
    Dm1V1MirrorCandidateRdcrStatePc34Compat state;
    Dm1V1MirrorCandidateRdcrStepPc34Compat steps[5];
    int stepCount;

    check_source_evidence();
    stepCount = DM1_V1_MirrorCandidateRdcr_DriveRegressionPc34Compat(
        &state, steps, 5);
    check_step_shape(steps, stepCount);
    check_first_death(&steps[0]);
    check_first_resurrect(&steps[1]);
    check_second_death(&steps[2]);
    check_second_resurrect(&steps[3]);
    check_panel_draw_after_race(&steps[4]);
    check_final_state(&state);

    if (gFailures == 0) {
        printf("PASS dm1_v1_mirror_candidate_resurrect_double_candidate_race_pc34_compat assertions=%d failures=0\n",
               gAssertions);
    } else {
        printf("FAIL dm1_v1_mirror_candidate_resurrect_double_candidate_race_pc34_compat assertions=%d failures=%d\n",
               gAssertions, gFailures);
    }
    return gFailures == 0 ? 0 : 1;
}
