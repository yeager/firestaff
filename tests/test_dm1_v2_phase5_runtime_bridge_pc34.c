#include "dm1_v2_phase5_runtime_bridge_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static unsigned char sq(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & 0x1F));
}

static void set_sq(unsigned char* squares, int height, int x, int y, unsigned char value)
{
    squares[x * height + y] = value;
}

static void setup_dungeon(struct DungeonDatState_Compat* dungeon,
                          struct DungeonMapDesc_Compat* map,
                          struct DungeonMapTiles_Compat* tiles,
                          unsigned char* squares,
                          int width,
                          int height)
{
    int x;
    int y;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, 0, (size_t)(width * height));
    map->width = (unsigned char)width;
    map->height = (unsigned char)height;
    tiles->squareData = squares;
    tiles->squareCount = width * height;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            set_sq(squares, height, x, y, sq(DUNGEON_ELEMENT_CORRIDOR, 0));
        }
    }
}

static void setup_party(struct PartyState_Compat* party,
                        int mapX,
                        int mapY,
                        int direction,
                        int championCount)
{
    int i;
    memset(party, 0, sizeof(*party));
    party->mapX = mapX;
    party->mapY = mapY;
    party->direction = direction;
    party->mapIndex = 0;
    party->championCount = championCount;
    for (i = 0; i < championCount && i < CHAMPION_MAX_PARTY; ++i) {
        party->champions[i].present = 1;
        party->champions[i].hp.current = 100;
        party->champions[i].hp.maximum = 100;
        party->champions[i].load = 100;
        party->champions[i].maxLoad = 500;
    }
}

static void init_camera_from_party(DM1_V2_CameraController* camera,
                                   const struct PartyState_Compat* party)
{
    DM1_V2_PlayerPos player;
    dm1_v2_pos_init(&player, party->mapX, party->mapY, party->direction);
    dm1_v2_camera_init(camera, &player);
}

static void check_source_evidence(void)
{
    const char* evidence = dm1_v2_phase5_runtime_bridge_source_evidence_pc34();
    CHECK(strstr(evidence, "COMMAND.C:2096-2106") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C:278-329") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C:330-346") != NULL);
    CHECK(strstr(evidence, "CHAMPION.C:1180-1215") != NULL);
    CHECK(strstr(evidence, "MOVESENS.C:752-818") != NULL);
    CHECK(strstr(evidence, "GAMELOOP.C:69-155") != NULL);
    CHECK(strstr(evidence, "DUNVIEW.C:8318-8612") != NULL);
    CHECK(strstr(evidence, "DRAWVIEW.C:709-722") != NULL);
}

static void test_forward_source_tick_starts_presentation_camera_only(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[8 * 8];
    struct PartyState_Compat party;
    struct PartyState_Compat partyBefore;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelinePc34Compat pipelineAfterSource;
    struct Dm1V1MovementPipelineResultPc34Compat sourceTick;
    struct Dm1V1MovementPipelineResultPc34Compat sourceTickAfterSource;
    DM1_V2_CameraController camera;
    DM1_V2_Phase5RuntimeBridgeResultPc34 bridge;
    int started;

    setup_dungeon(&dungeon, &map, &tiles, squares, 8, 8);
    setup_party(&party, 4, 4, DIR_NORTH, 1);
    partyBefore = party;
    init_camera_from_party(&camera, &partyBefore);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    CHECK(DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(&pipeline, 3, 0, 0) == 1);
    CHECK(DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &sourceTick) == 1);

    CHECK(sourceTick.provenance.commandAccepted == 1);
    CHECK(sourceTick.core.stepApplied == 1);
    CHECK(sourceTick.anyMovementOccurred == 1);
    CHECK(sourceTick.viewportDirty == 1);
    CHECK(pipeline.disabledMovementTicks > 0);
    CHECK(pipeline.projectileDisabledMovementTicks == 0);
    CHECK(party.mapX == 4);
    CHECK(party.mapY == 3);

    pipelineAfterSource = pipeline;
    sourceTickAfterSource = sourceTick;
    started = dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_pc34(
        &pipeline, &sourceTick, &party, &camera, 96, &bridge);

    CHECK(started == 1);
    CHECK(bridge.sourceTickAccepted == 1);
    CHECK(bridge.sourceStepAccepted == 1);
    CHECK(bridge.sourceTurnAccepted == 0);
    CHECK(bridge.cameraStarted == 1);
    CHECK(bridge.cameraMoveStarted == 1);
    CHECK(bridge.sourceCooldownTicks == pipelineAfterSource.disabledMovementTicks);
    CHECK(bridge.sourceProjectileCooldownTicks == pipelineAfterSource.projectileDisabledMovementTicks);
    CHECK(bridge.sourceGameTick == pipelineAfterSource.gameTick);
    CHECK(bridge.sourceLastPartyMovementTime == pipelineAfterSource.lastPartyMovementTime);
    CHECK(bridge.viewportRedrawRequested == sourceTickAfterSource.viewportDirty);
    CHECK(bridge.redrawCadencePreserved == 1);
    CHECK(bridge.sourceMutationForbidden == 1);
    CHECK(camera.fromX == partyBefore.mapX * DM1_V2_SUBPIXEL_SCALE);
    CHECK(camera.fromY == partyBefore.mapY * DM1_V2_SUBPIXEL_SCALE);
    CHECK(camera.targetX == party.mapX * DM1_V2_SUBPIXEL_SCALE);
    CHECK(camera.targetY == party.mapY * DM1_V2_SUBPIXEL_SCALE);

    dm1_v2_camera_tick(&camera, 32);
    dm1_v2_camera_tick(&camera, 64);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.visualX == party.mapX * DM1_V2_SUBPIXEL_SCALE);
    CHECK(camera.visualY == party.mapY * DM1_V2_SUBPIXEL_SCALE);

    CHECK(memcmp(&pipeline, &pipelineAfterSource, sizeof(pipeline)) == 0);
    CHECK(memcmp(&sourceTick, &sourceTickAfterSource, sizeof(sourceTick)) == 0);
    CHECK(party.mapX == 4);
    CHECK(party.mapY == 3);
    CHECK(party.direction == DIR_NORTH);
}

static void test_blocked_source_tick_does_not_start_camera_or_redraw(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[8 * 8];
    struct PartyState_Compat party;
    struct PartyState_Compat partyAfterSource;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelinePc34Compat pipelineAfterSource;
    struct Dm1V1MovementPipelineResultPc34Compat sourceTick;
    struct Dm1V1MovementPipelineResultPc34Compat sourceTickAfterSource;
    DM1_V2_CameraController camera;
    DM1_V2_Phase5RuntimeBridgeResultPc34 bridge;

    setup_dungeon(&dungeon, &map, &tiles, squares, 8, 8);
    set_sq(squares, 8, 4, 3, sq(DUNGEON_ELEMENT_WALL, 0));
    setup_party(&party, 4, 4, DIR_NORTH, 1);
    init_camera_from_party(&camera, &party);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    CHECK(DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(&pipeline, 3, 0, 0) == 1);
    CHECK(DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &sourceTick) == 1);

    CHECK(sourceTick.core.movementBlocked == 1);
    CHECK(sourceTick.blockedMovementVblankWaitRequested == 1);
    CHECK(sourceTick.viewportDirty == 0);
    CHECK(pipeline.disabledMovementTicks == 0);
    CHECK(pipeline.projectileDisabledMovementTicks == 0);
    partyAfterSource = party;
    pipelineAfterSource = pipeline;
    sourceTickAfterSource = sourceTick;

    CHECK(dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_pc34(
        &pipeline, &sourceTick, &party, &camera, 96, &bridge) == 0);
    CHECK(bridge.sourceMovementBlocked == 1);
    CHECK(bridge.cameraStarted == 0);
    CHECK(bridge.viewportRedrawRequested == 0);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(memcmp(&pipeline, &pipelineAfterSource, sizeof(pipeline)) == 0);
    CHECK(memcmp(&sourceTick, &sourceTickAfterSource, sizeof(sourceTick)) == 0);
    CHECK(memcmp(&party, &partyAfterSource, sizeof(party)) == 0);
}

static void test_turn_source_tick_starts_turn_camera_without_cooldown(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[8 * 8];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelinePc34Compat pipelineAfterSource;
    struct Dm1V1MovementPipelineResultPc34Compat sourceTick;
    struct Dm1V1MovementPipelineResultPc34Compat sourceTickAfterSource;
    DM1_V2_CameraController camera;
    DM1_V2_Phase5RuntimeBridgeResultPc34 bridge;

    setup_dungeon(&dungeon, &map, &tiles, squares, 8, 8);
    setup_party(&party, 4, 4, DIR_NORTH, 1);
    init_camera_from_party(&camera, &party);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    CHECK(DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(&pipeline, 1, 0, 0) == 1);
    CHECK(DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &sourceTick) == 1);

    CHECK(sourceTick.provenance.commandAccepted == 1);
    CHECK(sourceTick.core.turnApplied == 1);
    CHECK(sourceTick.anyTurnOccurred == 1);
    CHECK(sourceTick.core.stepApplied == 0);
    CHECK(sourceTick.viewportDirty == 1);
    CHECK(pipeline.disabledMovementTicks == 0);
    CHECK(pipeline.projectileDisabledMovementTicks == 0);
    CHECK(party.mapX == 4);
    CHECK(party.mapY == 4);
    CHECK(party.direction == DIR_WEST);

    pipelineAfterSource = pipeline;
    sourceTickAfterSource = sourceTick;
    CHECK(dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_pc34(
        &pipeline, &sourceTick, &party, &camera, 80, &bridge) == 1);
    CHECK(bridge.sourceTurnAccepted == 1);
    CHECK(bridge.sourceStepAccepted == 0);
    CHECK(bridge.cameraTurnStarted == 1);
    CHECK(bridge.sourceCooldownTicks == 0);
    CHECK(camera.fromFacingDir == DIR_NORTH);
    CHECK(camera.targetFacingDir == DIR_WEST);
    CHECK(camera.visualX == 4 * DM1_V2_SUBPIXEL_SCALE);
    CHECK(camera.visualY == 4 * DM1_V2_SUBPIXEL_SCALE);

    dm1_v2_camera_tick(&camera, 80);
    CHECK(!dm1_v2_camera_is_active(&camera));
    CHECK(camera.facingDir == DIR_WEST);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
    CHECK(memcmp(&pipeline, &pipelineAfterSource, sizeof(pipeline)) == 0);
    CHECK(memcmp(&sourceTick, &sourceTickAfterSource, sizeof(sourceTick)) == 0);

    init_camera_from_party(&camera, &party);
    CHECK(dm1_v2_phase5_runtime_bridge_start_camera_from_v1_tick_ex_pc34(
        &pipeline, &sourceTick, &party, &camera, 80, 1, &bridge) == 1);
    CHECK(bridge.sourceTurnAccepted == 1);
    CHECK(bridge.cameraTurnStarted == 1);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == 0);
    dm1_v2_camera_tick(&camera, 20);
    CHECK(dm1_v2_camera_turn_pan_offset_x(&camera) == -128);
    CHECK(party.mapX == 4);
    CHECK(party.mapY == 4);
    CHECK(party.direction == DIR_WEST);
}

int main(void)
{
    check_source_evidence();
    test_forward_source_tick_starts_presentation_camera_only();
    test_blocked_source_tick_does_not_start_camera_or_redraw();
    test_turn_source_tick_starts_turn_camera_without_cooldown();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_phase5_runtime_bridge_pc34: ok");
    return 0;
}
