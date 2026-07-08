#include "m11_game_view.h"

#include <stdio.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check_int(const char *label, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
    }
}

static void check_true(const char *label, int cond)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL %s\n", label);
    }
}

static void test_action_area_box(void)
{
    int x, y, w, h;
    check_true("action area zone", M11_GameView_GetV1ActionAreaZone(&x, &y, &w, &h));
    check_int("action area x", x, 224);
    check_int("action area y", y, 77);
    check_int("action area w", w, 96);
    check_int("action area h", h, 45);
}

static void test_action_menu_graphic_boxes(void)
{
    int x, y, w, h;
    check_true("3-row menu graphic", M11_GameView_GetV1ActionMenuGraphicZone(3, &x, &y, &w, &h));
    check_int("3-row x", x, 224);
    check_int("3-row y", y, 77);
    check_int("3-row w", w, 96);
    check_int("3-row h", h, 45);

    check_true("2-row menu graphic", M11_GameView_GetV1ActionMenuGraphicZone(2, &x, &y, &w, &h));
    check_int("2-row x", x, 224);
    check_int("2-row y", y, 77);
    check_int("2-row w", w, 96);
    check_int("2-row h", h, 33);

    check_true("1-row menu graphic", M11_GameView_GetV1ActionMenuGraphicZone(1, &x, &y, &w, &h));
    check_int("1-row x", x, 224);
    check_int("1-row y", y, 77);
    check_int("1-row w", w, 96);
    check_int("1-row h", h, 21);
}

static void test_action_row_and_icon_cells_stay_source_locked(void)
{
    int x, y, w, h;
    check_true("row0", M11_GameView_GetV1ActionMenuRowZone(0, &x, &y, &w, &h));
    check_int("row0 x", x, 234);
    check_int("row0 y", y, 86);
    check_int("row0 w", w, 85);
    check_int("row0 h", h, 11);
    check_true("row1", M11_GameView_GetV1ActionMenuRowZone(1, &x, &y, &w, &h));
    check_int("row1 y", y, 98);
    check_true("row2", M11_GameView_GetV1ActionMenuRowZone(2, &x, &y, &w, &h));
    check_int("row2 y", y, 110);

    check_true("icon0", M11_GameView_GetV1ActionIconCellZone(0, &x, &y, &w, &h));
    check_int("icon0 x", x, 233);
    check_true("icon3", M11_GameView_GetV1ActionIconCellZone(3, &x, &y, &w, &h));
    check_int("icon3 x", x, 299);
}

static void test_action_result_and_pass_zones(void)
{
    int x, y, w, h;
    check_int("action result zone id", M11_GameView_GetV1ActionResultZoneId(), 75);
    check_true("action result", M11_GameView_GetV1ActionResultZone(&x, &y, &w, &h));
    check_int("action result x", x, 224);
    check_int("action result y", y, 77);
    check_int("action result w", w, 96);
    check_int("action result h", h, 45);
    check_int("action pass zone id", M11_GameView_GetV1ActionPassZoneId(), 98);
    check_true("action pass", M11_GameView_GetV1ActionPassZone(&x, &y, &w, &h));
    check_int("action pass x", x, 285);
    check_int("action pass y", y, 77);
    check_int("action pass w", w, 34);
    check_int("action pass h", h, 7);
}

static void test_spell_area_boxes_stay_source_locked(void)
{
    int x, y, w, h;
    check_int("spell area zone id", M11_GameView_GetV1SpellAreaZoneId(), 13);
    check_true("spell area", M11_GameView_GetV1SpellAreaZone(&x, &y, &w, &h));
    check_int("spell area x", x, 233);
    check_int("spell area y", y, 42);
    check_int("spell area w", w, 87);
    check_int("spell area h", h, 25);
    check_int("spell bg graphic", M11_GameView_GetV1SpellAreaBackgroundGraphicId(), 9);
    check_int("spell lines graphic", M11_GameView_GetV1SpellAreaLinesGraphicId(), 11);

    check_int("caster panel zone id",
              M11_GameView_GetV1SpellCasterPanelZoneId(), 221);
    check_true("caster panel",
               M11_GameView_GetV1SpellCasterPanelZone(&x, &y, &w, &h));
    check_int("caster panel x", x, 233);
    check_int("caster panel y", y, 42);
    check_int("caster panel w", w, 87);
    check_int("caster panel h", h, 8);
    check_int("caster tab zone id",
              M11_GameView_GetV1SpellCasterTabZoneId(), 224);
    check_true("caster tab",
               M11_GameView_GetV1SpellCasterTabZone(&x, &y, &w, &h));
    check_int("caster tab x", x, 233);
    check_int("caster tab y", y, 42);
    check_int("caster tab w", w, 45);
    check_int("caster tab h", h, 8);

    check_int("available parent 0",
              M11_GameView_GetV1SpellAvailableSymbolParentZoneId(0), 245);
    check_int("available parent 5",
              M11_GameView_GetV1SpellAvailableSymbolParentZoneId(5), 250);
    check_int("available parent bad",
              M11_GameView_GetV1SpellAvailableSymbolParentZoneId(6), 0);
    check_int("available symbol 0",
              M11_GameView_GetV1SpellAvailableSymbolZoneId(0), 255);
    check_int("available symbol 5",
              M11_GameView_GetV1SpellAvailableSymbolZoneId(5), 260);
    check_int("available symbol bad",
              M11_GameView_GetV1SpellAvailableSymbolZoneId(-1), 0);
    check_int("champion symbol 0",
              M11_GameView_GetV1SpellChampionSymbolZoneId(0), 261);
    check_int("champion symbol 3",
              M11_GameView_GetV1SpellChampionSymbolZoneId(3), 264);
    check_int("champion symbol bad",
              M11_GameView_GetV1SpellChampionSymbolZoneId(4), 0);
    check_int("spell cast zone", M11_GameView_GetV1SpellCastZoneId(), 252);
    check_int("spell recant zone", M11_GameView_GetV1SpellRecantZoneId(), 254);
}

int main(void)
{
    test_action_area_box();
    test_action_menu_graphic_boxes();
    test_action_row_and_icon_cells_stay_source_locked();
    test_action_result_and_pass_zones();
    test_spell_area_boxes_stay_source_locked();
    printf("test_m11_v1_action_area_geometry_pc34_compat: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
