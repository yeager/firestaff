#include "dm2_v1_hud_tables.h"
#include <assert.h>
#include <stdio.h>

static void test_button_desc_count(void)
{
    assert(DM2_V1_HUD_BUTTON_COUNT == 62);
    assert(dm2_v1_hud_button_desc[0].gdat_category == 0x0002);
    assert(dm2_v1_hud_button_desc[0].button_id == 0x0000);
    assert(dm2_v1_hud_button_desc[0].action_type == 0x00);
}

static void test_button_desc_champion_panels(void)
{
    assert(dm2_v1_hud_button_desc[7].gdat_category == 0x009c);
    assert(dm2_v1_hud_button_desc[7].click_target == -1);
    assert(dm2_v1_hud_button_desc[7].action_type == 0x0e);
    assert(dm2_v1_hud_button_desc[10].gdat_category == 0x009d);
    assert(dm2_v1_hud_button_desc[10].action_type == 0x0f);
}

static void test_button_desc_inventory(void)
{
    for (int i = 30; i <= 33; i++) {
        assert(dm2_v1_hud_button_desc[i].gdat_category == 0x000b);
        assert(dm2_v1_hud_button_desc[i].action_type == 0x0d);
    }
    for (int i = 34; i <= 44; i++) {
        assert(dm2_v1_hud_button_desc[i].gdat_category == 0x000b);
    }
}

static void test_clickmap(void)
{
    assert(DM2_V1_HUD_CLICKMAP_COUNT == 83);
    assert(dm2_v1_hud_clickmap[0] == (int8_t)0x80);
    assert(dm2_v1_hud_clickmap[82] == (int8_t)0x80);
    assert(dm2_v1_hud_clickmap[5] == 0x06);
    assert(dm2_v1_hud_clickmap[6] == 0x07);
}

static void test_panel_layout(void)
{
    assert(DM2_V1_HUD_PANEL_COUNT == 76);
    assert(dm2_v1_hud_panel_layout[0].flags == 0x00);
    assert(dm2_v1_hud_panel_layout[0].rect_id == 0x0000);
    assert(dm2_v1_hud_panel_layout[8].flags == (int8_t)0x83);
    assert(dm2_v1_hud_panel_layout[8].param == 0x00);
    assert(dm2_v1_hud_panel_layout[8].rect_id == 0x0004);
    assert(dm2_v1_hud_panel_layout[75].rect_id == 0x0018);
}

static void test_action_icons(void)
{
    assert(DM2_V1_HUD_ACTION_ICON_COUNT == 10);
    assert(dm2_v1_hud_action_icons[0].gdat_flag == (int8_t)0x80);
    assert(dm2_v1_hud_action_icons[0].icon_id == 0x0000);
    assert(dm2_v1_hud_action_icons[4].gdat_flag == (int8_t)0x81);
    assert(dm2_v1_hud_action_icons[4].icon_id == 0x0012);
    assert(dm2_v1_hud_action_icons[9].icon_id == 0x0009);
}

int main(void)
{
    test_button_desc_count();
    test_button_desc_champion_panels();
    test_button_desc_inventory();
    test_clickmap();
    test_panel_layout();
    test_action_icons();
    assert(dm2_v1_hud_tables_source_evidence() != NULL);
    printf("All dm2_v1_hud_tables tests passed.\n");
    return 0;
}
