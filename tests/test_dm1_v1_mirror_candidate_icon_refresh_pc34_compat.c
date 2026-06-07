#include "dm1_v1_mirror_candidate_icon_refresh_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock anchors tested here:
 * CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182,
 * CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262,
 * CHAMPION.C F0285_CHAMPION_GetIndexInCell:180-210,
 * COMMAND.C F0380 candidate gates:2158-2182 and 2302-2311,
 * DEFS.H F0295/F0296/F0297/F0298 prototypes:7915-7931.
 */

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
        printf("PASS: %s [%s]\n", msg, anchor); \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_suppressed_candidate_without_inventory(
    const Dm1V1MirrorCandidateIconRefreshResultPc34Compat *result,
    const Dm1V1MirrorCandidateIconRefreshEvidencePc34Compat *evidence)
{
    const char *anchor = evidence->chamdrawRefreshAnchor;

    CHECK_REDMCSB(result->contractOnly == 1,
                  "probe is explicitly contract-only",
                  evidence->contractScope);
    CHECK_REDMCSB(strstr(evidence->contractScope, "no real party") != NULL,
                  "evidence rejects real party/chest/inventory claims",
                  evidence->contractScope);
    CHECK_REDMCSB(result->suppressedByCandidateWithoutInventory == 1,
                  "G0299 with no inventory returns before icon probes",
                  anchor);
    CHECK_REDMCSB(result->processedWithInventoryOpen == 0,
                  "no-inventory candidate path does not process refresh loops",
                  anchor);
    CHECK_REDMCSB(result->candidateOrdinalBefore == 3 &&
                      result->candidateOrdinalAfter == 3,
                  "candidate ordinal is preserved by icon refresh suppression",
                  anchor);
    CHECK_REDMCSB(result->inventoryOrdinalBefore == 0 &&
                      result->inventoryOrdinalAfter == 0,
                  "inventory ordinal remains closed in suppressed path",
                  anchor);
    CHECK_REDMCSB(result->c040PanelBefore == 1 &&
                      result->c040PanelAfter == 1,
                  "C040 candidate panel is not closed by F0296 suppression",
                  anchor);
    CHECK_REDMCSB(result->leaderHandCurrentIconBefore ==
                      result->leaderHandCurrentIconAfter,
                  "leader-hand current icon is not refreshed",
                  anchor);
    CHECK_REDMCSB(result->leaderHandPointerIconBefore ==
                      result->leaderHandPointerIconAfter,
                  "leader-hand pointer icon is not rebuilt",
                  anchor);
    CHECK_REDMCSB(result->leaderHandNameDrawCountBefore ==
                      result->leaderHandNameDrawCountAfter,
                  "leader-hand object name is not redrawn",
                  anchor);
    CHECK_REDMCSB(result->mousePointerHiddenBefore == 1 &&
                      result->mousePointerHiddenAfter == 1,
                  "pre-existing mouse-hidden flag is not reset before return",
                  anchor);
    CHECK_REDMCSB(result->mouseScreenUpdatePairsBefore ==
                      result->mouseScreenUpdatePairsAfter,
                  "screen update pair is not opened",
                  evidence->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(result->partyStatusSlotRefreshCountBefore ==
                      result->partyStatusSlotRefreshCountAfter,
                  "party status slot boxes are not probed",
                  anchor);
    CHECK_REDMCSB(result->partyActionIconDrawCountBefore ==
                      result->partyActionIconDrawCountAfter,
                  "action icons are not redrawn",
                  anchor);
    CHECK_REDMCSB(result->inventorySlotRefreshCountBefore ==
                      result->inventorySlotRefreshCountAfter,
                  "inventory slot boxes are not probed",
                  anchor);
    CHECK_REDMCSB(result->chestSlotRefreshCountBefore ==
                      result->chestSlotRefreshCountAfter,
                  "chest slot boxes are not probed",
                  anchor);
    CHECK_REDMCSB(result->viewportDrawCountBefore ==
                      result->viewportDrawCountAfter,
                  "viewport redraw flag is not set",
                  anchor);
    CHECK_REDMCSB(result->earlyReturnCountAfter ==
                      result->earlyReturnCountBefore + 1,
                  "suppression is recorded as the F0296 early return",
                  anchor);
    CHECK_REDMCSB(result->partyLoopVisitsAfter == result->partyLoopVisitsBefore,
                  "party hand loop is skipped",
                  anchor);
    CHECK_REDMCSB(result->inventoryLoopVisitsAfter ==
                      result->inventoryLoopVisitsBefore,
                  "inventory loop is skipped",
                  anchor);
    CHECK_REDMCSB(result->chestLoopVisitsAfter == result->chestLoopVisitsBefore,
                  "chest loop is skipped",
                  anchor);
    CHECK_REDMCSB(result->candidateOrdinalClearedCountBefore ==
                      result->candidateOrdinalClearedCountAfter,
                  "icon refresh does not clear G0299",
                  evidence->commandCandidateGateAnchor);
    CHECK_REDMCSB(result->panelClearedCountBefore ==
                      result->panelClearedCountAfter,
                  "icon refresh does not clear C040 panel state",
                  evidence->commandCandidateGateAnchor);
    CHECK_REDMCSB(result->commandQueueMutationCountBefore ==
                      result->commandQueueMutationCountAfter,
                  "icon refresh does not mutate command queue state",
                  evidence->commandCandidateGateAnchor);
}

static void test_inventory_open_candidate_refresh(
    const Dm1V1MirrorCandidateIconRefreshResultPc34Compat *result,
    const Dm1V1MirrorCandidateIconRefreshEvidencePc34Compat *evidence)
{
    const char *anchor = evidence->chamdrawRefreshAnchor;

    CHECK_REDMCSB(result->processedWithInventoryOpen == 1,
                  "candidate path proceeds when an inventory ordinal is open",
                  anchor);
    CHECK_REDMCSB(result->suppressedByCandidateWithoutInventory == 0,
                  "open inventory bypasses the no-inventory early return",
                  anchor);
    CHECK_REDMCSB(result->candidateOrdinalBefore == 3 &&
                      result->candidateOrdinalAfter == 3,
                  "open-inventory refresh preserves G0299",
                  anchor);
    CHECK_REDMCSB(result->inventoryOrdinalBefore == 2 &&
                      result->inventoryOrdinalAfter == 2,
                  "open inventory ordinal is preserved",
                  anchor);
    CHECK_REDMCSB(result->c040PanelBefore == 1 &&
                      result->c040PanelAfter == 1,
                  "open-inventory refresh keeps C040 ownership",
                  anchor);
    CHECK_REDMCSB(result->leaderHandCurrentIconAfter == 148,
                  "leader-hand icon is refreshed before party loops",
                  anchor);
    CHECK_REDMCSB(result->leaderHandPointerIconAfter == 148,
                  "leader-hand mouse pointer bitmap is refreshed",
                  anchor);
    CHECK_REDMCSB(result->leaderHandNameDrawCountAfter ==
                      result->leaderHandNameDrawCountBefore + 1,
                  "leader-hand object name redraw is counted",
                  anchor);
    CHECK_REDMCSB(result->mouseScreenUpdatePairsAfter >
                      result->mouseScreenUpdatePairsBefore,
                  "mutable changed icon opens a screen update pair",
                  evidence->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(result->mousePointerHiddenAfter == 1,
                  "changed status-box icon sets the mouse-hidden flag",
                  evidence->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(result->partyStatusSlotRefreshCountAfter >
                      result->partyStatusSlotRefreshCountBefore,
                  "party status hand slots are refreshed",
                  anchor);
    CHECK_REDMCSB(result->partyActionIconDrawCountAfter >
                      result->partyActionIconDrawCountBefore,
                  "changed action-hand slots redraw action icons",
                  anchor);
    CHECK_REDMCSB(result->inventorySlotRefreshCountAfter >
                      result->inventorySlotRefreshCountBefore,
                  "inventory slots are refreshed when inventory is open",
                  anchor);
    CHECK_REDMCSB(result->chestSlotRefreshCountAfter >
                      result->chestSlotRefreshCountBefore,
                  "open chest slots are refreshed by the same pass",
                  anchor);
    CHECK_REDMCSB(result->viewportDrawCountAfter ==
                      result->viewportDrawCountBefore + 1,
                  "inventory/chest changes request one viewport redraw",
                  anchor);
    CHECK_REDMCSB(result->earlyReturnCountAfter ==
                      result->earlyReturnCountBefore,
                  "open inventory path does not use the early return",
                  anchor);
    CHECK_REDMCSB(result->partyLoopVisitsAfter >
                      result->partyLoopVisitsBefore,
                  "party hand loop runs for non-inventory champions",
                  anchor);
    CHECK_REDMCSB(result->inventoryLoopVisitsAfter == 8,
                  "inventory loop visits the contract slot snapshot",
                  anchor);
    CHECK_REDMCSB(result->chestLoopVisitsAfter == 8,
                  "chest loop visits the contract slot snapshot",
                  anchor);
    CHECK_REDMCSB(result->candidateOrdinalClearedCountBefore ==
                      result->candidateOrdinalClearedCountAfter,
                  "open-inventory icon refresh still does not clear G0299",
                  evidence->commandCandidateGateAnchor);
    CHECK_REDMCSB(result->panelClearedCountBefore ==
                      result->panelClearedCountAfter,
                  "open-inventory icon refresh still does not close C040",
                  evidence->commandCandidateGateAnchor);
    CHECK_REDMCSB(result->commandQueueMutationCountBefore ==
                      result->commandQueueMutationCountAfter,
                  "open-inventory icon refresh is not a command dispatch",
                  evidence->commandCandidateGateAnchor);
}

static void test_source_lock_metadata(
    const Dm1V1MirrorCandidateIconRefreshProbePc34Compat *probe)
{
    const Dm1V1MirrorCandidateIconRefreshEvidencePc34Compat *e =
        probe->evidence;

    CHECK_REDMCSB(e != NULL,
                  "probe returns evidence struct",
                  "DEFS.H F0295/F0296/F0297/F0298 prototypes:7915-7931");
    CHECK_REDMCSB(strstr(e->chamdrawIconProbeAnchor, "F0295") != NULL,
                  "F0295 anchor is present",
                  e->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(strstr(e->chamdrawRefreshAnchor, "F0296") != NULL,
                  "F0296 anchor is present",
                  e->chamdrawRefreshAnchor);
    CHECK_REDMCSB(strstr(e->championPartyLoopAnchor, "F0285") != NULL,
                  "CHAMPION.C party-loop anchor is present",
                  e->championPartyLoopAnchor);
    CHECK_REDMCSB(strstr(e->commandCandidateGateAnchor, "2158") != NULL,
                  "COMMAND.C candidate gate anchor is present",
                  e->commandCandidateGateAnchor);
    CHECK_REDMCSB(strstr(e->defsPrototypeAnchor, "7915") != NULL,
                  "DEFS.H prototype anchor is present",
                  e->defsPrototypeAnchor);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "click_cancel_front_cell") != NULL,
                  "disjoint from click-cancel front-cell contract",
                  e->disjointFunctions);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "close_button_pc34") != NULL,
                  "disjoint from C040 close-button contract",
                  e->disjointFunctions);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "ProcessPanelCommand") != NULL,
                  "disjoint from reincarnate panel command contract",
                  e->disjointFunctions);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "ProcessResurrect") != NULL,
                  "disjoint from resurrect rearm contract",
                  e->disjointFunctions);
    CHECK_REDMCSB(strstr(e->disjointFunctions, "ProcessStatusBoxClick") != NULL,
                  "disjoint from status-box click contracts",
                  e->disjointFunctions);
    CHECK_REDMCSB(probe->suppressed.mutableIconLowRange == 1,
                  "F0295 mutable low icon range is modeled",
                  e->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(probe->suppressed.mutableIconWeaponBoundaryRejected == 1,
                  "F0295 rejects icon 32 weapon boundary",
                  e->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(probe->suppressed.mutableIconPotionRange == 1,
                  "F0295 mutable potion range is modeled",
                  e->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(probe->suppressed.mutableIconEmptyFlask == 1,
                  "F0295 mutable empty flask icon is modeled",
                  e->chamdrawIconProbeAnchor);
    CHECK_REDMCSB(probe->suppressed.immutableIconRejected == 1,
                  "F0295 immutable icon is rejected",
                  e->chamdrawIconProbeAnchor);
}

int main(void)
{
    Dm1V1MirrorCandidateIconRefreshProbePc34Compat probe =
        DM1_V1_MirrorCandidateIconRefresh_ProbePc34Compat();

    test_source_lock_metadata(&probe);
    test_suppressed_candidate_without_inventory(&probe.suppressed,
                                                probe.evidence);
    test_inventory_open_candidate_refresh(&probe.inventoryOpen,
                                          probe.evidence);

    printf("dm1_v1_mirror_candidate_icon_refresh assertions=%d passes=%d\n",
           gTests,
           gPasses);
    return gTests == gPasses ? 0 : 1;
}
