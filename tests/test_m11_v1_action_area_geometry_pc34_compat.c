#include "m11_game_view.h"
#include "dm1_v1_action_spell_m11_blit_plan_pc34_compat.h"

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

static void test_action_area_box(void)
{
    int x, y, w, h;
    check_true("action area zone", M11_GameView_GetV1ActionAreaZone(&x, &y, &w, &h));
    check_int("action area x", x, 233);
    check_int("action area y", y, 77);
    check_int("action area w", w, 87);
    check_int("action area h", h, 45);
    check_int("physical action clear x", dm1_v1_box_action_area_x_pc34(), 224);
    check_int("physical action clear y", dm1_v1_box_action_area_y_pc34(), 77);
    check_int("physical action clear w", dm1_v1_box_action_area_w_pc34(), 96);
    check_int("physical action clear h", dm1_v1_box_action_area_h_pc34(), 45);
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

static void test_action_spell_m11_blit_zone_ownership(void)
{
    DM1_V1_ActionSpellM11BlitPlanPc34 plan;
    check_true("one-row action blit plan",
               dm1_v1_action_spell_m11_blit_plan_build_pc34(
                   DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, 1, &plan));
    check_int("one-row action source graphic", plan.blits[0].graphicId, 10);
    check_int("one-row action source zone", plan.blits[0].zoneId, 79);
    check_int("one-row action source h", plan.blits[0].sourceH, 21);
    check_int("one-row action clear x", plan.clearX, 224);
    check_true("two-row action blit plan",
               dm1_v1_action_spell_m11_blit_plan_build_pc34(
                   DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, 2, &plan));
    check_int("two-row action source zone", plan.blits[0].zoneId, 77);
    check_int("two-row action source h", plan.blits[0].sourceH, 33);
    check_true("three-row action blit plan",
               dm1_v1_action_spell_m11_blit_plan_build_pc34(
                   DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34, 3, &plan));
    check_int("three-row action source zone", plan.blits[0].zoneId, 11);
    check_int("three-row action source h", plan.blits[0].sourceH, 45);
    check_true("spell blit plan",
               dm1_v1_action_spell_m11_blit_plan_build_pc34(
                   DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34, 0,
                   &plan));
    check_int("spell blit count", plan.blitCount, 3);
    check_int("spell C009 zone", plan.blits[0].zoneId, 13);
    check_int("spell C009 destination x", plan.blits[0].destinationX, 233);
    check_int("spell C009 destination y", plan.blits[0].destinationY, 42);
    check_int("spell C011 available zone", plan.blits[1].zoneId, 245);
    check_int("spell C011 available source y", plan.blits[1].sourceY, 13);
    check_int("spell C011 available destination y", plan.blits[1].destinationY, 50);
    check_int("spell C011 selected zone", plan.blits[2].zoneId, 261);
    check_int("spell C011 selected y", plan.blits[2].sourceY, 26);
    check_int("spell C011 selected destination y", plan.blits[2].destinationY, 62);
    check_int("spell clear x", plan.clearX, 224);
}

static void test_action_row_and_icon_cells_stay_source_locked(void)
{
    int x, y, w, h;
    M11_GameViewState state;
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

    /* CHAMPION.C F0330 publishes the per-champion C11 disable state; the
     * F0386 painter must hatch only the matching live action-hand cell. */
    M11_GameView_Init(&state);
    state.world.party.championCount = 2;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = 1;
    state.actionDisabledTicks[0] = 6;
    check_true("C11 action lock hatches live action hand",
               M11_GameView_ShouldHatchV1ActionIconCell(&state, 0));
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    state.actionDisabledTicks[1] = 6;
    check_int("empty action hand bypasses F0386 hatch",
              M11_GameView_ShouldHatchV1ActionIconCell(&state, 1), 0);
    state.candidateMirrorPanelActive = 1;
    check_true("G0299 hatch uses source action-hand gate",
               M11_GameView_ShouldHatchV1ActionIconCell(&state, 0));
    state.world.party.champions[0].hp.current = 0;
    check_int("dead champion returns before hatch",
              M11_GameView_ShouldHatchV1ActionIconCell(&state, 0), 0);
    check_int("invalid action cell cannot hatch",
              M11_GameView_ShouldHatchV1ActionIconCell(&state, 4), 0);
    M11_GameView_Shutdown(&state);
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

/* TODO(DM1-HUD-legacy-probe): these status/champion geometry accessors were
 * retired from M11 and have no current implementation.  Re-enable these two
 * cases only when a source-owned public probe API is restored; action/spell
 * geometry below remains active and uses live DM1 delegates. */
#if 0
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

    check_int("status box 0 zone", M11_GameView_GetV1StatusBoxZoneId(0), 151);
    check_int("status box 3 zone", M11_GameView_GetV1StatusBoxZoneId(3), 154);
    check_int("status box bad zone", M11_GameView_GetV1StatusBoxZoneId(4), 0);
    check_true("status box 3", M11_GameView_GetV1StatusBoxZone(3, &x, &y, &w, &h));
    check_int("status box 3 x", x, 207);
    check_int("status box 3 y", y, 0);
    check_int("status box 3 w", w, 67);
    check_int("status box 3 h", h, 29);

    check_int("bar graph 3 zone", M11_GameView_GetV1StatusBarGraphZoneId(3), 190);
    check_int("bar hp zone", M11_GameView_GetV1StatusBarZoneId(0), 195);
    check_int("bar stamina zone", M11_GameView_GetV1StatusBarZoneId(1), 199);
    check_int("bar mana value zone champ2", M11_GameView_GetV1StatusBarValueZoneId(2, 2), 205);
    check_true("bar champ2 stamina", M11_GameView_GetV1StatusBarZone(2, 1, &x, &y, &w, &h));
    check_int("bar champ2 stamina x", x, 191);
    check_int("bar champ2 stamina y", y, 0);
    check_int("bar champ2 stamina w", w, 4);
    check_int("bar champ2 stamina h", h, 25);

    check_int("hand parent champ3", M11_GameView_GetV1StatusHandParentZoneId(3), 210);
    check_int("hand zone champ3 action", M11_GameView_GetV1StatusHandZoneId(3, 1), 218);
    check_true("hand champ3 action", M11_GameView_GetV1StatusHandZone(3, 1, &x, &y, &w, &h));
    check_int("hand champ3 action x", x, 231);
    check_int("hand champ3 action y", y, 10);
    check_int("hand champ3 action w", w, 16);
    check_int("hand champ3 action h", h, 16);
    check_true("hand icon champ3 action", M11_GameView_GetV1StatusHandIconZone(3, 1, &x, &y, &w, &h));
    check_int("hand icon champ3 action x", x, 232);
    check_int("hand icon champ3 action y", y, 11);
    check_true("hand slot box champ3 action",
               M11_GameView_GetV1StatusHandSlotBoxZone(3, 1, &x, &y, &w, &h));
    check_int("hand slot box champ3 action w", w, 18);
    check_int("hand slot box champ3 action h", h, 18);
    check_int("slot box normal", M11_GameView_GetV1SlotBoxNormalGraphicId(), 33);
    check_int("slot box wounded", M11_GameView_GetV1SlotBoxWoundedGraphicId(), 34);
    check_int("slot box acting", M11_GameView_GetV1SlotBoxActingHandGraphicId(), 35);
    check_int("status action acting graphic",
              M11_GameView_GetV1StatusHandSlotGraphic(&state, 3, 1), 35);
    check_int("status action wounded empty icon",
              M11_GameView_GetV1StatusHandIconIndex(&state, 3, 1), 215);
    state.actingChampionOrdinal = 0;
    check_int("status action wounded graphic",
              M11_GameView_GetV1StatusHandSlotGraphic(&state, 3, 1), 34);
    check_int("food label graphic", M11_GameView_GetV1FoodLabelGraphicId(), 30);
    check_int("water label graphic", M11_GameView_GetV1WaterLabelGraphicId(), 31);
    check_int("poison label graphic", M11_GameView_GetV1PoisonLabelGraphicId(), 32);

    check_int("name clear champ3", M11_GameView_GetV1StatusNameClearZoneId(3), 162);
    check_int("name text champ3", M11_GameView_GetV1StatusNameTextZoneId(3), 166);
    check_true("name clear zone champ3", M11_GameView_GetV1StatusNameZone(3, &x, &y, &w, &h));
    check_int("name clear x", x, 207);
    check_int("name clear w", w, 43);
    check_true("name text zone champ3", M11_GameView_GetV1StatusNameTextZone(3, &x, &y, &w, &h));
    check_int("name text x", x, 208);
    check_int("name text w", w, 42);

    check_int("damage indicator zone champ3", M11_GameView_GetV1DamageIndicatorZoneId(3), 170);
    check_true("damage indicator champ3",
               M11_GameView_GetV1DamageIndicatorZone(3, 45, 7, &x, &y, &w, &h));
    check_int("damage indicator champ3 x", x, 218);
    check_int("damage indicator champ3 y", y, 11);
    check_int("inventory damage zone champ3",
              M11_GameView_GetV1InventoryDamageIndicatorZoneId(3), 182);
    check_true("inventory damage champ3",
               M11_GameView_GetV1InventoryDamageIndicatorZone(3, 32, 29, &x, &y, &w, &h));
    check_int("inventory damage champ3 x", x, 214);
    check_int("inventory damage champ3 y", y, 0);
    check_true("damage number champ3",
               M11_GameView_GetV1DamageNumberOrigin(3, &x, &y));
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
#endif

int main(void)
{
    test_action_area_box();
    test_action_menu_graphic_boxes();
    test_action_spell_m11_blit_zone_ownership();
    test_action_row_and_icon_cells_stay_source_locked();
    test_action_result_and_pass_zones();
    test_spell_area_boxes_stay_source_locked();
    printf("test_m11_v1_action_area_geometry_pc34_compat: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
