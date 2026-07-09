#include "m11_game_view.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_startup_layout.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures;
static unsigned char g_surface_pixel = 7u;

static void expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void fill_ready_engine(Nexus_V1_Engine *engine)
{
    memset(engine, 0, sizeof(*engine));
    engine->level_loaded = 1;
    engine->game.party_x = 3;
    engine->game.party_y = 4;
    engine->game.party_dir = 0;
    engine->game.tick_count = 0;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].data = &g_surface_pixel;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].w = 320;
    engine->ui.surfaces[NEXUS_SURFACE_TITLE].h = 200;
    engine->ui.surfaces[NEXUS_SURFACE_WARNING].data = &g_surface_pixel;
    engine->ui.surfaces[NEXUS_SURFACE_WARNING].w = 320;
    engine->ui.surfaces[NEXUS_SURFACE_WARNING].h = 200;
    engine->ui.surfaces[NEXUS_SURFACE_GAMEOVER].data = &g_surface_pixel;
    engine->ui.surfaces[NEXUS_SURFACE_GAMEOVER].w = 320;
    engine->ui.surfaces[NEXUS_SURFACE_GAMEOVER].h = 200;
    engine->ui_startup_surfaces_expected = 1;
    engine->ui_startup_surfaces_loaded = 1;
    engine->ui_faces_expected = NEXUS_MAX_CHAMPIONS;
    engine->ui_faces_loaded = NEXUS_MAX_CHAMPIONS;
    engine->menu_bpk_upload_receipt_valid = 1;
    engine->menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED;
    engine->menu_bpk_upload_receipt.ready_uploads = 3;
    engine->menu_bpk_upload_receipt.planned_rows = 3;
    engine->menu_bpk_decode_receipt_valid = 1;
    engine->menu_bpk_decode_receipt_attempted = 1;
    engine->menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED;
    engine->menu_bpk_decode_receipt.surface_entries = 3;
    engine->menu_bpk_decode_receipt.ready_stored_surfaces = 3;
    engine->sfx_runtime_receipt.status = NEXUS_SFX_RUNTIME_READY_DECODED;
    engine->sfx_runtime_receipt.level_index = 0;
    engine->sfx_runtime_receipt.cd_track = 2;
    engine->script_runtime_receipt.status =
        NEXUS_SCRIPT_RUNTIME_READY_PARSED;
    engine->script_runtime_receipt.level_index = 0;
    engine->script_runtime_receipt.candidate_source_loaded = 1;
    engine->script_runtime_receipt.candidate_source_bytes = 2388;
    engine->script_runtime_receipt.parser_supported = 1;
    engine->script_runtime_receipt.dispatch_enabled = 1;
    engine->script_runtime_receipt.rules_loaded = 2;
    engine->current_level.width = NEXUS_MAX_MAP_SIZE;
    engine->current_level.height = NEXUS_MAX_MAP_SIZE;
    engine->current_level.geometry_info.dmweb_container = 1;
    engine->current_level.geometry_info.mesh_ready = 1;
    engine->current_level.geometry_info.geometry_offset = 0x9000;
    engine->current_level.geometry_info.geometry_size = 2048;
    engine->current_level.geometry_info.collision_ref_count = 4;
    engine->current_level.geometry_info.collision_ref_unique_count = 1;
    engine->current_level.geometry_info.max_collision_ref = 5;
    engine->current_level.squares[4][3] = 1;
    engine->current_level.squares[3][3] = 1;
    engine->current_level.squares[4][4] = 1;
    engine->current_level.collision_refs[4][3] = 0x0100;
    engine->current_level.collision_refs[3][3] = 0x0fff;
    nexus_v1_champions_init(&engine->champions);
    (void)nexus_v1_champion_recruit(&engine->champions, 0);
}

static void fill_view(M11_GameViewState *view, Nexus_V1_Engine *engine)
{
    M11_GameView_Init(view);
    view->active = 1;
    view->sourceKind = M11_GAME_SOURCE_NEXUS_DGN;
    view->nexusEngine = engine;
    view->nexusState.level_loaded = engine->level_loaded;
    view->nexusState.party_x = engine->game.party_x;
    view->nexusState.party_y = engine->game.party_y;
    view->nexusState.party_dir = engine->game.party_dir;
    view->nexusState.title_loaded = 1;
    view->nexusState.champion_select_active = 1;
    view->nexusState.champion_cursor = 0;
    view->nexusState.champion_select_frame = 0;
}

static int count_nonzero_pixels(const unsigned char *pixels, int count)
{
    int i;
    int nonzero = 0;
    if (!pixels || count <= 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (pixels[i] != 0u) {
            ++nonzero;
        }
    }
    return nonzero;
}

int main(void)
{
    Nexus_V1_Engine engine;
    M11_GameViewState view;
    M11_GameInputResult result;
    unsigned char framebuffer[320 * 200];

    fill_ready_engine(&engine);
    fill_view(&view, &engine);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION);
    expect_true(result == M11_GAME_INPUT_REDRAW,
                "M11 Nexus champion action starts dungeon");
    expect_true(view.nexusState.champion_select_active == 0,
                "M11 Nexus champion start clears startup menu");
    expect_true(view.nexusState.startup_runtime_handoff_ready == 1 &&
                    view.nexusState.startup_dgn_render_ready == 1 &&
                    view.nexusState.startup_hud_ready == 1 &&
                    view.nexusState.startup_dgn_render_command_count > 0 &&
                    view.nexusState.startup_dgn_render_blocked == 0,
                "M11 Nexus startup gate exposes first DGN/HUD readiness");
    expect_true(view.nexusState.startup_host_caller_ready == 1 &&
                    view.nexusState.startup_host_capture_ready == 1 &&
                    view.nexusState.startup_host_dgn_ready == 1 &&
                    view.nexusState.startup_host_execute_startup_draws == 1 &&
                    view.nexusState.startup_host_execute_dgn_draws == 1 &&
                    view.nexusState.startup_bpk_handoff_consumed == 1 &&
                    view.nexusState.startup_prs3_blocker_consumed == 0 &&
                    view.nexusState.startup_dgn_handoff_consumed == 1 &&
                    view.nexusState.startup_no_fallback_visuals_enforced == 1 &&
                    view.nexusState.startup_suppress_fallback_visuals == 1 &&
                    view.nexusState.startup_suppress_legacy_placeholder_visuals == 1 &&
                    view.nexusState.startup_full_start_package_consumed == 1 &&
                    view.nexusState.startup_bundle_consumed == 1 &&
                    view.nexusState.startup_display_callers_use_package_receipt == 1 &&
                    view.nexusState.startup_saturn_timing_exact == 1 &&
                    view.nexusState.startup_saturn_capture_frames_exact == 1 &&
                    view.nexusState.startup_saturn_warning_frame == 0 &&
                    view.nexusState.startup_saturn_title_capture_frame == 48 &&
                    view.nexusState.startup_saturn_champion_capture_frame == 102 &&
                    view.nexusState.startup_saturn_save_capture_frame == -1 &&
                    view.nexusState.startup_saturn_dungeon_capture_frame == 102 &&
                    view.nexusState.startup_saturn_title_ready_frame == 102 &&
                    view.nexusState.startup_saturn_gameover_capture_frame == 0 &&
                    view.nexusState.startup_host_active_capture_frame == 102 &&
                    view.nexusState.startup_host_saturn_active_capture_frame == 102 &&
                    view.nexusState.startup_host_route_consumes_active_capture_frame == 1 &&
                    view.nexusState.startup_host_route_consumes_dungeon_capture_frame == 1 &&
                    view.nexusState.startup_host_route_capture_matrix_ready == 1 &&
                    view.nexusState.startup_host_route_capture_matrix_exact == 1 &&
                    view.nexusState.startup_host_saturn_non_title_capture_count == 2 &&
                    view.nexusState.startup_host_saturn_non_title_capture_mask == 6u &&
                    view.nexusState.startup_host_saturn_expected_capture_mask == 6u &&
                    view.nexusState.startup_title_timing_frame == -1 &&
                    view.nexusState.startup_title_timing_frame_max == 102 &&
                    view.nexusState.startup_title_timing_ready == 1 &&
                    view.nexusState.startup_package_capture_consumed == 1 &&
                    view.nexusState.startup_package_route_matches_capture_route == 1 &&
                    view.nexusState.startup_host_route_consumes_package_route == 1 &&
                    view.nexusState.startup_host_route_consumes_capture_matrix == 1 &&
                    view.nexusState.startup_dgn_route_consumes_startup_package == 1 &&
                    view.nexusState.startup_dgn_route_saturn_capture_exact == 1 &&
                    view.nexusState.startup_host_ownership_route_matches_capture_route == 1 &&
                    view.nexusState.startup_package_route_consumes_host_ownership == 1 &&
                    view.nexusState.startup_dgn_route_consumes_host_ownership == 1 &&
                    view.nexusState.startup_route_consumption_complete == 1 &&
                    view.nexusState.startup_non_title_saturn_capture_route_complete == 1 &&
                    view.nexusState.startup_dungeon_route_consumption_complete == 1 &&
                    view.nexusState.startup_single_saturn_owner_ready == 1 &&
                    view.nexusState.startup_title_menu_capture_route_joined == 1 &&
                    view.nexusState.startup_runtime_dgn_route_joined == 1 &&
                    view.nexusState.startup_blocked_route_suppresses_all_draws == 0 &&
                    view.nexusState.startup_copied_draw_command_count > 0 &&
                    view.nexusState.startup_copied_dgn_render_command_count > 0,
                "M11 Nexus startup gate consumes host-caller receipt for capture and DGN handoff");

    fill_ready_engine(&engine);
    fill_view(&view, &engine);
    view.nexusState.champion_select_active = 0;
    view.nexusState.startup_save_select_active = 1;
    view.nexusState.startup_save_row_count = 1;
    view.nexusState.startup_save_selected_row = 0;
    view.nexusState.startup_save_slot_mask = 0;
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN);
    expect_true(result == M11_GAME_INPUT_REDRAW &&
                    view.nexusState.startup_host_caller_ready == 1 &&
                    view.nexusState.startup_host_execute_startup_draws == 1 &&
                    view.nexusState.startup_saturn_save_capture_frame == 102 &&
                    view.nexusState.startup_saturn_champion_capture_frame == -1 &&
                    view.nexusState.startup_saturn_dungeon_capture_frame == -1 &&
                    view.nexusState.startup_host_active_capture_frame == 102 &&
                    view.nexusState.startup_host_saturn_active_capture_frame == 102 &&
                    view.nexusState.startup_host_route_consumes_active_capture_frame == 1 &&
                    view.nexusState.startup_host_route_consumes_dungeon_capture_frame == 0 &&
                    view.nexusState.startup_host_route_capture_matrix_ready == 1 &&
                    view.nexusState.startup_host_route_capture_matrix_exact == 1 &&
                    view.nexusState.startup_host_saturn_non_title_capture_count == 1 &&
                    view.nexusState.startup_host_saturn_non_title_capture_mask == 1u &&
                    view.nexusState.startup_host_saturn_expected_capture_mask == 1u &&
                    view.nexusState.startup_package_route_matches_capture_route == 1 &&
                    view.nexusState.startup_host_route_consumes_package_route == 1 &&
                    view.nexusState.startup_host_route_consumes_capture_matrix == 1 &&
                    view.nexusState.startup_dgn_route_consumes_startup_package == 0 &&
                    view.nexusState.startup_dgn_route_saturn_capture_exact == 0 &&
                    view.nexusState.startup_host_ownership_route_matches_capture_route == 1 &&
                    view.nexusState.startup_package_route_consumes_host_ownership == 1 &&
                    view.nexusState.startup_dgn_route_consumes_host_ownership == 0 &&
                    view.nexusState.startup_route_consumption_complete == 1 &&
                    view.nexusState.startup_non_title_saturn_capture_route_complete == 1 &&
                    view.nexusState.startup_dungeon_route_consumption_complete == 0 &&
                    view.nexusState.startup_copied_draw_command_count > 0,
                "M11 Nexus save startup route consumes Saturn save capture receipt");

    fill_ready_engine(&engine);
    fill_view(&view, &engine);
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_NONE);
    expect_true(result == M11_GAME_INPUT_IGNORED &&
                    view.nexusState.champion_select_active == 1,
                "M11 Nexus idle champion route does not mutate selection");
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(count_nonzero_pixels(framebuffer, (int)sizeof(framebuffer)) > 0,
                "M11 Nexus champion draw consumes real startup presentation");

    fill_ready_engine(&engine);
    fill_view(&view, &engine);
    engine.menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3;
    engine.menu_bpk_upload_receipt.blocked_prs3_uploads = 3;
    engine.menu_bpk_upload_receipt.blocks_real_menu_surface_render = 1;
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3;
    engine.menu_bpk_decode_receipt.blocked_prs3_surfaces = 3;
    engine.menu_bpk_decode_receipt.prs3_stream_plans = 3;
    engine.menu_bpk_decode_receipt.requires_prs3_decoder = 1;
    engine.menu_bpk_decode_receipt.decode_blocked = 1;
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION);
    expect_true(result == M11_GAME_INPUT_REDRAW,
                "M11 Nexus PRS3 startup action still redraws");
    expect_true(view.nexusState.champion_select_active == 0,
                "M11 Nexus PRS3 startup action consumes package route");
    expect_true(view.nexusState.startup_runtime_handoff_ready == 0 &&
                    view.nexusState.startup_dgn_render_ready == 0 &&
                    view.nexusState.startup_hud_ready == 0,
                "M11 Nexus blocked startup action exposes no runtime readiness");
    expect_true(view.nexusState.startup_prs3_blocker_consumed == 1 &&
                    view.nexusState.startup_no_fallback_visuals_enforced == 1 &&
                    view.nexusState.startup_suppress_fallback_visuals == 1 &&
                    view.nexusState.startup_suppress_legacy_placeholder_visuals == 1 &&
                    view.nexusState.startup_saturn_warning_frame == 0 &&
                    view.nexusState.startup_saturn_title_capture_frame == 48 &&
                    view.nexusState.startup_saturn_title_ready_frame == 102 &&
                    view.nexusState.startup_saturn_gameover_capture_frame == 0 &&
                    view.nexusState.startup_host_route_consumes_active_capture_frame == 0 &&
                    view.nexusState.startup_host_route_consumes_dungeon_capture_frame == 0 &&
                    view.nexusState.startup_host_route_capture_matrix_ready == 0,
                "M11 Nexus PRS3 startup consumes blocker without legacy fallback");

    fill_ready_engine(&engine);
    fill_view(&view, &engine);
    view.nexusState.title_active = 1;
    view.nexusState.champion_select_active = 0;
    engine.menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3;
    engine.menu_bpk_upload_receipt.blocked_prs3_uploads = 3;
    engine.menu_bpk_upload_receipt.blocks_real_menu_surface_render = 1;
    engine.menu_bpk_decode_receipt.route =
        NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3;
    engine.menu_bpk_decode_receipt.blocked_prs3_surfaces = 3;
    engine.menu_bpk_decode_receipt.prs3_stream_plans = 3;
    engine.menu_bpk_decode_receipt.requires_prs3_decoder = 1;
    engine.menu_bpk_decode_receipt.decode_blocked = 1;
    memset(framebuffer, 0x7f, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(count_nonzero_pixels(framebuffer, (int)sizeof(framebuffer)) > 0,
                "M11 Nexus draw uses real title capture before blocked menu fallback");

    if (g_failures) {
        fprintf(stderr,
                "test_m11_nexus_startup_runtime_handoff: %d failure(s)\n",
                g_failures);
        return 1;
    }
    puts("ok: M11 Nexus startup gate exposes DGN render/HUD readiness");
    return 0;
}
