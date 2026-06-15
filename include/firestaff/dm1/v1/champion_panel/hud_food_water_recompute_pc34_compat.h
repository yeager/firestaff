#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_HUD_FOOD_WATER_RECOMPUTE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_HUD_FOOD_WATER_RECOMPUTE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only source lock for the DM1/PC 3.4 champion-panel clock path:
 * - CHAMPION.C F0331:2331-2332 computes the three-bit time criterion.
 * - CHAMPION.C F0331:2450-2473 uses a 256 active / 64 resting statistic
 *   recovery cadence and clamps current values above maximum downward.
 * - CHAMPION.C F0331:2482-2496 dirties STATISTICS, optionally PANEL for the
 *   open food/water panel, then redraws all champion states through F0293.
 * - CHAMDRAW.C F0292_CHAMPION_DrawState:771-815 owns the C151..C154 67x29
 *   status-box repaint cascade, then F0292:898-935 redraws bars and the
 *   food/water/poison mouth and eye warnings.
 * - CHAMDRAW.C F0292_CHAMPION_DrawState:1060-1091 keeps the panel,
 *   action-hand, and action-icon partial redraw path ordered after bars.
 * - PANEL.C F0355:2299-2322 closes inventory by calling F0334, dirtying
 *   STATUS_BOX, and immediately calling F0292 for the old inventory champion.
 * - DEFS.H:780-817,873-876,1878,3778-3786,5700,5876-5881 provide the
 *   slot, champion, hand-slot, status-zone, G0305/G0423/G0425/G0426 symbols.
 */

#define FS_DM1_V1_HFW_CHAMPION_COUNT_PC34 4
#define FS_DM1_V1_HFW_STATUS_ZONE_FIRST_PC34 151
#define FS_DM1_V1_HFW_STATUS_ZONE_LAST_PC34 154
#define FS_DM1_V1_HFW_STATUS_BOX_WIDTH_PC34 67
#define FS_DM1_V1_HFW_STATUS_BOX_HEIGHT_PC34 29
#define FS_DM1_V1_HFW_STATUS_BOX_STRIDE_PC34 69
#define FS_DM1_V1_HFW_HAND_SLOT_WIDTH_PC34 18
#define FS_DM1_V1_HFW_HAND_SLOT_HEIGHT_PC34 18
#define FS_DM1_V1_HFW_READY_HAND_X_OFFSET_PC34 4
#define FS_DM1_V1_HFW_ACTION_HAND_X_OFFSET_PC34 24
#define FS_DM1_V1_HFW_HAND_SLOT_Y_PC34 10

#define FS_DM1_V1_HFW_ATTR_STATISTICS_PC34 0x0100u
#define FS_DM1_V1_HFW_ATTR_PANEL_PC34 0x0800u
#define FS_DM1_V1_HFW_ATTR_STATUS_BOX_PC34 0x1000u
#define FS_DM1_V1_HFW_ATTR_VIEWPORT_PC34 0x4000u
#define FS_DM1_V1_HFW_ATTR_ACTION_HAND_PC34 0x8000u

#define FS_DM1_V1_HFW_PANEL_FOOD_WATER_POISONED_PC34 565
#define FS_DM1_V1_HFW_SLOT_READY_HAND_PC34 0
#define FS_DM1_V1_HFW_SLOT_ACTION_HAND_PC34 1
#define FS_DM1_V1_HFW_SLOT_CHEST_1_PC34 30
#define FS_DM1_V1_HFW_SLOT_CHEST_8_PC34 37

#define FS_DM1_V1_HFW_ORDER_BARS_PC34 1
#define FS_DM1_V1_HFW_ORDER_ACTION_ICON_PC34 2
#define FS_DM1_V1_HFW_ORDER_ACTION_HAND_PC34 4
#define FS_DM1_V1_HFW_ORDER_FULL_TUPLE_PC34 7

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
    int width;
    int height;
    int zone;
} fs_dm1_v1_hfw_box_pc34_t;

typedef struct {
    uint32_t game_time;
    int resting;
    int alive;
    int candidate_ordinal;
    int champion_index;
    int inventory_champion_ordinal;
    int panel_content;
    int pressing_mouth;
    int pressing_eye;
} fs_dm1_v1_hfw_clock_input_pc34_t;

typedef struct {
    int valid;
    int time_criteria;
    int recovery_period;
    int recovery_due;
    uint16_t f0331_dirty_attributes;
    uint16_t f0355_close_dirty_attributes;
    int draw_all_champion_states;
    int status_box_zone;
    int status_box_repainted_by_close_hook;
    int statistics_repainted_by_clock;
    int panel_repainted_for_food_water;
    int mouth_warning_recomputed;
    int eye_warning_recomputed;
    int tuple_order_mask;
    fs_dm1_v1_hfw_box_pc34_t status_box;
    fs_dm1_v1_hfw_box_pc34_t ready_hand_box;
    fs_dm1_v1_hfw_box_pc34_t action_hand_box;
} fs_dm1_v1_hfw_clock_result_pc34_t;

static inline int fs_dm1_v1_hfw_clock_time_criteria_pc34(uint32_t game_time)
{
    return (int)((((game_time & 0x0080u) +
                   ((game_time & 0x0100u) >> 2)) +
                  ((game_time & 0x0040u) << 2)) >> 2);
}

static inline int fs_dm1_v1_hfw_recovery_period_pc34(int resting)
{
    return resting ? 64 : 256;
}

static inline int fs_dm1_v1_hfw_recovery_due_pc34(uint32_t game_time,
                                                  int resting)
{
    return ((game_time & (uint32_t)(resting ? 63 : 255)) == 0u) ? 1 : 0;
}

static inline int fs_dm1_v1_hfw_recover_stat_pc34(int current, int maximum)
{
    if (current < maximum) {
        return current + 1;
    }
    if (current > maximum && maximum > 0) {
        return current - (current / maximum);
    }
    return current;
}

static inline int fs_dm1_v1_hfw_status_zone_pc34(int champion_index)
{
    return FS_DM1_V1_HFW_STATUS_ZONE_FIRST_PC34 + champion_index;
}

static inline fs_dm1_v1_hfw_box_pc34_t
fs_dm1_v1_hfw_status_box_pc34(int champion_index)
{
    fs_dm1_v1_hfw_box_pc34_t box;
    box.left = champion_index * FS_DM1_V1_HFW_STATUS_BOX_STRIDE_PC34;
    box.top = 0;
    box.right = box.left + FS_DM1_V1_HFW_STATUS_BOX_WIDTH_PC34 - 1;
    box.bottom = FS_DM1_V1_HFW_STATUS_BOX_HEIGHT_PC34 - 1;
    box.width = FS_DM1_V1_HFW_STATUS_BOX_WIDTH_PC34;
    box.height = FS_DM1_V1_HFW_STATUS_BOX_HEIGHT_PC34;
    box.zone = fs_dm1_v1_hfw_status_zone_pc34(champion_index);
    return box;
}

static inline fs_dm1_v1_hfw_box_pc34_t
fs_dm1_v1_hfw_hand_slot_box_pc34(int champion_index, int hand_index)
{
    fs_dm1_v1_hfw_box_pc34_t box;
    const int offset = hand_index == FS_DM1_V1_HFW_SLOT_ACTION_HAND_PC34 ?
        FS_DM1_V1_HFW_ACTION_HAND_X_OFFSET_PC34 :
        FS_DM1_V1_HFW_READY_HAND_X_OFFSET_PC34;

    box.left = champion_index * FS_DM1_V1_HFW_STATUS_BOX_STRIDE_PC34 + offset;
    box.top = FS_DM1_V1_HFW_HAND_SLOT_Y_PC34;
    box.right = box.left + FS_DM1_V1_HFW_HAND_SLOT_WIDTH_PC34 - 1;
    box.bottom = box.top + FS_DM1_V1_HFW_HAND_SLOT_HEIGHT_PC34 - 1;
    box.width = FS_DM1_V1_HFW_HAND_SLOT_WIDTH_PC34;
    box.height = FS_DM1_V1_HFW_HAND_SLOT_HEIGHT_PC34;
    box.zone = hand_index;
    return box;
}

static inline fs_dm1_v1_hfw_clock_result_pc34_t
fs_dm1_v1_hfw_clock_probe_pc34(const fs_dm1_v1_hfw_clock_input_pc34_t *input)
{
    fs_dm1_v1_hfw_clock_result_pc34_t result;
    const int ordinal = input ? input->champion_index + 1 : 0;
    const int inventory_match =
        input && ordinal == input->inventory_champion_ordinal;
    const int food_water_panel =
        input && input->panel_content ==
            FS_DM1_V1_HFW_PANEL_FOOD_WATER_POISONED_PC34;

    result.valid = input &&
        input->champion_index >= 0 &&
        input->champion_index < FS_DM1_V1_HFW_CHAMPION_COUNT_PC34 &&
        input->alive &&
        ordinal != input->candidate_ordinal;
    result.time_criteria = input ?
        fs_dm1_v1_hfw_clock_time_criteria_pc34(input->game_time) : 0;
    result.recovery_period = input ?
        fs_dm1_v1_hfw_recovery_period_pc34(input->resting) : 0;
    result.recovery_due = input ?
        fs_dm1_v1_hfw_recovery_due_pc34(input->game_time, input->resting) : 0;
    result.f0331_dirty_attributes = 0;
    result.f0355_close_dirty_attributes = 0;
    result.draw_all_champion_states = 0;
    result.status_box_zone = input ?
        fs_dm1_v1_hfw_status_zone_pc34(input->champion_index) : 0;
    result.status_box_repainted_by_close_hook = 0;
    result.statistics_repainted_by_clock = 0;
    result.panel_repainted_for_food_water = 0;
    result.mouth_warning_recomputed = 0;
    result.eye_warning_recomputed = 0;
    result.tuple_order_mask = 0;
    result.status_box = input ?
        fs_dm1_v1_hfw_status_box_pc34(input->champion_index) :
        fs_dm1_v1_hfw_status_box_pc34(0);
    result.ready_hand_box = input ?
        fs_dm1_v1_hfw_hand_slot_box_pc34(input->champion_index, 0) :
        fs_dm1_v1_hfw_hand_slot_box_pc34(0, 0);
    result.action_hand_box = input ?
        fs_dm1_v1_hfw_hand_slot_box_pc34(input->champion_index, 1) :
        fs_dm1_v1_hfw_hand_slot_box_pc34(0, 1);

    if (!result.valid) {
        return result;
    }

    result.f0331_dirty_attributes = FS_DM1_V1_HFW_ATTR_STATISTICS_PC34;
    result.statistics_repainted_by_clock = 1;
    result.draw_all_champion_states = 1;
    result.mouth_warning_recomputed = inventory_match ? 1 : 0;
    result.eye_warning_recomputed = inventory_match ? 1 : 0;
    if (inventory_match &&
        (input->pressing_mouth || input->pressing_eye || food_water_panel)) {
        result.f0331_dirty_attributes |= FS_DM1_V1_HFW_ATTR_PANEL_PC34;
        result.panel_repainted_for_food_water = food_water_panel ? 1 : 0;
    }

    /*
     * PANEL.C F0355:2316-2322 is a separate close hook: it clears G0423,
     * calls F0334, marks MASK0x1000_STATUS_BOX, then calls F0292. This
     * helper records the hook without claiming F0331 itself dirties it.
     */
    if (inventory_match) {
        result.f0355_close_dirty_attributes =
            FS_DM1_V1_HFW_ATTR_STATUS_BOX_PC34;
        result.status_box_repainted_by_close_hook = 1;
    }

    result.tuple_order_mask =
        FS_DM1_V1_HFW_ORDER_BARS_PC34 |
        FS_DM1_V1_HFW_ORDER_ACTION_ICON_PC34 |
        FS_DM1_V1_HFW_ORDER_ACTION_HAND_PC34;
    return result;
}

static inline const char *
fs_dm1_v1_hfw_source_evidence_pc34(void)
{
    return "CHAMPION.C F0331:2331-2332 time criterion; "
           "CHAMPION.C F0331:2450-2473 rest-modulated stat recovery; "
           "CHAMPION.C F0331:2482-2496 STATISTICS/PANEL/F0293 clock route; "
           "CHAMDRAW.C F0292:771-815 C151..C154 67x29 status-box cascade; "
           "CHAMDRAW.C F0292:898-935 bars and food/water/poison warnings; "
           "CHAMDRAW.C F0292:1060-1091 panel/action-hand/action-icon tuple; "
           "PANEL.C F0355:2299-2322 close hook calls F0334, STATUS_BOX, F0292; "
           "PANEL.C F0345:1579-1615 reads G0423/M516 and draws food/water; "
           "DEFS.H:780-817 C30 chest slots, 873-876 M516, 1878 M070, "
           "3778-3786 C104/C151-C154, 5700 G0305, 5876-5881 G0423/G0425/G0426.";
}

#ifdef __cplusplus
}
#endif

#endif
