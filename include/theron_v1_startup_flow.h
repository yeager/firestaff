#ifndef THERON_V1_STARTUP_FLOW_H
#define THERON_V1_STARTUP_FLOW_H

#include "theron_v1_champions.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_track02.h"
#include "theron_v1_world.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bounded Theron's Quest startup flow:
 * title/new-game -> stage select -> Soul Room mirrors -> forcefield -> dungeon.
 * The manual and contemporary PC Engine references agree that the player
 * chooses a stage, resurrects up to three heroes from seven mirrors, then
 * enters the central forcefield. This module models that gate without claiming
 * pixel, CD-DA, animation, or full Track 02 menu byte parity. */

#define THERON_STARTUP_HERO_MIRROR_COUNT 7
#define THERON_STARTUP_MAX_COMPANIONS    3

typedef enum {
    THERON_STARTUP_PHASE_TITLE        = 0,
    THERON_STARTUP_PHASE_STAGE_SELECT = 1,
    THERON_STARTUP_PHASE_SOUL_ROOM    = 2,
    THERON_STARTUP_PHASE_READY        = 3,
    THERON_STARTUP_PHASE_IN_DUNGEON   = 4
} Theron_StartupPhase;

typedef enum {
    THERON_STARTUP_OK = 0,
    THERON_STARTUP_ERR_NULL = -1,
    THERON_STARTUP_ERR_BAD_STAGE = -2,
    THERON_STARTUP_ERR_STAGE_LOCKED = -3,
    THERON_STARTUP_ERR_BAD_MIRROR = -4,
    THERON_STARTUP_ERR_DUPLICATE_MIRROR = -5,
    THERON_STARTUP_ERR_PARTY_FULL = -6,
    THERON_STARTUP_ERR_NO_STAGE = -7,
    THERON_STARTUP_ERR_NOT_READY = -8,
    THERON_STARTUP_ERR_MIRROR_NOT_SELECTED = -9,
    THERON_STARTUP_ERR_DUNGEON_ENTRY = -10,
    THERON_STARTUP_ERR_LEVEL_LOAD = -11
} Theron_StartupResult;

typedef int (*Theron_StartupLevelLoadFn)(
    Theron_V1_World *world,
    Theron_DungeonID dungeon_id,
    void *userdata,
    char *receipt,
    size_t receipt_cap);

typedef struct {
    Theron_StartupPhase phase;
    Theron_DungeonID selected_dungeon;
    uint8_t selected_mirrors_mask; /* bit 0..6 */
    uint8_t companion_count;       /* 0..3 */
    uint8_t selected_mirror_order[THERON_STARTUP_MAX_COMPANIONS];
    uint8_t theron_present;
    uint8_t forcefield_entered;
} Theron_StartupFlow;

typedef struct {
    Theron_StartupPhase phase;
    Theron_DungeonID selected_dungeon;
    uint8_t selected_mirrors_mask;
    uint8_t companion_count;
    int selected_mirror_order[THERON_STARTUP_MAX_COMPANIONS];
} Theron_StartupFlowSnapshot;

typedef struct {
    Theron_StartupPhase phase;
    int selected_dungeon;
    int selected_mirrors_mask;
    int companion_count;
    const int *selected_mirror_order;
    int selected_mirror_order_count;
} Theron_StartupFlowSnapshotRequest;

typedef struct {
    int flow_changed;
    Theron_StartupFlowSnapshot flow;
    int set_level_loaded;
    int level_loaded;
    int set_startup_cursor;
    int startup_cursor;
    int set_continue_focus;
    int continue_focus;
    int set_party_pose;
    int party_x;
    int party_y;
    int party_dir;
    int set_tick_count;
    int tick_count;
    int set_save_resume;
    int save_resume_verdict;
    int save_resume_claim;
    int save_resume_active_slot;
    int save_resume_srm_active_slot;
    int save_resume_srm_import_status;
    int save_resume_srm_current_dungeon;
    int save_resume_srm_current_level;
    int save_resume_srm_quest_mask;
    int save_resume_tqsv_slots;
    int save_resume_srm_slots;
    char save_resume_srm_root[512];
} Theron_StartupStateReceipt;

typedef struct {
    const char *name;
    Theron_ChampionClass primary_class;
    uint8_t portrait_index;
} Theron_StartupMirrorMeta;

typedef enum {
    THERON_STARTUP_INPUT_NONE = 0,
    THERON_STARTUP_INPUT_UP,
    THERON_STARTUP_INPUT_DOWN,
    THERON_STARTUP_INPUT_LEFT,
    THERON_STARTUP_INPUT_RIGHT,
    THERON_STARTUP_INPUT_ACCEPT,
    THERON_STARTUP_INPUT_ACTION,
    THERON_STARTUP_INPUT_BACK
} Theron_StartupInput;

Theron_StartupInput theron_v1_startup_input_from_firestaff_menu_code(
    int menu_input);

typedef enum {
    THERON_STARTUP_ACTION_NONE = 0,
    THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER,
    THERON_STARTUP_ACTION_SHOW_STAGE_SELECT,
    THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR,
    THERON_STARTUP_ACTION_CONTINUE_SAVE,
    THERON_STARTUP_ACTION_CHOOSE_STAGE,
    THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR,
    THERON_STARTUP_ACTION_TOGGLE_MIRROR,
    THERON_STARTUP_ACTION_ENTER_FORCEFIELD
} Theron_StartupActionKind;

typedef struct {
    Theron_StartupActionKind kind;
    Theron_DungeonID selected_dungeon;
    int cursor;
    int continue_focus;
    int mirror_index;
} Theron_StartupAction;

typedef enum {
    THERON_STARTUP_PLAN_IGNORE = 0,
    THERON_STARTUP_PLAN_RETURN_TO_LAUNCHER,
    THERON_STARTUP_PLAN_SHOW_STAGE_SELECT,
    THERON_STARTUP_PLAN_MOVE_STAGE_CURSOR,
    THERON_STARTUP_PLAN_CONTINUE_SAVE,
    THERON_STARTUP_PLAN_CHOOSE_STAGE,
    THERON_STARTUP_PLAN_MOVE_SOUL_CURSOR,
    THERON_STARTUP_PLAN_TOGGLE_MIRROR,
    THERON_STARTUP_PLAN_ENTER_FORCEFIELD
} Theron_StartupActionPlanKind;

typedef struct {
    Theron_StartupActionPlanKind kind;
    Theron_DungeonID selected_dungeon;
    int cursor;
    int continue_focus;
    int mirror_index;
    const char *status_scope;
    const char *status;
    const char *alternate_status;
    const char *failure_status;
} Theron_StartupActionPlan;

typedef struct {
    Theron_StartupResult result;
    int handled;
    int flow_changed;
    int cursor_changed;
    int cursor;
    int continue_focus_changed;
    int continue_focus;
    int mirror_selected;
} Theron_StartupExecution;

typedef enum {
    THERON_STARTUP_INPUT_RESULT_IGNORED = 0,
    THERON_STARTUP_INPUT_RESULT_REDRAW = 1,
    THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER = 2
} Theron_StartupInputResult;

typedef struct {
    Theron_StartupInputResult input_result;
    const char *status_scope;
    const char *status;
    int flow_changed;
    int cursor_changed;
    int cursor;
    int continue_focus_changed;
    int continue_focus;
} Theron_StartupApplyReceipt;

typedef struct {
    Theron_StartupInputResult input_result;
    const char *status_scope;
    const char *status;
    const char *inspect_scope;
    char inspect_detail[320];
    const char *log_first_line;
    int log_receipt;
} Theron_StartupHostReceipt;

typedef struct {
    Theron_StartupResult result;
    Theron_StartupInputResult input_result;
    const char *status_scope;
    const char *status;
} Theron_StartupInputReceipt;

typedef enum {
    THERON_STARTUP_HIT_NONE = 0,
    THERON_STARTUP_HIT_PANEL,
    THERON_STARTUP_HIT_TITLE,
    THERON_STARTUP_HIT_CONTINUE,
    THERON_STARTUP_HIT_STAGE,
    THERON_STARTUP_HIT_MIRROR,
    THERON_STARTUP_HIT_FORCEFIELD
} Theron_StartupHitKind;

typedef struct {
    Theron_StartupHitKind kind;
    Theron_DungeonID selected_dungeon;
    int mirror_index;
} Theron_StartupHit;

enum {
    THERON_STARTUP_LAYOUT_LABEL_CAPACITY = 48,
    THERON_STARTUP_RENDER_ROW_CAPACITY = 80,
    THERON_STARTUP_RENDER_TEXT_CAPACITY = 128,
    THERON_STARTUP_RENDER_TEXT_CAPACITY_MAX = 24,
    THERON_STARTUP_LAYOUT_DECODED_NAME_CAPACITY = 16,
    THERON_STARTUP_LAYOUT_DECODED_TITLE_CAPACITY = 32,
    THERON_STARTUP_LAYOUT_ROSTER_CAPACITY = 8
};

typedef enum {
    THERON_STARTUP_RENDER_TEXT_SMALL = 0,
    THERON_STARTUP_RENDER_TEXT_SHADOW = 1,
    THERON_STARTUP_RENDER_TEXT_TITLE = 2,
    THERON_STARTUP_RENDER_TEXT_ACTIVE = 3,
    THERON_STARTUP_RENDER_TEXT_LOCKED = 4,
    THERON_STARTUP_RENDER_TEXT_PICKED = 5
} Theron_StartupRenderTextStyle;

typedef enum {
    THERON_STARTUP_LAYOUT_ELEMENT_TITLE = 1,
    THERON_STARTUP_LAYOUT_ELEMENT_CHAPTER,
    THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE,
    THERON_STARTUP_LAYOUT_ELEMENT_STAGE,
    THERON_STARTUP_LAYOUT_ELEMENT_MIRROR,
    THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD
} Theron_StartupLayoutElementKind;

typedef struct {
    Theron_StartupLayoutElementKind kind;
    Theron_StartupPhase phase;
    int cursor;
    int enabled;
    int selected;
    Theron_DungeonID dungeon_id;
    int mirror_index;
    int selected_order;
    int portrait_index;
    int primary_class;
    int save_kind; /* 0=none, 1=TQSV, 2=SRM */
    int save_slot;
    int x;
    int y;
    int w;
    int h;
    char label[THERON_STARTUP_LAYOUT_LABEL_CAPACITY];
    char decoded_name[THERON_STARTUP_LAYOUT_DECODED_NAME_CAPACITY];
    char decoded_title[THERON_STARTUP_LAYOUT_DECODED_TITLE_CAPACITY];
} Theron_StartupLayoutElement;

typedef struct {
    Theron_StartupPhase phase;
    Theron_DungeonID selected_dungeon;
    const Theron_DungeonProgression *progression;
    int soul_cursor;
    int continue_focus;
    int has_tqsv_continue;
    int tqsv_slot;
    int has_srm_continue;
    int srm_slot;
    char chapter_label[THERON_STARTUP_LAYOUT_LABEL_CAPACITY];
    char startup_text_prompt[THERON_STARTUP_RENDER_ROW_CAPACITY];
    const char *startup_roster_names[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY];
    const char *startup_roster_titles[THERON_STARTUP_LAYOUT_ROSTER_CAPACITY];
    int startup_roster_name_count;
    int selected_mirrors_mask;
    const int *selected_mirror_order;
    int selected_mirror_order_count;
} Theron_StartupLayoutState;

typedef struct {
    Theron_StartupPhase phase;
    int selected_dungeon;
    const void *boot_profile; /* Theron_V1_BootProfile*, kept opaque here. */
    const Theron_V1_World *world;
    int soul_cursor;
    int continue_focus;
    int has_tqsv_continue;
    int tqsv_slot;
    int has_srm_continue;
    int srm_slot;
    const char *startup_text_prompt;
    const char *const *startup_roster_names;
    const char *const *startup_roster_titles;
    int startup_roster_name_count;
    int selected_mirrors_mask;
    const int *selected_mirror_order;
    int selected_mirror_order_count;
} Theron_StartupLayoutStateRequest;

typedef struct {
    const void *boot_profile; /* Theron_V1_BootProfile*, kept opaque here. */
    const Theron_V1_World *world;
    const char *prefix;
} Theron_StartupChapterInspectRequest;

typedef struct {
    char inspect_scope[16];
    char inspect_detail[320];
    char marker_line[192];
} Theron_StartupChapterInspectReceipt;

typedef struct {
    int x;
    int y;
    Theron_StartupRenderTextStyle style;
    char text[THERON_STARTUP_RENDER_TEXT_CAPACITY];
} Theron_StartupRenderTextCommand;

typedef enum {
    THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT = 0,
    THERON_STARTUP_RENDER_GRAPHIC_DRAW_RECT,
    THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK,
    THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL,
    THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME,
    THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD
} Theron_StartupRenderGraphicKind;

typedef struct {
    Theron_StartupRenderGraphicKind kind;
    int x;
    int y;
    int w;
    int h;
    int color;
    int color2;
    int selected;
    int cursor;
    int ordinal;
} Theron_StartupRenderGraphicCommand;

#define THERON_STARTUP_RENDER_GRAPHIC_CAPACITY_MAX 32

typedef struct {
    Theron_StartupPhase phase;
    int background_color;
    int border_x;
    int border_y;
    int border_w;
    int border_h;
    int border_color;
    Theron_StartupRenderTextCommand text[
        THERON_STARTUP_RENDER_TEXT_CAPACITY_MAX];
    int text_count;
    Theron_StartupRenderGraphicCommand graphics[
        THERON_STARTUP_RENDER_GRAPHIC_CAPACITY_MAX];
    int graphic_count;
} Theron_StartupRenderPlan;

typedef void (*Theron_StartupFillRectFn)(
    void *userdata,
    int x,
    int y,
    int w,
    int h,
    int color);

typedef void (*Theron_StartupDrawRectFn)(
    void *userdata,
    int x,
    int y,
    int w,
    int h,
    int color);

typedef void (*Theron_StartupPlotPixelFn)(
    void *userdata,
    int x,
    int y,
    int color);

typedef struct {
    void *userdata;
    Theron_StartupFillRectFn fill_rect;
    Theron_StartupDrawRectFn draw_rect;
    Theron_StartupPlotPixelFn plot_pixel;
} Theron_StartupGraphicExecutor;

void theron_v1_startup_flow_init(Theron_StartupFlow *flow);
void theron_v1_startup_flow_snapshot_init(Theron_StartupFlowSnapshot *snapshot);
int theron_v1_startup_flow_snapshot_from_request(
    const Theron_StartupFlowSnapshotRequest *request,
    Theron_StartupFlowSnapshot *out_snapshot);
void theron_v1_startup_state_receipt_init(
    Theron_StartupStateReceipt *receipt);
void theron_v1_startup_flow_capture_snapshot(
    const Theron_StartupFlow *flow,
    Theron_StartupFlowSnapshot *snapshot);
int theron_v1_startup_state_receipt_from_flow(
    const Theron_StartupFlow *flow,
    Theron_StartupStateReceipt *out_receipt);
int theron_v1_startup_initial_title_state_receipt(
    const Theron_V1_World *world,
    Theron_StartupFlow *flow,
    Theron_StartupStateReceipt *out_receipt);
Theron_StartupResult theron_v1_startup_flow_rebuild_from_snapshot(
    const Theron_StartupFlowSnapshot *snapshot,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow);
Theron_StartupResult theron_v1_startup_flow_rebuild_from_request(
    const Theron_StartupFlowSnapshotRequest *request,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow);
Theron_StartupResult theron_v1_startup_flow_rebuild_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow);
Theron_StartupResult theron_v1_startup_flow_rebuild_from_facts_with_receipt(
    Theron_StartupPhase phase,
    int selected_dungeon,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow,
    Theron_StartupStateReceipt *out_receipt);
void theron_v1_startup_action_init(Theron_StartupAction *action);
void theron_v1_startup_action_plan_init(Theron_StartupActionPlan *plan);
void theron_v1_startup_hit_init(Theron_StartupHit *hit);
void theron_v1_startup_layout_state_init(Theron_StartupLayoutState *state);
int theron_v1_startup_layout_state_from_request(
    const Theron_StartupLayoutStateRequest *request,
    Theron_StartupLayoutState *out_state);
int theron_v1_startup_layout_state_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupLayoutState *out_state);
void theron_v1_startup_chapter_inspect_receipt_init(
    Theron_StartupChapterInspectReceipt *receipt);
int theron_v1_startup_chapter_inspect_receipt_from_request(
    const Theron_StartupChapterInspectRequest *request,
    Theron_StartupChapterInspectReceipt *out_receipt);
int theron_v1_startup_chapter_inspect_receipt_from_facts(
    const void *boot_profile,
    const Theron_V1_World *world,
    const char *prefix,
    Theron_StartupChapterInspectReceipt *out_receipt);
Theron_StartupResult theron_v1_startup_choose_stage(
    Theron_StartupFlow *flow,
    const Theron_DungeonProgression *progression,
    Theron_DungeonID dungeon_id);
Theron_StartupResult theron_v1_startup_show_stage_select(
    Theron_StartupFlow *flow,
    Theron_DungeonID dungeon_id);
Theron_StartupResult theron_v1_startup_select_mirror(
    Theron_StartupFlow *flow,
    int mirror_index);
Theron_StartupResult theron_v1_startup_deselect_mirror(
    Theron_StartupFlow *flow,
    int mirror_index);
Theron_StartupResult theron_v1_startup_toggle_mirror(
    Theron_StartupFlow *flow,
    int mirror_index,
    int *out_selected);
Theron_StartupResult theron_v1_startup_enter_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_Party *party);
Theron_StartupResult theron_v1_startup_enter_forcefield_with_roster(
    Theron_StartupFlow *flow,
    Theron_V1_Party *party,
    const char *const roster_names[],
    int roster_name_count);
Theron_StartupResult theron_v1_startup_enter_world_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world);
Theron_StartupResult theron_v1_startup_enter_runtime_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    Theron_StartupLevelLoadFn load_level,
    void *userdata,
    char *receipt,
    size_t receipt_cap);
Theron_StartupResult theron_v1_startup_return_to_stage_select_after_exit(
    Theron_V1_World *world,
    Theron_StartupFlow *flow,
    char *receipt,
    size_t receipt_cap);
Theron_StartupResult theron_v1_startup_return_to_stage_select_after_exit_with_receipt(
    Theron_V1_World *world,
    Theron_StartupFlow *flow,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_stage_available(
    const Theron_DungeonProgression *progression,
    Theron_DungeonID dungeon_id);
Theron_StartupResult theron_v1_startup_handle_input(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    Theron_StartupInput input,
    Theron_StartupAction *out_action);
Theron_StartupResult theron_v1_startup_handle_input_with_progression(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    const Theron_DungeonProgression *progression,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    Theron_StartupInput input,
    Theron_StartupAction *out_action);
Theron_StartupResult theron_v1_startup_handle_input_from_layout_state(
    const Theron_StartupLayoutState *state,
    Theron_StartupInput input,
    Theron_StartupAction *out_action);
Theron_StartupResult theron_v1_startup_handle_input_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupInput input,
    Theron_StartupAction *out_action);
int theron_v1_startup_handle_input_from_facts_with_receipt(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupInput input,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt);
int theron_v1_startup_handle_input_from_session_facts_with_receipt(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupInput input,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt);
Theron_StartupResult theron_v1_startup_handle_hit(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    const Theron_StartupHit *hit,
    Theron_StartupAction *out_action);
Theron_StartupResult theron_v1_startup_handle_hit_with_progression(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    const Theron_DungeonProgression *progression,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    const Theron_StartupHit *hit,
    Theron_StartupAction *out_action);
int theron_v1_startup_plan_for_action(
    const Theron_StartupAction *action,
    Theron_StartupActionPlan *out_plan);
void theron_v1_startup_execution_init(Theron_StartupExecution *execution);
void theron_v1_startup_apply_receipt_init(
    Theron_StartupApplyReceipt *receipt);
void theron_v1_startup_host_receipt_init(
    Theron_StartupHostReceipt *receipt);
int theron_v1_startup_host_receipt_from_flow_apply(
    const Theron_StartupApplyReceipt *apply_receipt,
    Theron_StartupHostReceipt *out_receipt);
int theron_v1_startup_execute_flow_plan(
    const Theron_StartupActionPlan *plan,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow,
    Theron_StartupExecution *out_execution);
int theron_v1_startup_apply_receipt_from_flow_execution(
    const Theron_StartupActionPlan *plan,
    const Theron_StartupExecution *execution,
    Theron_StartupApplyReceipt *out_receipt);
int theron_v1_startup_state_receipt_from_flow_apply(
    const Theron_StartupFlow *flow,
    const Theron_StartupApplyReceipt *apply_receipt,
    Theron_StartupStateReceipt *out_receipt);
int theron_v1_startup_execute_flow_plan_with_receipts(
    const Theron_StartupActionPlan *plan,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow,
    Theron_StartupExecution *out_execution,
    Theron_StartupApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt);
int theron_v1_startup_execute_flow_plan_from_facts_with_receipts(
    const Theron_StartupActionPlan *plan,
    Theron_StartupPhase phase,
    int selected_dungeon,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *out_flow,
    Theron_StartupExecution *out_execution,
    Theron_StartupApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt);
int theron_v1_startup_execute_flow_plan_from_facts_with_host_receipts(
    const Theron_StartupActionPlan *plan,
    Theron_StartupPhase phase,
    int selected_dungeon,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *out_flow,
    Theron_StartupExecution *out_execution,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt);
int theron_v1_startup_layout_build(
    const Theron_StartupLayoutState *state,
    Theron_StartupLayoutElement *elements,
    int max_elements);
int theron_v1_startup_layout_build_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupLayoutElement *elements,
    int max_elements);
int theron_v1_startup_layout_build_from_session_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupLayoutElement *elements,
    int max_elements);
int theron_v1_startup_layout_hit_at(
    Theron_StartupPhase phase,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    int x,
    int y,
    Theron_StartupHit *out_hit);
int theron_v1_startup_handle_pointer_from_layout_state(
    const Theron_StartupLayoutState *state,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    int x,
    int y,
    Theron_StartupResult *out_result,
    Theron_StartupAction *out_action);
int theron_v1_startup_handle_pointer_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y,
    Theron_StartupResult *out_result,
    Theron_StartupAction *out_action);
int theron_v1_startup_handle_pointer_from_facts_with_receipt(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt);
int theron_v1_startup_handle_pointer_from_session_facts_with_receipt(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt);
int theron_v1_startup_render_rows_build(
    const Theron_StartupLayoutState *state,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows);
int theron_v1_startup_render_rows_build_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows);
int theron_v1_startup_render_rows_build_from_session_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows);
int theron_v1_startup_render_plan_build(
    const Theron_StartupLayoutState *state,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    Theron_StartupRenderPlan *out_plan);
int theron_v1_startup_render_plan_build_from_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int has_tqsv_continue,
    int tqsv_slot,
    int has_srm_continue,
    int srm_slot,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupRenderPlan *out_plan);
int theron_v1_startup_render_plan_build_from_session_facts(
    Theron_StartupPhase phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    int soul_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    Theron_StartupRenderPlan *out_plan);
int theron_v1_startup_execute_graphics_plan(
    const Theron_StartupRenderPlan *plan,
    const Theron_StartupGraphicExecutor *executor);
int theron_v1_startup_receipt_phase(
    Theron_StartupPhase phase,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active);

const char *theron_v1_startup_phase_name(Theron_StartupPhase phase);
const char *theron_v1_startup_result_name(Theron_StartupResult result);
const char *theron_v1_startup_action_name(Theron_StartupActionKind action);
const char *theron_v1_startup_flow_source_evidence(void);
const Theron_StartupMirrorMeta *theron_v1_startup_mirror_meta(int mirror_index);
int theron_v1_startup_roster_index_for_mirror(int mirror_index);
int theron_v1_startup_mirror_index_for_roster(int roster_index);
const char *theron_v1_startup_class_name(Theron_ChampionClass cls);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_FLOW_H */
