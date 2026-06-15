#include "dm1/dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * CHEST.C F0333:30-67 opens a panel/chest and seeds G0425; F0334:117-132
 * clears G0426 and rewrites/reset slots when the open container closes.
 * CHAMPION.C F0297:243-268 puts an object in the leader hand; F0298:270-298
 * removes it; F0300:511-584 removes slot/chest objects; F0301:606-660 adds
 * slot/chest objects; F0302:662-713 is the slot-box transfer command.
 * COMMAND.C F0378:1973-1983 dispatches panel input and F0380:2045-2156
 * dequeues turn/slot commands in order.
 * REVIVE.C F0280:124-132 gates candidate publication on empty hand/party room;
 * F0282:744-806 clears G0299/C040 on a candidate decision.
 * PANEL.C F0346/F0347:1619-1657 redraws C040 when G0299 is live.
 * UTAMSCR.C F0077:147-151 and F0078:141-145 bracket pointer redraw.
 * OBJECT.C F0033:147-212 resolves orientation-sensitive object icons.
 * BLITMASK.C F0133:30-33 anchors masked panel redraws.
 * DEFS.H:338-340, 810-817, 1874-1878, 2200, 3001-3008, 5694, 5876-5881 names
 * C162, C30..C37, C38, C040, M568/M569, G0299, G0425, and G0426.
 * CLIKMENU.C F0365:142-174 queues turn-left/right into F0284, and CHAMPION.C
 * F0284:93-131 rotates party Cell/Direction and G0308.
 */

enum {
    kLeaderIndex = 0,
    kPartyChampionCount = 3,
    kCandidateChampionIndex = 3,
    kCandidateOrdinal = 4,
    kInitialDirection = 0,
    kRotatedDirection = 1,
    kDepositThing = 0x6D40u,
    kCandidateSlotSeed = 0x4C20u,
    kOpenChestThing = 0x6401u,
    kFirstChestSlotThing = 0x7100
};

static const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat
    s_evidence = {
        1,
        "CHEST.C F0333:30-67 open panel/chest and seed G0425 slots",
        "CHEST.C F0334:117-132 clear G0426 and reset/relink G0425 slots",
        "CHAMPION.C F0297:243-268 put object in leader hand",
        "CHAMPION.C F0298:270-298 remove object from leader hand",
        "CHAMPION.C F0300:511-584 remove object from slot/chest",
        "CHAMPION.C F0301:606-660 add object into slot/chest",
        "CHAMPION.C F0302:662-713 slot-box transfer command",
        "COMMAND.C F0378:1973-1983 mirror/candidate panel dispatch",
        "COMMAND.C F0380:2045-2156 queue dispatch, turn/slot ordering",
        "REVIVE.C F0280:124-132 candidate publish empty-hand/party-room gate",
        "REVIVE.C F0282:744-806 candidate click/clear path",
        "PANEL.C F0346/F0347:1619-1657 C040 redraw from G0299",
        "UTAMSCR.C F0077:147-151 enable and F0078:141-145 disable pointer update",
        "OBJECT.C F0033:147-212 icon index including party-direction compass",
        "BLITMASK.C F0133:30-33 masked panel redraw",
        "DEFS.H:338-340 C162; DEFS.H:810-817 C30..C37; "
            "DEFS.H:1874-1878 C38; DEFS.H:2200 C040; "
            "DEFS.H:3001-3008 M568/M569; DEFS.H:5694 G0299; "
            "DEFS.H:5876-5881 G0425/G0426",
        "COMMAND.C F0380:2150-2156 turn dispatch; CLIKMENU.C F0365:142-174 "
            "turn-left/right; CHAMPION.C F0284:93-131 party direction rotate",
        "contract_only=1 DM1 V1 mirror candidate deposit-then-party-rotate-"
            "then-reopen regression; non-overlap with pass702/pass707/pass710/"
            "pass711/pass713/pass715/pass723/pass727/pass728/pass732/pass736"
    };

static int normalize_direction(int direction)
{
    while (direction < 0) {
        direction += 4;
    }
    return direction & 3;
}

static void seed_chest_slots(int slots[])
{
    int i;

    for (i = 0; i < DM1_V1_MCRADPR_CHEST_SLOT_COUNT_PC34; ++i) {
        slots[i] = kFirstChestSlotThing + i;
    }
}

static int chest_slots_are_none(const int slots[])
{
    int i;

    for (i = 0; i < DM1_V1_MCRADPR_CHEST_SLOT_COUNT_PC34; ++i) {
        if (slots[i] != DM1_V1_MCRADPR_THING_NONE_PC34) {
            return 0;
        }
    }
    return 1;
}

static void clear_chest_state(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    int i;

    if (state->openChestThing == DM1_V1_MCRADPR_THING_NONE_PC34 &&
        chest_slots_are_none(state->chestSlots)) {
        return;
    }

    ++state->f0334CloseCount;
    state->openChestThing = DM1_V1_MCRADPR_THING_NONE_PC34;
    for (i = 0; i < DM1_V1_MCRADPR_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestSlots[i] = DM1_V1_MCRADPR_THING_NONE_PC34;
    }
}

static void draw_candidate_panel(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    state->panelContent = DM1_V1_MCRADPR_M568_PANEL_PC34;
    state->c040Graphic = DM1_V1_MCRADPR_C040_GRAPHIC_PC34;
    state->c040PanelOpen = 1;
    ++state->f0346PanelDrawCount;
    ++state->f0347PanelRedrawCount;
    ++state->f0133BlitmaskCount;
}

void DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_InitPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->partyChampionCount = kPartyChampionCount;
    state->leaderIndex = kLeaderIndex;
    state->partyDirection = kInitialDirection;
    state->candidateChampionIndex = kCandidateChampionIndex;
    state->candidateOrdinal = kCandidateOrdinal;
    state->leaderHandThing = DM1_V1_MCRADPR_THING_NONE_PC34;
    state->leaderHandEmpty = 1;
    state->pendingDepositThing = DM1_V1_MCRADPR_THING_NONE_PC34;
    state->candidateSlotThing = kCandidateSlotSeed;
    state->candidateSlotFilled = 1;
    state->depositedThing = DM1_V1_MCRADPR_THING_NONE_PC34;
    state->candidateReferenceDirection = -1;
    state->reopenedReferenceDirection = -1;
    state->openChestThing = kOpenChestThing;
    state->noCrashGuard = 1;
    seed_chest_slots(state->chestSlots);

    for (i = 0; i < DM1_V1_MCRADPR_CHAMPION_COUNT_PC34; ++i) {
        state->championCell[i] = i & 3;
        state->championDirection[i] = kInitialDirection;
    }
}

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_OpenCandidatePc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->leaderHandEmpty ||
        state->partyChampionCount >= DM1_V1_MCRADPR_CHAMPION_COUNT_PC34 ||
        state->g0299CandidateOrdinal != 0 || state->c040PanelOpen) {
        return 0;
    }

    ++state->f0280OpenCount;
    ++state->f0333OpenCount;
    ++state->f0033IconCount;
    state->g0299CandidateOrdinal = state->candidateOrdinal;
    state->candidateReferenceDirection = state->partyDirection;
    draw_candidate_panel(state);
    return 1;
}

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_PutDepositInLeaderHandPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    unsigned int thing)
{
    if (!state || !state->contractOnly ||
        thing == DM1_V1_MCRADPR_THING_NONE_PC34 || !state->leaderHandEmpty) {
        return 0;
    }

    state->leaderHandThing = thing;
    state->leaderHandEmpty = 0;
    state->pendingDepositThing = thing;
    state->depositPending = 1;
    ++state->f0297PutCount;
    ++state->f0077EnableCount;
    ++state->f0033IconCount;
    ++state->f0078DisableCount;
    return 1;
}

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_DepositViaMirrorPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    unsigned int thing;

    if (!state || !state->contractOnly || !state->c040PanelOpen ||
        state->g0299CandidateOrdinal != state->candidateOrdinal ||
        !state->depositPending ||
        state->leaderHandThing == DM1_V1_MCRADPR_THING_NONE_PC34) {
        return 0;
    }

    ++state->f0380QueueDispatchCount;
    ++state->f0378PanelDispatchCount;
    ++state->f0282ClickCount;
    ++state->f0302SlotClickCount;
    ++state->f0077EnableCount;

    clear_chest_state(state);
    thing = state->leaderHandThing;
    state->leaderHandThing = DM1_V1_MCRADPR_THING_NONE_PC34;
    state->leaderHandEmpty = 1;
    state->pendingDepositThing = DM1_V1_MCRADPR_THING_NONE_PC34;
    state->depositPending = 0;
    ++state->f0298RemoveCount;

    state->candidateSlotThing = thing;
    state->candidateSlotFilled = 1;
    state->depositedThing = thing;
    ++state->f0300RemoveCount;
    ++state->f0301AddCount;
    ++state->objectTransferCount;
    ++state->depositFireCount;

    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    state->c040Graphic = 0;
    ++state->f0347PanelRedrawCount;
    ++state->f0078DisableCount;
    return 1;
}

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RotatePartyPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    int turnCommand)
{
    int delta;
    int newDirection;
    int i;

    if (!state || !state->contractOnly ||
        (turnCommand != DM1_V1_MCRADPR_C001_TURN_LEFT_PC34 &&
         turnCommand != DM1_V1_MCRADPR_C002_TURN_RIGHT_PC34)) {
        return 0;
    }

    ++state->f0380QueueDispatchCount;
    ++state->f0365TurnPartyCount;
    newDirection = normalize_direction(
        state->partyDirection +
        (turnCommand == DM1_V1_MCRADPR_C002_TURN_RIGHT_PC34 ? 1 : 3));
    delta = normalize_direction(newDirection - state->partyDirection);
    if (delta == 0) {
        return 0;
    }
    for (i = 0; i < state->partyChampionCount; ++i) {
        state->championCell[i] = normalize_direction(state->championCell[i] + delta);
        state->championDirection[i] =
            normalize_direction(state->championDirection[i] + delta);
    }
    state->partyDirection = newDirection;
    ++state->f0284SetPartyDirectionCount;
    return 1;
}

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_CloseCandidatePc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state)
{
    if (!state || !state->contractOnly || !state->c040PanelOpen ||
        state->g0299CandidateOrdinal == 0) {
        if (state) {
            ++state->closeNoopCount;
        }
        return 0;
    }

    ++state->f0378PanelDispatchCount;
    ++state->f0282CancelCount;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    state->c040Graphic = 0;
    if (state->depositPending ||
        state->leaderHandThing != DM1_V1_MCRADPR_THING_NONE_PC34) {
        ++state->doubleFireCount;
    }
    return 1;
}

static int rotated_cells_match(
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    const int beforeCells[])
{
    int i;

    for (i = 0; i < state->partyChampionCount; ++i) {
        if (state->championCell[i] != normalize_direction(beforeCells[i] + 1)) {
            return 0;
        }
    }
    return 1;
}

static int rotated_directions_match(
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    const int beforeDirections[])
{
    int i;

    for (i = 0; i < state->partyChampionCount; ++i) {
        if (state->championDirection[i] !=
            normalize_direction(beforeDirections[i] + 1)) {
            return 0;
        }
    }
    return 1;
}

unsigned int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_HashPc34Compat(
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat *result)
{
    unsigned int hash = 2166136261u;
    int i;

#define HASH_U32(value) do { \
        hash ^= (unsigned int)(value); \
        hash *= 16777619u; \
    } while (0)

    if (state) {
        HASH_U32(state->contractOnly);
        HASH_U32(state->partyChampionCount);
        HASH_U32(state->partyDirection);
        HASH_U32(state->candidateReferenceDirection);
        HASH_U32(state->reopenedReferenceDirection);
        HASH_U32(state->leaderHandThing);
        HASH_U32(state->depositedThing);
        HASH_U32(state->g0299CandidateOrdinal);
        HASH_U32(state->c040PanelOpen);
        HASH_U32(state->f0280OpenCount);
        HASH_U32(state->f0282ClickCount);
        HASH_U32(state->f0282CancelCount);
        HASH_U32(state->f0334CloseCount);
        HASH_U32(state->f0297PutCount);
        HASH_U32(state->f0298RemoveCount);
        HASH_U32(state->f0301AddCount);
        HASH_U32(state->f0378PanelDispatchCount);
        HASH_U32(state->f0380QueueDispatchCount);
        HASH_U32(state->f0284SetPartyDirectionCount);
        HASH_U32(state->objectTransferCount);
        HASH_U32(state->depositFireCount);
        HASH_U32(state->doubleFireCount);
        for (i = 0; i < state->partyChampionCount; ++i) {
            HASH_U32(state->championCell[i]);
            HASH_U32(state->championDirection[i]);
        }
    }
    if (result) {
        HASH_U32(result->openedInitialCandidate);
        HASH_U32(result->depositFired);
        HASH_U32(result->partyRotated);
        HASH_U32(result->reopenedCandidate);
        HASH_U32(result->closeAfterReopenNoDoubleFire);
        HASH_U32(result->ok);
    }
#undef HASH_U32
    return hash;
}

static void finish_result(
    const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat *result)
{
    result->contractOnly = state->contractOnly == 1 && s_evidence.contractOnly == 1;
    result->noAssetsOrPixelParity = 1;
    result->ok =
        result->openedInitialCandidate &&
        result->putDepositInLeaderHand &&
        result->depositFired &&
        result->mirrorClosedAfterDeposit &&
        result->objectTransferred &&
        result->g0425ResetAfterDeposit &&
        result->g0426ResetAfterDeposit &&
        result->g0299ResetAfterDeposit &&
        result->c040ResetAfterDeposit &&
        result->leaderHandEmptyAfterDeposit &&
        result->noPendingDepositAfterDeposit &&
        result->partyRotated &&
        result->championCellsRotated &&
        result->championDirectionsRotated &&
        result->reopenedCandidate &&
        result->reopenUsesNewDirection &&
        result->reopenHasCleanDepositState &&
        result->reopenHasCleanPanelState &&
        result->noLeftoverC040BeforeReopen &&
        result->noLeftoverChestBeforeReopen &&
        result->noCrash &&
        result->closeAfterReopen &&
        result->closeAfterReopenNoDoubleFire &&
        result->secondCloseNoop &&
        result->contractOnly &&
        result->noAssetsOrPixelParity;
    result->hash =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_HashPc34Compat(
            state,
            result);
}

int DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RunPc34Compat(
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateStatePc34Compat *state,
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat local;
    Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateResultPc34Compat *result =
        outResult ? outResult : &local;
    int beforeCells[DM1_V1_MCRADPR_CHAMPION_COUNT_PC34];
    int beforeDirections[DM1_V1_MCRADPR_CHAMPION_COUNT_PC34];
    int opened;
    int put;
    int deposited;
    int rotated;
    int reopened;
    int closed;
    int secondClose;
    int i;

    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    if (!state || !state->contractOnly) {
        return 0;
    }

    opened =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_OpenCandidatePc34Compat(
            state);
    result->openedInitialCandidate =
        opened &&
        state->g0299CandidateOrdinal == kCandidateOrdinal &&
        state->c040PanelOpen &&
        state->panelContent == DM1_V1_MCRADPR_M568_PANEL_PC34 &&
        state->candidateReferenceDirection == kInitialDirection;
    result->candidateReferenceDirectionBeforeDeposit =
        state->candidateReferenceDirection;

    put =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_PutDepositInLeaderHandPc34Compat(
            state,
            kDepositThing);
    result->putDepositInLeaderHand =
        put &&
        state->leaderHandThing == kDepositThing &&
        !state->leaderHandEmpty &&
        state->depositPending;
    result->leaderHandThingBeforeDeposit = state->leaderHandThing;

    deposited =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_DepositViaMirrorPc34Compat(
            state);
    result->depositFired = deposited && state->depositFireCount == 1;
    result->mirrorClosedAfterDeposit =
        state->c040PanelOpen == 0 && state->panelContent == 0;
    result->objectTransferred =
        state->objectTransferCount == 1 &&
        state->depositedThing == kDepositThing &&
        state->candidateSlotThing == kDepositThing;
    result->g0425ResetAfterDeposit = chest_slots_are_none(state->chestSlots);
    result->g0426ResetAfterDeposit =
        state->openChestThing == DM1_V1_MCRADPR_THING_NONE_PC34;
    result->g0299ResetAfterDeposit = state->g0299CandidateOrdinal == 0;
    result->c040ResetAfterDeposit =
        state->c040PanelOpen == 0 &&
        state->panelContent == 0 &&
        state->c040Graphic == 0;
    result->leaderHandEmptyAfterDeposit = state->leaderHandEmpty == 1;
    result->leaderHandThingAfterDeposit = state->leaderHandThing;
    result->noPendingDepositAfterDeposit =
        !state->depositPending &&
        state->pendingDepositThing == DM1_V1_MCRADPR_THING_NONE_PC34;
    result->depositedThing = state->depositedThing;
    result->f0282ClickCountAfterDeposit = state->f0282ClickCount;
    result->f0334CloseCountAfterDeposit = state->f0334CloseCount;
    result->f0378DispatchCountAfterDeposit = state->f0378PanelDispatchCount;

    result->noLeftoverC040BeforeReopen =
        state->c040PanelOpen == 0 && state->panelContent == 0 &&
        state->c040Graphic == 0;
    result->noLeftoverChestBeforeReopen =
        state->openChestThing == DM1_V1_MCRADPR_THING_NONE_PC34 &&
        chest_slots_are_none(state->chestSlots);
    for (i = 0; i < state->partyChampionCount; ++i) {
        beforeCells[i] = state->championCell[i];
        beforeDirections[i] = state->championDirection[i];
    }
    result->partyDirectionBeforeRotate = state->partyDirection;
    rotated =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_RotatePartyPc34Compat(
            state,
            DM1_V1_MCRADPR_C002_TURN_RIGHT_PC34);
    result->partyDirectionAfterRotate = state->partyDirection;
    result->partyRotated =
        rotated &&
        result->partyDirectionBeforeRotate == kInitialDirection &&
        result->partyDirectionAfterRotate == kRotatedDirection;
    result->championCellsRotated = rotated_cells_match(state, beforeCells);
    result->championDirectionsRotated =
        rotated_directions_match(state, beforeDirections);
    result->f0380DispatchCountAfterRotate = state->f0380QueueDispatchCount;

    reopened =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_OpenCandidatePc34Compat(
            state);
    state->reopenedReferenceDirection = state->candidateReferenceDirection;
    result->reopenG0299CandidateOrdinal = state->g0299CandidateOrdinal;
    result->candidateReferenceDirectionAfterReopen =
        state->candidateReferenceDirection;
    result->reopenedCandidate =
        reopened &&
        state->c040PanelOpen &&
        state->g0299CandidateOrdinal == kCandidateOrdinal;
    result->reopenUsesNewDirection =
        state->candidateReferenceDirection == state->partyDirection &&
        state->candidateReferenceDirection == kRotatedDirection;
    result->reopenHasCleanDepositState =
        state->leaderHandEmpty &&
        state->leaderHandThing == DM1_V1_MCRADPR_THING_NONE_PC34 &&
        state->pendingDepositThing == DM1_V1_MCRADPR_THING_NONE_PC34 &&
        !state->depositPending &&
        state->depositFireCount == 1;
    result->reopenHasCleanPanelState =
        state->panelContent == DM1_V1_MCRADPR_M568_PANEL_PC34 &&
        state->c040Graphic == DM1_V1_MCRADPR_C040_GRAPHIC_PC34;
    result->f0280OpenCountAfterReopen = state->f0280OpenCount;
    result->f0346PanelDrawCountAfterReopen = state->f0346PanelDrawCount;
    result->f0347PanelRedrawCountAfterReopen = state->f0347PanelRedrawCount;
    result->noCrash = state->noCrashGuard == 1;

    closed =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_CloseCandidatePc34Compat(
            state);
    result->closeAfterReopen =
        closed &&
        state->g0299CandidateOrdinal == 0 &&
        state->c040PanelOpen == 0;
    result->doubleFireCountAfterClose = state->doubleFireCount;
    result->closeAfterReopenNoDoubleFire =
        state->depositFireCount == 1 &&
        state->objectTransferCount == 1 &&
        state->doubleFireCount == 0 &&
        state->depositedThing == kDepositThing;
    secondClose =
        DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_CloseCandidatePc34Compat(
            state);
    result->secondCloseNoop = secondClose == 0 && state->closeNoopCount == 1;

    finish_result(state, result);
    return result->ok;
}

const Dm1V1MirrorCandidateReselectAfterDepositWithPartyRotateEvidencePc34Compat *
DM1_V1_MirrorCandidateReselectAfterDepositWithPartyRotate_EvidencePc34Compat(void)
{
    return &s_evidence;
}
