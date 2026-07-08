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
    Nexus_V1_Engine *engine;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    int title_loaded;
    char dungeon_path[512];
    Nexus_V1_StartupLaunchReceipt startup_receipt;
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
    const char *boot_log_line;
} Nexus_V1_LauncherRuntimeReceipt;

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

void nexus_v1_launcher_startup_runtime_state_clear(
    Nexus_V1_StartupRuntimeState *state);
void nexus_v1_launcher_runtime_startup_snapshot_clear(
    Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot);
int nexus_v1_launcher_startup_host_facts_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupHostFacts *out_facts);
int nexus_v1_launcher_startup_host_facts_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupHostFacts *out_facts);
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
int nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_save_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_champion_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
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
