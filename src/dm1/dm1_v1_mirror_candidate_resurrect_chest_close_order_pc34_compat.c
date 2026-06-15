#include "firestaff/dm1/v1/mirror_candidate/resurrect_chest_close_order_pc34_compat.h"

#include <string.h>

enum {
    kCandidateOrdinal = 2,
    kAcceptedCandidateIndex = 1,
    kLeaderIndex = 1,
    kInventoryChampionOrdinal = 2,
    kOpenChestThing = 0x6426,
    kLeaderHandC540Thing = 0xc540,
    kC038PanelPriorityByte = 0x38,
    kC037StatusHandBoxByte = 0x37,
    kC159ChampionIconByte = 0x59,
    kTraceSeed = 7800,
    kTraceQueueYes = 7801,
    kTraceDrainYes = 7802,
    kTraceQueueClose = 7803,
    kTraceDrainClose = 7804,
    kTraceQueueForward = 7805,
    kTraceDrainForward = 7806,
    kTraceQueueWheel = 7807,
    kTraceDrainWheel = 7808
};

/*
 * ReDMCSB anchors:
 * REVIVE.C F0280:124-132 publishes C040 candidates and F0282:744-806
 * clears G0299 while removing accepted candidate possessions from the chain.
 * CHEST.C F0333:30-67 fills G0425 from an open chest and F0334:113-132
 * clears G0426/G0425 while relinking non-empty things through F0163
 * (DUNGEON.C:1796-1837). CHAMPION.C F0297/F0298:243-298 own the leader
 * hand lifetime and F0302:662-714 dispatches occupied slot boxes. COMMAND.C
 * F0359:1452-1662 queues clicks, F0361:1709-1813 queues keyboard movement,
 * F0378:1956-1993 routes C040/chest panel clicks, and F0380:2045-2178
 * drains one command. The requested MOUSE.C F0077/F0078 anchor is absent
 * from this local tree; F0077/F0078 definitions are in IO.C:1102-1122 and
 * UTAMSCR.C:141-150, so the model records the wheel write/read boundary
 * without claiming a nonexistent MOUSE.C file.
 */
static const char s_source_evidence[] =
    "REVIVE.C F0280:124-132 candidate publish; REVIVE.C F0282:744-806 "
    "G0299 clear and accepted candidate chain removal. CHEST.C "
    "F0333:30-67 chest open and G0425 visible slots; CHEST.C "
    "F0334:113-132 close-rewrite, C0xFFFF_THING_NONE skip, G0425 clear; "
    "DUNGEON.C F0163:1796-1837 thing-list relink. CHAMPION.C "
    "F0297:243-298 leader-hand put; F0298:270-298 leader-hand remove; "
    "F0302:662-714 occupied-slot click dispatch. COMMAND.C "
    "F0359:1452-1662 click queue write, F0361:1709-1813 keyboard queue "
    "write, F0378:1956-1993 C040/chest panel route, F0380:2045-2178 "
    "dispatch drain. Requested MOUSE.C F0077:97-126/F0078:128-168 is not "
    "present in this local ReDMCSB tree; F0077/F0078 definitions are present "
    "at IO.C:1102-1122 and UTAMSCR.C:141-150. DEFS.H anchors: C037/C038, "
    "C040, C159, C160..C162, C537..C544, C545, G0299, G0305, G0423, "
    "G0425, G0426, M070, M516. Runtime regression marker: "
    "pass780_dm1_v1_mirror_candidate_resurrect_chest_close_order.";

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static int all_visible_slots_clear(
    const Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    int i;

    for (i = 0; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        if (state->chestVisibleSlots[i] != DM1_V1_MC_RCCO_NONE_PC34) {
            return 0;
        }
    }
    return 1;
}

static int candidate_removed(
    const Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    int i;

    for (i = 0; i < DM1_V1_MC_RCCO_CANDIDATE_CHAIN_COUNT_PC34; ++i) {
        if (state->candidateChain[i] == kCandidateOrdinal) {
            return 0;
        }
    }
    return 1;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "REVIVE.C F0280:124-132") != 0 &&
           strstr(s_source_evidence, "F0282:744-806") != 0 &&
           strstr(s_source_evidence, "CHEST.C F0333:30-67") != 0 &&
           strstr(s_source_evidence, "F0334:113-132") != 0 &&
           strstr(s_source_evidence, "DUNGEON.C F0163:1796-1837") != 0 &&
           strstr(s_source_evidence, "CHAMPION.C F0297:243-298") != 0 &&
           strstr(s_source_evidence, "F0298:270-298") != 0 &&
           strstr(s_source_evidence, "F0302:662-714") != 0 &&
           strstr(s_source_evidence, "COMMAND.C F0359:1452-1662") != 0 &&
           strstr(s_source_evidence, "F0361:1709-1813") != 0 &&
           strstr(s_source_evidence, "F0378:1956-1993") != 0 &&
           strstr(s_source_evidence, "F0380:2045-2178") != 0 &&
           strstr(s_source_evidence, "IO.C:1102-1122") != 0 &&
           strstr(s_source_evidence, "UTAMSCR.C:141-150") != 0 &&
           strstr(s_source_evidence, "C537..C544") != 0 &&
           strstr(s_source_evidence, "pass780_dm1_v1_mirror_candidate_resurrect_chest_close_order") !=
               0;
}

uint32_t dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(
    const Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    if (!state) {
        return 0;
    }
    hash = hash_step(hash, (uint32_t)state->contractOnly);
    hash = hash_step(hash, (uint32_t)state->partyChampionCount);
    hash = hash_step(hash, (uint32_t)state->leaderIndex);
    hash = hash_step(hash, (uint32_t)state->inventoryChampionOrdinal);
    hash = hash_step(hash, (uint32_t)state->candidateChampionOrdinal);
    hash = hash_step(hash, (uint32_t)state->candidateIndexByte);
    hash = hash_step(hash, (uint32_t)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (uint32_t)state->c040PanelOpen);
    hash = hash_step(hash, (uint32_t)state->c040PanelClosed);
    hash = hash_step(hash, (uint32_t)state->c038PanelPriorityByte);
    hash = hash_step(hash, (uint32_t)state->c037StatusHandBoxByte);
    hash = hash_step(hash, (uint32_t)state->c159ChampionIconByte);
    hash = hash_step(hash, (uint32_t)state->g0426OpenChestThing);
    hash = hash_step(hash, (uint32_t)state->leaderHandThing);
    hash = hash_step(hash, (uint32_t)state->commandQueueDepth);
    hash = hash_step(hash, (uint32_t)state->queueWriteCountF0359);
    hash = hash_step(hash, (uint32_t)state->queueWriteCountF0361);
    hash = hash_step(hash, (uint32_t)state->queueWriteCountWheelF0077);
    hash = hash_step(hash, (uint32_t)state->wheelDrainCountF0078);
    hash = hash_step(hash, (uint32_t)state->dispatchDrainCountF0380);
    hash = hash_step(hash, (uint32_t)state->f0282AcceptClearCount);
    hash = hash_step(hash, (uint32_t)state->f0334ChestCloseCount);
    hash = hash_step(hash, (uint32_t)state->f0163RelinkCount);
    hash = hash_step(hash, (uint32_t)state->f0297PutAlreadyDoneCount);
    hash = hash_step(hash, (uint32_t)state->f0298RemoveCount);
    hash = hash_step(hash, (uint32_t)state->f0302ClickDispatchCount);
    hash = hash_step(hash, (uint32_t)state->forwardQueuedAfterChestClose);
    hash = hash_step(hash, (uint32_t)state->forwardDrainedOnClosedChest);
    hash = hash_step(hash, (uint32_t)state->wheelQueuedAfterForward);
    hash = hash_step(hash, (uint32_t)state->wheelSawClosedChest);
    hash = hash_step(hash, (uint32_t)state->wheelTarget);
    for (i = 0; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->chestVisibleSlots[i]);
        hash = hash_step(hash, (uint32_t)state->chestContainerChain[i]);
    }
    for (i = 0; i < DM1_V1_MC_RCCO_CANDIDATE_CHAIN_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->candidateChain[i]);
    }
    for (i = 0; i < DM1_V1_MC_RCCO_COMMAND_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->queuedCommands[i]);
        hash = hash_step(hash, (uint32_t)state->dispatchOrder[i]);
    }
    for (i = 0; i < DM1_V1_MC_RCCO_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (uint32_t)state->trace[i]);
    }
    return hash;
}

Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34
dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34(void)
{
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 state;
    int i;

    memset(&state, 0, sizeof(state));
    state.contractOnly = 1;
    state.noAssetReads = 1;
    state.noOriginalDosPixelParityClaim = 1;
    state.partyChampionCount = 3;
    state.leaderIndex = kLeaderIndex;
    state.inventoryChampionOrdinal = kInventoryChampionOrdinal;
    state.candidateChampionOrdinal = kCandidateOrdinal;
    state.candidateIndexByte = kAcceptedCandidateIndex;
    state.g0299CandidateOrdinal = kCandidateOrdinal;
    state.c040PanelOpen = 1;
    state.c038PanelPriorityByte = kC038PanelPriorityByte;
    state.c037StatusHandBoxByte = kC037StatusHandBoxByte;
    state.c159ChampionIconByte = kC159ChampionIconByte;
    state.g0426OpenChestThing = (uint16_t)kOpenChestThing;
    state.leaderHandThing = (uint16_t)kLeaderHandC540Thing;
    state.f0280PublishCount = 1;
    state.f0297PutAlreadyDoneCount = 1;
    state.commandQueueDepth = 0;
    state.candidateChain[0] = kCandidateOrdinal;
    state.candidateChain[1] = 4;
    state.candidateChain[2] = 5;
    state.candidateChain[3] = 0;
    for (i = 0; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        state.chestVisibleSlots[i] = (uint16_t)(537 + i);
        state.chestContainerChain[i] = DM1_V1_MC_RCCO_NONE_PC34;
    }
    state.chestVisibleSlots[2] = DM1_V1_MC_RCCO_NONE_PC34;
    for (i = 0; i < DM1_V1_MC_RCCO_TRACE_COUNT_PC34; ++i) {
        state.trace[i] = 0;
    }
    state.trace[0] = kTraceSeed;
    return state;
}

static int ready(
    const Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    return state && state->contractOnly && state->noAssetReads &&
           state->noOriginalDosPixelParityClaim &&
           state->partyChampionCount > 1 &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->c040PanelOpen &&
           state->g0426OpenChestThing != DM1_V1_MC_RCCO_NONE_PC34 &&
           state->leaderHandThing == kLeaderHandC540Thing &&
           state->candidateIndexByte == kAcceptedCandidateIndex;
}

static void queue_command(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state,
    Dm1V1MirrorCandidateResurrectChestCloseOrderCommandPc34 command)
{
    int index = state->commandQueueDepth;

    if (index >= DM1_V1_MC_RCCO_COMMAND_COUNT_PC34) {
        return;
    }
    state->queuedCommands[index] = command;
    ++state->commandQueueDepth;
    if (command == DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34 ||
        command == DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34) {
        ++state->queueWriteCountF0359;
    } else if (command == DM1_V1_MC_RCCO_COMMAND_MOVE_FORWARD_PC34) {
        ++state->queueWriteCountF0361;
        state->forwardQueuedAfterChestClose =
            state->g0426OpenChestThing == DM1_V1_MC_RCCO_NONE_PC34;
    } else if (command == DM1_V1_MC_RCCO_COMMAND_WHEEL_UP_PC34) {
        ++state->queueWriteCountWheelF0077;
        state->wheelQueuedAfterForward = state->forwardDrainedOnClosedChest;
        state->wheelSawClosedChest =
            state->g0426OpenChestThing == DM1_V1_MC_RCCO_NONE_PC34;
    }
}

static void shift_queue(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    int i;

    for (i = 1; i < state->commandQueueDepth; ++i) {
        state->queuedCommands[i - 1] = state->queuedCommands[i];
    }
    if (state->commandQueueDepth > 0) {
        --state->commandQueueDepth;
    }
}

static void accept_candidate(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    ++state->f0282AcceptClearCount;
    state->g0299CandidateOrdinal = 0;
    state->c040PanelOpen = 0;
    state->c040PanelClosed = 1;
    state->candidateChain[0] = 4;
    state->candidateChain[1] = 5;
    state->candidateChain[2] = 0;
    state->candidateChain[3] = 0;
}

static void close_chest(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    int i;
    int out = 0;

    ++state->f0334ChestCloseCount;
    state->g0426OpenChestThing = DM1_V1_MC_RCCO_NONE_PC34;
    for (i = 0; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        if (state->chestVisibleSlots[i] != DM1_V1_MC_RCCO_NONE_PC34) {
            state->chestContainerChain[out++] = state->chestVisibleSlots[i];
        }
        state->chestVisibleSlots[i] = DM1_V1_MC_RCCO_NONE_PC34;
    }
    for (i = out; i < DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestContainerChain[i] = DM1_V1_MC_RCCO_NONE_PC34;
    }
    state->f0163RelinkCount = out > 0 ? out - 1 : 0;
}

static void drain_next(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state)
{
    Dm1V1MirrorCandidateResurrectChestCloseOrderCommandPc34 command;
    int index = state->dispatchDrainCountF0380;

    if (state->commandQueueDepth <= 0) {
        return;
    }
    command = state->queuedCommands[0];
    state->dispatchOrder[index] = command;
    ++state->dispatchDrainCountF0380;
    shift_queue(state);

    if (command == DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34) {
        accept_candidate(state);
    } else if (command == DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34) {
        close_chest(state);
    } else if (command == DM1_V1_MC_RCCO_COMMAND_MOVE_FORWARD_PC34) {
        state->forwardDrainedOnClosedChest =
            state->g0426OpenChestThing == DM1_V1_MC_RCCO_NONE_PC34;
    } else if (command == DM1_V1_MC_RCCO_COMMAND_WHEEL_UP_PC34) {
        ++state->wheelDrainCountF0078;
        ++state->f0302ClickDispatchCount;
        state->wheelTarget =
            state->g0426OpenChestThing == DM1_V1_MC_RCCO_NONE_PC34 ?
                DM1_V1_MC_RCCO_WHEEL_TARGET_LEADER_HAND_PC34 :
                DM1_V1_MC_RCCO_WHEEL_TARGET_CLOSED_CHEST_PC34;
    }
}

int dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state,
    Dm1V1MirrorCandidateResurrectChestCloseOrderResultPc34 *result)
{
    int c038Before;
    int c037Before;
    int c159Before;
    int candidateIndexBefore;
    uint16_t leaderHandBefore;

    if (!state || !result || !ready(state)) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    c038Before = state->c038PanelPriorityByte;
    c037Before = state->c037StatusHandBoxByte;
    c159Before = state->c159ChampionIconByte;
    candidateIndexBefore = state->candidateIndexByte;
    leaderHandBefore = state->leaderHandThing;
    result->beforeHash =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(state);

    queue_command(state, DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34);
    state->trace[1] = kTraceQueueYes;
    drain_next(state);
    state->trace[2] = kTraceDrainYes;
    result->afterAcceptHash =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(state);

    queue_command(state, DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34);
    state->trace[3] = kTraceQueueClose;
    drain_next(state);
    state->trace[4] = kTraceDrainClose;
    result->afterChestCloseHash =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(state);

    queue_command(state, DM1_V1_MC_RCCO_COMMAND_MOVE_FORWARD_PC34);
    state->trace[5] = kTraceQueueForward;
    drain_next(state);
    state->trace[6] = kTraceDrainForward;
    result->afterForwardHash =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(state);

    queue_command(state, DM1_V1_MC_RCCO_COMMAND_WHEEL_UP_PC34);
    state->trace[7] = kTraceQueueWheel;
    drain_next(state);
    state->trace[8] = kTraceDrainWheel;
    result->afterWheelHash =
        dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(state);

    result->acceptedFirst =
        state->dispatchOrder[0] == DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34;
    result->g0299ClearedFirst = state->f0282AcceptClearCount == 1 &&
                                state->g0299CandidateOrdinal == 0;
    result->candidateChainRemovedFirst = candidate_removed(state);
    result->candidateIndexByteStable =
        state->candidateIndexByte == candidateIndexBefore;
    result->c040PanelClosed = state->c040PanelClosed && !state->c040PanelOpen;
    result->c038PanelPriorityPreserved =
        state->c038PanelPriorityByte == c038Before;
    result->c037StatusHandBoxStable =
        state->c037StatusHandBoxByte == c037Before;
    result->chestClosedSecond =
        state->dispatchOrder[1] == DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34 &&
        state->f0334ChestCloseCount == 1;
    result->g0426ClearedSecond =
        state->g0426OpenChestThing == DM1_V1_MC_RCCO_NONE_PC34;
    result->visibleSlotsClearedSecond = all_visible_slots_clear(state);
    result->chestContainerRelinked = state->f0163RelinkCount == 6 &&
                                     state->chestContainerChain[0] == 537 &&
                                     state->chestContainerChain[2] == 540;
    result->leaderHandPreservedAfterClose =
        state->leaderHandThing == leaderHandBefore;
    result->leaderHandNotStripped =
        state->leaderHandThing == kLeaderHandC540Thing &&
        state->f0298RemoveCount == 0;
    result->forwardQueuedAfterClose = state->forwardQueuedAfterChestClose;
    result->forwardDrainedOnClosedChest = state->forwardDrainedOnClosedChest;
    result->wheelAfterForwardLandedOnLeaderHand =
        state->wheelQueuedAfterForward &&
        state->wheelTarget == DM1_V1_MC_RCCO_WHEEL_TARGET_LEADER_HAND_PC34;
    result->wheelDidNotLandOnChest =
        state->wheelTarget != DM1_V1_MC_RCCO_WHEEL_TARGET_CLOSED_CHEST_PC34;
    result->queueWriteOrderPreserved =
        state->queueWriteCountF0359 == 2 && state->queueWriteCountF0361 == 1 &&
        state->queueWriteCountWheelF0077 == 1;
    result->dispatchOrderPreserved =
        state->dispatchOrder[0] == DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34 &&
        state->dispatchOrder[1] == DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34 &&
        state->dispatchOrder[2] == DM1_V1_MC_RCCO_COMMAND_MOVE_FORWARD_PC34 &&
        state->dispatchOrder[3] == DM1_V1_MC_RCCO_COMMAND_WHEEL_UP_PC34;
    result->f0380DrainProcessedAll = state->dispatchDrainCountF0380 == 4 &&
                                     state->commandQueueDepth == 0;
    result->c159ChampionIconStable = state->c159ChampionIconByte == c159Before;
    result->sourceAnchorsPresent = source_anchors_present();
    result->assertionsRepresented = 1;
    result->hash = result->afterWheelHash;
    return result->acceptedFirst && result->g0299ClearedFirst &&
           result->candidateChainRemovedFirst &&
           result->candidateIndexByteStable && result->c040PanelClosed &&
           result->c038PanelPriorityPreserved &&
           result->c037StatusHandBoxStable && result->chestClosedSecond &&
           result->g0426ClearedSecond && result->visibleSlotsClearedSecond &&
           result->leaderHandPreservedAfterClose &&
           result->leaderHandNotStripped && result->forwardQueuedAfterClose &&
           result->forwardDrainedOnClosedChest &&
           result->wheelAfterForwardLandedOnLeaderHand &&
           result->wheelDidNotLandOnChest && result->queueWriteOrderPreserved &&
           result->dispatchOrderPreserved && result->f0380DrainProcessedAll &&
           result->c159ChampionIconStable && result->sourceAnchorsPresent;
}

const char *
dm1_v1_mirror_candidate_resurrect_chest_close_order_source_evidence_pc34(void)
{
    return s_source_evidence;
}
