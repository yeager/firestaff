#ifndef DM2_V1_STARTUP_MENU_H
#define DM2_V1_STARTUP_MENU_H

#include <stdint.h>

#include "dm2_v1_new_game.h"
#include "dm2_v1_startup_layout.h"

typedef enum {
    DM2_V1_STARTUP_ROW_NONE = 0,
    DM2_V1_STARTUP_ROW_CONTINUE = 1,
    DM2_V1_STARTUP_ROW_SLOT = 2,
    DM2_V1_STARTUP_ROW_NEW_GAME = 3
} DM2_V1_StartupRowKind;

typedef enum {
    DM2_V1_STARTUP_ACTION_NONE = 0,
    DM2_V1_STARTUP_ACTION_CONTINUE = 1,
    DM2_V1_STARTUP_ACTION_LOAD_SLOT = 2,
    DM2_V1_STARTUP_ACTION_NEW_GAME = 3,
    DM2_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER = 4
} DM2_V1_StartupActionKind;

typedef enum {
    DM2_V1_STARTUP_INPUT_NONE = 0,
    DM2_V1_STARTUP_INPUT_UP = 1,
    DM2_V1_STARTUP_INPUT_DOWN = 2,
    DM2_V1_STARTUP_INPUT_ACCEPT = 3,
    DM2_V1_STARTUP_INPUT_ACTION = 4,
    DM2_V1_STARTUP_INPUT_BACK = 5
} DM2_V1_StartupInput;

DM2_V1_StartupInput dm2_v1_startup_input_from_firestaff_menu_code(
    int menu_input);

typedef struct {
    DM2_V1_StartupActionKind kind;
    int row;
    int slot;
} DM2_V1_StartupAction;

typedef enum {
    DM2_V1_STARTUP_PLAN_IGNORE = 0,
    DM2_V1_STARTUP_PLAN_CONTINUE = 1,
    DM2_V1_STARTUP_PLAN_LOAD_SLOT = 2,
    DM2_V1_STARTUP_PLAN_NEW_GAME = 3,
    DM2_V1_STARTUP_PLAN_RETURN_TO_LAUNCHER = 4
} DM2_V1_StartupPlanKind;

typedef struct {
    DM2_V1_StartupPlanKind kind;
    int slot;
    int rescan_saves_on_failure;
    const char *success_status;
    const char *failure_status;
} DM2_V1_StartupActionPlan;

typedef enum {
    DM2_V1_STARTUP_EXEC_IGNORE = 0,
    DM2_V1_STARTUP_EXEC_SESSION_READY = 1,
    DM2_V1_STARTUP_EXEC_STATUS_REDRAW = 2,
    DM2_V1_STARTUP_EXEC_RETURN_TO_LAUNCHER = 3
} DM2_V1_StartupExecutionKind;

typedef struct {
    DM2_V1_StartupExecutionKind kind;
    int rescan_saves;
    const char *status;
    DM2_V1_SessionState session;
} DM2_V1_StartupExecution;

typedef struct {
    int set_startup_menu_active;
    int startup_menu_active;
} DM2_V1_StartupModeUpdate;

typedef enum {
    DM2_V1_STARTUP_INPUT_RESULT_IGNORED = 0,
    DM2_V1_STARTUP_INPUT_RESULT_REDRAW = 1,
    DM2_V1_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER = 2
} DM2_V1_StartupInputResult;

typedef struct {
    DM2_V1_StartupInputResult result;
    int rescan_saves;
    const char *status_scope;
    const char *status;
} DM2_V1_StartupInputOutcome;

typedef struct {
    DM2_V1_StartupModeUpdate mode_update;
    DM2_V1_StartupInputOutcome outcome;
    int session_should_apply;
    int session_applied;
} DM2_V1_StartupApplyReceipt;

typedef enum {
    DM2_V1_STARTUP_HOST_INPUT_IGNORED = 0,
    DM2_V1_STARTUP_HOST_INPUT_REDRAW = 1,
    DM2_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER = 2
} DM2_V1_StartupHostInputResult;

typedef struct {
    DM2_V1_StartupModeUpdate mode_update;
    DM2_V1_StartupHostInputResult input_result;
    int rescan_saves;
    const char *status_scope;
    const char *status;
} DM2_V1_StartupHostReceipt;

typedef int (*DM2_V1_StartupSessionApplyFn)(
    void *userdata,
    const DM2_V1_SessionState *session);

typedef struct {
    char save_root[512];
    int resume_available;
    unsigned int slot_mask;
    int row_count;
    int selected_row;
} DM2_V1_StartupMenu;

typedef struct {
    char save_root[512];
    int resume_available;
    unsigned int slot_mask;
    int row_count;
    int selected_row;
} DM2_V1_StartupMenuSnapshot;

typedef struct {
    char save_root[512];
    int resume_available;
    unsigned int slot_mask;
    int row_count;
    int selected_row;
} DM2_V1_StartupMenuStateReceipt;

typedef struct {
    DM2_V1_StartupHostReceipt host_receipt;
    int menu_state_receipt_valid;
    DM2_V1_StartupMenuStateReceipt menu_state_receipt;
} DM2_V1_StartupHostActionReceipt;

typedef struct {
    const char *save_root;
    const char *fallback_save_root;
    int resume_available;
    unsigned int slot_mask;
    int selected_row;
    const char *scan_save_root;
} DM2_V1_StartupHostFacts;

enum {
    DM2_V1_STARTUP_ROW_LABEL_CAPACITY = 48
};

typedef struct {
    DM2_V1_StartupRowKind kind;
    int row;
    int slot;
    int selected;
    DM2_V1_StartupRect rect;
    DM2_V1_StartupRect highlight_rect;
    int text_x;
    int text_y;
    char label[DM2_V1_STARTUP_ROW_LABEL_CAPACITY];
} DM2_V1_StartupRenderRow;

typedef enum {
    DM2_V1_STARTUP_SAVE_PATH_INVALID = 0,
    DM2_V1_STARTUP_SAVE_PATH_LOADED = 1,
    DM2_V1_STARTUP_SAVE_PATH_LOAD_FAILED = 2
} DM2_V1_StartupSavePathResult;

int dm2_v1_startup_save_path_to_root_slot(const char *save_path,
                                          char *out_root,
                                          int out_root_cap,
                                          uint8_t *out_slot,
                                          int *out_last_session);
DM2_V1_StartupSavePathResult
dm2_v1_startup_load_session_from_save_path(const char *save_path,
                                           char *out_root,
                                           int out_root_cap,
                                           DM2_V1_SessionState *out_session,
                                           uint8_t *out_slot,
                                           int *out_last_session);
void dm2_v1_startup_menu_init(DM2_V1_StartupMenu *menu,
                              const char *save_root);
int dm2_v1_startup_menu_count_rows(int resume_available,
                                   unsigned int slot_mask);
int dm2_v1_startup_menu_scan_saves(DM2_V1_StartupMenu *menu);
int dm2_v1_startup_menu_refresh(DM2_V1_StartupMenu *menu,
                                int resume_available,
                                unsigned int slot_mask);
void dm2_v1_startup_menu_snapshot_init(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root);
int dm2_v1_startup_menu_snapshot_scan_saves(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root);
int dm2_v1_startup_menu_snapshot_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row);
int dm2_v1_startup_menu_snapshot_scan_saves_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    const char *scan_save_root);
void dm2_v1_startup_menu_state_receipt_init(
    DM2_V1_StartupMenuStateReceipt *receipt);
int dm2_v1_startup_menu_state_receipt_from_snapshot(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupMenuStateReceipt *out_receipt);
int dm2_v1_startup_menu_state_receipt_from_facts(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row);
int dm2_v1_startup_menu_state_receipt_scan_saves_from_facts(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    const char *scan_save_root);
int dm2_v1_startup_menu_state_receipt_scan_saves_from_host_facts(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const DM2_V1_StartupHostFacts *facts);
int dm2_v1_startup_menu_from_snapshot(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupMenu *out_menu);
int dm2_v1_startup_menu_snapshot_from_menu(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const DM2_V1_StartupMenu *menu);
int dm2_v1_startup_menu_snapshot_row_at(
    const DM2_V1_StartupMenuSnapshot *snapshot,
    int row,
    DM2_V1_StartupRowKind *out_kind,
    int *out_slot);
int dm2_v1_startup_menu_snapshot_handle_input(
    DM2_V1_StartupMenuSnapshot *snapshot,
    DM2_V1_StartupInput input,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_snapshot_handle_firestaff_input(
    DM2_V1_StartupMenuSnapshot *snapshot,
    int menu_input,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_firestaff_input_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const DM2_V1_StartupHostFacts *facts,
    int menu_input,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_snapshot_handle_hit(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const DM2_V1_StartupHit *hit,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_snapshot_handle_pointer(
    DM2_V1_StartupMenuSnapshot *snapshot,
    int row_count,
    int x,
    int y,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_pointer_from_facts(
    DM2_V1_StartupMenuSnapshot *snapshot,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_pointer_from_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_root,
    const char *fallback_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
    DM2_V1_StartupMenuStateReceipt *out_receipt,
    const DM2_V1_StartupHostFacts *facts,
    int x,
    int y,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_row_at(const DM2_V1_StartupMenu *menu,
                               int row,
                               DM2_V1_StartupRowKind *out_kind,
                               int *out_slot);
int dm2_v1_startup_menu_move_selected(DM2_V1_StartupMenu *menu,
                                      int delta);
int dm2_v1_startup_menu_activate_selected(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_input(DM2_V1_StartupMenu *menu,
                                     DM2_V1_StartupInput input,
                                     DM2_V1_StartupAction *out_action);
int dm2_v1_startup_menu_handle_hit(DM2_V1_StartupMenu *menu,
                                   const DM2_V1_StartupHit *hit,
                                   DM2_V1_StartupAction *out_action);
int dm2_v1_startup_plan_for_action(
    const DM2_V1_StartupAction *action,
    DM2_V1_StartupActionPlan *out_plan);
int dm2_v1_startup_execute_plan(
    const DM2_V1_StartupActionPlan *plan,
    const char *save_root,
    DM2_V1_StartupExecution *out_execution);
int dm2_v1_startup_execute_action(
    const DM2_V1_StartupAction *action,
    const char *save_root,
    DM2_V1_StartupExecution *out_execution);
int dm2_v1_startup_execution_mode_update(
    const DM2_V1_StartupExecution *execution,
    DM2_V1_StartupModeUpdate *out_update);
int dm2_v1_startup_execution_input_outcome(
    const DM2_V1_StartupExecution *execution,
    int session_applied,
    DM2_V1_StartupInputOutcome *out_outcome);
int dm2_v1_startup_apply_receipt_from_execution(
    const DM2_V1_StartupExecution *execution,
    int session_applied,
    DM2_V1_StartupApplyReceipt *out_receipt);
int dm2_v1_startup_execute_action_with_receipt(
    const DM2_V1_StartupAction *action,
    const char *save_root,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupApplyReceipt *out_receipt);
int dm2_v1_startup_execute_action_with_host_receipt(
    const DM2_V1_StartupAction *action,
    const char *save_root,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostReceipt *out_host_receipt);
void dm2_v1_startup_host_action_receipt_clear(
    DM2_V1_StartupHostActionReceipt *receipt);
int dm2_v1_startup_execute_action_from_host_facts_with_receipt(
    const DM2_V1_StartupAction *action,
    const DM2_V1_StartupHostFacts *facts,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt);
int dm2_v1_startup_execute_firestaff_input_from_host_facts_with_receipt(
    const DM2_V1_StartupHostFacts *facts,
    int menu_input,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt);
int dm2_v1_startup_execute_pointer_from_host_facts_with_receipt(
    const DM2_V1_StartupHostFacts *facts,
    int x,
    int y,
    DM2_V1_StartupSessionApplyFn apply_session,
    void *apply_userdata,
    DM2_V1_StartupExecution *out_execution,
    DM2_V1_StartupHostActionReceipt *out_receipt);
void dm2_v1_startup_host_receipt_clear(
    DM2_V1_StartupHostReceipt *receipt);
int dm2_v1_startup_host_receipt_from_apply_receipt(
    const DM2_V1_StartupApplyReceipt *apply_receipt,
    DM2_V1_StartupHostReceipt *out_receipt);
int dm2_v1_startup_execute_save_path(
    const char *save_path,
    char *out_save_root,
    int out_save_root_cap,
    DM2_V1_StartupExecution *out_execution);
int dm2_v1_startup_menu_build_render_rows(
    const DM2_V1_StartupMenu *menu,
    DM2_V1_StartupRenderRow *rows,
    int max_rows);
int dm2_v1_startup_receipt_phase(int startup_menu_active,
                                 char *out_phase,
                                 int out_phase_size,
                                 int *out_startup_active);

#endif
