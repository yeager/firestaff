#include "dm1_v1_mirror_candidate_thought_project_cancel_after_pickup_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * COMMAND.C F0378:1956-1994 maps C081 panel clicks to C160/C161/C162 while
 * M568 is visible, and F0380:2045-2159 preserves queued command identity.
 * CHEST.C F0333:30-67 copies an open chest into G0425/C30+ visible slots,
 * while F0334:113-132 clears G0426 and relinks non-empty G0425 entries.
 * CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand put/remove;
 * F0300:511-584, F0301:606-660, and F0302:662-713 own C30+ slot exchange.
 * REVIVE.C F0280:124-132 publishes the candidate only when leader hand and
 * party capacity allow it; F0282:744-806 handles C162 cancel cleanup.
 * PANEL.C F0346/F0347:1619-1657 redraws C040 while G0299 is non-zero.
 * UTAMSCR.C F0077/F0078:141-150 brackets mouse redraw updates.
 * DEFS.H:338-340, 810-817, 1874-1878, 2200, 3001-3008, 5694, 5876-5881 names
 * C162, C30..C37, C38, C040, M568/M569, G0299, G0425, and G0426.
 */

enum {
    kPartyCount = 3,
    kLeaderIndex = 0,
    kCandidateOrdinal = 3,
    kThoughtCellX = 11,
    kThoughtCellY = 7,
    kOpenChestThing = 0x6400,
    kThoughtScrollThing = 0x7157,
    kPreviousCellThing = 0x5120,
    kFirstChestThing = 0x7200,
    kPickedSlotIndex = 2,
    kCounterCount = 12
};

static const Dm1V1MirrorCancelAfterPickupEvidencePc34Compat s_evidence = {
    1,
    "COMMAND.C F0378:1956-1994 M568 C160/C161/C162 panel dispatch",
    "COMMAND.C F0380:2045-2159 queued command identity and panel/status guard",
    "CHEST.C F0333:30-67 G0426 open and G0425/C30+ materialization",
    "CHEST.C F0334:113-132 G0426 close and non-empty G0425 relink",
    "CHAMPION.C F0297:243-268 leader-hand put/load redraw",
    "CHAMPION.C F0298:270-298 leader-hand remove/load redraw",
    "CHAMPION.C F0300:511-584 C30+ slot removal",
    "CHAMPION.C F0301:606-660 C30+ slot add",
    "CHAMPION.C F0302:662-713 slot-box dispatch and swap order",
    "REVIVE.C F0280:124-132 candidate add gate",
    "REVIVE.C F0282:744-806 C162 cancel clears G0299 and party tail",
    "PANEL.C F0346/F0347:1619-1657 C040 redraw while G0299 is set",
    "UTAMSCR.C F0077/F0078:141-150 mouse update bracket",
    "DEFS.H:338-340 C160/C161/C162; 810-817 C30..C37; "
        "1874-1878 C38/M070; 2200 C040; 3001-3008 M568/M569; "
        "5694/5876-5881 G0299/G0423/G0425/G0426",
    "contract_only=1 DM1 V1 mirror-candidate thought-project "
        "scroll-pickup followed by ESC/C162 cancel; distinct from pass674 "
        "scroll-pickup+rotation+inventory-click and pass686 occupied-slot browse"
};

static const Dm1V1MirrorCancelAfterPickupSpecPc34Compat s_spec = {
    kLeaderIndex,
    kCandidateOrdinal,
    kThoughtCellX,
    kThoughtCellY,
    kOpenChestThing,
    kThoughtScrollThing,
    kPickedSlotIndex,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C30_SLOT_CHEST_1_PC34_COMPAT +
        kPickedSlotIndex,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C38_SLOT_BOX_CHEST_1_PC34_COMPAT +
        kPickedSlotIndex,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C040_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M569_PANEL_PC34_COMPAT
};

static const char s_source_evidence[] =
    "COMMAND.C F0378:1956-1994 routes M568 panel input to C160/C161/C162\n"
    "COMMAND.C F0380:2045-2159 preserves queued command identity before dispatch\n"
    "CHEST.C F0333:30-67 opens G0426 and copies chest links into G0425/C30+\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 entries\n"
    "CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand put/remove\n"
    "CHAMPION.C F0300:511-584 and F0301:606-660 own slot remove/add\n"
    "CHAMPION.C F0302:662-713 dispatches C30+ chest slot exchanges\n"
    "REVIVE.C F0280:124-132 opens candidate; F0282:744-806 cancels C162\n"
    "PANEL.C F0346/F0347:1619-1657 redraws C040 while G0299 is set\n"
    "UTAMSCR.C F0077/F0078:141-150 bracket mouse redraw updates\n"
    "DEFS.H C162/C30..C37/C38/C040/M568/M569/G0299/G0425/G0426";

static void copy_text(char *dst, const char *src, int dstBytes)
{
    if (!dst || dstBytes <= 0) {
        return;
    }
    if (!src) {
        src = "";
    }
    strncpy(dst, src, (size_t)dstBytes - 1u);
    dst[dstBytes - 1] = '\0';
}

static void copy_slots(int dst[], const int src[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        dst[i] = src[i];
    }
}

static void capture_counters(
    const Dm1V1MirrorCancelAfterPickupStatePc34Compat *state,
    int counters[])
{
    counters[0] = state->projectDispatchCount;
    counters[1] = state->pickupDispatchCount;
    counters[2] = state->cancelDispatchCount;
    counters[3] = state->f0333OpenCount;
    counters[4] = state->f0334CloseCount;
    counters[5] = state->f0297PutCount;
    counters[6] = state->f0298RemoveCount;
    counters[7] = state->f0300SlotRemoveCount;
    counters[8] = state->f0301SlotAddCount;
    counters[9] = state->f0302SlotDispatchCount;
    counters[10] = state->f0280OpenCount;
    counters[11] = state->f0282CancelCount;
}

static int unrelated_slots_preserved(
    const int before[],
    const int after[],
    int pickedSlot)
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (i != pickedSlot && before[i] != after[i]) {
            return 0;
        }
    }
    return 1;
}

static void seed_chest_slots(int slots[])
{
    int i;

    for (i = 0;
         i < DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        slots[i] = kFirstChestThing + i;
    }
    slots[kPickedSlotIndex] = kThoughtScrollThing;
}

void DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_InitPc34Compat(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyCount = kPartyCount;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->panelOpen = 1;
    state->panelContent =
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT;
    state->openChestThing = kOpenChestThing;
    state->cellX = kThoughtCellX;
    state->cellY = kThoughtCellY;
    state->cellThing = kPreviousCellThing;
    state->previousCellThing = kPreviousCellThing;
    state->thoughtThing = kThoughtScrollThing;
    seed_chest_slots(state->chestSlots);
    state->f0333OpenCount = 1;
    state->f0280OpenCount = 1;
    state->panelRedrawCount = 1;
    copy_text(state->thoughtText,
              "FUL BRO NETA scroll pickup thought",
              sizeof(state->thoughtText));
}

static int project_thought(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state)
{
    if (!state->panelOpen ||
        state->g0299CandidateOrdinal != state->candidateOrdinal ||
        state->cellThing != state->previousCellThing) {
        return 0;
    }
    state->cellThing = state->thoughtThing;
    state->thoughtProjected = 1;
    ++state->projectDispatchCount;
    ++state->panelRedrawCount;
    return 1;
}

static int scroll_pickup(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state)
{
    if (!state->thoughtProjected ||
        state->leaderHandThing !=
            DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT ||
        state->chestSlots[kPickedSlotIndex] != state->thoughtThing) {
        return 0;
    }
    ++state->mouseEnableCount;
    ++state->f0302SlotDispatchCount;
    ++state->pickupDispatchCount;
    ++state->f0300SlotRemoveCount;
    state->chestSlots[kPickedSlotIndex] =
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    state->leaderHandThing = state->thoughtThing;
    ++state->f0297PutCount;
    state->thoughtPickedUp = 1;
    ++state->mouseDisableCount;
    return 1;
}

static int cancel_after_pickup(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state)
{
    if (!state->thoughtPickedUp ||
        state->leaderHandThing != state->thoughtThing ||
        state->g0299CandidateOrdinal != state->candidateOrdinal) {
        return 0;
    }
    ++state->mouseEnableCount;
    ++state->cancelDispatchCount;
    ++state->f0282CancelCount;
    ++state->f0298RemoveCount;
    state->leaderHandThing =
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    state->leaderHandWasEmptiedByCancel = 1;
    state->cellThing = state->previousCellThing;
    state->thoughtProjected = 0;
    state->thoughtPickedUp = 0;
    state->thoughtRestoredByCancel = 1;
    state->g0299CandidateOrdinal = 0;
    state->panelOpen = 0;
    state->panelContent = 0;
    state->openChestThing =
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    ++state->f0334CloseCount;
    ++state->mouseDisableCount;
    return 1;
}

int DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_RunPc34Compat(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state,
    Dm1V1MirrorCancelAfterPickupResultPc34Compat *outResult)
{
    int projected;
    int pickedUp;
    int cancelled;

    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->spec = &s_spec;
    outResult->candidateOrdinalBefore = (int)state->g0299CandidateOrdinal;
    outResult->panelOpenBefore = state->panelOpen;
    outResult->panelContentBefore = state->panelContent;
    outResult->openChestBefore = state->openChestThing;
    outResult->leaderHandBefore = state->leaderHandThing;
    outResult->pickedSlotBefore = state->chestSlots[kPickedSlotIndex];
    outResult->cellThingBeforeProject = state->cellThing;
    outResult->previousCellThing = state->previousCellThing;
    outResult->thoughtThing = state->thoughtThing;
    capture_counters(state, outResult->countersBefore);
    copy_slots(outResult->chestSlotsBefore, state->chestSlots);

    projected = project_thought(state);
    outResult->cellThingAfterProject = state->cellThing;
    outResult->thoughtProjected = projected;

    pickedUp = scroll_pickup(state);
    outResult->candidateOrdinalAfterPickup = (int)state->g0299CandidateOrdinal;
    outResult->panelOpenAfterPickup = state->panelOpen;
    outResult->panelContentAfterPickup = state->panelContent;
    outResult->openChestAfterPickup = state->openChestThing;
    outResult->leaderHandAfterPickup = state->leaderHandThing;
    outResult->pickedSlotAfterPickup = state->chestSlots[kPickedSlotIndex];
    outResult->cellThingAfterPickup = state->cellThing;
    outResult->scrollPickupDispatched = pickedUp;
    capture_counters(state, outResult->countersAfterPickup);
    copy_slots(outResult->chestSlotsAfterPickup, state->chestSlots);

    cancelled = cancel_after_pickup(state);
    outResult->candidateOrdinalAfterCancel = (int)state->g0299CandidateOrdinal;
    outResult->panelOpenAfterCancel = state->panelOpen;
    outResult->panelContentAfterCancel = state->panelContent;
    outResult->openChestAfterCancel = state->openChestThing;
    outResult->leaderHandAfterCancel = state->leaderHandThing;
    outResult->pickedSlotAfterCancel = state->chestSlots[kPickedSlotIndex];
    outResult->cellThingAfterCancel = state->cellThing;
    outResult->cancelDispatched = cancelled;
    capture_counters(state, outResult->countersAfterCancel);
    copy_slots(outResult->chestSlotsAfterCancel, state->chestSlots);
    copy_text(outResult->thoughtTextAfterCancel,
              state->thoughtText,
              sizeof(outResult->thoughtTextAfterCancel));

    outResult->handEmptied =
        state->leaderHandThing ==
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    outResult->previousCellRestored =
        state->cellThing == state->previousCellThing;
    outResult->thoughtNoLongerProjected = state->thoughtProjected == 0;
    outResult->candidateCleared = state->g0299CandidateOrdinal == 0;
    outResult->chestClosed =
        state->openChestThing ==
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    outResult->pickedSlotRemainsEmpty =
        state->chestSlots[kPickedSlotIndex] ==
        DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT;
    outResult->unrelatedChestSlotsPreserved = unrelated_slots_preserved(
        outResult->chestSlotsBefore,
        outResult->chestSlotsAfterCancel,
        kPickedSlotIndex);
    outResult->textPreservedForAudit =
        strcmp(outResult->thoughtTextAfterCancel,
               "FUL BRO NETA scroll pickup thought") == 0;
    outResult->accepted =
        projected && pickedUp && cancelled && outResult->handEmptied &&
        outResult->previousCellRestored && outResult->candidateCleared &&
        outResult->chestClosed && outResult->pickedSlotRemainsEmpty &&
        outResult->unrelatedChestSlotsPreserved;
    return outResult->accepted;
}

const Dm1V1MirrorCancelAfterPickupEvidencePc34Compat *
DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const Dm1V1MirrorCancelAfterPickupSpecPc34Compat *
DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_SpecPc34Compat(void)
{
    return &s_spec;
}

const char *
DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_SourceEvidencePc34Compat(
    void)
{
    return s_source_evidence;
}
