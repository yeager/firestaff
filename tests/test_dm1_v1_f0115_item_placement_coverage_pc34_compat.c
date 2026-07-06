#include "dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat.h"
#include "dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat.h"
#include "dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.h"
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"

#include <stdio.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void test_d0_back_cell_only(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    expect_int("d0.count",
               (int)dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8005/8115 D0L/D0R F0115 calls");
    expect_int("d0l.present", d0l != NULL, 1,
               "ReDMCSB DUNVIEW.C:7960-8062 F0125");
    expect_int("d0r.present", d0r != NULL, 1,
               "ReDMCSB DUNVIEW.C:8064-8162 F0126");

    expect_int("d0l.cell.count",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(
                   d0l ? d0l->f0115_cell_order : 0),
               1, "ReDMCSB DUNVIEW.C:4547-4581 low-nibble walk");
    expect_int("d0r.cell.count",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(
                   d0r ? d0r->f0115_cell_order : 0),
               1, "ReDMCSB DUNVIEW.C:4547-4581 low-nibble walk");
    expect_int("d0l.named.first.back.right",
               d0l ? d0l->f0115_first_processed_cell : -1,
               2, "ReDMCSB DEFS.H:2660 C0x0002_CELL_ORDER_BACKRIGHT");
    expect_int("d0r.named.first.back.left",
               d0r ? d0r->f0115_first_processed_cell : -1,
               3, "ReDMCSB DEFS.H:2659 C0x0001_CELL_ORDER_BACKLEFT");
    expect_int("d0l.decoded.first.back.right",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(
                   d0l ? d0l->f0115_cell_order : 0, 0),
               1, "ReDMCSB DUNVIEW.C:4826 M001_ORDINAL_TO_INDEX");
    expect_int("d0r.decoded.first.back.left",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(
                   d0r ? d0r->f0115_cell_order : 0, 0),
               0, "ReDMCSB DUNVIEW.C:4826 M001_ORDINAL_TO_INDEX");

    expect_int("d0l.reject.front.left",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 0),
               0, "ReDMCSB DUNVIEW.C:4547-4581 front cell not iterated");
    expect_int("d0l.reject.front.right",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 1),
               0, "ReDMCSB DUNVIEW.C:4547-4581 front cell not iterated");
    expect_int("d0r.reject.front.left",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0r, 0),
               0, "ReDMCSB DUNVIEW.C:4547-4581 front cell not iterated");
    expect_int("d0r.reject.front.right",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0r, 1),
               0, "ReDMCSB DUNVIEW.C:4547-4581 front cell not iterated");
    expect_int("d0l.quarter.accept.back.right",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0l, 2),
               1, "ReDMCSB DUNVIEW.C:5295 D0L BACK_RIGHT gate");
    expect_int("d0r.quarter.accept.back.left",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0r, 3),
               1, "ReDMCSB DUNVIEW.C:5295 D0R BACK_LEFT gate");
}

static void test_d1_all_cells_kept_and_routes_are_split(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *d1l =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(4);
    const DM1V1D1LD1RF0115LanePc34Data *d1r =
        dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_for_view_square_pc34(5);
    int cell;

    expect_int("d1.count",
               (int)dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8524-8529 D1L/D1R dispatch");
    expect_int("d1l.present", d1l != NULL, 1,
               "ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L");
    expect_int("d1r.present", d1r != NULL, 1,
               "ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R");

    for (cell = 0; cell < 4; ++cell) {
        expect_int("d1.depth1.keeps.cell",
                   dm1_v1_viewport_d1l_d1r_f0115_thing_pass_depth1_keeps_cell_pc34(cell),
                   1, "ReDMCSB DUNVIEW.C:4920-4923 depth-1 item cells");
        expect_int("d1l.first.thing.slot",
                   dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(d1l, cell),
                   2 + cell, "ReDMCSB DEFS.H:2547-2549 M550_FIRST_THING");
        expect_int("d1r.first.thing.slot",
                   dm1_v1_viewport_d1l_d1r_f0115_thing_pass_first_thing_slot_pc34(d1r, cell),
                   2 + cell, "ReDMCSB DEFS.H:2547-2549 M550_FIRST_THING");
    }

    expect_int("d1.wall.rejected",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(0),
               1, "ReDMCSB DUNVIEW.C:7475/7644 wall returns before F0115");
    expect_int("d1.stairs.side.rejected",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_rejects_element_pc34(18),
               1, "ReDMCSB DUNVIEW.C:7466/7633 stairs-side returns");
    expect_int("d1.door.front.accepted",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(17),
               1, "ReDMCSB DUNVIEW.C:7494/7662 door-front F0115 pass1");
    expect_int("d1.corridor.accepted",
               dm1_v1_viewport_d1l_d1r_f0115_thing_pass_accepts_element_pc34(1),
               1, "ReDMCSB DUNVIEW.C:7536/7704 corridor F0115 pass");
}

static void test_d2_side_squares_never_borrow_d2c_item_zones(void)
{
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2l2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D2L2D2R2F0115ThingPassPc34 *d2r2 =
        dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_for_square_pc34(2);
    int cell;

    expect_int("d2.count",
               (int)dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:6837-6896 D2L2/D2R2 no-F0115 route");
    expect_int("d2l2.no.f0115", d2l2 ? d2l2->no_f0115_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6837-6865 F0678 has no F0115 call");
    expect_int("d2r2.no.f0115", d2r2 ? d2r2->no_f0115_contract : 0, 1,
               "ReDMCSB DUNVIEW.C:6868-6896 F0679 has no F0115 call");
    expect_int("d2l2.d2c.non.interference",
               d2l2 ? d2l2->d2c_f0115_non_interference : 0, 1,
               "ReDMCSB DUNVIEW.C:7244-7388 D2C owns 0x3421 route");
    expect_int("d2r2.d2c.non.interference",
               d2r2 ? d2r2->d2c_f0115_non_interference : 0, 1,
               "ReDMCSB DUNVIEW.C:7244-7388 D2C owns 0x3421 route");
    for (cell = 0; cell < 4; ++cell) {
        expect_int("d2l2.item.zone.none",
                   dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2l2, cell),
                   -1, "ReDMCSB DUNVIEW.C:6837-6865 no object-zone route");
        expect_int("d2r2.item.zone.none",
                   dm1_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2r2, cell),
                   -1, "ReDMCSB DUNVIEW.C:6868-6896 no object-zone route");
    }
}

static void test_d3_front_cells_clipped_back_cells_zoned(void)
{
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3l2 =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(1);
    const DM1_V1_D3L2D3R2F0115ThingPassPc34 *d3r2 =
        dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_for_square_pc34(2);

    expect_int("d3.count",
               (int)dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:6235-6357 D3L2/D3R2 routes");
    expect_int("d3l2.present", d3l2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:6235-6290 F0676");
    expect_int("d3r2.present", d3r2 != NULL, 1,
               "ReDMCSB DUNVIEW.C:6293-6357 F0677");

    expect_int("d3l2.front.left.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(d3l2, 0),
               0, "ReDMCSB DUNVIEW.C:4923 depth-3 front cells clipped");
    expect_int("d3l2.front.right.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(d3l2, 1),
               0, "ReDMCSB DUNVIEW.C:4923 depth-3 front cells clipped");
    expect_int("d3r2.front.left.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(d3r2, 0),
               0, "ReDMCSB DUNVIEW.C:4923 depth-3 front cells clipped");
    expect_int("d3r2.front.right.clipped",
               dm1_v1_viewport_d3l2_d3r2_f0115_cell_visible_pc34(d3r2, 1),
               0, "ReDMCSB DUNVIEW.C:4923 depth-3 front cells clipped");

    expect_int("d3l2.back.right.item.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(d3l2, 2),
               2514, "ReDMCSB DUNVIEW.C:5075 C2500 + row*4 + cell");
    expect_int("d3l2.back.left.item.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(d3l2, 3),
               2515, "ReDMCSB DUNVIEW.C:5075 C2500 + row*4 + cell");
    expect_int("d3r2.back.right.item.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(d3r2, 2),
               2518, "ReDMCSB DUNVIEW.C:5075 C2500 + row*4 + cell");
    expect_int("d3r2.back.left.item.zone",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(d3r2, 3),
               2519, "ReDMCSB DUNVIEW.C:5075 C2500 + row*4 + cell");
    expect_int("d3l2.front.item.zone.none",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(d3l2, 0),
               -1, "ReDMCSB DUNVIEW.C:4923 clipped cells do not bind zones");
    expect_int("d3r2.front.item.zone.none",
               dm1_v1_viewport_d3l2_d3r2_f0115_item_zone_pc34(d3r2, 1),
               -1, "ReDMCSB DUNVIEW.C:4923 clipped cells do not bind zones");
}

int main(void)
{
    test_d0_back_cell_only();
    test_d1_all_cells_kept_and_routes_are_split();
    test_d2_side_squares_never_borrow_d2c_item_zones();
    test_d3_front_cells_clipped_back_cells_zoned();

    if (g_failures) {
        printf("FAIL dm1_v1_f0115_item_placement_coverage_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_f0115_item_placement_coverage_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
