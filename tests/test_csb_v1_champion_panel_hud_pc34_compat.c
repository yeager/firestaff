/*
 * test_csb_v1_champion_panel_hud_pc34_compat.c
 *
 * Comprehensive test for the CSB champion panel HUD module.
 * Source-locked to ReDMCSB_WIP20210206 CHAMDRAW.C/INVNTORY.C.
 */
#include "csb_v1_champion_panel_hud_pc34_compat.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_champion_colors(void)
{
    assert(CSB_ChampionColor[0] == 7);
    assert(CSB_ChampionColor[1] == 11);
    assert(CSB_ChampionColor[2] == 8);
    assert(CSB_ChampionColor[3] == 14);
    printf("  champion_colors OK\n");
}

static void test_bar_graph_height(void)
{
    assert(CSB_ChampionPanel_BarGraphHeight(0, 100, 0) == 0);
    assert(CSB_ChampionPanel_BarGraphHeight(100, 100, 0) == 25);
    assert(CSB_ChampionPanel_BarGraphHeight(50, 100, 0) == 13);
    assert(CSB_ChampionPanel_BarGraphHeight(1, 100, 0) == 1);
    assert(CSB_ChampionPanel_BarGraphHeight(1, 25, 0) == 1);
    assert(CSB_ChampionPanel_BarGraphHeight(25, 25, 0) == 25);
    assert(CSB_ChampionPanel_BarGraphHeight(0, 0, 0) == 0);
    assert(CSB_ChampionPanel_BarGraphHeight(150, 100, 1) == 25);
    assert(CSB_ChampionPanel_BarGraphHeight(-5, 100, 0) == 0);
    printf("  bar_graph_height OK\n");
}

static void test_bar_fill_model(void)
{
    CSB_ChampionPanel_BarFillModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildPc34BarFillModel(0, 0, 50, 100, &m));
    assert(m.zoneId == 195);
    assert(m.width == 4);
    assert(m.height == 25);
    assert(m.blankColor == CSB_COLOR_DARKEST_GRAY);
    assert(m.fillColor == CSB_ChampionColor[0]);
    assert(m.emitsBlank == 1);
    assert(m.emitsFill == 1);
    assert(m.fillHeight > 0);
    assert(m.blankHeight + m.fillHeight == 25);

    assert(CSB_ChampionPanel_BuildPc34BarFillModel(0, 0, 100, 100, &m));
    assert(m.emitsBlank == 0);
    assert(m.emitsFill == 1);
    assert(m.fillHeight == 25);

    assert(CSB_ChampionPanel_BuildPc34BarFillModel(0, 0, 0, 100, &m));
    assert(m.emitsBlank == 1);
    assert(m.emitsFill == 0);
    assert(m.blankHeight == 25);

    assert(CSB_ChampionPanel_BuildPc34BarFillModel(1, 1, 50, 100, &m));
    assert(m.zoneId == 195 + 1 + 4);
    assert(m.fillColor == CSB_ChampionColor[1]);

    assert(CSB_ChampionPanel_BuildPc34BarFillModel(3, 2, 50, 100, &m));
    assert(m.zoneId == 195 + 3 + 8);
    assert(m.fillColor == CSB_ChampionColor[3]);

    assert(!CSB_ChampionPanel_BuildPc34BarFillModel(-1, 0, 50, 100, &m));
    assert(!CSB_ChampionPanel_BuildPc34BarFillModel(4, 0, 50, 100, &m));
    assert(!CSB_ChampionPanel_BuildPc34BarFillModel(0, 3, 50, 100, &m));
    assert(!CSB_ChampionPanel_BuildPc34BarFillModel(0, 0, 50, 0, &m));
    assert(!CSB_ChampionPanel_BuildPc34BarFillModel(0, 0, 50, 100, NULL));

    printf("  bar_fill_model OK\n");
}

static void test_bar_graph_screen_xy(void)
{
    int x, y;
    CSB_ChampionPanel_BarGraphScreenXY(0, 0, &x, &y);
    assert(x == 46 && y == 2);
    CSB_ChampionPanel_BarGraphScreenXY(0, 1, &x, &y);
    assert(x == 53);
    CSB_ChampionPanel_BarGraphScreenXY(0, 2, &x, &y);
    assert(x == 60);
    CSB_ChampionPanel_BarGraphScreenXY(1, 0, &x, &y);
    assert(x == 46 + 69);
    CSB_ChampionPanel_BarGraphScreenXY(3, 2, &x, &y);
    assert(x == 46 + 3 * 69 + 14);
    printf("  bar_graph_screen_xy OK\n");
}

static void test_status_box_model_alive(void)
{
    CSB_ChampionPanel_StatusBoxModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildStatusBoxModel(0, 0, 0, 100, &m));
    assert(m.drawKind == CSB_STATUS_BOX_DRAW_ALIVE);
    assert(m.fillColor == CSB_COLOR_DARKEST_GRAY);
    assert(m.drawPortrait == 0);
    assert(m.propagatedAttributes & CSB_ATTR_NAME_TITLE);
    assert(m.propagatedAttributes & CSB_ATTR_STATISTICS);
    assert(m.propagatedAttributes & CSB_ATTR_WOUNDS);
    assert(m.propagatedAttributes & CSB_ATTR_ACTION_HAND);
    printf("  status_box_model_alive OK\n");
}

static void test_status_box_model_inventory(void)
{
    CSB_ChampionPanel_StatusBoxModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildStatusBoxModel(2, 0, 1, 100, &m));
    assert(m.drawKind == CSB_STATUS_BOX_DRAW_ALIVE);
    assert(m.drawPortrait == 1);
    assert(m.propagatedAttributes == CSB_ATTR_STATISTICS);
    printf("  status_box_model_inventory OK\n");
}

static void test_status_box_model_dead(void)
{
    CSB_ChampionPanel_StatusBoxModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildStatusBoxModel(1, 0, 0, 0, &m));
    assert(m.drawKind == CSB_STATUS_BOX_DRAW_DEAD);
    assert(m.graphicId == CSB_GFX_DEAD_CHAMPION);
    assert(m.nameColor == CSB_COLOR_LIGHTEST_GRAY);
    assert(m.nameBackgroundColor == CSB_COLOR_DARK_GRAY);
    assert(m.drawActionIcon == 1);
    assert(m.stopAfterDead == 1);
    printf("  status_box_model_dead OK\n");
}

static void test_icon_bitmap_model(void)
{
    CSB_ChampionPanel_IconBitmapModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildIconBitmapModel(0, 0, 0, &m));
    assert(m.width == 19);
    assert(m.height == 14);
    assert(m.fillColor == CSB_ChampionColor[0]);
    assert(m.graphicId == CSB_GFX_CHAMPION_ICONS);
    assert(m.sourceX == 0);
    assert(m.transparentColor == CSB_COLOR_DARKEST_GRAY);

    assert(CSB_ChampionPanel_BuildIconBitmapModel(0, 1, 0, &m));
    assert(m.sourceX == 19);

    assert(CSB_ChampionPanel_BuildIconBitmapModel(0, 0, 1, &m));
    assert(m.sourceX == 3 * 19);

    assert(!CSB_ChampionPanel_BuildIconBitmapModel(-1, 0, 0, &m));
    assert(!CSB_ChampionPanel_BuildIconBitmapModel(4, 0, 0, &m));
    assert(!CSB_ChampionPanel_BuildIconBitmapModel(0, -1, 0, &m));
    assert(!CSB_ChampionPanel_BuildIconBitmapModel(0, 4, 0, &m));
    printf("  icon_bitmap_model OK\n");
}

static void test_slot_box_graphic(void)
{
    assert(CSB_ChampionPanel_SlotBoxGraphic(0, 0x0000, 0) == CSB_GFX_SLOT_NORMAL);
    assert(CSB_ChampionPanel_SlotBoxGraphic(0, 0x0001, 0) == CSB_GFX_SLOT_WOUNDED);
    assert(CSB_ChampionPanel_SlotBoxGraphic(1, 0x0000, 1) == CSB_GFX_SLOT_ACTING);
    assert(CSB_ChampionPanel_SlotBoxGraphic(1, 0x0002, 0) == CSB_GFX_SLOT_WOUNDED);
    assert(CSB_ChampionPanel_SlotBoxGraphic(1, 0x0002, 1) == CSB_GFX_SLOT_ACTING);
    assert(CSB_ChampionPanel_SlotBoxGraphic(6, 0, 0) == -1);
    assert(CSB_ChampionPanel_SlotBoxGraphic(-1, 0, 0) == -1);
    printf("  slot_box_graphic OK\n");
}

static void test_hand_slot_model(void)
{
    CSB_ChampionPanel_StatusHandSlotBoxModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildStatusHandSlotBoxModel(0, 0, 0, &m));
    assert(m.championIndex == 0);
    assert(m.handIndex == 0);
    assert(m.isActionHand == 0);
    assert(m.x == 4);
    assert(m.y == 10);
    assert(m.width == 18);
    assert(m.height == 18);
    assert(m.graphicId == CSB_GFX_SLOT_NORMAL);

    assert(CSB_ChampionPanel_BuildStatusHandSlotBoxModel(0, 1, 1, &m));
    assert(m.isActionHand == 1);
    assert(m.x == 24);
    assert(m.graphicId == CSB_GFX_SLOT_ACTING);

    assert(CSB_ChampionPanel_BuildStatusHandSlotBoxModel(2, 0, 0, &m));
    assert(m.x == 2 * 69 + 4);

    assert(!CSB_ChampionPanel_BuildStatusHandSlotBoxModel(-1, 0, 0, &m));
    assert(!CSB_ChampionPanel_BuildStatusHandSlotBoxModel(4, 0, 0, &m));
    assert(!CSB_ChampionPanel_BuildStatusHandSlotBoxModel(0, 2, 0, &m));
    printf("  hand_slot_model OK\n");
}

static void test_portrait_screen_x(void)
{
    assert(CSB_ChampionPanel_PortraitScreenX(0) == 7);
    assert(CSB_ChampionPanel_PortraitScreenX(1) == 76);
    assert(CSB_ChampionPanel_PortraitScreenX(2) == 145);
    assert(CSB_ChampionPanel_PortraitScreenX(3) == 214);
    assert(CSB_ChampionPanel_PortraitScreenX(-1) == 0);
    assert(CSB_ChampionPanel_PortraitScreenX(4) == 0);
    printf("  portrait_screen_x OK\n");
}

static void test_name_zone_x(void)
{
    assert(CSB_ChampionPanel_NameZoneX(0) == 0);
    assert(CSB_ChampionPanel_NameZoneX(1) == 69);
    assert(CSB_ChampionPanel_NameZoneX(2) == 138);
    assert(CSB_ChampionPanel_NameZoneX(3) == 207);
    printf("  name_zone_x OK\n");
}

static void test_name_color(void)
{
    assert(CSB_ChampionPanel_NameColor(0, 0) == CSB_COLOR_GOLD);
    assert(CSB_ChampionPanel_NameColor(1, 0) == CSB_COLOR_LIGHTEST_GRAY);
    assert(CSB_ChampionPanel_NameColor(2, 2) == CSB_COLOR_GOLD);
    printf("  name_color OK\n");
}

static void test_is_dead(void)
{
    assert(CSB_ChampionPanel_IsDeadStatusBox(0) == 1);
    assert(CSB_ChampionPanel_IsDeadStatusBox(-5) == 1);
    assert(CSB_ChampionPanel_IsDeadStatusBox(1) == 0);
    assert(CSB_ChampionPanel_IsDeadStatusBox(100) == 0);
    printf("  is_dead OK\n");
}

static void test_status_value_zone(void)
{
    assert(CSB_ChampionPanel_StatusValueZone(0) == CSB_ZONE_HEALTH_VALUE);
    assert(CSB_ChampionPanel_StatusValueZone(1) == CSB_ZONE_STAMINA_VALUE);
    assert(CSB_ChampionPanel_StatusValueZone(2) == CSB_ZONE_MANA_VALUE);
    assert(CSB_ChampionPanel_StatusValueZone(3) == -1);
    printf("  status_value_zone OK\n");
}

static void test_format_status_value(void)
{
    char buf[16];
    (void)buf;
    assert(CSB_ChampionPanel_FormatStatusValue(0, 100, 200,
        0, 0, 0, 0, buf, sizeof(buf)));
    assert(strcmp(buf, "100/200") == 0);

    assert(CSB_ChampionPanel_FormatStatusValue(1, 0, 0,
        1500, 2000, 0, 0, buf, sizeof(buf)));
    assert(strcmp(buf, "150/200") == 0);

    assert(CSB_ChampionPanel_FormatStatusValue(2, 0, 0,
        0, 0, 50, 80, buf, sizeof(buf)));
    assert(strcmp(buf, " 50/ 80") == 0);

    assert(!CSB_ChampionPanel_FormatStatusValue(0, 100, 200,
        0, 0, 0, 0, buf, 4));
    assert(!CSB_ChampionPanel_FormatStatusValue(0, 100, 200,
        0, 0, 0, 0, NULL, 16));
    assert(!CSB_ChampionPanel_FormatStatusValue(3, 0, 0,
        0, 0, 0, 0, buf, 16));
    printf("  format_status_value OK\n");
}

static void test_hand_slot_xy(void)
{
    int x, y;
    CSB_ChampionPanel_StatusHandSlotXY(0, 0, &x, &y);
    assert(x == 4 && y == 10);
    CSB_ChampionPanel_StatusHandSlotXY(0, 1, &x, &y);
    assert(x == 24 && y == 10);
    CSB_ChampionPanel_StatusHandSlotXY(1, 0, &x, &y);
    assert(x == 69 + 4);
    CSB_ChampionPanel_StatusHandSlotXY(3, 1, &x, &y);
    assert(x == 3 * 69 + 24);
    printf("  hand_slot_xy OK\n");
}

static void test_source_evidence(void)
{
    const char *ev = CSB_ChampionPanel_SourceEvidence();
    (void)ev;
    assert(ev != NULL);
    assert(strstr(ev, "ReDMCSB") != NULL);
    assert(strstr(ev, "F0287") != NULL);
    assert(strstr(ev, "F0291") != NULL);
    assert(strstr(ev, "F0354") != NULL);
    assert(strstr(ev, "G0046") != NULL);
    printf("  source_evidence OK\n");
}

static void test_format_integer_f0288(void)
{
    char buf[16];
    (void)buf;
    assert(CSB_ChampionPanel_FormatIntegerF0288(42, 1, 3, buf, sizeof(buf)));
    assert(strcmp(buf, " 42") == 0);
    assert(CSB_ChampionPanel_FormatIntegerF0288(7, 1, 3, buf, sizeof(buf)));
    assert(strcmp(buf, "  7") == 0);
    assert(CSB_ChampionPanel_FormatIntegerF0288(123, 1, 3, buf, sizeof(buf)));
    assert(strcmp(buf, "123") == 0);
    assert(CSB_ChampionPanel_FormatIntegerF0288(1234, 1, 3, buf, sizeof(buf)));
    assert(strcmp(buf, "1234") == 0);
    assert(CSB_ChampionPanel_FormatIntegerF0288(5, 0, 3, buf, sizeof(buf)));
    assert(strcmp(buf, "5") == 0);
    assert(!CSB_ChampionPanel_FormatIntegerF0288(42, 1, 3, NULL, 16));
    printf("  format_integer_f0288 OK\n");
}

static void test_inventory_slot_xy(void)
{
    int x, y;
    (void)y;
    (void)x;
    assert(CSB_ChampionPanel_InventorySlotXY(8, &x, &y));
    assert(x == 4 && y == 10);
    assert(CSB_ChampionPanel_InventorySlotXY(9, &x, &y));
    assert(x == 24 && y == 10);
    assert(CSB_ChampionPanel_InventorySlotXY(10, &x, &y));
    assert(x == 62 && y == 10);
    assert(CSB_ChampionPanel_InventorySlotXY(11, &x, &y));
    assert(x == 62 && y == 29);
    assert(CSB_ChampionPanel_InventorySlotXY(13, &x, &y));
    assert(x == 62 && y == 67);
    assert(CSB_ChampionPanel_InventorySlotXY(14, &x, &y));
    assert(x == 98 && y == 10);
    assert(CSB_ChampionPanel_InventorySlotXY(37, &x, &y));
    assert(x == 134 && y == 67);
    assert(!CSB_ChampionPanel_InventorySlotXY(7, &x, &y));
    assert(!CSB_ChampionPanel_InventorySlotXY(38, &x, &y));
    printf("  inventory_slot_xy OK\n");
}

static void test_empty_hand_icon(void)
{
    assert(CSB_ChampionPanel_EmptyHandIconIndex(0, 0) == 212);
    assert(CSB_ChampionPanel_EmptyHandIconIndex(1, 0) == 214);
    assert(CSB_ChampionPanel_EmptyHandIconIndex(0, 0x0001) == 213);
    assert(CSB_ChampionPanel_EmptyHandIconIndex(1, 0x0002) == 215);
    assert(CSB_ChampionPanel_EmptyHandIconIndex(0, 0x0002) == 212);
    assert(CSB_ChampionPanel_EmptyHandIconIndex(-1, 0) == -1);
    assert(CSB_ChampionPanel_EmptyHandIconIndex(2, 0) == -1);
    printf("  empty_hand_icon OK\n");
}

static void test_statistic_colors(void)
{
    assert(CSB_ChampionPanel_StatisticCurrentColor(40, 50) == CSB_COLOR_RED);
    assert(CSB_ChampionPanel_StatisticCurrentColor(50, 50) == CSB_COLOR_LIGHTEST_GRAY);
    assert(CSB_ChampionPanel_StatisticCurrentColor(60, 50) == CSB_COLOR_LIGHT_GREEN);
    assert(CSB_ChampionPanel_StatisticMaximumColor() == CSB_COLOR_LIGHTEST_GRAY);
    printf("  statistic_colors OK\n");
}

static void test_format_statistic_value(void)
{
    char cur[4], mx[5];
    (void)mx;
    (void)cur;
    assert(CSB_ChampionPanel_FormatStatisticValue(42, 50, cur, sizeof(cur), mx, sizeof(mx)));
    assert(strcmp(cur, " 42") == 0);
    assert(strcmp(mx, "/ 50") == 0);
    assert(!CSB_ChampionPanel_FormatStatisticValue(42, 50, NULL, 4, mx, 5));
    printf("  format_statistic_value OK\n");
}

static void test_statistic_row_model(void)
{
    CSB_ChampionPanel_StatisticRowModel row;
    (void)row;
    assert(CSB_ChampionPanel_BuildStatisticRowModel(30, 50, &row));
    assert(row.currentValue == 30);
    assert(row.maximumValue == 50);
    assert(row.currentColor == CSB_COLOR_RED);
    assert(row.maximumColor == CSB_COLOR_LIGHTEST_GRAY);
    assert(strcmp(row.currentText, " 30") == 0);
    assert(strcmp(row.maximumText, "/ 50") == 0);
    assert(!CSB_ChampionPanel_BuildStatisticRowModel(30, 50, NULL));
    printf("  statistic_row_model OK\n");
}

static void test_statistic_text_run_model(void)
{
    CSB_ChampionPanel_StatisticTextRunModel run;
    (void)run;
    assert(CSB_ChampionPanel_BuildStatisticTextRunModel(0, 30, 50, &run));
    assert(run.statisticIndex == 0);
    assert(run.nameZone == CSB_ZONE_SKILL_VALUE);
    assert(run.valueZone == CSB_ZONE_STATISTIC_VALUE);
    assert(run.nameX == 28);
    assert(run.currentX == 94);
    assert(run.maximumX == 94 + 6 * 3);
    assert(run.y == 34);
    assert(run.nameColor == CSB_COLOR_LIGHTEST_GRAY);
    assert(run.currentColor == CSB_COLOR_RED);

    assert(CSB_ChampionPanel_BuildStatisticTextRunModel(3, 50, 50, &run));
    assert(run.y == 34 + 7 * 3);
    assert(run.currentColor == CSB_COLOR_LIGHTEST_GRAY);

    assert(!CSB_ChampionPanel_BuildStatisticTextRunModel(-1, 0, 0, &run));
    assert(!CSB_ChampionPanel_BuildStatisticTextRunModel(6, 0, 0, &run));
    printf("  statistic_text_run_model OK\n");
}

static void test_load_color(void)
{
    assert(CSB_ChampionPanel_LoadColor(50, 100) == CSB_COLOR_LIGHTEST_GRAY);
    assert(CSB_ChampionPanel_LoadColor(101, 100) == CSB_COLOR_RED);
    assert(CSB_ChampionPanel_LoadColor(70, 100) == CSB_COLOR_YELLOW);
    assert(CSB_ChampionPanel_LoadColor(62, 100) == CSB_COLOR_LIGHTEST_GRAY);
    assert(CSB_ChampionPanel_LoadColor(63, 100) == CSB_COLOR_YELLOW);
    assert(CSB_ChampionPanel_LoadColor(10, 0) == CSB_COLOR_RED);
    assert(CSB_ChampionPanel_LoadColor(0, 0) == CSB_COLOR_LIGHTEST_GRAY);
    printf("  load_color OK\n");
}

static void test_format_load_value(void)
{
    char buf[32];
    (void)buf;
    assert(CSB_ChampionPanel_FormatLoadValue(125, 500, buf, sizeof(buf)));
    assert(strcmp(buf, " 12.5/ 50 KG") == 0);
    assert(CSB_ChampionPanel_FormatLoadValue(0, 300, buf, sizeof(buf)));
    assert(strcmp(buf, "  0.0/ 30 KG") == 0);
    assert(CSB_ChampionPanel_LoadValueZone() == 555);
    printf("  format_load_value OK\n");
}

static void test_food_water_poison_blit(void)
{
    const CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec *spec =
        CSB_ChampionPanel_F0658FoodWaterPoisonedBlitSpec_SourceLocked();
    (void)spec;
    assert(spec != NULL);
    assert(spec->blitCount == 3);
    assert(spec->sourceStartLine == 1598);
    assert(spec->sourceEndLine == 1606);
    assert(spec->blits[0].bitmapId == CSB_GFX_FOOD_LABEL);
    assert(spec->blits[0].zoneId == CSB_ZONE_FOOD);
    assert(spec->blits[0].requiresPoisoned == 0);
    assert(spec->blits[1].bitmapId == CSB_GFX_WATER_LABEL);
    assert(spec->blits[2].bitmapId == CSB_GFX_POISONED_LABEL);
    assert(spec->blits[2].requiresPoisoned == 1);
    assert(spec->sourceEvidence != NULL);
    assert(strstr(spec->sourceEvidence, "F0345") != NULL);
    printf("  food_water_poison_blit OK\n");
}

static void test_portrait_blit_model(void)
{
    CSB_ChampionPanel_PortraitBlitModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildPortraitBlitModel(0, &m));
    assert(m.graphicId == CSB_GFX_PORTRAITS);
    assert(m.sourceX == 0);
    assert(m.sourceY == 0);
    assert(m.destX == 7);
    assert(m.destY == CSB_PORTRAIT_Y);
    assert(m.width == CSB_PORTRAIT_WIDTH);
    assert(m.height == CSB_PORTRAIT_HEIGHT);
    assert(m.transparentColor == CSB_COLOR_DARKEST_GRAY);

    assert(CSB_ChampionPanel_BuildPortraitBlitModel(2, &m));
    assert(m.sourceY == 2 * 29);
    assert(m.destX == 2 * 69 + 7);

    assert(CSB_ChampionPanel_BuildPortraitBlitModel(3, &m));
    assert(m.sourceY == 3 * 29);
    assert(m.destX == 3 * 69 + 7);

    assert(!CSB_ChampionPanel_BuildPortraitBlitModel(-1, &m));
    assert(!CSB_ChampionPanel_BuildPortraitBlitModel(4, &m));
    assert(!CSB_ChampionPanel_BuildPortraitBlitModel(0, NULL));
    printf("  portrait_blit_model OK\n");
}

static void test_damage_flash_model(void)
{
    CSB_ChampionPanel_DamageFlashModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildDamageFlashModel(0, 0, &m));
    assert(m.championIndex == 0);
    assert(m.flashColor == 7);
    assert(m.normalColor == CSB_COLOR_DARKEST_GRAY);
    assert(m.flashTickCount == 2);
    assert(m.scheduledAttributes == CSB_ATTR_STATISTICS);
    assert(m.hasNewWounds == 0);

    assert(CSB_ChampionPanel_BuildDamageFlashModel(1, 0x0004, &m));
    assert(m.flashColor == 11);
    assert(m.hasNewWounds == 1);
    assert(m.scheduledAttributes == (CSB_ATTR_STATISTICS | CSB_ATTR_WOUNDS));

    assert(!CSB_ChampionPanel_BuildDamageFlashModel(-1, 0, &m));
    assert(!CSB_ChampionPanel_BuildDamageFlashModel(4, 0, &m));
    assert(!CSB_ChampionPanel_BuildDamageFlashModel(0, 0, NULL));
    printf("  damage_flash_model OK\n");
}

static void test_spell_area_model(void)
{
    CSB_ChampionPanel_SpellAreaModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildSpellAreaModel(&m));
    assert(m.backgroundGraphicId == CSB_GFX_SPELL_AREA);
    assert(m.areaX == 233);
    assert(m.areaY == 42);
    assert(m.areaW == 87);
    assert(m.areaH == 33);
    assert(m.casterZone == 221);
    assert(m.casterCommandId == 109);
    assert(m.runeZones[0] == 245);
    assert(m.runeZones[5] == 250);
    assert(m.runeCommandIds[0] == 101);
    assert(m.runeCommandIds[5] == 106);
    assert(m.castZone == 252);
    assert(m.castCommandId == 108);
    assert(m.recantZone == 254);
    assert(m.recantCommandId == 107);
    assert(!CSB_ChampionPanel_BuildSpellAreaModel(NULL));
    printf("  spell_area_model OK\n");
}

static void test_clock_tick_repaint_model(void)
{
    CSB_ChampionPanel_ClockTickRepaintModel m;
    (void)m;
    assert(CSB_ChampionPanel_BuildClockTickRepaintModel(&m));
    assert(m.repaintMask == CSB_ATTR_STATISTICS);
    assert(m.affectsBarGraphs == 1);
    assert(m.affectsStatValues == 1);
    assert(m.affectsLoadDisplay == 1);
    assert(!CSB_ChampionPanel_BuildClockTickRepaintModel(NULL));
    printf("  clock_tick_repaint_model OK\n");
}

static void test_null_safety(void)
{
    CSB_ChampionPanel_BarFillModel bm;
    CSB_ChampionPanel_StatusBoxModel sm;
    CSB_ChampionPanel_IconBitmapModel im;
    CSB_ChampionPanel_StatusHandSlotBoxModel hm;
    assert(!CSB_ChampionPanel_BuildPc34BarFillModel(0, 0, 50, 100, NULL));
    assert(!CSB_ChampionPanel_BuildStatusBoxModel(0, 0, 0, 100, NULL));
    assert(!CSB_ChampionPanel_BuildIconBitmapModel(0, 0, 0, NULL));
    assert(!CSB_ChampionPanel_BuildStatusHandSlotBoxModel(0, 0, 0, NULL));
    assert(!CSB_ChampionPanel_BuildStatisticRowModel(30, 50, NULL));
    assert(!CSB_ChampionPanel_BuildStatisticTextRunModel(0, 30, 50, NULL));
    assert(!CSB_ChampionPanel_BuildPortraitBlitModel(0, NULL));
    assert(!CSB_ChampionPanel_BuildDamageFlashModel(0, 0, NULL));
    assert(!CSB_ChampionPanel_BuildSpellAreaModel(NULL));
    assert(!CSB_ChampionPanel_BuildClockTickRepaintModel(NULL));
    (void)bm; (void)sm; (void)im; (void)hm;
    printf("  null_safety OK\n");
}

int main(void)
{
    printf("test_csb_v1_champion_panel_hud_pc34_compat:\n");
    test_champion_colors();
    test_bar_graph_height();
    test_bar_fill_model();
    test_bar_graph_screen_xy();
    test_status_box_model_alive();
    test_status_box_model_inventory();
    test_status_box_model_dead();
    test_icon_bitmap_model();
    test_slot_box_graphic();
    test_hand_slot_model();
    test_portrait_screen_x();
    test_name_zone_x();
    test_name_color();
    test_is_dead();
    test_status_value_zone();
    test_format_status_value();
    test_hand_slot_xy();
    test_format_integer_f0288();
    test_inventory_slot_xy();
    test_empty_hand_icon();
    test_statistic_colors();
    test_format_statistic_value();
    test_statistic_row_model();
    test_statistic_text_run_model();
    test_load_color();
    test_format_load_value();
    test_food_water_poison_blit();
    test_portrait_blit_model();
    test_damage_flash_model();
    test_spell_area_model();
    test_clock_tick_repaint_model();
    test_source_evidence();
    test_null_safety();
    printf("  ALL PASSED\n");
    return 0;
}
