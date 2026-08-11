/* Real FM Towns DM2 M11 GAME_LOAD -> active-session regression.
 * The selected HME-242 archive and the authenticated English companion stay
 * in their original locations; no game data is generated or unpacked. */

#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "asset_status_m12.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_dungeon_input_owner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_FMTOWNS_ROOT");
    const char *companion = getenv("FIRESTAFF_DM2_ENGLISH_COMPANION");
    M12_AssetStatus assets;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    DM2_V1_StartupMenuPointerLayout layout;
    DM2_V1_BootStartupMenuHudGdatReceipt startup_hud;
    DM2_V1_BootRuntimeReceipt runtime_receipt;
    unsigned char framebuffer[M11_FB_BYTES];
    char selected_runtime[1024];
    int step;
    int x;
    int y;
    M11_GameInputResult cycle_result;
    DM2_V1_BootExpandedRectReceipt action_rect;
    DM2_V1_BootExpandedRectReceipt command_rect;
    DM2_V1_BootExpandedRectReceipt viewport_rect;
    DM2_V1_DungeonInputOwner input_owner;
    DM2_V1_DungeonInputReceipt input_receipt;
    static const struct {
        uint16_t event_index;
        uint16_t rect_id;
    } source_pointer_routes[] = {
        { 112u, 0x003Bu }, { 113u, 0x003Fu }, { 114u, 0x0040u },
        { 115u, 0x0041u },
        { 116u, 0x004Au }, { 117u, 0x0046u }, { 118u, 0x004Bu },
        { 119u, 0x0047u }, { 120u, 0x004Cu }, { 121u, 0x0048u },
        { 122u, 0x004Du }, { 123u, 0x0049u }
    };

    if (!root || !root[0] || !companion || !companion[0]) {
        puts("SKIP: FM Towns root and English companion are required");
        return 0;
    }
    memset(&assets, 0, sizeof(assets));
    memset(selected_runtime, 0, sizeof(selected_runtime));
    M12_AssetStatus_ScanGame(&assets, root, "dm2");
    check(M12_AssetStatus_ResolveRuntimeDataDirForVersion(
                  &assets, "dm2", "fmtowns-ja", selected_runtime,
                  sizeof(selected_runtime)) == 1,
          "M12 resolves the authenticated FM Towns DM2 runtime owner");
    if (failures) return 1;

    memset(&spec, 0, sizeof(spec));
    spec.gameId = "dm2";
    spec.sourceId = "dm2";
    spec.title = "DUNGEON MASTER II";
    spec.dataDir = selected_runtime;
    spec.dm2EnglishCompanionPath = companion;
    spec.languageIndex = 1;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = M11_FB_WIDTH;
    spec.presentationHeight = M11_FB_HEIGHT;

    M11_GameView_Init(&view);
    check(dm2_v1_runtime_cycle_action_champion() == 0,
          "DM2 champion cycling stays unavailable before source GAME_LOAD");
    check(M11_GameView_Start(&view, &spec) == 1,
          "FM Towns DM2 starts through the M11 boot owner");
    for (step = 0; step < 20000 && view.dm2FmtownsSwooshActive; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    for (step = 0; step < 20000 && !view.dm2FmtownsTitleFinished; ++step) {
        (void)M11_GameView_AdvanceIdleTick(&view);
    }
    check(view.dm2FmtownsTitleFinished && view.dm2State.startup_menu_active,
          "FM Towns title reaches the source SKULL menu boundary");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);

    memset(&layout, 0, sizeof(layout));
    memset(&startup_hud, 0, sizeof(startup_hud));
    check(dm2_v1_boot_startup_menu_hud_gdat_receipt(
              (DM2_V1_BootProfile *)view.dm2BootProfile, &startup_hud) &&
              startup_hud.hud_static_plan_ready &&
              startup_hud.hud_static_command_count == 4,
          "FM Towns binds the native INTERFACE_GENERAL HUD chrome plan");
    check(dm2_v1_boot_startup_menu_pointer_layout(
              (DM2_V1_BootProfile *)view.dm2BootProfile, &layout) &&
              layout.valid && layout.new_game.w > 0 && layout.new_game.h > 0,
          "FM Towns SKULL menu exposes the real NEW GAME rectangle");
    x = layout.new_game.x + layout.new_game.w / 2;
    y = layout.new_game.y + layout.new_game.h / 2;
    check(M11_GameView_HandlePointerButton(
              &view, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
              M11_GAME_INPUT_REDRAW,
          "FM Towns NEW GAME dispatches through the source pointer route");
    check(!view.dm2State.startup_menu_active && view.dm2State.level_loaded,
          "FM Towns NEW GAME closes the menu only after GAME_LOAD commit");
    memset(&runtime_receipt, 0, sizeof(runtime_receipt));
    check(view.dm2BootProfile &&
              ((DM2_V1_BootProfile *)view.dm2BootProfile)
                  ->source_game_load_session_ready &&
              dm2_v1_boot_runtime_capture(
                  (DM2_V1_BootProfile *)view.dm2BootProfile,
                  &runtime_receipt) && runtime_receipt.runtime_ready,
          "FM Towns M11 publishes the complete source GAME_LOAD session");
    check(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) ==
              M11_GAME_INPUT_REDRAW &&
              M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
              M11_GAME_INPUT_REDRAW,
          "FM Towns active session accepts turn and movement through M11");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, M11_FB_WIDTH, M11_FB_HEIGHT);
    check(view.dm2State.level_loaded &&
              ((DM2_V1_BootProfile *)view.dm2BootProfile)
                  ->source_game_load_session_ready,
          "FM Towns active session keeps the source render owner bound to M11");
    cycle_result = M11_GameView_HandleInput(
        &view, M12_MENU_INPUT_CYCLE_CHAMPION);
    check(cycle_result == M11_GAME_INPUT_REDRAW ||
              cycle_result == M11_GAME_INPUT_IGNORED,
          "FM Towns champion cycling remains on the source selection boundary");
    check(dm2_v1_runtime_activate_action_hand(0, 0) == 1,
          "FM Towns selects the first retained champion's authentic action hand");
    check(dm2_v1_runtime_activate_action_hand(0, 1) == 1,
          "FM Towns retains the first champion's second authentic hand");
    memset(&input_owner, 0, sizeof(input_owner));
    check(dm2_v1_dungeon_input_owner_init_fmtowns(
              &input_owner, (DM2_V1_BootProfile *)view.dm2BootProfile),
          "FM Towns binds the native dungeon input owner");
    check(((DM2_V1_BootProfile *)view.dm2BootProfile)
                  ->fmtowns_skull_mouse_input_table_hash == 0x1500c4c9u,
          "FM Towns retains the authenticated full SKULL.EXP MOUSE_INPUT table receipt");
    {
        unsigned int candidate_count =
            dm2_v1_dungeon_input_owner_fmtowns_candidate_count(&input_owner);
        unsigned int candidate_index;
        int found_rune_quit = 0;
        int found_action_1 = 0;
        int found_panel_close = 0;
        check(candidate_count == DM2_V1_FMTOWNS_MOUSE_INPUT_RECORD_COUNT,
              "FM Towns exposes all 264 authenticated MOUSE_INPUT records");
        for (candidate_index = 0u; candidate_index < candidate_count;
             ++candidate_index) {
            DM2_V1_FmtownsMouseInputCandidate candidate;
            memset(&candidate, 0, sizeof(candidate));
            check(dm2_v1_dungeon_input_owner_fmtowns_candidate(
                      &input_owner, candidate_index, &candidate),
                  "FM Towns returns each authenticated MOUSE_INPUT record");
            if (candidate.event_index == 112u &&
                candidate.rect_id == 0x003Bu &&
                candidate.button_mask == 0x8002u)
                found_rune_quit = 1;
            if (candidate.event_index == 113u &&
                candidate.rect_id == 0x003Fu &&
                candidate.button_mask == 0x2002u)
                found_action_1 = 1;
            if (candidate.event_index == 11u) found_panel_close = 1;
            if (candidate.source_record_index == 119u) {
                Dm2TouchClickZonePc34Compat context;
                memset(&context, 0, sizeof(context));
                check(dm2_v1_dungeon_input_owner_fmtowns_candidate_context(
                          &input_owner, candidate_index, &context) &&
                          context.view ==
                              DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT &&
                          context.commandId == 113u &&
                          strcmp(context.groupName, "hand_panel.action_1") == 0,
                      "FM Towns action candidate retains its source panel context");
            }
        }
        check(found_rune_quit && found_action_1,
              "FM Towns retains the source rune-quit and action candidates");
        check(found_panel_close,
              "FM Towns retains the authenticated panel-close candidate");
        {
            unsigned int inventory_context_count = 0u;
            unsigned int inventory_native_gap_count = 0u;
            int exercised_inventory_route = 0;
            for (candidate_index = 0u; candidate_index < candidate_count;
                 ++candidate_index) {
                DM2_V1_FmtownsMouseInputCandidate candidate;
                Dm2TouchClickZonePc34Compat context;
                DM2_V1_BootExpandedRectReceipt native_rect;
                memset(&candidate, 0, sizeof(candidate));
                check(dm2_v1_dungeon_input_owner_fmtowns_candidate(
                          &input_owner, candidate_index, &candidate),
                      "FM Towns reads inventory candidate before routing it");
                unsigned int context_count =
                    dm2_v1_dungeon_input_owner_fmtowns_candidate_context_count(
                        &input_owner, candidate_index);
                unsigned int context_ordinal;
                int has_inventory_context = 0;
                for (context_ordinal = 0u; context_ordinal < context_count;
                     ++context_ordinal) {
                    memset(&context, 0, sizeof(context));
                    if (dm2_v1_dungeon_input_owner_fmtowns_candidate_context_at(
                            &input_owner, candidate_index, context_ordinal,
                            &context) &&
                        context.view == DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT) {
                        has_inventory_context = 1;
                        break;
                    }
                }
                if (!has_inventory_context) continue;
                memset(&native_rect, 0, sizeof(native_rect));
                {
                    int native_ok =
                        dm2_v1_dungeon_input_owner_fmtowns_candidate_native_rect(
                            &input_owner, candidate_index, &native_rect) &&
                        native_rect.valid && native_rect.rect.w > 0 &&
                        native_rect.rect.h > 0;
                    if (!native_ok) {
                        check(candidate_index == 47u || candidate_index == 48u ||
                                  candidate_index == 49u ||
                                  (candidate_index >= 52u && candidate_index <= 83u) ||
                                  candidate_index == 99u || candidate_index == 110u,
                              "FM Towns native inventory gaps are limited to source records without Towns RAW4 geometry");
                        ++inventory_native_gap_count;
                    } else {
                        ++inventory_context_count;
                        if (!exercised_inventory_route) {
                            DM2_V1_FmtownsUiRouteReceipt route;
                            unsigned int source_button =
                                candidate.button_mask & 0x0003u;
                            int16_t source_x = (int16_t)(
                                native_rect.rect.x + native_rect.rect.w / 2);
                            int16_t source_y = (int16_t)(
                                native_rect.rect.y + native_rect.rect.h / 2);
                            int16_t screen_x = (int16_t)(source_x / 2);
                            int16_t screen_y = (int16_t)(source_y / 2);
                            memset(&route, 0, sizeof(route));
                            check(source_button != 0u &&
                                      dm2_v1_dungeon_input_owner_fmtowns_route_context(
                                          &input_owner,
                                          DM2_V1_FMTOWNS_UI_INVENTORY,
                                          screen_x, screen_y, source_button,
                                          &route) &&
                                      route.accepted &&
                                      route.context ==
                                          DM2_V1_FMTOWNS_UI_INVENTORY &&
                                      route.source_context.view ==
                                          DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT &&
                                      route.native_rect.valid &&
                                      route.candidate.source_record_index ==
                                          candidate.source_record_index,
                                  "FM Towns inventory route uses authenticated native geometry and source context");
                            exercised_inventory_route = 1;
                        }
                    }
                }
            }
            check(inventory_context_count == 129u,
                  "FM Towns retains 129 source inventory contexts with native RAW4 geometry");
            check(inventory_native_gap_count == 37u,
                  "FM Towns keeps 37 inventory controls fail-closed without Towns RAW4 geometry");
            check(exercised_inventory_route,
                  "FM Towns exercises an authenticated inventory route against real RAW4 data");
        }
        {
            DM2_V1_BootProfile tampered_profile =
                *(DM2_V1_BootProfile *)view.dm2BootProfile;
            DM2_V1_DungeonInputOwner tampered_owner = input_owner;
            tampered_profile.fmtowns_skull_mouse_input_table[0] ^= 0x01u;
            tampered_owner.boot_profile = &tampered_profile;
            check(dm2_v1_dungeon_input_owner_fmtowns_candidate_count(
                      &tampered_owner) == 0u,
                  "FM Towns candidate receipt rejects mutated table bytes");
        }
    }
    {
        DM2_V1_BootProfile rejected_profile =
            *(DM2_V1_BootProfile *)view.dm2BootProfile;
        snprintf(rejected_profile.fmtowns_skull_md5,
                 sizeof(rejected_profile.fmtowns_skull_md5), "%s",
                 "00000000000000000000000000000000");
        memset(&input_owner, 0, sizeof(input_owner));
        check(!dm2_v1_dungeon_input_owner_init_fmtowns(
                  &input_owner, &rejected_profile),
              "FM Towns input owner rejects a non-authentic SKULL.EXP identity");
        check(dm2_v1_dungeon_input_owner_init_fmtowns(
                  &input_owner, (DM2_V1_BootProfile *)view.dm2BootProfile),
              "FM Towns input owner rebinds the authentic SKULL.EXP identity");
    }
    memset(&action_rect, 0, sizeof(action_rect));
    check(dm2_v1_boot_query_expanded_rect_receipt(
              (DM2_V1_BootProfile *)view.dm2BootProfile, 0x004Au,
              &action_rect) && action_rect.valid && action_rect.rect.w > 0 &&
              action_rect.rect.h > 0,
          "FM Towns exposes the source champion-1 action-hand RAW4 rectangle");
    if (action_rect.valid) {
        memset(&input_receipt, 0, sizeof(input_receipt));
        {
            int route_ok = dm2_v1_dungeon_input_owner_route(
                  &input_owner, action_rect.rect.x / 2,
                  action_rect.rect.y / 2, DM1_V1_MOUSE_MASK_LEFT_PC34,
                  NULL, NULL, &input_receipt);
            check(route_ok && input_receipt.accepted &&
                  input_receipt.event_index == 116u,
                  "FM Towns native action-hand rect resolves to event 116");
        }
        check(M11_GameView_HandlePointerButton(
                  &view, (action_rect.rect.x + action_rect.rect.w / 2) / 2,
                  (action_rect.rect.y + action_rect.rect.h / 2) / 2,
                  DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW,
              "FM Towns native action-hand click reaches the source event route");
    }
    memset(&command_rect, 0, sizeof(command_rect));
    check(dm2_v1_boot_query_expanded_rect_receipt(
              (DM2_V1_BootProfile *)view.dm2BootProfile, 0x003Fu,
              &command_rect) && command_rect.valid && command_rect.rect.w > 0 &&
              command_rect.rect.h > 0,
          "FM Towns exposes the source first action-panel command rectangle");
    if (command_rect.valid) {
        memset(&input_receipt, 0, sizeof(input_receipt));
        check(dm2_v1_dungeon_input_owner_route(
                  &input_owner, command_rect.rect.x / 2,
                  command_rect.rect.y / 2, DM1_V1_MOUSE_MASK_LEFT_PC34,
                  NULL, NULL, &input_receipt) && input_receipt.accepted &&
              input_receipt.event_index == 113u,
              "FM Towns action-panel rectangle resolves to source event 113");
    }
    {
        size_t route_index;
        for (route_index = 0u;
             route_index < sizeof(source_pointer_routes) /
                                sizeof(source_pointer_routes[0]);
             ++route_index) {
            DM2_V1_BootExpandedRectReceipt source_rect;
            memset(&source_rect, 0, sizeof(source_rect));
            check(dm2_v1_boot_query_expanded_rect_receipt(
                      (DM2_V1_BootProfile *)view.dm2BootProfile,
                      source_pointer_routes[route_index].rect_id,
                      &source_rect) && source_rect.valid &&
                      source_rect.rect.w > 0 && source_rect.rect.h > 0,
                  "FM Towns exposes every admitted action/hand source rectangle");
            if (!source_rect.valid) continue;
            memset(&input_receipt, 0, sizeof(input_receipt));
            check(dm2_v1_dungeon_input_owner_route(
                  &input_owner,
                  (int16_t)((source_rect.rect.x + source_rect.rect.w / 2) / 2),
                      (int16_t)((source_rect.rect.y + source_rect.rect.h / 2) / 2),
                      DM1_V1_MOUSE_MASK_LEFT_PC34,
                      NULL, NULL, &input_receipt) && input_receipt.accepted &&
                      input_receipt.event_index ==
                          source_pointer_routes[route_index].event_index,
                  "FM Towns action/hand rectangle preserves its source event");
        }
    }
    memset(&viewport_rect, 0, sizeof(viewport_rect));
    check(dm2_v1_boot_query_expanded_rect_receipt(
              (DM2_V1_BootProfile *)view.dm2BootProfile, 0x0007u,
              &viewport_rect) && viewport_rect.valid && viewport_rect.rect.w > 0 &&
              viewport_rect.rect.h > 0,
          "FM Towns exposes the source viewport RAW4 rectangle");
    if (viewport_rect.valid) {
        memset(&input_receipt, 0, sizeof(input_receipt));
        check(dm2_v1_dungeon_input_owner_route(
                  &input_owner, viewport_rect.rect.x / 2 + 1,
                  viewport_rect.rect.y / 2 + 1, DM1_V1_MOUSE_MASK_LEFT_PC34,
                  NULL, NULL, &input_receipt) && input_receipt.accepted &&
              input_receipt.event_index == 80u,
              "FM Towns viewport rectangle resolves to source event 0x50");
    }
    {
        DM2_V1_BootRuntimeRenderReceipt render;
        memset(&render, 0, sizeof(render));
        memset(framebuffer, 0, sizeof(framebuffer));
        (void)dm2_v1_boot_runtime_render_frame(
            (DM2_V1_BootProfile *)view.dm2BootProfile, framebuffer,
            M11_FB_WIDTH, M11_FB_WIDTH, M11_FB_HEIGHT, NULL, NULL, &render);
        check(render.render_result == 0 && render.v1_succeeded &&
                  render.runtime_m11_frame_receipt_consumed,
              "FM Towns active session produces an admitted source viewport frame");
        check(render.runtime_m11_frame_hud_material_plan_required &&
                  render.runtime_m11_frame_hud_material_plan_consumed &&
                  render.runtime_m11_frame_hud_material_plan_hash != 0u,
              "FM Towns active session consumes the native HUD material plan");
        {
            DM2_V1_RuntimeViewportClickReceipt click_receipt;
            memset(&click_receipt, 0, sizeof(click_receipt));
            check(!dm2_v1_runtime_route_viewport_click(0, 0, &click_receipt) &&
                      !click_receipt.accepted,
                  "DM2 rejects viewport points outside renderer-owned targets");
        }
        if (viewport_rect.valid) {
            check(M11_GameView_HandlePointerButton(
                      &view,
                      (viewport_rect.rect.x + viewport_rect.rect.w / 2) / 2,
                      (viewport_rect.rect.y + viewport_rect.rect.h / 2) / 2,
                      DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_IGNORED,
                  "DM2 viewport C080 does not fall through to DM1 interaction");
        }
    }
    M11_GameView_Shutdown(&view);
    if (failures) return 1;
    puts("PASS: DM2 FM Towns M11 NEW GAME reaches the source runtime");
    return 0;
}
