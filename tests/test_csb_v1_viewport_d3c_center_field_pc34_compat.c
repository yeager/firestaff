/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0118_DUNGEONVIEW_DrawSquareD3C_CPSF:6642-6833
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8490-8499
 * - DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4853-4920
 * CSB-lineage anchors:
 * - Viewport.cpp F3 draw-code slot and DrawCellF3 fallback:2057-2066,4110-4240
 * - Viewport.cpp RF3 relative (3,0) dispatch:6972-6984
 */

#include "csb_v1_viewport_d3c_center_field_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_nonnull(const char *id, const void *got, const char *anchor)
{
    ++g_assertions;
    if (!got) {
        printf("FAIL %s got=NULL at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonnull (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static CSB_V1_D3CCenterFieldInputPc34 base_input(int element)
{
    CSB_V1_D3CCenterFieldInputPc34 in = {
        element,
        false,
        false,
        false,
        false,
        0x4321
    };
    return in;
}

static void test_contract_metadata(void)
{
    const CSB_V1_D3CCenterFieldContractPc34 *c =
        csb_v1_viewport_d3c_center_field_contract_pc34();

    expect_nonnull("d3c.contract", c, "ReDMCSB DUNVIEW.C:6642-6833");
    if (!c) return;

    expect_bool("d3c.contract_only", c->contract_only, true,
                "contract-only synthetic gate");
    expect_int("d3c.view_square_index", c->view_square_index, 11,
               "ReDMCSB DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    expect_int("d3c.macro_view_square",
               CSB_V1_D3C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX, 11,
               "ReDMCSB DEFS.H:2607 M600_VIEW_SQUARE_D3C");
    expect_int("d3c.view_lane", c->view_lane, 0,
               "ReDMCSB DUNVIEW.C:371 G2026[11]");
    expect_int("d3c.view_depth", c->view_depth, 3,
               "ReDMCSB DUNVIEW.C:372 G2027[11]");
    expect_int("d3c.field_aspect", c->field_aspect, 2,
               "ReDMCSB DUNVIEW.C:377 G2035[11]");
    expect_int("d3c.wall_zone", c->wall_zone, 704,
               "ReDMCSB DEFS.H:4044 C704_ZONE_WALL_D3C");
    expect_int("d3c.door_frame_left_zone", c->door_frame_left_zone, 722,
               "ReDMCSB DEFS.H:4080 C722_ZONE_DOOR_FRAME_LEFT_D3C");
    expect_int("d3c.door_frame_right_zone", c->door_frame_right_zone, 723,
               "ReDMCSB DEFS.H:4081 C723_ZONE_DOOR_FRAME_RIGHT_D3C");
    expect_int("d3c.door_zone", c->door_zone, 3730,
               "ReDMCSB DEFS.H:4253 M625_ZONE_DOOR_D3C");
    expect_int("d3c.stairs_up_zone", c->stairs_up_front_zone, 803,
               "ReDMCSB DEFS.H:4142 C803_ZONE_STAIRS_UP_FRONT_D3C");
    expect_int("d3c.stairs_down_zone", c->stairs_down_front_zone, 816,
               "ReDMCSB DEFS.H:4155 C816_ZONE_STAIRS_DOWN_FRONT_D3C");
    expect_int("d3c.floor_pit_zone", c->floor_pit_zone, 853,
               "ReDMCSB DEFS.H:4200 C853_ZONE_FLOORPIT_D3C");
    expect_int("d3c.order_alcove", c->cell_order_alcove, 0x0000,
               "ReDMCSB DEFS.H:2658");
    expect_int("d3c.order_door_pass1", c->cell_order_door_pass1, 0x0218,
               "ReDMCSB DEFS.H:2669");
    expect_int("d3c.order_door_pass2", c->cell_order_door_pass2, 0x0349,
               "ReDMCSB DEFS.H:2672");
    expect_int("d3c.order_open", c->cell_order_open, 0x3421,
               "ReDMCSB DEFS.H:2676");
    expect_int("d3c.transparent_color", c->transparent_color, 10,
               "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    expect_contains("d3c.source_lock_gate",
                    CSB_V1_D3C_CENTER_FIELD_PC34_SOURCE_LOCK_GATE,
                    "parity-csb-d3c-center-field",
                    "CSB D3C source-lock gate marker");
}

static void test_frame_and_source_anchors(void)
{
    const CSB_V1_D3CCenterFieldContractPc34 *c =
        csb_v1_viewport_d3c_center_field_contract_pc34();
    const char *e = csb_v1_viewport_d3c_center_field_source_evidence_pc34();

    expect_nonnull("d3c.frame.contract", c, "ReDMCSB DUNVIEW.C:583");
    if (!c) return;

    expect_int("d3c.frame.x1", c->wall_frame.x1, 74, "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.x2", c->wall_frame.x2, 149, "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.y1", c->wall_frame.y1, 25, "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.y2", c->wall_frame.y2, 75, "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.byte_width", c->wall_frame.byte_width, 64,
               "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.height", c->wall_frame.height, 51,
               "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.blit_x", c->wall_frame.blit_x, 18,
               "ReDMCSB DUNVIEW.C:583");
    expect_int("d3c.frame.blit_y", c->wall_frame.blit_y, 0,
               "ReDMCSB DUNVIEW.C:583");
    expect_contains("d3c.anchor.f0118", c->redmcsb_f0118_anchor, "6642-6833",
                    "ReDMCSB DUNVIEW.C:6642-6833");
    expect_contains("d3c.anchor.f0128", c->redmcsb_f0128_anchor, "relative (3,0)",
                    "ReDMCSB DUNVIEW.C:8490-8499");
    expect_contains("d3c.anchor.f0115", c->redmcsb_f0115_anchor, "M600_VIEW_SQUARE_D3C",
                    "ReDMCSB DUNVIEW.C:4853-4920");
    expect_contains("d3c.anchor.defs", c->redmcsb_defs_anchor, "2607",
                    "ReDMCSB DEFS.H:2607");
    expect_contains("d3c.anchor.lineage", c->csb_lineage_anchor, "Viewport.cpp:2057-2066",
                    "CSB-lineage Viewport.cpp:2057-2066");
    expect_contains("d3c.anchor.non_lineage_marker",
                    c->non_lineage_replacement_marker,
                    "NON_CSB_LINEAGE_REPLACEMENT",
                    "non-CSB-lineage-replacement marker");
    expect_contains("d3c.evidence.source_lock", e, "Source-locked contract gate only",
                    "contract-only marker");
    expect_contains("d3c.evidence.f0128", e, "relative (3,0)",
                    "ReDMCSB DUNVIEW.C:8490-8499");
    expect_contains("d3c.evidence.lineage_drawcell", e, "DrawCellF3",
                    "CSB-lineage Viewport.cpp:4110-4240");
    expect_contains("d3c.evidence.non_replacement", e,
                    "NON_CSB_LINEAGE_REPLACEMENT",
                    "non-CSB-lineage-replacement marker");
    expect_contains("d3c.evidence.gate_marker", e,
                    CSB_V1_D3C_CENTER_FIELD_PC34_SOURCE_LOCK_GATE,
                    "CSB D3C source-lock gate marker");
}

static void test_wall_and_door_routes(void)
{
    CSB_V1_D3CCenterFieldInputPc34 in = base_input(
        CSB_V1_D3C_CENTER_FIELD_ELEMENT_WALL);
    CSB_V1_D3CCenterFieldPlanPc34 wall_plain;
    CSB_V1_D3CCenterFieldPlanPc34 wall_alcove;
    CSB_V1_D3CCenterFieldPlanPc34 door;

    wall_plain = csb_v1_viewport_d3c_center_field_plan_pc34(in);
    in.wall_ornament_is_alcove = true;
    wall_alcove = csb_v1_viewport_d3c_center_field_plan_pc34(in);

    in = base_input(CSB_V1_D3C_CENTER_FIELD_ELEMENT_DOOR_FRONT);
    in.door_has_button = true;
    door = csb_v1_viewport_d3c_center_field_plan_pc34(in);

    expect_int("d3c.wall_plain.route", (int)wall_plain.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_NO_ALCOVE,
               "ReDMCSB DUNVIEW.C:6697-6720");
    expect_bool("d3c.wall_plain.f0100", wall_plain.calls_f0100_wall, true,
                "ReDMCSB DUNVIEW.C:6699-6714");
    expect_bool("d3c.wall_plain.f0107", wall_plain.calls_f0107_alcove_probe, true,
                "ReDMCSB DUNVIEW.C:6716");
    expect_bool("d3c.wall_plain.no_f0115", wall_plain.f0115_call_count == 0, true,
                "ReDMCSB DUNVIEW.C:6716-6720 return");
    expect_bool("d3c.wall_plain.no_floor", wall_plain.calls_f0108_floor_ornament, false,
                "ReDMCSB DUNVIEW.C:6719-6720");
    expect_bool("d3c.wall_plain.returns", wall_plain.wall_returns_before_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:6720");
    expect_int("d3c.wall_plain.zone", wall_plain.wall_zone, 704,
               "ReDMCSB DEFS.H:4044");

    expect_int("d3c.wall_alcove.route", (int)wall_alcove.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_ALCOVE_THING_PASS,
               "ReDMCSB DUNVIEW.C:6716-6718");
    expect_int("d3c.wall_alcove.order", wall_alcove.first_cell_order, 0x0000,
               "ReDMCSB DEFS.H:2658");
    expect_int("d3c.wall_alcove.f0115_count", wall_alcove.f0115_call_count, 1,
               "ReDMCSB DUNVIEW.C:6815-6816");
    expect_bool("d3c.wall_alcove.no_field", wall_alcove.calls_f0113_field, false,
                "ReDMCSB DUNVIEW.C:6818-6833 teleporter-only field");

    expect_int("d3c.door.route", (int)door.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_DOOR_FRONT_DOUBLE_PASS,
               "ReDMCSB DUNVIEW.C:6721-6747");
    expect_bool("d3c.door.f0108", door.calls_f0108_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:6722");
    expect_int("d3c.door.first_order", door.first_cell_order, 0x0218,
               "ReDMCSB DUNVIEW.C:6723; DEFS.H:2669");
    expect_int("d3c.door.second_order", door.second_cell_order, 0x0349,
               "ReDMCSB DUNVIEW.C:6746; DEFS.H:2672");
    expect_int("d3c.door.f0115_count", door.f0115_call_count, 2,
               "ReDMCSB DUNVIEW.C:6723,6815-6816");
    expect_bool("d3c.door.f0104", door.calls_f0104, true,
                "ReDMCSB DUNVIEW.C:6734");
    expect_bool("d3c.door.f0105", door.calls_f0105, true,
                "ReDMCSB DUNVIEW.C:6735");
    expect_int("d3c.door.left_frame_zone", door.door_frame_left_zone, 722,
               "ReDMCSB DEFS.H:4080");
    expect_int("d3c.door.right_frame_zone", door.door_frame_right_zone, 723,
               "ReDMCSB DEFS.H:4081");
    expect_bool("d3c.door.button", door.calls_f0110_door_button, true,
                "ReDMCSB DUNVIEW.C:6737-6738");
    expect_bool("d3c.door.f0111", door.calls_f0111_door, true,
                "ReDMCSB DUNVIEW.C:6741-6744");
    expect_int("d3c.door.zone", door.door_zone, 3730,
               "ReDMCSB DEFS.H:4253 M625_ZONE_DOOR_D3C");
}

static void test_door_without_button_variant(void)
{
    CSB_V1_D3CCenterFieldInputPc34 in = base_input(
        CSB_V1_D3C_CENTER_FIELD_ELEMENT_DOOR_FRONT);
    CSB_V1_D3CCenterFieldPlanPc34 door = csb_v1_viewport_d3c_center_field_plan_pc34(in);

    expect_int("d3c.door_no_button.route", (int)door.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_DOOR_FRONT_DOUBLE_PASS,
               "ReDMCSB DUNVIEW.C:6721-6747");
    expect_bool("d3c.door_no_button.f0110", door.calls_f0110_door_button, false,
               "ReDMCSB DUNVIEW.C:6737-6738");
    expect_bool("d3c.door_no_button.f0111", door.calls_f0111_door, true,
               "ReDMCSB DUNVIEW.C:6741-6744");
    expect_int("d3c.door_no_button.order1", door.first_cell_order, 0x0218,
               "ReDMCSB DUNVIEW.C:6723");
    expect_int("d3c.door_no_button.order2", door.second_cell_order, 0x0349,
               "ReDMCSB DUNVIEW.C:6746");
    expect_int("d3c.door_no_button.f0115_count", door.f0115_call_count, 2,
               "ReDMCSB DUNVIEW.C:6723,6815-6816");
}

static void test_open_stairs_pit_and_teleporter_routes(void)
{
    CSB_V1_D3CCenterFieldPlanPc34 corridor =
        csb_v1_viewport_d3c_center_field_plan_pc34(
            base_input(CSB_V1_D3C_CENTER_FIELD_ELEMENT_CORRIDOR));
    CSB_V1_D3CCenterFieldInputPc34 in =
        base_input(CSB_V1_D3C_CENTER_FIELD_ELEMENT_STAIRS_FRONT);
    CSB_V1_D3CCenterFieldPlanPc34 stairs_down;
    CSB_V1_D3CCenterFieldPlanPc34 stairs_up;
    CSB_V1_D3CCenterFieldPlanPc34 pit_visible;
    CSB_V1_D3CCenterFieldPlanPc34 pit_invisible;
    CSB_V1_D3CCenterFieldPlanPc34 teleporter;
    CSB_V1_D3CCenterFieldPlanPc34 invalid;

    stairs_down = csb_v1_viewport_d3c_center_field_plan_pc34(in);
    in.stairs_up = true;
    stairs_up = csb_v1_viewport_d3c_center_field_plan_pc34(in);

    in = base_input(CSB_V1_D3C_CENTER_FIELD_ELEMENT_PIT);
    pit_visible = csb_v1_viewport_d3c_center_field_plan_pc34(in);
    in.pit_or_teleporter_visible = true;
    pit_invisible = csb_v1_viewport_d3c_center_field_plan_pc34(in);

    teleporter = csb_v1_viewport_d3c_center_field_plan_pc34(
        base_input(CSB_V1_D3C_CENTER_FIELD_ELEMENT_TELEPORTER));
    invalid = csb_v1_viewport_d3c_center_field_plan_pc34(base_input(99));

    expect_int("d3c.corridor.route", (int)corridor.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS,
               "ReDMCSB DUNVIEW.C:6811-6816");
    expect_bool("d3c.corridor.f0108", corridor.calls_f0108_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:6814");
    expect_int("d3c.corridor.order", corridor.first_cell_order, 0x3421,
               "ReDMCSB DUNVIEW.C:6813; DEFS.H:2676");
    expect_int("d3c.corridor.f0115_count", corridor.f0115_call_count, 1,
               "ReDMCSB DUNVIEW.C:6815-6816");
    expect_int("d3c.corridor.first_thing", corridor.first_thing, 0x4321,
               "ReDMCSB DUNVIEW.C:6816 M550_FIRST_THING");
    expect_bool("d3c.corridor.no_f0113", corridor.calls_f0113_field, false,
                "ReDMCSB DUNVIEW.C:6818-6833 teleporter only");

    expect_int("d3c.stairs_down.route", (int)stairs_down.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_DOWN_THING_PASS,
               "ReDMCSB DUNVIEW.C:6677-6696");
    expect_int("d3c.stairs_down.zone", stairs_down.stairs_zone, 816,
               "ReDMCSB DEFS.H:4155");
    expect_bool("d3c.stairs_down.f0104", stairs_down.calls_f0104, true,
                "ReDMCSB DUNVIEW.C:6693");
    expect_int("d3c.stairs_down.order", stairs_down.first_cell_order, 0x3421,
               "ReDMCSB DUNVIEW.C:6813");
    expect_int("d3c.stairs_up.route", (int)stairs_up.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_UP_THING_PASS,
               "ReDMCSB DUNVIEW.C:6666-6696");
    expect_int("d3c.stairs_up.zone", stairs_up.stairs_zone, 803,
               "ReDMCSB DEFS.H:4142");
    expect_bool("d3c.stairs_up.no_f0113", stairs_up.calls_f0113_field, false,
                "ReDMCSB DUNVIEW.C:6818-6833 teleporter only");

    expect_int("d3c.pit_visible.route", (int)pit_visible.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_VISIBLE_THING_PASS,
               "ReDMCSB DUNVIEW.C:6748-6763");
    expect_bool("d3c.pit_visible.bitmap", pit_visible.pit_uses_floor_pit_bitmap, true,
                "ReDMCSB DUNVIEW.C:6749-6760");
    expect_int("d3c.pit_visible.zone", pit_visible.floor_pit_zone, 853,
               "ReDMCSB DEFS.H:4200");
    expect_int("d3c.pit_visible.order", pit_visible.first_cell_order, 0x3421,
               "ReDMCSB DUNVIEW.C:6813");
    expect_int("d3c.pit_invisible.route", (int)pit_invisible.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_INVISIBLE_THING_PASS,
               "ReDMCSB DUNVIEW.C:6749 skip visible-pit bitmap");
    expect_bool("d3c.pit_invisible.no_bitmap", pit_invisible.pit_uses_floor_pit_bitmap, false,
                "ReDMCSB DUNVIEW.C:6749-6760");
    expect_int("d3c.pit_invisible.no_zone", pit_invisible.floor_pit_zone, -1,
               "synthetic visible pit branch");

    expect_int("d3c.teleporter.route", (int)teleporter.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD,
               "ReDMCSB DUNVIEW.C:6763-6833");
    expect_bool("d3c.teleporter.f0108", teleporter.calls_f0108_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:6814");
    expect_int("d3c.teleporter.order", teleporter.first_cell_order, 0x3421,
               "ReDMCSB DUNVIEW.C:6813");
    expect_int("d3c.teleporter.f0115_count", teleporter.f0115_call_count, 1,
               "ReDMCSB DUNVIEW.C:6815-6816");
    expect_bool("d3c.teleporter.f0113", teleporter.calls_f0113_field, true,
                "ReDMCSB DUNVIEW.C:6824-6831");
    expect_int("d3c.teleporter.field_zone", teleporter.field_zone, 704,
               "ReDMCSB DEFS.H:4044 C704_ZONE_WALL_D3C");
    expect_bool("d3c.teleporter.after_pass",
                teleporter.teleporter_field_after_thing_pass, true,
                "ReDMCSB DUNVIEW.C:6815-6833");

    expect_bool("d3c.invalid.accepted", invalid.accepted, false,
                "synthetic contract rejects non-D3C element");
    expect_int("d3c.invalid.route", (int)invalid.route,
               CSB_V1_D3C_CENTER_FIELD_ROUTE_INVALID,
               "synthetic contract rejects non-D3C element");
    expect_int("d3c.invalid.f0115_count", invalid.f0115_call_count, 0,
               "invalid route does not claim ReDMCSB tail");
}

static void test_synthetic_c10_and_route_names(void)
{
    const CSB_V1_D3CCenterFieldContractPc34 *c =
        csb_v1_viewport_d3c_center_field_contract_pc34();
    uint8_t source[10] = { 10, 1, 2, 10, 3, 4, 10, 5, 6, 10 };
    uint8_t dest[10] = { 77, 77, 77, 77, 77, 77, 77, 77, 77, 77 };

    expect_nonnull("d3c.c10.contract", c, "ReDMCSB DEFS.H:2088");
    if (!c) return;

    expect_int("d3c.c10.copied",
               csb_v1_viewport_d3c_center_field_apply_synthetic_c10_field_pc34(
                   c, source, 5, dest, 5, 5, 2),
               6, "synthetic C10 F0113 field contract");
    expect_int("d3c.c10.transparent0", dest[0], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d3c.c10.pixel1", dest[1], 1, "synthetic field copy");
    expect_int("d3c.c10.pixel2", dest[2], 2, "synthetic field copy");
    expect_int("d3c.c10.transparent3", dest[3], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d3c.c10.pixel4", dest[4], 3, "synthetic field copy");
    expect_int("d3c.c10.pixel5", dest[5], 4, "synthetic field copy");
    expect_int("d3c.c10.transparent6", dest[6], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d3c.c10.pixel7", dest[7], 5, "synthetic field copy");
    expect_int("d3c.c10.pixel8", dest[8], 6, "synthetic field copy");
    expect_int("d3c.c10.transparent9", dest[9], 77,
               "ReDMCSB DEFS.H:2088 C10 transparent");
    expect_int("d3c.c10.reject_null",
               csb_v1_viewport_d3c_center_field_apply_synthetic_c10_field_pc34(
                   NULL, source, 5, dest, 5, 5, 2),
               -1, "synthetic helper rejects missing contract");
    expect_int("d3c.c10.reject_stride",
               csb_v1_viewport_d3c_center_field_apply_synthetic_c10_field_pc34(
                   c, source, 4, dest, 5, 5, 2),
               -1, "synthetic helper rejects narrow source stride");
    expect_contains("d3c.route_name.teleporter",
                    csb_v1_viewport_d3c_center_field_route_name_pc34(
                        CSB_V1_D3C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD),
                    "teleporter", "route-name helper");
    expect_contains("d3c.route_name.door",
                    csb_v1_viewport_d3c_center_field_route_name_pc34(
                        CSB_V1_D3C_CENTER_FIELD_ROUTE_DOOR_FRONT_DOUBLE_PASS),
                    "door", "route-name helper");
    expect_contains("d3c.route_name.invalid",
                    csb_v1_viewport_d3c_center_field_route_name_pc34(
                        CSB_V1_D3C_CENTER_FIELD_ROUTE_INVALID),
                    "invalid", "route-name helper");
}

int main(void)
{
    test_contract_metadata();
    test_frame_and_source_anchors();
    test_wall_and_door_routes();
    test_door_without_button_variant();
    test_open_stairs_pit_and_teleporter_routes();
    test_synthetic_c10_and_route_names();

    if (g_failures) {
        printf("csb_v1_viewport_d3c_center_field_pc34_compat: %d failures / %d assertions\n",
               g_failures, g_assertions);
        return 1;
    }

    printf("csb_v1_viewport_d3c_center_field_pc34_compat: %d assertions passed\n",
           g_assertions);
    return 0;
}
