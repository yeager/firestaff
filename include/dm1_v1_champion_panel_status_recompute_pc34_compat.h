#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PC34_COMPAT_H

/*
 * DM1 V1 champion panel status recompute contract.
 *
 * Source-lock anchors:
 * - ReDMCSB PANEL.C F0344_INVENTORY_DrawPanel_FoodOrWaterBar:1493-1561
 *   maps food/water amounts to the panel bar extent and red/yellow/base color.
 * - ReDMCSB PANEL.C F0345_INVENTORY_DrawPanel_FoodWaterPoisoned:1563-1616
 *   redraws the food/water/poisoned panel, emits the poison label only when
 *   PoisonEventCount is non-zero, and redraws food and water bars.
 * - ReDMCSB PANEL.C F0347_INVENTORY_DrawPanel:1639-1691 selects the visible
 *   panel from the inventory champion action hand: empty/non-panel object falls
 *   back to food/water/poisoned, while chest/scroll/object content uses the
 *   object panel path.
 * - ReDMCSB PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:1832-1949
 *   mutates water/food/stamina/health, marks STATISTICS and sometimes PANEL,
 *   then calls F0292_CHAMPION_DrawState for the inventory champion.
 * - ReDMCSB CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 recomputes status
 *   bars, inventory HP/stamina/mana values, and the mouth warning border from
 *   food/water/poison state without requiring a status-box redraw.
 *
 * Contract only: this slice uses synthetic champion state and reports which
 * visible status outputs change after source-locked recompute. It does not load
 * game data, render bitmaps, or claim pixel parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_MAX_STEPS_PC34 8

#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATISTICS_PC34 0x0100u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_PANEL_PC34 0x0800u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_ACTION_HAND_PC34 0x8000u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATUS_BOX_PC34 0x1000u

#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_GFX_SLOT_NORMAL_PC34 33
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_GFX_SLOT_WOUNDED_PC34 34

#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_RED_PC34 8
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_YELLOW_PC34 11
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_FOOD_BASE_PC34 5
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_WATER_BASE_PC34 14

#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HP_BAR_PC34 0x00000001u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HP_VALUE_PC34 0x00000002u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STAMINA_BAR_PC34 0x00000004u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STAMINA_VALUE_PC34 0x00000008u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_MOUTH_BORDER_PC34 0x00000010u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_BAR_PC34 0x00000020u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_COLOR_PC34 0x00000040u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_BAR_PC34 0x00000080u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_COLOR_PC34 0x00000100u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_POISON_LABEL_PC34 0x00000200u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HAND_PANEL_PC34 0x00000400u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_ACTION_HAND_PC34 0x00000800u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STATUS_BOX_PC34 0x00001000u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_NAME_TITLE_PC34 0x00002000u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_LOAD_PC34 0x00004000u
#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WOUNDS_PC34 0x00008000u

#define DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_UNRELATED_VISUALS_PC34 \
    (DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STATUS_BOX_PC34 | \
     DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_NAME_TITLE_PC34 | \
     DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_LOAD_PC34 | \
     DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WOUNDS_PC34)

typedef enum {
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_FOOD_WATER_POISONED_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_OBJECT_PC34 = 1
} dm1_v1_champion_panel_status_recompute_pc34_compat_panel_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_HAND_EMPTY_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_HAND_OBJECT_PC34 = 1
} dm1_v1_champion_panel_status_recompute_pc34_compat_hand_t;

typedef enum {
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HEALTH_PC34 = 0,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_STAMINA_PC34 = 1,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_FOOD_PC34 = 2,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_WATER_PC34 = 3,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_POISON_PC34 = 4,
    DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HAND_PC34 = 5
} dm1_v1_champion_panel_status_recompute_pc34_compat_change_t;

typedef struct {
    int champion_index;
    int current_health;
    int maximum_health;
    int current_stamina;
    int maximum_stamina;
    int food;
    int water;
    int poison_event_count;
    dm1_v1_champion_panel_status_recompute_pc34_compat_hand_t action_hand;
    dm1_v1_champion_panel_status_recompute_pc34_compat_panel_t panel_content;
} dm1_v1_champion_panel_status_recompute_pc34_compat_state_t;

typedef struct {
    dm1_v1_champion_panel_status_recompute_pc34_compat_change_t change;
    int value;
} dm1_v1_champion_panel_status_recompute_pc34_compat_step_t;

typedef struct {
    dm1_v1_champion_panel_status_recompute_pc34_compat_state_t initial_state;
    dm1_v1_champion_panel_status_recompute_pc34_compat_step_t
        steps[DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_MAX_STEPS_PC34];
    int step_count;
} dm1_v1_champion_panel_status_recompute_pc34_compat_input_t;

typedef struct {
    int hp_bar_height;
    int hp_value;
    int stamina_bar_height;
    int stamina_value;
    int mouth_border_graphic;
    int food_bar_units;
    int food_color;
    int water_bar_units;
    int water_color;
    bool poison_label_visible;
    dm1_v1_champion_panel_status_recompute_pc34_compat_panel_t panel_content;
    dm1_v1_champion_panel_status_recompute_pc34_compat_hand_t action_hand;
} dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t;

typedef struct {
    uint16_t dirty_attributes;
    uint32_t changed_visuals;
    dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t before;
    dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t after;
    bool draw_state_called;
    bool statistics_recompute_requested;
    bool panel_recompute_requested;
    bool action_hand_recompute_requested;
    bool status_box_recompute_requested;
    bool unrelated_visuals_changed;
} dm1_v1_champion_panel_status_recompute_pc34_compat_step_result_t;

typedef struct {
    const char *status_recompute_anchor;
    const char *food_water_bar_anchor;
    const char *food_water_panel_anchor;
    const char *hand_dispatch_anchor;
    const char *mouth_command_anchor;
    const char *draw_state_anchor;
    const char *contract_scope;
    const char *no_real_asset_claim;
} dm1_v1_champion_panel_status_recompute_pc34_compat_evidence_t;

typedef struct {
    bool contract_only;
    bool loads_graphics_dat;
    bool loads_dungeon_dat;
    bool uses_synthetic_champion_state;
    bool covers_status_recompute_only;
    bool calls_draw_state_after_dirty_flags;
    bool statistics_dirty_recomputes_status_bars_and_mouth_border;
    bool panel_dirty_recomputes_food_water_poison_panel;
    bool action_hand_dirty_recomputes_action_slot_only;
    bool does_not_dirty_status_box;
    int max_steps;
} dm1_v1_champion_panel_status_recompute_pc34_compat_invariant_t;

typedef struct {
    dm1_v1_champion_panel_status_recompute_pc34_compat_invariant_t invariant;
    dm1_v1_champion_panel_status_recompute_pc34_compat_evidence_t evidence;
    dm1_v1_champion_panel_status_recompute_pc34_compat_state_t final_state;
    dm1_v1_champion_panel_status_recompute_pc34_compat_step_result_t
        steps[DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_MAX_STEPS_PC34];
    int step_count;
    uint32_t determinism_hash;
    bool null_input_defaults_used;
    bool rejected_overlarge_step_count;
} dm1_v1_champion_panel_status_recompute_pc34_compat_result_t;

dm1_v1_champion_panel_status_recompute_pc34_compat_result_t
dm1_v1_champion_panel_status_recompute_pc34_compat_run(
    const dm1_v1_champion_panel_status_recompute_pc34_compat_input_t *input);

const dm1_v1_champion_panel_status_recompute_pc34_compat_evidence_t *
dm1_v1_champion_panel_status_recompute_pc34_compat_evidence(void);

const char *
dm1_v1_champion_panel_status_recompute_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PC34_COMPAT_H */
