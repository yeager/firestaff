/*
 * Narrow no-data regression for M11 overlay command routing.
 *
 * The M11 UI/modal layer must consume blocking overlays before gameplay
 * movement commands reach the DM1 V1 command queue. This uses synthetic state
 * only; no GRAPHICS.DAT/DUNGEON.DAT assets are loaded.
 */

#include "m11_game_view.h"
#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

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

static int build_dm1_hoc_render_consumer_receipt_for_test(
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34* out)
{
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 postPlan;
    DM1_V1_StartupHandoffOutcome_PC34 outcome;
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 firstFrame;
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 frontWall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 boundary;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 floorThing;
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 thingConsumer;
    const DM1V1D1LD1RF0115LanePc34Data* lane;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!dm1_v1_startup_handoff_post_launch_plan_pc34("dm1", &postPlan) ||
        !dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            ENTRANCE_COMPAT_COMMAND_PATH_ENTER, &outcome) ||
        !dm1_v1_startup_hoc_first_frame_receipt_pc34(
            "dm1", &postPlan, &outcome, &firstFrame)) {
        return 0;
    }
    lane = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    if (!lane ||
        !DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 2, 2, &frontWall) ||
        !DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
            1, &frontWall, &render) ||
        !DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
            &render, &boundary) ||
        !dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
            lane, 5, 1, 1, 0, &floorThing) ||
        !DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
            &boundary, &floorThing, &thingConsumer) ||
        !dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
            &firstFrame, &thingConsumer, out)) {
        return 0;
    }
    return 1;
}

#define DM_PC_COLOR_BLACK 0
#define DM_PC_COLOR_CYAN 4
#define DM_PC_COLOR_YELLOW 11

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

/* ReDMCSB DUNVIEW.C F0128 draws the center lane D3C, D2C, D1C, while
 * Firestaff's receipt takes the party-near D1..D3 inputs.  Keep the synthetic
 * dungeon sampling local to this test; production M11 must consume the DM1
 * source-locked receipt rather than exporting another diagnostic adapter. */
static int test_dm1_center_lane_visibility(
    const M11_GameViewState* state,
    DM1_ViewportLaneVisibilityReceiptPc34* out,
    int center_x[3],
    int center_y[3],
    int center_element[3])
{
    static const int forward_x[4] = { 0, 1, 0, -1 };
    static const int forward_y[4] = { -1, 0, 1, 0 };
    const struct DungeonDatState_Compat* dungeon;
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int valid[3] = { 0, 0, 0 };
    int open[3] = { 0, 0, 0 };
    int door[3] = { 0, 0, 0 };
    int clear_side[3] = { 1, 1, 1 };
    int direction;
    int depth;

    if (!state || !out || !center_x || !center_y || !center_element) {
        return 0;
    }
    dungeon = state->world.dungeon;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->maps || !dungeon->tiles ||
        !dungeon->tiles->squareData || state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= dungeon->header.mapCount) {
        return 0;
    }
    map = &dungeon->maps[state->world.party.mapIndex];
    tiles = dungeon->tiles;
    direction = state->world.party.direction & 3;
    for (depth = 0; depth < 3; ++depth) {
        int x = state->world.party.mapX + forward_x[direction] * (depth + 1);
        int y = state->world.party.mapY + forward_y[direction] * (depth + 1);
        int index;
        int element;

        center_x[depth] = x;
        center_y[depth] = y;
        center_element[depth] = -1;
        if (x < 0 || y < 0 || x >= map->width || y >= map->height) {
            continue;
        }
        index = x * (int)map->height + y;
        if (index < 0 || index >= tiles->squareCount) {
            continue;
        }
        element = (tiles->squareData[index] >> 5) & 0x07;
        center_element[depth] = element;
        valid[depth] = 1;
        open[depth] = element != DUNGEON_ELEMENT_WALL;
        door[depth] = element == DUNGEON_ELEMENT_DOOR;
    }
    *out = dm1_viewport_3d_lane_visibility_from_cells_pc34(
        valid, open, door, clear_side, clear_side);
    return 1;
}

static int test_dm1_nearest_blocking_center_depth(
    const M11_GameViewState* state,
    int* out_depth,
    int* out_rel_forward,
    int* out_map_x,
    int* out_map_y,
    int* out_element)
{
    DM1_ViewportLaneVisibilityReceiptPc34 visibility;
    int center_x[3];
    int center_y[3];
    int center_element[3];
    int depth;

    if (!out_depth || !out_rel_forward || !out_map_x || !out_map_y ||
        !out_element || !test_dm1_center_lane_visibility(
            state, &visibility, center_x, center_y, center_element)) {
        return 0;
    }
    depth = visibility.nearest_blocking_center_depth_index;
    *out_depth = depth;
    *out_rel_forward = depth >= 0 ? depth + 1 : -1;
    *out_map_x = depth >= 0 ? center_x[depth] : -1;
    *out_map_y = depth >= 0 ? center_y[depth] : -1;
    *out_element = depth >= 0 ? center_element[depth] : -1;
    return 1;
}

static int test_dm1_center_content_visible_depth_mask(
    const M11_GameViewState* state,
    int* out_mask)
{
    DM1_ViewportLaneVisibilityReceiptPc34 visibility;
    int center_x[3];
    int center_y[3];
    int center_element[3];

    if (!out_mask || !test_dm1_center_lane_visibility(
            state, &visibility, center_x, center_y, center_element)) {
        return 0;
    }
    *out_mask = visibility.center_visible_depth_mask;
    return 1;
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

#if 0 /* Covered by DM1 resurrection-route tests, not command-queue routing. */
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

#endif

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
    ASSERT_EQ(M11_GameView_CloseSpellPanel(&state), 0,
              "C040 candidate blocks direct spell-panel close");
    ASSERT_EQ(state.spellPanelOpen, 1,
              "blocked spell close leaves panel open");
    ASSERT_EQ(state.spellRuneRow, 1,
              "blocked spell close preserves rune row");
    ASSERT_EQ(state.spellBuffer.runeCount, 1,
              "blocked spell close preserves rune count");
    ASSERT_EQ(state.spellBuffer.runes[0], 0x60,
              "blocked spell close preserves existing rune");
    ASSERT_EQ(M11_GameView_CastSpell(&state), 0,
              "C040 candidate blocks direct spell cast");
    ASSERT_EQ(state.world.party.champions[0].mana.current, 50,
              "blocked cast preserves mana");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked spell helpers keep candidate panel live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct spell helpers do not tick");
}

static void test_csb_cast_never_uses_dm1_spell_executor(void)
{
    M11_GameViewState state;
    uint32_t tick;
    unsigned short mana;

    seed_active_view(&state);
    state.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    state.spellPanelOpen = 1;
    state.spellRuneRow = 1;
    state.spellBuffer.runeCount = 2;
    state.spellBuffer.runes[0] = 0x60;
    state.spellBuffer.runes[1] = 0x61;
    state.world.party.champions[0].mana.current = 50;
    state.world.party.champions[0].mana.maximum = 50;
    tick = state.world.gameTick;
    mana = state.world.party.champions[0].mana.current;

    ASSERT_EQ(M11_GameView_CastSpell(&state), 0,
              "CSB cast is rejected without a CSB source executor");
    ASSERT_EQ(state.world.party.champions[0].mana.current, mana,
              "CSB cast rejection preserves champion mana");
    ASSERT_EQ(state.world.gameTick, tick,
              "CSB cast rejection does not run a DM1 tick");
    ASSERT_EQ(state.spellPanelOpen, 1,
              "CSB cast rejection preserves source spell panel");
    ASSERT_EQ(state.spellBuffer.runeCount, 2,
              "CSB cast rejection preserves source rune line");
    ASSERT_EQ(state.spellBuffer.runes[0], 0x60,
              "CSB cast rejection preserves first rune");
    ASSERT_EQ(state.spellBuffer.runes[1], 0x61,
              "CSB cast rejection preserves second rune");
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

static void test_candidate_panel_blocks_direct_map_toggle(void)
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

    ASSERT_EQ(M11_GameView_ToggleMapOverlay(&state), 0,
              "C040 candidate blocks direct map toggle");
    ASSERT_EQ(state.mapOverlayActive, 0,
              "blocked direct map toggle keeps map closed");
    ASSERT_EQ(state.inventoryPanelActive, 1,
              "blocked direct map toggle preserves candidate inventory panel");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked direct map toggle keeps C040 live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct map helper does not tick");
}

static void test_candidate_panel_uses_dm1_hoc_menu_route_receipt(void)
{
    M11_GameViewState state;
    DM1_V1_EntranceMenuRouteReceiptPc34 receipt;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;

    ASSERT_EQ(M11_GameView_GetDm1HocMenuRouteReceipt(&state, &receipt),
              1, "M11 exposes DM1 HoC route receipt");
    ASSERT_EQ(receipt.route, DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34,
              "normal HoC route starts in hall");
    ASSERT_EQ(receipt.showChampionPanel, 0,
              "hall route does not show champion panel");
    ASSERT_EQ(receipt.canEnterDungeon, 1,
              "hall route can enter when party has a champion");

    state.candidateMirrorOrdinal = 7;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    ASSERT_EQ(M11_GameView_GetDm1HocMenuRouteReceipt(&state, &receipt),
              1, "M11 maps live C040 state into DM1 HoC receipt");
    ASSERT_EQ(receipt.route, DM1_V1_ENTRANCE_MENU_ROUTE_LIVE_CHAMPION_PC34,
              "candidate panel uses live champion route");
    ASSERT_EQ(receipt.selectedChampionIndex, 7,
              "receipt carries selected mirror/champion ordinal");
    ASSERT_EQ(receipt.showChampionPanel, 1,
              "receipt marks champion panel as input owner");
    ASSERT_EQ(receipt.canCancelSelection, 1,
              "receipt exposes cancel route for C162");

    ASSERT_EQ(M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP),
              M11_GAME_INPUT_IGNORED,
              "DM1 HoC receipt blocks keyboard gameplay input");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 20, 170, 1),
              M11_GAME_INPUT_IGNORED,
              "DM1 HoC receipt blocks normal pointer gameplay input");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "receipt-routed input keeps C040 live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "receipt-routed C040 does not tick");
}

static void test_dm1_hoc_startup_render_consumer_is_m11_ready(void)
{
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34 consumer;

    ASSERT_EQ(build_dm1_hoc_render_consumer_receipt_for_test(&consumer),
              1,
              "DM1 exposes HoC startup render consumer receipt");
    ASSERT_EQ(consumer.ready, 1,
              "DM1 HoC startup render consumer is ready");
    ASSERT_EQ(consumer.consume_dm1_receipts_only, 1,
              "HoC startup render consumes DM1 receipts only");
    ASSERT_EQ(consumer.no_m11_fallback_scan, 1,
              "HoC startup render does not use M11 fallback scan");
    ASSERT_EQ(consumer.draw_opened_entrance_frame, 1,
              "DM1 receipt draws opened entrance frame");
    ASSERT_EQ(consumer.clear_champion_panel, 1,
              "DM1 receipt clears stale champion panel");
    ASSERT_EQ(consumer.render_hall_mirror_overlay, 1,
              "DM1 receipt renders Hall mirrors");
    ASSERT_EQ(consumer.draw_champion_mirror_wall_overlay, 1,
              "DM1 receipt draws champion mirror wall overlay");
    ASSERT_EQ(consumer.draw_real_floor_object, 1,
              "DM1 receipt allows real floor object");
    ASSERT_EQ(consumer.suppress_mirror_floor_item_payload, 1,
              "DM1 receipt suppresses mirror floor payload");
    ASSERT_EQ(consumer.suppress_mirror_projectile_payload, 1,
              "DM1 receipt suppresses mirror projectile payload");
    ASSERT_EQ(consumer.suppress_mirror_spell_effect_payload, 1,
              "DM1 receipt suppresses mirror spell payload");
    ASSERT_EQ(consumer.map_index, DM1_V1_ENTRANCE_MAP_INDEX_PC34,
              "DM1 receipt carries HoC entrance map");
    ASSERT_EQ(consumer.entrance_door_frame_index, 9,
              "DM1 receipt carries opened door frame");
    ASSERT_EQ(consumer.hall_overlay_kind,
              DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34,
              "DM1 receipt carries Hall mirror overlay kind");
    ASSERT_EQ(consumer.render_command_count, 3,
              "DM1 receipt carries ordered HoC render commands");
}

#if 0 /* Covered by CSB-owned startup receipt tests; M11 probe was retired. */
static void test_csb_startup_host_view_draw_receipt_is_m11_ready(void)
{
    int titleReceiptReady = 0;
    int titleDrawExecuted = 0;
    int titleHudExecuted = -1;
    int closedDoorReceiptReady = 0;
    int closedDoorDrawExecuted = 0;
    int closedDoorHudExecuted = 0;
    int utilityReceiptReady = 0;
    int utilityDrawExecuted = 0;
    int utilityHudExecuted = 0;
    int openingReceiptReady = 0;
    int openingDrawExecuted = 0;
    int consumedHostViewOnly = 0;
    int suppressLegacyUtilityFallback = 0;
    int packagedVisualCaptureReady = 0;
    int inputConsumesReceiptOnly = 0;
    int utilityInputDispatchReady = 0;
    int titleAssetDrawReady = 0;
    int closedDoorFallbackSuppressed = 0;
    int openingFrameDrawReady = 0;
    int fullVisualSequenceConsumed = 0;
    int runtimeRouteHardeningReady = 0;
    int runtimeRouteHardeningHashReady = 0;
    int runtimeHostCaptureGateReady = 0;
    int runtimeHostCaptureGateHashReady = 0;
    int titleStageRuntimeCaptureReady = 0;
    int titleStageRuntimeCaptureHashReady = 0;

    ASSERT_EQ(M11_GameView_ProbeCsbStartupHostViewDrawConsumerReceipt(
                  &titleReceiptReady,
                  &titleDrawExecuted,
                  &titleHudExecuted,
                  &closedDoorReceiptReady,
                  &closedDoorDrawExecuted,
                  &closedDoorHudExecuted,
                  &utilityReceiptReady,
                  &utilityDrawExecuted,
                  &utilityHudExecuted,
                  &openingReceiptReady,
                  &openingDrawExecuted,
                  &consumedHostViewOnly,
                  &suppressLegacyUtilityFallback,
                  &packagedVisualCaptureReady,
                  &inputConsumesReceiptOnly,
                  &utilityInputDispatchReady,
                  &titleAssetDrawReady,
                  &closedDoorFallbackSuppressed,
                  &openingFrameDrawReady,
                  &fullVisualSequenceConsumed,
                  &runtimeRouteHardeningReady,
                  &runtimeRouteHardeningHashReady,
                  &runtimeHostCaptureGateReady,
                  &runtimeHostCaptureGateHashReady,
                  &titleStageRuntimeCaptureReady,
                  &titleStageRuntimeCaptureHashReady),
              1,
              "M11 exposes CSB startup host-view draw receipt");
    ASSERT_EQ(titleReceiptReady, 1,
              "CSB title receipt is ready");
    ASSERT_EQ(titleDrawExecuted, 1,
              "CSB title draw executes through host-view receipt");
    ASSERT_EQ(titleHudExecuted, 0,
              "CSB title blocks HUD/menu draw while PRESENTS is active");
    ASSERT_EQ(closedDoorReceiptReady, 1,
              "CSB closed-door receipt is ready");
    ASSERT_EQ(closedDoorDrawExecuted, 1,
              "CSB closed-door draw executes through host-view receipt");
    ASSERT_EQ(closedDoorHudExecuted, 1,
              "CSB closed-door HUD/menu executes through receipt");
    ASSERT_EQ(utilityReceiptReady, 1,
              "CSB utility receipt is ready");
    ASSERT_EQ(utilityDrawExecuted, 1,
              "CSB utility startup draw executes through host-view receipt");
    ASSERT_EQ(utilityHudExecuted, 1,
              "CSB utility HUD/menu executes through receipt");
    ASSERT_EQ(openingReceiptReady, 1,
              "CSB opening receipt is ready");
    ASSERT_EQ(openingDrawExecuted, 1,
              "CSB door-opening draw executes through host-view receipt");
    ASSERT_EQ(consumedHostViewOnly, 1,
              "M11 CSB startup draw consumes host-view receipt only");
    ASSERT_EQ(suppressLegacyUtilityFallback, 1,
              "CSB receipt suppresses legacy utility fallback");
    ASSERT_EQ(packagedVisualCaptureReady, 1,
              "CSB packaged visual capture proof feeds M11 draw");
    ASSERT_EQ(inputConsumesReceiptOnly, 1,
              "M11 CSB startup input consumes dispatch receipt only");
    ASSERT_EQ(utilityInputDispatchReady, 1,
              "CSB utility input dispatch redraws HUD/menu through receipt");
    ASSERT_EQ(titleAssetDrawReady, 1,
              "CSB title uses real title asset path without fallback text");
    ASSERT_EQ(closedDoorFallbackSuppressed, 1,
              "CSB closed-door/menu receipt blocks fallback text path");
    ASSERT_EQ(openingFrameDrawReady, 1,
              "CSB door opening uses receipt-owned frame draw");
    ASSERT_EQ(fullVisualSequenceConsumed, 1,
              "M11 CSB startup requires the full title/HUD/door visual sequence receipt");
    ASSERT_EQ(runtimeRouteHardeningReady, 1,
              "M11 CSB startup routes require runtime route hardening");
    ASSERT_EQ(runtimeRouteHardeningHashReady, 1,
              "M11 CSB startup route hardening publishes route hashes");
    ASSERT_EQ(runtimeHostCaptureGateReady, 1,
              "M11 CSB startup requires full real-data runtime host capture gate");
    ASSERT_EQ(runtimeHostCaptureGateHashReady, 1,
              "M11 CSB startup runtime host capture gate publishes route and asset hashes");
    ASSERT_EQ(titleStageRuntimeCaptureReady, 1,
              "M11 CSB startup consumes PRESENTS, CHAOS zoom, CHAOS hold and STRIKES BACK runtime captures");
    ASSERT_EQ(titleStageRuntimeCaptureHashReady, 1,
              "M11 CSB startup title stage captures match packaged source hashes");
}

#endif

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

#if 0 /* The retired chest-close diagnostic is covered by DM1 chest tests. */
static void test_candidate_panel_blocks_direct_leader_hand_chest_helpers(void)
{
    M11_GameViewState state;
    unsigned short handItem;
    unsigned short chestThing;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    handItem = make_thing(THING_TYPE_JUNK, 0);
    chestThing = make_thing(THING_TYPE_CONTAINER, 0);
    state.inventoryPanelActive = 1;
    state.v1OpenChestThing = chestThing;
    state.v1OpenChestOpenedByEye = 1;
    tick = state.world.gameTick;
    direction = state.world.party.direction;

    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, handItem), 0,
              "C040 candidate blocks direct leader-hand set helper");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "blocked leader-hand set leaves hand empty");

    state.leaderHandObjectPresent = 1;
    state.leaderHandThing = handItem;
    state.leaderHandIconIndex = 7;
    M11_GameView_ClearV1LeaderHandObject(&state);
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), handItem,
              "C040 candidate blocks direct leader-hand clear helper");
    ASSERT_EQ(state.leaderHandIconIndex, 7,
              "blocked leader-hand clear preserves icon metadata");

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        chestThing;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 0,
              "C040 candidate blocks direct action-hand chest open helper");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "blocked chest open preserves existing C040-covered chest");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "C040 candidate blocks direct chest close helper");
    ASSERT_EQ(state.v1OpenChestOpenedByEye, 1,
              "blocked chest close preserves pressing-eye chest metadata");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked leader-hand/chest helpers keep C040 live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct leader-hand/chest helpers do not tick");
}

#endif

static void test_candidate_panel_blocks_direct_quickload_only(void)
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
    snprintf(state.inspectTitle, sizeof(state.inspectTitle), "KEEP");
    snprintf(state.inspectDetail, sizeof(state.inspectDetail), "UNCHANGED");

    ASSERT_EQ(M11_GameView_QuickLoad(&state), 0,
              "C040 candidate blocks direct quickload helper");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked quickload keeps C040 live");
    ASSERT_EQ(state.inventoryPanelActive, 1,
              "blocked quickload preserves candidate inventory panel");
    ASSERT_EQ(state.inspectTitle[0] == 'K' && state.inspectDetail[0] == 'U',
              1,
              "blocked quickload returns before path/status mutation");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 direct quickload helper does not tick");
}

static void test_candidate_panel_blocks_rest_and_source_save_commands(void)
{
    M11_GameViewState state;
    M11_GameInputResult result;
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;
    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;
    state.inventoryPanelActive = 1;

    /* ReDMCSB COMMAND.C F0380 lines 2340-2372 / BUG0_53:
     * C145 REST and C140 SAVE are ignored while
     * G0299_ui_CandidateChampionOrdinal owns the C040 candidate panel. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_REST_TOGGLE);
    ASSERT_EQ(result, M11_GAME_INPUT_IGNORED,
              "C040 candidate blocks source REST command");
    ASSERT_EQ(state.resting, 0,
              "blocked C145 REST does not enter party-resting state");
    ASSERT_EQ(state.world.partyIsResting, 0,
              "blocked C145 REST leaves source party-resting mirror clear");
    ASSERT_EQ(state.world.lifecycle.rest.isResting, 0,
              "blocked C145 REST leaves lifecycle rest state clear");

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_SAVE_GAME);
    ASSERT_EQ(result, M11_GAME_INPUT_IGNORED,
              "C040 candidate blocks source save command");
    ASSERT_EQ(state.lastSaveTick, 0,
              "blocked C140 SAVE does not mutate last-save tick");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "blocked rest/save keep C040 live");
    ASSERT_EQ(state.inventoryPanelActive, 1,
              "blocked rest/save preserve candidate inventory panel");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 rest/save commands do not tick");
}

static void test_candidate_panel_hides_stale_action_rows(void)
{
    M11_GameViewState state;
    unsigned char actions[3] = {0, 0, 0};
    uint32_t tick;
    int direction;

    seed_active_view(&state);
    tick = state.world.gameTick;
    direction = state.world.party.direction;

    ASSERT_EQ(M11_GameView_SetActingChampion(&state, 0), 1,
              "fixture opens action menu before C040 takeover");
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 1,
              "fixture resolves action rows before C040 takeover");

    state.candidateMirrorOrdinal = 1;
    state.candidateMirrorPartyIndex = 0;
    state.candidateMirrorPanelActive = 1;

    /* ReDMCSB MENU.C F0390 lines 751-754 clears G0506 when
     * G0299_ui_CandidateChampionOrdinal owns C040.  The read-side helper
     * must not expose stale rows while the candidate panel owns input. */
    ASSERT_EQ(M11_GameView_GetActingActionIndices(&state, actions), 0,
              "C040 candidate hides stale action rows before trigger");
    ASSERT_EQ(state.candidateMirrorPanelActive, 1,
              "read-side action-row query keeps C040 live");
    assert_no_pipeline_activity(&state, tick, direction,
                                "C040 action-row query does not tick");
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

#if 0 /* Arrow-zone diagnostics are covered by DM1 input/presentation tests. */
static unsigned char framebuffer_pixel(const unsigned char* framebuffer,
                                       int x,
                                       int y)
{
    return framebuffer[y * 320 + x];
}

static void draw_and_expect_arrow_feedback(M11_GameViewState* state,
                                           int arrowIndex,
                                           int cueColor,
                                           const char* label)
{
    unsigned char framebuffer[320 * 200];
    int x = 0, y = 0, w = 0, h = 0;
    (void)w;
    (void)h;
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    ASSERT_EQ(M11_GameView_GetV1MovementArrowZone(arrowIndex,
                                                  &x, &y, &w, &h),
              1, label);
    ASSERT_EQ(framebuffer_pixel(framebuffer, x, y), cueColor, label);
    ASSERT_EQ(framebuffer_pixel(framebuffer, x + w - 1, y + h - 1),
              cueColor, label);
}

static void test_keyboard_navigation_visually_marks_screen_arrows(void)
{
    unsigned char framebuffer[320 * 200];
    M11_GameViewState state;
    M11_GameInputResult result;
    int x = 0, y = 0, w = 0, h = 0;

    seed_active_view(&state);
    ASSERT_EQ(M11_GameView_GetV1MovementArrowZone(2, &x, &y, &w, &h),
              1, "forward arrow zone exists");
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_EQ(framebuffer_pixel(framebuffer, x, y), DM_PC_COLOR_BLACK,
              "inactive V1 forward arrow has no keyboard feedback");

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "keyboard up redraws for visual arrow feedback");
    draw_and_expect_arrow_feedback(&state, 2, DM_PC_COLOR_CYAN,
                                   "keyboard up marks C070 forward arrow");

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_STRAFE_LEFT);
    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "keyboard left-arrow/WASD strafe redraws feedback");
    draw_and_expect_arrow_feedback(&state, 5, DM_PC_COLOR_CYAN,
                                   "keyboard strafe-left marks C073 left arrow");

    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_LEFT);
    ASSERT_EQ(result, M11_GAME_INPUT_REDRAW,
              "keyboard Q/Home turn redraws feedback");
    draw_and_expect_arrow_feedback(&state, 0, DM_PC_COLOR_YELLOW,
                                   "keyboard turn-left marks C068 turn arrow");
    /* C068's source hit strip is narrower than the visible C013 turn tile.
     * Keyboard/controller feedback must cover that complete tile while
     * retaining the inner source-zone outline. */
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_EQ(framebuffer_pixel(framebuffer, 234 + 27, 125),
              DM_PC_COLOR_YELLOW,
              "keyboard turn-left marks the full visible C013 button width");

    while (state.v1MovementArrowVisualTicks > 0) {
        M11_GameView_TickAnimation(&state);
    }
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_EQ(framebuffer_pixel(framebuffer, x, y), DM_PC_COLOR_BLACK,
              "V1 movement arrow feedback clears after tick countdown");
}

#endif

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

#if 0 /* Retired M11 viewport diagnostics are covered by DM1-owned receipts. */
static void test_static_dungeon_effects_do_not_render_as_viewport_fireballs(void)
{
    M11_GameViewState state;
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat projectiles[1];
    unsigned char squareData[1];
    unsigned short squareFirstThings[2];
    unsigned char projectileRaw[8];
    unsigned char explosionRaw[8];
    int projectileCount = -1;
    int explosionCount = -1;
    int firstProjectileGfx = -2;
    int firstExplosionType = -2;

    seed_active_view(&state);
    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(projectiles, 0, sizeof(projectiles));
    memset(projectileRaw, 0, sizeof(projectileRaw));
    memset(explosionRaw, 0, sizeof(explosionRaw));

    squareData[0] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    squareFirstThings[0] = make_thing(THING_TYPE_PROJECTILE, 0);
    projectileRaw[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    projectileRaw[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);
    explosionRaw[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    explosionRaw[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);

    map.width = 1;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = projectileRaw;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.projectiles = projectiles;
    things.projectileCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = explosionRaw;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    world.dungeon = &dungeon;
    world.things = &things;

    ASSERT_EQ(M11_GameView_CountCellProjectiles(&world, 0, 0, 0), 0,
              "static dungeon projectile thing is not a visible fireball");
    squareFirstThings[0] = make_thing(THING_TYPE_EXPLOSION, 0);
    ASSERT_EQ(M11_GameView_CountCellExplosions(&world, 0, 0, 0), 0,
              "static dungeon explosion thing is not a visible fireball");

    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 0;
    state.world.party.mapY = 0;
    state.world.party.direction = 0;
    ASSERT_EQ(M11_GameView_ProbeViewportArtifactCounts(
                  &state, 0, 0, NULL, NULL, NULL,
                  &projectileCount, &explosionCount,
                  &firstProjectileGfx, &firstExplosionType),
              1,
              "viewport artifact probe samples synthetic square");
    ASSERT_EQ(projectileCount, 0,
              "viewport sample suppresses static projectile count");
    ASSERT_EQ(explosionCount, 0,
              "viewport sample suppresses static explosion count");
    ASSERT_EQ(firstProjectileGfx, -1,
              "viewport sample exposes no drawable static projectile");
    ASSERT_EQ(firstExplosionType, -1,
              "viewport sample exposes no drawable static explosion");

    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;

    ASSERT_EQ(M11_GameView_CountCellProjectiles(&world, 0, 0, 0), 0,
              "inactive zeroed runtime projectile slot is not visible");
    world.projectiles.entries[0].reserved3 = 1;
    ASSERT_EQ(M11_GameView_CountCellProjectiles(&world, 0, 0, 0), 1,
              "runtime projectile remains visible in viewport summary");

    world.explosions.count = 1;
    world.explosions.entries[0].slotIndex = 0;
    world.explosions.entries[0].mapIndex = 0;
    world.explosions.entries[0].mapX = 0;
    world.explosions.entries[0].mapY = 0;

    ASSERT_EQ(M11_GameView_CountCellExplosions(&world, 0, 0, 0), 0,
              "inactive zeroed runtime explosion slot is not visible");
    world.explosions.entries[0].reserved0 = 1;
    ASSERT_EQ(M11_GameView_CountCellExplosions(&world, 0, 0, 0), 1,
              "runtime explosion remains visible in viewport summary");

    state.world.projectiles.count = 1;
    state.world.projectiles.entries[0].slotIndex = 0;
    state.world.projectiles.entries[0].mapIndex = 0;
    state.world.projectiles.entries[0].mapX = 0;
    state.world.projectiles.entries[0].mapY = 0;
    state.world.projectiles.entries[0].projectileSubtype =
        PROJECTILE_SUBTYPE_KINETIC_ARROW;
    state.world.explosions.count = 1;
    state.world.explosions.entries[0].slotIndex = 0;
    state.world.explosions.entries[0].mapIndex = 0;
    state.world.explosions.entries[0].mapX = 0;
    state.world.explosions.entries[0].mapY = 0;
    state.world.explosions.entries[0].explosionType = -1;
    projectileCount = -1;
    explosionCount = -1;
    firstProjectileGfx = -2;
    firstExplosionType = -2;
    ASSERT_EQ(M11_GameView_ProbeViewportArtifactCounts(
                  &state, 0, 0, NULL, NULL, NULL,
                  &projectileCount, &explosionCount,
                  &firstProjectileGfx, &firstExplosionType),
              1,
              "viewport artifact probe samples runtime effects");
    ASSERT_EQ(projectileCount, 0,
              "inactive zeroed runtime projectile slot is suppressed in viewport sample");
    ASSERT_EQ(firstProjectileGfx, -1,
              "inactive zeroed runtime projectile exposes no drawable graphic");
    ASSERT_EQ(explosionCount, 0,
              "inactive zeroed runtime explosion slot is suppressed in viewport sample");
    ASSERT_EQ(firstExplosionType, -1,
              "inactive zeroed runtime explosion exposes no drawable type");

    state.world.projectiles.entries[0].reserved3 = 1;
    state.world.explosions.entries[0].reserved0 = 1;
    squareFirstThings[0] = make_thing(THING_TYPE_PROJECTILE, 0);
    projectiles[0].slot = PROJECTILE_SUBTYPE_FIREBALL;
    projectileCount = -1;
    explosionCount = -1;
    firstProjectileGfx = -2;
    firstExplosionType = -2;
    ASSERT_EQ(M11_GameView_ProbeViewportArtifactCounts(
                  &state, 0, 0, NULL, NULL, NULL,
                  &projectileCount, &explosionCount,
                  &firstProjectileGfx, &firstExplosionType),
              1,
              "viewport artifact probe samples active runtime effects");
    ASSERT_EQ(projectileCount, 1,
              "runtime projectile with resolved graphic remains visible");
    ASSERT_EQ(firstProjectileGfx >= 0, 1,
              "runtime projectile exposes drawable graphic");
    ASSERT_EQ(firstProjectileGfx,
              dm1_v1_projectile_graphic_index(0, 0),
              "runtime projectile graphic wins over stale static fireball");
    ASSERT_EQ(firstProjectileGfx !=
                  dm1_v1_projectile_subtype_graphic_index(
                      PROJECTILE_SUBTYPE_FIREBALL),
              1,
              "stale static fireball graphic is not selected");
    ASSERT_EQ(explosionCount, 0,
              "runtime explosion without drawable type is suppressed");
    ASSERT_EQ(firstExplosionType, -1,
              "invalid runtime explosion type is not drawable");

    state.world.explosions.entries[0].explosionType = C000_EXPLOSION_FIREBALL;
    projectileCount = -1;
    explosionCount = -1;
    firstProjectileGfx = -2;
    firstExplosionType = -2;
    ASSERT_EQ(M11_GameView_ProbeViewportArtifactCounts(
                  &state, 0, 0, NULL, NULL, NULL,
                  &projectileCount, &explosionCount,
                  &firstProjectileGfx, &firstExplosionType),
              1,
              "viewport artifact probe samples active runtime explosion");
    ASSERT_EQ(explosionCount, 1,
              "active runtime explosion with drawable type remains visible");
    ASSERT_EQ(firstExplosionType, C000_EXPLOSION_FIREBALL,
              "active runtime explosion exposes drawable type");
}

static void test_hoc_floor_items_route_through_dm1_receipt(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    unsigned char squareData[1];
    unsigned short squareFirstThings[2];
    unsigned char weaponRaw[8];
    int floorItemCount = -1;
    int summaryItemCount = -1;
    int elementType = -1;

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(weaponRaw, 0, sizeof(weaponRaw));

    squareData[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = make_thing(THING_TYPE_WEAPON, 0);
    weaponRaw[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    weaponRaw[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);

    map.width = 1;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.rawThingData[THING_TYPE_WEAPON] = weaponRaw;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    state.world.dungeon = &dungeon;
    state.world.things = &things;

    ASSERT_EQ(M11_GameView_ProbeViewportFloorItemCounts(
                  &state, 0, 0, NULL, NULL, &elementType,
                  &floorItemCount, &summaryItemCount),
              1,
              "HoC floor item probe samples current square");
    ASSERT_EQ(elementType, DUNGEON_ELEMENT_CORRIDOR,
              "synthetic HoC square is a corridor");
    ASSERT_EQ(floorItemCount, 0,
              "HoC map-0 item route suppresses loose floor rendering");
    ASSERT_EQ(summaryItemCount, 0,
              "HoC map-0 summary follows DM1 item route receipt");
}

static void test_hoc_front_mirror_receipt_uses_render_index(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensors[1];
    unsigned char squareData[2];
    unsigned short squareFirstThings[2];
    unsigned char sensorRaw[8];
    int wallOrnament = -1;
    int portrait = -1;
    int elementType = -1;

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(sensors, 0, sizeof(sensors));
    memset(sensorRaw, 0, sizeof(sensorRaw));

    squareData[0] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    squareData[1] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    squareFirstThings[0] = make_thing_cell(THING_TYPE_SENSOR, 0, 2);
    squareFirstThings[1] = THING_ENDOFLIST;
    sensorRaw[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    sensorRaw[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);
    sensors[0].sensorType = 127;
    sensors[0].sensorData = 13;
    sensors[0].ornamentOrdinal = 4;

    map.width = 1;
    map.height = 2;
    tiles.squareData = squareData;
    tiles.squareCount = 2;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    things.rawThingData[THING_TYPE_SENSOR] = sensorRaw;
    things.thingCounts[THING_TYPE_SENSOR] = 1;
    things.sensors = sensors;
    things.sensorCount = 1;
    state.world.dungeon = &dungeon;
    state.world.things = &things;
    state.world.party.mapX = 0;
    state.world.party.mapY = 1;
    state.world.party.direction = 0;
    state.mirrorCatalogAvailable = 1;
    state.mirrorCatalog.count = 24;

    ASSERT_EQ(M11_GameView_ProbeViewportRenderMetadata(
                  &state, 1, 0, NULL, NULL, &elementType,
                  &wallOrnament, &portrait, NULL, NULL),
              1,
              "HoC front mirror probe samples front wall");
    ASSERT_EQ(elementType, DUNGEON_ELEMENT_WALL,
              "HoC front mirror square is wall");
    ASSERT_EQ(wallOrnament, 4,
              "HoC front mirror receipt carries mirror frame ornament");
    ASSERT_EQ(portrait, 13,
              "HoC front mirror uses zero-based C026 render index");
    ASSERT_EQ(M11_GameView_GetFrontMirrorOrdinal(&state), 13,
              "front mirror selection consumes DM1 C127 render receipt");
    {
        DM1_V1_ChampionMirrorFrontWallReceiptPc34 frontWall;
        DM1_V1_ChampionMirrorRenderReceiptPc34 renderReceipt;
        ASSERT_EQ(DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
                      127, 13, 4, 2, 2, &frontWall),
                  1,
                  "DM1 HoC test builds ReDMCSB C127 front-wall receipt");
        ASSERT_EQ(DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
                      1,
                      &frontWall, &renderReceipt),
                  1,
                  "DM1 HoC test consumes DM1-owned viewport render receipt directly");
        ASSERT_EQ(renderReceipt.drawChampionPortrait, 1,
                  "DM1 render receipt owns C026 portrait draw gate");
        ASSERT_EQ(renderReceipt.graphicIndex, 26,
                  "DM1 render receipt owns C026 graphic id");
        ASSERT_EQ(renderReceipt.sourceX, 160,
                  "DM1 render receipt owns ordinal-13 C026 source x");
        ASSERT_EQ(renderReceipt.sourceY, 29,
                  "DM1 render receipt owns ordinal-13 C026 source y");
        ASSERT_EQ(renderReceipt.width, 32,
                  "DM1 render receipt owns C026 width");
        ASSERT_EQ(renderReceipt.height, 29,
                  "DM1 render receipt owns C026 height");
        ASSERT_EQ(renderReceipt.dstX, 96,
                  "DM1 render receipt owns D1C portrait destination x");
        ASSERT_EQ(renderReceipt.dstY, 35,
                  "DM1 render receipt owns D1C portrait destination y");
        ASSERT_EQ(renderReceipt.transparentColor, 1,
                  "DM1 render receipt owns C026 transparent color");
        ASSERT_EQ(renderReceipt.consumedWallSquareReceipt, 1,
                  "DM1 consumed the wall-square receipt for C026 draw");
    }

    squareFirstThings[0] = make_thing_cell(THING_TYPE_SENSOR, 0, 1);
    portrait = -2;
    ASSERT_EQ(M11_GameView_ProbeViewportRenderMetadata(
                  &state, 1, 0, NULL, NULL, NULL,
                  NULL, &portrait, NULL, NULL),
              1,
              "HoC side mirror probe samples front wall");
    ASSERT_EQ(portrait, -1,
              "HoC side-cell C127 does not render a floating portrait");
    ASSERT_EQ(M11_GameView_GetFrontMirrorOrdinal(&state), -1,
              "side-cell C127 is rejected by DM1 render receipt before selection");
}

static void test_runtime_projectiles_use_f0115_c2900_raw_rows(void)
{
    int x = -1;
    int y = -1;
    int legacyX = -1;
    int legacyY = -1;

    /* ReDMCSB DUNVIEW.C F0115 lines 5648-5656 gates projectiles through
     * G0218_aaaauc_Graphic558_ObjectCoordinateSets[0], and line 5681
     * matches the projectile's source view cell before drawing.  For PC34
     * layout 696, D1C/D2C front cells live only in the raw C2900 rows; the
     * older five-row fallback has no front-cell coordinate and would draw a
     * synthetic cue instead of the source-positioned projectile. */
    ASSERT_EQ(M11_GameView_GetF0115C2500C2900Row(1, 0), 8,
              "D1C uses raw C2900 row 8");
    ASSERT_EQ(M11_GameView_GetProjectileRawZonePointForRel(1, 0, 0, &x, &y),
              1,
              "D1C front-left projectile has raw C2900 coordinate");
    ASSERT_EQ(x, 83, "D1C front-left projectile raw X");
    ASSERT_EQ(y, 47, "D1C front-left projectile raw Y");
    ASSERT_EQ(M11_GameView_GetC2900ProjectileZonePoint(
                  M11_GameView_GetObjectSourceScaleIndex(0, 0),
                  0,
                  &legacyX,
                  &legacyY),
              0,
              "legacy five-row projectile table has no D1C front-left coordinate");

    x = y = -1;
    ASSERT_EQ(M11_GameView_GetF0115C2500C2900Row(2, 0), 5,
              "D2C uses raw C2900 row 5");
    ASSERT_EQ(M11_GameView_GetProjectileRawZonePointForRel(2, 0, 0, &x, &y),
              1,
              "D2C front-left projectile has raw C2900 coordinate");
    ASSERT_EQ(x, 92, "D2C front-left projectile raw X");
    ASSERT_EQ(y, 47, "D2C front-left projectile raw Y");
}

static void test_runtime_floor_items_use_f0115_c2500_raw_rows(void)
{
    static const struct {
        int relForward;
        int relSide;
        int row;
        const char* label;
    } routes[] = {
        {1, -1,  9, "D1L"},
        {1,  0,  8, "D1C"},
        {1,  1, 10, "D1R"},
        {2, -1,  6, "D2L"},
        {2,  0,  5, "D2C"},
        {2,  1,  7, "D2R"},
        {3, -2,  3, "D3L2"},
        {3, -1,  1, "D3L"},
        {3,  0,  0, "D3C"},
        {3,  1,  2, "D3R"},
        {3,  2,  4, "D3R2"}
    };
    static const short expected[11][4][2] = {
        {{   0,   0}, {   0,   0}, { 127,  70}, {  98,  70}},
        {{   0,   0}, {   0,   0}, {  62,  70}, {  25,  70}},
        {{   0,   0}, {   0,   0}, { 200,  70}, { 162,  70}},
        {{   0,   0}, {   0,   0}, {   2,  70}, { -35,  70}},
        {{   0,   0}, {   0,   0}, { 258,  70}, { 222,  70}},
        {{  94,  78}, { 131,  78}, { 136,  88}, {  89,  88}},
        {{  10,  78}, {  53,  79}, {  41,  88}, { -14,  89}},
        {{ 171,  78}, { 218,  78}, { 236,  89}, { 184,  88}},
        {{  83,  99}, { 141,  99}, { 150, 115}, {  76, 115}},
        {{ -40, 101}, {  24,  99}, {   5, 114}, { -79, 117}},
        {{ 200,  99}, { 262, 101}, { 301, 117}, { 220, 114}}
    };
    size_t i;

    /* ReDMCSB DUNVIEW.C F0115 lines 4811, 4923, and 5075 select
     * G2028[viewSquare] and then C2500 + row*4 + viewCell.  The
     * renderer must use the raw layout-696 C2500 rows for every visible
     * D1/D2/D3 object route, not the older five-row fallback that has no
     * front-cell entries for near rows. */
    for (i = 0; i < sizeof(routes) / sizeof(routes[0]); ++i) {
        int cell;
        ASSERT_EQ(M11_GameView_GetF0115C2500C2900Row(
                      routes[i].relForward, routes[i].relSide),
                  routes[i].row,
                  routes[i].label);
        for (cell = 0; cell < 4; ++cell) {
            int x = -999;
            int y = -999;
            int wantPresent = expected[routes[i].row][cell][0] != 0 ||
                              expected[routes[i].row][cell][1] != 0;
            ASSERT_EQ(M11_GameView_GetC2500ObjectRawZonePoint(
                          routes[i].row, cell, &x, &y),
                      wantPresent,
                      routes[i].label);
            if (wantPresent) {
                ASSERT_EQ(x, expected[routes[i].row][cell][0], routes[i].label);
                ASSERT_EQ(y, expected[routes[i].row][cell][1], routes[i].label);
            }
        }
    }

    ASSERT_EQ(M11_GameView_GetF0115C2500C2900Row(2, -2), -1,
              "D2L2 must not borrow D2C/D3L2 item rows");
    ASSERT_EQ(M11_GameView_GetF0115C2500C2900Row(2, 2), -1,
              "D2R2 must not borrow D2C/D3R2 item rows");
}

static void test_m11_runtime_samples_d2_d3_side_walls(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squareData[49];
    struct {
        int relForward;
        int relSide;
        int mapX;
        int mapY;
        int viewSquare;
        int c2500Row;
        const char* label;
    } cases[] = {
        {2, -1, 2, 2,  7,  6, "D2L"},
        {2,  1, 4, 2,  8,  7, "D2R"},
        {2, -2, 1, 2, -1, -1, "D2L2"},
        {2,  2, 5, 2, -1, -1, "D2R2"},
        {3, -2, 1, 1, 14,  3, "D3L2"},
        {3,  2, 5, 1, 15,  4, "D3R2"}
    };
    size_t i;

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(squareData, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5),
           sizeof(squareData));

    map.width = 7;
    map.height = 7;
    tiles.squareData = squareData;
    tiles.squareCount = (int)sizeof(squareData);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 0;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        int outX = -1;
        int outY = -1;
        int element = -1;
        int effective = -1;
        int isWall = 0;
        int isOpen = 1;
        unsigned char raw = 0xffu;
        squareData[cases[i].mapX * (int)map.height + cases[i].mapY] =
            DUNGEON_SQUARE_MASK_THING_LIST;
        ASSERT_EQ(M11_GameView_ProbeViewportCellClass(
                      &state, cases[i].relForward, cases[i].relSide,
                      &outX, &outY, &raw, &element, &effective,
                      &isWall, &isOpen),
                  1, cases[i].label);
        ASSERT_EQ(outX, cases[i].mapX, cases[i].label);
        ASSERT_EQ(outY, cases[i].mapY, cases[i].label);
        ASSERT_EQ(raw, DUNGEON_SQUARE_MASK_THING_LIST, cases[i].label);
        ASSERT_EQ(element, DUNGEON_ELEMENT_WALL, cases[i].label);
        ASSERT_EQ(effective, DUNGEON_ELEMENT_WALL, cases[i].label);
        ASSERT_EQ(isWall, 1, cases[i].label);
        ASSERT_EQ(isOpen, 0, cases[i].label);
        ASSERT_EQ(M11_GameView_GetF0115ViewSquareIndex(
                      cases[i].relForward, cases[i].relSide),
                  cases[i].viewSquare, cases[i].label);
        ASSERT_EQ(M11_GameView_GetF0115C2500C2900Row(
                      cases[i].relForward, cases[i].relSide),
                  cases[i].c2500Row, cases[i].label);
        squareData[cases[i].mapX * (int)map.height + cases[i].mapY] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
}

static void test_m11_runtime_draws_far_side_wall_with_near_side_blocker(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squareData[49];
    struct {
        int relForward;
        int relSide;
        int nearMapX;
        int nearMapY;
        int farMapX;
        int farMapY;
        const char* label;
    } cases[] = {
        {2, -1, 2, 3, 2, 2, "D2L behind D1L wall"},
        {2,  1, 4, 3, 4, 2, "D2R behind D1R wall"},
        {3, -2, 2, 3, 1, 1, "D3L2 behind D1L wall"},
        {3,  2, 4, 3, 5, 1, "D3R2 behind D1R wall"}
    };
    size_t i;

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(squareData, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5),
           sizeof(squareData));

    map.width = 7;
    map.height = 7;
    tiles.squareData = squareData;
    tiles.squareCount = (int)sizeof(squareData);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 0;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        int legacyLaneClear = -1;
        int drawsWithSourceOrder = 0;
        squareData[cases[i].nearMapX * (int)map.height + cases[i].nearMapY] =
            DUNGEON_SQUARE_MASK_THING_LIST;
        squareData[cases[i].farMapX * (int)map.height + cases[i].farMapY] =
            DUNGEON_SQUARE_MASK_THING_LIST;

        ASSERT_EQ(M11_GameView_ProbeSideWallDrawEligibility(
                      &state, cases[i].relForward, cases[i].relSide,
                      &legacyLaneClear, &drawsWithSourceOrder),
                  1, cases[i].label);
        ASSERT_EQ(legacyLaneClear, 0, cases[i].label);
        ASSERT_EQ(drawsWithSourceOrder, 1, cases[i].label);

        squareData[cases[i].nearMapX * (int)map.height + cases[i].nearMapY] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        squareData[cases[i].farMapX * (int)map.height + cases[i].farMapY] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
}

static void test_m11_runtime_draws_far_side_wall_with_center_blocker(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squareData[49];
    struct {
        int relForward;
        int relSide;
        int centerMapX;
        int centerMapY;
        int farMapX;
        int farMapY;
        const char* label;
    } cases[] = {
        {2, -1, 3, 3, 2, 2, "D2L behind D1C wall"},
        {2,  1, 3, 3, 4, 2, "D2R behind D1C wall"},
        {3, -1, 3, 2, 2, 1, "D3L behind D2C wall"},
        {3,  1, 3, 2, 4, 1, "D3R behind D2C wall"}
    };
    size_t i;

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(squareData, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5),
           sizeof(squareData));

    map.width = 7;
    map.height = 7;
    tiles.squareData = squareData;
    tiles.squareCount = (int)sizeof(squareData);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 0;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        int legacyLaneClear = -1;
        int drawsWithSourceOrder = 0;
        squareData[cases[i].centerMapX * (int)map.height + cases[i].centerMapY] =
            DUNGEON_SQUARE_MASK_THING_LIST;
        squareData[cases[i].farMapX * (int)map.height + cases[i].farMapY] =
            DUNGEON_SQUARE_MASK_THING_LIST;

        ASSERT_EQ(M11_GameView_ProbeSideWallDrawEligibility(
                      &state, cases[i].relForward, cases[i].relSide,
                      &legacyLaneClear, &drawsWithSourceOrder),
                  1, cases[i].label);
        ASSERT_EQ(legacyLaneClear, 1, cases[i].label);
        ASSERT_EQ(drawsWithSourceOrder, 1, cases[i].label);

        squareData[cases[i].centerMapX * (int)map.height + cases[i].centerMapY] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
        squareData[cases[i].farMapX * (int)map.height + cases[i].farMapY] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
}

#endif

static void test_hoc_floor_ornament_sources_follow_redmcsb(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensors[1];
    unsigned char squareData[7];
    unsigned short squareFirstThings[7];
    unsigned char sensorRaw[8];

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(sensors, 0, sizeof(sensors));
    memset(squareData, 0, sizeof(squareData));
    memset(squareFirstThings, 0, sizeof(squareFirstThings));
    memset(sensorRaw, 0, sizeof(sensorRaw));

    /* ReDMCSB DUNGEON.C F0172 applies both the map-zero random stream and
     * explicit floor-sensor overrides. Map zero is HoC; it is not special
     * cased out of the original ornament calculation. */
    squareData[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    squareFirstThings[0] = make_thing(THING_TYPE_SENSOR, 0);
    squareFirstThings[1] = THING_ENDOFLIST;
    squareFirstThings[2] = THING_ENDOFLIST;
    squareFirstThings[3] = THING_ENDOFLIST;
    squareFirstThings[4] = THING_ENDOFLIST;
    squareFirstThings[5] = THING_ENDOFLIST;
    squareFirstThings[6] = THING_ENDOFLIST;
    sensorRaw[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    sensorRaw[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);
    sensors[0].ornamentOrdinal = 3;

    map.width = 1;
    map.height = 7;
    map.randomFloorOrnamentCount = 8;
    tiles.squareData = squareData;
    tiles.squareCount = 7;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 7;
    things.rawThingData[THING_TYPE_SENSOR] = sensorRaw;
    things.thingCounts[THING_TYPE_SENSOR] = 1;
    things.sensors = sensors;
    things.sensorCount = 1;
    state.world.dungeon = &dungeon;
    state.world.things = &things;
    ASSERT_EQ(M11_GameView_GetFloorOrnamentOrdinal(&state, 0, 0), 3,
              "HoC floor sensor keeps its source ornament ordinal");
    sensors[0].ornamentOrdinal = 0;
    squareData[0] |= 0x08;
    ASSERT_EQ(M11_GameView_GetFloorOrnamentOrdinal(&state, 0, 0), 0,
              "HoC floor sensor ordinal zero suppresses random ornament");
    sensors[0].ornamentOrdinal = 3;
    squareFirstThings[0] = THING_ENDOFLIST;
    ASSERT_EQ(M11_GameView_GetFloorOrnamentOrdinal(&state, 0, 0), 3,
              "HoC random floor ornament follows F0172 map-zero stream");
}

static void test_m11_runtime_center_wall_blocks_deeper_corridor(void)
{
    M11_GameViewState state;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squareData[49];
    struct {
        int wallForward;
        int wallMapX;
        int wallMapY;
        int expectedDepth;
        const char* label;
    } cases[] = {
        {1, 3, 3, 0, "D1C wall blocks D2C/D3C corridor"},
        {2, 3, 2, 1, "D2C wall blocks D3C corridor"},
        {3, 3, 1, 2, "D3C wall blocks far center"}
    };
    size_t i;

    seed_active_view(&state);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(squareData, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5),
           sizeof(squareData));

    map.width = 7;
    map.height = 7;
    tiles.squareData = squareData;
    tiles.squareCount = (int)sizeof(squareData);
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    state.world.dungeon = &dungeon;
    state.world.party.mapIndex = 0;
    state.world.partyMapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 4;
    state.world.party.direction = 0;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        int depth = -2;
        int relForward = -2;
        int mapX = -2;
        int mapY = -2;
        int element = -2;
        int contentMask = -1;
        int expectedMask = (1 << cases[i].expectedDepth) - 1;

        memset(squareData, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5),
               sizeof(squareData));
        squareData[cases[i].wallMapX * (int)map.height + cases[i].wallMapY] =
            DUNGEON_SQUARE_MASK_THING_LIST;

        ASSERT_EQ(test_dm1_nearest_blocking_center_depth(
                      &state, &depth, &relForward, &mapX, &mapY, &element),
                  1, cases[i].label);
        ASSERT_EQ(depth, cases[i].expectedDepth, cases[i].label);
        ASSERT_EQ(relForward, cases[i].wallForward, cases[i].label);
        ASSERT_EQ(mapX, cases[i].wallMapX, cases[i].label);
        ASSERT_EQ(mapY, cases[i].wallMapY, cases[i].label);
        ASSERT_EQ(element, DUNGEON_ELEMENT_WALL, cases[i].label);
        ASSERT_EQ(test_dm1_center_content_visible_depth_mask(
                      &state, &contentMask),
                  1, cases[i].label);
        ASSERT_EQ(contentMask, expectedMask, cases[i].label);
    }

    memset(squareData, (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5),
           sizeof(squareData));
    {
        int depth = -2;
        int relForward = -2;
        int mapX = -2;
        int mapY = -2;
        int element = -2;
        int contentMask = -1;
        ASSERT_EQ(test_dm1_nearest_blocking_center_depth(
                      &state, &depth, &relForward, &mapX, &mapY, &element),
                  1, "open center corridor resolves");
        ASSERT_EQ(depth, -1, "open center corridor has no center blocker");
        ASSERT_EQ(relForward, -1, "open center corridor has no blocker rel forward");
        ASSERT_EQ(mapX, -1, "open center corridor has no blocker x");
        ASSERT_EQ(mapY, -1, "open center corridor has no blocker y");
        ASSERT_EQ(element, -1, "open center corridor has no blocker element");
        ASSERT_EQ(test_dm1_center_content_visible_depth_mask(
                      &state, &contentMask),
                  1, "open center corridor content mask resolves");
        ASSERT_EQ(contentMask, 7, "open center corridor draws D1/D2/D3 contents");
    }
}

int main(void)
{
    printf("=== M11 Overlay Command Queue Block Regression ===\n");
    printf("no game data: synthetic M11_GameViewState only\n\n");

    test_dialog_overlay_blocks_keyboard_command();
    test_inventory_overlay_blocks_keyboard_command();
    test_map_overlay_blocks_mouse_command();
    test_candidate_panel_blocks_direct_spell_helpers();
    test_csb_cast_never_uses_dm1_spell_executor();
    test_candidate_panel_blocks_direct_inventory_toggle();
    test_candidate_panel_blocks_direct_map_toggle();
    test_candidate_panel_uses_dm1_hoc_menu_route_receipt();
    test_dm1_hoc_startup_render_consumer_is_m11_ready();
    test_candidate_panel_blocks_direct_object_helpers();
    test_candidate_panel_blocks_direct_quickload_only();
    test_candidate_panel_blocks_rest_and_source_save_commands();
    test_candidate_panel_hides_stale_action_rows();
    test_hoc_floor_ornament_sources_follow_redmcsb();
    test_keyboard_positive_control_dispatches_without_overlay();
    test_keyboard_positive_control_dispatches_turn_without_overlay();
    test_mouse_positive_control_dispatches_without_overlay();
    test_m11_runtime_center_wall_blocks_deeper_corridor();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
