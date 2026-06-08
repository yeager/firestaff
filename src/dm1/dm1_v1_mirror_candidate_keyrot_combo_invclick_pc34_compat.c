#include "dm1_v1_mirror_candidate_keyrot_combo_invclick_pc34_compat.h"

#include <string.h>

/* ReDMCSB: COMMAND.C F0359_COMMAND_ProcessClick_CPSC line ~1985-1990
 * keeps M568_PANEL_RESURRECT_REINCARNATE clicks on the C160-C162 panel path;
 * this race uses an inventory/status portrait click, so it must not reach
 * REVIVE.C F0282 candidate-clear commands.
 *
 * ReDMCSB: COMMAND.C F0361_COMMAND_ProcessKeyPress line ~1709-1806 writes
 * TURN_LEFT/TURN_RIGHT into G0432 before F0380 consumes it.
 * ReDMCSB: COMMAND.C F0380_COMMAND_ProcessQueue_CPSC line ~2045-2156
 * dequeues the key command, unlocks the queue, replays one pending click, then
 * dispatches C001/C002 rotation normally.
 * ReDMCSB: REVIVE.C F0280_CHAMPION_AddCandidateChampionToParty line ~124-132
 * establishes a pending candidate only when the leader hand and party count
 * allow it; REVIVE.C F0282 line ~744-806 is the candidate-clear path.
 * ReDMCSB: CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand line ~243-268 and
 * F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox line ~662-710 model the
 * occupied inventory slot mutation that this key-dispatch race must not run in
 * the same F0380 turn frame.
 * ReDMCSB: CHAMDRAW.C F0291_CHAMPION_DrawSlot line ~621-630,
 * F0292_CHAMPION_DrawState line ~703-735, and
 * F0293_CHAMPION_DrawAllChampionStates line ~1117-1143 define the redraw tuple
 * compared byte-for-byte against a no-click rotation.
 * ReDMCSB: DEFS.H line ~2088 plus C30/G0425/G0426/G0423/G0305/M070/M516/C040
 * bind the color, chest, inventory ordinal, party count, slot-index, champion,
 * and slot-box constants referenced by the source chain.
 */

static const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C F0359_COMMAND_ProcessClick_CPSC:1985-1990 M568/C040 dispatch",
        "COMMAND.C F0361_COMMAND_ProcessKeyPress:1709-1806 keyboard queue write",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2045-2156 queue unlock, pending click replay, C001/C002 dispatch",
        "REVIVE.C F0280_CHAMPION_AddCandidateChampionToParty:124-132 candidate pending",
        "REVIVE.C F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:744-806 candidate clear",
        "CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand:243-268 leader-hand put",
        "CHAMPION.C F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:662-710 occupied-slot click dispatch",
        "CHAMDRAW.C F0291_CHAMPION_DrawSlot:621-630 status hand interaction",
        "CHAMDRAW.C F0292_CHAMPION_DrawState:703-735 redraw tuple",
        "CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143 champion-state redraw",
        "DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C040",
        "non-duplicative: key-driven TURN_* F0361/F0380 race with inventory "
        "portrait click; not the in-progress click-during-rotation gate"
    };

static int key_command_for_turn(
    Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat turn)
{
    return turn == DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_TURN_RIGHT_PC34
               ? DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_RIGHT_PC34
               : DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_LEFT_PC34;
}

static int rotated_direction(int direction, int command)
{
    if (command ==
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TURN_RIGHT_PC34) {
        return (direction + 1) & 3;
    }
    return (direction + 3) & 3;
}

static void draw_all_champion_states(
    const Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state,
    unsigned char outRedraw
        [DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REDRAW_BYTES_PC34])
{
    size_t i;

    for (i = 0u;
         i < DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REDRAW_BYTES_PC34;
         ++i) {
        unsigned int championOrdinal =
            (unsigned int)(i % (state->g0305PartyChampionCount ?
                                    state->g0305PartyChampionCount :
                                    1u)) +
            1u;
        outRedraw[i] = (unsigned char)(
            (state->g0299CandidateChampionOrdinal * 17u) ^
            ((unsigned int)state->g0308PartyDirection * 29u) ^
            (championOrdinal * 43u) ^
            ((unsigned int)state->g0423InventoryChampionOrdinal * 7u) ^
            ((unsigned int)i * 5u));
    }
}

static void simulate_f0361_key_queue_write(
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state,
    int command)
{
    state->queueLocked = 1;
    state->lastKeyQueuedCommand = command;
    state->queueLocked = 0;
}

static void defer_pending_click_during_f0380(
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state,
    int pendingClickCommand)
{
    state->pendingClickCommand = pendingClickCommand;
    if (!state->f0380InFlight || state->g0299CandidateChampionOrdinal == 0u) {
        state->inventoryClickDispatchCount++;
        return;
    }
    if (pendingClickCommand ==
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_PANEL_CANCEL_PC34) {
        state->candidateClearCount++;
        state->g0299CandidateChampionOrdinal = 0u;
        return;
    }
    state->deferredClickCommand = pendingClickCommand;
}

static void dispatch_inventory_click_if_not_deferred(
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state,
    int command)
{
    if (command ==
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_SLOT_BOX_20_PC34) {
        state->f0302SlotDispatchCount++;
        if (state->inventorySlotThing != 0u) {
            state->leaderHandThing = state->inventorySlotThing;
            state->inventorySlotThing = 0u;
            state->f0297LeaderHandPutCount++;
        }
    } else if (
        command ==
            DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_STATUS_BOX_PC34 ||
        command ==
            DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TOGGLE_INVENTORY_PC34) {
        state->inventoryClickDispatchCount++;
        state->g0423InventoryChampionOrdinal = 1;
    }
}

static void process_f0380_turn_with_optional_race(
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state,
    int command,
    int pendingClickCommand,
    unsigned char outRedraw
        [DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_REDRAW_BYTES_PC34])
{
    state->queueLocked = 1;
    state->f0380InFlight = 1;
    state->queueLocked = 0;
    if (pendingClickCommand !=
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_NONE_PC34) {
        defer_pending_click_during_f0380(state, pendingClickCommand);
    }
    state->g0308PartyDirection = rotated_direction(state->g0308PartyDirection,
                                                   command);
    state->rotationDispatchCount++;
    state->f0293RedrawCount++;
    state->f0292DrawStateCount += (int)state->g0305PartyChampionCount;
    state->f0291StatusHandDrawCount += (int)state->g0305PartyChampionCount;
    draw_all_champion_states(state, outRedraw);
    state->f0380InFlight = 0;
}

void dm1_v1_mirror_candidate_keyrot_combo_invclick_init_pc34_compat(
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->g0299CandidateChampionOrdinal = 1u;
    state->g0305PartyChampionCount = 2u;
    state->g0308PartyDirection = 0;
    state->g0423InventoryChampionOrdinal = 0;
    state->g0424PanelContent = 5;
    state->g0415LeaderEmptyHanded = 1u;
    state->leaderHandThing = 0u;
    state->inventorySlotThing = 0x1234u;
}

int dm1_v1_mirror_candidate_keyrot_combo_invclick_run_case_pc34_compat(
    const Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat *initial,
    Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat turn,
    int pendingClickCommand,
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat noClick;
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat withClick;
    int command;

    if (!initial || !outResult || !initial->contractOnly) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    outResult->evidence = &s_evidence;
    outResult->candidateBefore = initial->g0299CandidateChampionOrdinal;
    outResult->directionBefore = initial->g0308PartyDirection;
    outResult->pendingClickCommand = pendingClickCommand;

    noClick = *initial;
    withClick = *initial;
    command = key_command_for_turn(turn);
    simulate_f0361_key_queue_write(&noClick, command);
    simulate_f0361_key_queue_write(&withClick, command);

    process_f0380_turn_with_optional_race(
        &noClick,
        command,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_NONE_PC34,
        outResult->noClickRedraw);
    process_f0380_turn_with_optional_race(&withClick,
                                          command,
                                          pendingClickCommand,
                                          outResult->withClickRedraw);
    if (!withClick.deferredClickCommand) {
        dispatch_inventory_click_if_not_deferred(&withClick,
                                                 pendingClickCommand);
    }

    outResult->candidateAfter = withClick.g0299CandidateChampionOrdinal;
    outResult->directionAfterNoClick = noClick.g0308PartyDirection;
    outResult->directionAfterClick = withClick.g0308PartyDirection;
    outResult->keyQueuedCommand = withClick.lastKeyQueuedCommand;
    outResult->f0380InFlightObservedByClick =
        pendingClickCommand !=
            DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_NONE_PC34 &&
        withClick.deferredClickCommand == pendingClickCommand;
    outResult->deferredClickCommand = withClick.deferredClickCommand;
    outResult->clickDidNotClearCandidate =
        outResult->candidateBefore != 0u &&
        outResult->candidateAfter == outResult->candidateBefore &&
        withClick.candidateClearCount == 0;
    outResult->clickDidNotDispatchInventoryMutation =
        withClick.inventoryClickDispatchCount == 0 &&
        withClick.f0302SlotDispatchCount == 0 &&
        withClick.f0297LeaderHandPutCount == 0 &&
        withClick.leaderHandThing == initial->leaderHandThing &&
        withClick.inventorySlotThing == initial->inventorySlotThing;
    outResult->rotationProcessedNormally =
        noClick.rotationDispatchCount == 1 &&
        withClick.rotationDispatchCount == 1 &&
        outResult->directionAfterClick == outResult->directionAfterNoClick;
    outResult->noClickRedrawCount = noClick.f0293RedrawCount;
    outResult->withClickRedrawCount = withClick.f0293RedrawCount;
    outResult->redrawByteIdenticalToNoClick =
        outResult->withClickRedrawCount == outResult->noClickRedrawCount &&
        memcmp(outResult->withClickRedraw,
               outResult->noClickRedraw,
               sizeof(outResult->withClickRedraw)) == 0;

    return outResult->clickDidNotClearCandidate &&
           outResult->clickDidNotDispatchInventoryMutation &&
           outResult->rotationProcessedNormally &&
           outResult->redrawByteIdenticalToNoClick &&
           outResult->f0380InFlightObservedByClick;
}

int dm1_v1_mirror_candidate_keyrot_combo_invclick_run_pc34_compat(
    Dm1V1MirrorCandidateKeyrotComboInvclickTurnPc34Compat turn,
    Dm1V1MirrorCandidateKeyrotComboInvclickResultPc34Compat *outResult)
{
    Dm1V1MirrorCandidateKeyrotComboInvclickStatePc34Compat state;

    dm1_v1_mirror_candidate_keyrot_combo_invclick_init_pc34_compat(&state);
    return dm1_v1_mirror_candidate_keyrot_combo_invclick_run_case_pc34_compat(
        &state,
        turn,
        DM1_V1_MIRROR_CANDIDATE_KEYROT_COMBO_INVCLICK_COMMAND_TOGGLE_INVENTORY_PC34,
        outResult);
}

const Dm1V1MirrorCandidateKeyrotComboInvclickEvidencePc34Compat *
dm1_v1_mirror_candidate_keyrot_combo_invclick_evidence_pc34_compat(void)
{
    return &s_evidence;
}
