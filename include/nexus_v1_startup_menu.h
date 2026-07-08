#ifndef NEXUS_V1_STARTUP_MENU_H
#define NEXUS_V1_STARTUP_MENU_H

#include <stdint.h>
#include <stddef.h>

#include "nexus_v1_champions.h"
#include "nexus_v1_save.h"
#include "nexus_v1_startup_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NEXUS_V1_STARTUP_ROW_NONE = 0,
    NEXUS_V1_STARTUP_ROW_SLOT = 1,
    NEXUS_V1_STARTUP_ROW_NEW_GAME = 2
} Nexus_V1_StartupRowKind;

typedef enum {
    NEXUS_V1_STARTUP_ACTION_NONE = 0,
    NEXUS_V1_STARTUP_ACTION_LOAD_SLOT = 1,
    NEXUS_V1_STARTUP_ACTION_NEW_GAME = 2,
    NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER = 3,
    NEXUS_V1_STARTUP_ACTION_HOLD_TITLE = 4,
    NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT = 5,
    NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT = 6,
    NEXUS_V1_STARTUP_ACTION_BACK_TO_TITLE = 7,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_CURSOR = 8,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_ADDED = 9,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_SKIPPED = 10,
    NEXUS_V1_STARTUP_ACTION_CHAMPION_REMOVED = 11,
    NEXUS_V1_STARTUP_ACTION_START_DUNGEON = 12,
    NEXUS_V1_STARTUP_ACTION_NEED_CHAMPION = 13
} Nexus_V1_StartupActionKind;

typedef enum {
    NEXUS_V1_STARTUP_INPUT_NONE = 0,
    NEXUS_V1_STARTUP_INPUT_UP = 1,
    NEXUS_V1_STARTUP_INPUT_DOWN = 2,
    NEXUS_V1_STARTUP_INPUT_ACCEPT = 3,
    NEXUS_V1_STARTUP_INPUT_ACTION = 4,
    NEXUS_V1_STARTUP_INPUT_BACK = 5
} Nexus_V1_StartupInput;

Nexus_V1_StartupInput nexus_v1_startup_input_from_firestaff_menu_code(
    int menu_input);

typedef struct {
    Nexus_V1_StartupActionKind kind;
    int row;
    int slot;
    char path[512];
} Nexus_V1_StartupAction;

typedef enum {
    NEXUS_V1_STARTUP_SAVE_EXEC_IGNORE = 0,
    NEXUS_V1_STARTUP_SAVE_EXEC_STATUS_REDRAW = 1,
    NEXUS_V1_STARTUP_SAVE_EXEC_LOAD_SLOT = 2,
    NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_CHAMPIONS = 3,
    NEXUS_V1_STARTUP_SAVE_EXEC_SHOW_TITLE = 4
} Nexus_V1_StartupSaveExecutionKind;

typedef struct {
    Nexus_V1_StartupSaveExecutionKind kind;
    const char *status_scope;
    const char *status;
    const char *failure_status;
    char path[512];
} Nexus_V1_StartupSaveExecution;

typedef enum {
    NEXUS_V1_STARTUP_TITLE_EXEC_IGNORE = 0,
    NEXUS_V1_STARTUP_TITLE_EXEC_RETURN_TO_LAUNCHER = 1,
    NEXUS_V1_STARTUP_TITLE_EXEC_HOLD_TITLE = 2,
    NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_SAVE_SELECT = 3,
    NEXUS_V1_STARTUP_TITLE_EXEC_SHOW_CHAMPIONS = 4
} Nexus_V1_StartupTitleExecutionKind;

typedef struct {
    Nexus_V1_StartupTitleExecutionKind kind;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupTitleExecution;

typedef enum {
    NEXUS_V1_STARTUP_CHAMPION_EXEC_IGNORE = 0,
    NEXUS_V1_STARTUP_CHAMPION_EXEC_REDRAW = 1,
    NEXUS_V1_STARTUP_CHAMPION_EXEC_SET_CURSOR = 2,
    NEXUS_V1_STARTUP_CHAMPION_EXEC_START_DUNGEON = 3,
    NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_SAVE_SELECT = 4,
    NEXUS_V1_STARTUP_CHAMPION_EXEC_SHOW_TITLE = 5
} Nexus_V1_StartupChampionExecutionKind;

typedef struct {
    Nexus_V1_StartupChampionExecutionKind kind;
    int cursor;
    int select_last_save_row;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupChampionExecution;

typedef struct {
    int set_title_active;
    int title_active;
    int set_title_frame;
    int title_frame;
    int set_save_select_active;
    int save_select_active;
    int set_save_selected_row;
    int save_selected_row;
    int set_champion_select_active;
    int champion_select_active;
    int set_champion_cursor;
    int champion_cursor;
    int set_champion_frame;
    int champion_frame;
} Nexus_V1_StartupModeUpdate;

typedef enum {
    NEXUS_V1_STARTUP_APPLY_RESULT_IGNORE = 0,
    NEXUS_V1_STARTUP_APPLY_RESULT_REDRAW = 1,
    NEXUS_V1_STARTUP_APPLY_RESULT_RETURN_TO_LAUNCHER = 2
} Nexus_V1_StartupApplyResult;

typedef struct {
    Nexus_V1_StartupApplyResult result;
    Nexus_V1_StartupModeUpdate mode_update;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupApplyReceipt;

typedef enum {
    NEXUS_V1_STARTUP_HOST_INPUT_IGNORED = 0,
    NEXUS_V1_STARTUP_HOST_INPUT_REDRAW = 1,
    NEXUS_V1_STARTUP_HOST_INPUT_RETURN_TO_LAUNCHER = 2
} Nexus_V1_StartupHostInputResult;

typedef struct {
    Nexus_V1_StartupModeUpdate mode_update;
    Nexus_V1_StartupHostInputResult input_result;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupHostReceipt;

typedef int (*Nexus_V1_StartupLoadSaveFn)(void *userdata,
                                          const char *save_path);

typedef struct {
    char save_dir[512];
    unsigned int slot_mask;
    int row_count;
    int selected_row;
    Nexus_V1_SaveSlot slots[NEXUS_SAVE_MAX_SLOTS];
} Nexus_V1_StartupMenu;

typedef struct {
    char save_dir[512];
    unsigned int slot_mask;
    int row_count;
    int selected_row;
} Nexus_V1_StartupMenuSnapshot;

typedef struct {
    unsigned int slot_mask;
    int cursor;
    int frame;
} Nexus_V1_StartupChampionSnapshot;

typedef struct {
    int title_active;
    int title_frame;
    int save_select_active;
    int champion_select_active;
    const char *save_dir;
    unsigned int slot_mask;
    int save_selected_row;
    int save_row_count;
    Nexus_V1_ChampionPool *champion_pool;
    int champion_cursor;
    int champion_frame;
} Nexus_V1_StartupHostFacts;

typedef struct {
    char save_dir[512];
    unsigned int slot_mask;
    int row_count;
    int selected_row;
} Nexus_V1_StartupMenuStateReceipt;

typedef struct {
    unsigned int slot_mask;
    int cursor;
    int frame;
} Nexus_V1_StartupChampionStateReceipt;

typedef struct {
    int save_state_receipt_valid;
    Nexus_V1_StartupMenuStateReceipt save_state_receipt;
    int champion_state_receipt_valid;
    Nexus_V1_StartupChampionStateReceipt champion_state_receipt;
    Nexus_V1_StartupHostReceipt host_receipt;
} Nexus_V1_StartupHostActionReceipt;

typedef struct {
    Nexus_V1_StartupHostReceipt host_receipt;
} Nexus_V1_StartupIdleReceipt;

enum {
    NEXUS_V1_STARTUP_SAVE_ROW_LABEL_CAPACITY = 96,
    NEXUS_V1_STARTUP_CHROME_LABEL_CAPACITY = 96,
    NEXUS_V1_STARTUP_CHAMPION_ROW_LABEL_CAPACITY = 96,
    NEXUS_V1_STARTUP_CHAMPION_FOOTER_LABEL_CAPACITY = 96,
    NEXUS_V1_STARTUP_DRAW_LABEL_CAPACITY = 96
};

typedef enum {
    NEXUS_V1_STARTUP_DRAW_NONE = 0,
    NEXUS_V1_STARTUP_DRAW_TITLE_BACKGROUND = 1,
    NEXUS_V1_STARTUP_DRAW_FILL_RECT = 2,
    NEXUS_V1_STARTUP_DRAW_OUTLINE_RECT = 3,
    NEXUS_V1_STARTUP_DRAW_TEXT = 4,
    NEXUS_V1_STARTUP_DRAW_PORTRAIT = 5,
    NEXUS_V1_STARTUP_DRAW_BOOT_TITLE_FRAME = 6,
    NEXUS_V1_STARTUP_DRAW_WARNING_BACKGROUND = 7
} Nexus_V1_StartupDrawKind;

typedef enum {
    NEXUS_V1_STARTUP_TEXT_SMALL = 0,
    NEXUS_V1_STARTUP_TEXT_SHADOW = 1,
    NEXUS_V1_STARTUP_TEXT_TITLE = 2
} Nexus_V1_StartupTextStyle;

typedef struct {
    Nexus_V1_StartupDrawKind kind;
    Nexus_V1_StartupRect rect;
    int x;
    int y;
    int portrait_index;
    int title_frame;
    int text_color;
    int shadow_color;
    Nexus_V1_StartupTextStyle text_style;
    char label[NEXUS_V1_STARTUP_DRAW_LABEL_CAPACITY];
} Nexus_V1_StartupDrawCommand;

typedef void (*Nexus_V1_StartupDrawFn)(
    void *userdata,
    const Nexus_V1_StartupDrawCommand *command);

typedef struct {
    void *userdata;
    Nexus_V1_StartupDrawFn draw_title_background;
    Nexus_V1_StartupDrawFn fill_rect;
    Nexus_V1_StartupDrawFn outline_rect;
    Nexus_V1_StartupDrawFn draw_text;
    Nexus_V1_StartupDrawFn draw_portrait;
    Nexus_V1_StartupDrawFn draw_boot_title_frame;
    Nexus_V1_StartupDrawFn draw_warning_background;
} Nexus_V1_StartupDrawExecutor;

typedef struct {
    Nexus_V1_StartupRowKind kind;
    int row;
    int slot;
    int selected;
    Nexus_V1_StartupRect rect;
    Nexus_V1_StartupRect highlight_rect;
    int text_x;
    int text_y;
    char label[NEXUS_V1_STARTUP_SAVE_ROW_LABEL_CAPACITY];
} Nexus_V1_StartupSaveRenderRow;

typedef struct {
    int title_x;
    int title_y;
    int subtitle_x;
    int subtitle_y;
    int footer_x;
    int footer_y;
    char title[NEXUS_V1_STARTUP_CHROME_LABEL_CAPACITY];
    char subtitle[NEXUS_V1_STARTUP_CHROME_LABEL_CAPACITY];
    char footer[NEXUS_V1_STARTUP_CHROME_LABEL_CAPACITY];
} Nexus_V1_StartupChromeRender;

typedef struct {
    int row;
    int selected;
    int in_party;
    int portrait_index;
    Nexus_V1_StartupRect rect;
    Nexus_V1_StartupRect highlight_rect;
    int portrait_x;
    int portrait_y;
    int portrait_w;
    int portrait_h;
    int highlight_visible;
    int text_color;
    int shadow_color;
    int portrait_border_color;
    int party_marker_color;
    int text_x;
    int text_y;
    char label[NEXUS_V1_STARTUP_CHAMPION_ROW_LABEL_CAPACITY];
} Nexus_V1_StartupChampionRenderRow;

typedef struct {
    Nexus_V1_StartupRect rect;
    int text_x;
    int text_y;
    char label[NEXUS_V1_STARTUP_CHAMPION_FOOTER_LABEL_CAPACITY];
} Nexus_V1_StartupChampionFooterRender;

void nexus_v1_startup_menu_init(Nexus_V1_StartupMenu *menu,
                                const char *save_dir);
int nexus_v1_startup_menu_scan(Nexus_V1_StartupMenu *menu);
int nexus_v1_startup_menu_scan_or_new_game(Nexus_V1_StartupMenu *menu);
int nexus_v1_startup_menu_refresh(Nexus_V1_StartupMenu *menu,
                                  unsigned int slot_mask);
int nexus_v1_startup_menu_row_at(const Nexus_V1_StartupMenu *menu,
                                 int row,
                                 Nexus_V1_StartupRowKind *out_kind,
                                 int *out_slot);
int nexus_v1_startup_menu_selected_path(const Nexus_V1_StartupMenu *menu,
                                        char *out_path,
                                        size_t out_path_size);
int nexus_v1_startup_menu_move_selected(Nexus_V1_StartupMenu *menu,
                                        int delta);
int nexus_v1_startup_menu_activate_selected(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_title_handle_input(int title_frame,
                                        unsigned int slot_mask,
                                        Nexus_V1_StartupInput input,
                                        Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_boot_handle_input(int boot_frame,
                                       unsigned int slot_mask,
                                       Nexus_V1_StartupInput input,
                                       Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_title_handle_hit(int title_frame,
                                      unsigned int slot_mask,
                                      Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_input(Nexus_V1_StartupMenu *menu,
                                       Nexus_V1_StartupInput input,
                                       Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_hit(Nexus_V1_StartupMenu *menu,
                                     const Nexus_V1_StartupHit *hit,
                                     Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_snapshot_refresh(
    Nexus_V1_StartupMenuSnapshot *snapshot);
int nexus_v1_startup_menu_snapshot_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row);
int nexus_v1_startup_menu_snapshot_scan_or_new_game_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    int selected_row);
void nexus_v1_startup_menu_state_receipt_init(
    Nexus_V1_StartupMenuStateReceipt *receipt);
int nexus_v1_startup_menu_state_receipt_from_snapshot(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupMenuStateReceipt *out_receipt);
int nexus_v1_startup_menu_state_receipt_from_facts(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row);
int nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_facts(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    int selected_row);
int nexus_v1_startup_menu_state_receipt_scan_or_new_game_from_host_facts(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts);
int nexus_v1_startup_menu_snapshot_row_at(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    int row,
    Nexus_V1_StartupRowKind *out_kind,
    int *out_slot);
int nexus_v1_startup_menu_snapshot_handle_input(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupInput input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_firestaff_input_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_firestaff_input_from_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_firestaff_input_from_host_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_snapshot_handle_hit(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const Nexus_V1_StartupHit *hit,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_pointer_from_facts(
    Nexus_V1_StartupMenuSnapshot *snapshot,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int row_count,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_pointer_from_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    int row_count,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_menu_handle_pointer_from_host_facts_with_receipt(
    Nexus_V1_StartupMenuStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_execute_title_action(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupTitleExecution *out_execution);
int nexus_v1_startup_execute_save_action(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupSaveExecution *out_execution);
int nexus_v1_startup_save_execution_mode_update(
    const Nexus_V1_StartupSaveExecution *execution,
    Nexus_V1_StartupModeUpdate *out_update);
int nexus_v1_startup_apply_receipt_from_save_execution(
    const Nexus_V1_StartupSaveExecution *execution,
    int load_success,
    Nexus_V1_StartupApplyReceipt *out_receipt);
void nexus_v1_startup_host_receipt_clear(
    Nexus_V1_StartupHostReceipt *receipt);
int nexus_v1_startup_host_receipt_from_apply_receipt(
    const Nexus_V1_StartupApplyReceipt *apply_receipt,
    Nexus_V1_StartupHostReceipt *out_receipt);
int nexus_v1_startup_execute_save_action_with_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupApplyReceipt *out_receipt);
int nexus_v1_startup_execute_save_action_with_host_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostReceipt *out_host_receipt);
void nexus_v1_startup_host_action_receipt_clear(
    Nexus_V1_StartupHostActionReceipt *receipt);
void nexus_v1_startup_idle_receipt_clear(
    Nexus_V1_StartupIdleReceipt *receipt);
int nexus_v1_startup_advance_idle_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupIdleReceipt *out_receipt);
int nexus_v1_startup_execute_save_firestaff_input_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_startup_execute_save_pointer_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_startup_execute_champion_action(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupChampionExecution *out_execution);
int nexus_v1_startup_title_execution_mode_update(
    const Nexus_V1_StartupTitleExecution *execution,
    Nexus_V1_StartupModeUpdate *out_update);
int nexus_v1_startup_apply_receipt_from_title_execution(
    const Nexus_V1_StartupTitleExecution *execution,
    Nexus_V1_StartupApplyReceipt *out_receipt);
int nexus_v1_startup_execute_title_action_with_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupApplyReceipt *out_receipt);
int nexus_v1_startup_execute_title_action_with_host_receipt(
    const Nexus_V1_StartupAction *action,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostReceipt *out_host_receipt);
int nexus_v1_startup_execute_title_firestaff_input_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_startup_execute_title_pointer_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_startup_champion_execution_mode_update(
    const Nexus_V1_StartupChampionExecution *execution,
    int save_row_count,
    Nexus_V1_StartupModeUpdate *out_update);
int nexus_v1_startup_apply_receipt_from_champion_execution(
    const Nexus_V1_StartupChampionExecution *execution,
    int save_row_count,
    Nexus_V1_StartupApplyReceipt *out_receipt);
int nexus_v1_startup_execute_champion_action_with_receipt(
    const Nexus_V1_StartupAction *action,
    int save_row_count,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupApplyReceipt *out_receipt);
int nexus_v1_startup_execute_champion_action_with_host_receipt(
    const Nexus_V1_StartupAction *action,
    int save_row_count,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostReceipt *out_host_receipt);
int nexus_v1_startup_execute_champion_firestaff_input_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_startup_execute_champion_pointer_from_host_facts_with_receipt(
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_startup_menu_build_save_render_rows(
    const Nexus_V1_StartupMenu *menu,
    Nexus_V1_StartupSaveRenderRow *rows,
    int max_rows);
int nexus_v1_startup_menu_snapshot_build_save_render_rows(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupSaveRenderRow *rows,
    int max_rows);
int nexus_v1_startup_menu_build_save_chrome_render(
    Nexus_V1_StartupChromeRender *out_chrome);
int nexus_v1_startup_menu_build_champion_render_rows(
    const Nexus_V1_ChampionPool *pool,
    int cursor,
    Nexus_V1_StartupChampionRenderRow *rows,
    int max_rows,
    Nexus_V1_StartupChampionFooterRender *out_footer);
int nexus_v1_startup_menu_build_champion_render_rows_for_frame(
    const Nexus_V1_ChampionPool *pool,
    int cursor,
    int frame,
    Nexus_V1_StartupChampionRenderRow *rows,
    int max_rows,
    Nexus_V1_StartupChampionFooterRender *out_footer);
int nexus_v1_startup_champion_snapshot_refresh(
    const Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot);
int nexus_v1_startup_champion_snapshot_from_facts(
    const Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    unsigned int slot_mask,
    int cursor,
    int frame);
void nexus_v1_startup_champion_state_receipt_init(
    Nexus_V1_StartupChampionStateReceipt *receipt);
int nexus_v1_startup_champion_state_receipt_from_snapshot(
    const Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupChampionStateReceipt *out_receipt);
int nexus_v1_startup_champion_state_receipt_from_facts(
    const Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    unsigned int slot_mask,
    int cursor,
    int frame);
int nexus_v1_startup_champion_snapshot_handle_input(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupInput input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_firestaff_input_from_facts(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int firestaff_input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_firestaff_input_from_facts_with_receipt(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int firestaff_input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_firestaff_input_from_host_facts_with_receipt(
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int firestaff_input,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_snapshot_handle_hit(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    const Nexus_V1_StartupHit *hit,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_pointer_from_facts(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionSnapshot *snapshot,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_pointer_from_facts_with_receipt(
    Nexus_V1_ChampionPool *pool,
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    unsigned int slot_mask,
    int cursor,
    int frame,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_pointer_from_host_facts_with_receipt(
    Nexus_V1_StartupChampionStateReceipt *out_receipt,
    const Nexus_V1_StartupHostFacts *facts,
    int x,
    int y,
    Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_snapshot_build_render_rows(
    const Nexus_V1_ChampionPool *pool,
    const Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupChampionRenderRow *rows,
    int max_rows,
    Nexus_V1_StartupChampionFooterRender *out_footer);
int nexus_v1_startup_menu_build_champion_chrome_render(
    Nexus_V1_StartupChromeRender *out_chrome);
int nexus_v1_startup_presentation_build_title(
    int title_frame,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_build_save(
    const Nexus_V1_StartupMenuSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_build_save_from_facts(
    const char *save_dir,
    unsigned int slot_mask,
    int selected_row,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_build_save_from_host_facts(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_build_champion(
    const Nexus_V1_ChampionPool *pool,
    const Nexus_V1_StartupChampionSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_build_champion_from_facts(
    const Nexus_V1_ChampionPool *pool,
    unsigned int slot_mask,
    int cursor,
    int frame,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_build_champion_from_host_facts(
    const Nexus_V1_StartupHostFacts *facts,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_startup_presentation_execute(
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    const Nexus_V1_StartupDrawExecutor *executor);
int nexus_v1_startup_champion_handle_input(Nexus_V1_ChampionPool *pool,
                                           int *cursor,
                                           unsigned int slot_mask,
                                           Nexus_V1_StartupInput input,
                                           Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_champion_handle_hit(Nexus_V1_ChampionPool *pool,
                                         int *cursor,
                                         unsigned int slot_mask,
                                         const Nexus_V1_StartupHit *hit,
                                         Nexus_V1_StartupAction *out_action);
int nexus_v1_startup_receipt_phase(int title_active,
                                   int save_select_active,
                                   int champion_select_active,
                                   int title_frame,
                                   char *out_phase,
                                   int out_phase_size,
                                   int *out_startup_active,
                                   int *out_startup_frame);
int nexus_v1_startup_presentation_receipt(int title_active,
                                          int save_select_active,
                                          int champion_select_active,
                                          int title_frame,
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

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_STARTUP_MENU_H */
