#include "dm1_v1_mirror_candidate_resurrect_champion_switch_reopen_runtime_pc34_compat.h"

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

#define CHECK_CONTAINS(haystack, needle, msg) do { \
    ++gAssertions; \
    if ((haystack) == 0 || strstr((haystack), (needle)) == 0) { \
        ++gFailures; \
        printf("FAIL: %s missing='%s'\n", (msg), (needle)); \
    } \
} while (0)

static void check_source_evidence(void)
{
    const char *evidence = DM1_V1_MirrorCandidateRcsr_SourceEvidencePc34Compat();

    CHECK_CONTAINS(evidence, "REVIVE.C F0280:124-132",
                   "source evidence cites F0280 candidate set");
    CHECK_CONTAINS(evidence, "REVIVE.C F0282:744-806",
                   "source evidence cites F0282 panel route");
    CHECK_CONTAINS(evidence, "CHAMDRAW.C F0293:1117-1143",
                   "source evidence cites F0293 redraw");
    CHECK_CONTAINS(evidence, "CHAMDRAW.C F0291/F0296:551-552,1249-1252",
                   "source evidence cites F0291/F0296 chrome");
    CHECK_CONTAINS(evidence, "CHAMPION.C F0284:93-131",
                   "source evidence cites F0284 rotation");
    CHECK_CONTAINS(evidence, "CHAMPION.C F0297:243-298",
                   "source evidence cites F0297 hand put");
    CHECK_CONTAINS(evidence, "CHAMPION.C F0302:662-714",
                   "source evidence cites F0302 slot dispatch");
    CHECK_CONTAINS(evidence, "COMMAND.C F0359:1985-1990",
                   "source evidence cites C040 dispatch");
    CHECK_CONTAINS(evidence, "PANEL.C F0344/F0345:1895-1944,1946-1999",
                   "source evidence cites panel click/highlight");
    CHECK_CONTAINS(evidence, "PANEL.C F0354:2299-2322",
                   "source evidence cites champion switch");
    CHECK_CONTAINS(evidence, "PANEL.C F0352",
                   "source evidence cites pressing-eye route");
    CHECK_CONTAINS(evidence, "DEFS.H C30/G0425/G0426/M070/M516/C040",
                   "source evidence cites defs");
}

static void check_regression_steps(
    const Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    const Dm1V1MirrorCandidateRcsrStepPc34Compat *steps,
    int stepCount)
{
    int i;

    CHECK_EQ(stepCount, 4, "driver emits four runtime steps");
    CHECK_EQ(steps[0].stepId, 1, "step0 selects candidate");
    CHECK_EQ(steps[0].candidateOrdinalBefore, 0,
             "step0 starts with no G0299");
    CHECK_EQ(steps[0].candidateOrdinalAfter, 1,
             "step0 sets G0299 for champion 0");
    CHECK_EQ(steps[0].inventoryOrdinalAfter, 1,
             "step0 opens inventory on champion 0");
    CHECK_EQ(steps[0].panelChampionAfter, 0,
             "step0 C040 panel belongs to champion 0");
    CHECK_EQ(steps[0].routeF0293, 1,
             "step0 redraws all champion states");
    CHECK_EQ(steps[0].panelChromeStable, 1,
             "step0 has initialized panel chrome");

    CHECK_EQ(steps[1].stepId, 2, "step1 is first resurrect hand click");
    CHECK_EQ(steps[1].routeF0282, 1,
             "step1 routes through F0282");
    CHECK_EQ(steps[1].routeF0293, 1,
             "step1 redraws champion states");
    CHECK_EQ(steps[1].candidateOrdinalBefore, 1,
             "step1 sees original G0299");
    CHECK_EQ(steps[1].candidateOrdinalAfter, 1,
             "step1 preserves original G0299");
    CHECK_EQ(steps[1].inventoryOrdinalAfter, 1,
             "step1 keeps inventory on champion 0");
    CHECK_EQ(steps[1].panelChampionAfter, 0,
             "step1 keeps C040 on champion 0");
    CHECK_EQ(steps[1].g0299Preserved, 1,
             "step1 reports G0299 preserved");

    CHECK_EQ(steps[2].stepId, 3, "step2 is inventory champion switch");
    CHECK_EQ(steps[2].routeF0284, 1,
             "step2 routes through F0284");
    CHECK_EQ(steps[2].routeF0354, 1,
             "step2 routes through PANEL F0354/F0355");
    CHECK_EQ(steps[2].routeF0293, 1,
             "step2 redraws champion states");
    CHECK_EQ(steps[2].candidateOrdinalBefore, 1,
             "step2 starts with original G0299");
    CHECK_EQ(steps[2].candidateOrdinalAfter, 1,
             "step2 preserves original G0299");
    CHECK_EQ(steps[2].inventoryOrdinalBefore, 1,
             "step2 starts inventory on champion 0");
    CHECK_EQ(steps[2].inventoryOrdinalAfter, 3,
             "step2 switches inventory to champion 2");
    CHECK_EQ(steps[2].panelChampionBefore, 0,
             "step2 starts panel on champion 0");
    CHECK_EQ(steps[2].panelChampionAfter, 0,
             "step2 preserves original C040 panel candidate");
    CHECK_EQ(steps[2].g0299Preserved, 1,
             "step2 reports G0299 preserved");

    CHECK_EQ(steps[3].stepId, 4, "step3 reopens for a different champion");
    CHECK_EQ(steps[3].routeF0282, 1,
             "step3 routes through F0282");
    CHECK_EQ(steps[3].routeF0293, 1,
             "step3 redraws champion states");
    CHECK_EQ(steps[3].candidateOrdinalBefore, 1,
             "step3 starts from original candidate state");
    CHECK_EQ(steps[3].candidateOrdinalAfter, 3,
             "step3 opens G0299 for champion 2");
    CHECK_EQ(steps[3].inventoryOrdinalAfter, 3,
             "step3 keeps inventory on champion 2");
    CHECK_EQ(steps[3].panelChampionBefore, 0,
             "step3 starts from champion 0 C040 panel");
    CHECK_EQ(steps[3].panelChampionAfter, 2,
             "step3 reopens C040 panel for champion 2");
    CHECK_EQ(steps[3].panelChromeStable, 1,
             "step3 keeps panel chrome stable");

    CHECK_EQ(state->f0280CandidateSetCount, 1,
             "F0280 candidate set count");
    CHECK_EQ(state->f0282RouteCount, 2,
             "F0282 route count");
    CHECK_EQ(state->f0282ClearSkippedForHandIconCount, 2,
             "hand-icon clicks do not run final clear");
    CHECK_EQ(state->f0284SetPartyDirectionCount, 1,
             "F0284 direction switch count");
    CHECK_EQ(state->f0293DrawAllChampionStatesCount, 4,
             "F0293 redraw count for all panel states");
    CHECK_EQ(state->f0296DrawChangedObjectIconsCount, 4,
             "F0296 icon/panel chrome redraw count");
    CHECK_EQ(state->f0346DrawC040PanelCount, 3,
             "C040 panel draw count");
    CHECK_EQ(state->f0354ChampionSwitchCount, 1,
             "F0354 champion switch count");
    CHECK_EQ(state->f0355InventoryToggleCount, 1,
             "F0355 inventory toggle count");
    CHECK_EQ(state->f0359C040DispatchCount, 2,
             "F0359 M568/C040 dispatch count");
    CHECK_EQ(state->resurrectHandIconClickCount, 2,
             "resurrect hand icon click count");
    CHECK_EQ(state->preservedAcrossSwitch, 1,
             "original C040 candidate preserved across switch");
    CHECK_EQ(state->reopenedDifferentChampion, 1,
             "second click opens a different champion");
    CHECK_EQ(state->originalCandidateChampionIndex, 0,
             "original candidate champion index");
    CHECK_EQ(state->originalCandidateChampionOrdinal, 1,
             "original candidate champion ordinal");
    CHECK_EQ(state->candidateChampionOrdinal, 3,
             "final candidate champion ordinal");
    CHECK_EQ(state->inventoryChampionOrdinal, 3,
             "final inventory champion ordinal");
    CHECK_EQ(state->activePanelChampionIndex, 2,
             "final panel champion index");
    CHECK_EQ(state->partyDirection, 1,
             "party direction updated by F0284");

    for (i = 0; i < 3; ++i) {
        CHECK_EQ(state->champions[i].redrawStateCount, 4,
                 "each present champion receives F0293 redraw");
    }
    CHECK_EQ(state->champions[0].redrawPanelChromeCount, 2,
             "original candidate panel chrome drawn twice");
    CHECK_EQ(state->champions[2].redrawPanelChromeCount, 1,
             "reopened candidate panel chrome drawn once");
    CHECK_EQ(state->champions[0].redrawSlotCount, 60,
             "F0291 draws all champion 0 slots across two panels");
    CHECK_EQ(state->champions[2].redrawSlotCount, 30,
             "F0291 draws all champion 2 slots for reopened panel");
    CHECK_EQ(state->f0291DrawSlotCount, 90,
             "F0291 total slot draw count");
    CHECK_EQ(state->champions[0].cell, 1,
             "F0284 rotates champion 0 cell");
    CHECK_EQ(state->champions[1].cell, 2,
             "F0284 rotates champion 1 cell");
    CHECK_EQ(state->champions[2].cell, 3,
             "F0284 rotates champion 2 cell");
    CHECK_EQ(state->champions[0].direction, 1,
             "F0284 rotates champion 0 direction");
    CHECK_EQ(state->champions[1].direction, 1,
             "F0284 rotates champion 1 direction");
    CHECK_EQ(state->champions[2].direction, 1,
             "F0284 rotates champion 2 direction");
}

static void check_panel_chrome_and_slots(
    const Dm1V1MirrorCandidateRcsrStatePc34Compat *state)
{
    int i;

    CHECK_EQ(state->firstPanelChrome.panelContent, 568,
             "first panel content is M568");
    CHECK_EQ(state->reopenedPanelChrome.panelContent, 568,
             "reopened panel content is M568");
    CHECK_EQ(state->firstPanelChrome.panelGraphic, 40,
             "first panel uses C040 graphic");
    CHECK_EQ(state->reopenedPanelChrome.panelGraphic, 40,
             "reopened panel uses C040 graphic");
    CHECK_EQ(state->firstPanelChrome.panelBoxLeft,
             state->reopenedPanelChrome.panelBoxLeft,
             "panel chrome left edge stable");
    CHECK_EQ(state->firstPanelChrome.panelBoxTop,
             state->reopenedPanelChrome.panelBoxTop,
             "panel chrome top edge stable");
    CHECK_EQ(state->firstPanelChrome.panelBoxRight,
             state->reopenedPanelChrome.panelBoxRight,
             "panel chrome right edge stable");
    CHECK_EQ(state->firstPanelChrome.panelBoxBottom,
             state->reopenedPanelChrome.panelBoxBottom,
             "panel chrome bottom edge stable");
    CHECK_EQ(state->firstPanelChrome.panelByteWidth,
             state->reopenedPanelChrome.panelByteWidth,
             "panel byte width stable");
    CHECK_EQ(state->firstPanelChrome.transparentColor,
             state->reopenedPanelChrome.transparentColor,
             "panel transparent color stable");
    CHECK_EQ(state->firstPanelChrome.chestFirstSlot, 30,
             "first panel chrome records C30 chest first slot");
    CHECK_EQ(state->reopenedPanelChrome.chestFirstSlot, 30,
             "reopened panel chrome records C30 chest first slot");
    CHECK_EQ(state->firstPanelChrome.chestSlotProbeCount, 8,
             "first panel probes eight G0425 chest slots");
    CHECK_EQ(state->reopenedPanelChrome.chestSlotProbeCount, 8,
             "reopened panel probes eight G0425 chest slots");
    CHECK_EQ(state->g0426OpenChestThing, -1,
             "G0426 open chest remains none");
    for (i = 0; i < 8; ++i) {
        CHECK_EQ(state->g0425ChestSlots[i], -1,
                 "G0425 chest slot remains none");
    }
    for (i = 0; i < 30; ++i) {
        CHECK_EQ(state->champions[0].slots[i], i,
                 "champion 0 C00..C29 slot identity preserved");
    }
    for (i = 0; i < 30; ++i) {
        CHECK_EQ(state->champions[2].slots[i], 200 + i,
                 "champion 2 C00..C29 slot identity preserved");
    }
}

int main(void)
{
    Dm1V1MirrorCandidateRcsrStatePc34Compat state;
    Dm1V1MirrorCandidateRcsrStepPc34Compat steps[4];
    int stepCount;

    check_source_evidence();
    stepCount = DM1_V1_MirrorCandidateRcsr_DriveRegressionPc34Compat(
        &state, steps, 4);
    check_regression_steps(&state, steps, stepCount);
    check_panel_chrome_and_slots(&state);

    if (gFailures == 0) {
        printf("PASS dm1_v1_mirror_candidate_resurrect_champion_switch_reopen_runtime_pc34_compat assertions=%d failures=0\n",
               gAssertions);
    } else {
        printf("FAIL dm1_v1_mirror_candidate_resurrect_champion_switch_reopen_runtime_pc34_compat assertions=%d failures=%d\n",
               gAssertions, gFailures);
    }
    return gFailures == 0 ? 0 : 1;
}
