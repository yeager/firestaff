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

typedef enum CSB_V1_StartupEntranceDecision_PC34 {
    CSB_V1_STARTUP_ENTRANCE_DECISION_IGNORED_PC34 = 0,
    CSB_V1_STARTUP_ENTRANCE_DECISION_DISMISS_CREDITS_PC34 = 1,
    CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_DUNGEON_PC34 = 2,
    CSB_V1_STARTUP_ENTRANCE_DECISION_ENTER_BONUS_DUNGEON_PC34 = 3,
    CSB_V1_STARTUP_ENTRANCE_DECISION_RESUME_PC34 = 4,
    CSB_V1_STARTUP_ENTRANCE_DECISION_BEGIN_CREDITS_PC34 = 5,
    CSB_V1_STARTUP_ENTRANCE_DECISION_QUIT_PC34 = 6
} CSB_V1_StartupEntranceDecision_PC34;

typedef enum CSB_V1_StartupEntranceCommandPlanKind_PC34 {
    CSB_V1_STARTUP_ENTRANCE_PLAN_IGNORE_PC34 = 0,
    CSB_V1_STARTUP_ENTRANCE_PLAN_DISMISS_CREDITS_PC34 = 1,
    CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_DUNGEON_PC34 = 2,
    CSB_V1_STARTUP_ENTRANCE_PLAN_ENTER_BONUS_DUNGEON_PC34 = 3,
    CSB_V1_STARTUP_ENTRANCE_PLAN_RESUME_PC34 = 4,
    CSB_V1_STARTUP_ENTRANCE_PLAN_BEGIN_CREDITS_PC34 = 5,
    CSB_V1_STARTUP_ENTRANCE_PLAN_QUIT_PC34 = 6
} CSB_V1_StartupEntranceCommandPlanKind_PC34;

typedef struct CSB_V1_StartupEntranceCommandPlan_PC34 {
    CSB_V1_StartupEntranceCommandPlanKind_PC34 kind;
    int command_id;
    const char *status_scope;
    const char *status;
    const char *failure_status;
    const char *unavailable_status;
} CSB_V1_StartupEntranceCommandPlan_PC34;

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

typedef enum CSB_V1_StartupTitleBlitKind_PC34 {
    CSB_V1_STARTUP_TITLE_BLIT_NONE_PC34 = 0,
    CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34 = 1,
    CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34 = 2
} CSB_V1_StartupTitleBlitKind_PC34;

#define CSB_V1_STARTUP_FALLBACK_TEXT_ROW_CAP_PC34 5
#define CSB_V1_STARTUP_PRIMITIVE_COMMAND_CAP_PC34 4

typedef enum CSB_V1_StartupPrimitiveCommandKind_PC34 {
    CSB_V1_STARTUP_PRIMITIVE_NONE_PC34 = 0,
    CSB_V1_STARTUP_PRIMITIVE_FILL_RECT_PC34 = 1,
    CSB_V1_STARTUP_PRIMITIVE_DRAW_RECT_PC34 = 2,
    CSB_V1_STARTUP_PRIMITIVE_DOOR_PANEL_PC34 = 3
} CSB_V1_StartupPrimitiveCommandKind_PC34;

typedef struct CSB_V1_StartupFallbackTextRow_PC34 {
    int x;
    int y;
    int style;
    int visible;
    const char *text;
} CSB_V1_StartupFallbackTextRow_PC34;

typedef struct CSB_V1_StartupPrimitiveCommand_PC34 {
    CSB_V1_StartupPrimitiveCommandKind_PC34 kind;
    int x;
    int y;
    int w;
    int h;
    int color;
    int light_edge_color;
    int dark_edge_color;
    int visible;
} CSB_V1_StartupPrimitiveCommand_PC34;

#define CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34 8

typedef enum CSB_V1_StartupRenderCommandKind_PC34 {
    CSB_V1_STARTUP_RENDER_COMMAND_NONE_PC34 = 0,
    CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34 = 1,
    CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34 = 2,
    CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_OR_TEXT_PC34 = 3,
    CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34 = 4,
    CSB_V1_STARTUP_RENDER_COMMAND_OPENING_FRAME_IF_SURFACE_PC34 = 5,
    CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34 = 6,
    CSB_V1_STARTUP_RENDER_COMMAND_FALLBACK_IF_NO_SURFACE_PC34 = 7,
    CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34 = 8
} CSB_V1_StartupRenderCommandKind_PC34;

typedef struct CSB_V1_StartupRenderCommand_PC34 {
    CSB_V1_StartupRenderCommandKind_PC34 kind;
} CSB_V1_StartupRenderCommand_PC34;

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
    int utility_overlay_active;
    int runtime_start_valid;
    int runtime_start_x;
    int runtime_start_y;
    int runtime_start_dir;
} CSB_V1_StartupRenderState_PC34;

typedef struct CSB_V1_StartupRenderPlan_PC34 {
    CSB_V1_StartupRenderSurface_PC34 surface;
    int waiting_for_input;
    int source_asset_id;
    int surface_dest_x;
    int surface_dest_y;
    int surface_w;
    int surface_h;
    int surface_transparent_color;
    int title_source_step;
    int title_stage;
    int title_blit_kind;
    int title_transparent_color;
    int title_empty_fallback_x;
    int title_empty_fallback_y;
    int title_empty_fallback_style;
    const char *title_empty_fallback_text;
    int title_source_x;
    int title_source_y;
    int title_source_w;
    int title_source_h;
    int title_dest_x;
    int title_dest_y;
    int title_dest_w;
    int title_dest_h;
    int title_special_palette;
    int special_palette;
    int blink_prompt_visible;
    int opening_step;
    int closed_left_asset_id;
    int closed_right_asset_id;
    int closed_left_source_x;
    int closed_left_source_y;
    int closed_left_dest_x;
    int closed_left_dest_y;
    int closed_left_w;
    int closed_left_h;
    int closed_left_fallback_fill_color;
    int closed_left_fallback_light_edge_color;
    int closed_left_fallback_dark_edge_color;
    int closed_right_source_x;
    int closed_right_source_y;
    int closed_right_dest_x;
    int closed_right_dest_y;
    int closed_right_w;
    int closed_right_h;
    int closed_right_fallback_fill_color;
    int closed_right_fallback_light_edge_color;
    int closed_right_fallback_dark_edge_color;
    int opening_door_valid;
    int opening_door_step;
    int opening_left_source_x;
    int opening_left_source_y;
    int opening_left_dest_x;
    int opening_left_dest_y;
    int opening_left_w;
    int opening_left_h;
    int opening_right_source_x;
    int opening_right_source_y;
    int opening_right_dest_x;
    int opening_right_dest_y;
    int opening_right_w;
    int opening_right_h;
    int fallback_title_x;
    int fallback_title_y;
    int fallback_title_style;
    const char *fallback_title_text;
    int fallback_subtitle_x;
    int fallback_subtitle_y;
    int fallback_subtitle_style;
    const char *fallback_subtitle_text;
    int fallback_status_x;
    int fallback_status_y;
    int fallback_status_style;
    const char *fallback_status_text;
    int fallback_status_visible;
    int fallback_frame_valid;
    int fallback_frame_x;
    int fallback_frame_y;
    int fallback_frame_w;
    int fallback_frame_h;
    int fallback_frame_color;
    int fallback_detail_x;
    int fallback_detail_y;
    int fallback_detail_style;
    const char *fallback_detail_text;
    int fallback_detail_visible;
    int fallback_runtime_detail_visible;
    char fallback_runtime_detail_text[96];
    int fallback_prompt_x;
    int fallback_prompt_y;
    int fallback_prompt_style;
    const char *fallback_prompt_text;
    int fallback_text_row_count;
    CSB_V1_StartupFallbackTextRow_PC34 fallback_text_rows[
        CSB_V1_STARTUP_FALLBACK_TEXT_ROW_CAP_PC34];
    int primitive_command_count;
    CSB_V1_StartupPrimitiveCommand_PC34 primitive_commands[
        CSB_V1_STARTUP_PRIMITIVE_COMMAND_CAP_PC34];
    int render_command_count;
    CSB_V1_StartupRenderCommand_PC34 render_commands[
        CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34];
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
int csb_v1_startup_resolve_entrance_command_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    int command_id,
    CSB_V1_StartupEntranceDecision_PC34 *out_decision);
void csb_v1_startup_entrance_command_plan_init_pc34(
    CSB_V1_StartupEntranceCommandPlan_PC34 *plan);
int csb_v1_startup_plan_for_entrance_command_pc34(
    const CSB_V1_StartupCommandState_PC34 *state,
    int command_id,
    CSB_V1_StartupEntranceCommandPlan_PC34 *out_plan);
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
