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
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
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
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
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

int nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt)
{
    Nexus_V1_StartupHostFacts facts;
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

int nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_StartupHostFacts facts;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_presentation_build_save_from_host_facts(
        &facts,
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

int nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands)
{
    Nexus_V1_StartupHostFacts facts;
    if (!nexus_v1_launcher_startup_host_facts_from_runtime_state(
            state,
            &facts)) {
        return 0;
    }
    return nexus_v1_startup_presentation_build_champion_from_host_facts(
        &facts,
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
