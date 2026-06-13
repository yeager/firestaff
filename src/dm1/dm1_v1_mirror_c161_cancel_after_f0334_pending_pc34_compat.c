#include "firestaff/dm1/v1/mirror_candidate/c161_cancel_after_f0334_pending_pc34_compat.h"

#include <stddef.h>
#include <string.h>

enum {
    kC040PanelGraphic = 40,
    kC161PanelCancel = 161,
    kM568PanelResurrectReincarnate = 568,
    kCandidateOrdinal = 3,
    kCandidateOwnerIndex = 2,
    kClosedChestThingNone = 0,
    kLeaderHandItemType = 0x01c1,
    kLeaderHandWeight = 42,
    kLeaderHandCharges = 7
};

static const Dm1V1MirrorC161AfterF0334OpcodePc34 kTraceOpcodes[] = {
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0359_QUEUE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0360_ROUTE_CLICK_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0378_ROUTE_PANEL_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0380_DRAIN_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0077_BRACKET_OPEN_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0282_C161_CANCEL_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0286_REJECT_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0457_DRAW_ENABLED_MENUS_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0078_BRACKET_CLOSE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0347_PANEL_REDRAW_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_FOOD_READ_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_WATER_READ_PC34
};

static const char *const kSourceAnchors[] = {
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_CHAMPION_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_COMMAND_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_CHEST_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_MOUSE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_PANEL_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_REVIVE_PC34,
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_DEFS_PC34,
    "REVIVE.C F0282:744-806 C162 cancel branch is the PC cancel constant; this gate pins the queued C161/C162 mirror-candidate CANCEL lane as specified.",
    "PANEL.C F0347:1639-1693 re-enters F0345/F0344 after G0299/M568 clear for the leader settle.",
    "CHEST.C F0334:79-130 must have already cleared G0426 and G0425 before the cancel drains.",
    "COMMAND.C F0359:1452-1662 -> F0360:1692-1707 -> F0378:1956-1993 -> F0380:2045-2178 preserves the queued click route.",
    "CHAMPION.C F0297:243-268/F0298:270-298/F0301:606-660/F0302:662-714 are rejected for leader-hand and slot byte-stability.",
    "DEFS.H C040/C045/C160/C161/C162/C537..C544/G0299/G0424/G0425/G0426/M568/M070/M516 names the panel, slot, candidate, and champion bytes.",
    "REVIVE.C F0280:124-132 is the staged resurrect-pending precondition.",
    "REVIVE.C F0286 statistics-reset is rejected; CurrentHealth/Food/Water remain zero.",
    "MOUSE.C F0077:1-32 and F0078:33-64 screen-update bracketing balances around the cancel."
};

static const Dm1V1MirrorC161AfterF0334SourceLockPc34 kSourceLock = {
    1,
    1,
    (int)(sizeof(kTraceOpcodes) / sizeof(kTraceOpcodes[0])),
    kTraceOpcodes,
    kSourceAnchors,
    (int)(sizeof(kSourceAnchors) / sizeof(kSourceAnchors[0]))
};

/*
 * ReDMCSB source-lock summary:
 * CHAMPION.C F0297:243-268, F0298:270-298, F0300:485/564/575,
 * F0301:606-660, and F0302:662-714 own leader-hand, panel-dirty, chest-slot,
 * and inventory-slot dispatch; this regression records that they are not
 * touched by the cancel. COMMAND.C F0359:1452-1662, F0360:1692-1707,
 * F0378:1956-1993, and F0380:2045-2178 are the queued click route.
 * CHEST.C F0333:30-67 opens G0425/G0426 and F0334:79-130 closes and rewires
 * G0425 into the container list before the cancel is eligible. MOUSE.C
 * F0077:1-32/F0078:33-64 bracket the screen update. PANEL.C F0344:1493-1561,
 * F0345:1563-1617, F0346:1619-1637, F0347:1639-1693, and F0355:2280-2440
 * define the food/water, resurrect, panel-router, and inventory-toggle redraws.
 * REVIVE.C F0280:124-132 stages the pending candidate, F0282:744-806 clears
 * cancel state and draws enabled menus, and F0286 statistics-reset must not
 * fire. DEFS.H anchors C040/C045/C160/C161/C162/C537..C544/G0299/G0424/G0425/
 * G0426/M568/M070/M516 identify the constants and mutable bytes under test.
 */
static const char kSourceEvidence[] =
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_CHAMPION_PC34 "; "
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_COMMAND_PC34 "; "
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_CHEST_PC34 "; "
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_MOUSE_PC34 "; "
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_PANEL_PC34 "; "
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_REVIVE_PC34 "; "
    DM1_V1_MIRROR_C161_AFTER_F0334_ANCHOR_DEFS_PC34
    "; contract-only=1; no_game_data=1; lane=C161/C162 cancel after "
    "F0334 chest close and F0280 resurrect-pending.";

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & UINT32_C(0xff);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static int same_hand(Dm1V1MirrorC161AfterF0334LeaderHandPc34 a,
                     Dm1V1MirrorC161AfterF0334LeaderHandPc34 b)
{
    return a.itemType == b.itemType && a.weight == b.weight &&
           a.charges == b.charges;
}

static int chest_slots_clear(
    const Dm1V1MirrorC161AfterF0334StatePc34 *state)
{
    int i;

    for (i = 0; i < DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34;
         ++i) {
        if (state->g0425ChestSlots[i] != 0u) {
            return 0;
        }
    }
    return 1;
}

static int container_rewired(
    const Dm1V1MirrorC161AfterF0334StatePc34 *state)
{
    int i;
    int nonzero = 0;

    for (i = 0; i < DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34;
         ++i) {
        if (state->chestContainerSlots[i] != 0u) {
            nonzero = 1;
        }
    }
    return nonzero;
}

static Dm1V1MirrorC161AfterF0334RejectPc34 reject_reason(
    const Dm1V1MirrorC161AfterF0334StatePc34 *state,
    const Dm1V1MirrorC161AfterF0334ResultPc34 *result)
{
    if (!state || !result) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NULL_PC34;
    }
    if (!state->contractOnly || !state->noGameData) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NON_CONTRACT_PC34;
    }
    if (state->candidateOwnerIndex < 0 || state->g0299CandidateOrdinal == 0u) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NO_CANDIDATE_PC34;
    }
    if (state->candidateCurrentHealth > 0) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_ALIVE_CANDIDATE_PC34;
    }
    if (!state->resurrectPending) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NO_RESURRECT_PENDING_PC34;
    }
    if (state->g0426OpenChest != kClosedChestThingNone) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_CHEST_STILL_OPEN_PC34;
    }
    if (!state->f0334CloseFired || !state->chestCloseRewiredG0425 ||
        !chest_slots_clear(state) || !container_rewired(state)) {
        return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_F0334_NOT_FIRED_PC34;
    }
    return DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NONE_PC34;
}

static void append_trace(Dm1V1MirrorC161AfterF0334StatePc34 *state,
                         Dm1V1MirrorC161AfterF0334ResultPc34 *result,
                         Dm1V1MirrorC161AfterF0334OpcodePc34 opcode)
{
    Dm1V1MirrorC161AfterF0334TracePc34 *trace;

    if (result->opcodeCount >=
        DM1_V1_MIRROR_C161_AFTER_F0334_TRACE_CAPACITY_PC34) {
        return;
    }
    trace = &result->trace[result->opcodeCount++];
    trace->opcode = opcode;
    trace->candidateCurrentHealth = state->candidateCurrentHealth;
    trace->candidateFood = state->candidateFood;
    trace->candidateWater = state->candidateWater;
    trace->leaderHand = state->leaderHand;

    switch (opcode) {
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0359_QUEUE_PC34:
        ++result->f0359QueueCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0360_ROUTE_CLICK_PC34:
        ++result->f0360RouteClickCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0378_ROUTE_PANEL_PC34:
        ++result->f0378RouteCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0380_DRAIN_PC34:
        ++result->f0380DrainCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0282_C161_CANCEL_PC34:
        ++result->f0282C161CancelCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0077_BRACKET_OPEN_PC34:
        ++result->f0077BracketOpenCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0078_BRACKET_CLOSE_PC34:
        ++result->f0078BracketCloseCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0457_DRAW_ENABLED_MENUS_PC34:
        ++result->f0457DrawEnabledMenusCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0347_PANEL_REDRAW_PC34:
        ++result->f0347PanelRedrawCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_FOOD_READ_PC34:
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0344_WATER_READ_PC34:
        ++result->f0344FoodWaterReadCount;
        break;
    case DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0286_REJECT_PC34:
        ++result->f0286StatisticsResetRejectCount;
        break;
    }
}

const Dm1V1MirrorC161AfterF0334SourceLockPc34 *
dm1_v1_mirror_c161_cancel_after_f0334_pending_source_lock_pc34(void)
{
    return &kSourceLock;
}

const char *
dm1_v1_mirror_c161_cancel_after_f0334_pending_source_evidence_pc34(void)
{
    return kSourceEvidence;
}

Dm1V1MirrorC161AfterF0334StatePc34
dm1_v1_mirror_c161_cancel_after_f0334_pending_default_state_pc34(void)
{
    Dm1V1MirrorC161AfterF0334StatePc34 state;
    int i;

    memset(&state, 0, sizeof(state));
    state.contractOnly = 1;
    state.noGameData = 1;
    state.f0334CloseFired = 1;
    state.chestCloseRewiredG0425 = 1;
    state.g0426OpenChest = kClosedChestThingNone;
    state.leaderHandBeforeClose.itemType = kLeaderHandItemType;
    state.leaderHandBeforeClose.weight = kLeaderHandWeight;
    state.leaderHandBeforeClose.charges = kLeaderHandCharges;
    state.leaderHand = state.leaderHandBeforeClose;
    state.resurrectPending = 1;
    state.candidateOwnerIndex = kCandidateOwnerIndex;
    state.candidateCurrentHealth = 0;
    state.candidateFood = 0;
    state.candidateWater = 0;
    state.g0299CandidateOrdinal = kCandidateOrdinal;
    state.c040PanelOpen = 1;
    state.c040PanelGraphic = kC040PanelGraphic;
    state.c040PanelCommand = kM568PanelResurrectReincarnate;
    state.panelContentM568 = kM568PanelResurrectReincarnate;
    for (i = 0; i < DM1_V1_MIRROR_C161_AFTER_F0334_CHEST_SLOT_COUNT_PC34;
         ++i) {
        state.g0425ChestSlots[i] = 0u;
        state.chestContainerSlots[i] = (uint16_t)(0xc537u + (uint16_t)i);
    }
    return state;
}

int dm1_v1_mirror_c161_cancel_after_f0334_pending_run_pc34(
    Dm1V1MirrorC161AfterF0334StatePc34 *state,
    Dm1V1MirrorC161AfterF0334ResultPc34 *result)
{
    Dm1V1MirrorC161AfterF0334LeaderHandPc34 leaderHandBefore;
    Dm1V1MirrorC161AfterF0334RejectPc34 reject;
    size_t i;

    if (result) {
        memset(result, 0, sizeof(*result));
    }
    reject = reject_reason(state, result);
    if (result) {
        result->rejectCode = reject;
    }
    if (reject != DM1_V1_MIRROR_C161_AFTER_F0334_REJECT_NONE_PC34) {
        return 0;
    }

    leaderHandBefore = state->leaderHand;
    result->accepted = 1;
    result->chestSlotsRewiredBeforeCancel =
        chest_slots_clear(state) && container_rewired(state) &&
        state->g0426OpenChest == kClosedChestThingNone;
    result->leaderHandStableAcrossClose =
        same_hand(state->leaderHandBeforeClose, state->leaderHand);

    for (i = 0; i < sizeof(kTraceOpcodes) / sizeof(kTraceOpcodes[0]); ++i) {
        append_trace(state, result, kTraceOpcodes[i]);
        if (kTraceOpcodes[i] ==
            DM1_V1_MIRROR_C161_AFTER_F0334_OPCODE_F0282_C161_CANCEL_PC34) {
            state->g0299CandidateOrdinal = 0u;
            state->resurrectPending = 0;
            state->panelContentM568 = 0;
            state->c040PanelOpen = 0;
            state->c040PanelGraphic = 0;
            state->c040PanelCommand = 0;
        }
    }

    result->g0299Cleared = state->g0299CandidateOrdinal == 0u;
    result->resurrectPendingCleared = state->resurrectPending == 0;
    result->m568Cleared = state->panelContentM568 == 0;
    result->c040Cleared = state->c040PanelOpen == 0 &&
                           state->c040PanelGraphic == 0 &&
                           state->c040PanelCommand == 0;
    result->candidateStayedDead = state->candidateCurrentHealth == 0;
    result->candidateFoodWaterStayedZero =
        state->candidateFood == 0 && state->candidateWater == 0;
    result->leaderHandStableAcrossCancel =
        same_hand(leaderHandBefore, state->leaderHand);
    result->f0286StatisticsResetCallCount = 0;
    result->hash =
        dm1_v1_mirror_c161_cancel_after_f0334_pending_hash_pc34(result);
    (void)kC161PanelCancel;
    return 1;
}

uint32_t dm1_v1_mirror_c161_cancel_after_f0334_pending_hash_pc34(
    const Dm1V1MirrorC161AfterF0334ResultPc34 *result)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    if (!result) {
        return 0;
    }
    hash = hash_step(hash, (uint32_t)result->accepted);
    hash = hash_step(hash, (uint32_t)result->rejectCode);
    hash = hash_step(hash, (uint32_t)result->opcodeCount);
    hash = hash_step(hash, (uint32_t)result->f0359QueueCount);
    hash = hash_step(hash, (uint32_t)result->f0360RouteClickCount);
    hash = hash_step(hash, (uint32_t)result->f0378RouteCount);
    hash = hash_step(hash, (uint32_t)result->f0380DrainCount);
    hash = hash_step(hash, (uint32_t)result->f0282C161CancelCount);
    hash = hash_step(hash, (uint32_t)result->f0286StatisticsResetRejectCount);
    hash = hash_step(hash, (uint32_t)result->f0286StatisticsResetCallCount);
    hash = hash_step(hash, (uint32_t)result->f0077BracketOpenCount);
    hash = hash_step(hash, (uint32_t)result->f0078BracketCloseCount);
    hash = hash_step(hash, (uint32_t)result->f0457DrawEnabledMenusCount);
    hash = hash_step(hash, (uint32_t)result->f0347PanelRedrawCount);
    hash = hash_step(hash, (uint32_t)result->f0344FoodWaterReadCount);
    hash = hash_step(hash, (uint32_t)result->g0299Cleared);
    hash = hash_step(hash, (uint32_t)result->resurrectPendingCleared);
    hash = hash_step(hash, (uint32_t)result->m568Cleared);
    hash = hash_step(hash, (uint32_t)result->c040Cleared);
    hash = hash_step(hash, (uint32_t)result->candidateStayedDead);
    hash = hash_step(hash, (uint32_t)result->candidateFoodWaterStayedZero);
    hash = hash_step(hash, (uint32_t)result->leaderHandStableAcrossCancel);
    hash = hash_step(hash, (uint32_t)result->leaderHandStableAcrossClose);
    hash = hash_step(hash, (uint32_t)result->chestSlotsRewiredBeforeCancel);
    for (i = 0; i < result->opcodeCount; ++i) {
        hash = hash_step(hash, (uint32_t)result->trace[i].opcode);
        hash = hash_step(hash,
                         (uint32_t)result->trace[i].candidateCurrentHealth);
        hash = hash_step(hash, (uint32_t)result->trace[i].candidateFood);
        hash = hash_step(hash, (uint32_t)result->trace[i].candidateWater);
        hash = hash_step(hash, (uint32_t)result->trace[i].leaderHand.itemType);
        hash = hash_step(hash, (uint32_t)result->trace[i].leaderHand.weight);
        hash = hash_step(hash, (uint32_t)result->trace[i].leaderHand.charges);
    }
    return hash;
}
