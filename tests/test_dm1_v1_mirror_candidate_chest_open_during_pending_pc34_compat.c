/* ReDMCSB source-lock evidence:
 * CHEST.C F0333:30-32 same-open early return keeps the currently open chest
 * and G0425_aT_ChestSlots untouched.
 * CHEST.C F0333:53-76 first-eight G0425 slot materialization runs only when
 * a fresh chest is opened.
 * COMMAND.C F0359:1985-1990 M568/C040 dispatch consumes the action-hand click.
 * REVIVE.C F0280:124-132 F0280 candidate admission requires
 *               G0415_ui_LeaderEmptyHanded and a free party slot.
 * REVIVE.C F0282:744-806 F0282 candidate clear runs on cancel/confirm.
 * CHAMPION.C F0297:243-267 leader-hand put.
 * CHAMPION.C F0298:270-298 leader-hand remove.
 * DEFS.H C30_SLOT_CHEST_1, G0425_aT_ChestSlots[8], G0426_T_OpenChest,
 *       M070_HAND_SLOT_INDEX, M516_CHAMPIONS, C040_COMMAND_… bindings.
 */
#include "dm1_v1_mirror_candidate_chest_open_during_pending_pc34_compat.h"

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

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateChestOpenDuringPending_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(strstr(e->chestSameOpenGuardAnchor, "F0333:30-32") != NULL,
                  "evidence cites same-open guard",
                  e->chestSameOpenGuardAnchor);
    CHECK_REDMCSB(strstr(e->chestFirstOpenMaterializeAnchor,
                         "F0333:53-76") != NULL,
                  "evidence cites first-eight materialization",
                  e->chestFirstOpenMaterializeAnchor);
    CHECK_REDMCSB(strstr(e->commandPanelDispatchAnchor,
                         "F0359:1985-1990") != NULL,
                  "evidence cites M568/C040 panel dispatch",
                  e->commandPanelDispatchAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidateAdmitAnchor,
                         "F0280:124-132") != NULL,
                  "evidence cites F0280 candidate admission guard",
                  e->reviveCandidateAdmitAnchor);
    CHECK_REDMCSB(strstr(e->reviveCandidateClearAnchor,
                         "F0282:744-806") != NULL,
                  "evidence cites F0282 candidate clear",
                  e->reviveCandidateClearAnchor);
    CHECK_REDMCSB(strstr(e->championHandPutAnchor, "F0297:243-267") != NULL,
                  "evidence cites F0297 leader-hand put",
                  e->championHandPutAnchor);
    CHECK_REDMCSB(strstr(e->championHandRemoveAnchor,
                         "F0298:270-298") != NULL,
                  "evidence cites F0298 leader-hand remove",
                  e->championHandRemoveAnchor);
    CHECK_REDMCSB(strstr(e->defsBindingsAnchor, "C30_SLOT_CHEST_1") != NULL &&
                      strstr(e->defsBindingsAnchor, "G0425_aT_ChestSlots") != NULL &&
                      strstr(e->defsBindingsAnchor, "G0426_T_OpenChest") != NULL &&
                      strstr(e->defsBindingsAnchor, "M070_HAND_SLOT_INDEX") != NULL &&
                      strstr(e->defsBindingsAnchor, "M516_CHAMPIONS") != NULL,
                  "evidence cites DEFS.H C30/G0425/G0426/M070/M516/C040 bindings",
                  e->defsBindingsAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence marks gate as contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->nonOverlapNote, "NEW C508 chest") != NULL &&
                      strstr(e->nonOverlapNote, "SAME-chest") != NULL &&
                      strstr(e->nonOverlapNote, "wall or open floor") != NULL,
                  "evidence records non-overlap with sibling gates",
                  e->nonOverlapNote);
}

static void test_fixture_starts_with_c040_open_and_prior_chest(void)
{
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat state;

    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);

    CHECK_REDMCSB(state.panelContent ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_C040_PANEL_PC34_COMPAT,
                  "fixture starts with C040 panel content (M568)",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.c040PanelOpen == 1,
                  "fixture has C040 panel open",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.candidateChampionOrdinal == 2u,
                  "fixture publishes a live candidate ordinal",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(state.partyChampionCount == 2u,
                  "fixture includes appended candidate in party count",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.leaderIndex == 0,
                  "fixture has a leader for the leader-hand invariant",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(state.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "fixture leader hand starts empty (-1 sentinel)",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(state.priorOpenChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
                  "fixture records the prior chest that opened C040",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(state.newOpenChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
                  "fixture records the new chest for Scenario A",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentOpenChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
                  "fixture has the prior chest as currently open",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(state.currentChestSlots[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT0_PC34_COMPAT,
                  "fixture chest slot 0 holds prior slot thing",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT1_PC34_COMPAT,
                  "fixture chest slot 1 holds prior slot thing",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[2] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT2_PC34_COMPAT,
                  "fixture chest slot 2 holds prior slot thing",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[3] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT3_PC34_COMPAT,
                  "fixture chest slot 3 holds prior slot thing",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[4] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT,
                  "fixture chest slot 4 starts empty",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[5] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT,
                  "fixture chest slot 5 starts empty",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[6] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT,
                  "fixture chest slot 6 starts empty",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.currentChestSlots[7] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT,
                  "fixture chest slot 7 starts empty",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.f0333FirstOpenMaterializeCount == 0,
                  "fixture starts with no F0333 first-open count",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.f0333SameOpenEarlyReturnCount == 0,
                  "fixture starts with no F0333 same-open count",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(state.f0333NoOpenCount == 0,
                  "fixture starts with no F0333 no-open count",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.commandClickConsumeCount == 0,
                  "fixture starts with no command click consume count",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(state.candidateClearCount == 0,
                  "fixture starts with no F0282 candidate clear count",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.partyDecrementCount == 0,
                  "fixture starts with no party decrement count",
                  "REVIVE.C F0282:744-806");
}

static void test_scenario_a_new_chest_opens_under_pending_c040(void)
{
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat state;
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);
    accepted = DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "action-hand click on a new chest is accepted",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.clickConsumed == 1,
                  "COMMAND.C F0359 M568 dispatch consumed the click",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.commandClickConsumeCountAfter ==
                      result.commandClickConsumeCountBefore + 1,
                  "M568 click consume count increments once",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.dispatchedF0282 == 1,
                  "F0282 candidate clear dispatched",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.dispatchedF0333 == 1,
                  "F0333 first-open path dispatched",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333PathTaken ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_FIRST_OPEN_PC34_COMPAT,
                  "F0333 takes the first-open materialization path",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.candidateClearCountAfter ==
                      result.candidateClearCountBefore + 1,
                  "F0282 candidate clear count increments once",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.partyDecrementCountAfter ==
                      result.partyDecrementCountBefore + 1,
                  "F0282 candidate clear decrements party count",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.partyCountAfter == 1u,
                  "party count drops from two to one after F0282 clear",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.candidateOrdinalAfter == 0,
                  "F0282 clears the G0299 candidate ordinal",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.c040OpenAfter == 0,
                  "C040 panel state closes after click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.panelContentAfter == 0,
                  "M568 panel content is reset after click",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateCleared == 1,
                  "result reports candidate cleared in one step",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.panelClosed == 1,
                  "result reports C040 panel closed in one step",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.currentOpenChestAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
                  "G0426_T_OpenChest is set to the new chest thing",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333FirstOpenMaterializeCountAfter ==
                      result.f0333FirstOpenMaterializeCountBefore + 1,
                  "F0333 first-open count increments once",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333FirstOpenMaterializeSlotsAfter -
                          result.f0333FirstOpenMaterializeSlotsBefore ==
                      8,
                  "F0333 first-open writes eight G0425 slots",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333FirstOpenFirstSlotWriteCountAfter ==
                      result.f0333FirstOpenFirstSlotWriteCountBefore + 1,
                  "F0333 first-open writes the first slot once",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333FirstOpenRelinkCountAfter ==
                      result.f0333FirstOpenRelinkCountBefore + 7,
                  "F0333 first-open relinks the seven remaining slots",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot0After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT0_PC34_COMPAT,
                  "G0425[0] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot1After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT1_PC34_COMPAT,
                  "G0425[1] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot2After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT2_PC34_COMPAT,
                  "G0425[2] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot3After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT3_PC34_COMPAT,
                  "G0425[3] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot4After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT4_PC34_COMPAT,
                  "G0425[4] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot5After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT5_PC34_COMPAT,
                  "G0425[5] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot6After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT6_PC34_COMPAT,
                  "G0425[6] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.slot7After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT7_PC34_COMPAT,
                  "G0425[7] is materialized from the new chest",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.g0425MaterializedFromNewChest == 1,
                  "result summarizes new-chest materialization",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333SameOpenEarlyReturnCountAfter ==
                      result.f0333SameOpenEarlyReturnCountBefore,
                  "F0333 same-open guard is not taken on a new chest",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.leaderHandAfter == result.leaderHandBefore &&
                      result.leaderHandAfter ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "leader hand is still empty after the click",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(result.leaderHandMutationObserved == 0,
                  "F0297 was not invoked",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(result.f0297LeaderHandPutCountAfter ==
                      result.f0297LeaderHandPutCountBefore,
                  "F0297 leader-hand put count is unchanged",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(result.f0298LeaderHandRemoveCountAfter ==
                      result.f0298LeaderHandRemoveCountBefore,
                  "F0298 leader-hand remove count is unchanged",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(result.clickConsumedByCandidateClear == 1,
                  "click was consumed by the F0282 candidate-clear path",
                  "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.clickConsumedByChestOpen == 1,
                  "click was consumed by the F0333 chest-open path",
                  "COMMAND.C F0359:1985-1990; CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333NoOpenCountAfter ==
                      result.f0333NoOpenCountBefore,
                  "F0333 no-open counter is unchanged on new-chest click",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.priorOpenChestAfter == result.priorOpenChestBefore,
                  "the prior chest thing in state is preserved",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.newOpenChestAfter == result.newOpenChestBefore,
                  "the new chest thing in state is preserved",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(state.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "state keeps the leader hand empty",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(state.c040PanelOpen == 0 && state.candidateChampionOrdinal == 0u,
                  "state confirms F0282 candidate clear",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.currentOpenChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT,
                  "state confirms F0333 first-open landed",
                  "CHEST.C F0333:53-76");
}

static void test_scenario_b_same_chest_early_return_under_pending_c040(void)
{
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat state;
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);
    accepted = DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "action-hand click on the same chest is accepted",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.clickConsumed == 1,
                  "M568 dispatch consumed the same-chest click",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.commandClickConsumeCountAfter ==
                      result.commandClickConsumeCountBefore + 1,
                  "M568 click consume count increments once",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.dispatchedF0282 == 1,
                  "F0282 candidate clear dispatched",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.dispatchedF0333 == 1,
                  "F0333 same-open early-return path dispatched",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333PathTaken ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SAME_OPEN_GUARD_PC34_COMPAT,
                  "F0333 takes the same-open early-return path",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333SameOpenEarlyReturnCountAfter ==
                      result.f0333SameOpenEarlyReturnCountBefore + 1,
                  "F0333 same-open early-return count increments once",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333FirstOpenMaterializeCountAfter ==
                      result.f0333FirstOpenMaterializeCountBefore,
                  "F0333 first-open materialization does not run on same chest",
                  "CHEST.C F0333:30-32; CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333FirstOpenMaterializeSlotsAfter ==
                      result.f0333FirstOpenMaterializeSlotsBefore,
                  "F0333 same-open path writes zero G0425 slots",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333FirstOpenFirstSlotWriteCountAfter ==
                      result.f0333FirstOpenFirstSlotWriteCountBefore,
                  "F0333 same-open path skips the first-slot write",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333FirstOpenRelinkCountAfter ==
                      result.f0333FirstOpenRelinkCountBefore,
                  "F0333 same-open path skips the relink loop",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.currentOpenChestAfter == result.currentOpenChestBefore,
                  "G0426_T_OpenChest is unchanged on same-chest click",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.slot0After == result.slot0Before &&
                      result.slot1After == result.slot1Before &&
                      result.slot2After == result.slot2Before &&
                      result.slot3After == result.slot3Before,
                  "G0425[0..3] is preserved on same-chest click",
                      "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.slot4After == result.slot4Before &&
                      result.slot5After == result.slot5Before &&
                      result.slot6After == result.slot6Before &&
                      result.slot7After == result.slot7Before,
                  "G0425[4..7] is preserved on same-chest click",
                      "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.g0425PreservedFromPriorChest == 1,
                  "result summarizes same-open G0425 preservation",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.candidateClearCountAfter ==
                      result.candidateClearCountBefore + 1,
                  "F0282 candidate clear still runs on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.partyDecrementCountAfter ==
                      result.partyDecrementCountBefore + 1,
                  "F0282 still decrements party count on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.c040OpenAfter == 0,
                  "C040 panel state closes on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.candidateOrdinalAfter == 0,
                  "G0299 candidate ordinal is cleared on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.leaderHandAfter == result.leaderHandBefore &&
                      result.leaderHandAfter ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "leader hand stays empty on same-chest click",
                      "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(result.leaderHandMutationObserved == 0,
                  "F0297 was not invoked on same-chest click",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(result.f0297LeaderHandPutCountAfter ==
                      result.f0297LeaderHandPutCountBefore,
                  "F0297 leader-hand put count is unchanged on same-chest click",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(result.f0298LeaderHandRemoveCountAfter ==
                      result.f0298LeaderHandRemoveCountBefore,
                  "F0298 leader-hand remove count is unchanged on same-chest click",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(result.clickConsumedByCandidateClear == 1,
                  "click was consumed by the F0282 candidate-clear path on same chest",
                  "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.clickConsumedByChestOpen == 1,
                  "click was consumed by the F0333 same-open path",
                  "COMMAND.C F0359:1985-1990; CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333NoOpenCountAfter ==
                      result.f0333NoOpenCountBefore,
                  "F0333 no-open counter is unchanged on same-chest click",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.candidateCleared == 1,
                  "result reports candidate cleared on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.panelClosed == 1,
                  "result reports C040 panel closed on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "state keeps the leader hand empty on same-chest click",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(state.c040PanelOpen == 0,
                  "state confirms C040 panel closed on same-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.currentOpenChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
                  "state confirms G0426 is still the prior chest thing",
                      "CHEST.C F0333:30-32");
}

static void test_scenario_c_non_chest_cell_click_under_pending_c040(void)
{
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat state;
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat result;
    int accepted;

    DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(&state);
    accepted = DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "action-hand click on a non-chest cell is accepted",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.clickConsumed == 1,
                  "M568 dispatch consumed the non-chest click",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.commandClickConsumeCountAfter ==
                      result.commandClickConsumeCountBefore + 1,
                  "M568 click consume count increments once",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.dispatchedF0282 == 1,
                  "F0282 candidate clear dispatched on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.dispatchedF0333 == 0,
                  "F0333 is not dispatched on non-chest click",
                  "CHEST.C F0333:30-32; CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333PathTaken ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT,
                  "F0333 records non-chest cell path",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.f0333NoOpenCountAfter ==
                      result.f0333NoOpenCountBefore + 1,
                  "F0333 no-open count increments once",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.f0333SameOpenEarlyReturnCountAfter ==
                      result.f0333SameOpenEarlyReturnCountBefore,
                  "F0333 same-open guard is not taken on non-chest click",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.f0333FirstOpenMaterializeCountAfter ==
                      result.f0333FirstOpenMaterializeCountBefore,
                  "F0333 first-open materialization is not taken on non-chest click",
                  "CHEST.C F0333:53-76");
    CHECK_REDMCSB(result.f0333FirstOpenMaterializeSlotsAfter ==
                      result.f0333FirstOpenMaterializeSlotsBefore,
                  "F0333 no-open path writes zero G0425 slots",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.f0333FirstOpenFirstSlotWriteCountAfter ==
                      result.f0333FirstOpenFirstSlotWriteCountBefore,
                  "F0333 no-open path skips the first-slot write",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.f0333FirstOpenRelinkCountAfter ==
                      result.f0333FirstOpenRelinkCountBefore,
                  "F0333 no-open path skips the relink loop",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.currentOpenChestAfter == result.currentOpenChestBefore,
                  "G0426_T_OpenChest is unchanged on non-chest click",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.slot0After == result.slot0Before &&
                      result.slot1After == result.slot1Before &&
                      result.slot2After == result.slot2Before &&
                      result.slot3After == result.slot3Before,
                  "G0425[0..3] is preserved on non-chest click",
                      "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.slot4After == result.slot4Before &&
                      result.slot5After == result.slot5Before &&
                      result.slot6After == result.slot6Before &&
                      result.slot7After == result.slot7Before,
                  "G0425[4..7] is preserved on non-chest click",
                      "CHEST.C F0333:30-32");
    CHECK_REDMCSB(result.noChestOpened == 1,
                  "result summarizes non-chest no-op",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateClearCountAfter ==
                      result.candidateClearCountBefore + 1,
                  "F0282 candidate clear still runs on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.partyDecrementCountAfter ==
                      result.partyDecrementCountBefore + 1,
                  "F0282 still decrements party count on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.c040OpenAfter == 0,
                  "C040 panel state closes on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.candidateOrdinalAfter == 0,
                  "G0299 candidate ordinal is cleared on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.leaderHandAfter == result.leaderHandBefore &&
                      result.leaderHandAfter ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "leader hand stays empty on non-chest click",
                      "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(result.leaderHandMutationObserved == 0,
                  "F0297 was not invoked on non-chest click",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(result.f0297LeaderHandPutCountAfter ==
                      result.f0297LeaderHandPutCountBefore,
                  "F0297 leader-hand put count is unchanged on non-chest click",
                  "CHAMPION.C F0297:243-267");
    CHECK_REDMCSB(result.f0298LeaderHandRemoveCountAfter ==
                      result.f0298LeaderHandRemoveCountBefore,
                  "F0298 leader-hand remove count is unchanged on non-chest click",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(result.clickConsumedByCandidateClear == 1,
                  "click was consumed by the F0282 candidate-clear path on non-chest cell",
                  "COMMAND.C F0359:1985-1990; REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.clickConsumedByChestOpen == 0,
                  "click was NOT consumed by F0333 on non-chest cell",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.candidateCleared == 1,
                  "result reports candidate cleared on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.panelClosed == 1,
                  "result reports C040 panel closed on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT,
                  "state keeps the leader hand empty on non-chest click",
                  "CHAMPION.C F0298:270-298");
    CHECK_REDMCSB(state.c040PanelOpen == 0,
                  "state confirms C040 panel closed on non-chest click",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(state.currentOpenChestThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT,
                  "state confirms G0426 is still the prior chest thing on non-chest click",
                      "CHEST.C F0333:30-32");
    CHECK_REDMCSB(state.currentChestSlots[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT0_PC34_COMPAT &&
                      state.currentChestSlots[3] ==
                          DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT3_PC34_COMPAT,
                  "state keeps prior chest slots intact on non-chest click",
                      "CHEST.C F0333:30-32");
}

static void test_embedded_self_check(void)
{
    int passed = 0;
    int failed = 0;
    int ok = dm1_v1_mirror_candidate_chest_open_during_pending_run(&passed, &failed);

    CHECK_REDMCSB(ok == 1,
                  "embedded self-check passes",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(failed == 0,
                  "embedded self-check reports no failures",
                  "CHEST.C F0333:30-32");
    CHECK_REDMCSB(passed >= 18,
                  "embedded self-check exercises all three scenarios",
                  "COMMAND.C F0359:1985-1990");
}

int main(void)
{
    test_source_lock_metadata();
    test_fixture_starts_with_c040_open_and_prior_chest();
    test_scenario_a_new_chest_opens_under_pending_c040();
    test_scenario_b_same_chest_early_return_under_pending_c040();
    test_scenario_c_non_chest_cell_click_under_pending_c040();
    test_embedded_self_check();

    if (gTests != gPasses) {
        printf("%d/%d assertions passed\n", gPasses, gTests);
        return 1;
    }
    printf("%d assertions passed\n", gPasses);
    return 0;
}
