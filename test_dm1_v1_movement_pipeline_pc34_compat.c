#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_input_poll_pc34_compat.h"

/*
 * Integration test for the complete DM1 V1 Movement Command Pipeline.
 *
 * Exercises the full chain: input event → command queue → movement
 * validation → party state mutation → timing cooldown → post-move
 * environment resolution.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   MOVESENS.C: F0267_MOVE_GetMoveResult_CPSCE (movement validation),
 *               F0276_SENSOR_ProcessThingAdditionOrRemoval (sensors)
 *   COMMAND.C:  F0380 (process queue), F0358-F0361 (input routing)
 *   CLIKMENU.C: F0365 (turn), F0366 (step)
 *   CHAMPION.C: F0310 (movement ticks)
 *   GAMELOOP.C: cooldown decrement
 */

static int g_pass = 0;
static int g_fail = 0;

#define EXPECT(label, cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s: %s\n", label, #cond); \
        g_fail++; \
    } else { \
        g_pass++; \
    } \
} while(0)

#define EXPECT_INT(label, got, want) do { \
    int _g = (got), _w = (want); \
    if (_g != _w) { \
        fprintf(stderr, "FAIL %s: got=%d want=%d\n", label, _g, _w); \
        g_fail++; \
    } else { \
        g_pass++; \
    } \
} while(0)

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
    int width, int height)
{
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
    /* Fill with corridors */
    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
            set_sq(squares, height, x, y, sq(DUNGEON_ELEMENT_CORRIDOR, 0));
}

static void setup_two_level_stairs_dungeon(struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat maps[2],
    struct DungeonMapTiles_Compat tiles[2],
    unsigned char level0[5 * 5],
    unsigned char level1[5 * 5])
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(struct DungeonMapDesc_Compat) * 2);
    memset(tiles, 0, sizeof(struct DungeonMapTiles_Compat) * 2);
    memset(level0, 0, 25);
    memset(level1, 0, 25);

    for (int i = 0; i < 2; ++i) {
        maps[i].width = 5;
        maps[i].height = 5;
        maps[i].level = (unsigned char)i;
        maps[i].offsetMapX = 0;
        maps[i].offsetMapY = 0;
    }
    tiles[0].squareData = level0;
    tiles[0].squareCount = 25;
    tiles[1].squareData = level1;
    tiles[1].squareCount = 25;
    dungeon->header.mapCount = 2;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 5; ++y) {
            set_sq(level0, 5, x, y, sq(DUNGEON_ELEMENT_CORRIDOR, 0));
            set_sq(level1, 5, x, y, sq(DUNGEON_ELEMENT_CORRIDOR, 0));
        }
    }
}

static void setup_party(struct PartyState_Compat* party,
    int mapX, int mapY, int dir, int championCount)
{
    memset(party, 0, sizeof(*party));
    party->mapX = mapX;
    party->mapY = mapY;
    party->direction = dir;
    party->mapIndex = 0;
    party->championCount = championCount;
    for (int i = 0; i < championCount && i < CHAMPION_MAX_PARTY; ++i) {
        party->champions[i].present = 1;
        party->champions[i].hp.current = 100;
        party->champions[i].hp.maximum = 100;
        party->champions[i].load = 100;
        party->champions[i].maxLoad = 500;
    }
}

static struct Dm1V1InputEventPc34Compat key_event(int keyCode)
{
    struct Dm1V1InputEventPc34Compat ev;
    memset(&ev, 0, sizeof(ev));
    ev.kind = DM1_V1_INPUT_KIND_KEY;
    ev.keyCode = keyCode;
    return ev;
}

static unsigned short thing_ref(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

/* ---- Test: keyboard forward step on open corridor ---- */
static void test_keyboard_forward_step(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Enqueue forward key (numpad 5 / cursor up maps to MOVE_FORWARD) */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));  /* keypad 5 = forward */

    /* Process tick */
    int rc = DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT("fwd_step_rc", rc == 1);
    EXPECT("fwd_step_applied", result.core.stepApplied == 1);
    EXPECT("fwd_step_turn", result.core.turnApplied == 0);
    EXPECT("fwd_step_blocked", result.core.movementBlocked == 0);
    /* North step: Y decreases by 1 (source: MOVESENS.C direction system) */
    EXPECT_INT("fwd_step_x", party.mapX, 5);
    EXPECT_INT("fwd_step_y", party.mapY, 4);
    EXPECT_INT("fwd_step_dir", party.direction, DIR_NORTH);
    EXPECT("fwd_step_viewport", result.viewportDirty == 1);
    EXPECT("fwd_step_movement", result.anyMovementOccurred == 1);
    /* Cooldown set after successful step (CLIKMENU.C:330-346) */
    EXPECT("fwd_step_cooldown", pipeline.disabledMovementTicks > 0);
}

/* ---- Test: keyboard turn left ---- */
static void test_keyboard_turn_left(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Turn left key (numpad 4 / Home) */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB34));

    int rc = DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT("turn_left_rc", rc == 1);
    EXPECT("turn_left_applied", result.core.turnApplied == 1);
    EXPECT("turn_left_step", result.core.stepApplied == 0);
    /* N turn-left = W (source: F0700 TurnDirection, direction+3 mod 4) */
    EXPECT_INT("turn_left_dir", party.direction, DIR_WEST);
    EXPECT_INT("turn_left_x", party.mapX, 5);
    EXPECT_INT("turn_left_y", party.mapY, 5);
    EXPECT("turn_left_viewport", result.viewportDirty == 1);
    /* Turns do NOT set movement cooldown (CLIKMENU.C:142-179) */
    EXPECT_INT("turn_left_cooldown", pipeline.disabledMovementTicks, 0);
}

/* ---- Test: wall blocks movement ---- */
static void test_wall_blocks_movement(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    /* Place wall at (5,4) — directly north of party */
    set_sq(squares, 10, 5, 4, sq(DUNGEON_ELEMENT_WALL, 0));
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));

    int rc = DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT("wall_rc", rc == 1);
    EXPECT("wall_blocked", result.core.movementBlocked == 1);
    EXPECT("wall_step", result.core.stepApplied == 0);
    EXPECT_INT("wall_blocked_vblank_wait_requested", result.blockedMovementVblankWaitRequested, 1);
    EXPECT_INT("wall_blocked_vblank_wait_count", result.blockedMovementVblankWaitCount, 1);
    EXPECT_INT("wall_blocked_keeps_input_wait", result.blockedMovementKeepsInputWaitArmed, 1);
    EXPECT_INT("wall_blocked_no_viewport_dirty", result.viewportDirty, 0);
    EXPECT_INT("wall_x", party.mapX, 5);
    EXPECT_INT("wall_y", party.mapY, 5);
}

/* ---- Test: closed door blocks, open door passes ---- */
static void test_door_passability(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    /* Door state 3 = closed (blocks) */
    set_sq(squares, 10, 5, 4, sq(DUNGEON_ELEMENT_DOOR, 3));
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("door_closed_blocked", result.core.movementBlocked == 1);
    EXPECT_INT("door_closed_vblank_wait_requested", result.blockedMovementVblankWaitRequested, 1);
    EXPECT_INT("door_closed_y", party.mapY, 5);

    /* Door state 0 = open (passes) */
    set_sq(squares, 10, 5, 4, sq(DUNGEON_ELEMENT_DOOR, 0));
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("door_open_passed", result.core.stepApplied == 1);
    EXPECT_INT("door_open_no_blocked_vblank_wait", result.blockedMovementVblankWaitRequested, 0);
    EXPECT_INT("door_open_y", party.mapY, 4);
}

/* ---- Test: movement cooldown gate (F0380 movement-disabled check) ---- */
static void test_movement_cooldown_gate(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Move forward to set cooldown */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("gate_first_step", result.core.stepApplied == 1);
    EXPECT("gate_cooldown_set", pipeline.disabledMovementTicks > 0);

    /* Try another forward while cooldown active — should be gated */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("gate_blocked", result.core.queue.movementDisabledGate == 1);
    EXPECT("gate_not_applied", result.core.stepApplied == 0);
    EXPECT_INT("gate_y_unchanged", party.mapY, 4);

    /* Drain cooldown via DecrementCooldowns (GAMELOOP.C:150-155) */
    while (pipeline.disabledMovementTicks > 0) {
        DM1_V1_MovementPipeline_DecrementCooldownsPc34Compat(&pipeline);
    }

    /* Now movement should be accepted again */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("gate_drained_step", result.core.stepApplied == 1);
    EXPECT_INT("gate_drained_y", party.mapY, 3);
}

/* ---- Test: multi-command queue sequencing ---- */
static void test_queue_sequencing(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Enqueue: turn right, then forward */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB36));  /* turn right */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));  /* forward */

    /* Process turn right */
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("seq_turn", result.core.turnApplied == 1);
    EXPECT_INT("seq_dir_east", party.direction, DIR_EAST);

    /* Process forward (now east) — no cooldown from turn */
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("seq_step", result.core.stepApplied == 1);
    EXPECT_INT("seq_x", party.mapX, 6);  /* East step: X+1 */
    EXPECT_INT("seq_y", party.mapY, 5);
}

/* ---- Test: empty queue produces no movement ---- */
static void test_empty_queue(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* No input enqueued */
    int rc = DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("empty_rc", rc == 1);
    EXPECT("empty_no_step", result.core.stepApplied == 0);
    EXPECT("empty_no_turn", result.core.turnApplied == 0);
    EXPECT_INT("empty_x", party.mapX, 5);
    EXPECT_INT("empty_y", party.mapY, 5);
}

/* ---- Test: bounds check (map edge) ---- */
static void test_bounds_check(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[5 * 5];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 5, 5);
    /* Party at (2, 0) facing north — step would go to y=-1 */
    setup_party(&party, 2, 0, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("bounds_blocked", result.core.movementBlocked == 1);
    EXPECT_INT("bounds_y", party.mapY, 0);
}

/* ---- Test: source evidence string ---- */
static void test_source_evidence(void)
{
    const char* ev = DM1_V1_MovementPipeline_SourceEvidencePc34Compat();
    EXPECT("evidence_not_null", ev != NULL);
    EXPECT("evidence_has_F0267", strstr(ev, "F0267") != NULL);
    EXPECT("evidence_has_F0276", strstr(ev, "F0276") != NULL);
    EXPECT("evidence_has_COMMAND", strstr(ev, "COMMAND.C") != NULL);
    EXPECT("evidence_has_MOVESENS", strstr(ev, "MOVESENS") != NULL);
    EXPECT("evidence_has_GAMELOOP", strstr(ev, "GAMELOOP") != NULL);
    EXPECT("evidence_has_blocked_vblank", strstr(ev, "one PC-34 blocked-movement VBlank") != NULL);
}

/* ---- Test: compat provenance chain without fake original evidence ---- */
static void test_command_movement_viewport_provenance(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT_INT("prov_command_accepted", result.provenance.commandAccepted, 1);
    EXPECT_INT("prov_movement_applied", result.provenance.movementApplied, 1);
    EXPECT_INT("prov_viewport_present", result.provenance.viewportPresent, 1);
    EXPECT_INT("prov_not_original_runtime", result.provenance.originalRuntimeObserved, 0);
    EXPECT_INT("prov_no_pixel_parity_claim", result.provenance.noPixelParityClaim, 1);
    EXPECT("prov_command_evidence", strstr(result.provenance.commandAcceptedEvidence, "COMMAND.C:2075-2099") != NULL);
    EXPECT("prov_movement_evidence", strstr(result.provenance.movementAppliedEvidence, "CLIKMENU.C:325-328") != NULL);
    EXPECT("prov_viewport_evidence", strstr(result.provenance.viewportPresentEvidence, "DRAWVIEW.C:721-722") != NULL);
    EXPECT("prov_blocked_wait_evidence", strstr(result.provenance.viewportPresentEvidence, "CLIKMENU.C:317-323") != NULL);
}

/* ---- Test: backward step ---- */
static void test_backward_step(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Backward key (numpad 2) = MOVE_BACKWARD */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB32));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("back_step_applied", result.core.stepApplied == 1);
    /* North backward = south step: Y+1 */
    EXPECT_INT("back_step_y", party.mapY, 6);
    EXPECT_INT("back_step_x", party.mapX, 5);
}

/* ---- Test: strafe left/right ---- */
static void test_strafe(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Strafe right (numpad 3) = MOVE_RIGHT */
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB33));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("strafe_r_applied", result.core.stepApplied == 1);
    /* Facing north, right = east: X+1 */
    EXPECT_INT("strafe_r_x", party.mapX, 6);
    EXPECT_INT("strafe_r_y", party.mapY, 5);

    /* Reset and test strafe left (numpad 1) = MOVE_LEFT */
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB31));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("strafe_l_applied", result.core.stepApplied == 1);
    /* Facing north, left = west: X-1 */
    EXPECT_INT("strafe_l_x", party.mapX, 4);
    EXPECT_INT("strafe_l_y", party.mapY, 5);
}

/* ---- Test: source-locked stairs step consequences ---- */
static void test_stairs_step_consequence(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char level0[5 * 5];
    unsigned char level1[5 * 5];
    struct PartyState_Compat party;
    struct DungeonThings_Compat things;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_two_level_stairs_dungeon(&dungeon, maps, tiles, level0, level1);
    memset(&things, 0, sizeof(things));
    things.loaded = 1;

    /* CLIKMENU.C:271-276: stepping into a stairs square calls F0364 and
     * returns before the normal G0310 cooldown path at CLIKMENU.C:330-346.
     * DUNGEON.C:1508-1582 maps the target by level/offset and computes exit
     * facing from the destination stairs orientation/blocker probe.
     */
    set_sq(level0, 5, 2, 1, sq(DUNGEON_ELEMENT_STAIRS, 0));
    set_sq(level1, 5, 2, 1, sq(DUNGEON_ELEMENT_STAIRS, 0));
    setup_party(&party, 2, 2, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, &things, &party, NULL, &result);

    EXPECT("stairs_into_transition", result.core.stairTransitionApplied == 1);
    EXPECT("stairs_into_no_regular_step", result.core.stepApplied == 0);
    EXPECT("stairs_into_source_walk_off", result.core.stairSourceLeaveProcessed == 1);
    EXPECT("stairs_into_target_walk_off", result.core.stairTargetLeaveProcessed == 1);
    EXPECT("stairs_into_movement_flag", result.anyMovementOccurred == 1);
    EXPECT_INT("stairs_into_map", party.mapIndex, 1);
    EXPECT_INT("stairs_into_x", party.mapX, 2);
    EXPECT_INT("stairs_into_y", party.mapY, 1);
    EXPECT_INT("stairs_into_no_cooldown", pipeline.disabledMovementTicks, 0);

    /* CLIKMENU.C:264-267: backward while already on stairs consumes the
     * stairs before any relative backward coordinate step. */
    setup_two_level_stairs_dungeon(&dungeon, maps, tiles, level0, level1);
    set_sq(level0, 5, 2, 2, sq(DUNGEON_ELEMENT_STAIRS, 0));
    set_sq(level1, 5, 2, 2, sq(DUNGEON_ELEMENT_STAIRS, 0));
    setup_party(&party, 2, 2, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB32));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, &things, &party, NULL, &result);

    EXPECT("stairs_backward_transition", result.core.stairTransitionApplied == 1);
    EXPECT("stairs_backward_source_walk_off", result.core.stairSourceLeaveProcessed == 1);
    EXPECT("stairs_backward_no_target_walk_off", result.core.stairTargetLeaveProcessed == 0);
    EXPECT_INT("stairs_backward_map", party.mapIndex, 1);
    EXPECT_INT("stairs_backward_x_not_south", party.mapX, 2);
    EXPECT_INT("stairs_backward_y_not_south", party.mapY, 2);
    EXPECT_INT("stairs_backward_no_cooldown", pipeline.disabledMovementTicks, 0);

    /* CLIKMENU.C:325-328: moving from a stairs square to a non-stairs
     * square calls F0267 with CM1_MAPX_NOT_ON_A_SQUARE as the source.
     * That skips source-stairs walk-off processing, still applies the
     * destination walk-on pass, and continues into normal G0310 cooldown.
     */
    setup_two_level_stairs_dungeon(&dungeon, maps, tiles, level0, level1);
    set_sq(level0, 5, 2, 2, sq(DUNGEON_ELEMENT_STAIRS, 0));
    setup_party(&party, 2, 2, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline,
        key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, &things, &party, NULL, &result);

    EXPECT("stairs_source_forward_regular_step", result.core.stepApplied == 1);
    EXPECT("stairs_source_forward_no_transition", result.core.stairTransitionApplied == 0);
    EXPECT("stairs_source_forward_skip_source_walk_off", result.core.sourceStairsWalkOffSkipped == 1);
    EXPECT_INT("stairs_source_forward_map", party.mapIndex, 0);
    EXPECT_INT("stairs_source_forward_x", party.mapX, 2);
    EXPECT_INT("stairs_source_forward_y", party.mapY, 1);
    EXPECT("stairs_source_forward_sets_cooldown", pipeline.disabledMovementTicks > 0);
}


/* ---- Test: consolidated command→movement→viewport wall-order source lock ---- */
static void test_command_movement_viewport_wall_order_source_lock(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    const char* movementEvidence;
    const char* viewportEvidence;
    const DM1_ViewportDrawStep* step;
    const DM1_ViewportWallDrawSpec* wall;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT_INT("cmd_move_view.command", result.provenance.commandAccepted, 1);
    EXPECT_INT("cmd_move_view.movement", result.provenance.movementApplied, 1);
    EXPECT_INT("cmd_move_view.viewport", result.provenance.viewportPresent, 1);
    EXPECT_INT("cmd_move_view.no_original_runtime_claim", result.provenance.originalRuntimeObserved, 0);
    EXPECT_INT("cmd_move_view.no_pixel_claim", result.provenance.noPixelParityClaim, 1);

    movementEvidence = DM1_V1_MovementPipeline_SourceEvidencePc34Compat();
    EXPECT("cmd_move_view.evidence.command_mouse", strstr(movementEvidence, "COMMAND.C:106-114") != NULL);
    EXPECT("cmd_move_view.evidence.command_keyboard", strstr(movementEvidence, "COMMAND.C:252-260") != NULL);
    EXPECT("cmd_move_view.evidence.f0380_gate", strstr(movementEvidence, "COMMAND.C:2075-2099") != NULL);
    EXPECT("cmd_move_view.evidence.f0380_dispatch", strstr(movementEvidence, "COMMAND.C:2150-2156") != NULL);
    EXPECT("cmd_move_view.evidence.f0366_deltas", strstr(movementEvidence, "CLIKMENU.C:224-233") != NULL);
    EXPECT("cmd_move_view.evidence.f0366_collision", strstr(movementEvidence, "CLIKMENU.C:278-323") != NULL);
    EXPECT("cmd_move_view.evidence.f0366_timing", strstr(movementEvidence, "CLIKMENU.C:325-346") != NULL);
    EXPECT("cmd_move_view.evidence.gameloop_cooldown", strstr(movementEvidence, "GAMELOOP.C:150-155") != NULL);
    EXPECT("cmd_move_view.evidence.projectile_precheck", strstr(movementEvidence, "MOVESENS.C:433-435") != NULL);
    EXPECT("cmd_move_view.evidence.post_move_teleporter", strstr(movementEvidence, "MOVESENS.C:475-535") != NULL);
    EXPECT("cmd_move_view.evidence.post_move_pit", strstr(movementEvidence, "MOVESENS.C:538-606") != NULL);
    EXPECT("cmd_move_view.evidence.group_interlock", strstr(movementEvidence, "MOVESENS.C:830-887") != NULL);
    EXPECT("cmd_move_view.evidence.projectile_sensor_exception", strstr(movementEvidence, "MOVESENS.C:893-897") != NULL);
    EXPECT("cmd_move_view.evidence.viewport_order", strstr(movementEvidence, "DUNVIEW.C:8446-8542") != NULL);
    EXPECT("cmd_move_view.evidence.viewport_blit", strstr(movementEvidence, "DUNVIEW.C:8609-8610") != NULL);
    EXPECT("cmd_move_view.evidence.drawview_blit", strstr(movementEvidence, "DRAWVIEW.C:721-722") != NULL);

    viewportEvidence = dm1_viewport_3d_source_evidence();
    EXPECT("cmd_move_view.viewport_evidence.order", strstr(viewportEvidence, "DUNVIEW.C:8446-8542") != NULL);
    EXPECT("cmd_move_view.viewport_evidence.restore", strstr(viewportEvidence, "DUNVIEW.C:8577-8579") != NULL);
    EXPECT("cmd_move_view.viewport_evidence.f0097", strstr(viewportEvidence, "DRAWVIEW.C:721-722") != NULL);

    EXPECT_INT("cmd_move_view.draw_order.count", (int)dm1_viewport_3d_draw_order_count(), 19);
    step = dm1_viewport_3d_get_draw_order_step(3);
    EXPECT("cmd_move_view.draw_order.03", step && step->square == DM1_VIEW_SQUARE_D3L2 && strstr(step->source_lines, "8478-8482") != NULL);
    step = dm1_viewport_3d_get_draw_order_step(4);
    EXPECT("cmd_move_view.draw_order.04", step && step->square == DM1_VIEW_SQUARE_D3R2 && strstr(step->source_lines, "8483-8486") != NULL);
    step = dm1_viewport_3d_get_draw_order_step(18);
    EXPECT("cmd_move_view.draw_order.18", step && step->square == DM1_VIEW_SQUARE_D0C && strstr(step->source_lines, "8542") != NULL);

    wall = dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3L2);
    EXPECT("cmd_move_view.wall.d3l2", wall && wall->native_wall == DM1_WALL_D3L2 && wall->parity_wall == DM1_WALL_D3R2 && wall->wall_case_returns);
    wall = dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2C);
    EXPECT("cmd_move_view.wall.d2c", wall && wall->center_wall && wall->front_alcove_reveals_contents && strstr(wall->occlusion_source_lines, "7312") != NULL);
}

/* ---- Test: F0267 post-move environment side effects: pit + teleporter ---- */
static void test_post_move_environment_side_effects(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    unsigned char level0[5 * 5];
    unsigned char level1[5 * 5];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    /* MOVESENS.C:538-606: after a legal step onto an open, non-imaginary
     * pit, F0267 chains to the lower map and applies 20 fall damage to each
     * living champion.  This is not just position bookkeeping: the pipeline
     * must publish the map transition and HP side effect together. */
    setup_two_level_stairs_dungeon(&dungeon, maps, tiles, level0, level1);
    set_sq(level0, 5, 2, 1, sq(DUNGEON_ELEMENT_PIT, 0x08));
    setup_party(&party, 2, 2, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, key_event(0xAB35));
    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT("post_pit_step_applied", result.core.stepApplied == 1);
    EXPECT("post_pit_resolved", result.postMoveResolved == 1);
    EXPECT_INT("post_pit_count", result.postMove.pitCount, 1);
    EXPECT_INT("post_pit_map", party.mapIndex, 1);
    EXPECT_INT("post_pit_x", party.mapX, 2);
    EXPECT_INT("post_pit_y", party.mapY, 1);
    EXPECT_INT("post_pit_damage_recorded", result.postMove.championFallDamage[0], 20);
    EXPECT_INT("post_pit_hp_applied", party.champions[0].hp.current, 80);
    EXPECT_INT("post_pit_any_movement", result.anyMovementOccurred, 1);

    /* MOVESENS.C:475-535: teleporters only fire when open and scoped for
     * objects/party, then move the party to the target and apply absolute or
     * relative rotation before normal post-move completion. */
    {
        struct DungeonDatState_Compat tdungeon;
        struct DungeonMapDesc_Compat tmap;
        struct DungeonMapTiles_Compat ttiles;
        unsigned char tsquares[6 * 6];
        struct DungeonThings_Compat things;
        unsigned short firstThings[6 * 6];
        struct DungeonTeleporter_Compat teleporters[1];
        int i;

        setup_dungeon(&tdungeon, &tmap, &ttiles, tsquares, 6, 6);
        memset(&things, 0, sizeof(things));
        memset(teleporters, 0, sizeof(teleporters));
        for (i = 0; i < 6 * 6; ++i) firstThings[i] = THING_ENDOFLIST;
        things.loaded = 1;
        things.squareFirstThings = firstThings;
        things.squareFirstThingCount = 6 * 6;
        things.teleporters = teleporters;
        things.teleporterCount = 1;
        firstThings[(3 * 6) + 2] = thing_ref(THING_TYPE_TELEPORTER, 0);
        teleporters[0].next = THING_ENDOFLIST;
        teleporters[0].targetMapIndex = 0;
        teleporters[0].targetMapX = 1;
        teleporters[0].targetMapY = 4;
        teleporters[0].rotation = DIR_SOUTH;
        teleporters[0].absoluteRotation = 1;
        teleporters[0].scope = 0x02;

        set_sq(tsquares, 6, 3, 2, sq(DUNGEON_ELEMENT_TELEPORTER, 0));
        setup_party(&party, 2, 2, DIR_EAST, 1);
        DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
        DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, key_event(0xAB35));
        DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
            &pipeline, &tdungeon, &things, &party, NULL, &result);
        EXPECT_INT("post_teleporter_closed_no_chain", result.postMove.teleporterCount, 0);
        EXPECT_INT("post_teleporter_closed_x", party.mapX, 3);
        EXPECT_INT("post_teleporter_closed_y", party.mapY, 2);
        EXPECT_INT("post_teleporter_closed_dir", party.direction, DIR_EAST);

        set_sq(tsquares, 6, 3, 2, sq(DUNGEON_ELEMENT_TELEPORTER, 0x08));
        setup_party(&party, 2, 2, DIR_EAST, 1);
        DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
        DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, key_event(0xAB35));
        DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
            &pipeline, &tdungeon, &things, &party, NULL, &result);
        EXPECT_INT("post_teleporter_count", result.postMove.teleporterCount, 1);
        EXPECT_INT("post_teleporter_final_x", party.mapX, 1);
        EXPECT_INT("post_teleporter_final_y", party.mapY, 4);
        EXPECT_INT("post_teleporter_abs_rotation", party.direction, DIR_SOUTH);
        EXPECT_INT("post_teleporter_any_movement", result.anyMovementOccurred, 1);

        /* MOVESENS.C:438-606 resolves the teleporter chain before
         * MOVESENS.C:799-818 fires party leave/enter sensors.  A successful
         * step onto an open teleporter must therefore publish the final
         * target square enter sensor, not the intermediate teleporter
         * square sensor, even if the teleporter square carries its own sensor. */
        {
            struct DungeonSensor_Compat sensors[2];
            struct DungeonGroup_Compat groups[1];
            int teleporterCompactSftIndex = 1;
            memset(sensors, 0, sizeof(sensors));
            memset(groups, 0, sizeof(groups));
            sensors[0].next = THING_ENDOFLIST;
            sensors[0].sensorType = 13;
            sensors[0].sensorData = 11;
            sensors[1].next = THING_ENDOFLIST;
            sensors[1].sensorType = 13;
            sensors[1].sensorData = 22;
            groups[0].next = thing_ref(THING_TYPE_SENSOR, 1);
            things.sensors = sensors;
            things.sensorCount = 2;
            things.groups = groups;
            things.groupCount = 1;
            teleporters[0].next = thing_ref(THING_TYPE_SENSOR, 0);
            firstThings[teleporterCompactSftIndex] = thing_ref(THING_TYPE_TELEPORTER, 0);
            set_sq(tsquares, 6, 1, 4,
                sq(DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST));
            firstThings[0] = thing_ref(THING_TYPE_GROUP, 0);

            setup_party(&party, 2, 2, DIR_EAST, 1);
            DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
            EXPECT_INT("post_teleporter_intermediate_sft_sensor",
                firstThings[teleporterCompactSftIndex], thing_ref(THING_TYPE_TELEPORTER, 0));
            DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, key_event(0xAB35));
            DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
                &pipeline, &tdungeon, &things, &party, NULL, &result);
            EXPECT_INT("post_teleporter_group_delete_before_enter",
                result.postMoveDestinationGroupDeleted, 1);
            EXPECT_INT("post_teleporter_deleted_group_thing",
                result.postMoveDeletedGroupThing, thing_ref(THING_TYPE_GROUP, 0));
            EXPECT_INT("post_teleporter_group_unlinked_before_enter",
                firstThings[0], thing_ref(THING_TYPE_SENSOR, 1));
            EXPECT_INT("post_teleporter_sensor_order_final_enter_count", result.core.enterEffects.count, 1);
            EXPECT_INT("post_teleporter_sensor_order_final_enter_text", result.core.enterEffects.effects[0].textIndex, 22);
            EXPECT_INT("post_teleporter_sensor_order_not_intermediate", result.core.enterEffects.effects[0].textIndex == 11, 0);
        }
    }
}


/* ---- Test: direct pipeline command wrapper bypasses key/mouse routing ---- */
static void test_direct_command_wrapper_forward_step(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    struct Dm1V1QueuedCommandPc34Compat queued;
    int enqueued;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    enqueued = DM1_V1_MovementPipeline_EnqueueCommandPc34Compat(
        &pipeline, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0);
    EXPECT_INT("direct_wrapper_enqueued", enqueued, 1);
    EXPECT_INT("direct_wrapper_peek", DM1_V1_InputCommandQueue_PeekPc34Compat(
        &pipeline.commandQueue, &queued), 1);
    EXPECT_INT("direct_wrapper_command", queued.command, DM1_V1_COMMAND_MOVE_FORWARD);

    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("direct_wrapper_step", result.core.stepApplied == 1);
    EXPECT_INT("direct_wrapper_x", party.mapX, 5);
    EXPECT_INT("direct_wrapper_y", party.mapY, 4);
    EXPECT_INT("direct_wrapper_dequeued", result.core.queue.dequeued, 1);
}

/* ---- Test: original keyboard-buffer route to first redraw ---- */
static void test_original_keyboard_buffer_forward_route_to_first_redraw(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    M11_InputState input;
    unsigned short transcript[2];
    unsigned int transcriptCount = 0u;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);
    m11_input_init(&input);

    /* Source lock:
     * - INPUT.C:822-858 stores/extracts the 64-slot keyboard buffer.
     * - IO2.C:47-50 normalizes the shifted PC-34 up-arrow route to 0x004C.
     * - COMMAND.C:677-684 maps G0459 movement keyboard 0x004C to C003.
     * - COMMAND.C:1709-1813 F0361 queues the keyboard command.
     * - COMMAND.C:2045-2156 F0380 dequeues it and dispatches C003 to F0366.
     * - CLIKMENU.C:256-347 applies the forward step and arms the next redraw.
     *
     * The original runtime transcript blocker is represented here at the
     * keyboard-buffer boundary: F1097 stores one normalized key and F1098
     * extracts the same key before the command queue sees it.
     */
    EXPECT_INT("keyboard_buffer_store_i34e_forward", m11_input_store_key(&input, 0x004C), 1);
    EXPECT_INT("keyboard_buffer_available_after_store", m11_input_key_available(&input), 1);

    while (m11_input_key_available(&input)) {
        unsigned short key = m11_input_get_key(&input);
        if (transcriptCount < (sizeof(transcript) / sizeof(transcript[0]))) {
            transcript[transcriptCount++] = key;
        }
        EXPECT_INT("keyboard_buffer_enqueue_to_pipeline",
            DM1_V1_MovementPipeline_EnqueueInputPc34Compat(
                &pipeline,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, key, 0, 0, 0 }),
            1);
    }

    EXPECT_INT("keyboard_buffer_transcript_count", (int)transcriptCount, 1);
    EXPECT_INT("keyboard_buffer_transcript_key", transcript[0], 0x004C);
    EXPECT_INT("keyboard_buffer_empty_after_drain", m11_input_key_available(&input), 0);
    EXPECT_INT("keyboard_buffer_command_queued", (int)pipeline.commandQueue.count, 1);

    EXPECT_INT("keyboard_buffer_process_tick",
        DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
            &pipeline, &dungeon, NULL, &party, NULL, &result),
        1);
    EXPECT_INT("keyboard_buffer_route_command_c003", result.core.queue.command, DM1_V1_COMMAND_MOVE_FORWARD);
    EXPECT_INT("keyboard_buffer_route_dequeued", result.core.queue.dequeued, 1);
    EXPECT_INT("keyboard_buffer_route_dispatched_move", result.core.queue.dispatchedMove, 1);
    EXPECT_INT("keyboard_buffer_route_step_applied", result.core.stepApplied, 1);
    EXPECT_INT("keyboard_buffer_route_party_x", party.mapX, 5);
    EXPECT_INT("keyboard_buffer_route_party_y", party.mapY, 4);
    EXPECT_INT("keyboard_buffer_route_first_redraw_core", result.core.viewportRedrawRequested, 1);
    EXPECT_INT("keyboard_buffer_route_first_redraw_pipeline", result.viewportDirty, 1);
}

/* ---- Test: mouse click movement command ---- */
static void test_mouse_movement(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[10 * 10];
    struct PartyState_Compat party;
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;
    struct Dm1V1InputEventPc34Compat ev;

    setup_dungeon(&dungeon, &map, &tiles, squares, 10, 10);
    setup_party(&party, 5, 5, DIR_NORTH, 1);
    DM1_V1_MovementPipeline_InitPc34Compat(&pipeline);

    /* Click in the forward movement button box (263-289, 125-145) */
    memset(&ev, 0, sizeof(ev));
    ev.kind = DM1_V1_INPUT_KIND_MOUSE;
    ev.x = 276;
    ev.y = 135;
    ev.buttonMask = DM1_V1_BUTTON_LEFT;
    DM1_V1_MovementPipeline_EnqueueInputPc34Compat(&pipeline, ev);

    DM1_V1_MovementPipeline_ProcessOneTickPc34Compat(
        &pipeline, &dungeon, NULL, &party, NULL, &result);
    EXPECT("mouse_fwd_step", result.core.stepApplied == 1);
    EXPECT_INT("mouse_fwd_y", party.mapY, 4);
}

int main(void)
{
    printf("DM1 V1 Movement Pipeline Integration Tests\n");
    printf("Source: MOVESENS.C, COMMAND.C, CLIKMENU.C, CHAMPION.C, GAMELOOP.C\n\n");

    test_keyboard_forward_step();
    test_keyboard_turn_left();
    test_wall_blocks_movement();
    test_door_passability();
    test_movement_cooldown_gate();
    test_queue_sequencing();
    test_empty_queue();
    test_bounds_check();
    test_source_evidence();
    test_command_movement_viewport_provenance();
    test_backward_step();
    test_strafe();
    test_stairs_step_consequence();
    test_command_movement_viewport_wall_order_source_lock();
    test_post_move_environment_side_effects();
    test_direct_command_wrapper_forward_step();
    test_original_keyboard_buffer_forward_route_to_first_redraw();
    test_mouse_movement();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
