#ifndef NEXUS_V1_LAUNCHER_H
#define NEXUS_V1_LAUNCHER_H

/*
 * nexus_v1_launcher.h
 * ===================
 * Nexus V1 launcher — singleton engine lifecycle manager.
 *
 * Owns the Nexus_V1_Engine singleton. Provides:
 *   - launcher_init()       — discover game data, init engine
 *   - launcher_load_level()  — load a dungeon level (0-15)
 *   - launcher_get_engine()  — access singleton (for M11 render loop)
 *
 * Design rationale:
 *   The Nexus V1 engine is a self-contained object with its own game
 *   state, mechanics, and resource management. The launcher acts as a
 *   thin facade that owns the engine pointer and routes M12/M11 calls
 *   into it. This separates launcher concerns (data discovery, profile
 *   validation) from engine concerns (tick, render, save/load).
 *
 * Source: DM Nexus (Saturn) boot flow, NEXUS.C / NEXUS2.C engine
 * lifecycle, ReDMCSB boot/disk loading references.
 */

#include "nexus_v1_engine.h"
#include "firestaff_nexus_v1_boot_profile.h"
#include "nexus_v1_light_runtime.h"
#include "nexus_v1_startup_menu.h"
#include "nexus_v1_title.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Public API ─────────────────────────────────────────────────────── */

/* Initialize the Nexus V1 engine singleton.
 * - Scans data_dir for CUE/BIN (Saturn CD) or extracted files.
 * - Calls nexus_v1_init() on the singleton.
 * - Returns 0 on success, -1 on failure.
 * - Safe to call multiple times; only first call has effect
 *   (subsequent calls return 0 if already initialized). */
int nexus_v1_launcher_init(const char *data_dir);

/* Load a dungeon level (0-15) into the engine.
 * Calls nexus_v1_load_level() on the singleton.
 * Returns 0 on success, -1 if launcher not init'd or level load failed. */
int nexus_v1_launcher_load_level(int level);

/* Get the current Nexus V1 engine singleton.
 * Returns NULL if launcher not initialized.
 * The returned pointer is owned by the launcher — do not free it. */
Nexus_V1_Engine *nexus_v1_launcher_get_engine(void);

typedef struct {
    int title_surface_loaded;
    int warning_surface_loaded;
    int gameover_surface_loaded;
    int status_bg_surface_loaded;
    int title_screen_loaded;
    int startup_surfaces_loaded;
    int startup_surfaces_expected;
    int startup_surfaces_fallback;
    int faces_loaded;
    int faces_expected;
    int faces_fallback;
    int menu_bpk_upload_receipt_valid;
    Nexus_V1_BpkRuntimeUploadRoute menu_bpk_upload_route;
    int menu_bpk_planned_rows;
    int menu_bpk_blocked_prs3_uploads;
    int menu_bpk_blocks_real_menu_surface_render;
    int menu_bpk_fallback_visuals_permitted;
    Nexus_SfxRuntimeStatus startup_sfx_status;
    int startup_sfx_level_index;
    int startup_cd_track;
    int startup_sfx_blocks_real_playback;
    int startup_assets_ready;
    int startup_audio_handoff_ready;
    int main_menu_route_ready;
    int title_route_ready;
    int real_menu_surface_route_ready;
    int real_menu_surface_route_blocked;
    int save_menu_route_ready;
    int champion_menu_route_ready;
    const char *real_menu_surface_blocker;
    const char *startup_menu_asset_route;
} Nexus_V1_LauncherStartupAssetsReceipt;

typedef struct {
    Nexus_V1_Engine *engine;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    int title_loaded;
    char dungeon_path[512];
    Nexus_V1_StartupLaunchReceipt startup_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt startup_assets;
} Nexus_V1_LauncherBootReceipt;

typedef struct {
    Nexus_V1_Engine *engine;
    Nexus_TitleScreen *title_screen;
    int title_screen_keep;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    int title_loaded;
    char title[64];
    char source_id[32];
    char dungeon_path[512];
    Nexus_V1_StartupLaunchReceipt startup_receipt;
    Nexus_V1_StartupHostReceipt boot_status_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt startup_assets;
    const char *boot_log_line;
} Nexus_V1_LauncherRuntimeReceipt;

typedef enum {
    NEXUS_V1_STARTUP_LAUNCH_GATE_INVALID = 0,
    NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR = 1,
    NEXUS_V1_STARTUP_LAUNCH_GATE_TITLE_READY = 2,
    NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED = 3,
    NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY = 4
} Nexus_V1_StartupLaunchGateRoute;

typedef struct {
    Nexus_V1_StartupLaunchGateRoute route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    int engine_ready;
    int level_loaded;
    int title_ready;
    int title_draw_ready;
    int real_menu_ready;
    int save_menu_ready;
    int champion_menu_ready;
    int fallback_visuals_permitted;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
    const char *boot_log_line;
} Nexus_V1_StartupLaunchGateReceipt;

typedef enum {
    NEXUS_V1_STARTUP_ASSET_HANDOFF_INVALID = 0,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR = 1,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY = 2,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED = 3,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY = 4
} Nexus_V1_StartupAssetHandoffRoute;

typedef struct {
    Nexus_V1_StartupAssetHandoffRoute route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    int title_asset_handoff_ready;
    int real_menu_asset_handoff_ready;
    int audio_asset_handoff_ready;
    int main_menu_route_ready;
    int blocks_main_menu_route;
    int fallback_visuals_permitted;
    const char *title_asset_route;
    const char *menu_asset_route;
    const char *audio_asset_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupAssetHandoffReceipt;

typedef struct {
    Nexus_V1_Engine *engine;
    Nexus_V1_StartupHostReceipt host_receipt;
    int resumed;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    char dungeon_path[512];
    int nglt_decoded;
    char diagnostic[256];
    const char *log_line;
} Nexus_V1_LauncherResumeReceipt;

void nexus_v1_launcher_boot_receipt_clear(
    Nexus_V1_LauncherBootReceipt *receipt);
void nexus_v1_launcher_runtime_receipt_clear(
    Nexus_V1_LauncherRuntimeReceipt *receipt);
void nexus_v1_launcher_startup_launch_gate_receipt_clear(
    Nexus_V1_StartupLaunchGateReceipt *receipt);
void nexus_v1_launcher_startup_asset_handoff_receipt_clear(
    Nexus_V1_StartupAssetHandoffReceipt *receipt);
const char *nexus_v1_launcher_startup_launch_gate_route_name(
    Nexus_V1_StartupLaunchGateRoute route);
const char *nexus_v1_launcher_startup_asset_handoff_route_name(
    Nexus_V1_StartupAssetHandoffRoute route);
int nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupLaunchGateReceipt *out_receipt);
int nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupAssetHandoffReceipt *out_receipt);

typedef struct {
    int title_active;
    int title_frame;
    int save_select_active;
    int champion_select_active;
    const char *save_dir;
    unsigned int slot_mask;
    int save_selected_row;
    int save_row_count;
    Nexus_V1_Engine *engine;
    int champion_cursor;
    int champion_frame;
} Nexus_V1_StartupRuntimeState;

typedef struct {
    Nexus_V1_StartupRuntimeState runtime;
} Nexus_V1_LauncherRuntimeStartupSnapshot;

typedef enum {
    NEXUS_V1_STARTUP_MENU_PRESENTATION_INVALID = 0,
    NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE = 1,
    NEXUS_V1_STARTUP_MENU_PRESENTATION_CHAMPION = 2
} Nexus_V1_StartupMenuPresentationKind;

typedef struct {
    Nexus_V1_StartupMenuPresentationKind kind;
    int route_ready;
    int route_blocked;
    int draw_command_count;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupMenuPresentationReceipt;

typedef struct {
    Nexus_V1_StartupTitleRouteReceipt title_route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    int route_ready;
    int route_blocked;
    int title_draw_ready;
    int save_menu_ready;
    int champion_menu_ready;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupTitleHandoffReceipt;

typedef enum {
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID = 0,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED = 1,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START = 2,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED = 3,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE = 4
} Nexus_V1_StartupRuntimeHandoffRoute;

typedef struct {
    Nexus_V1_StartupRuntimeHandoffRoute route;
    Nexus_V1_StartupChampionExecution champion_execution;
    Nexus_V1_StartupHostActionReceipt host_action_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_V1_DgnRenderPlanReceipt render_plan;
    int runtime_ready;
    int dgn_render_ready;
    int hud_ready;
    int dgn_render_blocked;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int command_count;
    int fallback_visuals_permitted;
    const char *asset_route;
    const char *dgn_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupRuntimeHandoffReceipt;

typedef enum {
    NEXUS_V1_STARTUP_ROUTE_PROOF_INVALID = 0,
    NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED = 1,
    NEXUS_V1_STARTUP_ROUTE_PROOF_TITLE_READY = 2,
    NEXUS_V1_STARTUP_ROUTE_PROOF_MENU_READY = 3,
    NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY = 4
} Nexus_V1_StartupRouteProofRoute;

typedef struct {
    Nexus_V1_StartupRouteProofRoute route;
    Nexus_V1_StartupLaunchGateReceipt launch_gate;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupTitleHandoffReceipt title_handoff;
    Nexus_V1_StartupMenuPresentationReceipt menu_presentation;
    Nexus_V1_StartupRuntimeHandoffReceipt runtime_handoff;
    int saturn_asset_boot_ready;
    int title_route_ready;
    int menu_route_ready;
    int runtime_route_ready;
    int graphics_ready;
    int audio_ready;
    int title_menu_route_ready;
    int menu_runtime_route_ready;
    int first_runtime_route_ready;
    int audio_runtime_route_ready;
    int full_startup_route_ready;
    int fallback_visuals_permitted;
    const char *asset_route;
    const char *title_route;
    const char *menu_route;
    const char *runtime_route;
    const char *first_runtime_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupRouteProofReceipt;

void nexus_v1_launcher_startup_runtime_state_clear(
    Nexus_V1_StartupRuntimeState *state);
void nexus_v1_launcher_runtime_startup_snapshot_clear(
    Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot);
void nexus_v1_launcher_startup_menu_presentation_receipt_clear(
    Nexus_V1_StartupMenuPresentationReceipt *receipt);
void nexus_v1_launcher_startup_title_handoff_receipt_clear(
    Nexus_V1_StartupTitleHandoffReceipt *receipt);
void nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
    Nexus_V1_StartupRuntimeHandoffReceipt *receipt);
void nexus_v1_launcher_startup_route_proof_receipt_clear(
    Nexus_V1_StartupRouteProofReceipt *receipt);
int nexus_v1_launcher_startup_host_facts_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupHostFacts *out_facts);
int nexus_v1_launcher_startup_host_facts_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupHostFacts *out_facts);
int nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt);
int nexus_v1_launcher_startup_assets_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt);
int nexus_v1_launcher_startup_advance_idle_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupIdleReceipt *out_receipt);
int nexus_v1_launcher_startup_advance_idle_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupIdleReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_save_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_save_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_title_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
const char *nexus_v1_launcher_startup_runtime_handoff_route_name(
    Nexus_V1_StartupRuntimeHandoffRoute route);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
const char *nexus_v1_launcher_startup_route_proof_route_name(
    Nexus_V1_StartupRouteProofRoute route);
int nexus_v1_launcher_startup_route_proof_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt);
int nexus_v1_launcher_startup_route_proof_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt);
int nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_save_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_save_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_champion_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_champion_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_presentation_execute(
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    const Nexus_V1_StartupDrawExecutor *executor);
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
    int *out_title_ready);
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
    int *out_title_ready);
int nexus_v1_launcher_startup_resume_status_host_receipt(
    Nexus_V1_StartupResumeStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt);
int nexus_v1_launcher_startup_boot_status_host_receipt(
    Nexus_V1_StartupBootStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt);

int nexus_v1_launcher_boot_level0_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherBootReceipt *out_receipt);
int nexus_v1_launcher_boot_level0_runtime_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherRuntimeReceipt *out_receipt);
int nexus_v1_launcher_resume_from_save_path(
    const char *save_path,
    Nexus_V1_LightRuntime *light_runtime,
    Nexus_V1_LauncherResumeReceipt *out_receipt);

/* Shutdown the launcher and free the engine singleton.
 * Safe to call multiple times. */
void nexus_v1_launcher_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_LAUNCHER_H */
