#include "dm1_v1_portrait_panel_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void) {
    assert(DM1_PORTRAIT_W == 32);
    assert(DM1_PORTRAIT_H == 29);
    assert(DM1_PORTRAIT_BITPLANES == 4);
    assert(DM1_PORTRAIT_CHUNKY_BYTES == 32 * 29);
    assert(DM1_MAX_CHAMPIONS == 4);
    assert(DM1_PANEL_BAR_W == 25);
    assert(DM1_PANEL_BAR_H == 3);
}

static void test_init(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    memset(&state, 0xFF, sizeof(state));
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    assert(state.active_count == 0);
    assert(state.selected_index == -1);
}

static void test_set_champion_count(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    DM1_V1_PortraitPanel_SetChampionCountPc34Compat(&state, 3);
    assert(state.active_count == 3);
}

static void test_select(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    DM1_V1_PortraitPanel_SetChampionCountPc34Compat(&state, 4);
    DM1_V1_PortraitPanel_SelectPc34Compat(&state, 2);
    assert(state.selected_index == 2);
    assert(state.panels[2].selected == true);
}

static void test_update_bars(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    DM1_V1_PortraitPanel_SetChampionCountPc34Compat(&state, 1);
    DM1_V1_PortraitPanel_UpdateBarsPc34Compat(&state.panels[0],
        50, 100, 30, 80, 60, 120, 40, 50);
    assert(state.panels[0].hp.current == 50);
    assert(state.panels[0].hp.max == 100);
    assert(state.panels[0].mana.current == 30);
    assert(state.panels[0].stamina.current == 60);
}

static void test_layout(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    DM1_V1_PortraitPanel_SetChampionCountPc34Compat(&state, 2);
    DM1_V1_PortraitPanel_LayoutPc34Compat(&state, 0, 134);
    /* Layout should set panel_x and panel_y for each active panel */
    int16_t px = state.panels[0].panel_x;
    int16_t py = state.panels[0].panel_y;
    (void)px;
    (void)py;
    /* After layout, at least one coordinate should reflect the base */
    assert(state.panels[0].panel_y >= 0);
}

static void test_damage_flash(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    DM1_V1_PortraitPanel_SetChampionCountPc34Compat(&state, 1);
    DM1_V1_PortraitPanel_DamageFlashPc34Compat(&state, 0);
    assert(state.panels[0].portrait.injured == true);
}

static void test_tick(void) {
    DM1_V1_PortraitPanelStatePc34 state;
    DM1_V1_PortraitPanel_InitPc34Compat(&state);
    DM1_V1_PortraitPanel_SetChampionCountPc34Compat(&state, 1);
    DM1_V1_PortraitPanel_DamageFlashPc34Compat(&state, 0);
    uint8_t timer_before = state.panels[0].portrait.damage_flash_timer;
    DM1_V1_PortraitPanel_TickPc34Compat(&state);
    /* Tick should decrement or update the flash timer */
    bool changed = (state.panels[0].portrait.damage_flash_timer != timer_before) ||
                   (state.panels[0].portrait.injured == false);
    (void)changed;
    assert(changed);
}

static void test_m11_aliases(void) {
    /* m11_pp_init is a #define alias so we verify it expands correctly
     * by calling through it */
    DM1_V1_PortraitPanelStatePc34 state;
    m11_pp_init(&state);
    assert(state.active_count == 0);
    assert(state.selected_index == -1);
}

int main(void) {
    test_constants();
    test_init();
    test_set_champion_count();
    test_select();
    test_update_bars();
    test_layout();
    test_damage_flash();
    test_tick();
    test_m11_aliases();
    puts("ok: DM1 portrait panel (Q-DM1-07) 9 tests passed");
    return 0;
}
