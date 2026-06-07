#include "dm1_v1_champion_panel_status_recompute_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_u16(const char *id, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%04X want=0x%04X at %s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == 0x%04X (%s)\n", id, (unsigned)want, anchor);
    }
}

static void expect_u32(const char *id, uint32_t got, uint32_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%08X want=0x%08X at %s\n",
               id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == 0x%08X (%s)\n", id, (unsigned)want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == \"%s\" (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static dm1_v1_champion_panel_status_recompute_pc34_compat_input_t
make_sequence_input(void)
{
    dm1_v1_champion_panel_status_recompute_pc34_compat_input_t input;

    memset(&input, 0, sizeof(input));
    input.initial_state.champion_index = 0;
    input.initial_state.current_health = 100;
    input.initial_state.maximum_health = 100;
    input.initial_state.current_stamina = 600;
    input.initial_state.maximum_stamina = 1000;
    input.initial_state.food = 256;
    input.initial_state.water = 1024;
    input.initial_state.poison_event_count = 0;
    input.initial_state.action_hand =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_HAND_EMPTY_PC34;
    input.initial_state.panel_content =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_PANEL_FOOD_WATER_POISONED_PC34;
    input.step_count = 7;
    input.steps[0].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HEALTH_PC34;
    input.steps[0].value = 72;
    input.steps[1].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_STAMINA_PC34;
    input.steps[1].value = 440;
    input.steps[2].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_FOOD_PC34;
    input.steps[2].value = -16;
    input.steps[3].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_WATER_PC34;
    input.steps[3].value = -700;
    input.steps[4].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_POISON_PC34;
    input.steps[4].value = 1;
    input.steps[5].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HAND_PC34;
    input.steps[5].value = 1;
    input.steps[6].change =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_CHANGE_HAND_PC34;
    input.steps[6].value = 0;

    return input;
}

static void test_evidence_and_invariants(void)
{
    const dm1_v1_champion_panel_status_recompute_pc34_compat_evidence_t *evidence =
        dm1_v1_champion_panel_status_recompute_pc34_compat_evidence();
    dm1_v1_champion_panel_status_recompute_pc34_compat_result_t result =
        dm1_v1_champion_panel_status_recompute_pc34_compat_run(NULL);
    const char *source =
        dm1_v1_champion_panel_status_recompute_pc34_compat_source_evidence();

    expect_bool("invariant.contract_only", result.invariant.contract_only, true,
                "PANEL.C F0349:1945-1949 contract-only recompute gate");
    expect_bool("invariant.no_graphics_dat", result.invariant.loads_graphics_dat, false,
                "PANEL.C F0345:1597-1615 no GRAPHICS.DAT load");
    expect_bool("invariant.no_dungeon_dat", result.invariant.loads_dungeon_dat, false,
                "PANEL.C F0347:1658-1691 no DUNGEON.DAT load");
    expect_bool("invariant.synthetic_state",
                result.invariant.uses_synthetic_champion_state, true,
                "PANEL.C F0349:1832-1949 synthetic champion mutation sequence");
    expect_bool("invariant.recompute_only",
                result.invariant.covers_status_recompute_only, true,
                "CHAMDRAW.C F0292:898-935 bounded status recompute");
    expect_bool("invariant.draw_state_after_dirty",
                result.invariant.calls_draw_state_after_dirty_flags, true,
                "PANEL.C F0349:1945-1949 dirty flags before F0292");
    expect_bool("invariant.statistics_path",
                result.invariant.statistics_dirty_recomputes_status_bars_and_mouth_border, true,
                "CHAMDRAW.C F0292:898-935 statistics path");
    expect_bool("invariant.panel_path",
                result.invariant.panel_dirty_recomputes_food_water_poison_panel, true,
                "PANEL.C F0345:1563-1616 food/water/poison path");
    expect_bool("invariant.action_hand_path",
                result.invariant.action_hand_dirty_recomputes_action_slot_only, true,
                "CHAMDRAW.C F0292:1080-1091 action-hand path");
    expect_bool("invariant.no_status_box_dirty",
                result.invariant.does_not_dirty_status_box, true,
                "PANEL.C F0349:1945-1949 no MASK0x1000_STATUS_BOX");
    expect_int("invariant.max_steps", result.invariant.max_steps, 8,
               "synthetic sequence bounded to eight recompute steps");

    expect_str_eq("evidence.status_anchor", evidence->status_recompute_anchor,
                  "PANEL.C F0349:1945-1949 and CHAMDRAW.C F0292:898-935",
                  "PANEL.C F0349 plus CHAMDRAW.C F0292 anchor");
    expect_str_eq("evidence.food_bar_anchor", evidence->food_water_bar_anchor,
                  "PANEL.C F0344:1493-1561",
                  "PANEL.C F0344 food/water bar source");
    expect_str_eq("evidence.food_panel_anchor", evidence->food_water_panel_anchor,
                  "PANEL.C F0345:1563-1616",
                  "PANEL.C F0345 food/water/poison source");
    expect_str_eq("evidence.hand_anchor", evidence->hand_dispatch_anchor,
                  "PANEL.C F0347:1639-1691",
                  "PANEL.C F0347 action-hand dispatch source");
    expect_contains("source.f0349", source, "PANEL.C F0349:1832-1949",
                    "PANEL.C F0349 source evidence string");
    expect_contains("source.f0344", source, "amount < -512",
                    "PANEL.C F0344 color threshold evidence");
    expect_contains("source.f0292", source, "CHAMDRAW.C F0292:898-935",
                    "CHAMDRAW.C F0292 status recompute evidence");
    expect_bool("null_input.defaults", result.null_input_defaults_used, true,
                "synthetic default state accepted");
}

static void expect_step(
    const char *id,
    const dm1_v1_champion_panel_status_recompute_pc34_compat_step_result_t *step,
    uint16_t dirty,
    uint32_t visuals,
    bool panel_requested,
    const char *anchor)
{
    char buffer[96];

    snprintf(buffer, sizeof(buffer), "%s.dirty", id);
    expect_u16(buffer, step->dirty_attributes, dirty, anchor);
    snprintf(buffer, sizeof(buffer), "%s.changed_visuals", id);
    expect_u32(buffer, step->changed_visuals, visuals, anchor);
    snprintf(buffer, sizeof(buffer), "%s.draw_state", id);
    expect_bool(buffer, step->draw_state_called, true, anchor);
    snprintf(buffer, sizeof(buffer), "%s.panel_requested", id);
    expect_bool(buffer, step->panel_recompute_requested, panel_requested, anchor);
    snprintf(buffer, sizeof(buffer), "%s.status_box", id);
    expect_bool(buffer, step->status_box_recompute_requested, false,
                "PANEL.C F0349:1945-1949 no status-box dirty for stat recompute");
    snprintf(buffer, sizeof(buffer), "%s.unrelated_visuals", id);
    expect_bool(buffer, step->unrelated_visuals_changed, false,
                "status recompute does not alter name/load/wounds/status box");
}

static void test_status_recompute_sequence_exact_deltas(void)
{
    dm1_v1_champion_panel_status_recompute_pc34_compat_input_t input =
        make_sequence_input();
    dm1_v1_champion_panel_status_recompute_pc34_compat_result_t result =
        dm1_v1_champion_panel_status_recompute_pc34_compat_run(&input);
    const uint16_t statistics =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATISTICS_PC34;
    const uint16_t statistics_panel =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_STATISTICS_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_PANEL_PC34;
    const uint16_t panel_action =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_PANEL_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_ATTR_ACTION_HAND_PC34;
    const uint32_t hand_panel_delta =
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HAND_PANEL_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_ACTION_HAND_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_BAR_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_COLOR_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_BAR_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_COLOR_PC34 |
        DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_POISON_LABEL_PC34;

    expect_int("sequence.step_count", result.step_count, 7,
               "synthetic HP/stamina/food/water/poison/hand sequence");
    expect_bool("sequence.no_clamp", result.rejected_overlarge_step_count, false,
                "sequence within bounded step count");

    expect_step("health", &result.steps[0], statistics,
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HP_BAR_PC34 |
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_HP_VALUE_PC34,
                false,
                "CHAMDRAW.C F0292:898-907 HP bar/value recompute");
    expect_int("health.hp_height", result.steps[0].after.hp_bar_height, 18,
               "CHAMDRAW.C F0287 fixed-point 72/100 bar height");

    expect_step("stamina", &result.steps[1], statistics,
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STAMINA_BAR_PC34 |
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_STAMINA_VALUE_PC34,
                false,
                "CHAMDRAW.C F0292:898-907 stamina bar/value recompute");
    expect_int("stamina.height", result.steps[1].after.stamina_bar_height, 11,
               "CHAMDRAW.C F0287 fixed-point 440/1000 bar height");
    expect_int("stamina.value", result.steps[1].after.stamina_value, 44,
               "CHAMDRAW.C F0290 stamina display divides by 10");

    expect_step("food", &result.steps[2], statistics_panel,
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_MOUTH_BORDER_PC34 |
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_BAR_PC34 |
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_FOOD_COLOR_PC34,
                true,
                "PANEL.C F0349:1945-1949 plus F0344:1519-1525 food recompute");
    expect_int("food.color", result.steps[2].after.food_color,
               DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_YELLOW_PC34,
               "PANEL.C F0344:1522-1525 food below zero is yellow");
    expect_int("food.mouth", result.steps[2].after.mouth_border_graphic,
               DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_GFX_SLOT_WOUNDED_PC34,
               "CHAMDRAW.C F0292:908-918 negative food wounds mouth border");

    expect_step("water", &result.steps[3], statistics_panel,
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_BAR_PC34 |
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_WATER_COLOR_PC34,
                true,
                "PANEL.C F0344:1519-1525 water color recompute");
    expect_int("water.color", result.steps[3].after.water_color,
               DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_COLOR_RED_PC34,
               "PANEL.C F0344:1519-1521 water below -512 is red");

    expect_step("poison", &result.steps[4], statistics_panel,
                DM1_V1_CHAMPION_PANEL_STATUS_RECOMPUTE_VISUAL_POISON_LABEL_PC34,
                true,
                "PANEL.C F0345:1601-1606 poison label recompute");
    expect_bool("poison.visible", result.steps[4].after.poison_label_visible, true,
                "PANEL.C F0345:1601 PoisonEventCount gates label");

    expect_step("hand_object", &result.steps[5], panel_action,
                hand_panel_delta, true,
                "PANEL.C F0347:1658-1691 object action hand switches panel");
    expect_step("hand_empty", &result.steps[6], panel_action,
                hand_panel_delta, true,
                "PANEL.C F0347:1688-1691 empty action hand restores food panel");
    expect_bool("hand_object.no_statistics",
                result.steps[5].statistics_recompute_requested, false,
                "CHAMDRAW.C F0292:1080-1091 action hand path is not statistics");
    expect_bool("hand_empty.no_statistics",
                result.steps[6].statistics_recompute_requested, false,
                "CHAMDRAW.C F0292:1080-1091 action hand path is not statistics");
}

static void test_determinism_and_bounds(void)
{
    dm1_v1_champion_panel_status_recompute_pc34_compat_input_t input =
        make_sequence_input();
    dm1_v1_champion_panel_status_recompute_pc34_compat_result_t first =
        dm1_v1_champion_panel_status_recompute_pc34_compat_run(&input);
    dm1_v1_champion_panel_status_recompute_pc34_compat_result_t second =
        dm1_v1_champion_panel_status_recompute_pc34_compat_run(&input);
    dm1_v1_champion_panel_status_recompute_pc34_compat_input_t overlarge =
        make_sequence_input();
    dm1_v1_champion_panel_status_recompute_pc34_compat_result_t clamped;

    overlarge.step_count = 12;
    clamped =
        dm1_v1_champion_panel_status_recompute_pc34_compat_run(&overlarge);

    expect_u32("determinism.hash", first.determinism_hash,
               second.determinism_hash,
               "same synthetic recompute sequence yields same visual hash");
    expect_u32("determinism.step4", first.steps[4].changed_visuals,
               second.steps[4].changed_visuals,
               "PANEL.C F0345:1601-1606 poison label deterministic");
    expect_int("bounds.clamped_count", clamped.step_count, 8,
               "bounded synthetic runner clamps invalid step count");
    expect_bool("bounds.rejected", clamped.rejected_overlarge_step_count, true,
                "overlarge step sequence rejected");
}

int main(void)
{
    test_evidence_and_invariants();
    test_status_recompute_sequence_exact_deltas();
    test_determinism_and_bounds();

    if (g_failures) {
        printf("FAIL dm1_v1_champion_panel_status_recompute_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_champion_panel_status_recompute_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
