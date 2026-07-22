/*
 * test_dm1_v1_f0128_scheduler_m11_wiring.c
 *
 * Live-wiring integration test: the M11 game view's DM1 dungeon-view
 * pass must feed the F0128 per-square scheduler bridge
 * (dm1_v1_f0128_per_square_scheduler_pc34_compat, source-locked per
 * ReDMCSB DUNVIEW.C F0128:8318-8561) the live sampled 19-square view,
 * verify the plan, and dispatch the F0115 content loop from the plan's
 * per-square spans only after a mounted PC34 source preflights every step.
 * The published receipt is compared against a plan built directly from the
 * same scene through the contract API and must stay zeroed for non-DM1/
 * inactive frames.
 *
 * Data-free fixture: it verifies the explicit no-fallback boundary when
 * GRAPHICS.DAT is not mounted. No pixels or substitute surfaces are used.
 */
#include "m11_game_view.h"
#include "dm1_v1_f0128_per_square_scheduler_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, long got, long want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%ld want=%ld anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %ld anchor=%s\n", id, want, anchor);
    }
}

#define TEST_MAP_W 11
#define TEST_MAP_H 11

static unsigned char square_for_test(int elementType, int attributes)
{
    return (unsigned char)(((elementType & 0x07) << 5) | (attributes & 0x1f));
}

typedef struct TestDungeonFixture {
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squareData[TEST_MAP_W * TEST_MAP_H];
    unsigned short squareFirstThings[TEST_MAP_W * TEST_MAP_H];
    struct DungeonThings_Compat things;
} TestDungeonFixture;

static void seed_fixture(TestDungeonFixture* fixture)
{
    int i;
    memset(fixture, 0, sizeof(*fixture));
    for (i = 0; i < TEST_MAP_W * TEST_MAP_H; ++i) {
        fixture->squareData[i] =
            square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
        fixture->squareFirstThings[i] = THING_ENDOFLIST;
    }
    fixture->dungeon.header.mapCount = 1;
    fixture->dungeon.maps = fixture->maps;
    fixture->dungeon.tiles = fixture->tiles;
    fixture->dungeon.tilesLoaded = 1;
    fixture->maps[0].width = TEST_MAP_W;
    fixture->maps[0].height = TEST_MAP_H;
    fixture->tiles[0].squareData = fixture->squareData;
    fixture->tiles[0].squareCount = TEST_MAP_W * TEST_MAP_H;
    fixture->things.loaded = 1;
    fixture->things.squareFirstThings = fixture->squareFirstThings;
    fixture->things.squareFirstThingCount = TEST_MAP_W * TEST_MAP_H;
}

static void seed_state(M11_GameViewState* state, TestDungeonFixture* fixture)
{
    memset(state, 0, sizeof(*state));
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->world.dungeon = &fixture->dungeon;
    state->world.things = &fixture->things;
    /* Party centered in the 11x11 map facing north so every F0128 view
     * square (D4..D0, L2..R2) samples inside the fixture. */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 5;
    state->world.party.mapY = 5;
    state->world.party.direction = 0;
    state->world.party.championCount = 1;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].stamina.current = 100;
    state->world.party.champions[0].stamina.maximum = 100;
    state->world.party.champions[0].food = 2048;
    state->world.party.champions[0].water = 2048;
    state->world.party.champions[0].actionIndex = 255;
    state->world.party.champions[0].name[0] = 'H';
    state->world.party.champions[0].name[1] = 'A';
    state->world.party.champions[0].name[2] = 'L';
    state->world.party.champions[0].name[3] = 'K';
}

/* Expected contract input for the fixture scene: all corridor except
 * the caller-patched D1C element. */
static void expected_corridor_input(
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT])
{
    int i;
    for (i = 0; i < DM1_V1_F0128_VIEW_SQUARE_COUNT; ++i) {
        squares[i].element = DM1_V1_F0128_ELEMENT_CORRIDOR;
        squares[i].pitOrTeleporterVisible = 0;
        squares[i].frontWallOrnamentIsAlcove = 0;
        squares[i].hasFloorOrnament = 0;
    }
}

static void test_inactive_frame_publishes_no_plan(void)
{
    M11_GameViewState state;
    M11_Dm1F0128PerSquareSchedulerReceipt receipt;
    unsigned char framebuffer[320 * 200];

    M11_GameView_Init(&state);
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
    expect_int("inactive.valid", receipt.valid, 0,
               "no DM1 frame -> no F0128 scheduler receipt");
    expect_int("inactive.plan_ready", receipt.planReady, 0,
               "no DM1 frame -> no plan");
    expect_int("inactive.step_count", receipt.stepCount, 0,
               "no DM1 frame -> empty receipt");
    M11_GameView_Shutdown(&state);
}

static void test_corridor_scene_matches_contract_plan(void)
{
    M11_GameViewState state;
    TestDungeonFixture fixture;
    M11_Dm1F0128PerSquareSchedulerReceipt receipt;
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 expected;
    unsigned char framebuffer[320 * 200];

    seed_fixture(&fixture);
    seed_state(&state, &fixture);
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);

    expected_corridor_input(squares);
    expect_int("corridor.expected_builds",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares,
                                                              &expected),
               1, "contract plan for the all-corridor fixture builds");
    expect_int("corridor.receipt_valid", receipt.valid, 1,
               "ReDMCSB DUNVIEW.C F0128:8318-8561 live plan per DM1 frame");
    expect_int("corridor.plan_ready", receipt.planReady, 1,
               "live plan builds and passes invariant re-verify");
    expect_int("corridor.plan_not_driven_without_source", receipt.planDrivenContentLoop, 0,
               "data-free fixture cannot enter the source-only content loop");
    expect_int("corridor.step_count", receipt.stepCount,
               expected.stepCount,
               "live plan step count matches the contract API plan");
    expect_int("corridor.schedule_hash", (long)receipt.scheduleHash,
               (long)expected.scheduleHash,
               "live plan receipt hash matches the contract API plan");
    expect_int("corridor.no_fallback_content_count", receipt.f0115ContentSquareCount,
               0, "unmounted source leaves no legacy F0115 content loop");
    M11_GameView_Shutdown(&state);
}

static void test_door_front_scene_matches_contract_plan(void)
{
    M11_GameViewState state;
    TestDungeonFixture fixture;
    M11_Dm1F0128PerSquareSchedulerReceipt receipt;
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 expected;
    unsigned char framebuffer[320 * 200];
    int start = -1;
    int count = -1;

    seed_fixture(&fixture);
    /* D1C = one tile north of the party.  Door orientation bit 0x08
     * (North-South) matches the north-facing party, so F0172's axis
     * rule makes this a C17 door-front view. */
    fixture.squareData[5 * TEST_MAP_H + 4] =
        square_for_test(DUNGEON_ELEMENT_DOOR, 0x08 | 0x04);
    seed_state(&state, &fixture);
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);

    expected_corridor_input(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element =
        DM1_V1_F0128_ELEMENT_DOOR_FRONT;
    expect_int("door.expected_builds",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares,
                                                              &expected),
               1, "contract plan for the D1C door-front fixture builds");
    expect_int("door.plan_ready", receipt.planReady, 1,
               "D1C door-front live plan builds and verifies");
    expect_int("door.plan_not_driven_without_source", receipt.planDrivenContentLoop, 0,
               "door scene cannot enter the source-only content loop without GRAPHICS.DAT");
    expect_int("door.step_count", receipt.stepCount, expected.stepCount,
               "D1C door-front live step count matches contract plan");
    expect_int("door.schedule_hash", (long)receipt.scheduleHash,
               (long)expected.scheduleHash,
               "D1C door-front live hash matches contract plan");
    expect_int("door.d1c_span",
               DM1_V1_F0128_PerSquareSchedulerSquareSpanPc34Compat(
                   &expected, DM1_V1_F0128_VIEW_SQUARE_D1C, &start, &count),
               1, "D1C door-front span resolves in the expected plan");
    expect_int("door.d1c_span_count", count, 4,
               "ReDMCSB DUNVIEW.C:7873-7937 D1C pass1/frame/door/pass2");
    expect_int("door.no_fallback_content_count", receipt.f0115ContentSquareCount, 0,
               "no legacy door content loop executes without source material");
    M11_GameView_Shutdown(&state);
}

static void test_wall_scene_gates_center_content(void)
{
    M11_GameViewState state;
    TestDungeonFixture fixture;
    M11_Dm1F0128PerSquareSchedulerReceipt receipt;
    DM1_V1_F0128SchedulerSquarePc34 squares[DM1_V1_F0128_VIEW_SQUARE_COUNT];
    DM1_V1_F0128SchedulerPlanPc34 expected;
    unsigned char framebuffer[320 * 200];

    seed_fixture(&fixture);
    /* D1C wall: F0124 runs wall material + F0107 predicates and returns
     * before any F0115 pass, so the plan admits no D1C thing layer. */
    fixture.squareData[5 * TEST_MAP_H + 4] =
        square_for_test(DUNGEON_ELEMENT_WALL, 0);
    seed_state(&state, &fixture);
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);

    expected_corridor_input(squares);
    squares[DM1_V1_F0128_VIEW_SQUARE_D1C].element =
        DM1_V1_F0128_ELEMENT_WALL;
    expect_int("wall.expected_builds",
               DM1_V1_F0128_PerSquareSchedulerBuildPc34Compat(squares,
                                                              &expected),
               1, "contract plan for the D1C wall fixture builds");
    expect_int("wall.plan_ready", receipt.planReady, 1,
               "D1C wall live plan builds and verifies");
    expect_int("wall.plan_not_driven_without_source", receipt.planDrivenContentLoop, 0,
               "wall scene cannot enter the source-only content loop without GRAPHICS.DAT");
    expect_int("wall.step_count", receipt.stepCount, expected.stepCount,
               "D1C wall live step count matches contract plan");
    expect_int("wall.schedule_hash", (long)receipt.scheduleHash,
               (long)expected.scheduleHash,
               "D1C wall live hash matches contract plan");
    expect_int("wall.no_fallback_content_count", receipt.f0115ContentSquareCount, 0,
               "unmounted source executes no legacy content loop");
    M11_GameView_Shutdown(&state);
}

int main(void)
{
    DM1_V1_F0128_PerSquareSchedulerInitPc34Compat();
    test_inactive_frame_publishes_no_plan();
    test_corridor_scene_matches_contract_plan();
    test_door_front_scene_matches_contract_plan();
    test_wall_scene_gates_center_content();

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
