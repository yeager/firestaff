/* ReDMCSB source-lock evidence:
 * CHEST.C F0333:31-67 opens/materializes G0425.
 * CHEST.C F0334:113-132 closes by scanning visible G0425 slots in order.
 * CHAMPION.C F0297:243-298 and F0298 preserve leader-hand identity.
 * CHAMPION.C F0300/F0301/F0302 own C30+ chest-slot mutation order.
 * COMMAND.C F0359:1985-1990 keeps C040 active behind G0299.
 * REVIVE.C F0280:124-132 and F0282:744-806 clear pending candidates.
 * BLITMASK.C F0133:30-33 is recorded as state-draw routing only.
 */
#include "dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat.h"

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

static void test_evidence_metadata(void)
{
    const Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat *e =
        M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "evidence metadata is available",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(strstr(e->chestOpenAnchor, "F0333:31-67") != NULL,
                  "evidence cites chest-open materialization",
                  e->chestOpenAnchor);
    CHECK_REDMCSB(strstr(e->chestCloseAnchor, "F0334:113-132") != NULL,
                  "evidence cites chest close rewrite",
                  e->chestCloseAnchor);
    CHECK_REDMCSB(strstr(e->leaderHandPutAnchor, "F0297:243-298") != NULL,
                  "evidence cites leader-hand put",
                  e->leaderHandPutAnchor);
    CHECK_REDMCSB(strstr(e->leaderHandRemoveAnchor, "F0298") != NULL,
                  "evidence cites leader-hand remove",
                  e->leaderHandRemoveAnchor);
    CHECK_REDMCSB(strstr(e->slotRemoveAddDispatchAnchor, "F0300/F0301/F0302") != NULL,
                  "evidence cites chest-slot remove/add/dispatch",
                  e->slotRemoveAddDispatchAnchor);
    CHECK_REDMCSB(strstr(e->championStateDrawAnchor, "F0292") != NULL,
                  "evidence cites champion state draw",
                  e->championStateDrawAnchor);
    CHECK_REDMCSB(strstr(e->candidateDispatchGuardAnchor, "F0359:1985-1990") != NULL,
                  "evidence cites C040 candidate guard",
                  e->candidateDispatchGuardAnchor);
    CHECK_REDMCSB(strstr(e->candidateClearAnchor, "F0280:124-132") != NULL,
                  "evidence cites candidate clear",
                  e->candidateClearAnchor);
    CHECK_REDMCSB(strstr(e->candidateFullChainClearAnchor, "F0282:744-806") != NULL,
                  "evidence cites full-chain candidate clear",
                  e->candidateFullChainClearAnchor);
    CHECK_REDMCSB(strstr(e->defsAnchor, "G0425") != NULL &&
                      strstr(e->defsAnchor, "G0426") != NULL,
                  "evidence cites required DEFS globals",
                  e->defsAnchor);
    CHECK_REDMCSB(strstr(e->blitRoutingAnchor, "F0133:30-33") != NULL,
                  "evidence cites blit routing",
                  e->blitRoutingAnchor);
    CHECK_REDMCSB(strstr(e->contractScope, "contract-only") != NULL,
                  "evidence records contract-only scope",
                  e->contractScope);
}

static void test_default_context(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat context;

    M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_InitContextPc34Compat(
        &context);

    CHECK_REDMCSB(context.chestAOpenThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_A_PC34_COMPAT,
                  "context starts with chest A open",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(context.chestBThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_B_PC34_COMPAT,
                  "context names chest B target",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(context.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "leader hand starts empty",
                  "CHAMPION.C F0298");
    CHECK_REDMCSB(context.leaderHandOwnerOrdinal == 2u,
                  "leader hand owner starts at inventory champion",
                  "DEFS.H G0423/M516");
    CHECK_REDMCSB(context.candidateChampionOrdinal == 3u,
                  "candidate champion C is pending",
                  "DEFS.H G0299");
    CHECK_REDMCSB(context.inventoryChampionOrdinal == 2u,
                  "inventory champion ordinal is tracked",
                  "DEFS.H G0423");
    CHECK_REDMCSB(context.partyChampionCount == 4u,
                  "party roster count is tracked",
                  "DEFS.H G0305");
    CHECK_REDMCSB(context.partyRoster[2] == 3u,
                  "party roster contains candidate champion C",
                  "DEFS.H M516");
    CHECK_REDMCSB(context.candidateCurrentHealth == 87,
                  "candidate current health is part of context",
                  "CHAMPION.C F0292");
    CHECK_REDMCSB(context.g0425[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT,
                  "G0425 slot 0 starts populated",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(context.g0425[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT,
                  "G0425 slot 1 starts populated",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(context.g0425[2] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT2_THING_PC34_COMPAT,
                  "G0425 slot 2 starts populated",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(context.g0425[4] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT4_THING_PC34_COMPAT,
                  "G0425 slot 4 starts populated",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(context.g0425[3] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "G0425 slot 3 starts empty",
                  "DEFS.H C0xFFFF_THING_NONE");
    CHECK_REDMCSB(context.chestBLink[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "chest B link is initially empty in the fixture",
                  "CHEST.C F0334:113-132");
}

static Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat
run_case(int caseId)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result;
    int accepted =
        M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_RunCasePc34Compat(
            caseId, &result);

    CHECK_REDMCSB(accepted == 1 && result.accepted == 1,
                  "scenario run is accepted",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.evidence != NULL,
                  "scenario carries evidence",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.log.chestBOpen == 1,
                  "scenario records chest B open",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.log.chestCloseMidPickup == 1,
                  "scenario records close mid-pickup",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.log.f0334CloseCount == 1,
                  "scenario closes exactly once",
                  "CHEST.C F0334:113-132");
    return result;
}

static void test_pending_pickup_close(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_PENDING_PICKUP_CLOSE_PC34_COMPAT);

    CHECK_REDMCSB(result.log.leaderHandPickupFromG0425 == 1,
                  "pickup from G0425 is logged",
                  "CHAMPION.C F0302");
    CHECK_REDMCSB(result.log.pickupSlotIndex == 0,
                  "pickup targets G0425[0]",
                  "DEFS.H C30_SLOT_CHEST_1/G0425");
    CHECK_REDMCSB(result.log.pickupCompleted == 1,
                  "pickup completes before the close result is observed",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.pickedThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT,
                  "picked thing identity is slot 0",
                  "CHAMPION.C F0300");
    CHECK_REDMCSB(result.final.leaderHandThingAfter == result.pickedThing,
                  "leader hand keeps the picked thing",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.final.candidateStillPending == 1,
                  "candidate remains pending",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.final.candidateChampionOrdinalAfter == 3u,
                  "candidate ordinal C is preserved",
                  "DEFS.H G0299");
    CHECK_REDMCSB(result.final.g0425[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "G0425[0] is C0xFFFF after close",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.g0426After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "G0426 is cleared by close",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT,
                  "close link begins with former visible slot 1",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT2_THING_PC34_COMPAT,
                  "close link preserves former visible slot 2",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[2] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT4_THING_PC34_COMPAT,
                  "close link preserves former visible slot 4",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLinkCount == 3,
                  "picked thing is absent from compacted link",
                  "CHEST.C F0334:113-132");
}

static void test_confirm_before_close(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_BEFORE_CLOSE_PC34_COMPAT);

    CHECK_REDMCSB(result.log.candidateConfirmBeforeClose == 1,
                  "candidate confirm fires before close",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.log.f0280CandidateClearCount == 1,
                  "candidate clears through F0280",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.log.f0282CandidateFullClearCount == 0,
                  "full-chain clear does not run in confirm-before case",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.log.noDoubleCandidateClear == 1,
                  "candidate clear is not doubled",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.final.candidateCleared == 1,
                  "candidate is cleared",
                  "DEFS.H G0299");
    CHECK_REDMCSB(result.final.partyChampionCountAfter == 4u,
                  "F0280 clear keeps party count stable",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.final.leaderHandThingAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT,
                  "leader hand has picked-up thing after clear",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.final.chestBLinkCount == 3,
                  "close recompacts without the picked-up thing",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[0] != result.pickedThing,
                  "picked-up thing is not first linked item",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.log.f0292ChampionStateDrawCount == 1,
                  "candidate clear redraws champion state once",
                  "CHAMPION.C F0292");
    CHECK_REDMCSB(result.log.f0133BlitRouteCount == 1,
                  "candidate clear records blit route once",
                  "BLITMASK.C F0133:30-33");
    CHECK_REDMCSB(result.final.candidatePanelOpenAfter == 0,
                  "C040 panel is closed after confirm",
                  "COMMAND.C F0359:1985-1990");
}

static void test_confirm_after_close(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_AFTER_CLOSE_PC34_COMPAT);

    CHECK_REDMCSB(result.log.candidateConfirmAfterClose == 1,
                  "candidate confirm fires after close",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.log.f0282CandidateFullClearCount == 1,
                  "full-chain clear runs once",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.log.f0280CandidateClearCount == 0,
                  "F0280 clear does not also run",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.log.noDoubleCandidateClear == 1,
                  "candidate clear remains single-shot",
                  "REVIVE.C F0282:744-806");
    CHECK_REDMCSB(result.log.chestReopen == 1,
                  "chest reopen is logged",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.log.f0333OpenCount == 2,
                  "open plus reopen are both counted",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.final.g0426After ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_B_PC34_COMPAT,
                  "G0426 points at reopened chest B",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.final.g0425[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT,
                  "reopened G0425[0] preserves visible order",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.final.g0425[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT2_THING_PC34_COMPAT,
                  "reopened G0425[1] preserves visible order",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.final.g0425[2] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT4_THING_PC34_COMPAT,
                  "reopened G0425[2] preserves visible order",
                  "CHEST.C F0333:31-67");
    CHECK_REDMCSB(result.final.visibleSlotOrderPreserved == 1,
                  "visible-slot order is preserved across close/reopen",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.partyChampionCountAfter == 3u,
                  "full-chain clear decrements party count",
                  "REVIVE.C F0282:744-806");
}

static void test_last_slot_guard(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_LAST_SLOT_GUARD_PC34_COMPAT);

    CHECK_REDMCSB(result.log.pickupSlotIndex == 7,
                  "pickup targets last visible chest slot",
                  "DEFS.H G0425");
    CHECK_REDMCSB(result.pickedThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT7_THING_PC34_COMPAT,
                  "slot 7 item is picked",
                  "CHAMPION.C F0300");
    CHECK_REDMCSB(result.final.leaderHandThingAfter == result.pickedThing,
                  "leader hand identity is preserved for slot 7 pickup",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.log.inventoryCandidateGuardHonored == 1,
                  "inventory-candidate guard is honored",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.final.guardHonoredForInventoryCandidate == 1,
                  "final state records guard honored",
                  "DEFS.H G0423/G0299");
    CHECK_REDMCSB(result.final.candidateStillPending == 1,
                  "candidate remains pending for inventory champion",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.log.f0280CandidateClearCount == 0 &&
                      result.log.f0282CandidateFullClearCount == 0,
                  "no candidate clear is fired",
                  "REVIVE.C F0280/F0282");
    CHECK_REDMCSB(result.final.noDoubleClear == 1,
                  "no double-clear is possible",
                  "REVIVE.C F0280:124-132");
    CHECK_REDMCSB(result.final.chestBLink[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT,
                  "close keeps earlier slot 0 before last-slot removal",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT,
                  "close keeps earlier slot 1 before last-slot removal",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLinkCount == 2,
                  "slot 7 picked item is omitted from close link",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.candidateCurrentHealthAfter == 87,
                  "candidate health is preserved while pending",
                  "CHAMPION.C F0292");
}

static void test_occupied_hand_swap(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_OCCUPIED_HAND_SWAP_PC34_COMPAT);

    CHECK_REDMCSB(result.before.leaderHandThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_LEADER_HAND_PC34_COMPAT,
                  "leader hand starts occupied",
                  "CHAMPION.C F0298");
    CHECK_REDMCSB(result.log.objectRemovedFromSlotCount == 1,
                  "F0300 slot remove occurs once",
                  "CHAMPION.C F0300");
    CHECK_REDMCSB(result.log.objectRemovedFromLeaderHandCount == 1,
                  "F0298 leader-hand remove occurs once",
                  "CHAMPION.C F0298");
    CHECK_REDMCSB(result.log.objectPutInLeaderHandCount == 1,
                  "F0297 leader-hand put occurs once",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.log.objectAddedToSlotCount == 1,
                  "F0301 adds previous hand item to chest slot",
                  "CHAMPION.C F0301");
    CHECK_REDMCSB(result.log.noF0300F0301SequenceReversal == 1,
                  "slot remove/add sequence is not reversed",
                  "CHAMPION.C F0300/F0301/F0302");
    CHECK_REDMCSB(result.swappedInThing ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_LEADER_HAND_PC34_COMPAT,
                  "previous leader-hand thing is swapped into G0425",
                  "CHAMPION.C F0301");
    CHECK_REDMCSB(result.final.leaderHandThingAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT,
                  "leader hand receives chest slot thing",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.final.chestBLink[0] == result.swappedInThing,
                  "close compacts the swapped hand thing",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.swapIdentityPreserved == 1,
                  "swap identity is preserved into the link",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.candidateStillPending == 1,
                  "candidate pending state is untouched",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.final.chestBLinkCount == 4,
                  "close compacts swapped item plus remaining chest items",
                  "CHEST.C F0334:113-132");
}

static void test_empty_pickup_noop(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_EMPTY_PICKUP_NOOP_PC34_COMPAT);

    CHECK_REDMCSB(result.before.g0425[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "fixture starts with empty G0425[0]",
                  "DEFS.H C0xFFFF_THING_NONE");
    CHECK_REDMCSB(result.log.emptyPickupNoop == 1,
                  "empty-hand empty-slot pickup no-ops",
                  "CHAMPION.C F0302");
    CHECK_REDMCSB(result.log.pickupCompleted == 0,
                  "no pickup completes for empty slot",
                  "CHAMPION.C F0302");
    CHECK_REDMCSB(result.final.leaderHandThingAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "leader hand remains empty",
                  "CHAMPION.C F0298");
    CHECK_REDMCSB(result.log.objectRemovedFromSlotCount == 0,
                  "no slot remove occurs",
                  "CHAMPION.C F0300");
    CHECK_REDMCSB(result.log.objectPutInLeaderHandCount == 0,
                  "no leader-hand put occurs",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.final.candidateStillPending == 1,
                  "candidate pending state is untouched",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.final.chestBLink[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT,
                  "close starts with first non-empty visible slot",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[1] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT2_THING_PC34_COMPAT,
                  "close keeps second non-empty visible slot",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[2] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT4_THING_PC34_COMPAT,
                  "close keeps later non-empty visible slot",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLinkCount == 3,
                  "close compacts only the remaining non-empty slots",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.g0425[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
                  "G0425 remains cleared after close",
                  "CHEST.C F0334:113-132");
}

static void test_rotated_inventory_owner(void)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result =
        run_case(
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_ROTATED_INVENTORY_OWNER_PC34_COMPAT);

    CHECK_REDMCSB(result.before.inventoryChampionOrdinal == 2u,
                  "fixture starts with old inventory champion",
                  "DEFS.H G0423");
    CHECK_REDMCSB(result.before.candidateChampionOrdinal == 4u,
                  "fixture has pending candidate for rotated champion",
                  "DEFS.H G0299");
    CHECK_REDMCSB(result.final.inventoryChampionOrdinalAfter == 4u,
                  "inventory champion rotates during close",
                  "DEFS.H G0423");
    CHECK_REDMCSB(result.final.leaderHandOwnerOrdinalAfter == 4u,
                  "pickup lands in the new leader hand owner",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.final.leaderHandThingAfter ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT,
                  "new leader hand receives picked thing",
                  "CHAMPION.C F0297:243-298");
    CHECK_REDMCSB(result.log.inventoryCandidateGuardHonored == 1,
                  "candidate guard is honored after rotation",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.final.guardHonoredForInventoryCandidate == 1,
                  "final state records rotated guard",
                  "DEFS.H G0423/G0299");
    CHECK_REDMCSB(result.final.candidateStillPending == 1,
                  "candidate remains pending for new inventory champion",
                  "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(result.final.candidateChampionOrdinalAfter == 4u,
                  "candidate ordinal follows the rotated champion",
                  "DEFS.H G0299");
    CHECK_REDMCSB(result.log.f0280CandidateClearCount == 0 &&
                      result.log.f0282CandidateFullClearCount == 0,
                  "rotation does not clear the candidate",
                  "REVIVE.C F0280/F0282");
    CHECK_REDMCSB(result.final.chestBLinkCount == 3,
                  "close link excludes picked item after rotation",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(result.final.chestBLink[0] ==
                      DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT,
                  "rotation preserves close order of remaining slots",
                  "CHEST.C F0334:113-132");
}

int main(void)
{
    int helperPassed;
    int helperFailed;

    test_evidence_metadata();
    test_default_context();
    test_pending_pickup_close();
    test_confirm_before_close();
    test_confirm_after_close();
    test_last_slot_guard();
    test_occupied_hand_swap();
    test_empty_pickup_noop();
    test_rotated_inventory_owner();

    CHECK_REDMCSB(
        dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat_run(
            &helperPassed, &helperFailed) == 1,
        "test-facing helper run passes",
        "COMMAND.C F0359:1985-1990");
    CHECK_REDMCSB(helperFailed == 0,
                  "test-facing helper has no failed self checks",
                  "CHEST.C F0334:113-132");
    CHECK_REDMCSB(helperPassed == 6,
                  "test-facing helper reports six self checks",
                  "REVIVE.C F0282:744-806");

    printf("assertions=%d\n", gTests);
    if (gPasses != gTests) {
        printf("FAIL dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat "
               "passed=%d/%d\n",
               gPasses,
               gTests);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat "
           "assertions=%d\n",
           gTests);
    return 0;
}
