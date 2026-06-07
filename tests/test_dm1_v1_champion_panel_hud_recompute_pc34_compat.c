#include "dm1_v1_champion_panel_hud_recompute_pc34_compat.h"

#include <stdio.h>

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

static void expect_call(const char *id,
                        int champion_index,
                        int poison_state,
                        int food_state,
                        int water_state,
                        int want_valid,
                        int want_hand,
                        int want_icon,
                        int want_bars,
                        const char *anchor)
{
    char name[96];
    int hand = -7;
    int icon = -7;
    int bars = -7;
    int valid = dm1_v1_champion_panel_hud_recompute(champion_index,
                                                    poison_state,
                                                    food_state,
                                                    water_state,
                                                    &hand,
                                                    &icon,
                                                    &bars);

    snprintf(name, sizeof(name), "%s.valid", id);
    expect_int(name, valid, want_valid, anchor);
    snprintf(name, sizeof(name), "%s.redraw_action_hand", id);
    expect_int(name, hand, want_hand, anchor);
    snprintf(name, sizeof(name), "%s.redraw_action_icon", id);
    expect_int(name, icon, want_icon, anchor);
    snprintf(name, sizeof(name), "%s.redraw_bars", id);
    expect_int(name, bars, want_bars, anchor);
}

static void expect_order(const char *id,
                         int champion_index,
                         int poison_state,
                         int food_state,
                         int water_state,
                         int want_packed,
                         const char *anchor)
{
    int hand = -7;
    int icon = -7;
    int bars = -7;
    int valid = dm1_v1_champion_panel_hud_recompute(champion_index,
                                                    poison_state,
                                                    food_state,
                                                    water_state,
                                                    &hand,
                                                    &icon,
                                                    &bars);
    int packed = (hand << 2) | (icon << 1) | bars;

    (void)valid;
    expect_int(id, packed, want_packed, anchor);
}

static void test_invalid_rejection(void)
{
    const char *champion_anchor =
        "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 four champion slots";
    const char *state_anchor =
        "PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:1945-1949 "
        "bounded food/water/poison trigger state";

    expect_call("invalid_champion.minus_two", -2, 0, 0, 0, 0, 0, 0, 0,
                champion_anchor);
    expect_call("invalid_champion.minus_one", -1, 0, 0, 0, 0, 0, 0, 0,
                champion_anchor);
    expect_call("invalid_champion.four", 4, 0, 0, 0, 0, 0, 0, 0,
                champion_anchor);
    expect_call("invalid_champion.five", 5, 0, 0, 0, 0, 0, 0, 0,
                champion_anchor);

    expect_call("invalid_poison.minus_one", 0, -1, 0, 0, 0, 0, 0, 0,
                state_anchor);
    expect_call("invalid_poison.three", 0, 3, 0, 0, 0, 0, 0, 0,
                state_anchor);
    expect_call("invalid_food.minus_one", 0, 0, -1, 0, 0, 0, 0, 0,
                state_anchor);
    expect_call("invalid_food.three", 0, 0, 3, 0, 0, 0, 0, 0,
                state_anchor);
    expect_call("invalid_water.minus_one", 0, 0, 0, -1, 0, 0, 0, 0,
                state_anchor);
    expect_call("invalid_water.three", 0, 0, 0, 3, 0, 0, 0, 0,
                state_anchor);
}

static void test_source_locked_dispatch_sequence(void)
{
    const char *full_anchor =
        "PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:1945-1949 "
        "sets STATISTICS/PANEL then calls F0292";
    const char *bars_anchor =
        "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 recomputes bars and "
        "food/water/poison warning";
    const char *icon_anchor =
        "CHAMDRAW.C F0292_CHAMPION_DrawState:1060-1091 panel/action-hand "
        "and action-icon partial redraw";
    const char *no_overlap_anchor =
        "CHAMDRAW.C F0292_CHAMPION_DrawState:1080-1091 partial redraw is "
        "not used when poison/food/water stayed unchanged";

    expect_call("valid_initial.full_panel", 0, 0, 0, 0, 1, 1, 1, 1,
                full_anchor);
    expect_order("valid_initial.order_hand_icon_bars", 0, 0, 0, 1, 7,
                 "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 and "
                 "F0292_CHAMPION_DrawState:1060-1091 redraw-bit order");

    expect_call("idempotent_after_order.noop", 0, 0, 0, 1, 1, 0, 0, 0,
                no_overlap_anchor);
    expect_order("idempotent_after_order.order_zero", 0, 0, 0, 1, 0,
                 no_overlap_anchor);

    expect_call("food_only.redraw_all_bits", 0, 0, 1, 1, 1, 1, 1, 1,
                bars_anchor);
    expect_order("food_only.order_hand_icon_bars", 0, 0, 1, 2, 7,
                 "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 food change "
                 "precedes redraw tuple");

    expect_call("water_only.redraw_all_bits", 0, 0, 1, 0, 1, 1, 1, 1,
                bars_anchor);
    expect_order("water_only.order_hand_icon_bars", 0, 0, 2, 0, 7,
                 "CHAMDRAW.C F0292_CHAMPION_DrawState:898-935 water change "
                 "precedes redraw tuple");

    expect_call("poison_only.redraw_icon", 0, 1, 2, 0, 1, 1, 1, 1,
                icon_anchor);
    expect_order("poison_only.order_hand_icon_bars", 0, 2, 2, 0, 7,
                 "CHAMDRAW.C F0292_CHAMPION_DrawState:1060-1091 poison "
                 "warning/action icon route");

    expect_call("champion_change.full_panel", 1, 2, 2, 0, 1, 1, 1, 1,
                full_anchor);
    expect_order("champion_change.order_hand_icon_bars", 2, 2, 2, 0, 7,
                 "PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:"
                 "1945-1949 champion-index full panel route");

    expect_call("idempotent_same_inputs.noop", 2, 2, 2, 0, 1, 0, 0, 0,
                no_overlap_anchor);
    expect_order("idempotent_same_inputs.order_zero", 2, 2, 2, 0, 0,
                 no_overlap_anchor);

    expect_call("non_overlap_stable_warning.no_partial", 2, 2, 2, 0, 1, 0, 0, 0,
                no_overlap_anchor);
    expect_order("non_overlap_stable_warning.order_zero", 2, 2, 2, 0, 0,
                 no_overlap_anchor);
}

int main(void)
{
    test_invalid_rejection();
    test_source_locked_dispatch_sequence();

    if (g_failures) {
        printf("FAIL dm1_v1_champion_panel_hud_recompute_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions,
               g_failures);
        return 1;
    }

    printf("PASS dm1_v1_champion_panel_hud_recompute_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}
