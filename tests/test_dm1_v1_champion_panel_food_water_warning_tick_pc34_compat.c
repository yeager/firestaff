/*
 * test_dm1_v1_champion_panel_food_water_warning_tick_pc34_compat.c
 *
 * DM1 V1 food/water threshold warning on a single game tick.
 *
 * Pins the F0344 warning colour thresholds (PANEL.C:1519..1525), the
 * F0331 per-tick decay (CHAMPION.C:2360..2415), the F0834 floor clamp
 * at -1024 (CHAMPION.C:2413..2418), and the F0292:1060..1062 panel
 * sync mask + MASK0x1000_STATUS_BOX cascade as a single champion
 * transitions across NORMAL -> YELLOW -> RED across a bounded tick
 * window.  This is contract-only coverage; it does not load
 * GRAPHICS.DAT or DUNGEON.DAT, drive real M11 graphics, or claim
 * original parity.
 *
 * Disjoint from:
 *   test_dm1_v1_champion_panel_food_water_status_box
 *     (chest-close -> 67x29 status-box one-shot contract).
 *   test_dm1_v1_champion_panel_hud_food_water_recompute
 *     (F0331 clock-driven recompute + F0355 close hook cascade).
 *   test_dm1_v1_champion_panel_leader_swap_food_water
 *     (F0302 inventory-champion leader-swap + BUG0_39 flicker).
 *   test_dm1_v1_chm05_f0832_hunger_thirst_loop_guard
 *     (F0832 64-iteration loop-guard and stamina loss formula).
 *   test_dm1_v1_champion_panel_mouth_eye_poison_warning
 *     (mouth/eye warning border / palette flash contract).
 */

#include "dm1_v1_champion_panel_food_water_warning_tick_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *label, int got, int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
    } else {
        printf("PASS %s == %d (%s)\n", label, want, anchor);
    }
}

static void expect_u16(const char *label, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%04X want=0x%04X anchor=%s\n",
               label, (unsigned)got, (unsigned)want, anchor);
    } else {
        printf("PASS %s == 0x%04X (%s)\n",
               label, (unsigned)want, anchor);
    }
}

static void expect_str_contains(const char *label, const char *haystack,
                                const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", label, needle, anchor);
    }
}

static void expect_str_not_contains(const char *label, const char *haystack,
                                    const char *needle, const char *anchor)
{
    ++g_assertions;
    if (haystack && needle && strstr(haystack, needle) != NULL) {
        ++g_failures;
        printf("FAIL %s unexpectedly contains=\"%s\" anchor=%s\n",
               label, needle, anchor);
    } else {
        printf("PASS %s does NOT contain \"%s\" (%s)\n",
               label, needle, anchor);
    }
}

static void test_contract_anchors(void)
{
    const dm1_v1_champion_panel_food_water_warning_tick_contract_pc34_t
        *contract =
        dm1_v1_champion_panel_food_water_warning_tick_contract_pc34();
    const char *anchor =
        "PANEL.C F0344:1519..1525 colour thresholds; "
        "CHAMPION.C F0331:2360..2415 per-tick decay; "
        "CHAMDRAW.C F0292:1060..1062 panel sync mask";

    expect_int("contract.contract_only", contract->contract_only, 1, anchor);
    expect_int("contract.food_warn_threshold", contract->food_warn_threshold,
               0, "PANEL.C F0344:1522 amount < 0 -> YELLOW");
    expect_int("contract.food_red_threshold", contract->food_red_threshold,
               -512, "PANEL.C F0344:1519 amount < -512 -> RED");
    expect_int("contract.water_warn_threshold",
               contract->water_warn_threshold, 0,
               "PANEL.C F0344:1522 water < 0 -> YELLOW");
    expect_int("contract.water_red_threshold", contract->water_red_threshold,
               -512, "PANEL.C F0344:1519 water < -512 -> RED");
    expect_int("contract.food_floor", contract->food_floor, -1024,
               "CHAMPION.C F0331:2413 L1010_ps_Champion->Food clamp");
    expect_int("contract.water_floor", contract->water_floor, -1024,
               "CHAMPION.C F0331:2416 L1010_ps_Champion->Water clamp");
    expect_u16("contract.panel_mask", contract->panel_mask_value,
               DM1_V1_CPFWWT_MASK_PANEL_PC34,
               "DEFS.H:728 MASK0x0800_PANEL");
    expect_u16("contract.status_box_mask", contract->status_box_mask_value,
               DM1_V1_CPFWWT_MASK_STATUS_BOX_PC34,
               "DEFS.H:732 MASK0x1000_STATUS_BOX");
    expect_int("contract.food_yellow_colour",
               contract->food_band_yellow_color,
               DM1_V1_CPFWWT_COLOR_YELLOW_PC34,
               "PANEL.C F0344:1522 -> C11_COLOR_YELLOW");
    expect_int("contract.food_red_colour", contract->food_band_red_color,
               DM1_V1_CPFWWT_COLOR_RED_PC34,
               "PANEL.C F0344:1519 -> C08_COLOR_RED");
    expect_int("contract.food_normal_colour",
               contract->food_band_normal_color,
               DM1_V1_CPFWWT_COLOR_LIGHT_BROWN_PC34,
               "PANEL.C F0344:1524 -> C05_COLOR_LIGHT_BROWN base");
    expect_int("contract.water_normal_colour",
               contract->water_band_normal_color,
               DM1_V1_CPFWWT_COLOR_BLUE_PC34,
               "PANEL.C F0344:1524 -> C14_COLOR_BLUE base");
    expect_str_contains("source.f0331_decay_anchor",
                        contract->decay_anchor,
                        "CHAMPION.C F0331:2360..2415",
                        "F0331 per-tick decay reference");
    expect_str_contains("source.f0344_colour_anchor",
                        contract->colour_anchor,
                        "PANEL.C F0344:1519..1525",
                        "F0344 warning colour reference");
    expect_str_contains("source.f0292_panel_anchor",
                        contract->panel_sync_anchor,
                        "CHAMDRAW.C F0292:1060..1062",
                        "F0292 panel sync mask reference");
}

static void test_source_evidence_and_disjoint_contract(void)
{
    const char *source =
        dm1_v1_champion_panel_food_water_warning_tick_source_evidence_pc34();

    expect_str_contains("source.f0331_2360", source,
        "F0331:2360..2415",
        "CHAMPION.C F0331:2360..2415 inner do-while anchor");
    expect_str_contains("source.f0331_food_minus_2", source,
        "Food -= 2",
        "F0331:2392 / F0331:2398 food per-cycle decrement");
    expect_str_contains("source.f0331_water_minus_1", source,
        "Water -= 1",
        "F0331:2403 / F0331:2409 water per-cycle decrement");
    expect_str_contains("source.f0331_floor_clamp", source,
        "Food = -1024",
        "CHAMPION.C F0331:2413 floor clamp");
    expect_str_contains("source.f0344_red_threshold", source,
        "C08_COLOR_RED",
        "PANEL.C F0344:1519 red warning colour");
    expect_str_contains("source.f0344_yellow_threshold", source,
        "C11_COLOR_YELLOW",
        "PANEL.C F0344:1522 yellow warning colour");
    expect_str_contains("source.f0344_food_base", source,
        "C05_COLOR_LIGHT_BROWN",
        "PANEL.C F0344:1524 food base colour");
    expect_str_contains("source.f0344_water_base", source,
        "C14_COLOR_BLUE",
        "PANEL.C F0344:1524 water base colour");
    expect_str_contains("source.f0292_panel_mask", source,
        "MASK0x0800_PANEL",
        "CHAMDRAW.C F0292:1060..1062 panel sync mask");
    expect_str_contains("source.f0292_status_box_mask", source,
        "MASK0x1000_STATUS_BOX",
        "DEFS.H:732 status box mask anchor");
    expect_str_contains("source.defs_zones", source,
        "C500/C501/C502/C503",
        "DEFS.H:3869..3872 panel zones");
    expect_str_contains("source.contract_only", source,
        "contract_only=1",
        "evidence string marks slice as contract-only");

    /* Disjoint from the chest-close one-shot contract. */
    expect_str_not_contains("disjoint.chest_close_anchor", source,
        "CHEST.C F0334:113-132",
        "chest close anchor owned by food_water_status_box");
    /* Disjoint from the F0331 clock-driven recompute contract. */
    expect_str_not_contains("disjoint.f0355_close_hook", source,
        "F0355_INVENTORY_Toggle_CPSE",
        "close hook owned by hud_food_water_recompute");
    /* Disjoint from the F0302 inventory-champion leader-swap. */
    expect_str_not_contains("disjoint.f0302_inventory_branch", source,
        "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
        "leader-swap dispatch owned by leader_swap_food_water");
    /* Disjoint from the F0832 loop-guard test. */
    expect_str_not_contains("disjoint.f0832_loop_guard", source,
        "F0832_LIFECYCLE_TickHungerThirst_Compat",
        "loop-guard formula owned by chm05_f0832_hunger_thirst_loop_guard");
    /* Disjoint from the mouth/eye warning border contract. */
    expect_str_not_contains("disjoint.mouth_eye_anchor", source,
        "C545_ZONE_MOUTH",
        "mouth zone owned by mouth_eye_poison_warning");
}

static void test_default_input_is_well_formed(void)
{
    dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t input =
        dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34();
    const char *anchor =
        "dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34";

    expect_int("default.food_before", input.food_before, 6,
               "forced-delta default places food=6 so all crossings land");
    expect_int("default.water_before", input.water_before, 2,
               "forced-delta default places water=2 so all crossings land");
    expect_int("default.food_force_on",
               input.force_per_tick_food_delta, 1,
               "default exercises the forced-delta path");
    expect_int("default.water_force_on",
               input.force_per_tick_water_delta, 1,
               "default exercises the forced-delta path");
    expect_int("default.food_delta", input.forced_food_delta, 32,
               "default food delta = 32 so all crossings fit in 64 ticks");
    expect_int("default.water_delta", input.forced_water_delta, 17,
               "default water delta = 17 so all crossings fit in 64 ticks");
    (void)anchor;
}

static void test_invalid_input_rejected(void)
{
    dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t input =
        dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34();
    dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t result;
    const char *anchor =
        "dm1_v1_champion_panel_food_water_warning_tick_run_pc34 input guards";

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        NULL, 0, 32);
    expect_int("null_input.rejected_step_count", result.step_count, 0, anchor);

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, -1, 32);
    expect_int("negative_champion.rejected_step_count", result.step_count, 0,
               anchor);

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, DM1_V1_CPFWWT_CHAMPION_COUNT_PC34, 32);
    expect_int("out_of_range_champion.rejected_step_count", result.step_count,
               0, anchor);

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, 0, 0);
    expect_int("zero_tick_count.rejected_step_count", result.step_count, 0,
               anchor);

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, 0, 65);
    expect_int("oversize_tick_count.rejected_step_count", result.step_count, 0,
               anchor);
}

static void test_default_run_band_crossings_and_panel_mask(void)
{
    dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t input =
        dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34();
    dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t result;
    const char *anchor =
        "CHAMPION.C F0331:2360..2415 per-tick decay; "
        "PANEL.C F0344:1519..1525 colour thresholds; "
        "CHAMDRAW.C F0292:1060..1062 panel sync mask";

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, 0, 64);

    expect_int("default_run.contract_only", result.contract_only, 1, anchor);
    expect_int("default_run.loads_graphics_dat", result.loads_graphics_dat, 0,
               "contract-only slice, no live GRAPHICS.DAT");
    expect_int("default_run.loads_dungeon_dat", result.loads_dungeon_dat, 0,
               "contract-only slice, no live DUNGEON.DAT");
    expect_int("default_run.champion_index", result.champion_index, 0, anchor);
    expect_int("default_run.tick_count", result.tick_count, 64, anchor);
    expect_int("default_run.step_count", result.step_count, 64, anchor);

    /* Default food=6, -32/tick -> yellow at tick 0, red at tick 16,
     * floor at tick 32. */
    expect_int("default_run.food_band_crossings",
               result.food_band_crossings, 2,
               "default food crosses NORMAL->YELLOW then YELLOW->RED");
    expect_int("default_run.food_band_crossing_tick_indices[0]",
               result.food_band_crossing_tick_indices[0], 0,
               "food NORMAL->YELLOW at tick 0");
    expect_int("default_run.food_band_crossing_tick_indices[1]",
               result.food_band_crossing_tick_indices[1], 16,
               "food YELLOW->RED at tick 16");
    expect_int("default_run.food_floor_clamp_tick",
               result.food_floor_clamp_tick, 32,
               "food clamps to -1024 first at tick 32");

    /* Default water=2, -17/tick -> yellow at tick 0, red at tick 30,
     * floor at tick 60. */
    expect_int("default_run.water_band_crossings",
               result.water_band_crossings, 2,
               "default water crosses NORMAL->YELLOW then YELLOW->RED");
    expect_int("default_run.water_band_crossing_tick_indices[0]",
               result.water_band_crossing_tick_indices[0], 0,
               "water NORMAL->YELLOW at tick 0");
    expect_int("default_run.water_band_crossing_tick_indices[1]",
               result.water_band_crossing_tick_indices[1], 30,
               "water YELLOW->RED at tick 30");
    expect_int("default_run.water_floor_clamp_tick",
               result.water_floor_clamp_tick, 60,
               "water clamps to -1024 first at tick 60");

    expect_int("default_run.final_food_band", result.final_food_band,
               DM1_V1_CPFWWT_BAND_RED_PC34,
               "after 64 ticks food is pinned at red band");
    expect_int("default_run.final_water_band", result.final_water_band,
               DM1_V1_CPFWWT_BAND_RED_PC34,
               "after 64 ticks water is pinned at red band");
    expect_int("default_run.final_food_bar_color",
               result.final_food_bar_color,
               DM1_V1_CPFWWT_COLOR_RED_PC34,
               "final food colour is RED per F0344:1519");
    expect_int("default_run.final_water_bar_color",
               result.final_water_bar_color,
               DM1_V1_CPFWWT_COLOR_RED_PC34,
               "final water colour is RED per F0344:1519");

    expect_int("default_run.any_tick_set_panel_mask",
               result.any_tick_set_panel_mask, 1,
               "at least one warning tick fires F0292:1060 panel sync");
    expect_int("default_run.any_tick_set_status_box_mask",
               result.any_tick_set_status_box_mask, 1,
               "MASK0x1000_STATUS_BOX set on the same warning tick");
    expect_int("default_run.panel_warning_kind_mask_union",
               result.panel_warning_kind_mask_union,
               DM1_V1_CPFWWT_WARNING_KIND_BOTH_PC34,
               "both food and water warnings fire across the 64 ticks");
    expect_int("default_run.initial_panel_mask_clear",
               result.initial_panel_mask_clear, 0,
               "panel mask was set by tick 0 because food/water crossed");

    /* Tick 0 must show the yellow band colour and the panel mask set. */
    expect_int("default_run.tick0.food_band_before",
               result.steps[0].food_band_before,
               DM1_V1_CPFWWT_BAND_NORMAL_PC34,
               "food starts at NORMAL band");
    expect_int("default_run.tick0.food_band_after",
               result.steps[0].food_band_after,
               DM1_V1_CPFWWT_BAND_YELLOW_PC34,
               "food flips to YELLOW on tick 0");
    expect_int("default_run.tick0.food_bar_color",
               result.steps[0].food_bar_color,
               DM1_V1_CPFWWT_COLOR_YELLOW_PC34,
               "tick 0 food bar uses YELLOW per F0344:1522");
    expect_int("default_run.tick0.water_band_after",
               result.steps[0].water_band_after,
               DM1_V1_CPFWWT_BAND_YELLOW_PC34,
               "water flips to YELLOW on tick 0");
    expect_int("default_run.tick0.water_bar_color",
               result.steps[0].water_bar_color,
               DM1_V1_CPFWWT_COLOR_YELLOW_PC34,
               "tick 0 water bar uses YELLOW per F0344:1522");
    expect_int("default_run.tick0.panel_mask_set",
               result.steps[0].panel_mask_set_this_tick, 1,
               "tick 0 sets MASK0x0800_PANEL per F0292:1060");
    expect_int("default_run.tick0.status_box_mask_set",
               result.steps[0].status_box_mask_set_this_tick, 1,
               "tick 0 also sets MASK0x1000_STATUS_BOX for the cascade");
    expect_int("default_run.tick0.warning_kind_bits",
               result.steps[0].panel_warning_kind_bits,
               DM1_V1_CPFWWT_WARNING_KIND_BOTH_PC34,
               "tick 0 warning bits cover both food and water");

    /* Tick 15 (just before the food red crossing) must NOT yet be red. */
    expect_int("default_run.tick15.food_band_after",
               result.steps[15].food_band_after,
               DM1_V1_CPFWWT_BAND_YELLOW_PC34,
               "food still YELLOW on tick 15");
    expect_int("default_run.tick15.food_bar_color",
               result.steps[15].food_bar_color,
               DM1_V1_CPFWWT_COLOR_YELLOW_PC34,
               "tick 15 food bar colour YELLOW per F0344:1522");
    expect_int("default_run.tick15.food_warning_fired",
               result.steps[15].food_warning_fired_this_tick, 0,
               "tick 15 food warning did not fire (no band change)");

    /* Tick 16 is the food red crossing. */
    expect_int("default_run.tick16.food_band_after",
               result.steps[16].food_band_after,
               DM1_V1_CPFWWT_BAND_RED_PC34,
               "food flips to RED on tick 16");
    expect_int("default_run.tick16.food_bar_color",
               result.steps[16].food_bar_color,
               DM1_V1_CPFWWT_COLOR_RED_PC34,
               "tick 16 food bar colour RED per F0344:1519");
    expect_int("default_run.tick16.food_warning_fired",
               result.steps[16].food_warning_fired_this_tick, 1,
               "tick 16 food warning fires because band flipped");
    expect_int("default_run.tick16.panel_mask_set",
               result.steps[16].panel_mask_set_this_tick, 1,
               "tick 16 sets MASK0x0800_PANEL because food crossed to red");

    /* Tick 30 is the water red crossing. */
    expect_int("default_run.tick30.water_band_after",
               result.steps[30].water_band_after,
               DM1_V1_CPFWWT_BAND_RED_PC34,
               "water flips to RED on tick 30");
    expect_int("default_run.tick30.water_warning_fired",
               result.steps[30].water_warning_fired_this_tick, 1,
               "tick 30 water warning fires because band flipped");
    expect_int("default_run.tick30.panel_mask_set",
               result.steps[30].panel_mask_set_this_tick, 1,
               "tick 30 sets MASK0x0800_PANEL because water crossed to red");

    /* Tick 31 (no crossings) must NOT set the panel mask. */
    expect_int("default_run.tick31.food_warning_fired",
               result.steps[31].food_warning_fired_this_tick, 0,
               "tick 31 food warning does not fire");
    expect_int("default_run.tick31.water_warning_fired",
               result.steps[31].water_warning_fired_this_tick, 0,
               "tick 31 water warning does not fire");
    expect_int("default_run.tick31.panel_mask_set",
               result.steps[31].panel_mask_set_this_tick, 0,
               "tick 31 panel mask NOT set (no band change)");
    expect_int("default_run.tick31.status_box_mask_set",
               result.steps[31].status_box_mask_set_this_tick, 0,
               "tick 31 status box mask NOT set (no band change)");

    /* Tick 32 clamps food at -1024. */
    expect_int("default_run.tick32.food_after",
               result.steps[32].food_after, -1024,
               "food clamped at -1024 on tick 32");
    expect_int("default_run.tick32.clamp_to_floor_fired",
               result.steps[32].clamp_to_floor_fired, 1,
               "tick 32 fires the F0331:2413 floor clamp");
}

static void test_forced_normal_run_no_warning(void)
{
    dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t input =
        dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34();
    dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t result;
    const char *anchor =
        "F0344:1524 base colour path with food/water > 0; "
        "F0292:1060 panel mask must NOT fire when no band crosses";

    /* Tiny per-tick delta keeps food/water firmly above zero, so the
     * warning tick must never fire and the panel mask must stay clear. */
    input.food_before = 1024;
    input.water_before = 1024;
    input.forced_food_delta = 4;
    input.forced_water_delta = 2;
    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, 0, 32);

    expect_int("forced_normal_run.food_band_crossings",
               result.food_band_crossings, 0, anchor);
    expect_int("forced_normal_run.water_band_crossings",
               result.water_band_crossings, 0, anchor);
    expect_int("forced_normal_run.final_food_band",
               result.final_food_band,
               DM1_V1_CPFWWT_BAND_NORMAL_PC34, anchor);
    expect_int("forced_normal_run.final_water_band",
               result.final_water_band,
               DM1_V1_CPFWWT_BAND_NORMAL_PC34, anchor);
    expect_int("forced_normal_run.any_tick_set_panel_mask",
               result.any_tick_set_panel_mask, 0, anchor);
    expect_int("forced_normal_run.any_tick_set_status_box_mask",
               result.any_tick_set_status_box_mask, 0, anchor);
    expect_int("forced_normal_run.panel_warning_kind_mask_union",
               result.panel_warning_kind_mask_union, 0, anchor);
    expect_int("forced_normal_run.final_food_bar_color",
               result.final_food_bar_color,
               DM1_V1_CPFWWT_COLOR_LIGHT_BROWN_PC34,
               "food stays at base colour C05_COLOR_LIGHT_BROWN");
    expect_int("forced_normal_run.final_water_bar_color",
               result.final_water_bar_color,
               DM1_V1_CPFWWT_COLOR_BLUE_PC34,
               "water stays at base colour C14_COLOR_BLUE");
}

static void test_natural_decay_runs_through_warn_once(void)
{
    dm1_v1_champion_panel_food_water_warning_tick_input_pc34_t input =
        dm1_v1_champion_panel_food_water_warning_tick_default_input_pc34();
    dm1_v1_champion_panel_food_water_warning_tick_result_pc34_t result;
    const char *anchor =
        "F0833 LIFECYCLE natural decay path drives the gate when no "
        "forced-delta override is set; warning tick still fires on the "
        "first band crossing.";

    /* Disable the forced-delta path so F0833's stamina-cycles-driven
     * decay drives the gate. F0833 subtracts 4 * 2 = 8 from Food per
     * call when stamina is at half (currentStamina = maxStamina/2,
     * staminaGainCycleCount stays at 4). Food=64 -> 8 calls to reach
     * the yellow threshold at food=0. */
    input.food_before = 64;
    input.water_before = 64;
    input.force_per_tick_food_delta = 0;
    input.force_per_tick_water_delta = 0;

    result = dm1_v1_champion_panel_food_water_warning_tick_run_pc34(
        &input, 0, 32);

    expect_int("natural_decay.food_band_crossings",
               result.food_band_crossings, 1,
               "F0833 natural decay crosses food into YELLOW at least once");
    expect_int("natural_decay.food_band_crossing_tick_indices[0]",
               result.food_band_crossing_tick_indices[0], 8,
               "food NORMAL->YELLOW crossing on the eighth tick "
               "(64 -> 56 -> 48 -> ... -> 8 -> 0)");
    expect_int("natural_decay.tick8.food_warning_fired",
               result.steps[8].food_warning_fired_this_tick, 1,
               "tick 8 fires the warning because food crossed yellow");
    expect_int("natural_decay.tick8.panel_mask_set",
               result.steps[8].panel_mask_set_this_tick, 1,
               "tick 8 sets MASK0x0800_PANEL");
    expect_int("natural_decay.tick0.food_warning_fired",
               result.steps[0].food_warning_fired_this_tick, 0,
               "tick 0 food warning does NOT fire (food=64 still NORMAL)");
    expect_int("natural_decay.any_tick_set_panel_mask",
               result.any_tick_set_panel_mask, 1,
               "natural-decay run still fires the warning tick");
    (void)anchor;
}

int main(void)
{
    test_contract_anchors();
    test_source_evidence_and_disjoint_contract();
    test_default_input_is_well_formed();
    test_invalid_input_rejected();
    test_default_run_band_crossings_and_panel_mask();
    test_forced_normal_run_no_warning();
    test_natural_decay_runs_through_warn_once();

    printf("PASS test_dm1_v1_champion_panel_food_water_warning_tick_pc34_compat "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
