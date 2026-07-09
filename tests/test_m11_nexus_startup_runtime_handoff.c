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
    engine->ui_startup_surfaces_expected = 1;
    engine->ui_startup_surfaces_loaded = 1;
    engine->ui_faces_expected = NEXUS_MAX_CHAMPIONS;
    engine->ui_faces_loaded = NEXUS_MAX_CHAMPIONS;
    engine->menu_bpk_upload_receipt_valid = 1;
    engine->menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED;
    engine->menu_bpk_upload_receipt.ready_uploads = 3;
    engine->menu_bpk_upload_receipt.planned_rows = 3;
    engine->sfx_runtime_receipt.status = NEXUS_SFX_RUNTIME_READY_DECODED;
    engine->sfx_runtime_receipt.level_index = 0;
    engine->sfx_runtime_receipt.cd_track = 2;
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
    view->nexusState.champion_select_active = 1;
    view->nexusState.champion_cursor = 0;
    view->nexusState.champion_select_frame = 0;
}

int main(void)
{
    Nexus_V1_Engine engine;
    M11_GameViewState view;
    M11_GameInputResult result;

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

    fill_ready_engine(&engine);
    fill_view(&view, &engine);
    engine.menu_bpk_upload_receipt.route =
        NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3;
    engine.menu_bpk_upload_receipt.blocked_prs3_uploads = 3;
    engine.menu_bpk_upload_receipt.blocks_real_menu_surface_render = 1;
    result = M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION);
    expect_true(result == M11_GAME_INPUT_REDRAW,
                "M11 Nexus blocked startup action still redraws");
    expect_true(view.nexusState.champion_select_active == 1,
                "M11 Nexus blocked startup action stays in champion menu");
    expect_true(view.nexusState.startup_runtime_handoff_ready == 0 &&
                    view.nexusState.startup_dgn_render_ready == 0 &&
                    view.nexusState.startup_hud_ready == 0,
                "M11 Nexus blocked startup action exposes no runtime readiness");

    if (g_failures) {
        fprintf(stderr,
                "test_m11_nexus_startup_runtime_handoff: %d failure(s)\n",
                g_failures);
        return 1;
    }
    puts("ok: M11 Nexus startup gate exposes DGN render/HUD readiness");
    return 0;
}
