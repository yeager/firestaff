#include "m11_game_view.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

static int status_rect_to_xywh(const DM1_V1_ChampionStatusRectPc34* r,
                               int* x,
                               int* y,
                               int* w,
                               int* h)
{
    if (!r) return 0;
    if (x) *x = r->x;
    if (y) *y = r->y;
    if (w) *w = r->w;
    if (h) *h = r->h;
    return 1;
}

static void test_action_area_box(void)
{
    int x, y, w, h;
    check_true("action area zone", M11_GameView_GetV1ActionAreaZone(&x, &y, &w, &h));
    check_int("action area x", x, 233);
    check_int("action area y", y, 77);
    check_int("action area w", w, 87);
    check_int("action area h", h, 45);
}

static void test_action_menu_graphic_boxes(void)
{
    int x, y, w, h;
    check_true("3-row menu graphic", M11_GameView_GetV1ActionMenuGraphicZone(3, &x, &y, &w, &h));
    check_int("3-row x", x, 233);
    check_int("3-row y", y, 77);
    check_int("3-row w", w, 87);
    check_int("3-row h", h, 45);

    check_true("2-row menu graphic", M11_GameView_GetV1ActionMenuGraphicZone(2, &x, &y, &w, &h));
    check_int("2-row x", x, 233);
    check_int("2-row y", y, 77);
    check_int("2-row w", w, 87);
    check_int("2-row h", h, 33);

    check_true("1-row menu graphic", M11_GameView_GetV1ActionMenuGraphicZone(1, &x, &y, &w, &h));
    check_int("1-row x", x, 233);
    check_int("1-row y", y, 77);
    check_int("1-row w", w, 87);
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
    check_int("action result x", x, 233);
    check_int("action result y", y, 77);
    check_int("action result w", w, 87);
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
    check_int("spell area h", h, 33);
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

static void test_status_boxes_stay_source_locked(void)
{
    M11_GameViewState state;
    int x, y, w, h;
    int i;
    memset(&state, 0, sizeof(state));
    state.world.party.championCount = 4;
    state.world.party.champions[3].present = 1;
    state.world.party.champions[3].hp.current = 10;
    state.world.party.champions[3].wounds = 0x0002u;
    for (i = 0; i < 30; ++i) {
        state.world.party.champions[3].inventory[i] = 0xFFFFu;
    }
    state.actingChampionOrdinal = 4;

    DM1_V1_ChampionStatusRectPc34 rect;
    check_int("status box 0 zone", dm1_v1_champion_status_box_zone_id_pc34(0), 151);
    check_int("status box 3 zone", dm1_v1_champion_status_box_zone_id_pc34(3), 154);
    check_int("status box bad zone", dm1_v1_champion_status_box_zone_id_pc34(4), 0);
    check_true("status box 3", dm1_v1_champion_status_box_rect_pc34(3, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("status box 3 x", x, 207);
    check_int("status box 3 y", y, 0);
    check_int("status box 3 w", w, 67);
    check_int("status box 3 h", h, 29);

    check_int("bar graph 3 zone", dm1_v1_champion_status_bar_graph_zone_id_pc34(3), 190);
    check_int("bar hp zone", dm1_v1_champion_status_bar_zone_id_pc34(0), 195);
    check_int("bar stamina zone", dm1_v1_champion_status_bar_zone_id_pc34(1), 199);
    check_int("bar mana value zone champ2", dm1_v1_champion_status_bar_value_zone_id_pc34(2, 2), 205);
    check_true("bar champ2 stamina", dm1_v1_champion_status_bar_rect_pc34(2, 1, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("bar champ2 stamina x", x, 191);
    check_int("bar champ2 stamina y", y, 2);
    check_int("bar champ2 stamina w", w, 4);
    check_int("bar champ2 stamina h", h, 25);

    check_int("hand parent champ3", dm1_v1_champion_status_hand_parent_zone_id_pc34(3), 210);
    check_int("hand zone champ3 action", dm1_v1_champion_status_hand_zone_id_pc34(3, 1), 218);
    check_true("hand champ3 action", dm1_v1_champion_status_hand_rect_pc34(3, 1, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("hand champ3 action x", x, 231);
    check_int("hand champ3 action y", y, 10);
    check_int("hand champ3 action w", w, 16);
    check_int("hand champ3 action h", h, 16);
    check_true("hand icon champ3 action", dm1_v1_champion_status_hand_icon_rect_pc34(3, 1, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("hand icon champ3 action x", x, 232);
    check_int("hand icon champ3 action y", y, 11);
    check_true("hand slot box champ3 action",
               dm1_v1_champion_status_hand_slot_box_rect_pc34(3, 1, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("hand slot box champ3 action w", w, 18);
    check_int("hand slot box champ3 action h", h, 18);
    check_int("slot box normal", dm1_v1_graphic_slot_box_normal_pc34(), 33);
    check_int("slot box wounded", dm1_v1_graphic_slot_box_wounded_pc34(), 34);
    check_int("slot box acting", dm1_v1_graphic_slot_box_acting_hand_pc34(), 35);
    check_int("status action acting graphic",
              dm1_v1_champion_status_hand_slot_graphic_pc34(1, 0x0002u, 1), 35);
    check_int("status action wounded empty icon",
              M11_GameView_GetV1StatusHandIconIndex(&state, 3, 1), 215);
    state.actingChampionOrdinal = 0;
    check_int("status action wounded graphic",
              dm1_v1_champion_status_hand_slot_graphic_pc34(1, 0x0002u, 0), 34);
    check_int("food label graphic", dm1_v1_graphic_food_label_pc34(), 30);
    check_int("water label graphic", dm1_v1_graphic_water_label_pc34(), 31);
    check_int("poison label graphic", dm1_v1_graphic_poisoned_label_pc34(), 32);

    check_int("name clear champ3", dm1_v1_champion_status_name_clear_zone_id_pc34(3), 162);
    check_int("name text champ3", dm1_v1_champion_status_name_text_zone_id_pc34(3), 166);
    check_true("name clear zone champ3", dm1_v1_champion_status_name_rect_pc34(3, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("name clear x", x, 207);
    check_int("name clear w", w, 43);
    check_true("name text zone champ3", dm1_v1_champion_status_name_text_rect_pc34(3, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("name text x", x, 208);
    check_int("name text w", w, 42);

    check_int("damage indicator zone champ3", dm1_v1_champion_damage_indicator_zone_id_pc34(3), 170);
    check_true("damage indicator champ3",
               dm1_v1_champion_damage_indicator_rect_pc34(3, 45, 7, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("damage indicator champ3 x", x, 218);
    check_int("damage indicator champ3 y", y, 11);
    check_int("inventory damage zone champ3",
              dm1_v1_champion_inventory_damage_indicator_zone_id_pc34(3), 182);
    check_true("inventory damage champ3",
               dm1_v1_champion_inventory_damage_indicator_rect_pc34(3, 32, 29, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, &w, &h);
    check_int("inventory damage champ3 x", x, 214);
    check_int("inventory damage champ3 y", y, 0);
    check_true("damage number champ3",
               dm1_v1_champion_damage_number_origin_pc34(3, &rect));
    (void)status_rect_to_xywh(&rect, &x, &y, NULL, NULL);
    check_int("damage number champ3 x", x, 236);
    check_int("damage number champ3 y", y, 11);
    check_true("pc34 damage number champ3",
               M11_GameView_GetV1DamageNumberOriginPc34(3, 77, 1, &x, &y));
    check_int("pc34 damage number champ3 x", x, 225);
    check_int("pc34 damage number champ3 y", y, 16);
}

static void test_champion_icons_stay_source_locked(void)
{
    M11_GameViewState state;
    int x, y, w, h;
    memset(&state, 0, sizeof(state));
    state.world.party.championCount = 2;
    state.world.party.direction = 3;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].direction = 1;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].direction = 0;

    check_int("champion icon graphic", M11_GameView_GetV1ChampionIconGraphicId(), 28);
    check_int("champion icon zone 0", M11_GameView_GetV1ChampionIconZoneId(0), 113);
    check_int("champion icon zone 3", M11_GameView_GetV1ChampionIconZoneId(3), 116);
    check_true("champion icon 2 rect",
               M11_GameView_GetV1ChampionIconZone(2, &x, &y, &w, &h));
    check_int("champion icon 2 x", x, 301);
    check_int("champion icon 2 y", y, 15);
    check_int("champion icon w", w, 19);
    check_int("champion icon h", h, 14);
    check_int("champion icon source 0",
              M11_GameView_GetV1ChampionIconSourceIndex(&state, 0), 2);
    check_int("champion icon source 1",
              M11_GameView_GetV1ChampionIconSourceIndex(&state, 1), 1);
    check_int("champion icon source absent",
              M11_GameView_GetV1ChampionIconSourceIndex(&state, 2), -1);
}

int main(void)
{
    test_action_area_box();
    test_action_menu_graphic_boxes();
    test_action_row_and_icon_cells_stay_source_locked();
    test_action_result_and_pass_zones();
    test_spell_area_boxes_stay_source_locked();
    test_status_boxes_stay_source_locked();
    test_champion_icons_stay_source_locked();
    printf("test_m11_v1_action_area_geometry_pc34_compat: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
