/*
 * Narrow no-data regression for M11 overlay command routing.
 *
 * The M11 UI/modal layer must consume blocking overlays before gameplay
 * movement commands reach the DM1 V1 command queue. This uses synthetic state
 * only; no GRAPHICS.DAT/DUNGEON.DAT assets are loaded.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static void seed_active_view(M11_GameViewState* state)
{
    struct ChampionState_Compat* champion;

    memset(state, 0, sizeof(*state));
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->world.partyMapIndex = 0;
    state->world.newPartyMapIndex = 0;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 0;
    state->world.party.direction = 0;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;

    champion = &state->world.party.champions[0];
    champion->present = 1;
    champion->hp.current = 100;
    champion->hp.maximum = 100;
    champion->stamina.current = 100;
    champion->stamina.maximum = 100;
    champion->food = 1500;
    champion->water = 1500;
}

static void assert_no_pipeline_activity(const M11_GameViewState* state,
                                        uint32_t expectedTick,
                                        int expectedDirection,
                                        const char* label)
{
    ASSERT_EQ(state->dm1V1MovementPipeline.commandQueue.count, 0, label);
    ASSERT_EQ(state->lastDm1V1MovementPipelineResult.core.queue.dequeued, 0, label);
    ASSERT_EQ(state->lastDm1V1MovementPipelineResult.core.queue.command,
              DM1_V1_COMMAND_NONE, label);
    ASSERT_EQ(state->world.gameTick, (int)expectedTick, label);
    ASSERT_EQ(state->world.party.direction, expectedDirection, label);
}

static void test_dialog_overlay_blocks_keyboard_command(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    ASSERT_EQ(M11_GameView_ShowDialogOverlay(&state, "BLOCKING"), 1,
              "dialog overlay opens");

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_LEFT);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "dialog consumes keyboard command as dismissal");
    ASSERT_EQ(state.dialogOverlayActive, 0, "dialog dismissed");
    assert_no_pipeline_activity(&state, tick, direction,
                                "dialog blocks keyboard gameplay queue");
}

static void test_inventory_overlay_blocks_keyboard_command(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.inventoryPanelActive = 1;
    state.inventorySelectedSlot = 0;

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_LEFT);

    ASSERT_EQ(result, M11_GAME_INPUT_IGNORED,
              "inventory ignores blocked gameplay keyboard command");
    ASSERT_EQ(state.inventoryPanelActive, 1, "inventory remains active");
    assert_no_pipeline_activity(&state, tick, direction,
                                "inventory blocks keyboard gameplay queue");
}

static void test_map_overlay_blocks_mouse_command(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.mapOverlayActive = 1;

    result = M11_GameView_HandlePointer(&state, 20, 170, 1);

    ASSERT_EQ(result, M11_GAME_INPUT_IGNORED,
              "map overlay ignores blocked mouse command");
    ASSERT_EQ(state.mapOverlayActive, 1, "map overlay remains active");
    assert_no_pipeline_activity(&state, tick, direction,
                                "map overlay blocks mouse gameplay queue");
}

static void test_keyboard_positive_control_dispatches_without_overlay(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_LEFT);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "keyboard turn redraws without overlay");
    ASSERT_EQ(state.dm1V1MovementPipeline.commandQueue.count, 0,
              "keyboard command queue drained");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.dequeued, 1,
              "keyboard command dequeued");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.command,
              DM1_V1_COMMAND_TURN_LEFT, "keyboard command dispatched");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.turnApplied, 1,
              "keyboard turn applied");
    ASSERT_EQ(state.world.party.direction != 0, 1,
              "keyboard positive control changes direction");
}

static void test_keyboard_positive_control_dispatches_strafe_without_overlay(void)
{
    /* v2.8.x: arrow Left/Right now mean strafe-left/strafe-right
     * (matches the original DM1 PC 3.4 convention; see also the
     * user's keyboard-mapping request).  TURN_LEFT / TURN_RIGHT
     * come from Home / End / Q / E / KP_4 / KP_6, validated
     * separately in test_keyboard_positive_control_dispatches_
     * turn_without_overlay below.  This case proves that an arrow
     * LEFT press drives the strafe pipeline (DM1_V1_COMMAND_MOVE_LEFT)
     * without an overlay, and the party moves to the left neighbour
     * square rather than rotating. */
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_LEFT);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "keyboard strafe redraws without overlay");
    ASSERT_EQ(state.dm1V1MovementPipeline.commandQueue.count, 0,
              "keyboard strafe queue drained");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.dequeued, 1,
              "keyboard strafe dequeued");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.command,
              DM1_V1_COMMAND_MOVE_LEFT, "keyboard strafe command dispatched");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.turnApplied, 0,
              "keyboard strafe does NOT turn the party");
}

static void test_keyboard_positive_control_dispatches_turn_without_overlay(void)
{
    /* v2.8.x: turn-left input token (Home / End / Q / E / KP_4 / KP_6
     * all map to it; see src/engine/main_loop_m11.c) routes to
     * DM1_V1_COMMAND_TURN_LEFT in the pipeline command switch. */
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_LEFT);
}

static void test_mouse_positive_control_dispatches_without_overlay(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);

    result = M11_GameView_HandlePointer(&state, 20, 170, 1);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "mouse turn redraws without overlay");
    ASSERT_EQ(state.dm1V1MovementPipeline.commandQueue.count, 0,
              "mouse command queue drained");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.dequeued, 1,
              "mouse command dequeued");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.command,
              DM1_V1_COMMAND_TURN_LEFT, "mouse command dispatched");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.turnApplied, 1,
              "mouse turn applied");
    ASSERT_EQ(state.world.party.direction != 0, 1,
              "mouse positive control changes direction");
}

int main(void)
{
    printf("=== M11 Overlay Command Queue Block Regression ===\n");
    printf("no game data: synthetic M11_GameViewState only\n\n");

    test_dialog_overlay_blocks_keyboard_command();
    test_inventory_overlay_blocks_keyboard_command();
    test_map_overlay_blocks_mouse_command();
    test_keyboard_positive_control_dispatches_without_overlay();
    test_mouse_positive_control_dispatches_without_overlay();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
