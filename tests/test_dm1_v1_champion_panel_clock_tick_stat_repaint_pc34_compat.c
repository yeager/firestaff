#include "firestaff/dm1/v1/champion_panel/clock_tick_stat_repaint_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d [%s]\n", message, actual,
               expected, anchor ? anchor : "(null)");
    }
}

static void check_u16_eq(uint16_t actual, uint16_t expected,
                         const char *message, const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=0x%04X expected=0x%04X [%s]\n", message,
               (unsigned)actual, (unsigned)expected,
               anchor ? anchor : "(null)");
    }
}

static void check_u16_has(uint16_t actual, uint16_t mask,
                          const char *message, const char *anchor)
{
    ++g_assertions;
    if ((actual & mask) != mask) {
        ++g_failures;
        printf("FAIL %s actual=0x%04X missing-mask=0x%04X [%s]\n", message,
               (unsigned)actual, (unsigned)mask,
               anchor ? anchor : "(null)");
    }
}

static void check_u16_lacks(uint16_t actual, uint16_t mask,
                            const char *message, const char *anchor)
{
    ++g_assertions;
    if ((actual & mask) != 0u) {
        ++g_failures;
        printf("FAIL %s actual=0x%04X unexpected-mask=0x%04X [%s]\n", message,
               (unsigned)actual, (unsigned)mask,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    }
}

static void test_evidence_strings(void)
{
    const char *evidence = fs_dm1_v1_cts_source_evidence_pc34();
    check_contains("evidence.F0331", evidence,
                   "F0331_CHAMPION_ApplyTimeEffects_CPSF",
                   "CHAMPION.C F0331");
    check_contains("evidence.G0305", evidence, "G0305",
                   "DEFS.H:5700 G0305");
    check_contains("evidence.MASK0x0100", evidence, "MASK0x0100_STATISTICS",
                   "F0331:2494");
    check_contains("evidence.MASK0x0800", evidence, "MASK0x0800_PANEL",
                   "F0331:2495-2497");
    check_contains("evidence.MASK0x0400", evidence, "MASK0x0400_ICON",
                   "F0331:2491-2493");
    check_contains("evidence.F0293", evidence,
                   "F0293_CHAMPION_DrawAllChampionStates", "F0331:2503");
    check_contains("evidence.recovery_256", evidence, "256", "F0331:2487");
    check_contains("evidence.recovery_64", evidence, "64", "F0331:2487");

    const char *non_overlap = fs_dm1_v1_cts_non_overlap_pc34();
    check_contains("non_overlap.food_water_recompute", non_overlap,
                   "food_water_recompute", "hud_food_water_recompute");
    check_contains("non_overlap.portrait_state", non_overlap,
                   "portrait_state_redraw", "portrait_state_redraw");
    check_contains("non_overlap.hand_slot_priority", non_overlap,
                   "hand_slot_priority", "hand_slot_priority");
}

static void test_input_init(void)
{
    FsDm1V1CtsTickInputPc34 input;
    fs_dm1_v1_cts_input_init_pc34(&input);
    check_int_eq(input.party_champion_count, 0,
                 "init.party_champion_count", "F0331:2331");
    check_int_eq(input.inventory_champion_ordinal, 0,
                 "init.inventory_champion_ordinal", "G0423");
    check_int_eq(input.candidate_champion_ordinal, 0,
                 "init.candidate_champion_ordinal", "G0299");
    check_int_eq(input.party_direction, 0,
                 "init.party_direction", "G0308");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_int_eq(input.champions[i].alive, 1, "init.alive", "F0331:2335");
        check_int_eq(input.champions[i].ordinal, i + 1, "init.ordinal",
                     "F0331:2335");
    }
}

static void test_empty_party_early_return(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 0;
    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.valid, 1, "empty.valid", "F0331:2331");
    check_int_eq(result.draw_all_champion_states_invoked, 0,
                 "empty.no_f0293", "F0331:2503");
    check_int_eq(result.draw_all_champion_states_arg, 0x0000u,
                 "empty.f0293_arg_zero", "F0331:2503");
    check_int_eq(result.statistics_attribute_set_count, 0,
                 "empty.no_stat_dirty", "F0331:2494");
    check_int_eq(result.champion_skip_count, 0,
                 "empty.no_skip", "F0331:2331");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_int_eq(result.attributes_after[i], 0, "empty.attr",
                     "F0331:2331");
    }
}

static void test_full_party_alive_no_panel(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    input.last_creature_attack_time = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.valid, 1, "full.valid", "F0331:2335");
    check_int_eq(result.time_criteria_3bit, 0, "full.time_criteria_3bit",
                 "F0331:2334");
    check_int_eq(result.recovery_period, 256, "full.recovery_period",
                 "F0331:2487");
    check_int_eq(result.recovery_due_this_tick, 1,
                 "full.recovery_due_active_scratch_zero",
                 "F0331:2487");
    check_int_eq(result.statistics_attribute_set_count, 4,
                 "full.stat_dirty_count", "F0331:2494");
    check_int_eq(result.panel_attribute_set_index, -1,
                 "full.no_panel_set", "F0331:2495-2497");
    check_int_eq(result.icon_attribute_set_count, 0,
                 "full.no_icon_drift", "F0331:2491-2493");
    check_int_eq(result.direction_resync_champion_index, -1,
                 "full.no_direction_resync", "F0331:2491-2493");
    check_int_eq(result.dead_skipped, 0, "full.dead_skipped", "F0331:2335");
    check_int_eq(result.candidate_skipped, 0, "full.candidate_skipped",
                 "F0331:2335");
    check_int_eq(result.out_of_party_skipped, 0,
                 "full.out_of_party_skipped", "F0331:2335");
    check_int_eq(result.draw_all_champion_states_invoked, 1,
                 "full.f0293_invoked", "F0331:2503");
    check_int_eq(result.draw_state_calls_f0293_after_loop, 1,
                 "full.f0293_after_loop", "F0331:2503");
    check_u16_eq(result.draw_all_champion_states_arg, 0x0000u,
                 "full.f0293_arg", "F0331:2503");

    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_has(result.attributes_after[i],
                      FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                      "full.stat_attr_set", "F0331:2494");
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_PANEL_PC34, "full.no_panel_attr",
                        "F0331:2495-2497");
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_ICON_PC34, "full.no_icon_attr",
                        "F0331:2491-2493");
    }
}

static void test_dead_champion_skipped(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 1;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    input.champions[1].current_health = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.dead_skipped, 1, "dead.skipped", "F0331:2335");
    check_int_eq(result.statistics_attribute_set_count, 3,
                 "dead.stat_dirty_count", "F0331:2494");
    check_u16_lacks(result.attributes_after[1],
                    FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                    "dead.no_stat_attr", "F0331:2335");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        if (i == 1) continue;
        check_u16_has(result.attributes_after[i],
                      FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                      "dead.others_stat_attr", "F0331:2494");
    }
    check_int_eq(result.attributes_before[1], 0, "dead.pre_attr", "F0331");
}

static void test_candidate_champion_skipped(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 3;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.candidate_skipped, 1,
                 "candidate.skipped", "F0331:2335");
    check_int_eq(result.statistics_attribute_set_count, 3,
                 "candidate.stat_dirty_count", "F0331:2494");
    check_u16_lacks(result.attributes_after[2],
                    FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                    "candidate.no_stat_attr", "F0331:2335");
    check_int_eq(result.champion_skipped_reason[2], 2, "candidate.reason",
                 "F0331:2335");
}

static void test_out_of_party_champion_not_visited(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 2;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.out_of_party_skipped, 2, "oop.skipped",
                 "F0331:2335");
    check_int_eq(result.statistics_attribute_set_count, 2,
                 "oop.stat_dirty_count", "F0331:2494");
    for (int i = 2; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                        "oop.no_stat_attr", "F0331:2335");
        check_int_eq(result.attributes_after[i], 0, "oop.attr_zero",
                     "F0331:2335");
        check_int_eq(result.champion_skipped_reason[i], 4, "oop.reason",
                     "F0331:2335");
    }
    for (int i = 0; i < 2; ++i) {
        check_u16_has(result.attributes_after[i],
                      FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                      "oop.visited_stat_attr", "F0331:2494");
    }
}

static void test_inventory_champion_panel_food_water(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 2;
    input.candidate_champion_ordinal = 0;
    input.panel_content = FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.panel_attribute_set_index, 1,
                 "panel_fw.set_index", "F0331:2495-2497");
    check_u16_has(result.attributes_after[1],
                  FS_DM1_V1_CTS_ATTR_PANEL_PC34, "panel_fw.attr_set",
                  "F0331:2495-2497");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        if (i == 1) continue;
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_PANEL_PC34, "panel_fw.no_panel",
                        "F0331:2495-2497");
    }
}

static void test_inventory_champion_panel_skills_stats(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 4;
    input.candidate_champion_ordinal = 0;
    input.panel_content = FS_DM1_V1_CTS_PANEL_SKILLS_AND_STATISTICS_PC34;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.panel_attribute_set_index, 3,
                 "panel_ss.set_index", "F0331:2495-2497");
    check_u16_has(result.attributes_after[3],
                  FS_DM1_V1_CTS_ATTR_PANEL_PC34, "panel_ss.attr_set",
                  "F0331:2495-2497");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        if (i == 3) continue;
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_PANEL_PC34,
                        "panel_ss.no_panel_others", "F0331:2495-2497");
    }
}

static void test_panel_not_eligible(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 1;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 999;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.panel_attribute_set_index, -1,
                 "panel_no.no_set", "F0331:2495-2497");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_PANEL_PC34,
                        "panel_no.attr_clean", "F0331:2495-2497");
    }
}

static void test_direction_resync_icon(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 2;
    input.last_creature_attack_time = -100;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }
    /* champion 2's direction already matches so its ICON is NOT set. */
    input.champions[2].direction = 2;

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.direction_resync_champion_index, 0,
                 "icon.first_resync", "F0331:2491-2493");
    check_int_eq(result.icon_attribute_set_count, 3,
                 "icon.icon_count", "F0331:2491-2493");
    check_u16_has(result.attributes_after[0],
                  FS_DM1_V1_CTS_ATTR_ICON_PC34, "icon.attr_set",
                  "F0331:2491-2493");
    check_u16_has(result.attributes_after[1],
                  FS_DM1_V1_CTS_ATTR_ICON_PC34, "icon.attr_set_1",
                  "F0331:2491-2493");
    check_u16_lacks(result.attributes_after[2],
                    FS_DM1_V1_CTS_ATTR_ICON_PC34, "icon.match_no_set",
                    "F0331:2491-2493");
    check_u16_has(result.attributes_after[3],
                  FS_DM1_V1_CTS_ATTR_ICON_PC34, "icon.attr_set_3",
                  "F0331:2491-2493");
}

static void test_direction_resync_resting_suppresses(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 2;
    input.last_creature_attack_time = -100;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 1;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.icon_attribute_set_count, 0,
                 "icon_rest.no_icon", "F0331:2491-2493");
    check_int_eq(result.direction_resync_champion_index, -1,
                 "icon_rest.no_resync", "F0331:2491-2493");
    check_int_eq(result.recovery_period, 64,
                 "icon_rest.recovery_period_resting", "F0331:2487");
}

static void test_recovery_period_resting(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 1;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.recovery_period, 64, "rest.recovery_period_64",
                 "F0331:2487");
    check_int_eq(result.recovery_due_this_tick, 1,
                 "rest.recovery_due_scratch_zero", "F0331:2487");
    check_int_eq(result.statistic_recovery_applied_count, 4,
                 "rest.recovery_applied_count", "F0331:2487-2490");
}

static void test_f0293_called_once_with_zero_arg(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.draw_all_champion_states_invoked, 1,
                 "f0293.invoked_once", "F0331:2503");
    check_int_eq(result.draw_state_calls_f0293_after_loop, 1,
                 "f0293.after_loop_only", "F0331:2503");
    check_u16_eq(result.draw_all_champion_states_arg, 0x0000u,
                 "f0293.arg_zero", "F0331:2503");
}

static void test_attributes_byte_stable_outside_attr_set(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
        input.champions[i].attributes = (uint16_t)(0x0001u << i);
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_has(result.attributes_after[i],
                      (uint16_t)(0x0001u << i),
                      "byte_stable.pre_attr_kept", "F0331 M008_SET");
        check_u16_has(result.attributes_after[i],
                      FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                      "byte_stable.stat_set", "F0331:2494");
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_PANEL_PC34,
                        "byte_stable.no_panel", "F0331:2495-2497");
    }
}

static void test_time_criteria_deterministic(void)
{
    FsDm1V1CtsTickInputPc34 input_a;
    FsDm1V1CtsTickInputPc34 input_b;
    FsDm1V1CtsTickResultPc34 result_a;
    FsDm1V1CtsTickResultPc34 result_b;
    fs_dm1_v1_cts_input_init_pc34(&input_a);
    fs_dm1_v1_cts_input_init_pc34(&input_b);
    input_a.party_champion_count = 4;
    input_b.party_champion_count = 4;
    input_a.inventory_champion_ordinal = 0;
    input_b.inventory_champion_ordinal = 0;
    input_a.candidate_champion_ordinal = 0;
    input_b.candidate_champion_ordinal = 0;
    input_a.panel_content = 0;
    input_b.panel_content = 0;
    input_a.party_direction = 0;
    input_b.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input_a.champions[i].direction = 0;
        input_a.champions[i].party_is_resting = 0;
        input_b.champions[i] = input_a.champions[i];
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input_a, &result_a);
    fs_dm1_v1_cts_run_clock_tick_pc34(&input_b, &result_b);
    check_int_eq(result_a.hash, result_b.hash, "deterministic.hash",
                 "F0331 deterministic");
    check_int_eq(result_a.time_criteria_3bit, result_b.time_criteria_3bit,
                 "deterministic.time_criteria", "F0331:2334");
    check_int_eq(result_a.draw_all_champion_states_arg,
                 result_b.draw_all_champion_states_arg,
                 "deterministic.f0293_arg", "F0331:2503");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_eq(result_a.attributes_after[i], result_b.attributes_after[i],
                     "deterministic.attr_byte", "F0331 M008_SET");
    }
}

static void test_alive_directions_matching_no_resync(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 1;
    input.last_creature_attack_time = -100;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 1;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.icon_attribute_set_count, 0, "match.no_resync",
                 "F0331:2491-2493");
    check_int_eq(result.direction_resync_champion_index, -1,
                 "match.no_resync_idx", "F0331:2491-2493");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_ICON_PC34, "match.no_icon_attr",
                        "F0331:2491-2493");
    }
}

static void test_party_count_clamps_to_four(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    /* Out-of-range party count is clamped to the model champion count. */
    input.party_champion_count = 99;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.statistics_attribute_set_count, 4,
                 "clamp.four_visited", "F0331:2335 G0305");
    check_int_eq(result.out_of_party_skipped, 0,
                 "clamp.no_skip", "F0331:2335 G0305");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_has(result.attributes_after[i],
                      FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                      "clamp.stat_attr_set", "F0331:2494");
    }
}

static void test_party_count_negative(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = -1;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.valid, 1, "neg.valid", "F0331:2331");
    check_int_eq(result.draw_all_champion_states_invoked, 0,
                 "neg.no_f0293", "F0331:2503");
    check_int_eq(result.statistics_attribute_set_count, 0,
                 "neg.no_stat_dirty", "F0331:2331");
}

static void test_candidate_with_inventory_no_panel_attr(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 2;
    input.candidate_champion_ordinal = 3;
    input.panel_content = FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.panel_attribute_set_index, 1,
                 "cand_panel.set_index", "F0331:2495-2497");
    check_u16_has(result.attributes_after[1],
                  FS_DM1_V1_CTS_ATTR_PANEL_PC34, "cand_panel.attr_set",
                  "F0331:2495-2497");
    check_u16_lacks(result.attributes_after[2],
                    FS_DM1_V1_CTS_ATTR_PANEL_PC34,
                    "cand_panel.no_attr_for_candidate", "F0331:2335");
}

static void test_inventory_zero_never_panels(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.panel_attribute_set_index, -1,
                 "no_inv.no_set", "G0423 zero");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_lacks(result.attributes_after[i],
                        FS_DM1_V1_CTS_ATTR_PANEL_PC34,
                        "no_inv.no_panel_attr", "G0423 zero");
    }
}

static void test_hash_determinism_two_runs(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result_a;
    FsDm1V1CtsTickResultPc34 result_b;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 2;
    input.candidate_champion_ordinal = 0;
    input.panel_content = FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result_a);
    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result_b);
    check_int_eq(result_a.hash, result_b.hash, "hash2x.equal",
                 "F0331 deterministic");
    check_int_eq(result_a.panel_attribute_set_index, result_b.panel_attribute_set_index,
                 "hash2x.panel_index", "F0331 deterministic");
    check_int_eq(result_a.statistics_attribute_set_count,
                 result_b.statistics_attribute_set_count,
                 "hash2x.stat_dirty", "F0331 deterministic");
}

static void test_byte_stability_pre_attrs(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
        input.champions[i].attributes = (uint16_t)(0xAAAAu);
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_eq(result.attributes_before[i], 0xAAAAu,
                     "byte_stability.pre", "F0331 M008_SET");
        check_u16_has(result.attributes_after[i], 0xAAAAu,
                      "byte_stability.pre_kept", "F0331 M008_SET");
        check_u16_has(result.attributes_after[i],
                      FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                      "byte_stability.stat_or", "F0331:2494");
    }
}

static void test_dead_and_candidate_skip_count(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 3;
    input.panel_content = 0;
    input.party_direction = 0;
    input.champions[1].current_health = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.dead_skipped, 1, "mixed_dead.one_dead",
                 "F0331:2335");
    check_int_eq(result.candidate_skipped, 1, "mixed_dead.one_candidate",
                 "F0331:2335");
    check_int_eq(result.out_of_party_skipped, 0, "mixed_dead.no_oop",
                 "F0331:2335");
    check_int_eq(result.champion_skip_count, 2, "mixed_dead.total_skip",
                 "F0331:2335");
    check_int_eq(result.statistics_attribute_set_count, 2,
                 "mixed_dead.two_visited", "F0331:2494");
    check_int_eq(result.champion_skipped_reason[1], 1,
                 "mixed_dead.reason_1", "F0331:2335");
    check_int_eq(result.champion_skipped_reason[2], 2,
                 "mixed_dead.reason_2", "F0331:2335");
}

static void test_recovery_period_active_is_256(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.recovery_period, 256, "recov_active.256",
                 "F0331:2487");
    check_int_eq(result.statistic_recovery_applied_count, 4,
                 "recov_active.all_four", "F0331:2487-2490");
}

static void test_direction_quiet_recent_no_resync(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 2;
    input.last_creature_attack_time = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    /* scratch_game_time is 0, so last_attack (0) is NOT < (0 - 60). */
    check_int_eq(result.icon_attribute_set_count, 0, "quiet_recent.no_icon",
                 "F0331:2491-2493");
    check_int_eq(result.direction_resync_champion_index, -1,
                 "quiet_recent.no_resync", "F0331:2491-2493");
}

static void test_trace_index_count(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.trace[0] != 0, 1, "trace.size_nonzero",
                 "F0331 trace");
    check_int_eq(result.trace[1], 4, "trace.party_count", "F0331:2335");
    check_int_eq(result.trace[5], 4, "trace.stat_dirty",
                 "F0331:2494");
    check_int_eq(result.trace[6], 1, "trace.recovery_due", "F0331:2487");
    check_int_eq(result.trace[7], 256, "trace.recovery_period", "F0331:2487");
}

static void test_panel_food_water_panel_skills(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result_fw;
    FsDm1V1CtsTickResultPc34 result_ss;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 1;
    input.candidate_champion_ordinal = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    input.panel_content = FS_DM1_V1_CTS_PANEL_FOOD_WATER_POISONED_PC34;
    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result_fw);
    check_int_eq(result_fw.panel_attribute_set_index, 0,
                 "fw_ss.fw_index", "F0331:2495-2497");
    check_u16_has(result_fw.attributes_after[0],
                  FS_DM1_V1_CTS_ATTR_PANEL_PC34, "fw_ss.fw_attr",
                  "F0331:2495-2497");

    input.panel_content = FS_DM1_V1_CTS_PANEL_SKILLS_AND_STATISTICS_PC34;
    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result_ss);
    check_int_eq(result_ss.panel_attribute_set_index, 0,
                 "fw_ss.ss_index", "F0331:2495-2497");
    check_u16_has(result_ss.attributes_after[0],
                  FS_DM1_V1_CTS_ATTR_PANEL_PC34, "fw_ss.ss_attr",
                  "F0331:2495-2497");
    check_int_eq(result_fw.hash != result_ss.hash, 1, "fw_ss.hash_differs",
                 "F0331 deterministic panel_content in hash");
}

static void test_dead_zero_stamina(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    input.champions[1].current_health = 0;
    input.champions[1].current_stamina = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.dead_skipped, 1, "dead_zero.health_zero",
                 "F0331:2335");
    check_int_eq(result.statistics_attribute_set_count, 3,
                 "dead_zero.three_visited", "F0331:2494");
}

static void test_candidate_with_inventory_skills_panel(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 2;
    input.candidate_champion_ordinal = 1;
    input.panel_content = FS_DM1_V1_CTS_PANEL_SKILLS_AND_STATISTICS_PC34;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    /* candidate ordinal 1 is skipped; ordinal 2 (index 1) is inventory +
       alive, so PANEL is set on it. */
    check_int_eq(result.panel_attribute_set_index, 1,
                 "cand_skills.panel_index", "F0331:2495-2497");
    check_u16_has(result.attributes_after[1],
                  FS_DM1_V1_CTS_ATTR_PANEL_PC34, "cand_skills.attr",
                  "F0331:2495-2497");
    check_u16_lacks(result.attributes_after[0],
                    FS_DM1_V1_CTS_ATTR_PANEL_PC34, "cand_skills.cand_no",
                    "F0331:2335");
    check_int_eq(result.candidate_skipped, 1, "cand_skills.cand_skip",
                 "F0331:2335");
}

static void test_rejects(void)
{
    FsDm1V1CtsTickResultPc34 result;
    memset(&result, 0, sizeof(result));
    fs_dm1_v1_cts_run_clock_tick_pc34(NULL, &result);
    check_int_eq(result.valid, 0, "reject.null_input", "F0331 input");
}

static void test_byte_stability_stat_attr_is_additive(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
        input.champions[i].attributes = FS_DM1_V1_CTS_ATTR_STATISTICS_PC34;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_eq(result.attributes_after[i],
                     FS_DM1_V1_CTS_ATTR_STATISTICS_PC34,
                     "stat_idempotent", "F0331 M008_SET OR");
    }
}

static void test_empty_party_no_dirty(void)
{
    FsDm1V1CtsTickInputPc34 input;
    FsDm1V1CtsTickResultPc34 result;
    fs_dm1_v1_cts_input_init_pc34(&input);
    input.party_champion_count = 0;
    input.inventory_champion_ordinal = 0;
    input.candidate_champion_ordinal = 0;
    input.panel_content = 0;
    input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        input.champions[i].direction = 0;
        input.champions[i].party_is_resting = 0;
        input.champions[i].attributes = 0xFFFFu;
    }

    fs_dm1_v1_cts_run_clock_tick_pc34(&input, &result);
    check_int_eq(result.draw_all_champion_states_invoked, 0,
                 "empty_full.no_f0293", "F0331:2503");
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        check_u16_eq(result.attributes_after[i], 0xFFFFu,
                     "empty_full.attr_intact", "F0331 early return");
    }
}

int main(void)
{
    uint32_t hash;
    FsDm1V1CtsTickInputPc34 probe_input;
    FsDm1V1CtsTickResultPc34 probe_result;

    printf("probe=dm1_v1_champion_panel_clock_tick_stat_repaint_pc34_compat\n");
    printf("%s\n", fs_dm1_v1_cts_source_evidence_pc34());
    test_evidence_strings();
    test_input_init();
    test_empty_party_early_return();
    test_full_party_alive_no_panel();
    test_dead_champion_skipped();
    test_candidate_champion_skipped();
    test_out_of_party_champion_not_visited();
    test_inventory_champion_panel_food_water();
    test_inventory_champion_panel_skills_stats();
    test_panel_not_eligible();
    test_direction_resync_icon();
    test_direction_resync_resting_suppresses();
    test_recovery_period_resting();
    test_f0293_called_once_with_zero_arg();
    test_attributes_byte_stable_outside_attr_set();
    test_time_criteria_deterministic();
    test_alive_directions_matching_no_resync();
    test_party_count_clamps_to_four();
    test_party_count_negative();
    test_candidate_with_inventory_no_panel_attr();
    test_inventory_zero_never_panels();
    test_hash_determinism_two_runs();
    test_byte_stability_pre_attrs();
    test_dead_and_candidate_skip_count();
    test_recovery_period_active_is_256();
    test_direction_quiet_recent_no_resync();
    test_trace_index_count();
    test_panel_food_water_panel_skills();
    test_dead_zero_stamina();
    test_candidate_with_inventory_skills_panel();
    test_byte_stability_stat_attr_is_additive();
    test_empty_party_no_dirty();
    test_rejects();
    /* Compute the deterministic probe hash for the canonical full-party
     * 4-champion alive input (the same shape the related gates use). */
    fs_dm1_v1_cts_input_init_pc34(&probe_input);
    probe_input.party_champion_count = 4;
    probe_input.inventory_champion_ordinal = 0;
    probe_input.candidate_champion_ordinal = 0;
    probe_input.panel_content = 0;
    probe_input.party_direction = 0;
    for (int i = 0; i < FS_DM1_V1_CTS_CHAMPION_COUNT_PC34; ++i) {
        probe_input.champions[i].direction = 0;
        probe_input.champions[i].party_is_resting = 0;
    }
    fs_dm1_v1_cts_run_clock_tick_pc34(&probe_input, &probe_result);
    hash = probe_result.hash;
    if (g_failures || g_assertions < 200) {
        printf("FAIL assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures, hash);
        return 1;
    }
    printf("PASS test_dm1_v1_champion_panel_clock_tick_stat_repaint_pc34_compat "
           "assertions=%d failures=0 hash=0x%08X\n", g_assertions, hash);
    return 0;
}
