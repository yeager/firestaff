#include "dm1/dm1_v1_champion_panel_mouth_eye_poison_warning_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d expected=%d anchor=%s\n",
                label, actual, expected, anchor);
    }
}

static void check_true(const char *label, int value, const char *anchor)
{
    check_int(label, value ? 1 : 0, 1, anchor);
}

static void check_contains(const char *label, const char *text,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!text || !strstr(text, needle)) {
        ++g_failures;
        fprintf(stderr, "FAIL %s missing='%s' anchor=%s\n",
                label, needle, anchor);
    }
}

static void set_all_stats(
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat *state,
    int champion_index, int current, int maximum)
{
    int statistic_index;

    for (statistic_index = 0;
         statistic_index < DM1_V1_CPMEP_STATISTIC_COUNT;
         ++statistic_index) {
        state->champions[champion_index].statistic_current[statistic_index] =
            current;
        state->champions[champion_index].statistic_maximum[statistic_index] =
            maximum;
    }
}

static DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat build_or_fail(
    const DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat *state,
    const char *anchor)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    if (!DM1_V1_ChampionPanelMouthEyePoisonWarning_BuildPc34Compat(
            state, &result)) {
        memset(&result, 0, sizeof(result));
        ++g_assertions;
        ++g_failures;
        fprintf(stderr, "FAIL build_or_fail anchor=%s\n", anchor);
    } else {
        ++g_assertions;
    }
    return result;
}

static void test_closed_inventory_suppresses_warning_borders(const char *overlay)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 0;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 1;
    state.champions[1].food = -700;
    state.champions[1].water = -700;
    state.champions[1].poison_event_count = 1;
    set_all_stats(&state, 1, 49, 50);

    result = build_or_fail(&state, overlay);
    check_int("closed.selectedLeader", result.selected_champion_index, 0, overlay);
    check_int("closed.noMouthBorder", result.mouth_border_drawn, 0, overlay);
    check_int("closed.noEyeBorder", result.eye_border_drawn, 0, overlay);
    check_int("closed.noPanel", result.panel_drawn, 0, overlay);
    check_int("closed.noLeaderSwitch",
              result.leader_index_switched_to_inventory_champion, 0, overlay);
}

static void test_empty_mouth_eye_state(const char *overlay,
                                       const char *panel,
                                       const char *defs)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;

    result = build_or_fail(&state, overlay);
    check_int("empty.selected", result.selected_champion_index, 0, overlay);
    check_int("empty.mouthZone", result.mouth_zone, DM1_V1_CPMEP_ZONE_MOUTH, defs);
    check_int("empty.mouthNormal", result.mouth_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_NORMAL, overlay);
    check_int("empty.mouthNoWarning", result.mouth_warning_border_drawn, 0,
              overlay);
    check_int("empty.eyeZone", result.eye_zone, DM1_V1_CPMEP_ZONE_EYE, defs);
    check_int("empty.eyeNormal", result.eye_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_NORMAL, overlay);
    check_int("empty.eyeNoWarning", result.eye_warning_border_drawn, 0, overlay);
    check_int("empty.foodNoFlash", result.food_warning_border_flashes, 0, panel);
    check_int("empty.waterNoFlash", result.water_warning_border_flashes, 0,
              panel);
    check_int("empty.poisonNoFlash", result.poison_warning_border_flashes, 0,
              panel);
}

static void test_all_critical_simultaneously(const char *overlay,
                                             const char *panel,
                                             const char *defs)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 1;
    state.champions[1].food = -700;
    state.champions[1].water = -800;
    state.champions[1].poison_event_count = 2;
    set_all_stats(&state, 1, 49, 50);

    result = build_or_fail(&state, overlay);
    check_int("all.selectedInventoryChampion", result.selected_champion_index, 1,
              overlay);
    check_int("all.leaderSwitch",
              result.leader_index_switched_to_inventory_champion, 1, overlay);
    check_int("all.mouthWounded", result.mouth_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_WOUNDED, overlay);
    check_int("all.eyeWounded", result.eye_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_WOUNDED, overlay);
    check_int("all.firstLowStat", result.first_low_statistic_index, 0,
              overlay);
    check_int("all.foodFlash", result.food_warning_border_flashes, 1, panel);
    check_int("all.foodFlashDark", result.food_warning_palette_dark,
              DM1_V1_CPMEP_COLOR_RED_FLASH_DARK, defs);
    check_int("all.foodFlashLit", result.food_warning_palette_lit,
              DM1_V1_CPMEP_COLOR_RED_FLASH_LIT, defs);
    check_int("all.waterFlash", result.water_warning_border_flashes, 1, panel);
    check_int("all.poisonLabel", result.poison_label_drawn, 1, panel);
    check_int("all.poisonFlash", result.poison_warning_border_flashes, 1,
              panel);
}

static void test_only_food_critical(const char *overlay,
                                    const char *panel,
                                    const char *defs)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.champions[0].food = -700;

    result = build_or_fail(&state, overlay);
    check_int("food.mouthWarning", result.mouth_warning_border_drawn, 1,
              overlay);
    check_int("food.eyeNormal", result.eye_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_NORMAL, overlay);
    check_int("food.barRed", result.food_bar_color, 8, panel);
    check_int("food.flash", result.food_warning_border_flashes, 1, panel);
    check_int("food.flashDark", result.food_warning_palette_dark,
              DM1_V1_CPMEP_COLOR_RED_FLASH_DARK, defs);
    check_int("food.waterNoFlash", result.water_warning_border_flashes, 0,
              panel);
    check_int("food.poisonNoLabel", result.poison_label_drawn, 0, panel);
}

static void test_only_water_critical(const char *overlay,
                                     const char *panel,
                                     const char *defs)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.champions[0].water = -700;

    result = build_or_fail(&state, overlay);
    check_int("water.mouthWarning", result.mouth_warning_border_drawn, 1,
              overlay);
    check_int("water.eyeNormal", result.eye_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_NORMAL, overlay);
    check_int("water.foodNoFlash", result.food_warning_border_flashes, 0,
              panel);
    check_int("water.barRed", result.water_bar_color, 8, panel);
    check_int("water.flash", result.water_warning_border_flashes, 1, panel);
    check_int("water.flashLit", result.water_warning_palette_lit,
              DM1_V1_CPMEP_COLOR_RED_FLASH_LIT, defs);
    check_int("water.poisonNoLabel", result.poison_label_drawn, 0, panel);
}

static void test_only_poison_active(const char *overlay,
                                    const char *panel,
                                    const char *defs)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.champions[0].poison_event_count = 1;

    result = build_or_fail(&state, overlay);
    check_int("poison.mouthWarning", result.mouth_warning_border_drawn, 1,
              overlay);
    check_int("poison.eyeNormal", result.eye_border_graphic,
              DM1_V1_CPMEP_GFX_SLOT_NORMAL, overlay);
    check_int("poison.foodNoFlash", result.food_warning_border_flashes, 0,
              panel);
    check_int("poison.waterNoFlash", result.water_warning_border_flashes, 0,
              panel);
    check_int("poison.labelGraphic", result.poison_label_graphic,
              DM1_V1_CPMEP_GFX_POISONED_LABEL, defs);
    check_int("poison.labelDrawn", result.poison_label_drawn, 1, panel);
    check_int("poison.flash", result.poison_warning_border_flashes, 1, panel);
    check_int("poison.flashDark", result.poison_warning_palette_dark,
              DM1_V1_CPMEP_COLOR_RED_FLASH_DARK, defs);
}

/* Boundary value test for one champion at the exact ReDMCSB food/water
 * warning threshold.
 *
 * ReDMCSB PANEL.C:F0344_INVENTORY_DrawPanel_FoodOrWaterBar:1519-1526 uses
 *   if (P0712_i_Amount < -512) {
 *       L1070_i_Color = C08_COLOR_RED;
 *   } else {
 *       if (P0712_i_Amount < 0) {
 *           L1070_i_Color = C11_COLOR_YELLOW;
 *       } else {
 *           L1070_i_Color = caller_normal_color;
 *       }
 *   }
 *
 * and the panel slice mirrors that with
 *   food_critical = champion->food < -512;
 *   water_critical = champion->water < -512;
 *   dm1_v1_cpmep_food_or_water_color(amount, normal_color) returning
 *       RED for amount < -512, YELLOW for amount < 0, else normal_color.
 *
 * The test below pins the boundary on a SINGLE champion (champion 0) and
 * asserts both sides:
 *   - at food = -512 / water = -512: the strict-less-than means the bar
 *     color is YELLOW (not RED) and the warning border does NOT flash
 *     (no critical message emission).
 *   - at food = -513 / water = -513: just past the threshold, the bar
 *     color is RED and the warning border flashes the red palette pair
 *     (critical message emission per ReDMCSB).
 *   - at food = 0: the warning level drops back to normal
 *     bar color, mouth warning clears for the food axis only.
 *
 * The test only touches champion 0, keeping the assertion surface focused
 * on the threshold itself instead of multi-champion routing.
 */
static void test_food_water_warning_threshold_boundary(const char *overlay,
                                                      const char *panel,
                                                      const char *defs)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    /* Food = -512: strict-less-than keeps us out of the critical branch. */
    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 0;
    state.champions[0].food = -512;
    state.champions[0].water = 1024;
    state.champions[0].poison_event_count = 0;

    result = build_or_fail(&state, overlay);
    check_int("boundary.food_minus_512.selected",
              result.selected_champion_index, 0, overlay);
    check_int("boundary.food_minus_512.mouthWarning",
              result.mouth_warning_border_drawn, 1, overlay);
    check_int("boundary.food_minus_512.foodBarYellow",
              result.food_bar_color, 11, panel);
    check_int("boundary.food_minus_512.waterBarNormal",
              result.water_bar_color, 14, panel);
    check_int("boundary.food_minus_512.foodNoFlash",
              result.food_warning_border_flashes, 0, panel);
    check_int("boundary.food_minus_512.foodNoDarkPalette",
              result.food_warning_palette_dark, -1, panel);
    check_int("boundary.food_minus_512.foodNoLitPalette",
              result.food_warning_palette_lit, -1, panel);
    check_int("boundary.food_minus_512.waterNoFlash",
              result.water_warning_border_flashes, 0, panel);
    check_int("boundary.food_minus_512.poisonNoLabel",
              result.poison_label_drawn, 0, panel);

    /* Food = -513: one step past the boundary, critical warning message
     * emission (red bar + red flash palette pair). */
    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 0;
    state.champions[0].food = -513;
    state.champions[0].water = 1024;
    state.champions[0].poison_event_count = 0;

    result = build_or_fail(&state, overlay);
    check_int("boundary.food_minus_513.selected",
              result.selected_champion_index, 0, overlay);
    check_int("boundary.food_minus_513.foodBarRed",
              result.food_bar_color, 8, panel);
    check_int("boundary.food_minus_513.foodFlash",
              result.food_warning_border_flashes, 1, panel);
    check_int("boundary.food_minus_513.foodFlashDark",
              result.food_warning_palette_dark,
              DM1_V1_CPMEP_COLOR_RED_FLASH_DARK, defs);
    check_int("boundary.food_minus_513.foodFlashLit",
              result.food_warning_palette_lit,
              DM1_V1_CPMEP_COLOR_RED_FLASH_LIT, defs);
    check_int("boundary.food_minus_513.waterNoFlash",
              result.water_warning_border_flashes, 0, panel);

    /* Water = -512: boundary is symmetric, water stays in yellow band. */
    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 0;
    state.champions[0].food = 1024;
    state.champions[0].water = -512;
    state.champions[0].poison_event_count = 0;

    result = build_or_fail(&state, overlay);
    check_int("boundary.water_minus_512.mouthWarning",
              result.mouth_warning_border_drawn, 1, overlay);
    check_int("boundary.water_minus_512.foodBarNormal",
              result.food_bar_color, 5, panel);
    check_int("boundary.water_minus_512.waterBarYellow",
              result.water_bar_color, 11, panel);
    check_int("boundary.water_minus_512.foodNoFlash",
              result.food_warning_border_flashes, 0, panel);
    check_int("boundary.water_minus_512.waterNoFlash",
              result.water_warning_border_flashes, 0, panel);

    /* Water = -513: one step past the boundary, water axis critical. */
    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 0;
    state.champions[0].food = 1024;
    state.champions[0].water = -513;
    state.champions[0].poison_event_count = 0;

    result = build_or_fail(&state, overlay);
    check_int("boundary.water_minus_513.waterBarRed",
              result.water_bar_color, 8, panel);
    check_int("boundary.water_minus_513.waterFlash",
              result.water_warning_border_flashes, 1, panel);
    check_int("boundary.water_minus_513.waterFlashDark",
              result.water_warning_palette_dark,
              DM1_V1_CPMEP_COLOR_RED_FLASH_DARK, defs);
    check_int("boundary.water_minus_513.foodNoFlash",
              result.food_warning_border_flashes, 0, panel);

    /* Food = 0: lower boundary for the warning band itself. The bar
     * returns to the caller's normal color and the food warning
     * emission drops out, while the water axis is untouched. */
    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 0;
    state.champions[0].food = 0;
    state.champions[0].water = 1024;
    state.champions[0].poison_event_count = 0;

    result = build_or_fail(&state, overlay);
    check_int("boundary.food_zero.foodBarNormal",
              result.food_bar_color, 5, panel);
    check_int("boundary.food_zero.mouthNoWarning",
              result.mouth_warning_border_drawn, 0, overlay);
    check_int("boundary.food_zero.foodNoFlash",
              result.food_warning_border_flashes, 0, panel);
}

static void test_warning_follows_inventory_champion(const char *overlay,
                                                    const char *swap,
                                                    const char *panel)
{
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat state;
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat result;

    DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(&state);
    state.inventory_open = 1;
    state.leader_champion_index = 0;
    state.active_inventory_champion_index = 2;
    state.champions[0].food = 1024;
    state.champions[0].water = 1024;
    state.champions[0].poison_event_count = 0;
    state.champions[2].food = -700;
    state.champions[2].water = 1024;
    state.champions[2].poison_event_count = 0;

    result = build_or_fail(&state, swap);
    check_int("swap.selectedActiveInventory", result.selected_champion_index, 2,
              swap);
    check_int("swap.leaderSwitch",
              result.leader_index_switched_to_inventory_champion, 1, swap);
    check_int("swap.followsInventory",
              result.follows_inventory_champion_not_party_leader, 1, swap);
    check_int("swap.mouthWarningFromInventoryChampion",
              result.mouth_warning_border_drawn, 1, overlay);
    check_int("swap.foodFlashFromInventoryChampion",
              result.food_warning_border_flashes, 1, panel);
}

int main(void)
{
    const DM1_V1_ChampionPanelMouthEyePoisonWarningEvidencePc34Compat *e;

    e = DM1_V1_ChampionPanelMouthEyePoisonWarning_EvidencePc34Compat();
    check_true("evidence.contractOnly", e && e->contract_only,
               "CHAMDRAW.C:F0292_CHAMPION_DrawState:898-935");
    check_contains("evidence.overlay", e->inventory_overlay_anchor,
                   "CHAMDRAW.C:F0292_CHAMPION_DrawState:898-935",
                   "CHAMDRAW.C:F0292_CHAMPION_DrawState:898-935");
    check_contains("evidence.panel", e->panel_warning_anchor,
                   "PANEL.C:F0345_INVENTORY_DrawPanel_FoodWaterPoisoned:1579-1615",
                   "PANEL.C:F0345_INVENTORY_DrawPanel_FoodWaterPoisoned:1579-1615");
    check_contains("evidence.swap", e->inventory_swap_anchor,
                   "PANEL.C:F0355_INVENTORY_Toggle_CPSE:2299-2363",
                   "PANEL.C:F0355_INVENTORY_Toggle_CPSE:2299-2363");
    check_contains("evidence.video", e->video_primitive_anchor,
                   "BLITMASK.C:F0133_VIDEO_BlitBoxFilledWithMaskedBitmap:30-72",
                   "BLITMASK.C:F0133_VIDEO_BlitBoxFilledWithMaskedBitmap:30-72");

    test_closed_inventory_suppresses_warning_borders(e->inventory_overlay_anchor);
    test_empty_mouth_eye_state(e->inventory_overlay_anchor,
                               e->panel_warning_anchor,
                               e->defs_anchor);
    test_all_critical_simultaneously(e->inventory_overlay_anchor,
                                     e->panel_warning_anchor,
                                     e->defs_anchor);
    test_only_food_critical(e->inventory_overlay_anchor,
                            e->panel_warning_anchor,
                            e->defs_anchor);
    test_only_water_critical(e->inventory_overlay_anchor,
                             e->panel_warning_anchor,
                             e->defs_anchor);
    test_only_poison_active(e->inventory_overlay_anchor,
                            e->panel_warning_anchor,
                            e->defs_anchor);
    test_food_water_warning_threshold_boundary(e->inventory_overlay_anchor,
                                               e->panel_warning_anchor,
                                               e->defs_anchor);
    test_warning_follows_inventory_champion(e->inventory_overlay_anchor,
                                            e->inventory_swap_anchor,
                                            e->panel_warning_anchor);

    printf("sourceEvidence.overlay=%s\n", e->inventory_overlay_anchor);
    printf("sourceEvidence.panel=%s\n", e->panel_warning_anchor);
    printf("sourceEvidence.swap=%s\n", e->inventory_swap_anchor);
    printf("sourceEvidence.video=%s\n", e->video_primitive_anchor);
    printf("sourceEvidence.defs=%s\n", e->defs_anchor);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);

    return g_failures == 0 ? 0 : 1;
}
