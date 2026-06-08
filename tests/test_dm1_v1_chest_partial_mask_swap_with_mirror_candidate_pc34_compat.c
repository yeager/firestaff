/* ReDMCSB source-lock evidence:
 * CHEST.C F0333:30-67, F0334:113-132 cover open G0425 and close recompact.
 * CHAMPION.C F0297/F0298:243-298, F0300/F0301:511-515,606-614, and
 * F0302:688-710 cover leader hand, C30+ chest slots, and encumbrance.
 * COMMAND.C F0359:1985-1990 covers M568/C040 input ownership.
 * CHAMDRAW.C F0293:1117-1143 covers candidate boundary redraw.
 * REVIVE.C F0282:744-806 covers pending candidate clear.
 * OBJECT.C F0033:147-212 covers object pointer/name refresh.
 * BLITMASK.C F0133:30-33 covers partial-mask dispatch.
 */
#include "dm1/dm1_v1_chest_partial_mask_swap_with_mirror_candidate_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34
run_case(int caseId)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result;

    (void)dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
        caseId, &result);
    CHECK_REDMCSB(result.evidence != NULL,
                  "case carries evidence metadata",
                  "CHEST.C F0333:30-67");
    CHECK_REDMCSB(result.requestedPc34Slot ==
                      DM1_V1_CPSWMC_TARGET_PC34_SLOT_PC34,
                  "case records C30+ target slot",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.requestedChestSlotIndex ==
                      DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34,
                  "case records G0425 target index",
                  "CHAMPION.C F0300:511-515");
    return result;
}

static void test_evidence_metadata(void)
{
    const Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34 *e =
        dm1_v1_chest_partial_mask_swap_with_mirror_candidate_evidence_pc34();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "CHEST.C F0333:30-67");
    CHECK_REDMCSB(strstr(e->chestOpenAnchor, "F0333:30-67") != NULL,
                  "evidence cites chest open dispatch",
                  e->chestOpenAnchor);
    CHECK_REDMCSB(strstr(e->chestCloseAnchor, "F0334:113-132") != NULL,
                  "evidence cites chest close rewrite",
                  e->chestCloseAnchor);
    CHECK_REDMCSB(strstr(e->leaderHandAnchor, "F0297/F0298:243-298") != NULL,
                  "evidence cites leader hand put/remove",
                  e->leaderHandAnchor);
    CHECK_REDMCSB(strstr(e->chestSlotRemoveAnchor, "F0300:511-515") != NULL,
                  "evidence cites C30+ remove path",
                  e->chestSlotRemoveAnchor);
    CHECK_REDMCSB(strstr(e->chestSlotAddAnchor, "F0301:606-614") != NULL,
                  "evidence cites C30+ add path",
                  e->chestSlotAddAnchor);
    CHECK_REDMCSB(strstr(e->encumbranceAnchor, "F0302:688-710") != NULL,
                  "evidence cites F0302 encumbrance route",
                  e->encumbranceAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelGateAnchor, "F0359:1985-1990") != NULL,
                  "evidence cites M568/C040 panel gate",
                  e->commandPanelGateAnchor);
    CHECK_REDMCSB(strstr(e->championStateRedrawAnchor, "F0293:1117-1143") != NULL,
                  "evidence cites champion state redraw",
                  e->championStateRedrawAnchor);
    CHECK_REDMCSB(strstr(e->candidateClearAnchor, "F0282:744-806") != NULL,
                  "evidence cites pending candidate clear",
                  e->candidateClearAnchor);
    CHECK_REDMCSB(strstr(e->objectRefreshAnchor, "F0033:147-212") != NULL,
                  "evidence cites object pointer/name refresh",
                  e->objectRefreshAnchor);
    CHECK_REDMCSB(strstr(e->partialMaskAnchor, "F0133:30-33") != NULL,
                  "evidence cites partial-mask dispatch",
                  e->partialMaskAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence declares contract-only scope",
                  e->contractScope);
}

static void test_fixture_defaults(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 state;

    dm1_v1_chest_partial_mask_swap_with_mirror_candidate_init_pc34(&state);

    CHECK_REDMCSB(state.panelContent == DM1_V1_CPSWMC_PANEL_M568_C040_PC34,
                  "fixture starts with M568/C040 panel content",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "fixture has candidate panel up",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.candidateChampionOrdinal ==
                      DM1_V1_CPSWMC_CANDIDATE_ORDINAL_PC34,
                  "fixture has pending mirror candidate",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.openChestThing == DM1_V1_CPSWMC_CHEST_THING_PC34,
                  "fixture has G0426 open chest",
                  "CHEST.C F0333:30-67");
    CHECK_REDMCSB(state.g0425[DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34] ==
                      DM1_V1_CPSWMC_SLOT0_ITEM_PC34 +
                          DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34,
                  "fixture target G0425 slot is populated",
                  "CHEST.C F0333:53-67");
    CHECK_REDMCSB(state.leaderHandThing == DM1_V1_CPSWMC_LEADER_ITEM_PC34,
                  "fixture leader hand starts occupied",
                  "CHAMPION.C F0297/F0298:243-298");
    CHECK_REDMCSB((state.leaderHandAllowedMask &
                       DM1_PC34_ALLOWED_CONTAINER) != 0,
                  "fixture leader item has container mask overlap",
                  "CHAMPION.C F0302:697-710");
    CHECK_REDMCSB(state.leaderHandAllowedMask != DM1_PC34_ALLOWED_CONTAINER,
                  "fixture leader item uses a partial mask, not exact mask",
                  "BLITMASK.C F0133:30-33");
}

static void test_confirm_clears_candidate_and_recompacts(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result =
        run_case(DM1_V1_CPSWMC_CASE_CONFIRM_PC34);

    CHECK_REDMCSB(result.accepted == 1,
                  "confirm scenario accepts partial-mask swap",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.candidateWasActive == 1 && result.chestWasOpen == 1,
                  "confirm scenario starts with candidate and open chest",
                  "CHEST.C F0333:30-67; REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.f0359PanelGateDelta == 1,
                  "M568/C040 gate is evaluated",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.partialMaskDispatched == 1 &&
                      result.f0133PartialMaskDispatchDelta == 1,
                  "partial-mask dispatch is recorded",
                  "BLITMASK.C F0133:30-33");
    CHECK_REDMCSB(result.maskOverlap == DM1_PC34_ALLOWED_CONTAINER &&
                      result.maskExactMatch == 0,
                  "partial mask overlaps C30+ chest mask without exact match",
                  "CHAMPION.C F0302:697-710");
    CHECK_REDMCSB(result.f0302DispatchDelta == 1 &&
                      result.f0302SwapDelta == 1,
                  "F0302 dispatches exactly one swap",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.f0298LeaderHandRemoveDelta == 1 &&
                      result.f0297LeaderHandPutDelta == 1,
                  "leader hand remove/put refreshes once",
                  "CHAMPION.C F0297/F0298:243-298");
    CHECK_REDMCSB(result.f0300ChestSlotRemoveDelta == 1 &&
                      result.f0301ChestSlotAddDelta == 1,
                  "C30+ remove/add path uses G0425",
                  "CHAMPION.C F0300/F0301:511-515,606-614");
    CHECK_REDMCSB(result.leaderHandAfter == result.slotBefore,
                  "leader hand receives the former chest slot item",
                  "CHAMPION.C F0302:704-706");
    CHECK_REDMCSB(result.f0302EncumbranceRefreshDelta == 1,
                  "swap refreshes champion encumbrance",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.objectPointerRefreshDelta == 1 &&
                      result.objectNameRefreshDelta == 1,
                  "swap refreshes object pointer and name",
                  "OBJECT.C F0033:147-212");
    CHECK_REDMCSB(result.candidateClearDelta == 1 &&
                      result.candidateAfter == 0u,
                  "confirm clears pending candidate",
                  "REVIVE.C F0282:785-806");
    CHECK_REDMCSB(result.partyCountAfter == result.partyCountBefore - 1,
                  "confirm decrements pending candidate party count",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.f0293RedrawAllDelta == 1,
                  "confirm redraws champion states at candidate boundary",
                  "CHAMDRAW.C F0293:1117-1143");
    CHECK_REDMCSB(result.f0292DrawStateDelta >= 2,
                  "swap plus candidate clear redraw champion state",
                  "CHAMDRAW.C F0293:1117-1143");
    CHECK_REDMCSB(result.f0334CloseDelta == 1,
                  "confirm closes the open chest once",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.g0425RecompactDelta ==
                      DM1_V1_CPSWMC_SLOT_COUNT_PC34,
                  "confirm recompacts all non-empty G0425 entries",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.openChestAfter == DM1_V1_CPSWMC_NONE_PC34,
                  "confirm close clears G0426",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(result.closedChainCount == DM1_V1_CPSWMC_SLOT_COUNT_PC34,
                  "closed chain keeps eight visible entries",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.closedChain[DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34] ==
                      DM1_V1_CPSWMC_LEADER_ITEM_PC34,
                  "closed chain contains the swapped partial-mask item",
                  "CHEST.C F0334:123-132");
}

static void test_cancel_clears_candidate_without_swap(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result =
        run_case(DM1_V1_CPSWMC_CASE_CANCEL_PC34);

    CHECK_REDMCSB(result.accepted == 1 && result.candidateOwnedInput == 1,
                  "cancel is owned by the C040 candidate panel",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateClearDelta == 1 &&
                      result.candidateAfter == 0u,
                  "cancel clears pending candidate",
                  "REVIVE.C F0282:744-757");
    CHECK_REDMCSB(result.g0425Unchanged == 1,
                  "cancel leaves G0425 unchanged",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.leaderHandPreserved == 1,
                  "cancel preserves leader hand",
                  "CHAMPION.C F0297/F0298:243-298");
    CHECK_REDMCSB(result.f0133PartialMaskDispatchDelta == 0,
                  "cancel does not dispatch partial-mask blit",
                  "BLITMASK.C F0133:30-33");
    CHECK_REDMCSB(result.f0302SwapDelta == 0,
                  "cancel does not swap through F0302",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.f0334CloseDelta == 0 &&
                      result.openChestAfter == result.openChestBefore,
                  "cancel does not close the open chest in this compound gate",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.f0293RedrawAllDelta == 1,
                  "cancel redraws at candidate boundary",
                  "CHAMDRAW.C F0293:1117-1143");
}

static void test_no_candidate_uses_ordinary_partial_mask_route(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result =
        run_case(DM1_V1_CPSWMC_CASE_NO_CANDIDATE_PC34);

    CHECK_REDMCSB(result.accepted == 1 && result.ordinaryRoute == 1,
                  "no-candidate scenario takes ordinary chest slot route",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.candidateWasActive == 0 &&
                      result.candidateClearDelta == 0,
                  "ordinary route has no pending candidate clear",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.partialMaskDispatched == 1,
                  "ordinary route still dispatches partial-mask icon path",
                  "BLITMASK.C F0133:30-33");
    CHECK_REDMCSB(result.slotAfter == DM1_V1_CPSWMC_LEADER_ITEM_PC34,
                  "ordinary route writes leader item into G0425 slot",
                  "CHAMPION.C F0301:606-614");
    CHECK_REDMCSB(result.leaderHandAfter == result.slotBefore,
                  "ordinary route puts old slot item in leader hand",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.openChestAfter == result.openChestBefore,
                  "ordinary route leaves chest open",
                  "CHEST.C F0333:30-67");
    CHECK_REDMCSB(result.objectPointerRefreshDelta == 1 &&
                      result.objectNameRefreshDelta == 1,
                  "ordinary route refreshes object pointer/name",
                  "OBJECT.C F0033:147-212");
}

static void test_cross_champion_candidate_swap(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result =
        run_case(DM1_V1_CPSWMC_CASE_CROSS_CHAMPION_PC34);

    CHECK_REDMCSB(result.accepted == 1 && result.crossChampionRoute == 1,
                  "cross-champion route accepts partial-mask swap",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.selectedChampionBefore == 1 &&
                      result.selectedChampionAfter == 1,
                  "cross-champion route targets champion 1",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.candidateWasActive == 1 &&
                      result.candidateAfter == result.candidateBefore,
                  "candidate remains pending after cross-champion swap",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.selectedLoadAfter != result.selectedLoadBefore,
                  "selected champion encumbrance changes",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.leaderLoadAfter != result.leaderLoadBefore,
                  "leader-hand owner load refreshes",
                  "CHAMPION.C F0297/F0298:243-298");
    CHECK_REDMCSB(result.f0334CloseDelta == 0,
                  "cross-champion swap does not close chest",
                  "CHEST.C F0334:113-132");
}

static void test_empty_hand_candidate_rejects_without_side_effects(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result =
        run_case(DM1_V1_CPSWMC_CASE_EMPTY_HAND_CANDIDATE_PC34);

    CHECK_REDMCSB(result.accepted == 0 && result.rejected == 1,
                  "empty-hand candidate scenario rejects slot input",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateOwnedInput == 1,
                  "C040 owns input while leader hand is empty",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.noSideEffects == 1,
                  "empty-hand rejection has no side effects",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.g0425Unchanged == 1,
                  "empty-hand rejection preserves G0425",
                  "CHEST.C F0333:53-67");
    CHECK_REDMCSB(result.leaderHandPreserved == 1,
                  "empty-hand rejection preserves leader hand",
                  "CHAMPION.C F0297/F0298:243-298");
    CHECK_REDMCSB(result.f0302SwapDelta == 0 &&
                      result.f0133PartialMaskDispatchDelta == 0,
                  "empty-hand rejection dispatches no swap or partial mask",
                  "CHAMPION.C F0302:688-710; BLITMASK.C F0133:30-33");
    CHECK_REDMCSB(result.candidateAfter == result.candidateBefore,
                  "empty-hand rejection keeps candidate pending",
                  "REVIVE.C F0282:744-806");
}

static void test_closed_chest_candidate_rejects_without_side_effects(void)
{
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 result =
        run_case(DM1_V1_CPSWMC_CASE_CLOSED_CHEST_CANDIDATE_PC34);

    CHECK_REDMCSB(result.accepted == 0 && result.rejected == 1,
                  "closed-chest candidate scenario rejects slot input",
                  "CHEST.C F0333:30-67");
    CHECK_REDMCSB(result.chestWasOpen == 0 &&
                      result.openChestBefore == DM1_V1_CPSWMC_NONE_PC34,
                  "closed-chest scenario starts without G0426",
                  "CHEST.C F0334:113-116");
    CHECK_REDMCSB(result.candidateOwnedInput == 1,
                  "candidate owns input while chest slot is not available",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.noSideEffects == 1,
                  "closed-chest rejection has no side effects",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.g0425Unchanged == 1,
                  "closed-chest rejection preserves G0425",
                  "CHEST.C F0334:117-132");
    CHECK_REDMCSB(result.f0302DispatchDelta == 0 &&
                      result.f0302SwapDelta == 0,
                  "closed-chest rejection dispatches no F0302 swap",
                  "CHAMPION.C F0302:688-710");
    CHECK_REDMCSB(result.objectPointerRefreshDelta == 0 &&
                      result.objectNameRefreshDelta == 0,
                  "closed-chest rejection has no object refresh",
                  "OBJECT.C F0033:147-212");
}

static void test_self_check_entrypoint(void)
{
    int passed = 0;
    int failed = 0;
    int ok =
        dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_pc34(
            &passed, &failed);

    CHECK_REDMCSB(ok == 1,
                  "contract-only self check passes",
                  "CHEST.C F0333:30-67");
    CHECK_REDMCSB(passed >= 10 && failed == 0,
                  "contract-only self check records passing probes",
                  "CHEST.C F0334:113-132");
}

int main(void)
{
    test_evidence_metadata();
    test_fixture_defaults();
    test_confirm_clears_candidate_and_recompacts();
    test_cancel_clears_candidate_without_swap();
    test_no_candidate_uses_ordinary_partial_mask_route();
    test_cross_champion_candidate_swap();
    test_empty_hand_candidate_rejects_without_side_effects();
    test_closed_chest_candidate_rejects_without_side_effects();
    test_self_check_entrypoint();

    CHECK_REDMCSB(gTests >= 40,
                  "minimum assertion count is met",
                  "CHEST.C F0333:30-67");
    printf("assertions=%d\n", gTests);
    if (gPasses != gTests) {
        printf("FAIL dm1_v1_chest_partial_mask_swap_with_mirror_candidate_pc34_compat "
               "passed=%d/%d\n",
               gPasses,
               gTests);
        return 1;
    }
    printf("PASS dm1_v1_chest_partial_mask_swap_with_mirror_candidate_pc34_compat "
           "assertions=%d\n",
           gTests);
    return 0;
}
