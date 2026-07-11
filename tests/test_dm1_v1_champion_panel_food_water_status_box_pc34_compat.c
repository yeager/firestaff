#include "dm1_v1_champion_panel_food_water_status_box_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *label, int got, int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
    } else {
        printf("PASS %s == %d (%s)\n", label, want, anchor);
    }
}

static void expect_u16(const char *label, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%04X want=0x%04X anchor=%s\n",
               label, (unsigned)got, (unsigned)want, anchor);
    } else {
        printf("PASS %s == 0x%04X (%s)\n",
               label, (unsigned)want, anchor);
    }
}

static void expect_contains(const char *label, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", label, needle, anchor);
    }
}

static int frame_pixel(
    const dm1_v1_champion_panel_food_water_status_box_frame_pc34_t *frame,
    int x,
    int y)
{
    return frame->bytes[y * DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 + x];
}

static void expect_pixel(
    const char *label,
    const dm1_v1_champion_panel_food_water_status_box_frame_pc34_t *frame,
    int x,
    int y,
    int want,
    const char *anchor)
{
    char full_label[128];

    snprintf(full_label, sizeof(full_label), "%s[%d,%d]", label, x, y);
    expect_int(full_label, frame_pixel(frame, x, y), want, anchor);
}

static void expect_operation(
    const char *label,
    const dm1_v1_champion_panel_food_water_status_box_operation_pc34_t *op,
    dm1_v1_champion_panel_food_water_status_box_operation_kind_pc34_t kind,
    int sequence,
    int graphic_id,
    int zone_id,
    int transparent_color,
    int fill_color,
    const char *evidence_needle,
    const char *anchor)
{
    char item[128];

    snprintf(item, sizeof(item), "%s.kind", label);
    expect_int(item, op->kind, kind, anchor);
    snprintf(item, sizeof(item), "%s.sequence", label);
    expect_int(item, op->sequence, sequence, anchor);
    snprintf(item, sizeof(item), "%s.graphic", label);
    expect_int(item, op->graphic_id, graphic_id, anchor);
    snprintf(item, sizeof(item), "%s.zone", label);
    expect_int(item, op->zone_id, zone_id, anchor);
    snprintf(item, sizeof(item), "%s.transparent", label);
    expect_int(item, op->transparent_color, transparent_color, anchor);
    snprintf(item, sizeof(item), "%s.fill", label);
    expect_int(item, op->fill_color, fill_color, anchor);
    snprintf(item, sizeof(item), "%s.sourceEvidence", label);
    expect_contains(item, op->sourceEvidence, evidence_needle, anchor);
}

static void test_contract_and_evidence(void)
{
    const dm1_v1_champion_panel_food_water_status_box_contract_pc34_t *contract =
        dm1_v1_champion_panel_food_water_status_box_contract_pc34();
    const char *source =
        dm1_v1_champion_panel_food_water_status_box_source_evidence_pc34();

    expect_int("contract.contract_only", contract->contract_only, 1,
               "asset-free contract-only gate");
    expect_int("contract.champion_count", contract->champion_count, 4,
               "CHAMPION.C F0284:123-130 G0305 traversal");
    expect_int("contract.first_status_zone", contract->status_box_first_zone,
               151, "DEFS.H:3783 C151");
    expect_int("contract.last_status_zone", contract->status_box_last_zone,
               154, "DEFS.H:3786 C154");
    expect_int("contract.width", contract->status_box_width, 67,
               "CHAMDRAW.C F0292:773-775");
    expect_int("contract.height", contract->status_box_height, 29,
               "CHAMDRAW.C F0292:773-775");
    expect_int("contract.stride", contract->status_box_stride, 69,
               "DEFS.H:2157 C69");
    expect_int("contract.fill", contract->status_box_fill_color, 12,
               "CHAMDRAW.C F0292:786-789");
    expect_int("contract.border_transparent",
               contract->status_box_border_transparent_color, 10,
               "CHAMDRAW.C F0292:804/807 and DEFS.H:2088");
    expect_int("contract.panel_empty", contract->panel_empty_graphic, 20,
               "PANEL.C F0345:1597");
    expect_int("contract.food_graphic", contract->food_label_graphic, 30,
               "DEFS.H:2190 C030");
    expect_int("contract.water_graphic", contract->water_label_graphic, 31,
               "DEFS.H:2191 C031");
    expect_int("contract.food_zone", contract->food_label_zone, 500,
               "DEFS.H:3869 C500");
    expect_int("contract.water_zone", contract->water_label_zone, 501,
               "DEFS.H:3870 C501");
    expect_int("contract.food_bar_zone", contract->food_bar_zone, 103,
               "DEFS.H:3777 C103");
    expect_int("contract.water_bar_zone", contract->water_bar_zone, 104,
               "DEFS.H:3778 C104");
    expect_int("contract.food_base_color", contract->food_base_color, 5,
               "PANEL.C F0345:1614 and DEFS.H:2083");
    expect_int("contract.water_base_color", contract->water_base_color, 14,
               "PANEL.C F0345:1615 and DEFS.H:2092");
    expect_int("contract.yellow", contract->warning_yellow_color, 11,
               "PANEL.C F0344:1522-1524");
    expect_int("contract.red", contract->warning_red_color, 8,
               "PANEL.C F0344:1519-1521");
    expect_contains("contract.chest_anchor", contract->chest_close_anchor,
                    "113-132", "CHEST.C F0334:113-132");
    expect_contains("contract.menu_anchor", contract->menu_anchor,
                    "F0409/F0410/F0411",
                    "MENU.C F0409/F0410/F0411:1666-1721");
    expect_contains("contract.panel_anchor", contract->panel_draw_anchor,
                    "F0345:1563-1616", "PANEL.C F0345:1563-1616");
    expect_contains("contract.draw_state_anchor", contract->draw_state_anchor,
                    "771-789", "CHAMDRAW.C F0292:771-789");
    expect_contains("contract.champion_anchor", contract->champion_state_anchor,
                    "93-130", "CHAMPION.C F0284:93-130");
    expect_contains("contract.panel_close_anchor", contract->panel_close_anchor,
                    "2299-2322", "PANEL.C F0354:2299-2322");
    expect_contains("contract.defs_anchor", contract->defs_anchor,
                    "C151-C154", "DEFS.H:3783-3786");

    expect_contains("source.chest", source, "CHEST.C F0334:113-132",
                    "sourceEvidence chest close");
    expect_contains("source.panel_close", source, "PANEL.C F0354:2299-2322",
                    "sourceEvidence panel close");
    expect_contains("source.status_box", source, "CHAMDRAW.C F0292:771-789",
                    "sourceEvidence status box");
    expect_contains("source.c10", source, "C10_COLOR_FLESH",
                    "sourceEvidence C10 transparency");
    expect_contains("source.g0423", source, "G0423",
                    "sourceEvidence G0423 inventory champion");
    expect_contains("source.g0305", source, "G0305",
                    "sourceEvidence G0305 party count");
    expect_contains("source.f0345", source, "PANEL.C F0345:1579-1615",
                    "sourceEvidence food/water draw");
    expect_contains("source.f0344", source, "PANEL.C F0344:1493-1561",
                    "sourceEvidence food/water bar");
    expect_contains("source.menu", source, "MENU.C F0409/F0410/F0411",
                    "sourceEvidence requested MENU symbols");
    expect_contains("source.defs", source, "3869-3870",
                    "sourceEvidence C500/C501 zones");
}

static void test_default_order_frame_and_panel(void)
{
    dm1_v1_champion_panel_food_water_status_box_result_pc34_t result =
        dm1_v1_champion_panel_food_water_status_box_probe_pc34(NULL);

    expect_int("default.valid", result.valid, 1,
               "valid synthetic inventory champion");
    expect_int("default.contract_only", result.contract_only, 1,
               "asset-free contract-only gate");
    expect_int("default.no_graphics_dat", result.loads_graphics_dat, 0,
               "no GRAPHICS.DAT load");
    expect_int("default.no_dungeon_dat", result.loads_dungeon_dat, 0,
               "no DUNGEON.DAT load");
    expect_int("default.close_before_status", result.close_before_status_box,
               1, "CHEST.C F0334 before CHAMDRAW.C F0292");
    expect_int("default.status_before_panel", result.status_box_before_panel,
               1, "CHAMDRAW.C F0292 before PANEL.C F0345 panel blits");
    expect_int("default.chest_open", result.chest_was_open, 1,
               "CHEST.C F0334:113-116");
    expect_int("default.chest_cleared", result.open_chest_cleared, 1,
               "CHEST.C F0334:116");
    expect_int("default.non_empty_slots", result.non_empty_chest_slots, 3,
               "CHEST.C F0334:118-132");
    expect_u16("default.first_relink", result.relink_first_thing, 0x0101u,
               "CHEST.C F0334:123-127");
    expect_u16("default.last_relink", result.relink_last_thing, 0x0103u,
               "CHEST.C F0334:129-130");
    expect_int("default.inventory_index", result.inventory_champion_index, 1,
               "G0423 ordinal to M516 index");
    expect_int("default.party_count", result.party_champion_count, 4,
               "CHAMPION.C F0284:124 G0305");
    expect_int("default.g0423", result.g0423_inventory_champion_ordinal, 2,
               "PANEL.C F0345:1579 G0423 read");
    expect_int("default.g0305", result.g0305_party_champion_count, 4,
               "CHAMPION.C F0284:124 G0305 read");
    expect_int("default.food_counter", result.food_counter, 256,
               "PANEL.C F0345:1614 L1074_ps_Champion->Food");
    expect_int("default.water_counter", result.water_counter, -700,
               "PANEL.C F0345:1615 L1074_ps_Champion->Water");
    expect_int("default.food_word_color", result.food_word_color, 5,
               "PANEL.C F0345:1614 C05_COLOR_LIGHT_BROWN");
    expect_int("default.water_word_color", result.water_word_color, 14,
               "PANEL.C F0345:1615 C14_COLOR_BLUE");
    expect_int("default.food_bar_color", result.food_bar_color, 5,
               "PANEL.C F0344:1524-1525");
    expect_int("default.water_bar_color", result.water_bar_color, 8,
               "PANEL.C F0344:1519-1521");
    expect_int("default.food_units", result.food_bar_units, 4166,
               "PANEL.C F0344:1537-1544");
    expect_int("default.water_units", result.water_bar_units, 1054,
               "PANEL.C F0344:1537-1544");
    expect_int("default.menu_disambiguated",
               result.menu_f0409_f0410_f0411_disambiguated, 1,
               "MENU.C F0409/F0410/F0411:1666-1721");
    expect_int("default.operation_count", result.operation_count, 9,
               "ordered close/status/panel operation log");

    expect_int("frame.champion_index", result.frame.champion_index, 1,
               "PANEL.C F0345:1579 G0423 ordinal maps to champion index");
    expect_int("frame.zone", result.frame.zone_id, 152,
               "DEFS.H:3784 C152");
    expect_int("frame.screen_x", result.frame.screen_x, 69,
               "DEFS.H:2157 C69 stride");
    expect_int("frame.screen_y", result.frame.screen_y, 0,
               "CHAMDRAW.C F0292:773");
    expect_int("frame.width", result.frame.width, 67,
               "CHAMDRAW.C F0292:773-775");
    expect_int("frame.height", result.frame.height, 29,
               "CHAMDRAW.C F0292:773-775");
    expect_int("frame.fill_color", result.frame.fill_color, 12,
               "CHAMDRAW.C F0292:786-789");
    expect_int("frame.border_transparent", result.frame.border_transparent_color,
               10, "CHAMDRAW.C F0292:804/807");
    expect_int("frame.fill_pixels", result.frame.fill_pixel_count, 1943,
               "67x29 status-box byte count");
    expect_int("frame.border_preserved",
               result.frame.transparent_border_pixel_count, 188,
               "C10 transparent perimeter preserves destination");
    expect_pixel("frame.topleft", &result.frame, 0, 0, 12,
                 "CHAMDRAW.C F0292:786 plus C10 transparent border");
    expect_pixel("frame.topright", &result.frame, 66, 0, 12,
                 "CHAMDRAW.C F0292:786 plus C10 transparent border");
    expect_pixel("frame.bottomleft", &result.frame, 0, 28, 12,
                 "CHAMDRAW.C F0292:786 plus C10 transparent border");
    expect_pixel("frame.bottomright", &result.frame, 66, 28, 12,
                 "CHAMDRAW.C F0292:786 plus C10 transparent border");
    expect_pixel("frame.center", &result.frame, 33, 14, 12,
                 "CHAMDRAW.C F0292:786-789 fill");

    expect_operation("op0.close", &result.operations[0],
                     DM1_V1_CPFW_OP_CHEST_CLOSE_PC34, 0, -1, -1, -1, -1,
                     "CHEST.C F0334:113-132",
                     "CHEST.C F0334:113-132");
    expect_int("op0.amount", result.operations[0].amount, 3,
               "CHEST.C F0334:118-132 non-empty slots");
    expect_int("op0.units", result.operations[0].proportional_units, 1,
               "CHEST.C F0334:116 open chest cleared");
    expect_operation("op1.close_bracket", &result.operations[1],
                     DM1_V1_CPFW_OP_PANEL_CLOSE_BRACKET_PC34, 1, -1, -1,
                     -1, -1, "PANEL.C F0354:2299-2322",
                     "PANEL.C F0354:2299-2322");
    expect_operation("op2.status_fill", &result.operations[2],
                     DM1_V1_CPFW_OP_STATUS_BOX_FILL_PC34, 2, -1, 152, -1,
                     12, "CHAMDRAW.C F0292:771-789",
                     "CHAMDRAW.C F0292:771-789");
    expect_int("op2.amount", result.operations[2].amount, 1943,
               "67x29 status-box byte count");
    expect_int("op2.units", result.operations[2].proportional_units, 69,
               "DEFS.H:2157 C69");
    expect_operation("op3.border", &result.operations[3],
                     DM1_V1_CPFW_OP_STATUS_BOX_BORDER_PC34, 3, -1, 152,
                     10, 12, "DEFS.H:2088",
                     "CHAMDRAW.C F0292:804/807");
    expect_int("op3.amount", result.operations[3].amount, 188,
               "C10 transparent perimeter pixels");
    expect_operation("op4.panel_empty", &result.operations[4],
                     DM1_V1_CPFW_OP_PANEL_EMPTY_PC34, 4, 20, 101, 8, -1,
                     "PANEL.C F0345:1597", "PANEL.C F0345:1597");
    expect_operation("op5.food_label", &result.operations[5],
                     DM1_V1_CPFW_OP_FOOD_LABEL_PC34, 5, 30, 500, 12, 5,
                     "DEFS.H:2190,3869", "PANEL.C F0345:1598");
    expect_int("op5.amount", result.operations[5].amount, 256,
               "PANEL.C F0345:1614 food counter");
    expect_operation("op6.water_label", &result.operations[6],
                     DM1_V1_CPFW_OP_WATER_LABEL_PC34, 6, 31, 501, 12, 14,
                     "DEFS.H:2191,3870", "PANEL.C F0345:1599");
    expect_int("op6.amount", result.operations[6].amount, -700,
               "PANEL.C F0345:1615 water counter");
    expect_operation("op7.food_bar", &result.operations[7],
                     DM1_V1_CPFW_OP_FOOD_BAR_PC34, 7, -1, 103, -1, 5,
                     "F0345:1614", "PANEL.C F0344:1493-1561");
    expect_int("op7.units", result.operations[7].proportional_units, 4166,
               "PANEL.C F0344:1537-1544");
    expect_operation("op8.water_bar", &result.operations[8],
                     DM1_V1_CPFW_OP_WATER_BAR_PC34, 8, -1, 104, -1, 8,
                     "F0345:1615", "PANEL.C F0344:1493-1561");
    expect_int("op8.units", result.operations[8].proportional_units, 1054,
               "PANEL.C F0344:1537-1544");
    expect_contains("result.sourceEvidence", result.sourceEvidence,
                    "status box as 67x29 at stride C69",
                    "CHAMDRAW.C F0292:771-789 sourceEvidence");
}

static void test_counter_thresholds_and_validation(void)
{
    dm1_v1_champion_panel_food_water_status_box_input_pc34_t input =
        dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
    dm1_v1_champion_panel_food_water_status_box_result_pc34_t result;

    input.food = -16;
    input.water = -1;
    result = dm1_v1_champion_panel_food_water_status_box_probe_pc34(&input);
    expect_int("threshold.valid", result.valid, 1,
               "valid warning-color fixture");
    expect_int("threshold.food_yellow", result.food_bar_color, 11,
               "PANEL.C F0344:1522-1524");
    expect_int("threshold.water_yellow", result.water_bar_color, 11,
               "PANEL.C F0344:1522-1524");
    expect_int("threshold.food_units", result.food_bar_units, 3281,
               "PANEL.C F0344:1537-1544");
    expect_int("threshold.water_units", result.water_bar_units, 3330,
               "PANEL.C F0344:1537-1544");
    expect_int("threshold.food_op_color", result.operations[7].fill_color, 11,
               "PANEL.C F0344:1522-1524");
    expect_int("threshold.water_op_color", result.operations[8].fill_color, 11,
               "PANEL.C F0344:1522-1524");

    input.food = 2048;
    input.water = -1024;
    result = dm1_v1_champion_panel_food_water_status_box_probe_pc34(&input);
    expect_int("clamp.food_full", result.food_bar_units, 10000,
               "PANEL.C F0344:1537-1544 proportional max");
    expect_int("clamp.water_empty", result.water_bar_units, 0,
               "PANEL.C F0344:1537-1544 proportional min");
    expect_int("clamp.food_base_color", result.food_bar_color, 5,
               "PANEL.C F0344:1524-1525");
    expect_int("clamp.water_red", result.water_bar_color, 8,
               "PANEL.C F0344:1519-1521");

    input = dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
    input.open_chest_thing = 0xFFFFu;
    input.chest_slots[0] = 0xFFFFu;
    input.chest_slots[2] = 0xFFFFu;
    input.chest_slots[4] = 0xFFFFu;
    result = dm1_v1_champion_panel_food_water_status_box_probe_pc34(&input);
    expect_int("closed_chest.valid", result.valid, 1,
               "CHEST.C F0334:113-114 allows no-open-chest return");
    expect_int("closed_chest.was_open", result.chest_was_open, 0,
               "CHEST.C F0334:113-114");
    expect_int("closed_chest.cleared", result.open_chest_cleared, 0,
               "CHEST.C F0334:113-114");
    expect_int("closed_chest.non_empty", result.non_empty_chest_slots, 0,
               "CHEST.C F0334:118-132 skipped");
    expect_u16("closed_chest.first", result.relink_first_thing, 0xFFFEu,
               "CHEST.C F0334:117 end-of-list sentinel");
    expect_u16("closed_chest.last", result.relink_last_thing, 0xFFFEu,
               "CHEST.C F0334:117 end-of-list sentinel");
    expect_int("closed_chest.status_still_before_panel",
               result.status_box_before_panel, 1,
               "PANEL.C F0354 -> CHAMDRAW.C F0292 -> PANEL.C F0345 order");

    input = dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
    input.inventory_champion_ordinal = 5;
    result = dm1_v1_champion_panel_food_water_status_box_probe_pc34(&input);
    expect_int("invalid_ordinal.valid", result.valid, 0,
               "G0423 ordinal must be within G0305 party count");
    expect_int("invalid_ordinal.rejected", result.rejected_invalid_champion, 1,
               "CHAMPION.C F0284:124 G0305 domain");

    input = dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
    input.party_champion_count = 1;
    input.inventory_champion_ordinal = 2;
    result = dm1_v1_champion_panel_food_water_status_box_probe_pc34(&input);
    expect_int("invalid_party.valid", result.valid, 0,
               "G0423 ordinal must be within G0305 party count");
    expect_int("invalid_party.rejected", result.rejected_invalid_champion, 1,
               "CHAMPION.C F0284:124 G0305 domain");

    input = dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
    input.current_health = 0;
    result = dm1_v1_champion_panel_food_water_status_box_probe_pc34(&input);
    expect_int("dead.valid", result.valid, 0,
               "CHAMDRAW.C F0292:784 live branch requires CurrentHealth");
    expect_int("dead.rejected", result.rejected_dead_champion, 1,
               "CHAMDRAW.C F0292:784 live branch requires CurrentHealth");
}

int main(void)
{
    printf("== DM1 V1 champion panel food/water status-box slice ==\n");
    test_contract_and_evidence();
    test_default_order_frame_and_panel();
    test_counter_thresholds_and_validation();

    if (g_assertions < 60) {
        printf("FAIL assertion_count got=%d want>=60\n", g_assertions);
        return 1;
    }
    if (g_failures != 0) {
        printf("FAIL dm1_v1_champion_panel_food_water_status_box_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_champion_panel_food_water_status_box_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
