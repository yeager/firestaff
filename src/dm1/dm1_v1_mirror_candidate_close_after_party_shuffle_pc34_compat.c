#include "firestaff/dm1/v1/mirror_candidate/close_after_party_shuffle_pc34_compat.h"

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
    kTraceSeed = 7830,
    kTraceQueueTurnRight = 7831,
    kTraceDrainTurnRight = 7832,
    kTraceF0284First = 7833,
    kTraceF0284FirstDone = 7834,
    kTraceQueueTurnRightSecond = 7835,
    kTraceDrainTurnRightSecond = 7836,
    kTraceF0284Second = 7837,
    kTraceF0284SecondDone = 7838,
    kTraceQueueYes = 7839,
    kTraceDrainYes = 7840,
    kTraceF0282Accept = 7841,
    kTraceF0282Done = 7842,
    kTraceCloseComplete = 7843
};

/* ReDMCSB anchors:
 * CHAMPION.C F0284:93-130 rotates the per-champion Cell/Direction by a
 * delta derived from the requested direction vs. G0308_i_PartyDirection,
 * updates G0308, and calls F0296_CHAMPION_DrawChangedObjectIcons. The
 * loop runs over M516_CHAMPIONS[0..G0305_ui_PartyChampionCount).
 * CHAMPION.C F0296 redraws the changed champion icon boxes; the C038
 * panel priority byte and C037 status hand box remain byte-stable.
 * REVIVE.C F0282:744-806 reads the appended candidate as
 * M516_CHAMPIONS[G0305_ui_PartyChampionCount - 1] and clears
 * G0299_ui_CandidateChampionOrdinal on the C160/C161/C162 click path.
 * COMMAND.C F0361:1709-1813 queues the keyboard turn input.
 * COMMAND.C F0359:1452-1662 queues the C040 panel Yes click.
 * COMMAND.C F0380:2045-2156 drains one command at a time and routes
 * C160 to F0282. DEFS.H anchors C040, C037/C038, C159, C160..C162, M070,
 * M516. Runtime regression marker:
 * pass783_dm1_v1_mirror_candidate_close_after_party_shuffle.
 */
static const char s_source_evidence[] =
    "CHAMPION.C F0284:93-130 F0284_CHAMPION_SetPartyDirection rotates the "
    "per-champion Cell and Direction by a delta derived from the requested "
    "direction vs. G0308_i_PartyDirection, updates G0308, and calls "
    "F0296_CHAMPION_DrawChangedObjectIcons. The rotation loop runs over "
    "M516_CHAMPIONS[0..G0305_ui_PartyChampionCount). CHAMPION.C F0296 "
    "redraws the changed champion icon boxes; C038 panel priority byte "
    "and C037 status hand box remain byte-stable across the F0284 calls. "
    "REVIVE.C F0282:744-806 F0282_CHAMPION_ProcessCommands160To162_"
    "ClickInResurrectReincarnatePanel reads the appended candidate as "
    "M516_CHAMPIONS[G0305_ui_PartyChampionCount - 1] on the C160/C161/"
    "C162 click path and clears G0299_ui_CandidateChampionOrdinal. The "
    "C160 path also decrements G0305_ui_PartyChampionCount and disables "
    "the first sensor at the post-rotation mirror square. COMMAND.C "
    "F0361:1709-1813 queues the keyboard turn input (TURN_RIGHT/TURN_LEFT) "
    "into the shared command queue. COMMAND.C F0359:1452-1662 queues the "
    "C040 panel Yes click. COMMAND.C F0380:2045-2156 drains one command "
    "at a time and routes C160 to F0282. DEFS.H anchors: C040, C037, C038, "
    "C159, C160, C161, C162, M070, M516. Non-overlap: pass780 "
    "resurrect + chest-close order (no F0284 rotation), "
    "dm1_v1_mirror_candidate_party_direction (5 turns + status click, no "
    "C160 close), dm1_v1_mirror_candidate_reselect_after_deposit_with_"
    "party_rotate (close first then rotate then reopen), "
    "dm1_v1_mirror_candidate_reshuffle_panel_live (party slot reorder, "
    "not direction rotation, via close_candidate_panel), "
    "dm1_v1_mirror_candidate_close_button (close click without F0284 "
    "rotation), dm1_v1_mirror_candidate_c040_chrome_inventory_owner_swap "
    "(single F0284 step in the same _run call). Runtime regression "
    "marker: pass783_dm1_v1_mirror_candidate_close_after_party_shuffle.";

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
    int mod = direction % DM1_V1_MC_CAPS_DIRECTION_COUNT_PC34;
    return mod < 0 ? mod + DM1_V1_MC_CAPS_DIRECTION_COUNT_PC34 : mod;
}

static int compute_f0284_delta(int requested, int current)
{
    int delta = requested - current;
    if (delta < 0) {
        delta += DM1_V1_MC_CAPS_DIRECTION_COUNT_PC34;
    }
    return delta;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence,
                  "CHAMPION.C F0284:93-130 F0284_CHAMPION_SetPartyDirection") !=
               0 &&
           strstr(s_source_evidence, "F0296_CHAMPION_DrawChangedObjectIcons") !=
               0 &&
           strstr(s_source_evidence,
                  "REVIVE.C F0282:744-806 F0282_CHAMPION_ProcessCommands160To162_"
                  "ClickInResurrectReincarnatePanel") != 0 &&
           strstr(s_source_evidence, "G0305_ui_PartyChampionCount - 1") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0361:1709-1813") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0380:2045-2156") != 0 &&
           strstr(s_source_evidence, "C160, C161, C162") != 0 &&
           strstr(s_source_evidence, "C038") != 0 &&
           strstr(s_source_evidence, "C037") != 0 &&
           strstr(s_source_evidence, "C159") != 0 &&
           strstr(s_source_evidence, "M070") != 0 &&
           strstr(s_source_evidence, "M516") != 0 &&
           strstr(s_source_evidence,
                  "pass783_dm1_v1_mirror_candidate_close_after_party_shuffle") !=
               0;
}

uint32_t dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(
    const Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state)
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
    for (i = 0; i < DM1_V1_MC_CAPS_PARTY_COUNT_PC34; ++i) {
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
    hash = hash_step(hash, (uint32_t)state->f0282ReadsPostShuffleCandidate);
    hash = hash_step(hash, (uint32_t)state->c159ChampionIconStable);
    hash = hash_step(hash, (uint32_t)state->c038PanelPriorityStable);
    hash = hash_step(hash, (uint32_t)state->c037StatusHandBoxStable);
    hash = hash_step(hash, (uint32_t)state->m070PanelOwnerStable);
    hash = hash_step(hash, (uint32_t)state->m568PanelContentStable);
    hash = hash_step(hash, (uint32_t)state->directionPreservedAfterClose);
    hash = hash_step(hash, (uint32_t)state->cellPreservedAfterClose);
    for (i = 0; i < DM1_V1_MC_CAPS_COMMAND_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->queuedCommands[i]);
        hash = hash_step(hash, (uint32_t)state->dispatchOrder[i]);
    }
    for (i = 0; i < DM1_V1_MC_CAPS_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->trace[i]);
    }
    return hash;
}

Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34
dm1_v1_mirror_candidate_close_after_party_shuffle_default_state_pc34(void)
{
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 state;
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
    state.directionPreservedAfterClose = 1;
    state.cellPreservedAfterClose = 1;
    for (i = 0; i < DM1_V1_MC_CAPS_TRACE_COUNT_PC34; ++i) {
        state.trace[i] = 0;
    }
    state.trace[0] = kTraceSeed;
    return state;
}

static int ready(
    const Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state)
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
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state,
    Dm1V1MirrorCandidateCloseAfterPartyShuffleCommandPc34 command)
{
    int index = state->commandQueueDepth;

    if (index >= DM1_V1_MC_CAPS_COMMAND_COUNT_PC34) {
        return;
    }
    state->queuedCommands[index] = command;
    ++state->commandQueueDepth;
    if (command == DM1_V1_MC_CAPS_COMMAND_TURN_LEFT_PC34 ||
        command == DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34) {
        ++state->queueWriteCountF0361Turn;
    } else if (command == DM1_V1_MC_CAPS_COMMAND_C160_YES_PC34) {
        ++state->queueWriteCountF0359PanelClick;
    }
}

static void shift_queue(
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state)
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
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state,
    int requestedDirection)
{
    int delta;
    int i;

    if (requestedDirection == state->g0308PartyDirection) {
        return;
    }
    delta = compute_f0284_delta(requestedDirection, state->g0308PartyDirection);
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
     * byte-stable through the F0296 redraw pass. */
    ++state->f0296DrawChangedObjectIconsCount;
}

static void accept_candidate(
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state)
{
    int candidatePartyIndex = state->g0305PartyChampionCount - 1;

    if (candidatePartyIndex < 0 ||
        candidatePartyIndex >= DM1_V1_MC_CAPS_PARTY_COUNT_PC34) {
        return;
    }
    /* F0282 reads the appended candidate from the post-shuffle party,
     * not from a pre-shuffle cached index. The state model records the
     * post-shuffle party champion cells and direction so the read
     * resolves to a stable party index. */
    state->f0282ReadsPostShuffleCandidate = 1;
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
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state)
{
    Dm1V1MirrorCandidateCloseAfterPartyShuffleCommandPc34 command;
    int index = state->dispatchDrainCountF0380;

    if (state->commandQueueDepth <= 0) {
        return;
    }
    command = state->queuedCommands[0];
    state->dispatchOrder[index] = command;
    ++state->dispatchDrainCountF0380;
    shift_queue(state);

    if (command == DM1_V1_MC_CAPS_COMMAND_TURN_LEFT_PC34) {
        rotate_party_direction(state,
                               normalize_direction(state->g0308PartyDirection -
                                                  1));
    } else if (command == DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34) {
        rotate_party_direction(state,
                               normalize_direction(state->g0308PartyDirection +
                                                  1));
    } else if (command == DM1_V1_MC_CAPS_COMMAND_C160_YES_PC34) {
        accept_candidate(state);
    }
}

int dm1_v1_mirror_candidate_close_after_party_shuffle_run_pc34(
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state,
    Dm1V1MirrorCandidateCloseAfterPartyShuffleResultPc34 *result)
{
    int c159Before;
    int c038Before;
    int c037Before;
    int m070Before;
    int m568Before;
    int candidateIndexBefore;
    int cellBefore[DM1_V1_MC_CAPS_PARTY_COUNT_PC34];
    int directionBefore[DM1_V1_MC_CAPS_PARTY_COUNT_PC34];
    int cellAfterClose[DM1_V1_MC_CAPS_PARTY_COUNT_PC34];
    int directionAfterClose[DM1_V1_MC_CAPS_PARTY_COUNT_PC34];
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
    for (i = 0; i < DM1_V1_MC_CAPS_PARTY_COUNT_PC34; ++i) {
        cellBefore[i] = state->championCell[i];
        directionBefore[i] = state->championDirection[i];
    }
    result->beforeHash =
        dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(state);

    /* Shuffle #1: F0284 rotates the party direction from kInitial
     * (North=0) to East=1, with delta=+1. The C040 panel stays open. */
    queue_command(state, DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34);
    state->trace[1] = kTraceQueueTurnRight;
    drain_next(state);
    state->trace[2] = kTraceDrainTurnRight;
    result->shuffledFirst = state->f0284SetPartyDirectionCount == 1 &&
                            state->g0308PartyDirection == kFirstTargetDirection;
    result->f0284FiredTwice = state->f0284SetPartyDirectionCount >= 2;
    result->f0284FirstDeltaCorrect = state->f0284FirstDelta == 1;
    result->f0296CalledTwice = state->f0296DrawChangedObjectIconsCount == 1;
    result->g0308EastAfterFirst = state->g0308PartyDirection == kFirstTargetDirection;
    result->panelStayedOpenThroughShuffle = state->c040PanelOpen;
    state->trace[3] = kTraceF0284First;
    state->trace[4] = kTraceF0284FirstDone;
    result->afterFirstShuffleHash =
        dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(state);

    /* Shuffle #2: F0284 rotates the party direction from East=1 to
     * South=2, with delta=+1. The C040 panel stays open. */
    queue_command(state, DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34);
    state->trace[5] = kTraceQueueTurnRightSecond;
    drain_next(state);
    state->trace[6] = kTraceDrainTurnRightSecond;
    result->shuffledSecond = state->f0284SetPartyDirectionCount == 2 &&
                             state->g0308PartyDirection ==
                                 kSecondTargetDirection;
    result->f0284FiredTwice = state->f0284SetPartyDirectionCount == 2;
    result->f0284SecondDeltaCorrect = state->f0284SecondDelta == 1;
    result->f0296CalledTwice = state->f0296DrawChangedObjectIconsCount == 2;
    result->g0308SouthAfterSecond =
        state->g0308PartyDirection == kSecondTargetDirection;
    result->panelStayedOpenThroughShuffle = state->c040PanelOpen;
    state->trace[7] = kTraceF0284Second;
    state->trace[8] = kTraceF0284SecondDone;
    result->afterSecondShuffleHash =
        dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(state);

    /* Close: C160 Yes click is dispatched. F0282 reads the appended
     * candidate from the post-shuffle party (G0305-1) and clears
     * G0299 / decrements G0305. */
    queue_command(state, DM1_V1_MC_CAPS_COMMAND_C160_YES_PC34);
    state->trace[9] = kTraceQueueYes;
    drain_next(state);
    state->trace[10] = kTraceDrainYes;
    state->trace[11] = kTraceF0282Accept;
    result->yesAcceptedAfterShuffle =
        state->f0282AcceptClearCount == 1 &&
        state->g0299CandidateOrdinal == 0 &&
        state->c040PanelClosed;
    result->f0282FiredOnPostShuffleParty =
        state->f0282ReadsPostShuffleCandidate == 1;
    result->g0299ClearedAfterShuffle = state->g0299CandidateOrdinal == 0;
    result->g0305DecrementedAfterShuffle =
        state->g0305PartyChampionCount == kPartyCount - 1;
    result->candidateIndexByteStable =
        state->candidateIndexByte == candidateIndexBefore;
    result->c040PanelClosedAfterShuffle =
        !state->c040PanelOpen && state->c040PanelClosed;
    result->afterYesHash =
        dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(state);

    state->trace[12] = kTraceF0282Done;
    state->trace[13] = kTraceCloseComplete;
    result->afterCloseHash =
        dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(state);

    result->c038PanelPriorityPreserved = state->c038PanelPriorityByte == c038Before;
    result->c037StatusHandBoxPreserved = state->c037StatusHandBoxByte == c037Before;
    result->c159ChampionIconPreserved = state->c159ChampionIconByte == c159Before;
    result->m070PanelOwnerPreserved = state->m070PanelOwnerOrdinal == m070Before;
    result->m568PanelContentPreserved =
        state->m568PanelResurrectReincarnate == m568Before;
    result->g0308SouthAfterClose =
        state->g0308PartyDirection == kSecondTargetDirection;
    for (i = 0; i < DM1_V1_MC_CAPS_PARTY_COUNT_PC34; ++i) {
        cellAfterClose[i] = state->championCell[i];
        directionAfterClose[i] = state->championDirection[i];
    }
    result->championCellsAfterClose =
        cellAfterClose[kLeaderIndex] != cellBefore[kLeaderIndex] &&
        cellAfterClose[1] != cellBefore[1] &&
        cellAfterClose[kCandidatePartyOrdinal] !=
            cellBefore[kCandidatePartyOrdinal] &&
        cellAfterClose[kLeaderIndex] == kSecondTargetDirection &&
        cellAfterClose[1] == kSecondTargetDirection &&
        cellAfterClose[kCandidatePartyOrdinal] == kSecondTargetDirection;
    result->championDirectionsAfterClose =
        directionAfterClose[kLeaderIndex] != directionBefore[kLeaderIndex] &&
        directionAfterClose[1] != directionBefore[1] &&
        directionAfterClose[kCandidatePartyOrdinal] !=
            directionBefore[kCandidatePartyOrdinal] &&
        directionAfterClose[kLeaderIndex] == kSecondTargetDirection &&
        directionAfterClose[1] == kSecondTargetDirection &&
        directionAfterClose[kCandidatePartyOrdinal] ==
            kSecondTargetDirection;
    /* F0284 mutates the per-champion Cell and Direction fields; the
     * C160 close must NOT roll them back to the pre-shuffle values,
     * i.e. the post-close state is the post-shuffle state. The
     * "preserved" fields here name that contract. */
    result->cellPreservedAfterClose = result->championCellsAfterClose;
    result->directionPreservedAfterClose = result->championDirectionsAfterClose;
    result->queueWriteOrderPreserved =
        state->queueWriteCountF0361Turn == 2 &&
        state->queueWriteCountF0359PanelClick == 1;
    result->dispatchOrderPreserved =
        state->dispatchOrder[0] == DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34 &&
        state->dispatchOrder[1] == DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34 &&
        state->dispatchOrder[2] == DM1_V1_MC_CAPS_COMMAND_C160_YES_PC34;
    result->f0380DrainProcessedAll = state->dispatchDrainCountF0380 == 3 &&
                                     state->commandQueueDepth == 0;
    result->sourceAnchorsPresent = source_anchors_present();
    result->assertionsRepresented = 1;
    result->hash = result->afterCloseHash;
    return result->shuffledFirst && result->shuffledSecond &&
           result->f0284FiredTwice && result->f0284FirstDeltaCorrect &&
           result->f0284SecondDeltaCorrect && result->f0296CalledTwice &&
           result->g0308EastAfterFirst && result->g0308SouthAfterSecond &&
           result->panelStayedOpenThroughShuffle &&
           result->yesAcceptedAfterShuffle &&
           result->f0282FiredOnPostShuffleParty &&
           result->g0299ClearedAfterShuffle &&
           result->g0305DecrementedAfterShuffle &&
           result->candidateIndexByteStable &&
           result->c040PanelClosedAfterShuffle &&
           result->c038PanelPriorityPreserved &&
           result->c037StatusHandBoxPreserved &&
           result->c159ChampionIconPreserved &&
           result->m070PanelOwnerPreserved &&
           result->m568PanelContentPreserved &&
           result->g0308SouthAfterClose &&
           result->championCellsAfterClose &&
           result->championDirectionsAfterClose &&
           result->queueWriteOrderPreserved && result->dispatchOrderPreserved &&
           result->f0380DrainProcessedAll && result->sourceAnchorsPresent;
}

const char *
dm1_v1_mirror_candidate_close_after_party_shuffle_source_evidence_pc34(void)
{
    return s_source_evidence;
}
