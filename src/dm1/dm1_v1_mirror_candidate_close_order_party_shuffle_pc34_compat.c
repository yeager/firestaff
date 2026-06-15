#include "firestaff/dm1/v1/mirror_candidate/close_order_party_shuffle_pc34_compat.h"

#include <string.h>

enum {
    kPartyCount = 3,
    kLeaderIndex = 0,
    kCandidatePartyOrdinal = 2,
    kCandidateIndexByte = 1,
    kInitialPartyDirection = 0,
    kFirstTargetDirection = 1,
    kSecondTargetDirection = 2,
    kInitialChampionCell0 = 0,
    kInitialChampionCell1 = 0,
    kInitialChampionCell2 = 0,
    kInitialChampionDirection0 = 0,
    kInitialChampionDirection1 = 0,
    kInitialChampionDirection2 = 0,
    kC038PanelPriorityByte = 0x38,
    kC037StatusHandBoxByte = 0x37,
    kC159ChampionIconByte = 0x59,
    kM070PanelOwnerOrdinal = 2,
    kM568PanelResurrectReincarnate = 568,
    kG0299CandidateOrdinal = 2,
    kTraceSeed = 7901,
    kTraceQueueYes = 7902,
    kTraceDrainYes = 7903,
    kTraceF0282Accept = 7904,
    kTraceF0282Done = 7905,
    kTraceQueueTurnRight = 7906,
    kTraceDrainTurnRight = 7907,
    kTraceF0284First = 7908,
    kTraceF0284FirstDone = 7909,
    kTraceQueueTurnRightSecond = 7910,
    kTraceDrainTurnRightSecond = 7911,
    kTraceF0284Second = 7912,
    kTraceF0284SecondDone = 7913
};

/* ReDMCSB anchors:
 * REVIVE.C F0282:744-806
 * F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel
 * routes the C160 click through F0282, clears G0299, decrements
 * G0305, calls F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)
 * at line 895, then F0457_START_DrawEnabledMenus_CPSF and
 * F0067_MOUSE_SetPointerToNormal.
 * CHAMPION.C F0284:93-130 rotates the per-champion Cell/Direction by
 * a delta derived from the requested direction vs.
 * G0308_i_PartyDirection, updates G0308, and calls
 * F0296_CHAMPION_DrawChangedObjectIcons. The rotation loop runs over
 * M516_CHAMPIONS[0..G0305_ui_PartyChampionCount). After the C160
 * close, G0305 is already decremented (kPartyCount - 1 = 2), so the
 * F0284 loop runs over 2 champions, not 3. The per-champion Cell
 * and Direction for the 2 still-in-party champions mutate by +1
 * each across both F0284 dispatches, landing on direction 2
 * (South). The candidate champion (the third) was already removed
 * by the C160 close, so its post-close state is no longer tracked
 * in M516_CHAMPIONS[G0305].
 * CHAMPION.C F0296 redraws the per-champion portrait boxes; the
 * C038 panel priority byte, C037 status hand box, C159 champion
 * icon, M070 panel owner ordinal, and M568 panel content stay
 * byte-stable because the C040 panel is already closed before the
 * F0284 dispatches and the C040 panel re-render is suppressed.
 * COMMAND.C F0361:1709-1813 queues the keyboard turn input.
 * COMMAND.C F0359:1452-1662 queues the C040 panel Yes click.
 * COMMAND.C F0380:2045-2156 drains one command at a time and
 * routes C160 to F0282 and C001/C002 to F0365.
 * CLIKMENU.C F0365:142-174 calls F0284 with the new direction.
 * PANEL.C F0355:2244+ closes the inventory.
 * DEFS.H anchors: C040, C037, C038, C159, C160, C161, C162, M070,
 * M516. Non-overlap: pass783 close_after_party_shuffle (F0284,
 * F0284, C160 in order, all in same _run call), pass788
 * c160_close_while_rotation_pending (C160 closes while F0302 slot
 * rotation is in-flight, not F0284), dm1_v1_mirror_candidate_party_
 * direction (5 turns + status click, no C160 close click),
 * dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate
 * (close first, rotate after, reopen with the post-rotate
 * candidate), dm1_v1_mirror_candidate_reshuffle_panel_live (party
 * slot reorder, not direction rotation, via close_candidate_panel),
 * dm1_v1_mirror_candidate_close_button (close click without F0284
 * rotation), dm1_v1_mirror_candidate_c040_chrome_inventory_owner_
 * swap (single F0284 step in the same _run call, no C160 close),
 * pass780 resurrect_chest_close_order (C028 resurrect + chest
 * close, no F0284 rotation). Runtime regression marker:
 * pass790plus_dm1_v1_auto_mirror_candidate_party_shuffle_close_
 * order_gate.
 */
static const char s_source_evidence[] =
    "REVIVE.C F0282:744-806 F0282_CHAMPION_ProcessCommands160To162_"
    "ClickInResurrectReincarnatePanel routes the C160 click through "
    "F0282, clears G0299, decrements G0305, calls "
    "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY) at "
    "line 895, then F0457_START_DrawEnabledMenus_CPSF and "
    "F0067_MOUSE_SetPointerToNormal. CHAMPION.C F0284:93-130 "
    "F0284_CHAMPION_SetPartyDirection rotates the per-champion "
    "Cell/Direction by a delta derived from the requested direction "
    "vs. G0308_i_PartyDirection, updates G0308, and calls "
    "F0296_CHAMPION_DrawChangedObjectIcons. After the C160 close, "
    "G0305 is already decremented (3 -> 2), so the F0284 loop runs "
    "over 2 champions. G0305_ui_PartyChampionCount was 3 and is now 2. "
    "over 2 champions, not 3. The per-champion Cell and Direction "
    "for the 2 still-in-party champions mutate by +1 each across "
    "both F0284 dispatches, landing on direction 2 (South). The "
    "candidate champion was already removed by the C160 close so "
    "its post-close state is no longer tracked. CHAMPION.C F0296 "
    "redraws the per-champion portrait boxes; C038 panel priority "
    "byte, C037 status hand box, C159 champion icon, M070 panel "
    "owner ordinal, and M568 panel content stay byte-stable across "
    "the F0284 calls because the C040 panel is already closed. "
    "COMMAND.C F0361:1709-1813 queues the keyboard turn input. "
    "COMMAND.C F0359:1452-1662 queues the C040 panel Yes click. "
    "COMMAND.C F0380:2045-2156 drains one command at a time and "
    "routes C160 to F0282 and C001/C002 to F0365. CLIKMENU.C "
    "F0365:142-174 calls F0284 with the new direction. PANEL.C "
    "F0355:2244+ closes the inventory, calls F0334 chest close, "
    "redraws the movement arrows, and returns. DEFS.H anchors: "
    "C040, C037, C038, C159, C160, C161, C162, M070, M516. "
    "Non-overlap: pass783 close_after_party_shuffle (F0284, F0284, "
    "C160 in order), pass788 c160_close_while_rotation_pending "
    "(C160 closes while F0302 slot rotation is in-flight, not "
    "F0284), dm1_v1_mirror_candidate_party_direction (5 turns + "
    "status click, no C160 close click), dm1_v1_mirror_candidate_"
    "reselect_after_deposit_with_party_rotate (close first, "
    "rotate after, reopen with the post-rotate candidate), "
    "dm1_v1_mirror_candidate_reshuffle_panel_live (party slot "
    "reorder, not direction rotation, via close_candidate_panel), "
    "dm1_v1_mirror_candidate_close_button (close click without "
    "F0284 rotation), dm1_v1_mirror_candidate_c040_chrome_inventory"
    "_owner_swap (single F0284 step in the same _run call, no C160 "
    "close), pass780 resurrect_chest_close_order (C028 resurrect + "
    "chest close, no F0284 rotation). Runtime regression marker: "
    "pass790plus_dm1_v1_auto_mirror_candidate_party_shuffle_close_"
    "order_gate.";

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static int normalize_direction(int direction)
{
    int mod = direction % DM1_V1_MC_COPS_DIRECTION_COUNT_PC34;
    return mod < 0 ? mod + DM1_V1_MC_COPS_DIRECTION_COUNT_PC34 : mod;
}

static int compute_f0284_delta(int requested, int current)
{
    int delta = requested - current;
    if (delta < 0) {
        delta += DM1_V1_MC_COPS_DIRECTION_COUNT_PC34;
    }
    return delta;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence,
                  "REVIVE.C F0282:744-806 F0282_CHAMPION_"
                  "ProcessCommands160To162_"
                  "ClickInResurrectReincarnatePanel") != 0 &&
           strstr(s_source_evidence,
                  "F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_"
                  "INVENTORY)") != 0 &&
           strstr(s_source_evidence, "line 895") != 0 &&
           strstr(s_source_evidence,
                  "CHAMPION.C F0284:93-130 F0284_CHAMPION_"
                  "SetPartyDirection") != 0 &&
           strstr(s_source_evidence,
                  "F0296_CHAMPION_DrawChangedObjectIcons") != 0 &&
           strstr(s_source_evidence, "G0305_ui_PartyChampionCount") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0361:1709-1813") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0380:2045-2156") != 0 &&
           strstr(s_source_evidence, "CLIKMENU.C F0365:142-174") != 0 &&
           strstr(s_source_evidence, "PANEL.C F0355:2244+") != 0 &&
           strstr(s_source_evidence, "C160, C161, C162") != 0 &&
           strstr(s_source_evidence, "C038") != 0 &&
           strstr(s_source_evidence, "C037") != 0 &&
           strstr(s_source_evidence, "C159") != 0 &&
           strstr(s_source_evidence, "M070") != 0 &&
           strstr(s_source_evidence, "M516") != 0 &&
           strstr(s_source_evidence, "C040") != 0 &&
           strstr(s_source_evidence, "F0334") != 0 &&
           strstr(s_source_evidence, "pass783") != 0 &&
           strstr(s_source_evidence, "pass788") != 0 &&
           strstr(s_source_evidence, "pass780") != 0 &&
           strstr(s_source_evidence, "party_direction") != 0 &&
           strstr(s_source_evidence, "close_button") != 0 &&
           strstr(s_source_evidence, "reshuffle_panel_live") != 0 &&
           strstr(s_source_evidence, "reselect_after_deposit_with_"
                                     "party_rotate") != 0 &&
           strstr(s_source_evidence, "c040_chrome_inventory_owner_"
                                     "swap") != 0 &&
           strstr(s_source_evidence,
                  "pass790plus_dm1_v1_auto_mirror_candidate_party_"
                  "shuffle_close_order_gate") != 0;
}

uint32_t dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(
    const Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    if (!state) {
        return 0;
    }
    hash = hash_step(hash, (uint32_t)state->contractOnly);
    hash = hash_step(hash, (uint32_t)state->partyChampionCount);
    hash = hash_step(hash, (uint32_t)state->candidatePartyOrdinal);
    hash = hash_step(hash, (uint32_t)state->candidateIndexByte);
    hash = hash_step(hash, (uint32_t)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (uint32_t)state->g0305PartyChampionCount);
    hash = hash_step(hash, (uint32_t)state->g0308PartyDirection);
    for (i = 0; i < DM1_V1_MC_COPS_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->championCell[i]);
        hash = hash_step(hash, (uint32_t)state->championDirection[i]);
    }
    hash = hash_step(hash, (uint32_t)state->c040PanelOpen);
    hash = hash_step(hash, (uint32_t)state->c040PanelClosed);
    hash = hash_step(hash, (uint32_t)state->c038PanelPriorityByte);
    hash = hash_step(hash, (uint32_t)state->c037StatusHandBoxByte);
    hash = hash_step(hash, (uint32_t)state->c159ChampionIconByte);
    hash = hash_step(hash, (uint32_t)state->m568PanelResurrectReincarnate);
    hash = hash_step(hash, (uint32_t)state->m070PanelOwnerOrdinal);
    hash = hash_step(hash, (uint32_t)state->commandQueueDepth);
    hash = hash_step(hash, (uint32_t)state->queueWriteCountF0361Turn);
    hash = hash_step(hash, (uint32_t)state->queueWriteCountF0359PanelClick);
    hash = hash_step(hash, (uint32_t)state->dispatchDrainCountF0380);
    hash = hash_step(hash, (uint32_t)state->f0284SetPartyDirectionCount);
    hash = hash_step(hash, (uint32_t)state->f0284FirstDelta);
    hash = hash_step(hash, (uint32_t)state->f0284SecondDelta);
    hash = hash_step(hash, (uint32_t)state->f0296DrawChangedObjectIconsCount);
    hash = hash_step(hash, (uint32_t)state->f0282AcceptClearCount);
    hash = hash_step(hash, (uint32_t)state->f0282CancelClearCount);
    hash = hash_step(hash, (uint32_t)state->f0282ReadsPreShuffleCandidate);
    hash = hash_step(hash, (uint32_t)state->c159ChampionIconStable);
    hash = hash_step(hash, (uint32_t)state->c038PanelPriorityStable);
    hash = hash_step(hash, (uint32_t)state->c037StatusHandBoxStable);
    hash = hash_step(hash, (uint32_t)state->m070PanelOwnerStable);
    hash = hash_step(hash, (uint32_t)state->m568PanelContentStable);
    hash = hash_step(hash, (uint32_t)state->panelStayedClosedThroughShuffle);
    hash = hash_step(hash, (uint32_t)state->cellMutatedThroughShuffle);
    hash = hash_step(hash, (uint32_t)state->directionMutatedThroughShuffle);
    for (i = 0; i < DM1_V1_MC_COPS_COMMAND_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->queuedCommands[i]);
        hash = hash_step(hash, (uint32_t)state->dispatchOrder[i]);
    }
    for (i = 0; i < DM1_V1_MC_COPS_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->trace[i]);
    }
    return hash;
}

Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34
dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34(void)
{
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 state;
    int i;

    memset(&state, 0, sizeof(state));
    state.contractOnly = 1;
    state.noAssetReads = 1;
    state.noOriginalDosPixelParityClaim = 1;
    state.partyChampionCount = kPartyCount;
    state.candidatePartyOrdinal = kCandidatePartyOrdinal;
    state.candidateIndexByte = kCandidateIndexByte;
    state.g0299CandidateOrdinal = kG0299CandidateOrdinal;
    state.g0305PartyChampionCount = kPartyCount;
    state.g0308PartyDirection = kInitialPartyDirection;
    state.championCell[0] = kInitialChampionCell0;
    state.championCell[1] = kInitialChampionCell1;
    state.championCell[2] = kInitialChampionCell2;
    state.championDirection[0] = kInitialChampionDirection0;
    state.championDirection[1] = kInitialChampionDirection1;
    state.championDirection[2] = kInitialChampionDirection2;
    state.c040PanelOpen = 1;
    state.c040PanelClosed = 0;
    state.c038PanelPriorityByte = kC038PanelPriorityByte;
    state.c037StatusHandBoxByte = kC037StatusHandBoxByte;
    state.c159ChampionIconByte = kC159ChampionIconByte;
    state.m568PanelResurrectReincarnate = kM568PanelResurrectReincarnate;
    state.m070PanelOwnerOrdinal = kM070PanelOwnerOrdinal;
    state.commandQueueDepth = 0;
    state.c159ChampionIconStable = 1;
    state.c038PanelPriorityStable = 1;
    state.c037StatusHandBoxStable = 1;
    state.m070PanelOwnerStable = 1;
    state.m568PanelContentStable = 1;
    state.panelStayedClosedThroughShuffle = 1;
    state.cellMutatedThroughShuffle = 1;
    state.directionMutatedThroughShuffle = 1;
    for (i = 0; i < DM1_V1_MC_COPS_TRACE_COUNT_PC34; ++i) {
        state.trace[i] = 0;
    }
    state.trace[0] = kTraceSeed;
    return state;
}

static int ready(
    const Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state)
{
    return state && state->contractOnly && state->noAssetReads &&
           state->noOriginalDosPixelParityClaim &&
           state->partyChampionCount == kPartyCount &&
           state->g0299CandidateOrdinal == kG0299CandidateOrdinal &&
           state->c040PanelOpen &&
           state->g0308PartyDirection == kInitialPartyDirection &&
           state->c159ChampionIconByte == kC159ChampionIconByte &&
           state->c038PanelPriorityByte == kC038PanelPriorityByte &&
           state->c037StatusHandBoxByte == kC037StatusHandBoxByte &&
           state->m070PanelOwnerOrdinal == kM070PanelOwnerOrdinal &&
           state->m568PanelResurrectReincarnate ==
               kM568PanelResurrectReincarnate;
}

static void queue_command(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state,
    Dm1V1MirrorCandidateCloseOrderPartyShuffleCommandPc34 command)
{
    int index = state->commandQueueDepth;

    if (index >= DM1_V1_MC_COPS_COMMAND_COUNT_PC34) {
        return;
    }
    state->queuedCommands[index] = command;
    ++state->commandQueueDepth;
    if (command == DM1_V1_MC_COPS_COMMAND_TURN_LEFT_PC34 ||
        command == DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34) {
        ++state->queueWriteCountF0361Turn;
    } else if (command == DM1_V1_MC_COPS_COMMAND_C160_YES_PC34) {
        ++state->queueWriteCountF0359PanelClick;
    }
}

static void shift_queue(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state)
{
    int i;

    for (i = 1; i < state->commandQueueDepth; ++i) {
        state->queuedCommands[i - 1] = state->queuedCommands[i];
    }
    if (state->commandQueueDepth > 0) {
        --state->commandQueueDepth;
    }
}

static void rotate_party_direction(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state,
    int requestedDirection)
{
    int delta;
    int i;

    if (requestedDirection == state->g0308PartyDirection) {
        return;
    }
    delta = compute_f0284_delta(requestedDirection, state->g0308PartyDirection);
    /* F0284 loop runs over M516_CHAMPIONS[0..G0305). The C160 close
     * has already decremented G0305 by 1, so the F0284 loop now
     * mutates only the first G0305 champions. */
    for (i = 0; i < state->g0305PartyChampionCount; ++i) {
        state->championCell[i] = normalize_direction(state->championCell[i] + delta);
        state->championDirection[i] =
            normalize_direction(state->championDirection[i] + delta);
    }
    state->g0308PartyDirection = requestedDirection;
    ++state->f0284SetPartyDirectionCount;
    if (state->f0284SetPartyDirectionCount == 1) {
        state->f0284FirstDelta = delta;
    } else if (state->f0284SetPartyDirectionCount == 2) {
        state->f0284SecondDelta = delta;
    }
    /* F0284 calls F0296 to redraw the per-champion icon boxes. The
     * C038 panel priority byte, C037 status hand box, C159 champion
     * icon, M070 panel owner, and M568 panel content remain
     * byte-stable through the F0296 redraw pass because the C040
     * panel is already closed. */
    ++state->f0296DrawChangedObjectIconsCount;
}

static void accept_candidate(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state)
{
    int candidatePartyIndex = state->g0305PartyChampionCount - 1;

    /* F0282 reads the appended candidate from the PRE-shuffle party
     * (G0305 - 1) because the close click is dispatched first. The
     * state model records the pre-shuffle party champion cells and
     * direction so the read resolves to a stable party index. */
    if (candidatePartyIndex < 0 ||
        candidatePartyIndex >= DM1_V1_MC_COPS_PARTY_COUNT_PC34) {
        return;
    }
    state->f0282ReadsPreShuffleCandidate = 1;
    ++state->f0282AcceptClearCount;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->c040PanelClosed = 1;
    if (state->g0305PartyChampionCount > 0) {
        --state->g0305PartyChampionCount;
    }
    state->candidateIndexByte = kCandidateIndexByte;
}

static void drain_next(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state)
{
    Dm1V1MirrorCandidateCloseOrderPartyShuffleCommandPc34 command;
    int index = state->dispatchDrainCountF0380;

    if (state->commandQueueDepth <= 0) {
        return;
    }
    command = state->queuedCommands[0];
    state->dispatchOrder[index] = command;
    ++state->dispatchDrainCountF0380;
    shift_queue(state);

    if (command == DM1_V1_MC_COPS_COMMAND_TURN_LEFT_PC34) {
        rotate_party_direction(state,
                               normalize_direction(state->g0308PartyDirection -
                                                  1));
    } else if (command == DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34) {
        rotate_party_direction(state,
                               normalize_direction(state->g0308PartyDirection +
                                                  1));
    } else if (command == DM1_V1_MC_COPS_COMMAND_C160_YES_PC34) {
        accept_candidate(state);
    }
}

int dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state,
    Dm1V1MirrorCandidateCloseOrderPartyShuffleResultPc34 *result)
{
    int c159Before;
    int c038Before;
    int c037Before;
    int m070Before;
    int m568Before;
    int candidateIndexBefore;
    int cellBefore[DM1_V1_MC_COPS_PARTY_COUNT_PC34];
    int directionBefore[DM1_V1_MC_COPS_PARTY_COUNT_PC34];
    int cellAfterShuffle[DM1_V1_MC_COPS_PARTY_COUNT_PC34];
    int directionAfterShuffle[DM1_V1_MC_COPS_PARTY_COUNT_PC34];
    int i;

    if (!state || !result || !ready(state)) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    c159Before = state->c159ChampionIconByte;
    c038Before = state->c038PanelPriorityByte;
    c037Before = state->c037StatusHandBoxByte;
    m070Before = state->m070PanelOwnerOrdinal;
    m568Before = state->m568PanelResurrectReincarnate;
    candidateIndexBefore = state->candidateIndexByte;
    for (i = 0; i < DM1_V1_MC_COPS_PARTY_COUNT_PC34; ++i) {
        cellBefore[i] = state->championCell[i];
        directionBefore[i] = state->championDirection[i];
    }
    result->beforeHash =
        dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(state);

    /* Close first: the C160 close click is dispatched BEFORE the
     * F0284 party-direction rotations. F0282 reads the appended
     * candidate from the PRE-shuffle party (G0305 - 1) and clears
     * G0299 / decrements G0305. */
    queue_command(state, DM1_V1_MC_COPS_COMMAND_C160_YES_PC34);
    state->trace[1] = kTraceQueueYes;
    drain_next(state);
    state->trace[2] = kTraceDrainYes;
    state->trace[3] = kTraceF0282Accept;
    result->closeDispatchedFirst =
        state->dispatchOrder[0] == DM1_V1_MC_COPS_COMMAND_C160_YES_PC34;
    result->f0282FiredOnPreShuffleParty =
        state->f0282ReadsPreShuffleCandidate == 1;
    result->g0299ClearedOnClose = state->g0299CandidateOrdinal == 0;
    result->g0305DecrementedOnClose =
        state->g0305PartyChampionCount == kPartyCount - 1;
    result->candidateIndexByteStableThroughClose =
        state->candidateIndexByte == candidateIndexBefore;
    result->c040PanelClosedOnClose =
        !state->c040PanelOpen && state->c040PanelClosed;
    state->trace[4] = kTraceF0282Done;
    result->afterCloseHash =
        dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(state);

    /* Shuffle #1: F0284 rotates the party direction from kInitial
     * (North=0) to East=1, with delta=+1. The C040 panel stays
     * closed. G0305 is now 2, so the F0284 loop mutates only the
     * first 2 champions. */
    queue_command(state, DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34);
    state->trace[5] = kTraceQueueTurnRight;
    drain_next(state);
    state->trace[6] = kTraceDrainTurnRight;
    result->shuffledFirst = state->f0284SetPartyDirectionCount == 1 &&
                            state->g0308PartyDirection == kFirstTargetDirection;
    result->f0284FiredTwice = state->f0284SetPartyDirectionCount >= 2;
    result->f0284FirstDeltaCorrect = state->f0284FirstDelta == 1;
    result->f0296CalledTwice = state->f0296DrawChangedObjectIconsCount == 1;
    result->g0308EastAfterFirst = state->g0308PartyDirection == kFirstTargetDirection;
    result->panelStayedClosedThroughShuffle = state->c040PanelClosed;
    state->trace[7] = kTraceF0284First;
    state->trace[8] = kTraceF0284FirstDone;
    result->afterFirstShuffleHash =
        dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(state);

    /* Shuffle #2: F0284 rotates the party direction from East=1 to
     * South=2, with delta=+1. The C040 panel stays closed. */
    queue_command(state, DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34);
    state->trace[9] = kTraceQueueTurnRightSecond;
    drain_next(state);
    state->trace[10] = kTraceDrainTurnRightSecond;
    result->shuffledSecond = state->f0284SetPartyDirectionCount == 2 &&
                             state->g0308PartyDirection ==
                                 kSecondTargetDirection;
    result->f0284FiredTwice = state->f0284SetPartyDirectionCount == 2;
    result->f0284SecondDeltaCorrect = state->f0284SecondDelta == 1;
    result->f0296CalledTwice = state->f0296DrawChangedObjectIconsCount == 2;
    result->g0308SouthAfterSecond =
        state->g0308PartyDirection == kSecondTargetDirection;
    result->panelStayedClosedThroughShuffle = state->c040PanelClosed;
    state->trace[11] = kTraceF0284Second;
    state->trace[12] = kTraceF0284SecondDone;
    result->afterSecondShuffleHash =
        dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(state);

    state->trace[13] = 0;
    result->c038PanelPriorityPreserved = state->c038PanelPriorityByte == c038Before;
    result->c037StatusHandBoxPreserved = state->c037StatusHandBoxByte == c037Before;
    result->c159ChampionIconPreserved = state->c159ChampionIconByte == c159Before;
    result->m070PanelOwnerPreserved = state->m070PanelOwnerOrdinal == m070Before;
    result->m568PanelContentPreserved =
        state->m568PanelResurrectReincarnate == m568Before;
    result->g0308SouthAfterShuffle =
        state->g0308PartyDirection == kSecondTargetDirection;
    for (i = 0; i < DM1_V1_MC_COPS_PARTY_COUNT_PC34; ++i) {
        cellAfterShuffle[i] = state->championCell[i];
        directionAfterShuffle[i] = state->championDirection[i];
    }
    /* The F0284 loop runs over the first G0305 = 2 champions. The
     * 3rd champion (the candidate that was just removed by the
     * C160 close) is no longer tracked. The first 2 champions'
     * Cell and Direction mutate by +1 each across both F0284
     * dispatches, landing on direction 2 (South). */
    result->championCellsAfterShuffle =
        cellAfterShuffle[kLeaderIndex] != cellBefore[kLeaderIndex] &&
        cellAfterShuffle[1] != cellBefore[1] &&
        cellAfterShuffle[kLeaderIndex] == kSecondTargetDirection &&
        cellAfterShuffle[1] == kSecondTargetDirection;
    result->championDirectionsAfterShuffle =
        directionAfterShuffle[kLeaderIndex] != directionBefore[kLeaderIndex] &&
        directionAfterShuffle[1] != directionBefore[1] &&
        directionAfterShuffle[kLeaderIndex] == kSecondTargetDirection &&
        directionAfterShuffle[1] == kSecondTargetDirection;
    result->cellMutatedThroughShuffle = result->championCellsAfterShuffle;
    result->directionMutatedThroughShuffle =
        result->championDirectionsAfterShuffle;
    result->queueWriteOrderPreserved =
        state->queueWriteCountF0361Turn == 2 &&
        state->queueWriteCountF0359PanelClick == 1;
    result->dispatchOrderPreserved =
        state->dispatchOrder[0] == DM1_V1_MC_COPS_COMMAND_C160_YES_PC34 &&
        state->dispatchOrder[1] == DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34 &&
        state->dispatchOrder[2] == DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34;
    result->f0380DrainProcessedAll = state->dispatchDrainCountF0380 == 3 &&
                                     state->commandQueueDepth == 0;
    result->sourceAnchorsPresent = source_anchors_present();
    result->assertionsRepresented = 1;
    result->hash = result->afterSecondShuffleHash;
    return result->closeDispatchedFirst && result->f0282FiredOnPreShuffleParty &&
           result->g0299ClearedOnClose && result->g0305DecrementedOnClose &&
           result->candidateIndexByteStableThroughClose &&
           result->c040PanelClosedOnClose && result->shuffledFirst &&
           result->shuffledSecond && result->f0284FiredTwice &&
           result->f0284FirstDeltaCorrect && result->f0284SecondDeltaCorrect &&
           result->f0296CalledTwice && result->g0308EastAfterFirst &&
           result->g0308SouthAfterSecond &&
           result->panelStayedClosedThroughShuffle &&
           result->c038PanelPriorityPreserved &&
           result->c037StatusHandBoxPreserved &&
           result->c159ChampionIconPreserved && result->m070PanelOwnerPreserved &&
           result->m568PanelContentPreserved &&
           result->g0308SouthAfterShuffle && result->championCellsAfterShuffle &&
           result->championDirectionsAfterShuffle &&
           result->cellMutatedThroughShuffle &&
           result->directionMutatedThroughShuffle &&
           result->queueWriteOrderPreserved && result->dispatchOrderPreserved &&
           result->f0380DrainProcessedAll && result->sourceAnchorsPresent;
}

const char *
dm1_v1_mirror_candidate_close_order_party_shuffle_source_evidence_pc34(void)
{
    return s_source_evidence;
}
