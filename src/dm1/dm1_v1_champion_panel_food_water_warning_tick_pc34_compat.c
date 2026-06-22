#include "dm1_v1_champion_panel_food_water_warning_tick_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_CPFWWT_WARN_THRESHOLD = 0,
    DM1_V1_CPFWWT_RED_THRESHOLD = -512,
    DM1_V1_CPFWWT_FLOOR = -1024,
    DM1_V1_CPFWWT_FOOD_NORMAL_DECAY = 2,
    DM1_V1_CPFWWT_WATER_NORMAL_DECAY = 1,
    DM1_V1_CPFWWT_MAX_STEPS = 64
};

static const char s_source_evidence[] =
    "contract_only=1; CHAMPION.C F0331:2360..2415 owns the inner do-while "
    "that decrements Champion->Food by 2 (above-half) or "
    "staminaGainCycleCount>>1 (below-half) per cycle and Champion->Water "
    "by 1 (above-half) or staminaGainCycleCount>>2 (below-half), with the "
    "F0832 hard 64-iteration loop-guard as the v1 safety bound. "
    "CHAMPION.C:2392 L1010_ps_Champion->Food -= 2 (above-half); "
    "CHAMPION.C:2398 Food -= staminaGainCycleCount >> 1 (below-half). "
    "CHAMPION.C:2403 L1010_ps_Champion->Water -= 1 (above-half); "
    "CHAMPION.C:2409 Water -= staminaGainCycleCount >> 2 (below-half). "
    "CHAMPION.C F0331:2413..2418 floor: Food = -1024 / Water = -1024. "
    "PANEL.C F0344:1519..1525 maps the per-champion amount to colour: "
    "amount<-512 -> C08_COLOR_RED, -512<=amount<0 -> C11_COLOR_YELLOW, "
    "amount>=0 -> base colour (C05_COLOR_LIGHT_BROWN food / C14_COLOR_BLUE "
    "water). CHAMDRAW.C F0292:1060..1062 fires MASK0x0800_PANEL only "
    "when (MASK0x0800_PANEL && L0863_B_IsInventoryChampion), so the "
    "warning tick reuses the same panel-sync mask as the leader-swap and "
    "F0345 close hook rows; the chest-close -> status-box cascade keeps "
    "MASK0x1000_STATUS_BOX set so F0292 redraws the 67x29 status box. "
    "DEFS.H C08/C11/C05/C14 supply the colour constants; DEFS.H:728/732 "
    "supply MASK0x0800_PANEL / MASK0x1000_STATUS_BOX; DEFS.H:3869..3872 "
    "supply C500/C501/C502/C503 food/water/poisoned/arrow-eye zones.";

static const dm1_v1_champion_panel_food_water_warning_tick_contract_pc34_t
    s_contract = {
        1,
        DM1_V1_CPFWWT_WARN_THRESHOLD,
        DM1_V1_CPFWWT_RED_THRESHOLD,
        DM1_V1_CPFWWT_WARN_THRESHOLD,
        DM1_V1_CPFWWT_RED_THRESHOLD,
        DM1_V1_CPFWWT_FLOOR,
        DM1_V1_CPFWWT_FLOOR,
        DM1_V1_CPFWWT_MASK_PANEL_PC34,
        DM1_V1_CPFWWT_MASK_STATUS_BOX_PC34,
        DM1_V1_CPFWWT_COLOR_LIGHT_BROWN_PC34,
        DM1_V1_CPFWWT_COLOR_YELLOW_PC34,
        DM1_V1_CPFWWT_COLOR_RED_PC34,
        DM1_V1_CPFWWT_COLOR_BLUE_PC34,
        DM1_V1_CPFWWT_COLOR_YELLOW_PC34,
        DM1_V1_CPFWWT_COLOR_RED_PC34,
        "PANEL.C F0344:1519..1525 (amount<-512 red, <0 yellow, >=0 base)",
        "CHAMPION.C F0331:2360..2415 (Food -=2 above-half, Water -=1 "
        "above-half, clamped at Food = -1024 / Water = -1024)",
        "CHAMDRAW.C F0292:1060..1062 (MASK0x0800_PANEL on warning tick)",
        "PANEL.C F0344:1519..1525 colour selection; DEFS.H C05/C08/C11/C14",
        "DEFS.H:728/732 MASK0x0800_PANEL / MASK0x1000_STATUS_BOX; "
        "DEFS.H:3869..3872 C500/C501/C502/C503 zones"
    };

const dm1_v1_champion_panel_food_water_warning_tick_contract_pc34_t *
dm1_v1_champion_panel_food_water_warning_tick_contract_pc34(void)
{
    return &s_contract;
}

const char *
dm1_v1_champion_panel_food_water_warning_tick_source_evidence_pc34(void)
{
    return s_source_evidence;
}

dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t
dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34(void)
{
    dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t input;

    memset(&input, 0, sizeof(input));
    /*
     * Default forced-delta path (test-only accelerator; not the natural
     * F0331 per-tick decay of food=2 / water=1 above-half) lands every
     * band crossing inside a 64-tick window so the gate is fully
     * deterministic:
     *   food:   start 6,  -32/tick ->  yellow tick 0,  red tick 16,
     *           floor  tick 32
     *   water:  start 2,  -17/tick ->  yellow tick 0,  red tick 30,
     *           floor  tick 60
     * The F0834 clamp at -1024 is still routed through the gate so
     * the floor clamp fires on the predicted tick.
     */
    input.food_before = 6;
    input.water_before = 2;
    input.current_stamina = 50;
    input.max_stamina = 100;
    input.is_resting = 0;
    input.game_time = 1024u;
    input.last_movement_time = 0u;
    input.force_per_tick_food_delta = 1;
    input.force_per_tick_water_delta = 1;
    input.forced_food_delta = 32;
    input.forced_water_delta = 17;
    return input;
}

static int dm1_v1_cpfwwt_valid_input(
    const dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t *input,
    int champion_index,
    int tick_count)
{
    return input != NULL &&
           champion_index >= 0 &&
           champion_index < DM1_V1_CPFWWT_CHAMPION_COUNT_PC34 &&
           tick_count > 0 &&
           tick_count <= DM1_V1_CPFWWT_MAX_STEPS;
}

static int dm1_v1_cpfwwt_band_for_amount(int16_t amount)
{
    if (amount < (int16_t)DM1_V1_CPFWWT_RED_THRESHOLD) {
        return DM1_V1_CPFWWT_BAND_RED_PC34;
    }
    if (amount < (int16_t)DM1_V1_CPFWWT_WARN_THRESHOLD) {
        return DM1_V1_CPFWWT_BAND_YELLOW_PC34;
    }
    return DM1_V1_CPFWWT_BAND_NORMAL_PC34;
}

static int dm1_v1_cpfwwt_color_for_band_food(int band)
{
    switch (band) {
    case DM1_V1_CPFWWT_BAND_RED_PC34:
        return DM1_V1_CPFWWT_COLOR_RED_PC34;
    case DM1_V1_CPFWWT_BAND_YELLOW_PC34:
        return DM1_V1_CPFWWT_COLOR_YELLOW_PC34;
    default:
        return DM1_V1_CPFWWT_COLOR_LIGHT_BROWN_PC34;
    }
}

static int dm1_v1_cpfwwt_color_for_band_water(int band)
{
    switch (band) {
    case DM1_V1_CPFWWT_BAND_RED_PC34:
        return DM1_V1_CPFWWT_COLOR_RED_PC34;
    case DM1_V1_CPFWWT_BAND_YELLOW_PC34:
        return DM1_V1_CPFWWT_COLOR_YELLOW_PC34;
    default:
        return DM1_V1_CPFWWT_COLOR_BLUE_PC34;
    }
}

dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t
dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
    const dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t *input,
    int champion_index,
    int tick_count)
{
    dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t result;
    struct ChampionLifecycleState_Compat champ;
    int tick_index;
    int16_t food_before;
    int16_t water_before;
    int food_band_prev;
    int water_band_prev;
    int food_crossings;
    int water_crossings;
    int food_floor_clamp_tick;
    int water_floor_clamp_tick;
    int any_panel_mask;
    int any_status_box_mask;
    int panel_warning_kind_mask_union;
    int final_panel_mask_cleared;

    memset(&result, 0, sizeof(result));
    result.contract_only = 1;
    result.loads_graphics_dat = 0;
    result.loads_dungeon_dat = 0;
    result.sourceEvidence = s_source_evidence;
    result.food_floor_clamp_tick = -1;
    result.water_floor_clamp_tick = -1;

    if (!dm1_v1_cpfwwt_valid_input(input, champion_index, tick_count)) {
        return result;
    }

    memset(&champ, 0, sizeof(champ));
    champ.food = input->food_before;
    champ.water = input->water_before;
    champ.maxStamina = (uint16_t)input->max_stamina;

    result.champion_index = champion_index;
    result.tick_count = tick_count;
    any_panel_mask = 0;
    any_status_box_mask = 0;
    panel_warning_kind_mask_union = 0;
    final_panel_mask_cleared = 0;
    food_crossings = 0;
    water_crossings = 0;
    food_floor_clamp_tick = -1;
    water_floor_clamp_tick = -1;

    food_before = champ.food;
    water_before = champ.water;
    food_band_prev = dm1_v1_cpfwwt_band_for_amount(food_before);
    water_band_prev = dm1_v1_cpfwwt_band_for_amount(water_before);

    for (tick_index = 0; tick_index < tick_count; ++tick_index) {
        struct HungerThirstInput_Compat tick_input;
        struct HungerThirstResult_Compat tick_output;
        int16_t food_after;
        int16_t water_after;
        int food_band_after;
        int water_band_after;
        int food_warning_fired;
        int water_warning_fired;
        int panel_mask_set;
        int status_box_mask_set;
        int16_t warning_bits;
        int clamp_fired;

        memset(&tick_input, 0, sizeof(tick_input));
        memset(&tick_output, 0, sizeof(tick_output));
        tick_input.currentStamina = input->current_stamina;
        tick_input.maxStamina = input->max_stamina;
        tick_input.isResting = input->is_resting;
        tick_input.gameTime = input->game_time + (uint32_t)tick_index;
        tick_input.lastMovementTime = input->last_movement_time;

        if (input->force_per_tick_food_delta ||
            input->force_per_tick_water_delta) {
            /* Deterministic delta override path: skip F0833's
             * stamina-cycles-driven decay and apply a per-tick delta
             * directly so the test can land the threshold crossings on
             * chosen ticks. The override still routes through the
             * F0834 clamp so the gate proves the floor and the
             * warning-colour projection. */
            if (input->force_per_tick_food_delta) {
                champ.food = (int16_t)(champ.food - input->forced_food_delta);
            }
            if (input->force_per_tick_water_delta) {
                champ.water =
                    (int16_t)(champ.water - input->forced_water_delta);
            }
            F0834_LIFECYCLE_ClampFoodWater_Compat(&champ.food, &champ.water);
            tick_output.netStaminaChange = 0;
            tick_output.healthDamage = 0;
            tick_output.newFood = champ.food;
            tick_output.newWater = champ.water;
        } else {
            F0833_LIFECYCLE_ApplyHungerThirstFull_Compat(&champ, &tick_input,
                                                         &tick_output);
            champ.food = tick_output.newFood;
            champ.water = tick_output.newWater;
        }

        food_after = champ.food;
        water_after = champ.water;

        /* Detect the floor clamp independently of the forced-delta path
         * so the test can assert which tick first hit -1024. */
        clamp_fired = 0;
        if (food_after == (int16_t)DM1_V1_CPFWWT_FLOOR &&
            food_before > (int16_t)DM1_V1_CPFWWT_FLOOR &&
            food_floor_clamp_tick < 0) {
            food_floor_clamp_tick = tick_index;
            clamp_fired = 1;
        }
        if (water_after == (int16_t)DM1_V1_CPFWWT_FLOOR &&
            water_before > (int16_t)DM1_V1_CPFWWT_FLOOR &&
            water_floor_clamp_tick < 0) {
            water_floor_clamp_tick = tick_index;
            clamp_fired = 1;
        }

        food_band_after = dm1_v1_cpfwwt_band_for_amount(food_after);
        water_band_after = dm1_v1_cpfwwt_band_for_amount(water_after);
        food_warning_fired = (food_band_after != food_band_prev);
        water_warning_fired = (water_band_after != water_band_prev);
        warning_bits = 0;
        if (food_warning_fired) {
            warning_bits |= (int16_t)DM1_V1_CPFWWT_WARNING_KIND_FOOD_PC34;
        }
        if (water_warning_fired) {
            warning_bits |= (int16_t)DM1_V1_CPFWWT_WARNING_KIND_WATER_PC34;
        }

        /* F0292:1060..1062 sets MASK0x0800_PANEL only on the warning tick
         * (the tick that flips a band) AND keeps MASK0x1000_STATUS_BOX set
         * so the chest-close -> status-box cascade fires in the same
         * M11 frame. */
        panel_mask_set = (warning_bits != 0) ? 1 : 0;
        status_box_mask_set = panel_mask_set;

        if (panel_mask_set) {
            any_panel_mask = 1;
        }
        if (status_box_mask_set) {
            any_status_box_mask = 1;
        }
        panel_warning_kind_mask_union |= warning_bits;

        if (food_band_after != food_band_prev &&
            food_crossings < 4) {
            result.food_band_crossing_tick_indices[food_crossings] =
                tick_index;
            food_crossings++;
        }
        if (water_band_after != water_band_prev &&
            water_crossings < 4) {
            result.water_band_crossing_tick_indices[water_crossings] =
                tick_index;
            water_crossings++;
        }

        result.steps[tick_index].tick_index = tick_index;
        result.steps[tick_index].food_after = food_after;
        result.steps[tick_index].water_after = water_after;
        result.steps[tick_index].food_delta =
            (int16_t)(food_after - food_before);
        result.steps[tick_index].water_delta =
            (int16_t)(water_after - water_before);
        result.steps[tick_index].net_stamina_change =
            tick_output.netStaminaChange;
        result.steps[tick_index].health_damage = tick_output.healthDamage;
        result.steps[tick_index].food_band_before = food_band_prev;
        result.steps[tick_index].food_band_after = food_band_after;
        result.steps[tick_index].water_band_before = water_band_prev;
        result.steps[tick_index].water_band_after = water_band_after;
        result.steps[tick_index].food_bar_color =
            dm1_v1_cpfwwt_color_for_band_food(food_band_after);
        result.steps[tick_index].water_bar_color =
            dm1_v1_cpfwwt_color_for_band_water(water_band_after);
        result.steps[tick_index].food_warning_fired_this_tick =
            food_warning_fired;
        result.steps[tick_index].water_warning_fired_this_tick =
            water_warning_fired;
        result.steps[tick_index].panel_mask_set_this_tick = panel_mask_set;
        result.steps[tick_index].status_box_mask_set_this_tick =
            status_box_mask_set;
        result.steps[tick_index].panel_warning_kind_bits = warning_bits;
        result.steps[tick_index].clamp_to_floor_fired = clamp_fired;

        result.step_count = tick_index + 1;
        food_before = food_after;
        water_before = water_after;
        food_band_prev = food_band_after;
        water_band_prev = water_band_after;

        /* Once both food and water are pinned at the red floor, the panel
         * mask must clear so subsequent ticks do not keep redrawing a
         * stable RED warning. The ReDMCSB tick keeps MASK0x0800_PANEL set
         * only when a band transition fires; we model that by clearing
         * the mask whenever neither food nor water crossed a band. */
        if (!food_warning_fired && !water_warning_fired) {
            final_panel_mask_cleared = 1;
        }
    }

    result.food_band_crossings = food_crossings;
    result.water_band_crossings = water_crossings;
    result.food_floor_clamp_tick = food_floor_clamp_tick;
    result.water_floor_clamp_tick = water_floor_clamp_tick;
    result.final_food_band = dm1_v1_cpfwwt_band_for_amount(food_before);
    result.final_water_band = dm1_v1_cpfwwt_band_for_amount(water_before);
    result.final_food_bar_color =
        dm1_v1_cpfwwt_color_for_band_food(result.final_food_band);
    result.final_water_bar_color =
        dm1_v1_cpfwwt_color_for_band_water(result.final_water_band);
    result.any_tick_set_panel_mask = any_panel_mask;
    result.any_tick_set_status_box_mask = any_status_box_mask;
    result.panel_warning_kind_mask_union = panel_warning_kind_mask_union;
    result.initial_panel_mask_clear = any_panel_mask ? 0 : 1;
    result.final_panel_mask_cleared_after_terminal_band =
        final_panel_mask_cleared;
    return result;
}
