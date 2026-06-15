#include "dm1_v1_champion_panel_status_recompute_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.
 *
 * PANEL.C F0349:1945-1949 is the mouth-command status recompute handoff:
 * the champion is marked STATISTICS, marked PANEL when the food/water panel is
 * visible, and then passed to F0292. PANEL.C F0345:1597-1615 and
 * F0344:1519-1525 define the food/water/poison visible result. PANEL.C
 * F0347:1658-1691 defines the action-hand panel dispatch.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. PANEL.C F0344_INVENTORY_DrawPanel_FoodOrWaterBar:"
    "1493-1561 maps food/water amounts to a proportional bar and selects red "
    "for amount < -512, yellow for amount < 0, else the supplied base color. "
    "PANEL.C F0345_INVENTORY_DrawPanel_FoodWaterPoisoned:1563-1616 blits the "
    "empty panel, food/water labels, optional poisoned label, and food/water "
    "bars. PANEL.C F0347_INVENTORY_DrawPanel:1639-1691 dispatches action-hand "
    "content to food/water/poisoned when empty or to object rendering when the "
    "hand contains panel content. PANEL.C F0349:1832-1949 mutates water, food, "
    "stamina, health, marks STATISTICS, conditionally marks PANEL, then calls "
    "F0292_CHAMPION_DrawState. CHAMDRAW.C F0292:898-935 recomputes bars, "
    "inventory values, and the mouth warning border from food/water/poison; "
    "CHAMDRAW.C F0292:1060-1078 dispatches PANEL to F0345/F0347 and "
    "CHAMDRAW.C F0292:1080-1091 redraws only action hand/action icon for "
    "ACTION_HAND.";

static const dm1_v1_champion_panel_status_recompute_pc34_compat_evidence_t
    s_evidence = {
        "PANEL.C F0349:1945-1949 and CHAMDRAW.C F0292:898-935",
        "PANEL.C F0344:1493-1561",
        "PANEL.C F0345:1563-1616",
        "PANEL.C F0347:1639-1691",
        "PANEL.C F0349:1832-1949",
        "CHAMDRAW.C F0292:898-935,1060-1091",
        "contract-only status recompute visible-delta gate",
        "no real-asset bitmap parity; no GRAPHICS.DAT/DUNGEON.DAT load"
    };

static const dm1_v1_champion_panel_status_recompute_pc34_compat_invariant_t
    s_invariant = {
        true,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_MAX_STEPS_PC34
    };

static int ceil_bar_height(int current, int maximum)
{
    long scaled;

    if (current <= 0 || maximum <= 0) {
        return 0;
    }
    scaled = (((long)current << 10) * 25L) / (long)maximum;
    return (scaled & 0x3FFL) ? (int)(scaled >> 10) + 1 : (int)(scaled >> 10);
}

static int food_or_water_color(int amount, int base_color)
{
    /*
     * PANEL.C F0344:1519-1525: red below -512, yellow below zero, otherwise
     * the caller-provided food or water base color.
     */
    if (amount < -512) {
        return DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_RED_PC34;
    }
    if (amount < 0) {
        return DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_YELLOW_PC34;
    }
    return base_color;
}

static int food_or_water_bar_units(int amount)
{
    int normalized;

    /*
     * PANEL.C F0344:1537-1544: PC34 normalizes by adding 1024 and scaling
     * against a 3072-point food/water range before filling the proportional
     * zone. This model keeps the same deterministic 0..10000 unit scale.
     */
    normalized = amount + 1024;
    if (normalized < 0) {
        normalized = 0;
    }
    if (normalized > 3072) {
        normalized = 3072;
    }
    if (normalized == 3072) {
        normalized = 3071;
    }
    return (int)(((long)normalized * 10000L) / 3072L);
}

static dm1_v1_champion_panel_status_recompute_pc34_compat_panel_t
panel_from_hand(
    dm1_v1_champion_panel_status_recompute_pc34_compat_hand_t action_hand)
{
    /*
     * PANEL.C F0347:1658-1691: the action hand selects object rendering when
     * it contains panel content; otherwise F0345 redraws food/water/poisoned.
     */
    return action_hand ==
                   DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_HAND_OBJECT_PC34
               ? DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_OBJECT_PC34
               : DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_FOOD_WATER_POISONED_PC34;
}

static dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t
visible_from_state(
    const dm1_v1_champion_panel_status_recompute_pc34_compat_state_t *state)
{
    dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t visible;
    bool food_water_panel;

    memset(&visible, 0, sizeof(visible));
    visible.hp_bar_height =
        ceil_bar_height(state->current_health, state->maximum_health);
    visible.hp_value = state->current_health;
    visible.stamina_bar_height =
        ceil_bar_height(state->current_stamina, state->maximum_stamina);
    visible.stamina_value = state->current_stamina / 10;
    visible.mouth_border_graphic =
        (state->food < 0 || state->water < 0 || state->poison_event_count)
            ? DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_GFX_SLOT_WOUNDED_PC34
            : DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_GFX_SLOT_NORMAL_PC34;
    visible.action_hand = state->action_hand;
    visible.panel_content = panel_from_hand(state->action_hand);

    food_water_panel =
        visible.panel_content ==
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_FOOD_WATER_POISONED_PC34;
    visible.food_bar_units = food_water_panel
        ? food_or_water_bar_units(state->food)
        : -1;
    visible.food_color = food_water_panel
        ? food_or_water_color(
              state->food,
              DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_FOOD_BASE_PC34)
        : -1;
    visible.water_bar_units = food_water_panel
        ? food_or_water_bar_units(state->water)
        : -1;
    visible.water_color = food_water_panel
        ? food_or_water_color(
              state->water,
              DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_WATER_BASE_PC34)
        : -1;
    visible.poison_label_visible = food_water_panel &&
                                   state->poison_event_count > 0;

    return visible;
}

static uint32_t changed_visuals(
    const dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t *before,
    const dm1_v1_champion_panel_status_recompute_pc34_compat_visible_t *after)
{
    uint32_t changed = 0;

    if (before->hp_bar_height != after->hp_bar_height) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HP_BAR_PC34;
    }
    if (before->hp_value != after->hp_value) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HP_VALUE_PC34;
    }
    if (before->stamina_bar_height != after->stamina_bar_height) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STAMINA_BAR_PC34;
    }
    if (before->stamina_value != after->stamina_value) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STAMINA_VALUE_PC34;
    }
    if (before->mouth_border_graphic != after->mouth_border_graphic) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_MOUTH_BORDER_PC34;
    }
    if (before->food_bar_units != after->food_bar_units) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_BAR_PC34;
    }
    if (before->food_color != after->food_color) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_COLOR_PC34;
    }
    if (before->water_bar_units != after->water_bar_units) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_BAR_PC34;
    }
    if (before->water_color != after->water_color) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_COLOR_PC34;
    }
    if (before->poison_label_visible != after->poison_label_visible) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_POISON_LABEL_PC34;
    }
    if (before->panel_content != after->panel_content) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HAND_PANEL_PC34;
    }
    if (before->action_hand != after->action_hand) {
        changed |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_ACTION_HAND_PC34;
    }

    return changed;
}

static void hash_mix(uint32_t *hash, uint32_t value)
{
    *hash ^= value;
    *hash *= 16777619u;
}

static void apply_step(
    dm1_v1_champion_panel_status_recompute_pc34_compat_state_t *state,
    const dm1_v1_champion_panel_status_recompute_pc34_compat_step_t *step)
{
    switch (step->change) {
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HEALTH_PC34:
        state->current_health = step->value;
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_STAMINA_PC34:
        state->current_stamina = step->value;
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_FOOD_PC34:
        state->food = step->value;
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_WATER_PC34:
        state->water = step->value;
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_POISON_PC34:
        state->poison_event_count = step->value;
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HAND_PC34:
        state->action_hand = step->value
            ? DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_HAND_OBJECT_PC34
            : DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_HAND_EMPTY_PC34;
        state->panel_content = panel_from_hand(state->action_hand);
        break;
    }
}

static uint16_t dirty_attributes_for_step(
    const dm1_v1_champion_panel_status_recompute_pc34_compat_state_t *before,
    const dm1_v1_champion_panel_status_recompute_pc34_compat_step_t *step)
{
    uint16_t dirty = 0;

    switch (step->change) {
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HEALTH_PC34:
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_STAMINA_PC34:
        dirty |=
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATISTICS_PC34;
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_FOOD_PC34:
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_WATER_PC34:
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_POISON_PC34:
        /*
         * PANEL.C F0349:1945-1949 and CHAMPION.C F0322:1950-1962 set
         * STATISTICS, conditionally add PANEL while the food/water panel is
         * visible, and then call F0292 for the changed champion.
         */
        dirty |=
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATISTICS_PC34;
        if (before->panel_content ==
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_FOOD_WATER_POISONED_PC34) {
            dirty |= DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_PANEL_PC34;
        }
        break;
    case DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HAND_PC34:
        dirty |=
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_PANEL_PC34 |
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_ACTION_HAND_PC34;
        break;
    }

    return dirty;
}

dm1_v1_champion_panel_status_recompute_pc34_compat_result_t
dm1_v1_champion_panel_status_recompute_pc34_compat_run(
    const dm1_v1_champion_panel_status_recompute_pc34_compat_input_t *input)
{
    dm1_v1_champion_panel_status_recompute_pc34_compat_result_t result;
    dm1_v1_champion_panel_status_recompute_pc34_compat_input_t local_input;
    dm1_v1_champion_panel_status_recompute_pc34_compat_state_t state;
    int i;

    memset(&result, 0, sizeof(result));
    result.invariant = s_invariant;
    result.evidence = s_evidence;
    result.determinism_hash = 2166136261u;

    if (!input) {
        memset(&local_input, 0, sizeof(local_input));
        local_input.initial_state.maximum_health = 100;
        local_input.initial_state.current_health = 100;
        local_input.initial_state.maximum_stamina = 1000;
        local_input.initial_state.current_stamina = 1000;
        local_input.initial_state.panel_content =
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_FOOD_WATER_POISONED_PC34;
        input = &local_input;
        result.null_input_defaults_used = true;
    }

    result.step_count = input->step_count;
    if (result.step_count >
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_MAX_STEPS_PC34) {
        result.step_count =
            DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_MAX_STEPS_PC34;
        result.rejected_overlarge_step_count = true;
    }

    state = input->initial_state;
    state.panel_content = panel_from_hand(state.action_hand);

    for (i = 0; i < result.step_count; ++i) {
        dm1_v1_champion_panel_status_recompute_pc34_compat_step_result_t *step_result =
            &result.steps[i];
        dm1_v1_champion_panel_status_recompute_pc34_compat_state_t before_state =
            state;

        step_result->before = visible_from_state(&state);
        step_result->dirty_attributes =
            dirty_attributes_for_step(&before_state, &input->steps[i]);
        apply_step(&state, &input->steps[i]);
        step_result->after = visible_from_state(&state);
        step_result->changed_visuals =
            changed_visuals(&step_result->before, &step_result->after);
        step_result->draw_state_called = true;
        step_result->statistics_recompute_requested =
            (step_result->dirty_attributes &
             DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATISTICS_PC34) != 0;
        step_result->panel_recompute_requested =
            (step_result->dirty_attributes &
             DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_PANEL_PC34) != 0;
        step_result->action_hand_recompute_requested =
            (step_result->dirty_attributes &
             DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_ACTION_HAND_PC34) != 0;
        step_result->status_box_recompute_requested =
            (step_result->dirty_attributes &
             DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATUS_BOX_PC34) != 0;
        step_result->unrelated_visuals_changed =
            (step_result->changed_visuals &
             DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_UNRELATED_VISUALS_PC34) != 0;

        hash_mix(&result.determinism_hash, (uint32_t)step_result->dirty_attributes);
        hash_mix(&result.determinism_hash, step_result->changed_visuals);
        hash_mix(&result.determinism_hash,
                 (uint32_t)step_result->after.hp_bar_height);
        hash_mix(&result.determinism_hash,
                 (uint32_t)step_result->after.stamina_bar_height);
        hash_mix(&result.determinism_hash,
                 (uint32_t)step_result->after.food_bar_units);
        hash_mix(&result.determinism_hash,
                 (uint32_t)step_result->after.water_bar_units);
        hash_mix(&result.determinism_hash,
                 step_result->after.poison_label_visible ? 1u : 0u);
    }

    result.final_state = state;
    return result;
}

const dm1_v1_champion_panel_status_recompute_pc34_compat_evidence_t *
dm1_v1_champion_panel_status_recompute_pc34_compat_evidence(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_status_recompute_pc34_compat_source_evidence(void)
{
    return s_source_evidence;
}
