#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dm1_v1_movement_pipeline_pc34_compat.h"

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
    struct Dm1V1MovementPipelinePc34Compat pipeline;
    struct Dm1V1MovementPipelineResultPc34Compat result;

    setup_two_level_stairs_dungeon(&dungeon, maps, tiles, level0, level1);

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
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT("stairs_into_transition", result.core.stairTransitionApplied == 1);
    EXPECT("stairs_into_no_regular_step", result.core.stepApplied == 0);
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
        &pipeline, &dungeon, NULL, &party, NULL, &result);

    EXPECT("stairs_backward_transition", result.core.stairTransitionApplied == 1);
    EXPECT_INT("stairs_backward_map", party.mapIndex, 1);
    EXPECT_INT("stairs_backward_x_not_south", party.mapX, 2);
    EXPECT_INT("stairs_backward_y_not_south", party.mapY, 2);
    EXPECT_INT("stairs_backward_no_cooldown", pipeline.disabledMovementTicks, 0);
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
    test_mouse_movement();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
