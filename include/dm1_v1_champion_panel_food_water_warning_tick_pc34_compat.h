#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_FOOD_WATER_WARNING_TICK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_FOOD_WATER_WARNING_TICK_PC34_COMPAT_H

/*
 * DM1 V1 food/water threshold warning on a single game tick.
 *
 * Source-lock anchors (ReDMCSB):
 * - CHAMPION.C F0331:2360..2415 (F0331_CHAMPION_ApplyTimeEffects_CPSF) owns
 *   the inner do-while that decrements Champion->Food / Champion->Water
 *   every game tick and clamps both at -1024 (LIFECYCLE_FOOD_FLOOR /
 *   LIFECYCLE_WATER_FLOOR). Each cycle subtracts 2 from Food and 1 from
 *   Water when stamina is above half, or staminaGainCycleCount>>1 /
 *   staminaGainCycleCount>>2 when stamina is below half.
 * - PANEL.C F0344:1519..1525 (F0344_INVENTORY_DrawPanel_FoodOrWaterBar) maps
 *   the per-champion amount to one of three colours:
 *     amount < -512         -> C08_COLOR_RED   (severe warning)
 *     -512 <= amount < 0    -> C11_COLOR_YELLOW (mid warning)
 *     amount >= 0           -> base colour (C05_COLOR_LIGHT_BROWN for food,
 *                                            C14_COLOR_BLUE for water).
 * - CHAMDRAW.C F0292:1060..1062 routes the food/water/poisoned panel redraw
 *   through F0345 only when (MASK0x0800_PANEL && L0863_B_IsInventoryChampion).
 *   The gate below fires MASK0x0800_PANEL exactly on the tick where the
 *   champion's food/water crosses a warning threshold (normal->yellow,
 *   yellow->red), matching F0292's per-tick redraw-on-warning contract.
 * - DEFS.H C08/C11/C05/C14 supply the colour constants. DEFS.H:726/728/732
 *   supply MASK0x0200_LOAD / MASK0x0800_PANEL / MASK0x1000_STATUS_BOX. The
 *   F0292:1060 panel-sync path also keeps MASK0x1000_STATUS_BOX set so the
 *   chest-close -> status-box -> food/water cascade fires on the same tick.
 * - DEFS.H C500/C501 (food/water label zones) + C103/C104 (food/water bar
 *   zones) are the F0345:1598..1615 draw targets that consume the colour
 *   selection produced by F0344:1519..1525.
 *
 * Contract-only slice: this module consumes the existing Phase 18 lifecycle
 * tick (F0832/F0833) and projects the F0344 warning colour + the F0292
 * panel-sync mask transitions for ONE champion across a bounded sequence
 * of ticks. It does not load GRAPHICS.DAT or DUNGEON.DAT, does not run
 * M11 graphics, does not claim original parity.
 *
 * Disjoint scope (must not duplicate):
 * - test_dm1_v1_champion_panel_food_water_status_box_pc34_compat owns the
 *   chest-close -> 67x29 status-box -> food/water draw order (C151..C154).
 *   This gate owns the single-tick threshold crossing across many ticks,
 *   not the one-shot close->draw contract.
 * - test_dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat owns
 *   the F0331 clock-driven recompute + F0355 close hook + status-box
 *   cascade. This gate is the warning colour + panel-sync mask projection
 *   for those recomputes; it does not own the recompute delta or close hook.
 * - test_dm1_v1_champion_panel_leader_swap_food_water_pc34_compat owns the
 *   F0302 inventory-champion leader-swap dispatch + BUG0_39 flicker; not
 *   the per-tick warning transition.
 * - test_dm1_v1_chm05_f0832_hunger_thirst_loop_guard_pc34_compat owns the
 *   F0832 64-iteration loop-guard and the stamina loss formula; this gate
 *   consumes the F0833 tick output but does not retest the loop-guard.
 * - test_dm1_v1_champion_panel_mouth_eye_poison_warning_pc34_compat owns
 *   the mouth/eye warning border / palette flash; this gate owns the food/
 *   water bar colour and the panel-sync mask, not the border flashing.
 */

#include <stdbool.h>
#include <stdint.h>

#include "memory_champion_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Match DEFS.H:728/732 panel-redraw masks used by F0292:1060..1062. */
#define DM1_V1_CPFWWT_MASK_PANEL_PC34       0x0800u
#define DM1_V1_CPFWWT_MASK_STATUS_BOX_PC34  0x1000u

/* Match DEFS.H:2076..2092,2157 colour constants used by F0344:1519..1525. */
#define DM1_V1_CPFWWT_COLOR_RED_PC34        8
#define DM1_V1_CPFWWT_COLOR_YELLOW_PC34     11
#define DM1_V1_CPFWWT_COLOR_LIGHT_BROWN_PC34 5
#define DM1_V1_CPFWWT_COLOR_BLUE_PC34       14

/* Max champion count mirrored from DEFS.H CHAMPION_MAX_PARTY (= 4). */
#define DM1_V1_CPFWWT_CHAMPION_COUNT_PC34   4

/* Per-tick input contract. Matches F0833_HungerThirstInput_Compat one tick. */
typedef struct {
    int16_t food_before;
    int16_t water_before;
    int16_t current_stamina;
    int16_t max_stamina;
    uint8_t is_resting;
    uint8_t padding[3];
    uint32_t game_time;
    uint32_t last_movement_time;
    /* If non-zero, this tick clamps F0832's per-cycle subtraction to a
     * fixed delta so the gate can land the threshold crossing exactly on
     * a chosen tick. Otherwise F0833's natural decay wins. */
    uint8_t force_per_tick_food_delta;
    uint8_t force_per_tick_water_delta;
    uint8_t force_padding[2];
    int16_t forced_food_delta;
    int16_t forced_water_delta;
} dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t;

typedef enum {
    DM1_V1_CPFWWT_WARNING_KIND_NONE_PC34   = 0,
    DM1_V1_CPFWWT_WARNING_KIND_FOOD_PC34   = 1,
    DM1_V1_CPFWWT_WARNING_KIND_WATER_PC34  = 2,
    DM1_V1_CPFWWT_WARNING_KIND_BOTH_PC34   = 3
} dm1_v1_champion_panel_food_water_warning_tick_warning_kind_pc34_t;

typedef enum {
    DM1_V1_CPFWWT_BAND_NORMAL_PC34 = 0,
    DM1_V1_CPFWWT_BAND_YELLOW_PC34 = 1,
    DM1_V1_CPFWWT_BAND_RED_PC34    = 2
} dm1_v1_champion_panel_food_water_warning_tick_band_pc34_t;

typedef struct {
    int tick_index;
    int16_t food_after;
    int16_t water_after;
    int16_t food_delta;
    int16_t water_delta;
    int16_t net_stamina_change;
    int16_t health_damage;
    int food_band_before;
    int food_band_after;
    int water_band_before;
    int water_band_after;
    int food_bar_color;
    int water_bar_color;
    int food_warning_fired_this_tick;
    int water_warning_fired_this_tick;
    int panel_mask_set_this_tick;
    int status_box_mask_set_this_tick;
    int16_t panel_warning_kind_bits;
    int clamp_to_floor_fired;
} dm1_v1_champion_panel_food_water_warning_tick_step_pc34_t;

typedef struct {
    int contract_only;
    int loads_graphics_dat;
    int loads_dungeon_dat;
    int champion_index;
    int tick_count;
    int food_band_crossings;
    int water_band_crossings;
    int food_band_crossing_tick_indices[4];
    int water_band_crossing_tick_indices[4];
    int food_floor_clamp_tick;
    int water_floor_clamp_tick;
    int final_food_band;
    int final_water_band;
    int final_food_bar_color;
    int final_water_bar_color;
    int any_tick_set_panel_mask;
    int any_tick_set_status_box_mask;
    int panel_warning_kind_mask_union;
    int initial_panel_mask_clear;
    int final_panel_mask_cleared_after_terminal_band;
    int step_count;
    dm1_v1_champion_panel_food_water_warning_tick_step_pc34_t
        steps[64];
    const char *sourceEvidence;
} dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t;

typedef struct {
    int contract_only;
    int food_warn_threshold;
    int food_red_threshold;
    int water_warn_threshold;
    int water_red_threshold;
    int food_floor;
    int water_floor;
    int panel_mask_value;
    int status_box_mask_value;
    int food_band_normal_color;
    int food_band_yellow_color;
    int food_band_red_color;
    int water_band_normal_color;
    int water_band_yellow_color;
    int water_band_red_color;
    const char *threshold_anchor;
    const char *decay_anchor;
    const char *panel_sync_anchor;
    const char *colour_anchor;
    const char *defs_anchor;
} dm1_v1_champion_panel_food_water_warning_tick_contract_pc34_t;

const dm1_v1_champion_panel_food_water_warning_tick_contract_pc34_t *
dm1_v1_champion_panel_food_water_warning_tick_contract_pc34(void);

const char *
dm1_v1_champion_panel_food_water_warning_tick_source_evidence_pc34(void);

dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t
dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34(void);

/*
 * Run the gate for one champion across up to 64 ticks. The per-tick food/
 * water decay comes from Phase 18 F0833 (the in-tree ReDMCSB F0331 mirror);
 * the warning colour per tick is F0344:1519..1525; the panel-sync mask is
 * F0292:1060..1062. The result records every tick's colour band, warning
 * kind, mask transitions, and floor-clamp tick.
 */
dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t
dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
    const dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t *input,
    int champion_index,
    int tick_count);

#ifdef __cplusplus
}
#endif

#endif
