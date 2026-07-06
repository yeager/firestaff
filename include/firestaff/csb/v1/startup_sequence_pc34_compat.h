#ifndef FIRESTAFF_CSB_V1_STARTUP_SEQUENCE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_SEQUENCE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CSB_V1_StartupStage_PC34 {
    CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 = 1,
    CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 = 2,
    CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 = 3,
    CSB_V1_STARTUP_STAGE_ENTRANCE_LOAD_BLACK_PC34 = 4,
    CSB_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34 = 5,
    CSB_V1_STARTUP_STAGE_ENTRANCE_PRE_OPEN_DELAY_PC34 = 6,
    CSB_V1_STARTUP_STAGE_ENTRANCE_DOOR_OPENING_PC34 = 7,
    CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34 = 8
} CSB_V1_StartupStage_PC34;

typedef enum CSB_V1_StartupInput_PC34 {
    CSB_V1_STARTUP_INPUT_NONE_PC34 = 0,
    CSB_V1_STARTUP_INPUT_ACCEPT_PC34 = 1,
    CSB_V1_STARTUP_INPUT_ACTION_PC34 = 2,
    CSB_V1_STARTUP_INPUT_BACK_PC34 = 3,
    CSB_V1_STARTUP_INPUT_DISK_MENU_PC34 = 4
} CSB_V1_StartupInput_PC34;

typedef enum CSB_V1_StartupEntranceAction_PC34 {
    CSB_V1_STARTUP_ENTRANCE_ACTION_NONE_PC34 = 0,
    CSB_V1_STARTUP_ENTRANCE_ACTION_ENTER_DUNGEON_PC34 = 1,
    CSB_V1_STARTUP_ENTRANCE_ACTION_RESUME_PC34 = 2,
    CSB_V1_STARTUP_ENTRANCE_ACTION_QUIT_PC34 = 3
} CSB_V1_StartupEntranceAction_PC34;

typedef enum CSB_V1_StartupEntranceCommand_PC34 {
    CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34 = 0,
    CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34 = 200,
    CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_BONUS_DUNGEON_PC34 = 201,
    CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34 = 202,
    CSB_V1_STARTUP_ENTRANCE_COMMAND_DRAW_CREDITS_PC34 = 203,
    CSB_V1_STARTUP_ENTRANCE_COMMAND_QUIT_PC34 = 216
} CSB_V1_StartupEntranceCommand_PC34;

typedef struct CSB_V1_StartupTickState_PC34 {
    int entrance_frame;
    int title_active;
    int title_frame;
    int title_source_step;
    int entrance_source_step;
    int credits_active;
    int credits_remaining_ticks;
    int opening_active;
    int opening_delay_ticks;
    int opening_step;
    int door_step_count;
} CSB_V1_StartupTickState_PC34;

typedef struct CSB_V1_StartupTickResult_PC34 {
    int redraw;
    int title_finished;
    int reached_entrance_wait;
    int credits_finished;
    int door_opening_finished;
} CSB_V1_StartupTickResult_PC34;

typedef enum CSB_V1_StartupRenderSurface_PC34 {
    CSB_V1_STARTUP_RENDER_NONE_PC34 = 0,
    CSB_V1_STARTUP_RENDER_TITLE_PC34 = 1,
    CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34 = 2,
    CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 = 3,
    CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34 = 4,
    CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34 = 5,
    CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 = 6
} CSB_V1_StartupRenderSurface_PC34;

typedef struct CSB_V1_StartupRenderState_PC34 {
    int entrance_active;
    int entrance_frame;
    int title_active;
    int title_frame;
    int entrance_source_step;
    int credits_active;
    int opening_active;
    int opening_delay_ticks;
    int opening_step;
} CSB_V1_StartupRenderState_PC34;

typedef struct CSB_V1_StartupRenderPlan_PC34 {
    CSB_V1_StartupRenderSurface_PC34 surface;
    int waiting_for_input;
    int title_source_step;
    int blink_prompt_visible;
    int opening_step;
} CSB_V1_StartupRenderPlan_PC34;

typedef struct CSB_V1_StartupCommandState_PC34 {
    int title_active;
    int title_frame;
    int title_source_step;
    int entrance_active;
    int entrance_source_step;
    int entrance_dismissed;
    int credits_active;
    int credits_remaining_ticks;
    int opening_active;
    int opening_delay_ticks;
    int opening_step;
    int pending_command;
} CSB_V1_StartupCommandState_PC34;

const char* csb_v1_startup_stage_name_pc34(CSB_V1_StartupStage_PC34 stage);
int csb_v1_startup_stage_after_pc34(CSB_V1_StartupStage_PC34 later,
                                    CSB_V1_StartupStage_PC34 earlier);
int csb_v1_startup_title_total_ticks_pc34(void);
int csb_v1_startup_title_presents_ticks_pc34(void);
int csb_v1_startup_title_stage_for_frame_pc34(int frame);
unsigned int csb_v1_startup_title_source_step_for_frame_pc34(int frame);
int csb_v1_startup_entrance_wait_stage_pc34(void);
int csb_v1_startup_entrance_pre_open_delay_ticks_pc34(void);
int csb_v1_startup_entrance_credits_ticks_pc34(void);
int csb_v1_startup_entrance_action_for_input_pc34(
    int credits_active,
    CSB_V1_StartupInput_PC34 input);
int csb_v1_startup_entrance_command_for_action_pc34(
    CSB_V1_StartupEntranceAction_PC34 action);
int csb_v1_startup_advance_tick_pc34(
    CSB_V1_StartupTickState_PC34 *state,
    CSB_V1_StartupTickResult_PC34 *out_result);
int csb_v1_startup_build_render_plan_pc34(
    const CSB_V1_StartupRenderState_PC34 *state,
    CSB_V1_StartupRenderPlan_PC34 *out_plan);
int csb_v1_startup_init_command_state_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    int skip_startup);
int csb_v1_startup_entrance_accepts_input_pc34(
    const CSB_V1_StartupCommandState_PC34 *state);
int csb_v1_startup_begin_door_opening_pc34(
    CSB_V1_StartupCommandState_PC34 *state,
    int pending_command);
int csb_v1_startup_finish_door_opening_pc34(
    CSB_V1_StartupCommandState_PC34 *state);
int csb_v1_startup_begin_credits_pc34(
    CSB_V1_StartupCommandState_PC34 *state);
int csb_v1_startup_dismiss_credits_pc34(
    CSB_V1_StartupCommandState_PC34 *state);
int csb_v1_startup_quit_to_launcher_pc34(
    CSB_V1_StartupCommandState_PC34 *state);
int csb_v1_startup_receipt_phase_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    int entrance_frame,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame);
int csb_v1_startup_sequence_source_order_valid_pc34(void);
const char* csb_v1_startup_sequence_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
