/*
 * nexus_v1_launcher.c
 * ===================
 * Nexus V1 launcher — singleton engine lifecycle manager.
 *
 * Owns the Nexus_V1_Engine singleton for the duration of the session.
 * All other Nexus V1 modules are accessed through nexus_v1_engine.c
 * which uses the launcher's engine field.
 *
 * Source: DM Nexus (Saturn) boot flow, NEXUS.C / NEXUS2.C engine
 * lifecycle, ReDMCSB boot/disk loading references.
 */

#include "nexus_v1_launcher.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_save.h"
#include "nexus_v1_world.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ── Singleton ──────────────────────────────────────────────────────── */
static Nexus_V1_Engine s_engine;
static int s_initialized = 0;

/* ── Public API ─────────────────────────────────────────────────────── */

int nexus_v1_launcher_init(const char *data_dir) {
    if (!data_dir) {
        printf("Nexus launcher: NULL data_dir\n");
        return -1;
    }

    /* Already initialized — return success without re-init */
    if (s_initialized) {
        printf("Nexus launcher: already initialized\n");
        return 0;
    }

    /* Init the engine singleton.
     * nexus_v1_init() auto-detects ISO vs extracted files and
     * populates the full engine: ISO reader, game state, mechanics,
     * champions, creatures, sound, and font.
     * Source: nexus_v1_engine.c nexus_v1_init() */
    int rc = nexus_v1_init(&s_engine, data_dir);
    if (rc != 0) {
        printf("Nexus launcher: nexus_v1_init failed for '%s'\n", data_dir);
        return -1;
    }

    s_initialized = 1;
    printf("Nexus launcher: initialized (data_dir='%s')\n", data_dir);
    return 0;
}

int nexus_v1_launcher_load_level(int level) {
    if (!s_initialized) {
        printf("Nexus launcher: not initialized — call nexus_v1_launcher_init first\n");
        return -1;
    }
    if (level < 0 || level > 15) {
        printf("Nexus launcher: invalid level %d (must be 0-15)\n", level);
        return -1;
    }
    int rc = nexus_v1_load_level(&s_engine, level);
    if (rc != 0) {
        printf("Nexus launcher: failed to load level %d\n", level);
        return -1;
    }
    printf("Nexus launcher: loaded level %d\n", level);
    return 0;
}

Nexus_V1_Engine *nexus_v1_launcher_get_engine(void) {
    if (!s_initialized) {
        return NULL;
    }
    return &s_engine;
}

void nexus_v1_launcher_boot_receipt_clear(
    Nexus_V1_LauncherBootReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_launch_receipt_clear(&receipt->startup_receipt);
}

void nexus_v1_launcher_runtime_receipt_clear(
    Nexus_V1_LauncherRuntimeReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_launch_receipt_clear(&receipt->startup_receipt);
    nexus_v1_startup_host_receipt_clear(&receipt->boot_status_receipt);
}

void nexus_v1_launcher_startup_launch_gate_receipt_clear(
    Nexus_V1_StartupLaunchGateReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_LAUNCH_GATE_INVALID;
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_asset_handoff_receipt_clear(
    Nexus_V1_StartupAssetHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_INVALID;
}

const char *nexus_v1_launcher_startup_launch_gate_route_name(
    Nexus_V1_StartupLaunchGateRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_LAUNCH_GATE_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR: return "data-error";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_TITLE_READY: return "title-ready";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED:
        return "menu-asset-blocked";
    case NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY: return "menu-ready";
    default: return "unknown";
    }
}

const char *nexus_v1_launcher_startup_asset_handoff_route_name(
    Nexus_V1_StartupAssetHandoffRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR: return "data-error";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY: return "title-ready";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED:
        return "menu-blocked";
    case NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY:
        return "main-menu-ready";
    default: return "unknown";
    }
}

int nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupLaunchGateReceipt *out_receipt)
{
    const Nexus_V1_LauncherStartupAssetsReceipt *assets;

    nexus_v1_launcher_startup_launch_gate_receipt_clear(out_receipt);
    if (!runtime || !out_receipt) {
        return 0;
    }
    assets = &runtime->startup_assets;
    out_receipt->assets = *assets;
    out_receipt->engine_ready = runtime->engine ? 1 : 0;
    out_receipt->level_loaded = runtime->level_loaded ? 1 : 0;
    out_receipt->title_ready = runtime->title_loaded ? 1 : 0;
    out_receipt->title_draw_ready = assets->title_route_ready ? 1 : 0;
    out_receipt->real_menu_ready =
        assets->real_menu_surface_route_ready ? 1 : 0;
    out_receipt->save_menu_ready = assets->save_menu_route_ready ? 1 : 0;
    out_receipt->champion_menu_ready =
        assets->champion_menu_route_ready ? 1 : 0;
    out_receipt->fallback_visuals_permitted =
        assets->menu_bpk_fallback_visuals_permitted ? 1 : 0;
    out_receipt->asset_route = assets->startup_menu_asset_route;
    out_receipt->asset_blocker = assets->real_menu_surface_blocker;
    out_receipt->boot_log_line = runtime->boot_log_line;

    if (!runtime->engine || !runtime->level_loaded) {
        out_receipt->route = NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR;
        out_receipt->status_scope =
            runtime->startup_receipt.host_receipt.status_scope
                ? runtime->startup_receipt.host_receipt.status_scope
                : "BOOT";
        out_receipt->status =
            runtime->startup_receipt.host_receipt.status
                ? runtime->startup_receipt.host_receipt.status
                : "NEXUS DATA ERROR";
        out_receipt->host_receipt = runtime->startup_receipt.host_receipt;
        if (!out_receipt->host_receipt.status) {
            out_receipt->host_receipt.status_scope = out_receipt->status_scope;
            out_receipt->host_receipt.status = out_receipt->status;
        }
        return 1;
    }

    if (!assets->title_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-title-assets";
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        out_receipt->host_receipt.status_scope = out_receipt->status_scope;
        out_receipt->host_receipt.status = out_receipt->status;
        return 1;
    }

    if (!assets->real_menu_surface_route_ready) {
        out_receipt->route =
            NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
        out_receipt->host_receipt.input_result =
            NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
        out_receipt->host_receipt.status_scope = out_receipt->status_scope;
        out_receipt->host_receipt.status = out_receipt->status;
        return 1;
    }

    out_receipt->route = assets->save_menu_route_ready &&
                         assets->champion_menu_route_ready
        ? NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY
        : NEXUS_V1_STARTUP_LAUNCH_GATE_TITLE_READY;
    out_receipt->status_scope = "STARTUP";
    out_receipt->status = out_receipt->route ==
        NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY
            ? "NEXUS MENU READY"
            : "NEXUS TITLE";
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = out_receipt->status_scope;
    out_receipt->host_receipt.status = out_receipt->status;
    return 1;
}

static int nexus_v1_launcher_startup_asset_handoff_from_parts(
    Nexus_V1_Engine *engine,
    int level_loaded,
    int title_loaded,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    const char *boot_status,
    Nexus_V1_StartupAssetHandoffReceipt *out_receipt)
{
    Nexus_V1_MenuBpkRendererHandoffReceipt renderer_handoff;

    nexus_v1_launcher_startup_asset_handoff_receipt_clear(out_receipt);
    if (!out_receipt || !assets) {
        return 0;
    }

    out_receipt->assets = *assets;
    memset(&renderer_handoff, 0, sizeof(renderer_handoff));
    if (engine &&
        nexus_v1_menu_bpk_renderer_handoff_receipt(engine,
                                                   &renderer_handoff) == 0) {
        out_receipt->menu_bpk_renderer_handoff = renderer_handoff;
        out_receipt->menu_bpk_renderer_handoff_valid =
            renderer_handoff.receipt_valid ? 1 : 0;
        out_receipt->menu_bpk_prs3_blocks_real_menu_route =
            renderer_handoff.status ==
            NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3;
    }
    out_receipt->title_asset_handoff_ready =
        engine && title_loaded && assets->title_route_ready;
    out_receipt->real_menu_asset_handoff_ready =
        assets->real_menu_surface_route_ready &&
        (!out_receipt->menu_bpk_renderer_handoff_valid ||
         renderer_handoff.can_render_stored_surfaces) ? 1 : 0;
    out_receipt->audio_asset_handoff_ready =
        assets->startup_audio_handoff_ready ? 1 : 0;
    out_receipt->main_menu_route_ready =
        engine &&
        level_loaded &&
        out_receipt->title_asset_handoff_ready &&
        out_receipt->audio_asset_handoff_ready &&
        out_receipt->real_menu_asset_handoff_ready;
    out_receipt->fallback_visuals_permitted =
        assets->menu_bpk_fallback_visuals_permitted ||
        (out_receipt->menu_bpk_renderer_handoff_valid &&
         renderer_handoff.blocks_real_menu_surface_render &&
         renderer_handoff.fallback_visuals_permitted) ? 1 : 0;
    out_receipt->saturn_asset_handoff_ready =
        out_receipt->title_asset_handoff_ready &&
        out_receipt->audio_asset_handoff_ready;
    out_receipt->real_asset_route_ready =
        out_receipt->main_menu_route_ready &&
        !out_receipt->fallback_visuals_permitted;
    out_receipt->blocks_main_menu_route =
        out_receipt->main_menu_route_ready ? 0 : 1;
    out_receipt->title_asset_route =
        out_receipt->title_asset_handoff_ready
            ? "ready-title-assets"
            : "blocked-title-assets";
    out_receipt->menu_asset_route = assets->startup_menu_asset_route;
    out_receipt->audio_asset_route =
        out_receipt->audio_asset_handoff_ready
            ? "ready-track02-sfx"
            : "blocked-track02-sfx";

    if (!engine || !level_loaded) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR;
        out_receipt->status_scope = "BOOT";
        out_receipt->status = boot_status
            ? boot_status
            : "NEXUS DATA ERROR";
    } else if (!out_receipt->title_asset_handoff_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = "blocked-title-assets";
    } else if (out_receipt->menu_bpk_prs3_blocks_real_menu_route) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = "blocked-menu-bpk-prs3";
    } else if (!out_receipt->real_menu_asset_handoff_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-menu-assets";
    } else if (out_receipt->main_menu_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "main-menu-ready";
    } else {
        out_receipt->route = NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = out_receipt->audio_asset_handoff_ready
            ? "title-ready"
            : "blocked-track02-sfx";
    }
    return 1;
}

int nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupAssetHandoffReceipt *out_receipt)
{
    if (!runtime) {
        nexus_v1_launcher_startup_asset_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_asset_handoff_from_parts(
        runtime->engine,
        runtime->level_loaded,
        runtime->title_loaded,
        &runtime->startup_assets,
        runtime->startup_receipt.host_receipt.status,
        out_receipt);
}

static void nexus_v1_launcher_resume_receipt_clear(
    Nexus_V1_LauncherResumeReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

static int nexus_v1_surface_loaded(const Nexus_V1_Engine *engine,
                                   Nexus_UISurfaceType surface)
{
    if (!engine || surface < 0 || surface >= NEXUS_SURFACE_COUNT) {
        return 0;
    }
    return engine->ui.surfaces[surface].data &&
           engine->ui.surfaces[surface].w > 0 &&
           engine->ui.surfaces[surface].h > 0;
}

static void nexus_v1_launcher_fill_startup_assets_receipt(
    const Nexus_V1_Engine *engine,
    int title_loaded,
    Nexus_V1_LauncherStartupAssetsReceipt *receipt)
{
    Nexus_V1_BpkRuntimeUploadReceipt bpk;
    Nexus_SfxRuntimeReceipt sfx;

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->menu_bpk_upload_route = NEXUS_V1_BPK_UPLOAD_ROUTE_INVALID;
    receipt->startup_sfx_status = NEXUS_SFX_RUNTIME_MISSING;
    receipt->startup_sfx_level_index = -1;
    receipt->startup_cd_track = -1;
    if (!engine) {
        return;
    }

    receipt->title_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_TITLE);
    receipt->warning_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_WARNING);
    receipt->gameover_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_GAMEOVER);
    receipt->status_bg_surface_loaded =
        nexus_v1_surface_loaded(engine, NEXUS_SURFACE_STABG);
    receipt->title_screen_loaded = title_loaded ? 1 : 0;
    receipt->startup_surfaces_loaded =
        nexus_v1_startup_surfaces_loaded_count(engine);
    receipt->startup_surfaces_expected =
        nexus_v1_startup_surfaces_expected_count(engine);
    receipt->startup_surfaces_fallback =
        nexus_v1_startup_surfaces_fallback_count(engine);
    receipt->faces_loaded = nexus_v1_startup_faces_loaded_count(engine);
    receipt->faces_expected = nexus_v1_startup_faces_expected_count(engine);
    receipt->faces_fallback = nexus_v1_startup_faces_fallback_count(engine);

    memset(&bpk, 0, sizeof(bpk));
    if (nexus_v1_menu_bpk_upload_plan_receipt(engine, &bpk) == 0) {
        receipt->menu_bpk_upload_receipt_valid = 1;
        receipt->menu_bpk_upload_route = bpk.route;
        receipt->menu_bpk_planned_rows = (int)bpk.planned_rows;
        receipt->menu_bpk_blocked_prs3_uploads =
            (int)bpk.blocked_prs3_uploads;
        receipt->menu_bpk_blocks_real_menu_surface_render =
            bpk.blocks_real_menu_surface_render;
        receipt->menu_bpk_fallback_visuals_permitted =
            bpk.fallback_visuals_permitted;
    }

    memset(&sfx, 0, sizeof(sfx));
    if (nexus_v1_current_level_sfx_runtime_receipt(engine, &sfx) == 0) {
        receipt->startup_sfx_status = sfx.status;
        receipt->startup_sfx_level_index = sfx.level_index;
        receipt->startup_cd_track = sfx.cd_track;
        receipt->startup_sfx_blocks_real_playback =
            sfx.blocks_real_sfx_playback;
        receipt->startup_audio_handoff_ready =
            sfx.level_index == 0 && sfx.cd_track == 2 ? 1 : 0;
    }

    receipt->startup_assets_ready =
        receipt->title_screen_loaded &&
        nexus_v1_startup_surfaces_ready(engine) &&
        nexus_v1_startup_faces_ready(engine);
    receipt->main_menu_route_ready =
        engine->level_loaded &&
        receipt->startup_assets_ready &&
        receipt->startup_audio_handoff_ready;
    receipt->title_route_ready =
        receipt->title_screen_loaded &&
        nexus_v1_startup_surfaces_ready(engine);
    receipt->real_menu_surface_route_ready =
        receipt->menu_bpk_upload_receipt_valid &&
        receipt->menu_bpk_upload_route == NEXUS_V1_BPK_UPLOAD_ROUTE_READY_STORED &&
        !receipt->menu_bpk_blocks_real_menu_surface_render;
    receipt->real_menu_surface_route_blocked =
        receipt->real_menu_surface_route_ready ? 0 : 1;
    if (!nexus_v1_startup_surfaces_ready(engine)) {
        receipt->real_menu_surface_blocker = "startup-surfaces";
        receipt->startup_menu_asset_route = "blocked-startup-surfaces";
    } else if (!nexus_v1_startup_faces_ready(engine)) {
        receipt->real_menu_surface_blocker = "faces";
        receipt->startup_menu_asset_route = "blocked-faces";
    } else if (!receipt->menu_bpk_upload_receipt_valid) {
        receipt->real_menu_surface_blocker = "menu-bpk";
        receipt->startup_menu_asset_route = "blocked-menu-bpk";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_PRS3) {
        receipt->real_menu_surface_blocker = "menu-bpk-prs3";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-prs3";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_BLOCKED_TRUNCATED) {
        receipt->real_menu_surface_blocker = "menu-bpk-truncated";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-truncated";
    } else if (receipt->menu_bpk_upload_route ==
               NEXUS_V1_BPK_UPLOAD_ROUTE_NO_SURFACES) {
        receipt->real_menu_surface_blocker = "menu-bpk-no-surfaces";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-no-surfaces";
    } else if (receipt->real_menu_surface_route_ready) {
        receipt->real_menu_surface_blocker = "none";
        receipt->startup_menu_asset_route = "ready-real-menu-surfaces";
    } else {
        receipt->real_menu_surface_blocker = "menu-bpk-invalid";
        receipt->startup_menu_asset_route = "blocked-menu-bpk-invalid";
    }
    receipt->save_menu_route_ready =
        receipt->startup_audio_handoff_ready &&
        receipt->real_menu_surface_route_ready;
    receipt->champion_menu_route_ready =
        receipt->save_menu_route_ready &&
        nexus_v1_startup_faces_ready(engine);
}

static int nexus_v1_launcher_startup_assets_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_assets)
{
    if (!state || !state->engine || !out_assets) {
        return 0;
    }
    nexus_v1_launcher_fill_startup_assets_receipt(
        state->engine,
        1,
        out_assets);
    return 1;
}

static void nexus_v1_launcher_fill_save_asset_blocked_route(
    const Nexus_V1_StartupHostFacts *facts,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    Nexus_V1_StartupDrawCommand commands[80];

    nexus_v1_startup_save_route_receipt_clear(out_receipt);
    if (!facts || !assets || !out_receipt) {
        return;
    }
    out_receipt->route = NEXUS_V1_STARTUP_SAVE_ROUTE_ASSET_BLOCKED;
    out_receipt->host_input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->status_scope = "ASSETS";
    out_receipt->status = assets->startup_menu_asset_route
        ? assets->startup_menu_asset_route
        : "blocked-startup-assets";
    if (nexus_v1_startup_menu_state_receipt_from_facts(
            &out_receipt->save_state_receipt,
            facts->save_dir,
            facts->slot_mask,
            facts->save_selected_row)) {
        out_receipt->save_state_receipt_valid = 1;
        out_receipt->row_count = out_receipt->save_state_receipt.row_count;
        out_receipt->selected_row =
            out_receipt->save_state_receipt.selected_row;
        out_receipt->slot_mask =
            (int)out_receipt->save_state_receipt.slot_mask;
        out_receipt->draw_command_count =
            nexus_v1_startup_presentation_build_save_from_facts(
                out_receipt->save_state_receipt.save_dir,
                out_receipt->save_state_receipt.slot_mask,
                out_receipt->save_state_receipt.selected_row,
                commands,
                (int)(sizeof(commands) / sizeof(commands[0])));
    }
}

static void nexus_v1_launcher_fill_save_asset_blocked_action(
    const Nexus_V1_StartupHostFacts *facts,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
        out_execution->kind = NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE;
        out_execution->status_scope = "ASSETS";
        out_execution->status = assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    }
    if (!out_receipt) {
        return;
    }
    nexus_v1_startup_host_action_receipt_clear(out_receipt);
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = "ASSETS";
    out_receipt->host_receipt.status =
        assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    if (facts && nexus_v1_startup_menu_state_receipt_from_facts(
            &out_receipt->save_state_receipt,
            facts->save_dir,
            facts->slot_mask,
            facts->save_selected_row)) {
        out_receipt->save_state_receipt_valid = 1;
    }
}

static int nexus_v1_launcher_startup_save_assets_blocked(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_assets)
{
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             out_assets)) {
        return 0;
    }
    return out_assets->save_menu_route_ready ? 0 : 1;
}

static int nexus_v1_launcher_startup_champion_assets_blocked(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_assets)
{
    if (!nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             out_assets)) {
        return 0;
    }
    return out_assets->champion_menu_route_ready ? 0 : 1;
}

static void nexus_v1_launcher_fill_champion_asset_blocked_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
        out_execution->kind = NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE;
        out_execution->cursor = -1;
        out_execution->status_scope = "ASSETS";
        out_execution->status = assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    }
    if (!out_receipt) {
        return;
    }
    nexus_v1_startup_host_action_receipt_clear(out_receipt);
    out_receipt->host_receipt.input_result =
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    out_receipt->host_receipt.status_scope = "ASSETS";
    out_receipt->host_receipt.status =
        assets && assets->startup_menu_asset_route
            ? assets->startup_menu_asset_route
            : "blocked-startup-assets";
    if (facts && nexus_v1_startup_champion_state_receipt_from_facts(
            facts->champion_pool,
            &out_receipt->champion_state_receipt,
            facts->slot_mask,
            facts->champion_cursor,
            facts->champion_frame)) {
        out_receipt->champion_state_receipt_valid = 1;
    }
}

void nexus_v1_launcher_startup_runtime_state_clear(
    Nexus_V1_StartupRuntimeState *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
}

void nexus_v1_launcher_runtime_startup_snapshot_clear(
    Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot)
{
    if (!snapshot) {
        return;
    }
    memset(snapshot, 0, sizeof(*snapshot));
}

void nexus_v1_launcher_startup_menu_presentation_receipt_clear(
    Nexus_V1_StartupMenuPresentationReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->kind = NEXUS_V1_STARTUP_MENU_PRESENTATION_INVALID;
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_title_handoff_receipt_clear(
    Nexus_V1_StartupTitleHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_startup_title_route_receipt_clear(&receipt->title_route);
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
    Nexus_V1_StartupRuntimeHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID;
    receipt->dgn_handoff.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->render_plan.status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->script_receipt.status = NEXUS_SCRIPT_RUNTIME_MISSING;
    receipt->script_receipt.level_index = -1;
    receipt->party_x = -1;
    receipt->party_y = -1;
    receipt->party_dir = -1;
    nexus_v1_startup_host_action_receipt_clear(
        &receipt->host_action_receipt);
}

void nexus_v1_launcher_startup_runtime_route_receipt_clear(
    Nexus_V1_StartupRuntimeRouteReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID;
    receipt->startup_sfx_status = NEXUS_SFX_RUNTIME_MISSING;
    receipt->startup_sfx_level_index = -1;
    receipt->startup_cd_track = -1;
    receipt->dgn_handoff_status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->dgn_render_plan_status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
    receipt->script_runtime_status = NEXUS_SCRIPT_RUNTIME_MISSING;
    nexus_v1_startup_host_action_receipt_clear(&receipt->host_action_receipt);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
        &receipt->runtime_handoff);
}

void nexus_v1_launcher_startup_route_proof_receipt_clear(
    Nexus_V1_StartupRouteProofReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_INVALID;
    nexus_v1_launcher_startup_launch_gate_receipt_clear(
        &receipt->launch_gate);
    nexus_v1_launcher_startup_asset_handoff_receipt_clear(
        &receipt->asset_handoff);
    nexus_v1_launcher_startup_title_handoff_receipt_clear(
        &receipt->title_handoff);
    nexus_v1_launcher_startup_menu_presentation_receipt_clear(
        &receipt->menu_presentation);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(
        &receipt->runtime_route_receipt);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
        &receipt->runtime_handoff);
}

void nexus_v1_launcher_startup_full_start_receipt_clear(
    Nexus_V1_StartupFullStartReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = NEXUS_V1_STARTUP_FULL_START_INVALID;
    receipt->cd_track = -1;
    receipt->sfx_status = NEXUS_SFX_RUNTIME_MISSING;
    nexus_v1_launcher_startup_launch_gate_receipt_clear(
        &receipt->launch_gate);
    nexus_v1_launcher_startup_asset_handoff_receipt_clear(
        &receipt->asset_handoff);
    nexus_v1_startup_host_receipt_clear(&receipt->host_receipt);
}

void nexus_v1_launcher_startup_full_start_consumer_receipt_clear(
    Nexus_V1_StartupFullStartConsumerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    nexus_v1_launcher_startup_full_start_receipt_clear(
        &receipt->full_start);
    nexus_v1_launcher_startup_title_handoff_receipt_clear(
        &receipt->title_handoff);
    nexus_v1_launcher_startup_menu_presentation_receipt_clear(
        &receipt->presentation);
    nexus_v1_startup_save_route_receipt_clear(&receipt->save_route);
}

const char *nexus_v1_launcher_startup_runtime_handoff_route_name(
    Nexus_V1_StartupRuntimeHandoffRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED:
        return "asset-blocked";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START: return "not-start";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED:
        return "dgn-blocked";
    case NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE:
        return "ready-render-state";
    default: return "unknown";
    }
}

const char *nexus_v1_launcher_startup_route_proof_route_name(
    Nexus_V1_StartupRouteProofRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_ROUTE_PROOF_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED:
        return "asset-blocked";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_TITLE_READY: return "title-ready";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_MENU_READY: return "menu-ready";
    case NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY:
        return "runtime-ready";
    default: return "unknown";
    }
}

const char *nexus_v1_launcher_startup_full_start_route_name(
    Nexus_V1_StartupFullStartRoute route)
{
    switch (route) {
    case NEXUS_V1_STARTUP_FULL_START_INVALID: return "invalid";
    case NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS:
        return "blocked-assets";
    case NEXUS_V1_STARTUP_FULL_START_WARNING_TITLE_READY:
        return "warning-title-ready";
    case NEXUS_V1_STARTUP_FULL_START_MENU_READY: return "menu-ready";
    default: return "unknown";
    }
}

static void nexus_v1_launcher_fill_full_start_host_route(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupFullStartReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    receipt->host_receipt.input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->host_receipt.status_scope = receipt->status_scope;
    receipt->host_receipt.status = receipt->status;
    receipt->m11_host_route = "blocked-startup";
    receipt->m11_host_route_ready = 0;

    if (!state || !receipt->full_start_menu_ready) {
        return;
    }
    receipt->host_receipt.status_scope = "STARTUP";
    if (state->save_select_active && receipt->save_status_ready) {
        receipt->m11_host_route = "save-menu";
        receipt->host_receipt.status = "NEXUS SAVE SELECT";
        receipt->m11_host_route_ready = 1;
    } else if (state->champion_select_active &&
               receipt->champion_status_ready) {
        receipt->m11_host_route = "champion-menu";
        receipt->host_receipt.status = "NEXUS CHAMPIONS";
        receipt->m11_host_route_ready = 1;
    } else if (state->title_active && receipt->title_status_ready) {
        receipt->m11_host_route = "title-warning";
        receipt->host_receipt.status = "NEXUS TITLE";
        receipt->m11_host_route_ready = 1;
    } else {
        receipt->m11_host_route = "startup-menu";
        receipt->host_receipt.status = "full-start-menu-ready";
        receipt->m11_host_route_ready = 1;
    }
}

int nexus_v1_launcher_startup_host_facts_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupHostFacts *out_facts)
{
    if (!out_facts) {
        return 0;
    }
    memset(out_facts, 0, sizeof(*out_facts));
    if (!state) {
        return 0;
    }
    out_facts->title_active = state->title_active;
    out_facts->title_frame = state->title_frame;
    out_facts->save_select_active = state->save_select_active;
    out_facts->champion_select_active = state->champion_select_active;
    out_facts->save_dir = state->save_dir;
    out_facts->slot_mask = state->slot_mask;
    out_facts->save_selected_row = state->save_selected_row;
    out_facts->save_row_count = state->save_row_count;
    out_facts->champion_pool = state->engine ? &state->engine->champions : NULL;
    out_facts->champion_cursor = state->champion_cursor;
    out_facts->champion_frame = state->champion_frame;
    return 1;
}

int nexus_v1_launcher_startup_host_facts_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupHostFacts *out_facts)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_host_facts_from_runtime_state(
        &snapshot->runtime,
        out_facts);
}

int nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    return nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                               out_receipt);
}

int nexus_v1_launcher_startup_assets_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt)
{
    if (!snapshot) {
        if (out_receipt) {
            memset(out_receipt, 0, sizeof(*out_receipt));
        }
        return 0;
    }
    return nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

int nexus_v1_launcher_startup_advance_idle_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupIdleReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
        &facts,
        out_receipt);
}

int nexus_v1_launcher_startup_advance_idle_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupIdleReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_advance_idle_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_action(&facts,
                                                         &assets,
                                                         out_execution,
                                                         out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_save_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_save_firestaff_input_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_action(&facts,
                                                         &assets,
                                                         out_execution,
                                                         out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_save_pointer_from_host_facts_with_receipt(
        &facts,
        x,
        y,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_save_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
        &snapshot->runtime,
        x,
        y,
        load_save,
        load_userdata,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_route(&facts,
                                                        &assets,
                                                        out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_save_route_receipt_from_host_facts_input(
        &facts,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_save_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_save_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_save_asset_blocked_route(&facts,
                                                        &assets,
                                                        out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_save_route_receipt_from_host_facts_pointer(
        &facts,
        x,
        y,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_save_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
        &snapshot->runtime,
        x,
        y,
        load_save,
        load_userdata,
        out_receipt);
}

static void nexus_v1_launcher_fill_title_asset_blocked_action(
    const Nexus_V1_StartupTitleHandoffReceipt *handoff,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (out_execution) {
        memset(out_execution, 0, sizeof(*out_execution));
        out_execution->kind = NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE;
        out_execution->status_scope = "ASSETS";
        out_execution->status = handoff && handoff->status
            ? handoff->status
            : "blocked-startup-assets";
    }
    if (!out_receipt) {
        return;
    }
    nexus_v1_startup_host_action_receipt_clear(out_receipt);
    if (handoff) {
        out_receipt->host_receipt = handoff->host_receipt;
    }
}

int nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_StartupTitleHandoffReceipt handoff;
    if (nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
            state,
            menu_input,
            &handoff) &&
        handoff.route_blocked) {
        nexus_v1_launcher_fill_title_asset_blocked_action(&handoff,
                                                          out_execution,
                                                          out_receipt);
        return out_receipt ? 1 : 0;
    }
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_execute_title_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_title_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_StartupTitleHandoffReceipt handoff;
    if (nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
            state,
            &handoff) &&
        handoff.route_blocked) {
        nexus_v1_launcher_fill_title_asset_blocked_action(&handoff,
                                                          out_execution,
                                                          out_receipt);
        return out_receipt ? 1 : 0;
    }
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_execute_title_pointer_from_host_facts_with_receipt(
        &facts,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_title_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
        &snapshot->runtime,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_title_route_receipt_from_host_facts_input(
        &facts,
        menu_input,
        out_receipt);
}

int nexus_v1_launcher_startup_title_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_receipt);
}

int nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_title_route_receipt_from_host_facts_pointer(
        &facts,
        out_receipt);
}

int nexus_v1_launcher_startup_title_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

static int nexus_v1_launcher_title_route_asset_ready(
    Nexus_V1_StartupTitleRoute route,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets)
{
    if (!assets) {
        return 0;
    }
    switch (route) {
    case NEXUS_V1_STARTUP_TITLE_ROUTE_SAVE_SELECT:
        return assets->save_menu_route_ready ? 1 : 0;
    case NEXUS_V1_STARTUP_TITLE_ROUTE_CHAMPION_SELECT:
        return assets->champion_menu_route_ready ? 1 : 0;
    case NEXUS_V1_STARTUP_TITLE_ROUTE_HOLD:
    case NEXUS_V1_STARTUP_TITLE_ROUTE_RETURN_TO_LAUNCHER:
        return 1;
    case NEXUS_V1_STARTUP_TITLE_ROUTE_INVALID:
    default:
        return 0;
    }
}

static void nexus_v1_launcher_fill_title_handoff_receipt(
    Nexus_V1_StartupTitleHandoffReceipt *receipt,
    const Nexus_V1_StartupTitleRouteReceipt *title_route,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets)
{
    int route_ready;

    if (!receipt || !title_route || !assets) {
        return;
    }
    route_ready = nexus_v1_launcher_title_route_asset_ready(
        title_route->route,
        assets);
    nexus_v1_launcher_startup_title_handoff_receipt_clear(receipt);
    receipt->title_route = *title_route;
    receipt->assets = *assets;
    receipt->title_draw_ready = assets->title_route_ready ? 1 : 0;
    receipt->save_menu_ready = assets->save_menu_route_ready ? 1 : 0;
    receipt->champion_menu_ready = assets->champion_menu_route_ready ? 1 : 0;
    receipt->route_ready = route_ready;
    receipt->route_blocked = route_ready ? 0 : 1;
    receipt->asset_route = assets->startup_menu_asset_route;
    receipt->asset_blocker = assets->real_menu_surface_blocker;
    receipt->status_scope = route_ready ? title_route->status_scope : "ASSETS";
    receipt->status = route_ready
        ? title_route->status
        : (assets->startup_menu_asset_route
               ? assets->startup_menu_asset_route
               : "blocked-startup-assets");
    receipt->host_receipt.input_result =
        route_ready ? title_route->host_input_result
                    : NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->host_receipt.status_scope = receipt->status_scope;
    receipt->host_receipt.status = receipt->status;
    if (route_ready) {
        receipt->host_receipt.mode_update.set_title_active =
            title_route->set_title_active;
        receipt->host_receipt.mode_update.title_active =
            title_route->title_active;
        receipt->host_receipt.mode_update.set_title_frame =
            title_route->set_title_frame;
        receipt->host_receipt.mode_update.title_frame =
            title_route->next_title_frame;
        receipt->host_receipt.mode_update.set_save_select_active =
            title_route->set_save_select_active;
        receipt->host_receipt.mode_update.save_select_active =
            title_route->save_select_active;
        receipt->host_receipt.mode_update.set_save_selected_row =
            title_route->set_save_selected_row;
        receipt->host_receipt.mode_update.save_selected_row =
            title_route->save_selected_row;
        receipt->host_receipt.mode_update.set_champion_select_active =
            title_route->set_champion_select_active;
        receipt->host_receipt.mode_update.champion_select_active =
            title_route->champion_select_active;
        receipt->host_receipt.mode_update.set_champion_cursor =
            title_route->set_champion_cursor;
        receipt->host_receipt.mode_update.champion_cursor =
            title_route->champion_cursor;
    }
}

int nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupTitleRouteReceipt title_route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;

    nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
            state,
            menu_input,
            &title_route) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    nexus_v1_launcher_fill_title_handoff_receipt(out_receipt,
                                                 &title_route,
                                                 &assets);
    return 1;
}

int nexus_v1_launcher_startup_title_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_receipt);
}

int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupTitleRouteReceipt title_route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;

    nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
            state,
            &title_route) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    nexus_v1_launcher_fill_title_handoff_receipt(out_receipt,
                                                 &title_route,
                                                 &assets);
    return 1;
}

int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_title_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
        &snapshot->runtime,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_champion_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_champion_asset_blocked_receipt(
            &facts,
            &assets,
            out_execution,
            out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_champion_firestaff_input_from_host_facts_with_receipt(
        &facts,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
        &snapshot->runtime,
        menu_input,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    if (nexus_v1_launcher_startup_champion_assets_blocked(state, &assets)) {
        nexus_v1_launcher_fill_champion_asset_blocked_receipt(
            &facts,
            &assets,
            out_execution,
            out_receipt);
        return out_receipt ? 1 : 0;
    }
    return nexus_v1_startup_execute_champion_pointer_from_host_facts_with_receipt(
        &facts,
        x,
        y,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_execute_champion_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
        &snapshot->runtime,
        x,
        y,
        out_execution,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;
    Nexus_V1_DgnRenderPlanReceipt render_plan;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_ScriptRuntimeReceipt script_receipt;

    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
    if (!state || !execution || !out_receipt ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets) ||
        !nexus_v1_launcher_startup_asset_handoff_from_parts(
            state->engine,
            state->engine ? state->engine->level_loaded : 0,
            assets.title_route_ready,
            &assets,
            NULL,
            &asset_handoff)) {
        return 0;
    }

    out_receipt->champion_execution = *execution;
    if (host_action) {
        out_receipt->host_action_receipt = *host_action;
    }
    out_receipt->assets = assets;
    out_receipt->asset_handoff = asset_handoff;
    out_receipt->asset_route = assets.startup_menu_asset_route;
    out_receipt->fallback_visuals_permitted =
        asset_handoff.fallback_visuals_permitted;

    if (asset_handoff.blocks_main_menu_route ||
        !asset_handoff.real_asset_route_ready ||
        !assets.champion_menu_route_ready) {
        out_receipt->route =
            NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = asset_handoff.status
            ? asset_handoff.status
            : assets.startup_menu_asset_route
            ? assets.startup_menu_asset_route
            : "blocked-startup-assets";
        return 1;
    }

    if (execution->kind != NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START;
        out_receipt->status_scope = execution->status_scope;
        out_receipt->status = execution->status;
        return 1;
    }

    out_receipt->level_loaded = state->engine && state->engine->level_loaded;
    if (!state->engine || !state->engine->level_loaded ||
        nexus_v1_current_level_dgn_renderer_handoff_receipt(
            state->engine,
            &dgn_handoff) != 0) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->status_scope = "DGN";
        out_receipt->status = "missing-dgn-runtime";
        return 1;
    }

    out_receipt->dgn_handoff = dgn_handoff;
    out_receipt->dgn_route =
        nexus_v1_dgn_renderer_handoff_status_name(dgn_handoff.status);
    memset(&render_plan, 0, sizeof(render_plan));
    if (nexus_v1_level_build_dgn_view_render_plan(
            &state->engine->current_level,
            state->engine->game.party_x,
            state->engine->game.party_y,
            state->engine->game.party_dir,
            out_commands,
            max_commands,
            &render_plan) != 0 ||
        !render_plan.plan_ready ||
        render_plan.blocks_real_dgn_mesh_render) {
        out_receipt->route = NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
        out_receipt->render_plan = render_plan;
        out_receipt->dgn_render_blocked = 1;
        out_receipt->fallback_visuals_permitted =
            render_plan.fallback_visuals_permitted;
        out_receipt->status_scope = "DGN";
        out_receipt->status = out_receipt->dgn_route
            ? out_receipt->dgn_route
            : "blocked-dgn-render";
        return 1;
    }

    out_receipt->route =
        NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE;
    out_receipt->render_plan = render_plan;
    memset(&script_receipt, 0, sizeof(script_receipt));
    if (nexus_v1_current_level_script_runtime_receipt(
            state->engine,
            &script_receipt) == 0) {
        out_receipt->script_receipt = script_receipt;
        out_receipt->script_runtime_blocked =
            script_receipt.blocks_real_script_dispatch ? 1 : 0;
        out_receipt->script_runtime_ready =
            !out_receipt->script_runtime_blocked &&
            script_receipt.status != NEXUS_SCRIPT_RUNTIME_MISSING;
    }
    out_receipt->runtime_ready = 1;
    out_receipt->dgn_render_ready = 1;
    out_receipt->hud_ready = out_receipt->level_loaded ? 1 : 0;
    out_receipt->dgn_render_blocked = 0;
    out_receipt->party_x = state->engine->game.party_x;
    out_receipt->party_y = state->engine->game.party_y;
    out_receipt->party_dir = state->engine->game.party_dir;
    out_receipt->command_count = render_plan.command_count;
    out_receipt->fallback_visuals_permitted =
        render_plan.fallback_visuals_permitted;
    out_receipt->status_scope = "DGN";
    out_receipt->status = "ready-render-state";
    return 1;
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
        &snapshot->runtime,
        execution,
        host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
            state,
            menu_input,
            &execution,
            &host_action)) {
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
        state,
        &execution,
        &host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
            state,
            x,
            y,
            &execution,
            &host_action)) {
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
        state,
        &execution,
        &host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
        &snapshot->runtime,
        menu_input,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_handoff_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer(
        &snapshot->runtime,
        x,
        y,
        out_commands,
        max_commands,
        out_receipt);
}

static void nexus_v1_launcher_fill_runtime_route_receipt(
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    const Nexus_V1_StartupRuntimeHandoffReceipt *handoff,
    const Nexus_V1_DgnRenderCommand *commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    if (!out_receipt || !handoff) {
        return;
    }
    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (execution) {
        out_receipt->champion_execution = *execution;
    }
    if (host_action) {
        out_receipt->host_action_receipt = *host_action;
        out_receipt->host_action_valid = 1;
    }
    out_receipt->runtime_handoff = *handoff;
    out_receipt->route = handoff->route;
    out_receipt->runtime_route_ready = handoff->runtime_ready ? 1 : 0;
    out_receipt->runtime_route_blocked =
        handoff->route == NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED ||
        handoff->route == NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED;
    out_receipt->startup_sfx_status = handoff->assets.startup_sfx_status;
    out_receipt->startup_sfx_level_index =
        handoff->assets.startup_sfx_level_index;
    out_receipt->startup_cd_track = handoff->assets.startup_cd_track;
    out_receipt->startup_audio_handoff_ready =
        handoff->assets.startup_audio_handoff_ready ? 1 : 0;
    out_receipt->startup_sfx_blocks_real_playback =
        handoff->assets.startup_sfx_blocks_real_playback ? 1 : 0;
    out_receipt->dgn_handoff_status = handoff->dgn_handoff.status;
    out_receipt->dgn_render_plan_status = handoff->render_plan.status;
    out_receipt->dgn_render_plan_ready =
        handoff->render_plan.plan_ready ? 1 : 0;
    out_receipt->dgn_render_command_count =
        handoff->render_plan.command_count;
    out_receipt->dgn_render_floor_count = handoff->render_plan.floor_count;
    out_receipt->dgn_render_wall_count = handoff->render_plan.wall_count;
    out_receipt->dgn_blocks_real_mesh_render =
        handoff->render_plan.blocks_real_dgn_mesh_render ? 1 : 0;
    if (commands && handoff->render_plan.command_count > 0) {
        out_receipt->first_dgn_render_command_kind = commands[0].kind;
    }
    out_receipt->script_runtime_status = handoff->script_receipt.status;
    out_receipt->script_runtime_ready =
        handoff->script_runtime_ready ? 1 : 0;
    out_receipt->script_runtime_blocked =
        handoff->script_runtime_blocked ? 1 : 0;
    out_receipt->script_candidate_source_bytes =
        handoff->script_receipt.candidate_source_bytes;
    out_receipt->script_rules_loaded = handoff->script_receipt.rules_loaded;
    out_receipt->consumed_by_nexus =
        handoff->route != NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID;
    out_receipt->fallback_visuals_permitted =
        handoff->fallback_visuals_permitted ? 1 : 0;
    out_receipt->status_scope = handoff->status_scope;
    out_receipt->status = handoff->status;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupRuntimeHandoffReceipt handoff;

    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
            state,
            execution,
            host_action,
            out_commands,
            max_commands,
            &handoff)) {
        return 0;
    }
    nexus_v1_launcher_fill_runtime_route_receipt(
        execution,
        host_action,
        &handoff,
        out_commands,
        out_receipt);
    return 1;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
            state,
            menu_input,
            &execution,
            &host_action) ||
        !nexus_v1_launcher_startup_runtime_route_from_champion_execution(
            state,
            &execution,
            &host_action,
            out_commands,
            max_commands,
            out_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    Nexus_V1_StartupChampionExecution execution;
    Nexus_V1_StartupHostActionReceipt host_action;

    memset(&execution, 0, sizeof(execution));
    nexus_v1_startup_host_action_receipt_clear(&host_action);
    nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
    if (!nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
            state,
            x,
            y,
            &execution,
            &host_action) ||
        !nexus_v1_launcher_startup_runtime_route_from_champion_execution(
            state,
            &execution,
            &host_action,
            out_commands,
            max_commands,
            out_receipt)) {
        return 0;
    }
    return 1;
}

int nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
        &snapshot->runtime,
        menu_input,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_runtime_route_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_runtime_route_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_runtime_route_from_champion_pointer(
        &snapshot->runtime,
        x,
        y,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_route_proof_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupDrawCommand draw_commands[80];

    nexus_v1_launcher_startup_route_proof_receipt_clear(out_receipt);
    if (!out_receipt || !runtime || !state ||
        !nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
            runtime,
            &out_receipt->launch_gate) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    if (!nexus_v1_launcher_startup_asset_handoff_from_parts(
            state->engine,
            state->engine ? state->engine->level_loaded : runtime->level_loaded,
            assets.title_route_ready,
            &assets,
            runtime->startup_receipt.host_receipt.status,
            &out_receipt->asset_handoff)) {
        return 0;
    }

    out_receipt->assets = assets;
    out_receipt->saturn_asset_boot_ready =
        out_receipt->launch_gate.engine_ready &&
        out_receipt->launch_gate.level_loaded &&
        assets.title_route_ready &&
        assets.startup_audio_handoff_ready;
    out_receipt->title_route_ready = assets.title_route_ready ? 1 : 0;
    out_receipt->menu_route_ready =
        assets.save_menu_route_ready && assets.champion_menu_route_ready;
    out_receipt->title_art_loaded = assets.title_surface_loaded ? 1 : 0;
    out_receipt->warning_art_loaded = assets.warning_surface_loaded ? 1 : 0;
    out_receipt->startup_surfaces_real_ready =
        assets.startup_surfaces_expected > 0 &&
        assets.startup_surfaces_loaded == assets.startup_surfaces_expected &&
        assets.startup_surfaces_fallback == 0;
    out_receipt->faces_real_ready =
        assets.faces_expected > 0 &&
        assets.faces_loaded == assets.faces_expected &&
        assets.faces_fallback == 0;
    out_receipt->full_start_graphics_ready =
        out_receipt->startup_surfaces_real_ready &&
        out_receipt->faces_real_ready &&
        assets.real_menu_surface_route_ready;
    out_receipt->save_load_menu_route_ready =
        assets.save_menu_route_ready &&
        out_receipt->full_start_graphics_ready;
    out_receipt->startup_ui_route_ready =
        out_receipt->title_route_ready &&
        out_receipt->full_start_graphics_ready &&
        assets.startup_audio_handoff_ready;
    if (!out_receipt->title_route_ready) {
        out_receipt->startup_ui_blocker = "title";
    } else if (!out_receipt->startup_surfaces_real_ready) {
        out_receipt->startup_ui_blocker = "startup-surfaces";
    } else if (!out_receipt->faces_real_ready) {
        out_receipt->startup_ui_blocker = "faces";
    } else if (!assets.real_menu_surface_route_ready) {
        out_receipt->startup_ui_blocker =
            assets.real_menu_surface_blocker
                ? assets.real_menu_surface_blocker
                : "menu-bpk";
    } else if (!assets.startup_audio_handoff_ready) {
        out_receipt->startup_ui_blocker = "track02-sfx";
    } else {
        out_receipt->startup_ui_blocker = "none";
    }
    out_receipt->audio_ready = assets.startup_audio_handoff_ready ? 1 : 0;
    out_receipt->startup_sfx_status = assets.startup_sfx_status;
    out_receipt->startup_sfx_level_index = assets.startup_sfx_level_index;
    out_receipt->startup_cd_track = assets.startup_cd_track;
    out_receipt->startup_sfx_blocks_real_playback =
        assets.startup_sfx_blocks_real_playback ? 1 : 0;
    out_receipt->title_menu_route_ready =
        out_receipt->title_route_ready && out_receipt->menu_route_ready;
    out_receipt->fallback_visuals_permitted =
        assets.menu_bpk_fallback_visuals_permitted ||
        out_receipt->launch_gate.fallback_visuals_permitted ||
        out_receipt->asset_handoff.fallback_visuals_permitted;
    out_receipt->asset_route = assets.startup_menu_asset_route;
    out_receipt->status_scope = out_receipt->launch_gate.status_scope;
    out_receipt->status = out_receipt->launch_gate.status;

    if (state->title_active) {
        (void)nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
            state,
            9,
            &out_receipt->title_handoff);
        out_receipt->title_route =
            nexus_v1_startup_title_route_name(
                out_receipt->title_handoff.title_route.route);
    }

    if (state->save_select_active) {
        if (nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->menu_presentation)) {
            out_receipt->menu_route = out_receipt->menu_presentation.status;
        }
    } else if (state->champion_select_active) {
        if (nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->menu_presentation)) {
            out_receipt->menu_route = out_receipt->menu_presentation.status;
        }
    }

    if (execution) {
        (void)nexus_v1_launcher_startup_runtime_route_from_champion_execution(
            state,
            execution,
            host_action,
            out_commands,
            max_commands,
            &out_receipt->runtime_route_receipt);
        out_receipt->runtime_handoff =
            out_receipt->runtime_route_receipt.runtime_handoff;
        out_receipt->runtime_route =
            nexus_v1_launcher_startup_runtime_handoff_route_name(
                out_receipt->runtime_route_receipt.route);
        out_receipt->runtime_route_ready =
            out_receipt->runtime_route_receipt.runtime_route_ready;
        out_receipt->menu_runtime_route_ready =
            out_receipt->menu_route_ready &&
            out_receipt->runtime_route_ready;
        out_receipt->first_runtime_route_ready =
            out_receipt->runtime_handoff.render_plan.plan_ready &&
            out_receipt->runtime_handoff.command_count > 0 &&
            !out_receipt->runtime_handoff.render_plan
                 .blocks_real_dgn_mesh_render;
        out_receipt->first_runtime_route =
            out_receipt->first_runtime_route_ready
                ? "first-dgn-render-state"
                : out_receipt->runtime_route;
        out_receipt->script_runtime_status =
            out_receipt->runtime_route_receipt.script_runtime_status;
        out_receipt->script_candidate_source_bytes =
            out_receipt->runtime_route_receipt.script_candidate_source_bytes;
        out_receipt->script_runtime_route_blocked =
            out_receipt->runtime_route_receipt.script_runtime_blocked;
        out_receipt->script_runtime_route_ready =
            out_receipt->runtime_route_receipt.script_runtime_ready;
    }

    out_receipt->graphics_ready =
        assets.title_route_ready &&
        out_receipt->full_start_graphics_ready &&
        (!execution || out_receipt->runtime_handoff.render_plan.plan_ready);
    out_receipt->audio_runtime_route_ready =
        out_receipt->audio_ready && out_receipt->first_runtime_route_ready;
    out_receipt->audio_runtime_route_blocked =
        out_receipt->audio_runtime_route_ready ? 0 : 1;
    out_receipt->full_startup_route_ready =
        out_receipt->saturn_asset_boot_ready &&
        out_receipt->title_menu_route_ready &&
        out_receipt->menu_runtime_route_ready &&
        out_receipt->first_runtime_route_ready &&
        out_receipt->audio_runtime_route_ready &&
        !out_receipt->script_runtime_route_blocked &&
        !out_receipt->fallback_visuals_permitted;

    if (out_receipt->runtime_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY;
        out_receipt->status_scope = "RUNTIME";
        out_receipt->status = "runtime-ready";
    } else if (out_receipt->launch_gate.route ==
                   NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR ||
               out_receipt->launch_gate.route ==
                   NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED ||
               !out_receipt->asset_handoff.title_asset_handoff_ready ||
               out_receipt->asset_handoff.blocks_main_menu_route ||
               !assets.title_route_ready ||
               assets.real_menu_surface_route_blocked) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED;
        out_receipt->status_scope = out_receipt->asset_handoff.status_scope
            ? out_receipt->asset_handoff.status_scope
            : out_receipt->launch_gate.status_scope
            ? out_receipt->launch_gate.status_scope
            : "ASSETS";
        out_receipt->status = out_receipt->asset_handoff.status
            ? out_receipt->asset_handoff.status
            : out_receipt->launch_gate.status
            ? out_receipt->launch_gate.status
            : (assets.startup_menu_asset_route
                   ? assets.startup_menu_asset_route
                   : "blocked-startup-assets");
    } else if (out_receipt->menu_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_MENU_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "menu-ready";
    } else if (out_receipt->title_route_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_ROUTE_PROOF_TITLE_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "title-ready";
    }
    return 1;
}

int nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupFullStartReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;

    nexus_v1_launcher_startup_full_start_receipt_clear(out_receipt);
    if (!out_receipt || !runtime || !state ||
        !nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
            runtime,
            &out_receipt->launch_gate) ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    if (!nexus_v1_launcher_startup_asset_handoff_from_parts(
            state->engine,
            state->engine ? state->engine->level_loaded : runtime->level_loaded,
            assets.title_route_ready,
            &assets,
            runtime->startup_receipt.host_receipt.status,
            &out_receipt->asset_handoff)) {
        return 0;
    }

    out_receipt->assets = assets;
    out_receipt->warning_art_loaded = assets.warning_surface_loaded ? 1 : 0;
    out_receipt->title_art_loaded = assets.title_surface_loaded ? 1 : 0;
    out_receipt->warning_status_ready = out_receipt->warning_art_loaded;
    out_receipt->title_status_ready =
        out_receipt->title_art_loaded && assets.title_route_ready;
    out_receipt->boot_warning_title_ready =
        out_receipt->warning_art_loaded &&
        out_receipt->title_art_loaded &&
        assets.title_route_ready;
    out_receipt->startup_surfaces_real_ready =
        assets.startup_surfaces_expected > 0 &&
        assets.startup_surfaces_loaded == assets.startup_surfaces_expected &&
        assets.startup_surfaces_fallback == 0;
    out_receipt->faces_real_ready =
        assets.faces_expected > 0 &&
        assets.faces_loaded == assets.faces_expected &&
        assets.faces_fallback == 0;
    out_receipt->menu_bpk_route_ready =
        assets.real_menu_surface_route_ready ? 1 : 0;
    out_receipt->save_menu_route_ready =
        assets.save_menu_route_ready ? 1 : 0;
    out_receipt->champion_menu_route_ready =
        assets.champion_menu_route_ready ? 1 : 0;
    out_receipt->audio_track02_ready =
        assets.startup_audio_handoff_ready ? 1 : 0;
    out_receipt->cd_track = assets.startup_cd_track;
    out_receipt->sfx_status = assets.startup_sfx_status;
    out_receipt->sfx_blocks_real_playback =
        assets.startup_sfx_blocks_real_playback ? 1 : 0;
    out_receipt->full_start_graphics_ready =
        out_receipt->boot_warning_title_ready &&
        out_receipt->startup_surfaces_real_ready &&
        out_receipt->faces_real_ready &&
        out_receipt->menu_bpk_route_ready;
    out_receipt->save_status_ready =
        out_receipt->full_start_graphics_ready &&
        out_receipt->save_menu_route_ready &&
        out_receipt->audio_track02_ready;
    out_receipt->champion_status_ready =
        out_receipt->full_start_graphics_ready &&
        out_receipt->champion_menu_route_ready &&
        out_receipt->audio_track02_ready;
    out_receipt->full_start_menu_ready =
        out_receipt->full_start_graphics_ready &&
        out_receipt->save_menu_route_ready &&
        out_receipt->champion_menu_route_ready &&
        out_receipt->audio_track02_ready &&
        !out_receipt->asset_handoff.fallback_visuals_permitted;
    out_receipt->fallback_visuals_permitted =
        out_receipt->asset_handoff.fallback_visuals_permitted;
    out_receipt->asset_route = assets.startup_menu_asset_route;

    if (!out_receipt->boot_warning_title_ready) {
        out_receipt->startup_ui_blocker = "title-warning";
    } else if (!out_receipt->startup_surfaces_real_ready) {
        out_receipt->startup_ui_blocker = "startup-surfaces";
    } else if (!out_receipt->faces_real_ready) {
        out_receipt->startup_ui_blocker = "faces";
    } else if (!out_receipt->menu_bpk_route_ready) {
        out_receipt->startup_ui_blocker =
            assets.real_menu_surface_blocker
                ? assets.real_menu_surface_blocker
                : "menu-bpk";
    } else if (!out_receipt->audio_track02_ready) {
        out_receipt->startup_ui_blocker = "track02-sfx";
    } else {
        out_receipt->startup_ui_blocker = "none";
    }

    if (out_receipt->full_start_menu_ready) {
        out_receipt->route = NEXUS_V1_STARTUP_FULL_START_MENU_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "full-start-menu-ready";
    } else if (out_receipt->boot_warning_title_ready &&
               !out_receipt->asset_handoff.blocks_main_menu_route) {
        out_receipt->route =
            NEXUS_V1_STARTUP_FULL_START_WARNING_TITLE_READY;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "warning-title-ready";
    } else {
        out_receipt->route = NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS;
        out_receipt->status_scope = "ASSETS";
        out_receipt->status = out_receipt->asset_handoff.status
            ? out_receipt->asset_handoff.status
            : out_receipt->startup_ui_blocker;
    }
    nexus_v1_launcher_fill_full_start_host_route(state, out_receipt);
    return 1;
}

int nexus_v1_launcher_startup_full_start_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupFullStartReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_full_start_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
        runtime,
        &snapshot->runtime,
        out_receipt);
}

static void nexus_v1_launcher_fill_full_start_consumer_status(
    Nexus_V1_StartupFullStartConsumerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    receipt->m11_ready = receipt->full_start.m11_host_route_ready ? 1 : 0;
    receipt->m12_ready = receipt->full_start.full_start_menu_ready ? 1 : 0;
    receipt->redraw =
        receipt->full_start.host_receipt.input_result ==
        NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->consumer_route = receipt->full_start.m11_host_route
        ? receipt->full_start.m11_host_route
        : "blocked-startup";
    receipt->status_scope = receipt->full_start.host_receipt.status_scope
        ? receipt->full_start.host_receipt.status_scope
        : receipt->full_start.status_scope;
    receipt->status = receipt->full_start.host_receipt.status
        ? receipt->full_start.host_receipt.status
        : receipt->full_start.status;
}

int nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartConsumerReceipt *out_receipt)
{
    Nexus_V1_StartupDrawCommand draw_commands[80];

    nexus_v1_launcher_startup_full_start_consumer_receipt_clear(out_receipt);
    if (!out_receipt || !runtime || !state ||
        !nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
            runtime,
            state,
            &out_receipt->full_start)) {
        return 0;
    }
    nexus_v1_launcher_fill_full_start_consumer_status(out_receipt);
    if (!out_receipt->full_start.full_start_menu_ready) {
        return 1;
    }

    if (state->title_active) {
        out_receipt->title_handoff_valid =
            nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
                state,
                menu_input,
                &out_receipt->title_handoff);
        if (out_receipt->title_handoff_valid) {
            out_receipt->draw_command_count =
                out_receipt->title_handoff.title_draw_ready ? 1 : 0;
        }
    } else if (state->save_select_active) {
        memset(draw_commands, 0, sizeof(draw_commands));
        out_receipt->presentation_valid =
            nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->presentation);
        if (out_receipt->presentation_valid) {
            out_receipt->draw_command_count =
                out_receipt->presentation.draw_command_count;
        }
        out_receipt->save_route_valid =
            nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
                state,
                menu_input,
                load_save,
                load_userdata,
                &out_receipt->save_route);
        if (out_receipt->save_route_valid) {
            out_receipt->save_row_count = out_receipt->save_route.row_count;
            out_receipt->selected_row = out_receipt->save_route.selected_row;
        }
    } else if (state->champion_select_active) {
        memset(draw_commands, 0, sizeof(draw_commands));
        out_receipt->presentation_valid =
            nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
                state,
                draw_commands,
                (int)(sizeof(draw_commands) / sizeof(draw_commands[0])),
                &out_receipt->presentation);
        if (out_receipt->presentation_valid) {
            out_receipt->draw_command_count =
                out_receipt->presentation.draw_command_count;
        }
    }
    return 1;
}

int nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartConsumerReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_full_start_consumer_receipt_clear(
            out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
        runtime,
        &snapshot->runtime,
        menu_input,
        load_save,
        load_userdata,
        out_receipt);
}

int nexus_v1_launcher_startup_route_proof_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_route_proof_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_route_proof_from_runtime_state(
        runtime,
        &snapshot->runtime,
        execution,
        host_action,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!state) {
        return 0;
    }
    return nexus_v1_startup_presentation_build_save_from_facts(
        state->save_dir,
        state->slot_mask,
        state->save_selected_row,
        out_commands,
        max_commands);
}

int nexus_v1_launcher_startup_presentation_build_save_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands);
}

static void nexus_v1_launcher_fill_menu_presentation_receipt(
    Nexus_V1_StartupMenuPresentationReceipt *receipt,
    Nexus_V1_StartupMenuPresentationKind kind,
    const Nexus_V1_LauncherStartupAssetsReceipt *assets,
    int route_ready,
    int command_count)
{
    if (!receipt || !assets) {
        return;
    }
    nexus_v1_launcher_startup_menu_presentation_receipt_clear(receipt);
    receipt->kind = kind;
    receipt->assets = *assets;
    receipt->route_ready = route_ready ? 1 : 0;
    receipt->route_blocked = route_ready ? 0 : 1;
    receipt->draw_command_count = route_ready ? command_count : 0;
    receipt->asset_route = assets->startup_menu_asset_route;
    receipt->asset_blocker = assets->real_menu_surface_blocker;
    receipt->status_scope = route_ready ? "STARTUP" : "ASSETS";
    receipt->status = route_ready
        ? (kind == NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE
               ? "NEXUS SAVE SELECT"
               : "NEXUS CHAMPIONS")
        : (assets->startup_menu_asset_route
               ? assets->startup_menu_asset_route
               : "blocked-startup-assets");
    receipt->host_receipt.input_result = NEXUS_V1_STARTUP_HOST_INPUT_REDRAW;
    receipt->host_receipt.status_scope = receipt->status_scope;
    receipt->host_receipt.status = receipt->status;
}

int nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    int command_count = 0;

    nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    if (assets.save_menu_route_ready) {
        command_count =
            nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
                state,
                out_commands,
                max_commands);
    }
    nexus_v1_launcher_fill_menu_presentation_receipt(
        out_receipt,
        NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE,
        &assets,
        assets.save_menu_route_ready,
        command_count);
    return 1;
}

int nexus_v1_launcher_startup_save_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!state) {
        return 0;
    }
    return nexus_v1_startup_presentation_build_champion_from_facts(
        state->engine ? &state->engine->champions : NULL,
        state->slot_mask,
        state->champion_cursor,
        state->champion_frame,
        out_commands,
        max_commands);
}

int nexus_v1_launcher_startup_presentation_build_champion_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands);
}

int nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    int command_count = 0;

    nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
    if (!out_receipt ||
        !nexus_v1_launcher_startup_assets_from_runtime_state(state,
                                                             &assets)) {
        return 0;
    }
    if (assets.champion_menu_route_ready) {
        command_count =
            nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
                state,
                out_commands,
                max_commands);
    }
    nexus_v1_launcher_fill_menu_presentation_receipt(
        out_receipt,
        NEXUS_V1_STARTUP_MENU_PRESENTATION_CHAMPION,
        &assets,
        assets.champion_menu_route_ready,
        command_count);
    return 1;
}

int nexus_v1_launcher_startup_champion_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt)
{
    if (!snapshot) {
        nexus_v1_launcher_startup_menu_presentation_receipt_clear(out_receipt);
        return 0;
    }
    return nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
        &snapshot->runtime,
        out_commands,
        max_commands,
        out_receipt);
}

int nexus_v1_launcher_startup_presentation_execute(
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    const Nexus_V1_StartupDrawExecutor *executor)
{
    return nexus_v1_startup_presentation_execute(
        commands,
        command_count,
        executor);
}

int nexus_v1_launcher_startup_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!state) {
        return 0;
    }
    return nexus_v1_startup_presentation_receipt(
        state->title_active,
        state->save_select_active,
        state->champion_select_active,
        state->title_frame,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_startup_frame,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int nexus_v1_launcher_startup_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!snapshot) {
        return 0;
    }
    return nexus_v1_launcher_startup_presentation_receipt_from_runtime_state(
        &snapshot->runtime,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_startup_frame,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int nexus_v1_launcher_startup_resume_status_host_receipt(
    Nexus_V1_StartupResumeStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    return nexus_v1_startup_resume_status_host_receipt(status, out_receipt);
}

int nexus_v1_launcher_startup_boot_status_host_receipt(
    Nexus_V1_StartupBootStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt)
{
    return nexus_v1_startup_boot_status_host_receipt(status, out_receipt);
}

static void nexus_v1_launcher_fill_boot_receipt(
    const char *data_dir,
    Nexus_V1_Engine *engine,
    int title_loaded,
    Nexus_V1_LauncherBootReceipt *receipt)
{
    if (!receipt || !engine) {
        return;
    }
    receipt->engine = engine;
    receipt->level_loaded = engine->level_loaded;
    receipt->party_x = engine->game.party_x;
    receipt->party_y = engine->game.party_y;
    receipt->party_dir = engine->game.party_dir;
    receipt->tick_count = engine->game.tick_count;
    receipt->title_loaded = title_loaded ? 1 : 0;
    snprintf(receipt->dungeon_path,
             sizeof(receipt->dungeon_path),
             "%s/LEV00.DGN",
             data_dir ? data_dir : "");
    nexus_v1_launcher_fill_startup_assets_receipt(
        engine,
        title_loaded,
        &receipt->startup_assets);
}

int nexus_v1_launcher_boot_level0_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherBootReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
    Nexus_V1_Engine *engine;
    int title_loaded = 0;

    if (out_receipt) {
        nexus_v1_launcher_boot_receipt_clear(out_receipt);
    }
    if (!data_dir || !out_receipt) {
        return 0;
    }
    if (nexus_v1_launcher_init(data_dir) != 0) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        return 0;
    }
    if (nexus_v1_launcher_load_level(0) != 0) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_LEVEL_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        nexus_v1_launcher_shutdown();
        return 0;
    }
    engine = nexus_v1_launcher_get_engine();
    if (!engine) {
        (void)nexus_v1_startup_boot_status_host_receipt(
            NEXUS_V1_STARTUP_BOOT_STATUS_DATA_ERROR,
            &out_receipt->startup_receipt.host_receipt);
        return 0;
    }

    /* New selected-entry boots always start from Nexus defaults; save
     * resume applies persisted party/tick state in the M11 resume path. */
    nexus_v1_game_init(&engine->game, engine->data_dir);
    engine->game.current_level = 0;
    nexus_v1_champions_init(&engine->champions);
    if (engine->mechanics) {
        nexus_mechanics_init(engine->mechanics,
                             engine->game.party_x,
                             engine->game.party_y,
                             engine->game.party_dir);
        engine->mechanics->map_index = 0;
    }

    if (title) {
        title_loaded = nexus_title_load(title, engine) == 0 && title->loaded;
    }

    memset(&facts, 0, sizeof(facts));
    facts.champion_pool = &engine->champions;
    if (!nexus_v1_startup_launch_from_host_facts_with_receipt(
            &facts,
            &out_receipt->startup_receipt)) {
        nexus_v1_launcher_shutdown();
        return 0;
    }
    nexus_v1_launcher_fill_boot_receipt(data_dir,
                                        engine,
                                        title_loaded,
                                        out_receipt);
    return 1;
}

int nexus_v1_launcher_boot_level0_runtime_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherRuntimeReceipt *out_receipt)
{
    Nexus_V1_LauncherBootReceipt boot_receipt;
    Nexus_V1_StartupBootStatus boot_status;

    nexus_v1_launcher_runtime_receipt_clear(out_receipt);
    nexus_v1_launcher_boot_receipt_clear(&boot_receipt);
    if (!out_receipt) {
        return 0;
    }
    snprintf(out_receipt->title,
             sizeof(out_receipt->title),
             "%s",
             NEXUS_V1_GAME_LABEL);
    snprintf(out_receipt->source_id,
             sizeof(out_receipt->source_id),
             "%s",
             NEXUS_V1_GAME_ID);
    if (!nexus_v1_launcher_boot_level0_startup(data_dir,
                                               title,
                                               &boot_receipt)) {
        out_receipt->startup_receipt = boot_receipt.startup_receipt;
        return 0;
    }

    out_receipt->engine = boot_receipt.engine;
    out_receipt->title_screen = title;
    out_receipt->title_screen_keep = boot_receipt.title_loaded ? 1 : 0;
    out_receipt->level_loaded = boot_receipt.level_loaded;
    out_receipt->party_x = boot_receipt.party_x;
    out_receipt->party_y = boot_receipt.party_y;
    out_receipt->party_dir = boot_receipt.party_dir;
    out_receipt->tick_count = boot_receipt.tick_count;
    out_receipt->title_loaded = boot_receipt.title_loaded;
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s",
             boot_receipt.dungeon_path);
    out_receipt->startup_receipt = boot_receipt.startup_receipt;
    out_receipt->startup_assets = boot_receipt.startup_assets;
    boot_status = boot_receipt.title_loaded
        ? NEXUS_V1_STARTUP_BOOT_STATUS_TITLE
        : NEXUS_V1_STARTUP_BOOT_STATUS_TITLE_FALLBACK;
    (void)nexus_v1_launcher_startup_boot_status_host_receipt(
        boot_status,
        &out_receipt->boot_status_receipt);
    out_receipt->boot_log_line = boot_receipt.title_loaded
        ? "T0: NEXUS TITLE LOADED"
        : "T0: NEXUS TITLE FALLBACK";
    return 1;
}

int nexus_v1_launcher_resume_from_save_path(
    const char *save_path,
    Nexus_V1_LightRuntime *light_runtime,
    Nexus_V1_LauncherResumeReceipt *out_receipt)
{
    Nexus_V1_SaveHeader header;
    Nexus_V1_ChampionPool champions;
    Nexus_V1_World world;
    Nexus_V1_Engine *engine;
    Nexus_SaveResult result;
    char nglt_diagnostic[256];
    int level;

    nexus_v1_launcher_resume_receipt_clear(out_receipt);
    if (!save_path || !save_path[0] || !out_receipt) {
        return 0;
    }

    memset(&header, 0, sizeof(header));
    memset(&champions, 0, sizeof(champions));
    memset(&world, 0, sizeof(world));
    memset(nglt_diagnostic, 0, sizeof(nglt_diagnostic));

    result = nexus_v1_load_full_from_path_with_runtime(
        save_path,
        &header,
        &champions,
        &world,
        light_runtime,
        &out_receipt->nglt_decoded,
        nglt_diagnostic,
        sizeof(nglt_diagnostic),
        out_receipt->diagnostic,
        sizeof(out_receipt->diagnostic));
    if (result != NEXUS_SAVE_OK) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_FAILED,
            &out_receipt->host_receipt);
        if (!out_receipt->diagnostic[0]) {
            snprintf(out_receipt->diagnostic,
                     sizeof(out_receipt->diagnostic),
                     "%s",
                     nexus_v1_save_strerror(result));
        }
        return 0;
    }

    level = world.party_level;
    if (level < 0 || level > 15) {
        level = header.current_level;
    }
    if (level < 0 || level > 15) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_INVALID,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "bad level %d",
                 level);
        return 0;
    }
    if (world.party_dir < 0 || world.party_dir > 3) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_DIR_INVALID,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "bad dir %d",
                 world.party_dir);
        return 0;
    }

    if (nexus_v1_launcher_load_level(level) != 0) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_LEVEL_ERROR,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "level %d load failed",
                 level);
        return 0;
    }

    engine = nexus_v1_launcher_get_engine();
    if (!engine) {
        (void)nexus_v1_startup_resume_status_host_receipt(
            NEXUS_V1_STARTUP_RESUME_STATUS_ENGINE_LOST,
            &out_receipt->host_receipt);
        snprintf(out_receipt->diagnostic,
                 sizeof(out_receipt->diagnostic),
                 "engine lost");
        return 0;
    }

    engine->champions = champions;
    engine->game.current_level = level;
    engine->game.party_x = world.party_x;
    engine->game.party_y = world.party_y;
    engine->game.party_dir = world.party_dir;
    engine->game.tick_count = (int)header.game_time;
    if (engine->mechanics) {
        engine->mechanics->map_index = level;
        engine->mechanics->party_x = world.party_x;
        engine->mechanics->party_y = world.party_y;
        engine->mechanics->party_dir = world.party_dir;
        engine->mechanics->total_ticks = header.game_time;
        engine->mechanics->pending_level_change = -1;
        engine->mechanics->pending_teleport = 0;
        engine->mechanics->input_head = 0;
        engine->mechanics->input_tail = 0;
        engine->mechanics->input_count = 0;
    }

    out_receipt->engine = engine;
    out_receipt->resumed = 1;
    out_receipt->level_loaded = engine->level_loaded;
    out_receipt->party_x = engine->game.party_x;
    out_receipt->party_y = engine->game.party_y;
    out_receipt->party_dir = engine->game.party_dir;
    out_receipt->tick_count = engine->game.tick_count;
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s/LEV%02d.DGN",
             engine->data_dir,
             level);
    (void)nexus_v1_startup_resume_status_host_receipt(
        NEXUS_V1_STARTUP_RESUME_STATUS_RESUMED,
        &out_receipt->host_receipt);
    out_receipt->log_line = out_receipt->nglt_decoded
        ? "T0: NEXUS RESUMED + LIGHT RUNTIME"
        : "T0: NEXUS RESUMED";
    return 1;
}

void nexus_v1_launcher_shutdown(void) {
    if (!s_initialized) {
        return;
    }
    nexus_v1_shutdown(&s_engine);
    s_initialized = 0;
    printf("Nexus launcher: shut down\n");
}
