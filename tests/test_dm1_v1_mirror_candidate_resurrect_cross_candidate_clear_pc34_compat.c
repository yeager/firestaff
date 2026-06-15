#include "dm1/dm1_v1_mirror_candidate_resurrect_cross_candidate_clear_pc34_compat.h"

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
        DM1_V1_MirrorCandidateRcc_SourceEvidencePc34Compat();

    CHECK_CONTAINS(evidence, "COMMAND.C F0359:1985-1990",
                   "source evidence cites C040 command dispatch");
    CHECK_CONTAINS(evidence, "REVIVE.C F0280:124-132",
                   "source evidence cites candidate admission");
    CHECK_CONTAINS(evidence, "REVIVE.C F0282:744-806",
                   "source evidence cites cancel/confirm handling");
    CHECK_CONTAINS(evidence, "CHAMPION.C F0284:93-131",
                   "source evidence cites party direction identity");
    CHECK_CONTAINS(evidence, "F0297:243-298",
                   "source evidence cites leader-hand put");
    CHECK_CONTAINS(evidence, "F0298:270-298",
                   "source evidence cites leader-hand remove");
    CHECK_CONTAINS(evidence, "F0300:511-515",
                   "source evidence cites slot clear");
    CHECK_CONTAINS(evidence, "F0301:606-614",
                   "source evidence cites slot write");
    CHECK_CONTAINS(evidence, "F0302:662-714",
                   "source evidence cites slot click guard");
    CHECK_CONTAINS(evidence, "CHEST.C F0333:30-67",
                   "source evidence cites chest open route");
    CHECK_CONTAINS(evidence, "F0334:113-132",
                   "source evidence cites chest close rewrite");
    CHECK_CONTAINS(evidence, "PANEL.C F0344:1493-1561",
                   "source evidence cites food bar helper");
    CHECK_CONTAINS(evidence, "F0345:1563-1615",
                   "source evidence cites food/water panel");
    CHECK_CONTAINS(evidence, "F0346:1619-1635",
                   "source evidence cites C040 draw");
    CHECK_CONTAINS(evidence, "F0347:1639-1656",
                   "source evidence cites panel dispatch");
    CHECK_CONTAINS(evidence, "DEFS.H:338/2088/2200/3002",
                   "source evidence cites defs anchors");
}

static void check_step_shape(
    const Dm1V1MirrorCandidateRccStepPc34Compat *steps,
    int stepCount)
{
    CHECK_EQ(stepCount, 5, "driver emits five cross-candidate steps");
    CHECK_EQ(steps[0].stepId,
             DM1_V1_MIRROR_CANDIDATE_RCC_STEP_SEED_STALE_B_PANEL_PC34_COMPAT,
             "step0 seeds stale B panel");
    CHECK_EQ(steps[1].stepId,
             DM1_V1_MIRROR_CANDIDATE_RCC_STEP_CLOSE_STALE_B_PANEL_PC34_COMPAT,
             "step1 closes stale B panel");
    CHECK_EQ(steps[2].stepId,
             DM1_V1_MIRROR_CANDIDATE_RCC_STEP_QUEUE_FRESH_A_PANEL_PC34_COMPAT,
             "step2 queues fresh A panel");
    CHECK_EQ(steps[3].stepId,
             DM1_V1_MIRROR_CANDIDATE_RCC_STEP_RESURRECT_A_PC34_COMPAT,
             "step3 resurrects A");
    CHECK_EQ(steps[4].stepId,
             DM1_V1_MIRROR_CANDIDATE_RCC_STEP_SETTLE_PANEL_PC34_COMPAT,
             "step4 settles the panel");
    CHECK_CONTAINS(steps[0].name, "STALE_B",
                   "step0 name contains stale B");
    CHECK_CONTAINS(steps[2].name, "FRESH_A",
                   "step2 name contains fresh A");
    CHECK_CONTAINS(steps[3].redmcsbAnchor, "F0359",
                   "step3 anchor names F0359 dispatch");
}

static void check_seed_stale_b(
    const Dm1V1MirrorCandidateRccStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 0,
             "seed starts without a global candidate");
    CHECK_EQ(step->candidateOrdinalAfter, 2,
             "seed installs stale B candidate ordinal");
    CHECK_EQ(step->inventoryOrdinalAfter, 2,
             "seed inventory ordinal points at stale B");
    CHECK_EQ(step->panelOwnerBefore, -1,
             "seed starts with no panel owner");
    CHECK_EQ(step->panelOwnerAfter, 1,
             "seed panel owner is champion B");
    CHECK_EQ(step->panelOpenBefore, 0,
             "seed starts with panel closed");
    CHECK_EQ(step->panelOpenAfter, 1,
             "seed leaves C040 panel open");
    CHECK_EQ(step->leaderIndexAfter, 0,
             "seed keeps leader at champion A");
    CHECK_EQ(step->healthBefore[1], 41,
             "B starts alive before stale seed");
    CHECK_EQ(step->healthAfter[1], 0,
             "B is dead with stale panel open");
    CHECK_EQ(step->routeF0280, 1,
             "seed records F0280 candidate path");
    CHECK_EQ(step->routeF0346, 1,
             "seed draws C040 panel");
    CHECK_EQ(step->routeF0347, 1,
             "seed routes through F0347");
    CHECK_EQ(step->bStillDead, 1,
             "seed B is dead");
    CHECK_EQ(step->staleBClosedWithoutResurrect, 0,
             "seed has not closed stale panel yet");
}

static void check_close_stale_b(
    const Dm1V1MirrorCandidateRccStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 2,
             "close starts with stale B candidate ordinal");
    CHECK_EQ(step->candidateOrdinalAfter, 0,
             "close clears stale B candidate ordinal");
    CHECK_EQ(step->inventoryOrdinalAfter, 0,
             "close clears inventory ordinal");
    CHECK_EQ(step->panelOwnerBefore, 1,
             "close starts with B as panel owner");
    CHECK_EQ(step->panelOwnerAfter, -1,
             "close removes stale panel owner");
    CHECK_EQ(step->panelOpenBefore, 1,
             "close starts with C040 open");
    CHECK_EQ(step->panelOpenAfter, 0,
             "close ends with C040 closed");
    CHECK_EQ(step->healthAfter[1], 0,
             "close leaves B dead");
    CHECK_EQ(step->routeF0282, 1,
             "close uses F0282 cancel-shaped rollback");
    CHECK_EQ(step->routeF0334, 1,
             "close rewrites/clears panel chest state");
    CHECK_EQ(step->staleBClosedWithoutResurrect, 1,
             "close marks B panel closed without resurrect");
    CHECK_EQ(step->bStillDead, 1,
             "close does not resurrect B");
}

static void check_queue_fresh_a(
    const Dm1V1MirrorCandidateRccStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 0,
             "fresh queue starts after stale clear");
    CHECK_EQ(step->candidateOrdinalAfter, 1,
             "fresh queue installs A candidate ordinal");
    CHECK_EQ(step->inventoryOrdinalBefore, 0,
             "fresh queue starts with inventory clear");
    CHECK_EQ(step->inventoryOrdinalAfter, 1,
             "fresh queue points inventory at A candidate");
    CHECK_EQ(step->panelOwnerBefore, -1,
             "fresh queue starts with no stale panel owner");
    CHECK_EQ(step->panelOwnerAfter, 0,
             "fresh queue panel owner is A");
    CHECK_EQ(step->panelOpenBefore, 0,
             "fresh queue starts with panel closed");
    CHECK_EQ(step->panelOpenAfter, 1,
             "fresh queue opens C040 for A");
    CHECK_EQ(step->healthBefore[0], 40,
             "A is alive before fresh queue");
    CHECK_EQ(step->healthAfter[0], 0,
             "A is pending/dead after fresh queue");
    CHECK_EQ(step->healthAfter[1], 0,
             "B remains dead after fresh queue");
    CHECK_EQ(step->routeF0280, 1,
             "fresh queue records F0280");
    CHECK_EQ(step->routeF0302, 1,
             "fresh queue records F0302 candidate guard");
    CHECK_EQ(step->routeF0344, 1,
             "fresh queue records F0344 redraw adjacency");
    CHECK_EQ(step->routeF0345, 1,
             "fresh queue records F0345 redraw adjacency");
    CHECK_EQ(step->routeF0346, 1,
             "fresh queue draws C040");
    CHECK_EQ(step->routeF0347, 1,
             "fresh queue dispatches C040 panel");
    CHECK_EQ(step->staleBClosedWithoutResurrect, 1,
             "fresh queue preserves stale B close fact");
    CHECK_EQ(step->freshAIsCurrentCandidate, 1,
             "fresh queue makes A the current candidate");
    CHECK_EQ(step->leaderOwnsFreshA, 1,
             "leader owns A candidate");
    CHECK_EQ(step->bStillDead, 1,
             "fresh queue does not resurrect B");
}

static void check_resurrect_a(
    const Dm1V1MirrorCandidateRccStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 1,
             "resurrect starts with A candidate ordinal");
    CHECK_EQ(step->candidateOrdinalAfter, 0,
             "resurrect clears candidate ordinal");
    CHECK_EQ(step->inventoryOrdinalBefore, 1,
             "resurrect starts with inventory on A");
    CHECK_EQ(step->inventoryOrdinalAfter, 1,
             "resurrect leaves inventory on A");
    CHECK_EQ(step->panelOwnerBefore, 0,
             "resurrect starts with A panel owner");
    CHECK_EQ(step->panelOpenBefore, 1,
             "resurrect starts with A panel open");
    CHECK_EQ(step->panelOpenAfter, 0,
             "resurrect closes A panel after commit");
    CHECK_EQ(step->healthBefore[0], 0,
             "A is dead before resurrect");
    CHECK_EQ(step->healthAfter[0], 40,
             "A is restored to half max health");
    CHECK_EQ(step->healthAfter[1], 0,
             "B remains dead during A resurrect");
    CHECK_EQ(step->routeF0282, 1,
             "resurrect routes through F0282");
    CHECK_EQ(step->routeF0297, 1,
             "resurrect observes leader-hand put path");
    CHECK_EQ(step->routeF0298, 1,
             "resurrect observes leader-hand remove path");
    CHECK_EQ(step->routeF0300, 1,
             "resurrect observes slot clear");
    CHECK_EQ(step->routeF0301, 1,
             "resurrect observes slot write");
    CHECK_EQ(step->routeF0302, 1,
             "resurrect preserves slot guard");
    CHECK_EQ(step->routeF0359, 1,
             "resurrect dispatches from C040");
    CHECK_EQ(step->staleBClosedWithoutResurrect, 1,
             "resurrect keeps stale B close fact");
    CHECK_EQ(step->leaderOwnsFreshA, 1,
             "resurrect keeps leader ownership of A candidate route");
    CHECK_EQ(step->bStillDead, 1,
             "resurrect does not resurrect B");
    CHECK_EQ(step->aResurrected, 1,
             "resurrect commits A");
}

static void check_settle_panel(
    const Dm1V1MirrorCandidateRccStepPc34Compat *step)
{
    CHECK_EQ(step->candidateOrdinalBefore, 0,
             "settle starts with no pending candidate");
    CHECK_EQ(step->candidateOrdinalAfter, 0,
             "settle leaves no pending candidate");
    CHECK_EQ(step->panelOpenBefore, 0,
             "settle starts with panel closed");
    CHECK_EQ(step->panelOpenAfter, 0,
             "settle leaves panel closed");
    CHECK_EQ(step->leaderIndexBefore, 0,
             "settle starts with leader A");
    CHECK_EQ(step->leaderIndexAfter, 0,
             "settle preserves leader A");
    CHECK_EQ(step->healthAfter[0], 40,
             "settle keeps A alive");
    CHECK_EQ(step->healthAfter[1], 0,
             "settle keeps B dead");
    CHECK_EQ(step->routeF0333, 1,
             "settle records F0333 panel/chest route");
    CHECK_EQ(step->routeF0334, 1,
             "settle records F0334 close rewrite");
    CHECK_EQ(step->staleBClosedWithoutResurrect, 1,
             "settle preserves stale close invariant");
    CHECK_EQ(step->leaderOwnsFreshA, 1,
             "settle preserves leader ownership invariant");
    CHECK_EQ(step->bStillDead, 1,
             "settle does not resurrect B");
    CHECK_EQ(step->aResurrected, 1,
             "settle keeps A resurrected");
}

static void check_final_state(
    const Dm1V1MirrorCandidateRccStatePc34Compat *state)
{
    int championIndex;
    int slotIndex;

    CHECK_EQ(state->partyChampionCount, 4,
             "final party keeps four champion slots");
    CHECK_EQ(state->leaderIndex, 0,
             "final leader remains champion A");
    CHECK_EQ(state->leaderEmptyHanded, 1,
             "final leader hand remains empty for C040 dispatch");
    CHECK_EQ(state->candidateChampionOrdinal, 0,
             "final G0299 candidate ordinal is clear");
    CHECK_EQ(state->inventoryChampionOrdinal, 1,
             "final inventory ordinal remains champion A");
    CHECK_EQ(state->candidateOwnerChampionIndex, -1,
             "final candidate owner is clear after commit");
    CHECK_EQ(state->activePanelChampionIndex, -1,
             "final active panel champion is clear");
    CHECK_EQ(state->stalePanelOwnerChampionIndex, 1,
             "final remembers B as stale owner");
    CHECK_EQ(state->staleCandidateOrdinal, 2,
             "final remembers stale B ordinal");
    CHECK_EQ(state->staleCandidateFreed, 1,
             "final freed stale B candidate");
    CHECK_EQ(state->stalePanelClosedWithoutResurrect, 1,
             "final closed B panel without resurrect");
    CHECK_EQ(state->freshCandidateQueuedAfterStaleClose, 1,
             "final queued A only after stale close");
    CHECK_EQ(state->leaderOwnedFreshCandidateThroughout, 1,
             "final leader owned fresh A candidate route");
    CHECK_EQ(state->bPanelEverResurrected, 0,
             "final never resurrected through B stale panel");
    CHECK_EQ(state->f0280CandidateSetCount, 1,
             "final has exactly one fresh F0280 candidate add");
    CHECK_EQ(state->f0282ResurrectRouteCount, 1,
             "final has exactly one F0282 resurrect route");
    CHECK_EQ(state->f0282CancelRouteCount, 1,
             "final has exactly one stale cancel-shaped clear");
    CHECK_EQ(state->f0297PutLeaderHandCount, 1,
             "final leader-hand put count");
    CHECK_EQ(state->f0298RemoveLeaderHandCount, 1,
             "final leader-hand remove count");
    CHECK_EQ(state->f0300SlotClearCount, 1,
             "final slot clear count");
    CHECK_EQ(state->f0301SlotWriteCount, 1,
             "final slot write count");
    CHECK_EQ(state->f0302SlotWritebackGuardCount, 2,
             "final F0302 guard count");
    CHECK_EQ(state->f0333OpenChestRouteCount, 1,
             "final F0333 settle route count");
    CHECK_EQ(state->f0334CloseChestRewriteCount, 2,
             "final F0334 stale-close and settle count");
    CHECK_EQ(state->f0344FoodWaterBarRedrawCount, 2,
             "final F0344 redraw count for two C040 draws");
    CHECK_EQ(state->f0345FoodWaterPanelRedrawCount, 2,
             "final F0345 redraw count for two C040 draws");
    CHECK_EQ(state->f0346DrawC040PanelCount, 2,
             "final C040 drawn for stale B and fresh A");
    CHECK_EQ(state->f0347DrawPanelDispatchCount, 2,
             "final F0347 dispatch count");
    CHECK_EQ(state->f0359C040DispatchCount, 1,
             "final C040 command dispatched only once");
    CHECK_EQ(state->g0426OpenChestThing, -1,
             "final G0426 open chest is none");
    CHECK_EQ(state->panel.panelOpen, 0,
             "final panel is closed");
    CHECK_EQ(state->panel.panelContent, 568,
             "final panel content records M568");
    CHECK_EQ(state->panel.panelGraphic, 40,
             "final panel graphic records C040");
    CHECK_EQ(state->panel.ownerChampionIndex, -1,
             "final panel owner is clear");
    CHECK_EQ(state->panel.candidateOrdinal, 0,
             "final panel candidate ordinal is clear");
    CHECK_EQ(state->panel.drawGeneration, 2,
             "final panel draw generation");
    CHECK_EQ(state->panel.closeGeneration, 2,
             "final panel close generation");
    CHECK_EQ(state->panel.chestFirstSlot, 30,
             "final panel records C30 first chest slot");
    CHECK_EQ(state->panel.chestSlotProbeCount, 8,
             "final panel records eight chest slots");
    for (slotIndex = 0;
         slotIndex < DM1_V1_MIRROR_CANDIDATE_RCC_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++slotIndex) {
        CHECK_EQ(state->g0425ChestSlots[slotIndex], -1,
                 "final G0425 chest slots remain empty");
    }
    CHECK_EQ(state->champions[0].currentHealth, 40,
             "final A health");
    CHECK_EQ(state->champions[0].deathCount, 1,
             "final A death count");
    CHECK_EQ(state->champions[0].resurrectCount, 1,
             "final A resurrect count");
    CHECK_EQ(state->champions[1].currentHealth, 0,
             "final B remains dead");
    CHECK_EQ(state->champions[1].deathCount, 1,
             "final B death count");
    CHECK_EQ(state->champions[1].resurrectCount, 0,
             "final B resurrect count");
    CHECK_EQ(state->champions[1].candidateCloseCount, 1,
             "final B stale candidate close count");
    CHECK_EQ(state->champions[2].currentHealth, 42,
             "final C remains untouched");
    CHECK_EQ(state->champions[3].currentHealth, 43,
             "final D remains untouched");
    CHECK_NE(state->champions[0].portraitOrdinal,
             state->champions[1].portraitOrdinal,
             "final A and B portraits stay distinct");
    for (championIndex = 0;
         championIndex < DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT;
         ++championIndex) {
        CHECK_EQ(state->champions[championIndex].present, 1,
                 "final champion slot remains present");
        CHECK_EQ(state->champions[championIndex].portraitOrdinal,
                 20 + championIndex,
                 "final champion portrait identity remains stable");
        CHECK_EQ(state->champions[championIndex].redrawStateCount, 1,
                 "final settle redraws every champion once");
        for (slotIndex = 0;
             slotIndex < DM1_V1_MIRROR_CANDIDATE_RCC_SLOT_COUNT_PC34_COMPAT;
             ++slotIndex) {
            CHECK_EQ(state->champions[championIndex].slots[slotIndex],
                     100 * championIndex + slotIndex,
                     "final champion C00..C29 slots remain stable");
        }
    }
}

int main(void)
{
    Dm1V1MirrorCandidateRccStatePc34Compat state;
    Dm1V1MirrorCandidateRccStepPc34Compat steps[5];
    int stepCount;

    check_source_evidence();
    stepCount = DM1_V1_MirrorCandidateRcc_DriveRegressionPc34Compat(
        &state, steps, 5);
    check_step_shape(steps, stepCount);
    check_seed_stale_b(&steps[0]);
    check_close_stale_b(&steps[1]);
    check_queue_fresh_a(&steps[2]);
    check_resurrect_a(&steps[3]);
    check_settle_panel(&steps[4]);
    check_final_state(&state);

    if (gFailures == 0) {
        printf("PASS dm1_v1_mirror_candidate_resurrect_cross_candidate_clear_pc34_compat assertions=%d failures=0\n",
               gAssertions);
    } else {
        printf("FAIL dm1_v1_mirror_candidate_resurrect_cross_candidate_clear_pc34_compat assertions=%d failures=%d\n",
               gAssertions, gFailures);
    }
    return gFailures == 0 ? 0 : 1;
}
