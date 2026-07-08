#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_values(void)
{
    /* DATA.C:119 G0000 init: { 224, 319, 42, 74 }. */
    const int *t = dm1_v1_box_spell_area_table_pc34();
    int n = dm1_v1_box_spell_area_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 224);
    CHECK(t[1] == 319);
    CHECK(t[2] == 42);
    CHECK(t[3] == 74);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_box_spell_area_x_pc34() == 224);
    CHECK(dm1_v1_box_spell_area_y_pc34() == 319);
    CHECK(dm1_v1_box_spell_area_w_pc34() == 42);
    CHECK(dm1_v1_box_spell_area_h_pc34() == 74);
}

static void test_screen_contract_helpers(void)
{
    DM1_V1_SpellAreaRectPc34 graphic = dm1_v1_spell_area_graphic_rect_pc34();
    DM1_V1_SpellAreaRectPc34 click = dm1_v1_spell_area_click_rect_pc34();
    DM1_V1_SpellAreaRectPc34 source = dm1_v1_spell_area_source_box_rect_pc34();
    DM1_V1_SpellAreaRectPc34 caster_panel =
        dm1_v1_spell_caster_panel_rect_pc34();
    DM1_V1_SpellAreaRectPc34 caster_tab =
        dm1_v1_spell_caster_tab_rect_pc34();

    CHECK(DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 == 9);
    CHECK(DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 == 11);
    CHECK(DM1_V1_SPELL_AREA_ZONE_ID_PC34 == 13);
    CHECK(DM1_V1_SPELL_CASTER_PANEL_ZONE_ID_PC34 == 221);
    CHECK(DM1_V1_SPELL_CASTER_TAB_ZONE_ID_PC34 == 224);
    CHECK(DM1_V1_SPELL_AREA_CAST_ZONE_ID_PC34 == 252);
    CHECK(DM1_V1_SPELL_AREA_RECANT_ZONE_ID_PC34 == 254);

    CHECK(graphic.x == 233);
    CHECK(graphic.y == 42);
    CHECK(graphic.w == 87);
    CHECK(graphic.h == 25);
    CHECK(click.x == 233);
    CHECK(click.y == 42);
    CHECK(click.w == 87);
    CHECK(click.h == 33);
    CHECK(source.x == 224);
    CHECK(source.y == 42);
    CHECK(source.w == 96);
    CHECK(source.h == 33);
    CHECK(caster_panel.x == 233);
    CHECK(caster_panel.y == 42);
    CHECK(caster_panel.w == 87);
    CHECK(caster_panel.h == 8);
    CHECK(caster_tab.x == 233);
    CHECK(caster_tab.y == 42);
    CHECK(caster_tab.w == 45);
    CHECK(caster_tab.h == 8);

    CHECK(dm1_v1_spell_available_symbol_parent_zone_id_pc34(0) == 245);
    CHECK(dm1_v1_spell_available_symbol_parent_zone_id_pc34(5) == 250);
    CHECK(dm1_v1_spell_available_symbol_parent_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_spell_available_symbol_parent_zone_id_pc34(6) == 0);
    CHECK(dm1_v1_spell_available_symbol_zone_id_pc34(0) == 255);
    CHECK(dm1_v1_spell_available_symbol_zone_id_pc34(5) == 260);
    CHECK(dm1_v1_spell_available_symbol_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_spell_available_symbol_zone_id_pc34(6) == 0);
    CHECK(dm1_v1_spell_champion_symbol_zone_id_pc34(0) == 261);
    CHECK(dm1_v1_spell_champion_symbol_zone_id_pc34(3) == 264);
    CHECK(dm1_v1_spell_champion_symbol_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_spell_champion_symbol_zone_id_pc34(4) == 0);
}

static void test_rune_contract_helpers(void)
{
    DM1_V1_SpellLabelSourceZonePc34 available =
        dm1_v1_spell_label_source_zone_pc34(0);
    DM1_V1_SpellLabelSourceZonePc34 selected =
        dm1_v1_spell_label_source_zone_pc34(1);
    char abbrev[3];

    CHECK(DM1_V1_SPELL_RUNE_ROW_COUNT_PC34 == 4);
    CHECK(DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34 == 6);
    CHECK(DM1_V1_SPELL_RUNE_SEQUENCE_MAX_PC34 == 4);
    CHECK(DM1_V1_SPELL_RUNE_VALUE_BASE_PC34 == 0x60);

    CHECK(dm1_v1_spell_rune_value_pc34(0, 0) == 0x60);
    CHECK(dm1_v1_spell_rune_value_pc34(0, 5) == 0x65);
    CHECK(dm1_v1_spell_rune_value_pc34(1, 0) == 0x66);
    CHECK(dm1_v1_spell_rune_value_pc34(2, 3) == 0x6f);
    CHECK(dm1_v1_spell_rune_value_pc34(3, 5) == 0x77);
    CHECK(dm1_v1_spell_rune_value_pc34(-1, 0) == -1);
    CHECK(dm1_v1_spell_rune_value_pc34(4, 0) == -1);
    CHECK(dm1_v1_spell_rune_value_pc34(0, -1) == -1);
    CHECK(dm1_v1_spell_rune_value_pc34(0, 6) == -1);

    CHECK(strcmp(dm1_v1_spell_rune_name_pc34(0, 0), "LO") == 0);
    CHECK(strcmp(dm1_v1_spell_rune_name_pc34(1, 3), "FUL") == 0);
    CHECK(strcmp(dm1_v1_spell_rune_name_pc34(2, 3), "IR") == 0);
    CHECK(strcmp(dm1_v1_spell_rune_name_pc34(3, 4), "RA") == 0);
    CHECK(dm1_v1_spell_rune_name_pc34(4, 0) == 0);
    CHECK(dm1_v1_spell_rune_name_pc34(0, 6) == 0);

    CHECK(dm1_v1_spell_rune_abbrev_pc34(2, 2, abbrev) == 1);
    CHECK(abbrev[0] == 'K');
    CHECK(abbrev[1] == 'A');
    CHECK(abbrev[2] == '\0');
    CHECK(dm1_v1_spell_rune_abbrev_pc34(4, 0, abbrev) == 0);
    CHECK(abbrev[0] == '?');
    CHECK(abbrev[1] == '?');
    CHECK(abbrev[2] == '\0');
    CHECK(dm1_v1_spell_rune_abbrev_pc34(0, 0, 0) == 0);

    CHECK(DM1_V1_SPELL_LABEL_CELL_W_PC34 == 14);
    CHECK(DM1_V1_SPELL_LABEL_CELL_H_PC34 == 13);
    CHECK(DM1_V1_SPELL_LABEL_AVAILABLE_Y_PC34 == 13);
    CHECK(DM1_V1_SPELL_LABEL_SELECTED_Y_PC34 == 26);
    CHECK(available.x == 0);
    CHECK(available.y == 13);
    CHECK(available.w == 14);
    CHECK(available.h == 13);
    CHECK(selected.x == 0);
    CHECK(selected.y == 26);
    CHECK(selected.w == 14);
    CHECK(selected.h == 13);
}

static void test_spell_panel_state_receipts(void)
{
    DM1_V1_SpellPanelStatePc34 state;
    DM1_V1_SpellPanelReceiptPc34 receipt;

    memset(&state, 0, sizeof(state));
    state.active = 1;

    CHECK(dm1_v1_spell_panel_command_allowed_pc34(&state) == 1);
    receipt = dm1_v1_spell_panel_open_pc34(&state);
    CHECK(receipt.accepted == 1);
    CHECK(receipt.panel_open == 1);
    CHECK(receipt.rune_row == 0);
    CHECK(receipt.rune_count == 0);
    CHECK(receipt.clear_runes == 1);
    CHECK(receipt.append_rune == 0);

    state.panel_open = 1;
    receipt = dm1_v1_spell_panel_enter_rune_pc34(&state, 3);
    CHECK(receipt.accepted == 1);
    CHECK(receipt.panel_open == 1);
    CHECK(receipt.rune_row == 1);
    CHECK(receipt.rune_count == 1);
    CHECK(receipt.append_rune == 1);
    CHECK(receipt.rune_value == 0x63);
    CHECK(receipt.rune_symbol_index == 3);
    CHECK(strcmp(receipt.rune_name, "EE") == 0);

    state.rune_row = 3;
    state.rune_count = 3;
    receipt = dm1_v1_spell_panel_enter_rune_pc34(&state, 5);
    CHECK(receipt.accepted == 1);
    CHECK(receipt.rune_row == 3);
    CHECK(receipt.rune_count == 4);
    CHECK(receipt.rune_value == 0x77);
    CHECK(strcmp(receipt.rune_name, "SAR") == 0);

    state.rune_count = 4;
    receipt = dm1_v1_spell_panel_enter_rune_pc34(&state, 0);
    CHECK(receipt.accepted == 0);
    CHECK(receipt.append_rune == 0);

    state.rune_count = 1;
    receipt = dm1_v1_spell_panel_enter_rune_pc34(&state, 6);
    CHECK(receipt.accepted == 0);
    CHECK(receipt.rune_value == -1);

    receipt = dm1_v1_spell_panel_clear_pc34(&state);
    CHECK(receipt.accepted == 1);
    CHECK(receipt.panel_open == 1);
    CHECK(receipt.rune_row == 0);
    CHECK(receipt.rune_count == 0);
    CHECK(receipt.clear_runes == 1);

    receipt = dm1_v1_spell_panel_close_pc34(&state);
    CHECK(receipt.accepted == 1);
    CHECK(receipt.panel_open == 0);
    CHECK(receipt.clear_runes == 1);

    state.candidate_panel_active = 1;
    CHECK(dm1_v1_spell_panel_command_allowed_pc34(&state) == 0);
    CHECK(dm1_v1_spell_panel_open_pc34(&state).accepted == 0);
    CHECK(dm1_v1_spell_panel_enter_rune_pc34(&state, 0).accepted == 0);
    CHECK(dm1_v1_spell_panel_clear_pc34(&state).accepted == 0);
    CHECK(dm1_v1_spell_panel_close_pc34(&state).accepted == 0);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_box_spell_area_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 224);
    rc = dm1_v1_box_spell_area_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 319);
    rc = dm1_v1_box_spell_area_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 42);
    rc = dm1_v1_box_spell_area_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 74);
    CHECK(dm1_v1_box_spell_area_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_spell_area_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_spell_area_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    /* All 4 components non-negative. */
    CHECK(dm1_v1_box_spell_area_x_pc34() >= 0);
    CHECK(dm1_v1_box_spell_area_y_pc34() >= 0);
    CHECK(dm1_v1_box_spell_area_w_pc34() > 0);
    CHECK(dm1_v1_box_spell_area_h_pc34() > 0);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxSpellAreaResultPc34 r;
    int ok = dm1_v1_box_spell_area_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 4);
    CHECK(r.tableEntries[0] == 224);
    CHECK(r.tableEntries[1] == 319);
    CHECK(r.tableEntries[2] == 42);
    CHECK(r.tableEntries[3] == 74);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs224 == 1);
    CHECK(r.yIs319 == 1);
    CHECK(r.wIs42 == 1);
    CHECK(r.hIs74 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.byteAligned == 1);
    CHECK(r.withinRowRange == 1);
    CHECK(r.withinBoxBounds == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_box_spell_area_get_pc34(i, &v);
        CHECK(rc == 1);
        CHECK(r.tableEntries[i] == v);
    }
}

int main(void)
{
    test_table_values();
    test_accessor_functions();
    test_screen_contract_helpers();
    test_rune_contract_helpers();
    test_spell_panel_state_receipts();
    test_get_function();
    test_components_non_negative();
    test_run_accepted();
    printf("dm1_v1_box_spell_area: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
