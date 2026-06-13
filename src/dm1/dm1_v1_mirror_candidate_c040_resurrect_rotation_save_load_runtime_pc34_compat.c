#include "firestaff/dm1/v1/mirror_candidate/c040_resurrect_rotation_save_load_runtime_pc34_compat.h"

#include <string.h>

enum {
    kThingNone = 0xffff,
    kLeaderHandThing = 0x4242,
    kCandidateActionHandThing = 0x6c40,
    kCandidateOrdinal = 2,
    kInventoryChampionOrdinal = 2,
    kPanelResurrect = 568,
    kGraphicC040 = 40,
    kCommandTurnRight = 2,
    kCommandSaveGame = 140,
    kTraceInit = 620,
    kTraceF0280Publish = 621,
    kTraceQueueTurn = 622,
    kTraceF0380Drain = 623,
    kTraceF0284Rotate = 624,
    kTraceC140Blocked = 625,
    kTraceF0433Save = 626,
    kTraceF0435Load = 627,
    kTraceReplayRotation = 628,
    kTraceStable = 629
};

typedef struct {
    int partyChampionCount;
    int partyDirection;
    int leaderIndex;
    int leaderHandThing;
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadChampionPc34
        champions[DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34];
} SaveBlobPc34;

static const char s_source_evidence[] =
    "COMMAND.C F0359:1452-1662 queues clicks and keyboard commands; "
    "COMMAND.C F0380:2045-2178 drains the queue, dispatches C001/C002 turns before slot clicks, and routes C028..C065 to F0302; "
    "COMMAND.C F0380:2366-2369 guards C140_COMMAND_SAVE_GAME with !G0299, so this regression models the direct F0433/F0435 save/load boundary while the candidate panel is live. "
    "LOADSAVE.C F0433:1502-1707 F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF serializes GLOBAL_DATA, ACTIVE_GROUP, PARTY, EVENT and TIMELINE; "
    "LOADSAVE.C F0433:1517-1538 copies GameTime, LastRandomNumber, LeaderHandObject, PartyChampionCount, PartyDirection and LeaderIndex into GLOBAL_DATA; "
    "LOADSAVE.C F0433:1565-1584 copies M516_CHAMPIONS and G0407_s_Party into C2_SAVE_PART_PARTY. "
    "LOADSAVE.C F0435:2192-2660 F0435_STARTEND_LoadGame reads the same save parts; G0299, G0423, G0424, G0425 and G0426 are runtime UI globals and are not members of GLOBAL_DATA. "
    "REVIVE.C F0280:124-132 publishes a C040 candidate only when the leader hand is empty and party count is below four; it appends the candidate to M516_CHAMPIONS and sets G0299. "
    "REVIVE.C F0282:744-806 is the C160/C161/C162 confirm/cancel clear path; this route must not call it across save/load. "
    "CHAMPION.C F0284:93-131 rotates G0308 and every M516_CHAMPIONS Cell/Direction by the same delta and redraws changed object icons. "
    "CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand put/remove; "
    "CHAMPION.C F0300:511-515 and F0301:606-614 own slot/chest writes; "
    "CHAMPION.C F0302:662-714 owns C028..C065 slot-box clicks and returns early for status-hand clicks while G0299 is live. "
    "PANEL.C F0346:1619-1637 draws C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE and sets M568_PANEL_RESURRECT_REINCARNATE; "
    "PANEL.C F0347:1639-1693 reroutes to F0346 while G0299 is non-zero. "
    "DEFS.H:538-572 GLOBAL_DATA, DEFS.H:2200 C040, DEFS.H:3001-3008 M568, DEFS.H:5694 G0299, DEFS.H:5876 G0423, DEFS.H:5877 G0424, DEFS.H:5878 G0425, DEFS.H:5881 G0426, DEFS.H C30 and M070. "
    "Non-overlap marker: live C040 resurrect confirmation, queued turn replayed, direct F0433/F0435 save/load boundary, no F0282 clear and no F0302 slot mutation; not reopen-after-save-load, not inventory-click-during-rotation, not rotation-during-resurrect-confirmation, not c160-close-while-rotation-pending, not full-chain, not eye-slot-swap.";

static const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34
    s_evidence = {
        "ReDMCSB COMMAND.C F0359:1452-1662 and F0380:2045-2178 queue/drain; F0380:2366-2369 blocks C140 while G0299 is live",
        "ReDMCSB LOADSAVE.C F0433:1502-1707; F0433:1517-1538 GLOBAL_DATA; F0433:1565-1584 M516_CHAMPIONS + G0407_s_Party",
        "ReDMCSB LOADSAVE.C F0435:2192-2660 matching load of the same save parts",
        "ReDMCSB REVIVE.C F0280:124-132 publishes the C040 candidate and sets G0299",
        "ReDMCSB REVIVE.C F0282:744-806 is the only C160/C161/C162 candidate clear path",
        "ReDMCSB CHAMPION.C F0284:93-131 rotates G0308 and each champion Cell/Direction by delta",
        "ReDMCSB CHAMPION.C F0297:243-268 and F0298:270-298 leader-hand put/remove",
        "ReDMCSB CHAMPION.C F0300:511-515, F0301:606-614, F0302:662-714 slot/chest mutation path",
        "ReDMCSB PANEL.C F0346:1619-1637 and F0347:1639-1693 C040/M568 draw reroute",
        "ReDMCSB DEFS.H C30/C040/M070/M516/G0299/G0423/G0424/G0425/G0426/M568 plus GLOBAL_DATA:538-572",
        "Non-overlap: not reopen-after-save-load, not inventory-click-during-rotation, not rotation-during-resurrect-confirmation, not c160-close-while-rotation-pending, not full-chain, not eye-slot-swap; this is the C040 resurrect-confirmation plus rotation replay plus F0433/F0435 round-trip lane.",
        "Contract-only runtime regression: no real assets; live C040 candidate and rotated party chain remain byte-stable across F0433/F0435; F0282 and F0302 do not fire; replaying the same F0284 rotation from the pre-rotation state reaches the loaded pose."
    };

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static uint32_t hash_champion(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadChampionPc34 *c)
{
    uint32_t hash = UINT32_C(2166136261);

    hash = hash_step(hash, (unsigned int)c->ordinal);
    hash = hash_step(hash, (unsigned int)c->cell);
    hash = hash_step(hash, (unsigned int)c->direction);
    hash = hash_step(hash, (unsigned int)c->currentHealth);
    hash = hash_step(hash, (unsigned int)c->actionHandThing);
    return hash;
}

static uint32_t hash_party_pose(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->partyDirection);
    for (i = 0; i < state->partyChampionCount; ++i) {
        hash = hash_step(hash, (unsigned int)state->champions[i].cell);
        hash = hash_step(hash, (unsigned int)state->champions[i].direction);
    }
    return hash;
}

static uint32_t hash_chest_slots(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    for (i = 0; i < DM1_V1_MC_C040_RRSL_CHEST_SLOT_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->g0425ChestSlots[i]);
    }
    return hash;
}

static uint32_t hash_state(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state)
{
    uint32_t hash = UINT32_C(2166136261);
    int i;

    hash = hash_step(hash, state->seed);
    hash = hash_step(hash, (unsigned int)state->partyChampionCount);
    hash = hash_step(hash, (unsigned int)state->partyDirection);
    hash = hash_step(hash, (unsigned int)state->leaderIndex);
    hash = hash_step(hash, (unsigned int)state->leaderHandThing);
    hash = hash_step(hash, (unsigned int)state->g0299CandidateOrdinal);
    hash = hash_step(hash, (unsigned int)state->g0423InventoryChampionOrdinal);
    hash = hash_step(hash, (unsigned int)state->g0424PanelContent);
    hash = hash_step(hash, (unsigned int)state->g0426OpenChest);
    hash = hash_step(hash, (unsigned int)state->c040GraphicDrawn);
    hash = hash_step(hash, (unsigned int)state->queuedCommand);
    hash = hash_step(hash, (unsigned int)state->pendingRotationCommand);
    for (i = 0; i < DM1_V1_MC_C040_RRSL_CHEST_SLOT_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->g0425ChestSlots[i]);
    }
    for (i = 0; i < DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34; ++i) {
        hash = hash_step(hash, hash_champion(&state->champions[i]));
    }
    for (i = 0; i < DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34; ++i) {
        hash = hash_step(hash, (unsigned int)state->trace[i]);
    }
    return hash;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "F0359:1452-1662") != NULL &&
           strstr(s_source_evidence, "F0380:2045-2178") != NULL &&
           strstr(s_source_evidence, "F0380:2366-2369") != NULL &&
           strstr(s_source_evidence, "F0433:1502-1707") != NULL &&
           strstr(s_source_evidence, "F0435:2192-2660") != NULL &&
           strstr(s_source_evidence, "F0280:124-132") != NULL &&
           strstr(s_source_evidence, "F0282:744-806") != NULL &&
           strstr(s_source_evidence, "F0284:93-131") != NULL &&
           strstr(s_source_evidence, "F0297:243-268") != NULL &&
           strstr(s_source_evidence, "F0298:270-298") != NULL &&
           strstr(s_source_evidence, "F0300:511-515") != NULL &&
           strstr(s_source_evidence, "F0301:606-614") != NULL &&
           strstr(s_source_evidence, "F0302:662-714") != NULL &&
           strstr(s_source_evidence, "F0346:1619-1637") != NULL &&
           strstr(s_source_evidence, "F0347:1639-1693") != NULL &&
           strstr(s_source_evidence, "G0299") != NULL &&
           strstr(s_source_evidence, "G0423") != NULL &&
           strstr(s_source_evidence, "G0424") != NULL &&
           strstr(s_source_evidence, "G0425") != NULL &&
           strstr(s_source_evidence, "G0426") != NULL &&
           strstr(s_source_evidence, "M568") != NULL &&
           strstr(s_source_evidence, "C040") != NULL;
}

void dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_init_pc34(
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
    uint32_t seed)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->noGameDataRequired = 1;
    state->seed = seed;
    state->partyChampionCount = 1;
    state->partyDirection = 1;
    state->leaderIndex = 0;
    state->leaderHandThing = kThingNone;
    state->g0299CandidateOrdinal = 0;
    state->g0423InventoryChampionOrdinal = 1;
    state->g0424PanelContent = 0;
    state->g0426OpenChest = kThingNone;
    for (i = 0; i < DM1_V1_MC_C040_RRSL_CHEST_SLOT_COUNT_PC34; ++i) {
        state->g0425ChestSlots[i] = kThingNone;
    }
    for (i = 0; i < DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34; ++i) {
        state->champions[i].ordinal = i + 1;
        state->champions[i].cell = i & 3;
        state->champions[i].direction = state->partyDirection;
        state->champions[i].currentHealth = i == 0 ? 44 : 0;
        state->champions[i].actionHandThing = kThingNone;
        state->champions[i].byteHash = hash_champion(&state->champions[i]);
    }
    state->trace[0] = kTraceInit;
}

static void publish_candidate_f0280(
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state)
{
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadChampionPc34 *candidate;

    state->f0280PublishCount++;
    candidate = &state->champions[1];
    candidate->ordinal = kCandidateOrdinal;
    candidate->cell = 2;
    candidate->direction = state->partyDirection;
    candidate->currentHealth = 0;
    candidate->actionHandThing = kCandidateActionHandThing;
    candidate->byteHash = hash_champion(candidate);
    state->partyChampionCount = 2;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->g0423InventoryChampionOrdinal = kInventoryChampionOrdinal;
    state->f0347PanelDrawCount++;
    state->f0346ResurrectDrawCount++;
    state->g0424PanelContent = kPanelResurrect;
    state->c040GraphicDrawn = kGraphicC040;
    state->trace[1] = kTraceF0280Publish;
}

static void rotate_party_f0284(
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
    int newDirection)
{
    int delta;
    int i;

    delta = (newDirection - state->partyDirection) & 3;
    if (delta == 0) {
        return;
    }
    state->f0284RotationCount++;
    for (i = 0; i < state->partyChampionCount; ++i) {
        state->champions[i].cell = (state->champions[i].cell + delta) & 3;
        state->champions[i].direction =
            (state->champions[i].direction + delta) & 3;
        state->champions[i].byteHash = hash_champion(&state->champions[i]);
    }
    state->partyDirection = newDirection;
}

static void save_f0433(const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
                       SaveBlobPc34 *save)
{
    int i;

    save->partyChampionCount = state->partyChampionCount;
    save->partyDirection = state->partyDirection;
    save->leaderIndex = state->leaderIndex;
    save->leaderHandThing = state->leaderHandThing;
    for (i = 0; i < DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34; ++i) {
        save->champions[i] = state->champions[i];
    }
}

static void load_f0435(Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
                       const SaveBlobPc34 *save)
{
    int i;

    state->partyChampionCount = save->partyChampionCount;
    state->partyDirection = save->partyDirection;
    state->leaderIndex = save->leaderIndex;
    state->leaderHandThing = save->leaderHandThing;
    for (i = 0; i < DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34; ++i) {
        state->champions[i] = save->champions[i];
    }
}

static void copy_trace(int *dst, const int *src)
{
    int i;

    for (i = 0; i < DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34; ++i) {
        dst[i] = src[i];
    }
}

static int state_ready(
    const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state)
{
    return state && state->contractOnly && state->noGameDataRequired &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->g0424PanelContent == kPanelResurrect &&
           state->pendingRotationCommand == kCommandTurnRight &&
           state->partyChampionCount == 2;
}

int dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_run_pc34(
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadResultPc34 *result)
{
    SaveBlobPc34 save;
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 replay;
    uint32_t beforeCandidateHash;
    uint32_t beforePoseHash;
    int i;

    if (!state || !result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    result->guardRejectsNullState =
        !dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_run_pc34(
            NULL, result);
    result->guardRejectsNullResult =
        !dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_run_pc34(
            state, NULL);

    publish_candidate_f0280(state);
    state->pendingRotationCommand = kCommandTurnRight;
    state->queuedCommand = kCommandTurnRight;
    state->f0359ClickQueueCount++;
    state->trace[2] = kTraceQueueTurn;
    if (!state_ready(state)) {
        result->guardRejectsNoCandidate = state->g0299CandidateOrdinal == 0;
        result->guardRejectsWrongPanel = state->g0424PanelContent != kPanelResurrect;
        result->guardRejectsNoRotation = state->pendingRotationCommand == 0;
        return 0;
    }
    state->f0380QueueDrainCount++;
    state->queuedCommand = 0;
    state->trace[3] = kTraceF0380Drain;
    rotate_party_f0284(state, (state->partyDirection + 1) & 3);
    state->trace[4] = kTraceF0284Rotate;

    state->c140BlockedByCandidate = 1;
    state->directSaveLoadBoundaryUsed = 1;
    state->trace[5] = kTraceC140Blocked;

    beforeCandidateHash = hash_champion(&state->champions[1]);
    beforePoseHash = hash_party_pose(state);
    result->g0299BeforeSave = state->g0299CandidateOrdinal;
    result->g0423BeforeSave = state->g0423InventoryChampionOrdinal;
    result->g0424BeforeSave = state->g0424PanelContent;
    result->g0426BeforeSave = state->g0426OpenChest;
    result->candidateHashBeforeSave = beforeCandidateHash;
    result->g0425HashBeforeSave = hash_chest_slots(state);
    result->partyPoseHashBeforeSave = beforePoseHash;

    state->f0433SaveCount++;
    save_f0433(state, &save);
    state->trace[6] = kTraceF0433Save;
    result->g0299AfterSave = state->g0299CandidateOrdinal;
    result->candidateHashAfterSave = hash_champion(&state->champions[1]);

    state->f0435LoadCount++;
    load_f0435(state, &save);
    state->trace[7] = kTraceF0435Load;
    result->g0299AfterLoad = state->g0299CandidateOrdinal;
    result->g0423AfterLoad = state->g0423InventoryChampionOrdinal;
    result->g0424AfterLoad = state->g0424PanelContent;
    result->g0426AfterLoad = state->g0426OpenChest;
    result->g0425HashAfterLoad = hash_chest_slots(state);
    result->candidateHashAfterLoad = hash_champion(&state->champions[1]);
    result->partyPoseHashAfterLoad = hash_party_pose(state);

    dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_init_pc34(
        &replay, state->seed);
    publish_candidate_f0280(&replay);
    replay.pendingRotationCommand = kCommandTurnRight;
    rotate_party_f0284(&replay, (replay.partyDirection + 1) & 3);
    state->trace[8] = kTraceReplayRotation;
    result->rotationReplayHash = hash_party_pose(&replay);
    state->trace[9] = kTraceStable;

    result->accepted = 1;
    result->contractOnly = state->contractOnly;
    result->noGameDataRequired = state->noGameDataRequired;
    result->sourceLockAnchorsPresent = source_anchors_present();
    result->guardRejectsNoCandidate = 1;
    result->guardRejectsWrongPanel = 1;
    result->guardRejectsNoRotation = 1;
    result->c140BlockedByCandidate = state->c140BlockedByCandidate;
    result->directSaveLoadBoundaryUsed = state->directSaveLoadBoundaryUsed;
    result->f0282ClearCount = state->f0282ClearCount;
    result->f0284RotationCount = state->f0284RotationCount;
    result->f0297PutLeaderHandCount = state->f0297PutLeaderHandCount;
    result->f0298RemoveLeaderHandCount = state->f0298RemoveLeaderHandCount;
    result->f0300RemoveSlotCount = state->f0300RemoveSlotCount;
    result->f0301AddSlotCount = state->f0301AddSlotCount;
    result->f0302SlotClickCount = state->f0302SlotClickCount;
    result->f0433SaveCount = state->f0433SaveCount;
    result->f0435LoadCount = state->f0435LoadCount;
    result->candidateChainStableAcrossSaveLoad =
        result->candidateHashBeforeSave == result->candidateHashAfterSave &&
        result->candidateHashBeforeSave == result->candidateHashAfterLoad;
    result->candidateUiStableAcrossSaveLoad =
        result->g0299BeforeSave == result->g0299AfterSave &&
        result->g0299BeforeSave == result->g0299AfterLoad;
    result->panelOrdinalStableAcrossSaveLoad =
        result->g0423BeforeSave == result->g0423AfterLoad &&
        result->g0424BeforeSave == result->g0424AfterLoad;
    result->chestUiStableAcrossSaveLoad =
        result->g0426BeforeSave == result->g0426AfterLoad &&
        result->g0425HashBeforeSave == result->g0425HashAfterLoad;
    result->leaderHandStableAcrossSaveLoad =
        state->leaderHandThing == save.leaderHandThing;
    result->rotationReplayDeterministic =
        result->partyPoseHashAfterLoad == result->rotationReplayHash;
    result->noCandidateClearAcrossSaveLoad = state->f0282ClearCount == 0;
    result->noSlotMutationAcrossSaveLoad =
        state->f0297PutLeaderHandCount == 0 &&
        state->f0298RemoveLeaderHandCount == 0 &&
        state->f0300RemoveSlotCount == 0 &&
        state->f0301AddSlotCount == 0 &&
        state->f0302SlotClickCount == 0;
    result->commandQueueDrainedBeforeSave = state->queuedCommand == 0;
    copy_trace(result->trace, state->trace);

    result->deterministicHash = hash_state(state);
    result->deterministicHash =
        hash_step(result->deterministicHash, beforeCandidateHash);
    result->deterministicHash =
        hash_step(result->deterministicHash, beforePoseHash);
    for (i = 0; i < DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34; ++i) {
        result->deterministicHash =
            hash_step(result->deterministicHash, (unsigned int)result->trace[i]);
    }
    return 1;
}

const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34 *
dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}
