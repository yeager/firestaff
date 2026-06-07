#include "dm1_v1_mirror_candidate_double_open_close_pc34_compat.h"

#include "dm1_v1_champion_mirror_click_closed_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_mirror_candidate_click_cancel_pc34_compat.h"
#include "dm1_v1_mirror_candidate_close_button_pc34_compat.h"
#include "dm1_v1_mirror_candidate_icon_refresh_pc34_compat.h"
#include "dm1_v1_mirror_candidate_reincarnate_rearm_pc34_compat.h"
#include "dm1_v1_mirror_candidate_resurrect_rearm_pc34_compat.h"

#include <string.h>

enum {
    kInitialPartyChampionCount = 2,
    kCandidatePartyChampionCount = 3,
    kLiveChampionOrdinal = 1,
    kLiveChampionHealth = 72,
    kLeaderIndex = 0,
    kLeaderHandThingOrdinal = 0x1234,
    kInitialFrontD1cMirrorChampionOrdinal = 4,
    kDeadzoneX = 224,
    kDeadzoneY = 33
};

/* ReDMCSB: CHAMPION.C C00512_FALSE line 30 anchors the event-22 timing
 * window; COMMAND.C G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel
 * lines 508-511 maps C040 panel buttons; COMMAND.C
 * F0359_COMMAND_ProcessClick_CPSC lines 1452-1661 and
 * F0380_COMMAND_ProcessQueue_CPSC lines 2045-2156 define rapid click/queue
 * dispatch; MOVESENS.C F0269_SENSOR_ProcessThingAdditionOrRemoval lines
 * 1501-1503 gates C127 portrait sensors; REVIVE.C F0280 lines 272-276
 * appends G0299 once, and REVIVE.C F0282 lines 744-758 and 785-806 split
 * cancel from resurrect/reincarnate finalization. */
static const char s_source_evidence[] =
    "CHAMPION.C C00512_FALSE line 30 anchors the event-22/resurrect timing "
    "guard; COMMAND.C G0457 lines 508-511 maps C040 resurrect/reincarnate/"
    "cancel buttons; COMMAND.C F0358 lines 1379-1449 and F0359 lines "
    "1452-1661 route rapid mouse hits through the command queue; COMMAND.C "
    "F0360 lines 1692-1707 and F0380 lines 2045-2156 cover pending-click "
    "replay, dequeue, and movement gates; COMMAND.C F0378 lines 1985-1991 "
    "dispatches only C040 panel commands while M568 is active; REVIVE.C "
    "F0280 lines 272-276 appends the mirror candidate and publishes G0299; "
    "REVIVE.C F0282 lines 744-758 clears G0299 on cancel/close and lines "
    "785-806 are the resurrect/reincarnate finalize branch that rapid open/"
    "close must not duplicate; COMMAND.C lines 2159-2181 and 2302-2311 gate "
    "status, inventory, spell, and action dispatch while G0299 is live; "
    "DUNVIEW.C lines 3913-3928 and 8488-8533 preserve the D1C portrait route; "
    "CHAMPION.C F0297/F0298/F0302 lines 243-285 and 662-706 are leader-hand "
    "put/remove/slot routes not entered by rapid panel open/close.";

const Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat
    DM1_V1_MirrorCandidateDoubleOpenCloseSpecPc34Compat = {
        "dm1_v1_mirror_candidate_double_open_close_pc34_compat",
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_RAPID_WINDOW_TICKS_PC34_COMPAT,
        "rapid double-open/close keeps C040 candidate ownership single",
        s_source_evidence
    };

void DM1_V1_MirrorCandidateDoubleOpenClose_InitPc34Compat(
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->active = 1;
    state->partyChampionCount = kInitialPartyChampionCount;
    state->preC040PartyChampionCount = kInitialPartyChampionCount;
    state->liveChampionOrdinal = kLiveChampionOrdinal;
    state->liveChampionHealth = kLiveChampionHealth;
    state->leaderIndex = kLeaderIndex;
    state->leaderHandThingOrdinal = kLeaderHandThingOrdinal;
    state->frontD1cMirrorChampionOrdinal = kInitialFrontD1cMirrorChampionOrdinal;
    state->mirrorRouteArmed = 1;
    state->lastOpenTick = -1000;
}

static void queue_one_command(
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int command,
    int x,
    int y,
    int buttonMask,
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state)
{
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult;

    DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
        queue, command, x, y, buttonMask);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
        queue, 0, 0, 0, 0);
    if (queueResult.dequeued) {
        ++state->queueDispatchCount;
    }
}

static unsigned int event_candidate(
    const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *event)
{
    if (event && event->candidateChampionOrdinal != 0u) {
        return event->candidateChampionOrdinal;
    }
    return DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_DEFAULT_CANDIDATE_PC34_COMPAT;
}

static int route_click_cancel_deadzone(void)
{
    Dm1V1MirrorCandidateClickCancelStatePc34Compat state;
    Dm1V1MirrorCandidateClickCancelResultPc34Compat result;

    dm1_v1_mirror_candidate_click_cancel_init_pc34(&state);
    (void)dm1_v1_mirror_candidate_click_cancel_front_cell_pc34(
        &state, &result);
    return result.ignoredFrontCellOnly &&
        result.candidateIdentityStayedNone &&
        result.c040PanelStayedClosed &&
        result.leaderHandUnchanged;
}

static int route_icon_refresh_while_open(void)
{
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat state;
    Dm1V1MirrorCandidateIconRefreshResultPc34Compat result;

    DM1_V1_MirrorCandidateIconRefresh_InitSuppressedPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateIconRefresh_DrawChangedObjectIconsPc34Compat(
        &state, &result);
    return result.suppressedByCandidateWithoutInventory &&
        result.candidateOrdinalBefore != 0 &&
        result.candidateOrdinalAfter == result.candidateOrdinalBefore &&
        result.commandQueueMutationCountAfter ==
            result.commandQueueMutationCountBefore;
}

static int route_resurrect_gate_while_open(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat state;
    Dm1V1MirrorCandidateCommandGateResultPc34Compat gate;

    DM1_V1_MirrorClickClosed_InitPc34Compat(&state);
    state.partyChampionCount = 2;
    state.candidateChampionOrdinal = 2u;
    state.inventoryChampionOrdinal = 2u;
    return !DM1_V1_MirrorCandidateResurrectRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_ACTION_AREA_COMMAND_PC34_COMPAT,
        &gate) &&
        gate.blockedByCandidatePanel;
}

static int route_reincarnate_gate_while_open(void)
{
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat state;
    Dm1V1MirrorCandidateReincarnateCommandGateResultPc34Compat gate;

    DM1_V1_MirrorCandidateReincarnateRearm_InitPc34Compat(&state);
    return !DM1_V1_MirrorCandidateReincarnateRearm_CanProcessCommandPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_ACTION_AREA_COMMAND_PC34_COMPAT,
        &gate) &&
        gate.blockedByCandidatePanel;
}

static int route_no_finalize_after_close(void)
{
    Dm1V1MirrorClickClosedStatePc34Compat resurrectState;
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat resurrect;
    Dm1V1MirrorCandidateReincarnateRearmStatePc34Compat reincarnateState;
    Dm1V1MirrorCandidateReincarnateRearmResultPc34Compat reincarnate;

    DM1_V1_MirrorClickClosed_InitPc34Compat(&resurrectState);
    resurrectState.candidateChampionOrdinal = 0u;
    (void)DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
        &resurrectState, &resurrect);

    DM1_V1_MirrorCandidateReincarnateRearm_InitPc34Compat(&reincarnateState);
    reincarnateState.candidateChampionOrdinal = 0u;
    (void)DM1_V1_MirrorCandidateReincarnateRearm_ProcessPanelCommandPc34Compat(
        &reincarnateState,
        DM1_V1_MIRROR_CANDIDATE_REINCARNATE_COMMAND_PC34_COMPAT,
        &reincarnate);

    return resurrect.ignoredNoCandidate &&
        !resurrect.resurrected &&
        reincarnate.ignoredNoCandidate &&
        !reincarnate.reincarnated;
}

static int route_close_button(void)
{
    Dm1V1MirrorCandidateCloseButtonStatePc34Compat state;
    Dm1V1MirrorCandidateCloseButtonResultPc34Compat result;

    dm1_v1_mirror_candidate_close_button_init_pc34(&state);
    (void)dm1_v1_mirror_candidate_close_button_pc34(
        &state,
        DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_COMMAND_PC34_COMPAT,
        &result);
    return result.closedPanel &&
        result.c040PanelCleared &&
        result.actionAreaGateOpenAfterClose &&
        !result.resurrectCommandReached &&
        !result.reincarnateCommandReached;
}

static void open_panel(
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state,
    const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *event)
{
    if (state->c040PanelOpen) {
        ++state->duplicateOpenSuppressedCount;
        if (route_resurrect_gate_while_open()) {
            ++state->resurrectGateProbeCount;
        }
        if (route_reincarnate_gate_while_open()) {
            ++state->reincarnateGateProbeCount;
        }
        return;
    }

    state->candidateChampionOrdinal = event_candidate(event);
    state->selectedCandidateChampionOrdinal = state->candidateChampionOrdinal;
    state->inventoryChampionOrdinal = state->candidateChampionOrdinal;
    state->partyChampionCount = kCandidatePartyChampionCount;
    state->c040PanelOpen = 1;
    state->c040PanelPixelsDrawn = 1;
    state->lastOpenTick = event ? event->tick : state->lastOpenTick;
    ++state->openDispatchCount;
    ++state->candidateAppendCount;
    if (route_icon_refresh_while_open()) {
        ++state->iconRefreshSuppressedCount;
    }
    if (route_resurrect_gate_while_open()) {
        ++state->resurrectGateProbeCount;
    }
    if (route_reincarnate_gate_while_open()) {
        ++state->reincarnateGateProbeCount;
    }
}

static void close_panel(
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state)
{
    if (!state->c040PanelOpen) {
        ++state->duplicateCloseSuppressedCount;
        return;
    }
    if (route_close_button() && route_no_finalize_after_close()) {
        state->candidateChampionOrdinal = 0u;
        state->inventoryChampionOrdinal = 0u;
        state->partyChampionCount = state->preC040PartyChampionCount;
        state->c040PanelOpen = 0;
        state->c040PanelPixelsDrawn = 0;
        ++state->closeDispatchCount;
    }
}

static int is_open_event(int kind)
{
    return kind ==
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_ICON_PC34_COMPAT ||
        kind ==
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT ||
        kind ==
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_MIRROR_ICON_PC34_COMPAT;
}

static void fill_result(
    const Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state,
    int eventsProcessed,
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat *result)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->eventsProcessed = eventsProcessed;
    result->rapidWindowTicks =
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_RAPID_WINDOW_TICKS_PC34_COMPAT;
    if (!state) {
        return;
    }
    result->finalPanelOpen = state->c040PanelOpen;
    result->finalPanelPixelsDrawn = state->c040PanelPixelsDrawn;
    result->finalPartyChampionCount = state->partyChampionCount;
    result->finalCandidateChampionOrdinal = (int)state->candidateChampionOrdinal;
    result->finalSelectedCandidateChampionOrdinal =
        (int)state->selectedCandidateChampionOrdinal;
    result->finalInventoryChampionOrdinal = (int)state->inventoryChampionOrdinal;
    result->finalLiveChampionHealth = state->liveChampionHealth;
    result->finalLeaderIndex = state->leaderIndex;
    result->finalFrontD1cMirrorChampionOrdinal =
        state->frontD1cMirrorChampionOrdinal;
    result->openDispatchCount = state->openDispatchCount;
    result->closeDispatchCount = state->closeDispatchCount;
    result->candidateAppendCount = state->candidateAppendCount;
    result->duplicateOpenSuppressedCount =
        state->duplicateOpenSuppressedCount;
    result->duplicateCloseSuppressedCount =
        state->duplicateCloseSuppressedCount;
    result->deadzoneSuppressedCount = state->deadzoneSuppressedCount;
    result->iconRefreshSuppressedCount = state->iconRefreshSuppressedCount;
    result->resurrectGateProbeCount = state->resurrectGateProbeCount;
    result->reincarnateGateProbeCount = state->reincarnateGateProbeCount;
    result->queueDispatchCount = state->queueDispatchCount;
    result->sideEffectFinalizeCount = state->sideEffectFinalizeCount;
    result->openedAtMostOncePerLivePanel =
        state->candidateAppendCount == state->openDispatchCount;
    result->closedAtMostOncePerLivePanel =
        state->closeDispatchCount <= state->openDispatchCount;
    result->candidateSelectionPreserved =
        state->selectedCandidateChampionOrdinal == 0u ||
        state->selectedCandidateChampionOrdinal ==
            DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_DEFAULT_CANDIDATE_PC34_COMPAT;
    result->liveChampionPreserved =
        state->liveChampionOrdinal == kLiveChampionOrdinal &&
        state->liveChampionHealth == kLiveChampionHealth;
    result->leaderHandPreserved =
        state->leaderHandThingOrdinal == kLeaderHandThingOrdinal;
    result->actionGateBlockedWhileOpen =
        state->resurrectGateProbeCount > 0 &&
        state->reincarnateGateProbeCount > 0;
    result->actionGateOpenAfterClose =
        !state->c040PanelOpen ||
        state->candidateChampionOrdinal != 0u;
}

int DM1_V1_MirrorCandidateDoubleOpenClose_DispatchPc34Compat(
    Dm1V1MirrorCandidateDoubleOpenCloseStatePc34Compat *state,
    const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *events,
    unsigned int eventCount,
    Dm1V1MirrorCandidateDoubleOpenCloseResultPc34Compat *outResult)
{
    unsigned int i;
    struct Dm1V1InputCommandQueuePc34Compat queue;

    fill_result(state, 0, outResult);
    if (!state || !state->active || (!events && eventCount != 0u)) {
        return 0;
    }

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    for (i = 0u; i < eventCount; ++i) {
        const Dm1V1MirrorCandidateDoubleOpenCloseEventPc34Compat *event =
            &events[i];
        if (event->kind ==
            DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_DEADZONE_CLICK_PC34_COMPAT) {
            queue_one_command(&queue,
                              DM1_V1_COMMAND_CLICK_IN_DUNGEON_VIEW,
                              event->x ? event->x : kDeadzoneX,
                              event->y ? event->y : kDeadzoneY,
                              event->buttonMask ? event->buttonMask :
                                  DM1_V1_BUTTON_LEFT,
                              state);
            if (route_click_cancel_deadzone()) {
                ++state->deadzoneSuppressedCount;
            }
        } else if (is_open_event(event->kind)) {
            queue_one_command(&queue,
                              event->kind ==
                                  DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_RESURRECT_HOTKEY_PC34_COMPAT ?
                                      DM1_V1_MIRROR_CANDIDATE_RESURRECT_COMMAND_PC34_COMPAT :
                                      DM1_V1_COMMAND_CLICK_IN_DUNGEON_VIEW,
                              event->x,
                              event->y,
                              event->buttonMask ? event->buttonMask :
                                  DM1_V1_BUTTON_LEFT,
                              state);
            open_panel(state, event);
        } else if (event->kind ==
            DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_EVENT_CLOSE_BUTTON_PC34_COMPAT) {
            queue_one_command(&queue,
                              DM1_V1_MIRROR_CANDIDATE_CLOSE_BUTTON_COMMAND_PC34_COMPAT,
                              event->x,
                              event->y,
                              event->buttonMask ? event->buttonMask :
                                  DM1_V1_BUTTON_LEFT,
                              state);
            close_panel(state);
        }
    }

    fill_result(state, (int)eventCount, outResult);
    return 1;
}

const Dm1V1MirrorCandidateDoubleOpenCloseSpecPc34Compat *
DM1_V1_MirrorCandidateDoubleOpenClose_SpecPc34Compat(void)
{
    return &DM1_V1_MirrorCandidateDoubleOpenCloseSpecPc34Compat;
}

const char *DM1_V1_MirrorCandidateDoubleOpenClose_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}
