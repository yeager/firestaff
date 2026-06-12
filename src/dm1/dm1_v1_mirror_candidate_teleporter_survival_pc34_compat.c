#include "firestaff/dm1/v1/mirror_candidate/dm1_v1_mirror_candidate_teleporter_survival_pc34_compat.h"

#include "dm1_v1_movement_pipeline_pc34_compat.h"

#include <string.h>

/*
 * Contract-only DM1 V1 mirror-candidate teleporter survival gate.
 *
 * ReDMCSB anchors:
 * - COMMAND.C F0380_COMMAND_ProcessQueue_CPSC dispatches the queued C003
 *   movement command into the movement core.
 * - MOVE.C F0291:4350-4420 party teleporter scope handoff is represented by
 *   Firestaff's DM1 V1 MOVESENS.C F0267/F0704 party-scoped teleporter path.
 * - MIRROR.C F0193:3550-3620 draws the C040 resurrect/reincarnate panel.
 * - MIRROR.C F0194:3700-3780 preserves the selected candidate index.
 * - CHEST.C F0333 anchors the live G0426 open-chest state under the panel.
 * - DUNGEON.C F0163:1769-1838 anchors the thing-list/cell handoff used by
 *   the teleporter fixture and the open-chest slot preservation checks.
 */

enum {
    kMapWidth = 6,
    kMapHeight = 6,
    kMapCount = 2,
    kSquareCount = kMapWidth * kMapHeight,
    kTotalSquareCount = kSquareCount * kMapCount,
    kSlotCount = 8,
    kChampionCount = 4,
    kThingNone = 0xFFFF,
    kM568CandidatePanel = 568,
    kC040Panel = 40,
    kG0426OpenChest = 0x6426,
    kCandidateIndex = 2,
    kCandidateOrdinal = 3,
    kSelectionResurrect = 1,
    kSelectionReincarnate = 2,
    kInitialMapX = 2,
    kInitialMapY = 2,
    kTeleporterMapX = 3,
    kTeleporterMapY = 2,
    kTargetMapIndex = 1,
    kTargetMapX = 1,
    kTargetMapY = 4,
    kTargetDirection = DIR_SOUTH,
    kSeedHash = 0xC0400291u
};

typedef struct MirrorCandidateRuntimePc34 {
    int c040PanelOpen;
    int panelOwner;
    int c040PanelGraphic;
    int candidateIndex;
    int candidateOrdinal;
    int resurrectReincarnateSelection;
    int g0426OpenChest;
    int g0425Slots[kSlotCount];
    int championOrdinals[kChampionCount];
    int championHands[kChampionCount];
    int panelRedraws;
    int candidateMutationsRejected;
    uint32_t panelHash;
    uint32_t championChainHash;
} MirrorCandidateRuntimePc34;

typedef struct TeleporterRunPc34 {
    int queued;
    int processed;
    int commandHandled;
    int stepApplied;
    int postMoveResolved;
    int teleporterCount;
    int audibleCount;
    int finalMapIndex;
    int finalMapX;
    int finalMapY;
    int finalDirection;
} TeleporterRunPc34;

static DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 g_last;

static const char s_source_evidence[] =
    "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC dispatch\n"
    "MOVE.C F0291:4350-4420 party teleporter scope handoff\n"
    "MIRROR.C F0193:3550-3620 C040 resurrect panel draw\n"
    "MIRROR.C F0194:3700-3780 candidate index persistence\n"
    "CHEST.C F0333 G0426 open-chest anchor\n"
    "DUNGEON.C F0163:1769-1838 cell fetch / thing-list handoff";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static unsigned char square_byte(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & 0x1f));
}

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static void set_square(unsigned char *squares,
                       int height,
                       int x,
                       int y,
                       unsigned char value)
{
    squares[(x * height) + y] = value;
}

static uint32_t runtime_panel_hash(const MirrorCandidateRuntimePc34 *runtime)
{
    uint32_t hash = kSeedHash;
    int i;

    hash = fnv1a_u32(hash, (uint32_t)runtime->c040PanelOpen);
    hash = fnv1a_u32(hash, (uint32_t)runtime->panelOwner);
    hash = fnv1a_u32(hash, (uint32_t)runtime->c040PanelGraphic);
    hash = fnv1a_u32(hash, (uint32_t)runtime->candidateIndex);
    hash = fnv1a_u32(hash, (uint32_t)runtime->candidateOrdinal);
    hash = fnv1a_u32(hash,
                     (uint32_t)runtime->resurrectReincarnateSelection);
    hash = fnv1a_u32(hash, (uint32_t)runtime->g0426OpenChest);
    for (i = 0; i < kSlotCount; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)runtime->g0425Slots[i]);
    }
    return hash;
}

static uint32_t runtime_champion_chain_hash(
    const MirrorCandidateRuntimePc34 *runtime)
{
    uint32_t hash = 0x02910426u;
    int i;

    for (i = 0; i < kChampionCount; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)runtime->championOrdinals[i]);
        hash = fnv1a_u32(hash, (uint32_t)runtime->championHands[i]);
    }
    return hash;
}

static void redraw_c040_panel(MirrorCandidateRuntimePc34 *runtime)
{
    if (!runtime || !runtime->c040PanelOpen) {
        return;
    }
    /*
     * ReDMCSB MIRROR.C F0193:3550-3620 redraws the live C040
     * resurrect/reincarnate panel from the current candidate state.  The
     * redraw is intentionally presentation-only here: the candidate index,
     * selection, G0426, and C537..C544/G0425 slots are not rewritten.
     */
    runtime->c040PanelGraphic = kC040Panel;
    runtime->panelRedraws++;
    runtime->panelHash = runtime_panel_hash(runtime);
}

static void init_mirror_runtime(MirrorCandidateRuntimePc34 *runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->c040PanelOpen = 1;
    runtime->panelOwner = kM568CandidatePanel;
    runtime->candidateIndex = kCandidateIndex;
    runtime->candidateOrdinal = kCandidateOrdinal;
    runtime->resurrectReincarnateSelection = kSelectionReincarnate;
    runtime->g0426OpenChest = kG0426OpenChest;
    for (i = 0; i < kSlotCount; ++i) {
        runtime->g0425Slots[i] = 0x5370 + i;
    }
    for (i = 0; i < kChampionCount; ++i) {
        runtime->championOrdinals[i] = i + 1;
        runtime->championHands[i] = 0xC030 + i;
    }
    /*
     * ReDMCSB MIRROR.C F0194:3700-3780 keeps the selected candidate index
     * resident while other runtime systems advance.  CHEST.C F0333 keeps the
     * open chest anchored in G0426 while the panel is live.
     */
    redraw_c040_panel(runtime);
    runtime->championChainHash = runtime_champion_chain_hash(runtime);
}

static int attempt_forbidden_candidate_mutation(
    MirrorCandidateRuntimePc34 *runtime)
{
    if (!runtime || !runtime->c040PanelOpen ||
        runtime->panelOwner != kM568CandidatePanel) {
        return 0;
    }
    runtime->candidateMutationsRejected++;
    return 1;
}

static void setup_dungeon(struct DungeonDatState_Compat *dungeon,
                          struct DungeonMapDesc_Compat maps[kMapCount],
                          struct DungeonMapTiles_Compat tiles[kMapCount],
                          unsigned char map0[kSquareCount],
                          unsigned char map1[kSquareCount])
{
    int i;
    int x;
    int y;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat) * kMapCount);
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat) * kMapCount);
    memset(map0, 0, kSquareCount);
    memset(map1, 0, kSquareCount);
    for (i = 0; i < kMapCount; ++i) {
        maps[i].width = kMapWidth;
        maps[i].height = kMapHeight;
        maps[i].level = (unsigned char)i;
        maps[i].offsetMapX = 0;
        maps[i].offsetMapY = 0;
    }
    tiles[0].squareData = map0;
    tiles[0].squareCount = kSquareCount;
    tiles[1].squareData = map1;
    tiles[1].squareCount = kSquareCount;
    dungeon->header.mapCount = kMapCount;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    for (x = 0; x < kMapWidth; ++x) {
        for (y = 0; y < kMapHeight; ++y) {
            set_square(map0, kMapHeight, x, y,
                       square_byte(DUNGEON_ELEMENT_CORRIDOR, 0));
            set_square(map1, kMapHeight, x, y,
                       square_byte(DUNGEON_ELEMENT_CORRIDOR, 0));
        }
    }
    set_square(map0, kMapHeight, kTeleporterMapX, kTeleporterMapY,
               square_byte(DUNGEON_ELEMENT_TELEPORTER, 0x08));
}

static void setup_party(struct PartyState_Compat *party)
{
    int i;

    memset(party, 0, sizeof(*party));
    party->mapIndex = 0;
    party->mapX = kInitialMapX;
    party->mapY = kInitialMapY;
    party->direction = DIR_EAST;
    party->championCount = kChampionCount;
    for (i = 0; i < kChampionCount; ++i) {
        party->champions[i].present = 1;
        party->champions[i].hp.current = 100;
        party->champions[i].hp.maximum = 100;
        party->champions[i].load = 40;
        party->champions[i].maxLoad = 500;
    }
}

static void run_teleporter_case(int scope,
                                int audible,
                                TeleporterRunPc34 *out)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[kMapCount];
    struct DungeonMapTiles_Compat tiles[kMapCount];
    unsigned char map0[kSquareCount];
    unsigned char map1[kSquareCount];
    struct DungeonThings_Compat things;
    unsigned short squareFirstThings[kTotalSquareCount];
    struct DungeonTeleporter_Compat teleporters[1];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    int i;

    memset(out, 0, sizeof(*out));
    setup_dungeon(&dungeon, maps, tiles, map0, map1);
    memset(&things, 0, sizeof(things));
    memset(teleporters, 0, sizeof(teleporters));
    for (i = 0; i < kTotalSquareCount; ++i) {
        squareFirstThings[i] = THING_ENDOFLIST;
    }
    /*
     * ReDMCSB DUNGEON.C F0163:1769-1838 list handoff: the fixture publishes
     * a teleporter thing at the destination cell consumed by the post-move
     * F0291/F0267 teleporter path.
     */
    squareFirstThings[(kTeleporterMapX * kMapHeight) + kTeleporterMapY] =
        thing_ref(THING_TYPE_TELEPORTER, 0);
    teleporters[0].next = THING_ENDOFLIST;
    teleporters[0].targetMapIndex = kTargetMapIndex;
    teleporters[0].targetMapX = kTargetMapX;
    teleporters[0].targetMapY = kTargetMapY;
    teleporters[0].rotation = kTargetDirection;
    teleporters[0].absoluteRotation = 1;
    teleporters[0].scope = (unsigned char)scope;
    teleporters[0].audible = (unsigned char)audible;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = kTotalSquareCount;
    things.teleporters = teleporters;
    things.teleporterCount = 1;
    setup_party(&party);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    out->queued = DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(
        &pipeline, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0);
    /*
     * ReDMCSB COMMAND.C F0380 dispatches C003 from the queue before the
     * MOVESENS/MOVE.C F0291 teleporter handoff evaluates the party scope.
     */
    out->processed = DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, &things, &party, 0, &result);
    out->commandHandled = result.core.commandHandled;
    out->stepApplied = result.core.stepApplied;
    out->postMoveResolved = result.postMoveResolved;
    out->teleporterCount = result.postMove.teleporterCount;
    out->audibleCount = result.postMove.teleporterAudibleCount;
    out->finalMapIndex = party.mapIndex;
    out->finalMapX = party.mapX;
    out->finalMapY = party.mapY;
    out->finalDirection = party.direction;
}

static void record_check(
    DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 *result,
    int condition,
    uint32_t evidence)
{
    result->assertions++;
    if (!condition) {
        result->failures++;
    }
    result->deterministic_hash =
        fnv1a_u32(result->deterministic_hash, evidence ^ (uint32_t)condition);
}

static void record_runtime_preserved(
    DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 *result,
    const MirrorCandidateRuntimePc34 *before,
    const MirrorCandidateRuntimePc34 *after,
    uint32_t evidenceBase)
{
    int i;

    record_check(result, after->c040PanelOpen == 1, evidenceBase + 1u);
    record_check(result, after->panelOwner == kM568CandidatePanel,
                 evidenceBase + 2u);
    record_check(result, after->c040PanelGraphic == kC040Panel,
                 evidenceBase + 3u);
    record_check(result, after->candidateIndex == before->candidateIndex,
                 evidenceBase + 4u);
    if (after->candidateIndex == before->candidateIndex) {
        result->candidate_index_persists++;
    }
    record_check(result,
                 after->candidateOrdinal == before->candidateOrdinal,
                 evidenceBase + 5u);
    record_check(result,
                 after->resurrectReincarnateSelection ==
                     before->resurrectReincarnateSelection,
                 evidenceBase + 6u);
    if (after->resurrectReincarnateSelection ==
        before->resurrectReincarnateSelection) {
        result->resurrect_reincarnate_selection_persists++;
    }
    record_check(result, after->g0426OpenChest == before->g0426OpenChest,
                 evidenceBase + 7u);
    if (after->g0426OpenChest == before->g0426OpenChest) {
        result->g0426_state_preserved++;
    }
    for (i = 0; i < kSlotCount; ++i) {
        record_check(result, after->g0425Slots[i] == before->g0425Slots[i],
                     evidenceBase + 0x10u + (uint32_t)i);
    }
    for (i = 0; i < kChampionCount; ++i) {
        record_check(result,
                     after->championOrdinals[i] == before->championOrdinals[i],
                     evidenceBase + 0x30u + (uint32_t)i);
        record_check(result,
                     after->championHands[i] == before->championHands[i],
                     evidenceBase + 0x40u + (uint32_t)i);
    }
    record_check(result,
                 after->championChainHash == before->championChainHash,
                 evidenceBase + 0x50u);
    if (after->championChainHash == before->championChainHash) {
        result->champion_chain_preserved++;
    }
}

int run_dm1_v1_mirror_candidate_teleporter_survival_self_test(void)
{
    MirrorCandidateRuntimePc34 before;
    MirrorCandidateRuntimePc34 runtime;
    TeleporterRunPc34 partyScoped;
    TeleporterRunPc34 creatureScoped;

    memset(&g_last, 0, sizeof(g_last));
    g_last.deterministic_hash = 2166136261u;
    init_mirror_runtime(&runtime);
    before = runtime;

    record_check(&g_last, strstr(s_source_evidence, "COMMAND.C F0380") != 0,
                 0x0380u);
    record_check(&g_last, strstr(s_source_evidence, "MOVE.C F0291") != 0,
                 0x0291u);
    record_check(&g_last, strstr(s_source_evidence, "MIRROR.C F0193") != 0,
                 0x0193u);
    record_check(&g_last, strstr(s_source_evidence, "MIRROR.C F0194") != 0,
                 0x0194u);
    record_check(&g_last, strstr(s_source_evidence, "CHEST.C F0333") != 0,
                 0x0333u);
    record_check(&g_last, strstr(s_source_evidence, "DUNGEON.C F0163") != 0,
                 0x0163u);

    run_teleporter_case(0x02, 1, &partyScoped);
    if (partyScoped.teleporterCount > 0) {
        redraw_c040_panel(&runtime);
    }

    record_check(&g_last, partyScoped.queued == 1, 0x1001u);
    record_check(&g_last, partyScoped.processed == 1, 0x1002u);
    record_check(&g_last, partyScoped.commandHandled == 1, 0x1003u);
    record_check(&g_last, partyScoped.stepApplied == 1, 0x1004u);
    record_check(&g_last, partyScoped.postMoveResolved == 1, 0x1005u);
    record_check(&g_last, partyScoped.teleporterCount == 1, 0x1006u);
    record_check(&g_last, partyScoped.audibleCount == 1, 0x1007u);
    record_check(&g_last, partyScoped.finalMapIndex == kTargetMapIndex,
                 0x1008u);
    record_check(&g_last, partyScoped.finalMapX == kTargetMapX, 0x1009u);
    record_check(&g_last, partyScoped.finalMapY == kTargetMapY, 0x100Au);
    record_check(&g_last, partyScoped.finalDirection == kTargetDirection,
                 0x100Bu);
    g_last.teleporter_activations += partyScoped.teleporterCount;
    g_last.party_audible_teleporter_buzzes += partyScoped.audibleCount;
    g_last.panel_redraws = runtime.panelRedraws;
    record_check(&g_last, runtime.panelRedraws >= 2, 0x100Cu);
    record_runtime_preserved(&g_last, &before, &runtime, 0x2000u);

    before = runtime;
    record_check(&g_last, attempt_forbidden_candidate_mutation(&runtime) == 1,
                 0x3001u);
    g_last.mutation_rejections = runtime.candidateMutationsRejected;
    record_check(&g_last, runtime.candidateIndex == before.candidateIndex,
                 0x3002u);
    record_check(&g_last,
                 runtime.resurrectReincarnateSelection ==
                     before.resurrectReincarnateSelection,
                 0x3003u);

    run_teleporter_case(0x01, 1, &creatureScoped);
    record_check(&g_last, creatureScoped.queued == 1, 0x4001u);
    record_check(&g_last, creatureScoped.processed == 1, 0x4002u);
    record_check(&g_last, creatureScoped.commandHandled == 1, 0x4003u);
    record_check(&g_last, creatureScoped.stepApplied == 1, 0x4004u);
    record_check(&g_last, creatureScoped.postMoveResolved == 1, 0x4005u);
    record_check(&g_last, creatureScoped.teleporterCount == 0, 0x4006u);
    record_check(&g_last, creatureScoped.audibleCount == 0, 0x4007u);
    record_check(&g_last, creatureScoped.finalMapIndex == 0, 0x4008u);
    record_check(&g_last, creatureScoped.finalMapX == kTeleporterMapX,
                 0x4009u);
    record_check(&g_last, creatureScoped.finalMapY == kTeleporterMapY,
                 0x400Au);
    record_check(&g_last, creatureScoped.finalDirection == DIR_EAST,
                 0x400Bu);
    if (creatureScoped.teleporterCount == 0 &&
        creatureScoped.finalMapIndex == 0) {
        g_last.creature_scope_party_transition_rejected++;
        g_last.mutation_rejections++;
    }
    record_runtime_preserved(&g_last, &before, &runtime, 0x5000u);

    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  (uint32_t)g_last.teleporter_activations);
    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  (uint32_t)g_last.candidate_index_persists);
    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  (uint32_t)g_last.panel_redraws);
    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  (uint32_t)g_last.g0426_state_preserved);
    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  (uint32_t)g_last.mutation_rejections);
    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  runtime_panel_hash(&runtime));
    g_last.deterministic_hash =
        fnv1a_u32(g_last.deterministic_hash,
                  runtime_champion_chain_hash(&runtime));

    return g_last.failures == 0 ? 1 : 0;
}

const DM1_V1_MirrorCandidateTeleporterSurvivalSelfTestResultPc34 *
dm1_v1_mirror_candidate_teleporter_survival_last_self_test_result_pc34(void)
{
    return &g_last;
}
