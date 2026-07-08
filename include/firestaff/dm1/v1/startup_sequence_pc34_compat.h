#ifndef FIRESTAFF_DM1_V1_STARTUP_SEQUENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_STARTUP_SEQUENCE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_StartupStage_PC34 {
    DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34 = 1,
    DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34 = 2,
    DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34 = 3,
    DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34 = 4,
    DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34 = 5,
    DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34 = 6
} DM1_V1_StartupStage_PC34;

typedef enum DM1_V1_StartupLaunchPath_PC34 {
    DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34 = 1,
    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34 = 2,
    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34 = 3
} DM1_V1_StartupLaunchPath_PC34;

typedef enum DM1_V1_StartupHandoffAction_PC34 {
    DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 = 0,
    DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34 = 1,
    DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34 = 2,
    DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34 = 3,
    DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34 = 4
} DM1_V1_StartupHandoffAction_PC34;

typedef struct DM1_V1_StartupHandoffPreludePlan_PC34 {
    int required;
    int source_order_valid;
    int play_swsh;
    int discard_presentation_after_swsh;
    const char* game_id;
    const char* failure_evidence;
} DM1_V1_StartupHandoffPreludePlan_PC34;

typedef struct DM1_V1_StartupHandoffPostLaunchPlan_PC34 {
    int required;
    int play_title;
    int play_entrance;
    int entrance_auto_enter_ms;
    const char* source_id;
} DM1_V1_StartupHandoffPostLaunchPlan_PC34;

typedef struct DM1_V1_StartupHandoffOutcome_PC34 {
    int title_played;
    int entrance_command;
    DM1_V1_StartupHandoffAction_PC34 action;
    const char* status;
} DM1_V1_StartupHandoffOutcome_PC34;

typedef struct DM1_V1_StartupHostApplyResult_PC34 {
    int handled;
    int quit_requested;
    int resume_requested;
    int resume_loaded;
    int resume_used_backup;
    char resume_path[512];
} DM1_V1_StartupHostApplyResult_PC34;

typedef struct DM1_V1_StartupHandoffCallbacks_PC34 {
    void* user;
    int (*report_source_order_failure)(void* user, const char* evidence);
    int (*raise_window)(void* user);
    int (*play_swsh)(void* user, const char* game_id, int preserve_audio);
    int (*discard_presentation_texture)(void* user);
    int (*play_title)(void* user, const char* source_id, int* out_played_any_frame);
    int (*play_entrance)(void* user,
                         const char* source_id,
                         int auto_enter_after_ms,
                         int* out_entrance_command);
} DM1_V1_StartupHandoffCallbacks_PC34;

typedef struct DM1_V1_StartupHostCallbacks_PC34 {
    void* user;
    int (*set_game_active)(void* user, int active);
    int (*resolve_resume_save_path)(void* user,
                                    const char* source_id,
                                    char* out_path,
                                    int out_path_size);
    int (*load_resume_save_path)(void* user,
                                 const char* save_path,
                                 int* out_used_backup);
    int (*log_resume_loaded)(void* user,
                             const char* save_path,
                             int used_backup);
    int (*log_resume_missing)(void* user, const char* save_path);
    int (*log_entrance_skipped)(void* user);
} DM1_V1_StartupHostCallbacks_PC34;

typedef struct DM1_V1_StartupSelectedLaunchCallbacks_PC34 {
    void* user;
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks;
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks;
    int (*open_selected_entry)(void* user,
                               char* out_source_id,
                               int out_source_id_size);
    int (*after_open)(void* user);
    int (*draw_opened)(void* user);
    int (*mark_launch_failed)(void* user);
} DM1_V1_StartupSelectedLaunchCallbacks_PC34;

typedef struct DM1_V1_StartupSelectedLaunchResult_PC34 {
    int handled;
    int opened;
    int launch_failed;
    DM1_V1_StartupHandoffOutcome_PC34 handoff_outcome;
    DM1_V1_StartupHostApplyResult_PC34 host_apply_result;
} DM1_V1_StartupSelectedLaunchResult_PC34;

enum {
    DM1_V1_STARTUP_BOOT_PROBE_SOURCE_ID_CAPACITY_PC34 = 32,
    DM1_V1_STARTUP_BOOT_PROBE_PHASE_CAPACITY_PC34 = 48,
    DM1_V1_STARTUP_BOOT_PROBE_ANIMATION_CAPACITY_PC34 = 48
};

typedef struct DM1_V1_StartupBootProbeFacts_PC34 {
    const char* source_id;
    int level_loaded;
    int intro_bypassed;
    int map_index;
    int party_x;
    int party_y;
    int party_dir;
    int champion_count;
    int runtime_tick;
    unsigned int world_tick;
} DM1_V1_StartupBootProbeFacts_PC34;

typedef struct DM1_V1_StartupBootProbeReceipt_PC34 {
    int handled;
    char source_id[DM1_V1_STARTUP_BOOT_PROBE_SOURCE_ID_CAPACITY_PC34];
    int dm1_startup_intro_bypassed;
    char startup_phase[DM1_V1_STARTUP_BOOT_PROBE_PHASE_CAPACITY_PC34];
    int startup_active;
    int startup_frame;
    char startup_animation[DM1_V1_STARTUP_BOOT_PROBE_ANIMATION_CAPACITY_PC34];
    int startup_animation_active;
    int startup_title_frame;
    int startup_title_frame_max;
    int startup_title_ready;
    int level_loaded;
    int map_index;
    int party_x;
    int party_y;
    int party_dir;
    int champion_count;
    int runtime_tick;
    unsigned int world_tick;
} DM1_V1_StartupBootProbeReceipt_PC34;

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage);
int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier);
int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path);
int dm1_v1_startup_source_visible_handoff_required_pc34(const char* game_id);
int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed);
int dm1_v1_startup_selected_entry_receipt_valid_pc34(const char* game_id,
                                                     int intro_bypassed);
int dm1_v1_startup_handoff_prelude_plan_pc34(
    const char* game_id,
    DM1_V1_StartupHandoffPreludePlan_PC34* out_plan);
int dm1_v1_startup_handoff_post_launch_plan_pc34(
    const char* source_id,
    DM1_V1_StartupHandoffPostLaunchPlan_PC34* out_plan);
int dm1_v1_startup_execute_handoff_prelude_pc34(
    const char* game_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks);
int dm1_v1_startup_execute_handoff_post_launch_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    int* out_title_played,
    int* out_entrance_command);
int dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
    int entrance_command,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome);
int dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome);
int dm1_v1_startup_apply_handoff_outcome_pc34(
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const char* source_id,
    const DM1_V1_StartupHostCallbacks_PC34* callbacks,
    DM1_V1_StartupHostApplyResult_PC34* out_result);
int dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks,
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome,
    DM1_V1_StartupHostApplyResult_PC34* out_result);
int dm1_v1_startup_execute_selected_launch_transaction_pc34(
    const char* selected_game_id,
    const DM1_V1_StartupSelectedLaunchCallbacks_PC34* callbacks,
    DM1_V1_StartupSelectedLaunchResult_PC34* out_result);
int dm1_v1_startup_receipt_phase_pc34(int level_loaded,
                                      int intro_bypassed,
                                      char* out_phase,
                                      int out_phase_size);
int dm1_v1_startup_boot_probe_receipt_pc34(int level_loaded,
                                           int intro_bypassed,
                                           char* out_phase,
                                           int out_phase_size,
                                           int* out_startup_active,
                                           char* out_animation,
                                           int out_animation_size,
                                           int* out_animation_active,
                                           int* out_title_frame,
                                           int* out_title_frame_max,
                                           int* out_title_ready);
int dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
    const DM1_V1_StartupBootProbeFacts_PC34* facts,
    DM1_V1_StartupBootProbeReceipt_PC34* out_receipt);
int dm1_v1_startup_sequence_source_order_valid_pc34(void);
const char* dm1_v1_startup_sequence_source_evidence_pc34(void);
unsigned int dm1_v1_startup_title_zoom_steps_pc34(void);
unsigned int dm1_v1_startup_title_source_animation_steps_pc34(void);
unsigned int dm1_v1_startup_title_frame_bank_equivalent_steps_pc34(void);
unsigned int dm1_v1_startup_title_presents_hold_vblanks_pc34(void);
unsigned int dm1_v1_startup_title_vblank_tick_ms_pc34(void);
unsigned int dm1_v1_startup_title_presents_hold_ms_pc34(void);
unsigned int dm1_v1_startup_title_post_zoom_vblanks_pc34(void);
unsigned int dm1_v1_startup_title_final_guard_vblanks_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
