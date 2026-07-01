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

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static void seed_active_view(M11_GameViewState* state)
{
    struct ChampionState_Compat* champion;
    int i;

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
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champion->inventory[i] = THING_NONE;
    }
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

static void test_c161_reincarnate_opens_rename_modal(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.candidateMirrorOrdinal = 0;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    memcpy(state.world.party.champions[0].name, "OLD", 4U);
    memcpy(state.world.party.champions[0].title, "TITLE", 6U);

    result = M11_GameView_HandlePointer(&state, 180, 115, 1);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "C161 click opens reincarnate rename modal");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "candidate panel remains live while F0281 rename is active");
    ASSERT_EQ(state.candidateMirrorRenameActive, 1,
              "F0281 rename state is active after C161");
    ASSERT_EQ(state.candidateMirrorRename.fieldMode,
              DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT,
              "rename starts in name field");
    ASSERT_EQ(state.world.party.champions[0].name[0], '\0',
              "C161 F0281 clears champion name before input");
    ASSERT_EQ(state.world.party.champions[0].title[0], '\0',
              "C161 F0281 clears champion title before input");
    assert_no_pipeline_activity(&state, tick, direction,
                                "rename modal blocks gameplay queue");
}

static void test_c161_rename_accept_finishes_reincarnation(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);
    state.candidateMirrorOrdinal = 0;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    state.world.party.champions[0].hp.current = 80;
    state.world.party.champions[0].hp.maximum = 80;
    state.world.party.champions[0].stamina.current = 60;
    state.world.party.champions[0].stamina.maximum = 60;
    state.world.party.champions[0].mana.current = 40;
    state.world.party.champions[0].mana.maximum = 40;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 180, 115, 1),
              M11_GAME_INPUT_REDRAW,
              "C161 opens rename before accepting reincarnation");
    ASSERT_EQ(M11_GameView_HandleMirrorCandidateRenameClick(&state, 108, 116),
              1, "rename character-grid click enters A");
    ASSERT_EQ(M11_GameView_ApplyMirrorCandidateRenameAscii(&state, '\r'),
              1, "rename return moves from name to title");
    result = M11_GameView_HandleMirrorCandidateRenameClick(&state, 205, 150);

    ASSERT_EQ(result, 1, "rename OK click accepted");
    ASSERT_EQ(state.candidateMirrorRenameActive, 0,
              "rename state clears after OK");
    ASSERT_EQ(state.candidateMirrorPanelActive, 0,
              "candidate panel closes after accepted rename");
    ASSERT_EQ(state.inventoryPanelActive, 0,
              "inventory panel closes after accepted reincarnation");
    ASSERT_EQ(state.world.party.champions[0].name[0] == 'A' &&
              state.world.party.champions[0].name[1] == '\0', 1,
              "accepted rename copies name to champion");
    ASSERT_EQ(state.world.party.champions[0].hp.maximum < 80, 1,
              "accepted rename then applies reincarnation vitals");
}

static void test_c161_rename_duplicate_name_keeps_modal_open(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);
    state.world.party.championCount = 2;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 80;
    state.world.party.champions[1].hp.maximum = 80;
    state.world.party.champions[1].stamina.current = 60;
    state.world.party.champions[1].stamina.maximum = 60;
    state.world.party.champions[1].mana.current = 40;
    state.world.party.champions[1].mana.maximum = 40;
    memcpy(state.world.party.champions[0].name, "A", 2U);
    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 1;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 180, 115, 1),
              M11_GAME_INPUT_REDRAW,
              "C161 opens duplicate-name rename modal");
    ASSERT_EQ(M11_GameView_ApplyMirrorCandidateRenameAscii(&state, 'A'),
              1, "duplicate-name test enters A");
    result = M11_GameView_HandleMirrorCandidateRenameClick(&state, 205, 150);

    ASSERT_EQ(result, 1, "duplicate-name OK redraws feedback");
    ASSERT_EQ(state.candidateMirrorRenameActive, 1,
              "duplicate name keeps rename state active");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "duplicate name keeps candidate panel active");
    ASSERT_EQ(state.candidateMirrorRename.okAccepted, 0,
              "duplicate name clears pending OK acceptance");
    ASSERT_EQ(state.candidateMirrorRename.duplicateNameRejectedCount, 1,
              "duplicate name rejection is counted");
    ASSERT_EQ(state.world.party.champions[1].name[0], '\0',
              "duplicate name is not copied to candidate");
    ASSERT_EQ(state.world.party.champions[1].hp.maximum, 80,
              "duplicate name does not apply reincarnation vitals");
}

static void test_candidate_panel_blocks_direct_spell_helpers(void)
{
    M11_GameViewState state;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.candidateMirrorOrdinal = 0;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 0,
              "C040 candidate blocks direct spell-panel open");
    ASSERT_EQ(state.spellPanelOpen, 0,
              "blocked spell open leaves panel closed");

    state.spellPanelOpen = 1;
    state.spellRuneRow = 1;
    state.spellBuffer.runeCount = 1;
    state.spellBuffer.runes[0] = 0x60;
    state.world.party.champions[0].mana.current = 50;
    state.world.party.champions[0].mana.maximum = 50;

    ASSERT_EQ(M11_GameView_EnterRune(&state, 5), 0,
              "C040 candidate blocks direct rune entry");
    ASSERT_EQ(state.spellBuffer.runeCount, 1,
              "blocked rune entry preserves rune count");
    ASSERT_EQ(state.spellBuffer.runes[0], 0x60,
              "blocked rune entry preserves existing rune");
    ASSERT_EQ(M11_GameView_ClearSpell(&state), 0,
              "C040 candidate blocks direct recant clear");
    ASSERT_EQ(state.spellBuffer.runeCount, 1,
              "blocked recant preserves rune count");
    ASSERT_EQ(M11_GameView_CastSpell(&state), 0,
              "C040 candidate blocks direct spell cast");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 50,
              "blocked cast preserves mana");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked spell helpers keep candidate panel live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct spell helpers do not tick");
}

static void test_candidate_panel_blocks_direct_inventory_toggle(void)
{
    M11_GameViewState state;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;
    state.inventorySelectedSlot = 7;

    ASSERT_EQ(M11_GameView_ToggleInventoryPanel(&state), 0,
              "C040 candidate blocks direct inventory toggle");
    ASSERT_EQ(state.inventoryPanelActive, 1,
              "blocked direct inventory toggle keeps panel open");
    ASSERT_EQ(state.inventorySelectedSlot, 7,
              "blocked direct inventory toggle preserves selection");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked direct inventory toggle keeps C040 live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct inventory helper does not tick");
}

static void test_candidate_panel_blocks_direct_object_helpers(void)
{
    M11_GameViewState state;
    struct ChampionState_Compat* champion;
    unsigned short handItem;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    champion = &state.world.party.champions[0];
    handItem = make_thing(THING_TYPE_JUNK, 0);
    champion->inventory[CHAMPION_SLOT_HAND_RIGHT] = handItem;
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    ASSERT_EQ(M11_GameView_PickupItem(&state), 0,
              "C040 candidate blocks direct pickup helper");
    ASSERT_EQ(M11_GameView_DropItem(&state), 0,
              "C040 candidate blocks direct drop helper");
    ASSERT_EQ(M11_GameView_UseItem(&state), 0,
              "C040 candidate blocks direct use helper");
    ASSERT_EQ(champion->inventory[CHAMPION_SLOT_HAND_RIGHT], handItem,
              "blocked object helpers preserve hand item");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked object helpers keep C040 live");
    ASSERT_EQ(state.inventoryPanelActive, 1,
              "blocked object helpers keep inventory panel open");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct object helpers do not tick");
}

static void test_keyboard_positive_control_dispatches_without_overlay(void)
{
    /* v2.8.x: arrow Left/Right now mean strafe-left/strafe-right
     * (matches the original DM1 PC 3.4 convention; see also the
     * user's keyboard-mapping request).  TURN_LEFT / TURN_RIGHT
     * come from Home / End / Q / E / KP_4 / KP_6, validated
     * separately in test_keyboard_positive_control_dispatches_
     * turn_without_overlay below.  This case proves that an arrow
     * LEFT press drives the strafe pipeline (DM1_V1_COMMAND_MOVE_LEFT)
     * without an overlay, and the party moves to the left neighbour
     * square rather than rotating.
     *
     * Note: the SDL scancode layer in src/engine/main_loop_m11.c
     * translates SDLK_LEFT into M12_MENU_INPUT_STRAFE_LEFT, not
     * M12_MENU_INPUT_LEFT. M12_MENU_INPUT_LEFT retains its historical
     * turn-left semantics in the gameplay pipeline (so existing
     * probe code stays working unchanged). */
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_STRAFE_LEFT);

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

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "keyboard turn redraws without overlay");
    ASSERT_EQ(state.dm1V1MovementPipeline.commandQueue.count, 0,
              "keyboard turn queue drained");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.dequeued, 1,
              "keyboard turn dequeued");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.command,
              DM1_V1_COMMAND_TURN_LEFT, "keyboard turn command dispatched");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.turnApplied, 1,
              "keyboard turn applied");
    ASSERT_EQ(state.world.party.direction != 0, 1,
              "keyboard positive turn control changes direction");
}

static void test_mouse_positive_control_dispatches_without_overlay(void)
{
    /* v2.8.x: the menu arrow-click LEFT button (handled by
     * m11_dispatch_arrow_command at ReDMCSB COMMAND.C G0448 command
     * 1 / C068) now drives the strafe pipeline instead of the turn
     * pipeline, matching the new keyboard convention where arrow
     * keys mean strafe (the original DM1 PC 3.4 convention; see
     * also the user's keyboard-mapping request).  The party moves
     * to the left neighbour square rather than rotating. */
    M11_GameViewState state;
    M11_GameInputResult result;

    seed_active_view(&state);

    result = M11_GameView_HandlePointer(&state, 20, 170, 1);

    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "mouse strafe redraws without overlay");
    ASSERT_EQ(state.dm1V1MovementPipeline.commandQueue.count, 0,
              "mouse command queue drained");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.dequeued, 1,
              "mouse command dequeued");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.queue.command,
              DM1_V1_COMMAND_MOVE_LEFT, "mouse strafe command dispatched");
    ASSERT_EQ(state.lastDm1V1MovementPipelineResult.core.turnApplied, 0,
              "mouse strafe does NOT turn the party");
}

int main(void)
{
    printf("=== M11 Overlay Command Queue Block Regression ===\n");
    printf("no game data: synthetic M11_GameViewState only\n\n");

    test_dialog_overlay_blocks_keyboard_command();
    test_inventory_overlay_blocks_keyboard_command();
    test_map_overlay_blocks_mouse_command();
    test_c161_reincarnate_opens_rename_modal();
    test_c161_rename_accept_finishes_reincarnation();
    test_c161_rename_duplicate_name_keeps_modal_open();
    test_candidate_panel_blocks_direct_spell_helpers();
    test_candidate_panel_blocks_direct_inventory_toggle();
    test_candidate_panel_blocks_direct_object_helpers();
    test_keyboard_positive_control_dispatches_without_overlay();
    test_keyboard_positive_control_dispatches_turn_without_overlay();
    test_mouse_positive_control_dispatches_without_overlay();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
