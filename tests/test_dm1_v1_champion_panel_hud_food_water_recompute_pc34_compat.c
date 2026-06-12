#include "firestaff/dm1/v1/champion_panel/hud_food_water_recompute_pc34_compat.h"

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
    }
}

static void expect_contains(const char *label, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    }
}

static void expect_not_contains(const char *label, const char *haystack,
                                const char *needle, const char *anchor)
{
    ++g_assertions;
    if (haystack && needle && strstr(haystack, needle) != NULL) {
        ++g_failures;
        printf("FAIL %s unexpectedly contains=\"%s\" anchor=%s\n",
               label, needle, anchor);
    }
}

static fs_dm1_v1_hfw_clock_input_pc34_t make_clock_input(void)
{
    fs_dm1_v1_hfw_clock_input_pc34_t input;

    input.game_time = 256;
    input.resting = 0;
    input.alive = 1;
    input.candidate_ordinal = 0;
    input.champion_index = 1;
    input.inventory_champion_ordinal = 2;
    input.panel_content = FS_DM1_V1_HFW_PANEL_FOOD_WATER_POISONED_PC34;
    input.pressing_mouth = 0;
    input.pressing_eye = 0;
    return input;
}

static void expect_box(const char *label,
                       const fs_dm1_v1_hfw_box_pc34_t *box,
                       int left,
                       int top,
                       int right,
                       int bottom,
                       int width,
                       int height,
                       int zone,
                       const char *anchor)
{
    char field[128];

    snprintf(field, sizeof(field), "%s.left", label);
    expect_int(field, box->left, left, anchor);
    snprintf(field, sizeof(field), "%s.top", label);
    expect_int(field, box->top, top, anchor);
    snprintf(field, sizeof(field), "%s.right", label);
    expect_int(field, box->right, right, anchor);
    snprintf(field, sizeof(field), "%s.bottom", label);
    expect_int(field, box->bottom, bottom, anchor);
    snprintf(field, sizeof(field), "%s.width", label);
    expect_int(field, box->width, width, anchor);
    snprintf(field, sizeof(field), "%s.height", label);
    expect_int(field, box->height, height, anchor);
    snprintf(field, sizeof(field), "%s.zone", label);
    expect_int(field, box->zone, zone, anchor);
}

static void test_source_evidence_and_disjoint_contract(void)
{
    const char *source = fs_dm1_v1_hfw_source_evidence_pc34();

    expect_contains("source.f0331_clock", source,
                    "CHAMPION.C F0331:2331-2332",
                    "clock path bit pattern");
    expect_contains("source.f0331_recovery", source,
                    "CHAMPION.C F0331:2450-2473",
                    "rest-modulated statistic recovery");
    expect_contains("source.f0331_dirty", source,
                    "CHAMPION.C F0331:2482-2496",
                    "clock dirty-to-F0293 route");
    expect_contains("source.f0292_status_box", source,
                    "CHAMDRAW.C F0292:771-815",
                    "status-box fill/status cascade");
    expect_contains("source.f0292_tuple", source,
                    "CHAMDRAW.C F0292:1060-1091",
                    "panel/action hand/action icon tuple");
    expect_contains("source.f0355_hook", source,
                    "PANEL.C F0355:2299-2322",
                    "inventory close repaint hook");
    expect_contains("source.f0345_food_water", source,
                    "PANEL.C F0345:1579-1615",
                    "food/water panel draw");
    expect_contains("source.defs_c30", source, "DEFS.H:780-817 C30",
                    "slot/chest constants");
    expect_contains("source.defs_m070", source, "1878 M070",
                    "hand-slot macro anchor");
    expect_contains("source.defs_globals", source,
                    "5700 G0305, 5876-5881 G0423/G0425/G0426",
                    "global symbol anchors");

    expect_not_contains("disjoint.f0393", source, "F0393",
                        "spell_area_overlay slice excluded");
    expect_not_contains("disjoint.f0394", source, "F0394",
                        "spell_area_overlay slice excluded");
    expect_not_contains("disjoint.f0397", source, "F0397",
                        "spell_area_overlay slice excluded");
    expect_not_contains("disjoint.f0398", source, "F0398",
                        "spell_area_overlay slice excluded");
    expect_not_contains("disjoint.damage_indicator", source,
                        "damage indicator",
                        "damage-indicator slice excluded");
}

static void test_clock_time_criteria_pattern(void)
{
    uint32_t game_time;
    const char *anchor =
        "CHAMPION.C F0331:2331-2332 "
        "(((GT&0x80)+((GT&0x100)>>2)+((GT&0x40)<<2))>>2)";

    for (game_time = 0; game_time < 512; ++game_time) {
        char label[96];
        const int want = (int)((((game_time & 0x0080u) +
                                ((game_time & 0x0100u) >> 2)) +
                               ((game_time & 0x0040u) << 2)) >> 2);
        const int got = fs_dm1_v1_hfw_clock_time_criteria_pc34(game_time);

        snprintf(label, sizeof(label), "time_criteria.%03u", (unsigned)game_time);
        expect_int(label, got, want, anchor);
        snprintf(label, sizeof(label), "time_criteria.lowbits.%03u",
                 (unsigned)game_time);
        expect_int(label, got & 15, 0, anchor);
    }

    expect_int("time_criteria.000", fs_dm1_v1_hfw_clock_time_criteria_pc34(0),
               0, anchor);
    expect_int("time_criteria.064", fs_dm1_v1_hfw_clock_time_criteria_pc34(64),
               64, anchor);
    expect_int("time_criteria.128", fs_dm1_v1_hfw_clock_time_criteria_pc34(128),
               32, anchor);
    expect_int("time_criteria.192", fs_dm1_v1_hfw_clock_time_criteria_pc34(192),
               96, anchor);
    expect_int("time_criteria.256", fs_dm1_v1_hfw_clock_time_criteria_pc34(256),
               16, anchor);
    expect_int("time_criteria.320", fs_dm1_v1_hfw_clock_time_criteria_pc34(320),
               80, anchor);
    expect_int("time_criteria.384", fs_dm1_v1_hfw_clock_time_criteria_pc34(384),
               48, anchor);
    expect_int("time_criteria.448", fs_dm1_v1_hfw_clock_time_criteria_pc34(448),
               112, anchor);
}

static void test_rest_modulated_recovery_cadence(void)
{
    uint32_t game_time;
    const char *anchor =
        "CHAMPION.C F0331:2450-2473 active mask 255, resting mask 63";

    expect_int("recovery_period.active",
               fs_dm1_v1_hfw_recovery_period_pc34(0), 256, anchor);
    expect_int("recovery_period.resting",
               fs_dm1_v1_hfw_recovery_period_pc34(1), 64, anchor);

    for (game_time = 0; game_time < 512; ++game_time) {
        char label[96];
        const int active_due = (game_time & 255u) == 0u;
        const int resting_due = (game_time & 63u) == 0u;

        snprintf(label, sizeof(label), "recovery.active.%03u",
                 (unsigned)game_time);
        expect_int(label, fs_dm1_v1_hfw_recovery_due_pc34(game_time, 0),
                   active_due, anchor);
        snprintf(label, sizeof(label), "recovery.resting.%03u",
                 (unsigned)game_time);
        expect_int(label, fs_dm1_v1_hfw_recovery_due_pc34(game_time, 1),
                   resting_due, anchor);
    }
}

static void test_statistic_recovery_clamp(void)
{
    int current;
    const char *anchor =
        "CHAMPION.C F0331:2466-2473 current<max increments; "
        "current>max decrements by current/max";

    for (current = 0; current <= 30; ++current) {
        char label[96];
        int want = current;

        if (current < 10) {
            want = current + 1;
        } else if (current > 10) {
            want = current - (current / 10);
        }
        snprintf(label, sizeof(label), "recover_stat.max10.%02d", current);
        expect_int(label, fs_dm1_v1_hfw_recover_stat_pc34(current, 10),
                   want, anchor);
    }

    expect_int("recover_stat.equal",
               fs_dm1_v1_hfw_recover_stat_pc34(42, 42), 42, anchor);
    expect_int("recover_stat.clamp.99_25",
               fs_dm1_v1_hfw_recover_stat_pc34(99, 25), 96, anchor);
}

static void test_status_box_and_hand_slot_geometry(void)
{
    int champion;
    const char *status_anchor =
        "CHAMDRAW.C F0292:771-789 with DEFS.H:3783-3786 C151..C154";
    const char *hand_anchor =
        "DATA.C G0030 slot boxes: champion*69+4/24, y=10, 18x18; "
        "DEFS.H:1878 M070";

    for (champion = 0; champion < FS_DM1_V1_HFW_CHAMPION_COUNT_PC34;
         ++champion) {
        char label[96];
        fs_dm1_v1_hfw_box_pc34_t status =
            fs_dm1_v1_hfw_status_box_pc34(champion);
        fs_dm1_v1_hfw_box_pc34_t ready =
            fs_dm1_v1_hfw_hand_slot_box_pc34(champion, 0);
        fs_dm1_v1_hfw_box_pc34_t action =
            fs_dm1_v1_hfw_hand_slot_box_pc34(champion, 1);
        const int base = champion * FS_DM1_V1_HFW_STATUS_BOX_STRIDE_PC34;

        snprintf(label, sizeof(label), "status_box.champion%d", champion);
        expect_box(label, &status, base, 0, base + 66, 28, 67, 29,
                   151 + champion, status_anchor);
        snprintf(label, sizeof(label), "ready_hand.champion%d", champion);
        expect_box(label, &ready, base + 4, 10, base + 21, 27, 18, 18,
                   0, hand_anchor);
        snprintf(label, sizeof(label), "action_hand.champion%d", champion);
        expect_box(label, &action, base + 24, 10, base + 41, 27, 18, 18,
                   1, hand_anchor);
    }

    expect_int("zone.first", fs_dm1_v1_hfw_status_zone_pc34(0), 151,
               status_anchor);
    expect_int("zone.last", fs_dm1_v1_hfw_status_zone_pc34(3), 154,
               status_anchor);
    expect_int("slot.ready", FS_DM1_V1_HFW_SLOT_READY_HAND_PC34, 0,
               "DEFS.H:780 C00_SLOT_READY_HAND");
    expect_int("slot.action", FS_DM1_V1_HFW_SLOT_ACTION_HAND_PC34, 1,
               "DEFS.H:781 C01_SLOT_ACTION_HAND");
    expect_int("slot.chest_first", FS_DM1_V1_HFW_SLOT_CHEST_1_PC34, 30,
               "DEFS.H:810 C30_SLOT_CHEST_1");
    expect_int("slot.chest_last", FS_DM1_V1_HFW_SLOT_CHEST_8_PC34, 37,
               "DEFS.H:817 C37_SLOT_CHEST_8");
}

static void test_clock_to_draw_state_recompute_gate(void)
{
    fs_dm1_v1_hfw_clock_input_pc34_t input = make_clock_input();
    fs_dm1_v1_hfw_clock_result_pc34_t result =
        fs_dm1_v1_hfw_clock_probe_pc34(&input);
    const char *clock_anchor =
        "CHAMPION.C F0331:2482-2496 STATISTICS/PANEL then F0293";
    const char *status_anchor =
        "PANEL.C F0355:2316-2322 STATUS_BOX close hook; "
        "CHAMDRAW.C F0292:771-815 status-box cascade";
    const char *tuple_anchor =
        "CHAMDRAW.C F0292:898-935 bars then F0292:1060-1091 "
        "action-hand/action-icon tuple";

    expect_int("clock.valid", result.valid, 1, clock_anchor);
    expect_u16("clock.dirty", result.f0331_dirty_attributes,
               FS_DM1_V1_HFW_ATTR_STATISTICS_PC34 |
               FS_DM1_V1_HFW_ATTR_PANEL_PC34,
               clock_anchor);
    expect_int("clock.no_status_box_dirty",
               result.f0331_dirty_attributes &
                   FS_DM1_V1_HFW_ATTR_STATUS_BOX_PC34,
               0, "F0331 does not claim PANEL.C F0355 STATUS_BOX hook");
    expect_int("clock.draw_all_states", result.draw_all_champion_states, 1,
               "CHAMPION.C F0331:2496 F0293_CHAMPION_DrawAllChampionStates");
    expect_int("clock.statistics_repainted",
               result.statistics_repainted_by_clock, 1,
               "CHAMDRAW.C F0292:898-935 statistics redraw");
    expect_int("clock.panel_repainted",
               result.panel_repainted_for_food_water, 1,
               "CHAMPION.C F0331:2485-2490 food/water panel dirty");
    expect_int("clock.mouth_warning",
               result.mouth_warning_recomputed, 1,
               "CHAMDRAW.C F0292:908-918 mouth warning");
    expect_int("clock.eye_warning",
               result.eye_warning_recomputed, 1,
               "CHAMDRAW.C F0292:920-933 eye warning");
    expect_u16("close_hook.status_box_dirty",
               result.f0355_close_dirty_attributes,
               FS_DM1_V1_HFW_ATTR_STATUS_BOX_PC34,
               status_anchor);
    expect_int("close_hook.status_box_repainted",
               result.status_box_repainted_by_close_hook, 1,
               status_anchor);
    expect_int("tuple.order_mask", result.tuple_order_mask,
               FS_DM1_V1_HFW_ORDER_FULL_TUPLE_PC34,
               tuple_anchor);

    expect_box("result.status_box", &result.status_box, 69, 0, 135, 28,
               67, 29, 152, status_anchor);
    expect_box("result.ready_hand", &result.ready_hand_box, 73, 10, 90, 27,
               18, 18, 0, tuple_anchor);
    expect_box("result.action_hand", &result.action_hand_box, 93, 10, 110, 27,
               18, 18, 1, tuple_anchor);

    input.panel_content = 0;
    input.pressing_mouth = 1;
    result = fs_dm1_v1_hfw_clock_probe_pc34(&input);
    expect_u16("pressing_mouth.panel_dirty",
               result.f0331_dirty_attributes,
               FS_DM1_V1_HFW_ATTR_STATISTICS_PC34 |
               FS_DM1_V1_HFW_ATTR_PANEL_PC34,
               "CHAMPION.C F0331:2485 pressing mouth panel route");
    expect_int("pressing_mouth.food_panel_flag",
               result.panel_repainted_for_food_water, 0,
               "pressing-mouth route is disjoint from food/water panel content");

    input = make_clock_input();
    input.inventory_champion_ordinal = 1;
    result = fs_dm1_v1_hfw_clock_probe_pc34(&input);
    expect_u16("non_inventory_clock.stats_only",
               result.f0331_dirty_attributes,
               FS_DM1_V1_HFW_ATTR_STATISTICS_PC34,
               "CHAMPION.C F0331:2483 inventory ordinal gate");
    expect_u16("non_inventory_close_hook.none",
               result.f0355_close_dirty_attributes, 0,
               "PANEL.C F0355 hook is only for current inventory champion");
    expect_int("non_inventory.mouth_warning", result.mouth_warning_recomputed,
               0, "CHAMDRAW.C F0292:906 inventory champion gate");

    input = make_clock_input();
    input.candidate_ordinal = 2;
    result = fs_dm1_v1_hfw_clock_probe_pc34(&input);
    expect_int("candidate.invalid", result.valid, 0,
               "CHAMPION.C F0331:2334 skips candidate champion ordinal");
    expect_u16("candidate.no_dirty", result.f0331_dirty_attributes, 0,
               "candidate skip prevents dirty attributes");

    input = make_clock_input();
    input.alive = 0;
    result = fs_dm1_v1_hfw_clock_probe_pc34(&input);
    expect_int("dead.invalid", result.valid, 0,
               "CHAMPION.C F0331:2334 requires CurrentHealth");
    expect_u16("dead.no_dirty", result.f0331_dirty_attributes, 0,
               "dead champion skip prevents dirty attributes");
}

int main(void)
{
    test_source_evidence_and_disjoint_contract();
    test_clock_time_criteria_pattern();
    test_rest_modulated_recovery_cadence();
    test_statistic_recovery_clamp();
    test_status_box_and_hand_slot_geometry();
    test_clock_to_draw_state_recompute_gate();

    if (g_assertions < 200) {
        ++g_failures;
        printf("FAIL assertion_floor got=%d want>=200 anchor=task contract\n",
               g_assertions);
    }

    if (g_failures) {
        printf("FAIL dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions,
               g_failures);
        return 1;
    }

    printf("PASS dm1_v1_champion_panel_hud_food_water_recompute_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}
