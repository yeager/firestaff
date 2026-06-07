#include "dm1_v1_champion_panel_hud_recompute_pc34_compat.h"

typedef struct DM1V1ChampionPanelHudRecomputeStatePc34Compat {
    int initialized;
    int champion_index;
    int poison_state;
    int food_state;
    int water_state;
} DM1V1ChampionPanelHudRecomputeStatePc34Compat;

static DM1V1ChampionPanelHudRecomputeStatePc34Compat g_last_state = {
    0, -1, -1, -1, -1
};

static int valid_champion_index(int champion_index)
{
    return champion_index >= 0 && champion_index <= 3;
}

static int valid_three_state_value(int state)
{
    return state >= 0 && state <= 2;
}

int dm1_v1_champion_panel_hud_recompute(int champion_index,
                                         int poison_state,
                                         int food_state,
                                         int water_state,
                                         int *redraw_action_hand,
                                         int *redraw_action_icon,
                                         int *redraw_bars)
{
    int champion_changed;
    int poison_changed;
    int food_changed;
    int water_changed;
    int redraw;

    if (redraw_action_hand) {
        *redraw_action_hand = 0;
    }
    if (redraw_action_icon) {
        *redraw_action_icon = 0;
    }
    if (redraw_bars) {
        *redraw_bars = 0;
    }

    if (!redraw_action_hand || !redraw_action_icon || !redraw_bars) {
        return 0;
    }
    if (!valid_champion_index(champion_index) ||
        !valid_three_state_value(poison_state) ||
        !valid_three_state_value(food_state) ||
        !valid_three_state_value(water_state)) {
        return 0;
    }

    champion_changed = !g_last_state.initialized ||
                       champion_index != g_last_state.champion_index;
    poison_changed = !g_last_state.initialized ||
                     poison_state != g_last_state.poison_state;
    food_changed = !g_last_state.initialized ||
                   food_state != g_last_state.food_state;
    water_changed = !g_last_state.initialized ||
                    water_state != g_last_state.water_state;

    /*
     * ReDMCSB CHAMDRAW.C F0292:898-935 redraws bars and the mouth warning
     * from STATISTICS. PANEL.C F0349:1945-1949 is the panel trigger that
     * re-enters F0292 after mouth food/water changes. CHAMDRAW.C
     * F0292:1060-1091 keeps the panel/action-hand/icon route distinct from
     * the no-change path, so repeated identical input is intentionally quiet.
     */
    redraw = champion_changed || poison_changed || food_changed || water_changed;
    if (redraw) {
        *redraw_action_hand = 1;
        *redraw_action_icon = 1;
        *redraw_bars = 1;
    }

    g_last_state.initialized = 1;
    g_last_state.champion_index = champion_index;
    g_last_state.poison_state = poison_state;
    g_last_state.food_state = food_state;
    g_last_state.water_state = water_state;

    return 1;
}
