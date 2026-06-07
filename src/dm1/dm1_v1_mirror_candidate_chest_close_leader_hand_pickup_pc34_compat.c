#include "dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors:
 * CHEST.C F0333:31-67 materializes the current chest links into G0425.
 * CHEST.C F0334:113-132 clears G0426 and rewrites non-empty G0425 slots.
 * CHAMPION.C F0297:243-298 / F0298 move identity through the leader hand.
 * CHAMPION.C F0300/F0301/F0302 remove/add/dispatch C30+ chest slots.
 * COMMAND.C F0359:1985-1990 keeps M568/C040 pending behind G0299.
 * REVIVE.C F0280:124-132 clears a pending candidate without a double clear.
 * REVIVE.C F0282:744-806 owns the full-chain portrait/state clear.
 */

enum {
    kCandidateC = 3,
    kInventoryChampion = 2,
    kRotatedInventoryChampion = 4,
    kInitialPartyCount = 4,
    kCandidateHealth = 87
};

static const Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat
    s_evidence = {
        "ReDMCSB CHEST.C F0333:31-67 open materialization into G0425",
        "ReDMCSB CHEST.C F0334:113-132 close clears G0426 and rewrites "
        "non-empty G0425 slots",
        "ReDMCSB CHAMPION.C F0297:243-298 leader-hand put with charges, "
        "weight, and load refresh",
        "ReDMCSB CHAMPION.C F0298 leader-hand remove",
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 C30 chest slot remove/add "
        "and slot-box dispatch",
        "ReDMCSB CHAMPION.C F0292 champion state draw",
        "ReDMCSB COMMAND.C F0359:1985-1990 M568/C040 dispatch guarded by "
        "G0299_ui_CandidateChampionOrdinal",
        "ReDMCSB REVIVE.C F0280:124-132 pending candidate clear",
        "ReDMCSB REVIVE.C F0282:744-806 full-chain candidate clear plus "
        "portrait/state redraw",
        "ReDMCSB DEFS.H:2088/C30/G0425/G0426/G0299/G0305/G0423/M516",
        "ReDMCSB BLITMASK.C F0133:30-33 champion state blit routing",
        "contract-only synthetic runtime; no M11 graphics, framebuffer, "
        "asset, savegame, or dungeon IO is performed"
    };

static int is_none(int thing)
{
    return thing ==
           DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
}

static void fill_none(int values[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        values[i] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    }
}

static void copy_slots(int dst[], const int src[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

void M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_InitContextPc34Compat(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context)
{
    if (!context) {
        return;
    }

    memset(context, 0, sizeof(*context));
    context->chestAOpenThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_A_PC34_COMPAT;
    context->chestBThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_B_PC34_COMPAT;
    context->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    context->leaderHandOwnerOrdinal = kInventoryChampion;
    context->candidateChampionOrdinal = kCandidateC;
    context->inventoryChampionOrdinal = kInventoryChampion;
    context->partyChampionCount = kInitialPartyCount;
    context->partyRoster[0] = 1u;
    context->partyRoster[1] = kInventoryChampion;
    context->partyRoster[2] = kCandidateC;
    context->partyRoster[3] = kRotatedInventoryChampion;
    context->candidateCurrentHealth = kCandidateHealth;
    context->g0425[0] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT;
    context->g0425[1] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT;
    context->g0425[2] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT2_THING_PC34_COMPAT;
    context->g0425[3] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    context->g0425[4] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT4_THING_PC34_COMPAT;
    context->g0425[5] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    context->g0425[6] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    context->g0425[7] =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    fill_none(context->chestBLink,
              DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT);
    fill_none(context->closeVisitOrder,
              DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT);
}

static void open_chest_b(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log)
{
    log->chestBOpen = 1;
    ++log->f0333OpenCount;
}

static void draw_candidate_state(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log)
{
    ++log->f0292ChampionStateDrawCount;
    ++log->f0133BlitRouteCount;
}

static void clear_candidate_f0280(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log)
{
    if (context->candidateChampionOrdinal != 0u) {
        context->candidateChampionOrdinal = 0u;
        log->candidateClear = 1;
        ++log->f0280CandidateClearCount;
        draw_candidate_state(log);
    }
}

static void clear_candidate_full_chain_f0282(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log)
{
    if (context->candidateChampionOrdinal != 0u) {
        context->candidateChampionOrdinal = 0u;
        log->candidateClear = 1;
        log->candidateFullChainClear = 1;
        ++log->f0282CandidateFullClearCount;
        if (context->partyChampionCount > 0u) {
            --context->partyChampionCount;
        }
        draw_candidate_state(log);
    }
}

static int pickup_from_g0425(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log,
    int slotIndex,
    int *pickedThing,
    int *swappedInThing)
{
    int slotThing;

    if (!context || slotIndex < 0 ||
        slotIndex >=
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT) {
        return 0;
    }

    log->leaderHandPickupFromG0425 = 1;
    log->pickupSlotIndex = slotIndex;
    ++log->c040DispatchGuardCount;
    slotThing = context->g0425[slotIndex];
    if (is_none(slotThing) && is_none(context->leaderHandThing)) {
        log->emptyPickupNoop = 1;
        if (pickedThing) {
            *pickedThing =
                DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
        }
        if (swappedInThing) {
            *swappedInThing =
                DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
        }
        return 0;
    }

    ++log->objectRemovedFromSlotCount;
    if (!is_none(context->leaderHandThing)) {
        ++log->objectRemovedFromLeaderHandCount;
    }
    context->g0425[slotIndex] = context->leaderHandThing;
    context->leaderHandThing = slotThing;
    ++log->objectPutInLeaderHandCount;
    if (!is_none(context->g0425[slotIndex])) {
        ++log->objectAddedToSlotCount;
    }
    log->pickupCompleted = 1;
    log->noF0300F0301SequenceReversal = 1;
    if (pickedThing) {
        *pickedThing = slotThing;
    }
    if (swappedInThing) {
        *swappedInThing = context->g0425[slotIndex];
    }
    return 1;
}

static void close_chest_b_f0334(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log)
{
    int i;

    ++log->f0334CloseCount;
    log->chestCloseMidPickup = 1;
    log->closeVisitCount = 0;
    log->closeLinkCount = 0;
    fill_none(context->chestBLink,
              DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT);
    fill_none(context->closeVisitOrder,
              DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT);

    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (!is_none(context->g0425[i])) {
            context->closeVisitOrder[log->closeVisitCount] = i;
            context->chestBLink[log->closeLinkCount] = context->g0425[i];
            ++log->closeVisitCount;
            ++log->closeLinkCount;
        }
        context->g0425[i] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    }
    context->chestAOpenThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
}

static void reopen_chest_b_f0333(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat *log)
{
    int i;

    log->chestReopen = 1;
    ++log->f0333OpenCount;
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        context->g0425[i] = context->chestBLink[i];
    }
    context->chestAOpenThing = context->chestBThing;
}

static void complete_result(
    const Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat
        *context,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat *result)
{
    int i;
    int expectedOrderIndex;

    copy_slots(result->final.chestBLink,
               context->chestBLink,
               DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT);
    copy_slots(result->final.g0425,
               context->g0425,
               DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT);
    result->final.chestBLinkCount = result->log.closeLinkCount;
    result->final.leaderHandThingAfter = context->leaderHandThing;
    result->final.leaderHandOwnerOrdinalAfter = context->leaderHandOwnerOrdinal;
    result->final.candidateChampionOrdinalAfter =
        context->candidateChampionOrdinal;
    result->final.inventoryChampionOrdinalAfter =
        context->inventoryChampionOrdinal;
    result->final.partyChampionCountAfter = context->partyChampionCount;
    result->final.candidateCurrentHealthAfter = context->candidateCurrentHealth;
    result->final.candidatePanelOpenAfter =
        context->candidateChampionOrdinal != 0u;
    result->final.g0426After = context->chestAOpenThing;
    result->final.candidateStillPending =
        context->candidateChampionOrdinal != 0u;
    result->final.candidateCleared = context->candidateChampionOrdinal == 0u;
    result->final.pickedThingRemovedFromG0425 =
        result->pickedThing ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT ||
        context->g0425[result->log.pickupSlotIndex] != result->pickedThing;
    result->final.swapIdentityPreserved =
        result->swappedInThing ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT ||
        result->final.chestBLink[0] == result->swappedInThing;
    result->final.guardHonoredForInventoryCandidate =
        result->log.inventoryCandidateGuardHonored;
    result->log.noDoubleCandidateClear =
        result->log.f0280CandidateClearCount +
            result->log.f0282CandidateFullClearCount <=
        1;
    result->final.noDoubleClear = result->log.noDoubleCandidateClear;

    expectedOrderIndex = 0;
    result->final.visibleSlotOrderPreserved = 1;
    for (i = 0;
         i <
         DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (!is_none(result->before.g0425[i]) && result->before.g0425[i] !=
                                                  result->pickedThing) {
            if (context->chestBLink[expectedOrderIndex] !=
                result->before.g0425[i]) {
                result->final.visibleSlotOrderPreserved = 0;
            }
            ++expectedOrderIndex;
        }
    }
}

static void apply_case_fixture(
    int caseId,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context)
{
    if (caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_LAST_SLOT_GUARD_PC34_COMPAT) {
        context->candidateChampionOrdinal = kInventoryChampion;
        context->inventoryChampionOrdinal = kInventoryChampion;
        context->g0425[0] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT;
        context->g0425[1] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT;
        context->g0425[2] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
        context->g0425[4] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
        context->g0425[7] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT7_THING_PC34_COMPAT;
    } else if (
        caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_OCCUPIED_HAND_SWAP_PC34_COMPAT) {
        context->leaderHandThing =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_LEADER_HAND_PC34_COMPAT;
    } else if (
        caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_EMPTY_PICKUP_NOOP_PC34_COMPAT) {
        context->g0425[0] =
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    } else if (
        caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_ROTATED_INVENTORY_OWNER_PC34_COMPAT) {
        context->candidateChampionOrdinal = kRotatedInventoryChampion;
        context->inventoryChampionOrdinal = kInventoryChampion;
    }
}

int M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_RunCasePc34Compat(
    int caseId,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result;
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat context;
    int pickupSlotIndex;
    int reopenAfterClose;

    memset(&result, 0, sizeof(result));
    result.evidence = &s_evidence;
    result.caseId = caseId;
    M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_InitContextPc34Compat(
        &context);
    apply_case_fixture(caseId, &context);
    result.before = context;
    result.log.pickupSlotIndex = 0;
    result.pickedThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;
    result.swappedInThing =
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT;

    if (caseId <
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_PENDING_PICKUP_CLOSE_PC34_COMPAT ||
        caseId >
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_ROTATED_INVENTORY_OWNER_PC34_COMPAT) {
        if (outResult) {
            *outResult = result;
        }
        return 0;
    }

    pickupSlotIndex =
        caseId ==
                DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_LAST_SLOT_GUARD_PC34_COMPAT
            ? 7
            : 0;
    reopenAfterClose =
        caseId ==
            DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_AFTER_CLOSE_PC34_COMPAT;

    open_chest_b(&result.log);

    if (caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_BEFORE_CLOSE_PC34_COMPAT) {
        result.log.candidateConfirmBeforeClose = 1;
        clear_candidate_f0280(&context, &result.log);
    }

    (void)pickup_from_g0425(
        &context, &result.log, pickupSlotIndex, &result.pickedThing,
        &result.swappedInThing);

    if (caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_ROTATED_INVENTORY_OWNER_PC34_COMPAT) {
        context.inventoryChampionOrdinal = kRotatedInventoryChampion;
        context.leaderHandOwnerOrdinal = kRotatedInventoryChampion;
    }

    if (context.inventoryChampionOrdinal == context.candidateChampionOrdinal &&
        context.candidateChampionOrdinal != 0u) {
        result.log.inventoryCandidateGuardHonored = 1;
    }

    close_chest_b_f0334(&context, &result.log);

    if (caseId ==
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_AFTER_CLOSE_PC34_COMPAT) {
        result.log.candidateConfirmAfterClose = 1;
        clear_candidate_full_chain_f0282(&context, &result.log);
    }

    if (reopenAfterClose) {
        reopen_chest_b_f0333(&context, &result.log);
    }

    result.accepted = 1;
    complete_result(&context, &result);
    if (outResult) {
        *outResult = result;
    }
    return 1;
}

const Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat *
M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_EvidencePc34Compat(void)
{
    return &s_evidence;
}

static void self_check(int condition, int *passed, int *failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

int dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat_run(
    int *passed,
    int *failed)
{
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat result;

    if (!passed || !failed) {
        return 0;
    }
    *passed = 0;
    *failed = 0;

    (void)M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_RunCasePc34Compat(
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_PENDING_PICKUP_CLOSE_PC34_COMPAT,
        &result);
    self_check(result.log.pickupCompleted == 1, passed, failed);
    self_check(result.final.candidateStillPending == 1, passed, failed);
    self_check(result.final.g0425[0] ==
                   DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT,
               passed,
               failed);

    (void)M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_RunCasePc34Compat(
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_AFTER_CLOSE_PC34_COMPAT,
        &result);
    self_check(result.final.candidateCleared == 1, passed, failed);
    self_check(result.final.visibleSlotOrderPreserved == 1, passed, failed);
    self_check(result.log.chestReopen == 1, passed, failed);
    return *failed == 0;
}
