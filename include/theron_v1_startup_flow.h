#ifndef THERON_V1_STARTUP_FLOW_H
#define THERON_V1_STARTUP_FLOW_H

#include "theron_v1_champions.h"
#include "theron_v1_dungeon_progression.h"
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
    THERON_STARTUP_ERR_MIRROR_NOT_SELECTED = -9
} Theron_StartupResult;

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
    THERON_STARTUP_LAYOUT_LABEL_CAPACITY = 48
};

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
    int selected_mirrors_mask;
    const int *selected_mirror_order;
    int selected_mirror_order_count;
} Theron_StartupLayoutState;

void theron_v1_startup_flow_init(Theron_StartupFlow *flow);
void theron_v1_startup_action_init(Theron_StartupAction *action);
void theron_v1_startup_action_plan_init(Theron_StartupActionPlan *plan);
void theron_v1_startup_hit_init(Theron_StartupHit *hit);
void theron_v1_startup_layout_state_init(Theron_StartupLayoutState *state);
Theron_StartupResult theron_v1_startup_choose_stage(
    Theron_StartupFlow *flow,
    const Theron_DungeonProgression *progression,
    Theron_DungeonID dungeon_id);
Theron_StartupResult theron_v1_startup_select_mirror(
    Theron_StartupFlow *flow,
    int mirror_index);
Theron_StartupResult theron_v1_startup_deselect_mirror(
    Theron_StartupFlow *flow,
    int mirror_index);
Theron_StartupResult theron_v1_startup_enter_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_Party *party);
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
int theron_v1_startup_layout_build(
    const Theron_StartupLayoutState *state,
    Theron_StartupLayoutElement *elements,
    int max_elements);
int theron_v1_startup_layout_hit_at(
    Theron_StartupPhase phase,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    int x,
    int y,
    Theron_StartupHit *out_hit);
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
