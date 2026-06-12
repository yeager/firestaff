#include "dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

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

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_accessors_and_contract_markers(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l;
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r;

    dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_init_pc34();
    d0l = dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    d0r = dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    expect_int("count",
               (int)dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8005/8115 D0L/D0R thing-pass routes");
    expect_int("spec_count",
               (int)dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_count_pc34(),
               2, "ReDMCSB DUNVIEW.C:8005/8115 D0L/D0R thing-pass routes");
    expect_int("d0l.present", d0l != NULL, 1,
               "ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L");
    expect_int("d0r.present", d0r != NULL, 1,
               "ReDMCSB DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R");
    expect_int("unknown.null",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(9) == NULL,
               1, "ReDMCSB DEFS.H:2596-2606 only side=1/2 are this gate");
    expect_int("at0.d0l",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_at_pc34(0) == d0l,
               1, "ReDMCSB DUNVIEW.C:8005 left fixture first");
    expect_int("at1.d0r",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_at_pc34(1) == d0r,
               1, "ReDMCSB DUNVIEW.C:8115 right fixture second");
    expect_int("past.end.null",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_at_pc34(2) == NULL,
               1, "contract-only accessor bounds");
    expect_int("spec_for_side.d0l",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_for_side_pc34(1) == d0l,
               1, "ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("spec_for_side.d0r",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_for_side_pc34(2) == d0r,
               1, "ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("spec_for_side.unknown.null",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_spec_for_side_pc34(0) == NULL,
               1, "contract-only accessor bounds");
    expect_int("d0l.contract_only", d0l ? d0l->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d0r.contract_only", d0r ? d0r->source_locked_contract_only : 0,
               1, "ReDMCSB DUNVIEW.C:4547 F0115 contract-only marker");
    expect_int("d0l.no_asset_parity", d0l ? d0l->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d0r.no_asset_parity", d0r ? d0r->no_real_asset_bitmap_parity : 0,
               1, "contract-only no real-asset bitmap parity");
    expect_int("d0l.no_game_data", d0l ? d0l->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
    expect_int("d0r.no_game_data", d0r ? d0r->no_game_data_load : 0,
               1, "contract-only synthetic fixture");
}

static void test_element_dispatch_matrix(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    /* ReDMCSB: DUNVIEW.C F0125/F0126 dispatch matrix; only PIT,
     * CORRIDOR, DOOR_SIDE, TELEPORTER call F0115. WALL and
     * STAIRS_SIDE return before F0115. */
    expect_int("d0l.wall.no_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_WALL_PC34),
               0, "ReDMCSB DUNVIEW.C:8007-8038 WALL returns before F0115");
    expect_int("d0l.pit.calls_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34),
               1, "ReDMCSB DUNVIEW.C:7996-8005 PIT falls through to F0115");
    expect_int("d0l.corridor.calls_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_CORRIDOR_PC34),
               1, "ReDMCSB DUNVIEW.C:7996-8005 CORRIDOR calls F0115");
    expect_int("d0l.door_side.calls_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_DOOR_SIDE_PC34),
               1, "ReDMCSB DUNVIEW.C:7996-8005 DOOR_SIDE calls F0115");
    expect_int("d0l.teleporter.calls_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34),
               1, "ReDMCSB DUNVIEW.C:7996-8005 TELEPORTER calls F0115");
    expect_int("d0l.stairs_side.no_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_STAIRS_SIDE_PC34),
               0, "ReDMCSB DUNVIEW.C:7972-7988 STAIRS_SIDE returns before F0115");
    expect_int("d0l.door_front.not_applicable",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_DOOR_FRONT_PC34),
               0, "D0L has no DOOR_FRONT element (that is D1C)");

    expect_int("d0r.wall.no_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0r, DM1_V1_D0L_D0R_F0115_ELEMENT_WALL_PC34),
               0, "ReDMCSB DUNVIEW.C:8117-8144 WALL returns before F0115");
    expect_int("d0r.pit.calls_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0r, DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34),
               1, "ReDMCSB DUNVIEW.C:8106-8115 PIT calls F0115");
    expect_int("d0r.stairs_side.no_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0r, DM1_V1_D0L_D0R_F0115_ELEMENT_STAIRS_SIDE_PC34),
               0, "ReDMCSB DUNVIEW.C:8083-8100 STAIRS_SIDE returns");
    expect_int("d0r.teleporter.calls_f0115",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(
                   d0r, DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34),
               1, "ReDMCSB DUNVIEW.C:8106-8115 TELEPORTER calls F0115");

    /* Return-before guards. */
    expect_int("d0l.wall.returns",
               dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_WALL_PC34),
               1, "ReDMCSB DUNVIEW.C:8007-8038 WALL return");
    expect_int("d0l.stairs_side.returns",
               dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_STAIRS_SIDE_PC34),
               1, "ReDMCSB DUNVIEW.C:7972-7988 STAIRS_SIDE return");
    expect_int("d0l.pit.no_return",
               dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(
                   d0l, DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34),
               0, "ReDMCSB DUNVIEW.C:7996-8005 PIT falls through to F0115");
    expect_int("d0r.wall.returns",
               dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(
                   d0r, DM1_V1_D0L_D0R_F0115_ELEMENT_WALL_PC34),
               1, "ReDMCSB DUNVIEW.C:8117-8144 WALL return");

    /* Invalid input guard. */
    expect_int("null.element",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("bad.element",
               dm1_v1_viewport_d0l_d0r_f0115_element_calls_f0115_pc34(d0l, 9),
               -1, "invalid element index");
    expect_int("null.element.returns",
               dm1_v1_viewport_d0l_d0r_f0115_element_returns_before_pc34(NULL, 0),
               -1, "invalid input guard");
}

static void test_f0112_f0113_dispatch_and_call_ordering(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    /* ReDMCSB: DUNVIEW.C F0112 ceiling-pit is invoked at lines
     * 8003/8113, BEFORE F0115 at lines 8005/8115. */
    expect_int("d0l.f0112.line", d0l ? d0l->f0112_call_line : -1, 8003,
               "ReDMCSB DUNVIEW.C:8003 F0112_DUNGEONVIEW_DrawCeilingPit");
    expect_int("d0r.f0112.line", d0r ? d0r->f0112_call_line : -1, 8113,
               "ReDMCSB DUNVIEW.C:8113 F0112_DUNGEONVIEW_DrawCeilingPit");
    expect_int("d0l.f0115.line", d0l ? d0l->f0115_call_line : -1, 8005,
               "ReDMCSB DUNVIEW.C:8005 F0115 call site");
    expect_int("d0r.f0115.line", d0r ? d0r->f0115_call_line : -1, 8115,
               "ReDMCSB DUNVIEW.C:8115 F0115 call site");
    expect_int("d0l.f0113.line.range_ok",
               d0l ? (d0l->f0113_call_line >= 8050 && d0l->f0113_call_line <= 8059) : 0,
               1, "ReDMCSB DUNVIEW.C:8050-8059 F0113_DUNGEONVIEW_DrawField");
    expect_int("d0r.f0113.line.range_ok",
               d0r ? (d0r->f0113_call_line >= 8150 && d0r->f0113_call_line <= 8159) : 0,
               1, "ReDMCSB DUNVIEW.C:8150-8159 F0113_DUNGEONVIEW_DrawField");

    /* F0112 < F0115: ceiling-pit comes before thing-pass. */
    expect_int("d0l.f0112.before.f0115",
               d0l ? (d0l->f0112_call_line < d0l->f0115_call_line) : 0, 1,
               "ReDMCSB DUNVIEW.C:8003 < 8005");
    expect_int("d0r.f0112.before.f0115",
               d0r ? (d0r->f0112_call_line < d0r->f0115_call_line) : 0, 1,
               "ReDMCSB DUNVIEW.C:8113 < 8115");

    /* F0115 < F0113: thing-pass comes before teleporter field. */
    expect_int("d0l.f0115.before.f0113",
               d0l ? (d0l->f0115_call_line < d0l->f0113_call_line) : 0, 1,
               "ReDMCSB DUNVIEW.C:8005 < 8050-8059");
    expect_int("d0r.f0115.before.f0113",
               d0r ? (d0r->f0115_call_line < d0r->f0113_call_line) : 0, 1,
               "ReDMCSB DUNVIEW.C:8115 < 8150-8159");

    /* F0113 is dispatched only for TELEPORTER. */
    expect_int("d0l.f0113.teleporter_only",
               d0l ? d0l->element_dispatches_f0113_field[DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34] : -1,
               1, "ReDMCSB DUNVIEW.C:8050-8059 TELEPORTER-gated F0113 call");
    expect_int("d0l.f0113.pit_disabled",
               d0l ? d0l->element_dispatches_f0113_field[DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34] : -1,
               0, "ReDMCSB DUNVIEW.C PIT does not reach F0113");
    expect_int("d0l.f0113.corridor_disabled",
               d0l ? d0l->element_dispatches_f0113_field[DM1_V1_D0L_D0R_F0115_ELEMENT_CORRIDOR_PC34] : -1,
               0, "ReDMCSB DUNVIEW.C CORRIDOR does not reach F0113");
    expect_int("d0l.f0113.door_side_disabled",
               d0l ? d0l->element_dispatches_f0113_field[DM1_V1_D0L_D0R_F0115_ELEMENT_DOOR_SIDE_PC34] : -1,
               0, "ReDMCSB DUNVIEW.C DOOR_SIDE does not reach F0113");
    expect_int("d0r.f0113.teleporter_only",
               d0r ? d0r->element_dispatches_f0113_field[DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34] : -1,
               1, "ReDMCSB DUNVIEW.C:8150-8159 TELEPORTER-gated F0113 call");
    expect_int("d0r.f0113.pit_disabled",
               d0r ? d0r->element_dispatches_f0113_field[DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34] : -1,
               0, "ReDMCSB DUNVIEW.C PIT does not reach F0113");

    /* F0112 is dispatched for PIT/CORRIDOR/DOOR_SIDE/TELEPORTER. */
    expect_int("d0l.f0112.teleporter",
               d0l ? d0l->element_dispatches_f0112_ceiling[DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34] : -1,
               1, "ReDMCSB DUNVIEW.C:8003 TELEPORTER ceiling-pit");
    expect_int("d0l.f0112.pit",
               d0l ? d0l->element_dispatches_f0112_ceiling[DM1_V1_D0L_D0R_F0115_ELEMENT_PIT_PC34] : -1,
               1, "ReDMCSB DUNVIEW.C:8003 PIT ceiling-pit");
    expect_int("d0l.f0112.wall_disabled",
               d0l ? d0l->element_dispatches_f0112_ceiling[DM1_V1_D0L_D0R_F0115_ELEMENT_WALL_PC34] : -1,
               0, "ReDMCSB DUNVIEW.C WALL returns before F0112");
    expect_int("d0r.f0112.teleporter",
               d0r ? d0r->element_dispatches_f0112_ceiling[DM1_V1_D0L_D0R_F0115_ELEMENT_TELEPORTER_PC34] : -1,
               1, "ReDMCSB DUNVIEW.C:8113 TELEPORTER ceiling-pit");
}

static void test_single_cell_order_contract(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    /* The D0L/D0R F0115 cell-order constants are SINGLE-nibble:
     *   0x0002 = BACK_RIGHT only (one cell, no front cells)
     *   0x0001 = BACK_LEFT only (one cell, no front cells)
     * This is the unique "front cell order" aspect: the F0115
     * nibble-walk terminates after the first non-zero nibble, so no
     * front cell ordinal is ever iterated. */
    expect_int("decode.d0l.cell_count",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(0x0002u),
               1, "ReDMCSB DUNVIEW.C:4547-4581 single non-zero nibble");
    expect_int("decode.d0r.cell_count",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(0x0001u),
               1, "ReDMCSB DUNVIEW.C:4547-4581 single non-zero nibble");

    /* Ordinal 0 (the first/only cell). The decode returns the
     * 0-based processed-cell ordinal (nibble value minus 1): nibble
     * 2 → ordinal 1, nibble 1 → ordinal 0. This matches the
     * existing D0L2/D0R2 decode convention at DEFS.H:2659-2660. */
    expect_int("decode.d0l.ordinal0",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(0x0002u, 0),
               1, "ReDMCSB DUNVIEW.C:4547-4581 nibble 2 → ordinal 1");
    expect_int("decode.d0r.ordinal0",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(0x0001u, 0),
               0, "ReDMCSB DUNVIEW.C:4547-4581 nibble 1 → ordinal 0");

    /* Ordinal 1+ returns -1 (no second cell). */
    expect_int("decode.d0l.ordinal1.terminator",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(0x0002u, 1),
               -1, "ReDMCSB DUNVIEW.C:4561-4564 zero nibble terminates");
    expect_int("decode.d0r.ordinal1.terminator",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(0x0001u, 1),
               -1, "ReDMCSB DUNVIEW.C:4561-4564 zero nibble terminates");

    /* Door-pass markers terminate. */
    expect_int("decode.door.pass.marker8",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(0x0018u),
               0, "ReDMCSB DUNVIEW.C:4563 door-pass marker 8");
    expect_int("decode.door.pass.marker9",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_count_pc34(0x0039u),
               0, "ReDMCSB DUNVIEW.C:4563 door-pass marker 9");

    /* Bad ordinal. */
    expect_int("decode.bad.ordinal.high",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(0x0001u, 4),
               -1, "decode guard");
    expect_int("decode.bad.ordinal.negative",
               dm1_v1_viewport_d0l_d0r_f0115_processed_cell_ordinal_pc34(0x0001u, -1),
               -1, "decode guard");

    /* The single-cell-order contract: each spec records
     * f0115_processed_cells == 1 and the first cell is the BACK cell
     * for that side. */
    expect_int("d0l.processed_cells", d0l ? d0l->f0115_processed_cells : -1, 1,
               "ReDMCSB DUNVIEW.C:8005 single-nibble 0x0002");
    expect_int("d0r.processed_cells", d0r ? d0r->f0115_processed_cells : -1, 1,
               "ReDMCSB DUNVIEW.C:8115 single-nibble 0x0001");
    expect_int("d0l.first_cell", d0l ? d0l->f0115_first_processed_cell : -1, 2,
               "ReDMCSB DEFS.H:2644 BACK_RIGHT");
    expect_int("d0r.first_cell", d0r ? d0r->f0115_first_processed_cell : -1, 3,
               "ReDMCSB DEFS.H:2645 BACK_LEFT");
    expect_int("d0l.cell_order", d0l ? (int)d0l->f0115_cell_order : -1, 0x0002,
               "ReDMCSB DUNVIEW.C:8005; DEFS.H:2660");
    expect_int("d0r.cell_order", d0r ? (int)d0r->f0115_cell_order : -1, 0x0001,
               "ReDMCSB DUNVIEW.C:8115; DEFS.H:2659");
}

static void test_front_cell_ordinal_exclusion(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581; the D0L/D0R cell-order
     * constant is single-nibble BACK only, so the FRONT_LEFT (0) and
     * FRONT_RIGHT (1) view cells are never iterated by the thing
     * pass on these side lanes. */
    expect_int("d0l.front_left_absent",
               d0l ? d0l->f0115_front_left_ordinal_present : -1, 0,
               "ReDMCSB DEFS.H:2642 FRONT_LEFT not in 0x0002");
    expect_int("d0l.front_right_absent",
               d0l ? d0l->f0115_front_right_ordinal_present : -1, 0,
               "ReDMCSB DEFS.H:2643 FRONT_RIGHT not in 0x0002");
    expect_int("d0l.back_left_absent",
               d0l ? d0l->f0115_back_left_ordinal_present : -1, 0,
               "ReDMCSB DEFS.H:2645 BACK_LEFT not in 0x0002");
    expect_int("d0l.back_right_present",
               d0l ? d0l->f0115_back_right_ordinal_present : -1, 1,
               "ReDMCSB DEFS.H:2644 BACK_RIGHT in 0x0002");

    expect_int("d0r.front_left_absent",
               d0r ? d0r->f0115_front_left_ordinal_present : -1, 0,
               "ReDMCSB DEFS.H:2642 FRONT_LEFT not in 0x0001");
    expect_int("d0r.front_right_absent",
               d0r ? d0r->f0115_front_right_ordinal_present : -1, 0,
               "ReDMCSB DEFS.H:2643 FRONT_RIGHT not in 0x0001");
    expect_int("d0r.back_left_present",
               d0r ? d0r->f0115_back_left_ordinal_present : -1, 1,
               "ReDMCSB DEFS.H:2645 BACK_LEFT in 0x0001");
    expect_int("d0r.back_right_absent",
               d0r ? d0r->f0115_back_right_ordinal_present : -1, 0,
               "ReDMCSB DEFS.H:2644 BACK_RIGHT not in 0x0001");

    /* Spec accessor for front-cell-presence. */
    expect_int("d0l.front_left.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 0),
               0, "ReDMCSB DUNVIEW.C:8005 front cell never iterated");
    expect_int("d0l.front_right.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 1),
               0, "ReDMCSB DUNVIEW.C:8005 front cell never iterated");
    expect_int("d0l.back_right.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 2),
               1, "ReDMCSB DUNVIEW.C:8005 BACK_RIGHT iterated");
    expect_int("d0l.back_left.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 3),
               0, "ReDMCSB DUNVIEW.C:8005 BACK_LEFT not iterated");
    expect_int("d0r.front_left.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0r, 0),
               0, "ReDMCSB DUNVIEW.C:8115 front cell never iterated");
    expect_int("d0r.front_right.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0r, 1),
               0, "ReDMCSB DUNVIEW.C:8115 front cell never iterated");
    expect_int("d0r.back_left.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0r, 3),
               1, "ReDMCSB DUNVIEW.C:8115 BACK_LEFT iterated");
    expect_int("d0r.back_right.present.accessor",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0r, 2),
               0, "ReDMCSB DUNVIEW.C:8115 BACK_RIGHT not iterated");

    /* Bad input guards. */
    expect_int("null.front_cell",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(NULL, 0),
               -1, "invalid input guard");
    expect_int("bad.cell",
               dm1_v1_viewport_d0l_d0r_f0115_front_cell_ordinal_present_pc34(d0l, 9),
               -1, "invalid view cell");
}

static void test_creature_quarter_cell_gate(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    /* ReDMCSB: DUNVIEW.C F0115 line 5295 quarter-creature cell gate:
     * D0L only accepts BACK_RIGHT (cell 2), D0R only accepts
     * BACK_LEFT (cell 3). All other cells (including the front
     * cells 0/1) are rejected for the quarter-square creature. */
    expect_int("d0l.qcell.back_right.accepted",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0l, 2),
               1, "ReDMCSB DUNVIEW.C:5295 D0L BACK_RIGHT accepted");
    expect_int("d0l.qcell.back_left.rejected",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0l, 3),
               0, "ReDMCSB DUNVIEW.C:5295 D0L BACK_LEFT rejected");
    expect_int("d0l.qcell.front_left.rejected",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0l, 0),
               0, "ReDMCSB DUNVIEW.C:5295 D0L FRONT_LEFT rejected");
    expect_int("d0l.qcell.front_right.rejected",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0l, 1),
               0, "ReDMCSB DUNVIEW.C:5295 D0L FRONT_RIGHT rejected");
    expect_int("d0r.qcell.back_left.accepted",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0r, 3),
               1, "ReDMCSB DUNVIEW.C:5295 D0R BACK_LEFT accepted");
    expect_int("d0r.qcell.back_right.rejected",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0r, 2),
               0, "ReDMCSB DUNVIEW.C:5295 D0R BACK_RIGHT rejected");
    expect_int("d0r.qcell.front_left.rejected",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0r, 0),
               0, "ReDMCSB DUNVIEW.C:5295 D0R FRONT_LEFT rejected");
    expect_int("d0r.qcell.front_right.rejected",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(d0r, 1),
               0, "ReDMCSB DUNVIEW.C:5295 D0R FRONT_RIGHT rejected");

    /* Per-spec reject flags. */
    expect_int("d0l.qcell.br_only", d0l ? d0l->creature_quarter_cell_back_right_only : 0, 1,
               "ReDMCSB DUNVIEW.C:5295 D0L BACK_RIGHT-only");
    expect_int("d0l.qcell.not_bl", d0l ? d0l->creature_quarter_cell_back_left_only : 0, 0,
               "ReDMCSB DUNVIEW.C:5295 D0L not BACK_LEFT");
    expect_int("d0r.qcell.bl_only", d0r ? d0r->creature_quarter_cell_back_left_only : 0, 1,
               "ReDMCSB DUNVIEW.C:5295 D0R BACK_LEFT-only");
    expect_int("d0r.qcell.not_br", d0r ? d0r->creature_quarter_cell_back_right_only : 0, 0,
               "ReDMCSB DUNVIEW.C:5295 D0R not BACK_RIGHT");
    expect_int("d0l.qcell.reject_fl", d0l ? d0l->creature_zone_rejects_front_left : 0, 1,
               "ReDMCSB DUNVIEW.C:5295 D0L FRONT_LEFT rejected");
    expect_int("d0l.qcell.reject_fr", d0l ? d0l->creature_zone_rejects_front_right : 0, 1,
               "ReDMCSB DUNVIEW.C:5295 D0L FRONT_RIGHT rejected");
    expect_int("d0r.qcell.reject_fl", d0r ? d0r->creature_zone_rejects_front_left : 0, 1,
               "ReDMCSB DUNVIEW.C:5295 D0R FRONT_LEFT rejected");
    expect_int("d0r.qcell.reject_fr", d0r ? d0r->creature_zone_rejects_front_right : 0, 1,
               "ReDMCSB DUNVIEW.C:5295 D0R FRONT_RIGHT rejected");

    /* Bad input. */
    expect_int("null.qcell",
               dm1_v1_viewport_d0l_d0r_f0115_creature_quarter_cell_accepted_pc34(NULL, 2),
               -1, "invalid input guard");
}

static void test_dispatch_order_and_view_square_lane_metadata(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    /* D0L dispatched before D0R. */
    expect_int("dispatch.d0l_first",
               (d0l ? d0l->view_square_index : 9) < (d0r ? d0r->view_square_index : -1),
               1, "ReDMCSB DUNVIEW.C:8536-8541 F0128 D0L then D0R");
    expect_int("d0l.dispatched_marker", d0l ? d0l->dispatched_after_d0l_before_d0r : 0, 1,
               "ReDMCSB DUNVIEW.C:8536-8537 D0L first");
    expect_int("d0r.dispatched_marker", d0r ? d0r->dispatched_after_d0l_before_d0r : 0, 1,
               "ReDMCSB DUNVIEW.C:8540-8541 D0R second");

    /* View-square / depth / lane metadata. */
    expect_int("d0l.view_square", d0l ? d0l->view_square_index : -1, 1,
               "ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("d0r.view_square", d0r ? d0r->view_square_index : -1, 2,
               "ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("d0l.depth", d0l ? d0l->view_depth : -1, 0,
               "ReDMCSB DUNVIEW.C:372 G2027[1]");
    expect_int("d0r.depth", d0r ? d0r->view_depth : -1, 0,
               "ReDMCSB DUNVIEW.C:372 G2027[2]");
    expect_int("d0l.lane", d0l ? d0l->view_lane : 99, -1,
               "ReDMCSB DUNVIEW.C:371 G2026[1]");
    expect_int("d0r.lane", d0r ? d0r->view_lane : 99, 1,
               "ReDMCSB DUNVIEW.C:371 G2026[2]");
    expect_int("d0l.side", d0l ? d0l->side : -1, 1,
               "ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L");
    expect_int("d0r.side", d0r ? d0r->side : -1, 2,
               "ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R");
    expect_int("d0l.f0163_guard", d0l ? d0l->f0163_not_called_by_draw : 0, 1,
               "ReDMCSB DUNGEON.C:1769-1838 F0163");
    expect_int("d0r.f0164_guard", d0r ? d0r->f0164_not_called_by_draw : 0, 1,
               "ReDMCSB DUNGEON.C:1840-1905 F0164");
    expect_int("d0l.f0172_source", d0l ? d0l->f0172_square_aspect_source : 0, 1,
               "ReDMCSB DUNGEON.C:2466-2523 F0172");
    expect_int("d0r.f0172_source", d0r ? d0r->f0172_square_aspect_source : 0, 1,
               "ReDMCSB DUNGEON.C:2466-2523 F0172");
    expect_int("mutation.d0l",
               dm1_v1_viewport_d0l_d0r_f0115_is_draw_mutating_pc34(d0l),
               0, "ReDMCSB DUNGEON.C:1769-1838 F0163 not called by draw");
    expect_int("mutation.d0r",
               dm1_v1_viewport_d0l_d0r_f0115_is_draw_mutating_pc34(d0r),
               0, "ReDMCSB DUNGEON.C:1840-1905 F0164 not called by draw");
    expect_int("mutation.null",
               dm1_v1_viewport_d0l_d0r_f0115_is_draw_mutating_pc34(NULL),
               -1, "invalid input guard");
}

static void test_f0112_f0115_f0113_composition(void)
{
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);
    DM1_V1_D0LD0RF0115FrontCellOrderTracePc34 trace;

    /* C10-transparent: ceiling-pit transparent leaves base unchanged. */
    expect_int("compose.d0l.f0112.transparent",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   d0l, 0x33, 10, 0x22, 0x11, &trace),
               0, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("compose.d0l.f0112.transparent.after", trace.after_f0112_ceiling_pit, 0x33,
               "ReDMCSB DEFS.H:2088 C10 transparent preserve");
    expect_int("compose.d0l.f0112.transparent.flag", trace.f0112_ceiling_pit_transparent, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    /* Normal pass: F0112 → F0115 → F0113. */
    expect_int("compose.d0l.opaque",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   d0l, 0x33, 0x44, 0x55, 0x66, &trace),
               0, "ReDMCSB DUNVIEW.C:8003/8005/8050-8059 sequence");
    expect_int("compose.d0l.after_f0112", trace.after_f0112_ceiling_pit, 0x44,
               "ReDMCSB DUNVIEW.C:8003 ceiling-pit blend");
    expect_int("compose.d0l.after_f0115", trace.after_f0115_thing_pass, 0x55,
               "ReDMCSB DUNVIEW.C:8005 thing-pass blend");
    expect_int("compose.d0l.after_f0113", trace.after_f0113_field, 0x66,
               "ReDMCSB DUNVIEW.C:8050-8059 field blend");
    expect_int("compose.d0l.f0112_calls", trace.f0112_ceiling_pit_calls, 1,
               "ReDMCSB DUNVIEW.C:8003 one F0112 call");
    expect_int("compose.d0l.f0115_calls", trace.f0115_calls, 1,
               "ReDMCSB DUNVIEW.C:8005 one F0115 call");
    expect_int("compose.d0l.f0113_calls", trace.f0113_field_calls, 1,
               "ReDMCSB DUNVIEW.C:8050-8059 one F0113 call");
    expect_int("compose.d0l.f0112_before_f0115", trace.f0112_f0115_order_ok, 1,
               "ReDMCSB DUNVIEW.C:8003 < 8005");
    expect_int("compose.d0l.f0115_before_f0113", trace.f0115_f0113_order_ok, 1,
               "ReDMCSB DUNVIEW.C:8005 < 8050-8059");
    expect_int("compose.d0l.rejected_front", trace.rejected_front_cell_ordinal, 1,
               "ReDMCSB DUNVIEW.C:4547-4581 front cell never iterated");
    expect_int("compose.d0l.accepted_back", trace.accepted_back_cell_ordinal, 1,
               "ReDMCSB DUNVIEW.C:8005 BACK_RIGHT iterated");

    /* D0R mirrors the order with 0x0001 BACK_LEFT. */
    expect_int("compose.d0r.opaque",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   d0r, 0x33, 0x44, 0x55, 0x66, &trace),
               0, "ReDMCSB DUNVIEW.C:8113/8115/8150-8159 sequence");
    expect_int("compose.d0r.after_f0112", trace.after_f0112_ceiling_pit, 0x44,
               "ReDMCSB DUNVIEW.C:8113 ceiling-pit blend");
    expect_int("compose.d0r.after_f0115", trace.after_f0115_thing_pass, 0x55,
               "ReDMCSB DUNVIEW.C:8115 thing-pass blend");
    expect_int("compose.d0r.after_f0113", trace.after_f0113_field, 0x66,
               "ReDMCSB DUNVIEW.C:8150-8159 field blend");

    /* C10-transparent: F0115 transparent preserves F0112. */
    expect_int("compose.d0l.f0115.transparent",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   d0l, 0x33, 0x44, 10, 0x66, &trace),
               0, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH F0115");
    expect_int("compose.d0l.f0115.transparent.after", trace.after_f0115_thing_pass, 0x44,
               "ReDMCSB DUNVIEW.C:8005 C10 transparent preserve");
    expect_int("compose.d0l.f0115.transparent.flag", trace.f0115_thing_pass_transparent, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    /* C10-transparent: F0113 transparent preserves F0115. */
    expect_int("compose.d0l.f0113.transparent",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   d0l, 0x33, 0x44, 0x55, 10, &trace),
               0, "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH F0113");
    expect_int("compose.d0l.f0113.transparent.after", trace.after_f0113_field, 0x55,
               "ReDMCSB DUNVIEW.C:8050-8059 C10 transparent preserve");
    expect_int("compose.d0l.f0113.transparent.flag", trace.f0113_field_transparent, 1,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    /* Bad input guards. */
    expect_int("compose.null.spec",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   NULL, 0, 1, 2, 3, &trace),
               -1, "invalid input guard");
    expect_int("compose.null.trace",
               dm1_v1_viewport_d0l_d0r_f0115_compose_pixel_pc34(
                   d0l, 0, 1, 2, 3, NULL),
               -1, "invalid input guard");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e = dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_source_evidence_pc34();
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0l =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(1);
    const DM1_V1_D0LD0RF0115FrontCellOrderPc34 *d0r =
        dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_for_square_pc34(2);

    expect_contains("evidence.contract_only", e, "source_locked_contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_real_asset_bitmap_parity=1",
                    "source evidence");
    expect_contains("evidence.no_game_data", e, "no_game_data_load=1",
                    "source evidence");
    expect_contains("evidence.f0125", e, "F0125_DUNGEONVIEW_DrawSquareD0L",
                    "DUNVIEW.C D0L function");
    expect_contains("evidence.f0126", e, "F0126_DUNGEONVIEW_DrawSquareD0R",
                    "DUNVIEW.C D0R function");
    expect_contains("evidence.f0112.before", e, "DUNVIEW.C F0125 line 8003",
                    "F0112 before F0115 anchor");
    expect_contains("evidence.f0115.call", e, "F0115 call at line 8005/8115",
                    "DUNVIEW.C F0115 call sites");
    expect_contains("evidence.f0113.after", e, "8050-8059",
                    "DUNVIEW.C F0113 D0L line range");
    expect_contains("evidence.f0113.d0r", e, "8150-8159",
                    "DUNVIEW.C F0113 D0R line range");
    expect_contains("evidence.f0115.5295", e, "line 5295",
                    "DUNVIEW.C F0115 quarter-creature gate");
    expect_contains("evidence.f0115.body", e, "4547-4581",
                    "DUNVIEW.C F0115 nibble-walk body");
    expect_contains("evidence.f0128", e, "8536-8541",
                    "DUNVIEW.C F0128 dispatch");
    expect_contains("evidence.f0163", e, "F0163 lines 1769-1838",
                    "DUNGEON.C F0163 mutation anchor");
    expect_contains("evidence.f0164", e, "F0164 lines 1840-1905",
                    "DUNGEON.C F0164 mutation anchor");
    expect_contains("evidence.f0172", e, "F0172 lines 2466-2523",
                    "DUNGEON.C F0172 square aspect anchor");
    expect_contains("evidence.defs.2088", e, "DEFS.H line 2088",
                    "DEFS.H C10_COLOR_FLESH");
    expect_contains("evidence.defs.2597", e, "lines 2597-2598",
                    "DEFS.H M610/M611 view squares");
    expect_contains("evidence.defs.2642", e, "lines 2642-2645",
                    "DEFS.H view-cell ordinals");
    expect_contains("evidence.defs.2659", e, "lines 2659-2660",
                    "DEFS.H cell-order constants");
    expect_contains("evidence.defs.4056", e, "lines 4056-4057",
                    "DEFS.H C716/C717 wall zones");
    expect_contains("evidence.defs.4250", e, "lines 4250-4260",
                    "DEFS.H D0 door-zone absence");
    expect_contains("d0l.anchor", d0l ? d0l->redmcsb_dispatch_anchor : NULL,
                    "F0125_DUNGEONVIEW_DrawSquareD0L", "fixture source anchor");
    expect_contains("d0r.anchor", d0r ? d0r->redmcsb_dispatch_anchor : NULL,
                    "F0126_DUNGEONVIEW_DrawSquareD0R", "fixture source anchor");
    expect_contains("d0l.f0115.anchor", d0l ? d0l->redmcsb_f0115_anchor : NULL,
                    "5295", "fixture F0115 quarter-creature gate anchor");
    expect_contains("d0r.f0115.anchor", d0r ? d0r->redmcsb_f0115_anchor : NULL,
                    "5295", "fixture F0115 quarter-creature gate anchor");
    expect_contains("d0l.dungeon.anchor", d0l ? d0l->redmcsb_dungeon_anchor : NULL,
                    "F0172", "fixture DUNGEON.C anchor");
    expect_contains("d0r.defs.anchor", d0r ? d0r->redmcsb_defs_anchor : NULL,
                    "4250-4260", "fixture DEFS.H anchor");
}

int main(void)
{
    test_accessors_and_contract_markers();
    test_element_dispatch_matrix();
    test_f0112_f0113_dispatch_and_call_ordering();
    test_single_cell_order_contract();
    test_front_cell_ordinal_exclusion();
    test_creature_quarter_cell_gate();
    test_dispatch_order_and_view_square_lane_metadata();
    test_f0112_f0115_f0113_composition();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat "
               "failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d0l_d0r_f0115_front_cell_order_pc34_compat "
           "%d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
