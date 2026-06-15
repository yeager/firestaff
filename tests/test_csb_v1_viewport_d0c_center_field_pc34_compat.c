/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF:8537-8542
 * - DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4853-4920
 * - DEFS.H M609/C728/C811/C812/C824/C825/C862/C871/C715/C0x0021:2596,4086,4150-4164,4209,4218,4055,2662
 */

#include "csb/csb_v1_viewport_d0c_center_field_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    return expect_int(label,
                      haystack && needle && strstr(haystack, needle) != NULL,
                      1,
                      anchor);
}

static CSB_V1_ViewportD0CCenterFieldInputPc34 input_for(int element)
{
    CSB_V1_ViewportD0CCenterFieldInputPc34 in = {
        element,
        0,
        0,
        0x321
    };
    return in;
}

static int test_contract_metadata(void)
{
    int ok = 1;
    const CSB_V1_ViewportD0CCenterFieldContractPc34 *c =
        csb_v1_viewport_d0c_center_field_contract_pc34();

    ok &= expect_int("contract.non_null", c != NULL, 1,
                     "ReDMCSB DUNVIEW.C:8164-8310");
    ok &= expect_int("contract.only", c ? c->contract_only : 0, 1,
                     "source-locked contract-only gate");
    ok &= expect_int("view_square.d0c", c ? c->view_square : -1, 0,
                     "ReDMCSB DEFS.H:2596 M609_VIEW_SQUARE_D0C");
    ok &= expect_int("view_lane.d0c", c ? c->view_lane : -9, 0,
                     "ReDMCSB DUNVIEW.C:371 G2026[0]");
    ok &= expect_int("view_depth.d0c", c ? c->view_depth : -1, 0,
                     "ReDMCSB DUNVIEW.C:372 G2027[0]");
    ok &= expect_int("field_aspect.d0c", c ? c->field_aspect : -1, 13,
                     "ReDMCSB DUNVIEW.C:377 G2035[0]");
    ok &= expect_int("transparent.c10", c ? c->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("cell_order.d0c", c ? c->f0115_cell_order : -1, 0x0021,
                     "ReDMCSB DUNVIEW.C:8294; DEFS.H:2662");
    ok &= expect_int("zone.door_frame", c ? c->door_frame_zone : -1, 728,
                     "ReDMCSB DUNVIEW.C:8235; DEFS.H:4086");
    ok &= expect_int("zone.floor_pit", c ? c->floor_pit_zone : -1, 862,
                     "ReDMCSB DUNVIEW.C:8282; DEFS.H:4209");
    ok &= expect_int("zone.ceiling_pit", c ? c->ceiling_pit_zone : -1, 871,
                     "ReDMCSB DUNVIEW.C:8292; DEFS.H:4218");
    ok &= expect_int("zone.field", c ? c->field_zone : -1, 715,
                     "ReDMCSB DUNVIEW.C:8308; DEFS.H:4055");
    ok &= expect_contains("evidence.f0127", c ? c->redmcsb_f0127_anchor : NULL,
                          "F0127", "ReDMCSB DUNVIEW.C:8164");
    ok &= expect_contains("evidence.f0128", c ? c->redmcsb_f0128_anchor : NULL,
                          "D0C", "ReDMCSB DUNVIEW.C:8542");
    ok &= expect_contains("evidence.lineage", c ? c->csb_lineage_anchor : NULL,
                          "Viewport.cpp:7140-7157", "CSBWin Viewport.cpp:7140-7157");
    ok &= expect_contains("source.nondup",
                          csb_v1_viewport_d0c_center_field_source_evidence_pc34(),
                          "does not duplicate the existing D0L/D0R",
                          "ReDMCSB DUNVIEW.C:8537-8542");

    return ok;
}

static int test_door_stairs_and_pit_routes(void)
{
    int ok = 1;
    CSB_V1_ViewportD0CCenterFieldInputPc34 in = input_for(
        CSB_V1_D0C_CENTER_FIELD_ELEMENT_DOOR_SIDE);
    CSB_V1_ViewportD0CCenterFieldPlanPc34 door =
        csb_v1_viewport_d0c_center_field_plan_pc34(in);
    CSB_V1_ViewportD0CCenterFieldPlanPc34 stairs_up;
    CSB_V1_ViewportD0CCenterFieldPlanPc34 stairs_down;
    CSB_V1_ViewportD0CCenterFieldPlanPc34 pit_visible;
    CSB_V1_ViewportD0CCenterFieldPlanPc34 pit_invisible;

    ok &= expect_int("door.route", (int)door.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_DOOR_FRAME,
                     "ReDMCSB DUNVIEW.C:8219-8235");
    ok &= expect_int("door.f0104", door.calls_f0104, 1,
                     "ReDMCSB DUNVIEW.C:8235");
    ok &= expect_int("door.zone", door.door_frame_zone, 728,
                     "ReDMCSB DEFS.H:4086");
    ok &= expect_int("door.no_f0111", door.calls_f0111, 0,
                     "ReDMCSB DUNVIEW.C:8164-8310 excludes F0111");

    in = input_for(CSB_V1_D0C_CENTER_FIELD_ELEMENT_STAIRS_FRONT);
    in.stairs_up = 1;
    stairs_up = csb_v1_viewport_d0c_center_field_plan_pc34(in);
    in.stairs_up = 0;
    stairs_down = csb_v1_viewport_d0c_center_field_plan_pc34(in);

    ok &= expect_int("stairs_up.route", (int)stairs_up.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_UP_PAIR,
                     "ReDMCSB DUNVIEW.C:8244-8253");
    ok &= expect_int("stairs_up.f0104", stairs_up.calls_f0104, 1,
                     "ReDMCSB DUNVIEW.C:8252");
    ok &= expect_int("stairs_up.f0105", stairs_up.calls_f0105, 1,
                     "ReDMCSB DUNVIEW.C:8253");
    ok &= expect_int("stairs_up.left_zone", stairs_up.left_stairs_zone, 811,
                     "ReDMCSB DEFS.H:4150");
    ok &= expect_int("stairs_up.right_zone", stairs_up.right_stairs_zone, 812,
                     "ReDMCSB DEFS.H:4151");
    ok &= expect_int("stairs_down.route", (int)stairs_down.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_DOWN_PAIR,
                     "ReDMCSB DUNVIEW.C:8257-8270");
    ok &= expect_int("stairs_down.left_zone", stairs_down.left_stairs_zone, 824,
                     "ReDMCSB DEFS.H:4163");
    ok &= expect_int("stairs_down.right_zone", stairs_down.right_stairs_zone, 825,
                     "ReDMCSB DEFS.H:4164");

    in = input_for(CSB_V1_D0C_CENTER_FIELD_ELEMENT_PIT);
    in.pit_or_teleporter_visible = 0;
    pit_visible = csb_v1_viewport_d0c_center_field_plan_pc34(in);
    in.pit_or_teleporter_visible = 1;
    pit_invisible = csb_v1_viewport_d0c_center_field_plan_pc34(in);

    ok &= expect_int("pit.visible.route", (int)pit_visible.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_VISIBLE,
                     "ReDMCSB DUNVIEW.C:8276-8282");
    ok &= expect_int("pit.visible.invisible_graphic",
                     pit_visible.uses_invisible_pit_graphic, 0,
                     "ReDMCSB DUNVIEW.C:8279-8282");
    ok &= expect_int("pit.invisible.route", (int)pit_invisible.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_INVISIBLE,
                     "ReDMCSB DUNVIEW.C:8279-8282");
    ok &= expect_int("pit.invisible.graphic",
                     pit_invisible.uses_invisible_pit_graphic, 1,
                     "ReDMCSB DUNVIEW.C:8279-8282");
    ok &= expect_int("pit.zone", pit_invisible.floor_pit_zone, 862,
                     "ReDMCSB DEFS.H:4209");

    return ok;
}

static int test_common_tail_and_teleporter_route(void)
{
    int ok = 1;
    CSB_V1_ViewportD0CCenterFieldPlanPc34 corridor =
        csb_v1_viewport_d0c_center_field_plan_pc34(
            input_for(CSB_V1_D0C_CENTER_FIELD_ELEMENT_CORRIDOR));
    CSB_V1_ViewportD0CCenterFieldPlanPc34 teleporter =
        csb_v1_viewport_d0c_center_field_plan_pc34(
            input_for(CSB_V1_D0C_CENTER_FIELD_ELEMENT_TELEPORTER));
    CSB_V1_ViewportD0CCenterFieldPlanPc34 invalid =
        csb_v1_viewport_d0c_center_field_plan_pc34(input_for(99));

    ok &= expect_int("corridor.route", (int)corridor.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS,
                     "ReDMCSB DUNVIEW.C:8286-8294");
    ok &= expect_int("corridor.f0112", corridor.calls_f0112, 1,
                     "ReDMCSB DUNVIEW.C:8286-8292");
    ok &= expect_int("corridor.ceiling_zone", corridor.ceiling_pit_zone, 871,
                     "ReDMCSB DEFS.H:4218");
    ok &= expect_int("corridor.f0115", corridor.calls_f0115, 1,
                     "ReDMCSB DUNVIEW.C:8294");
    ok &= expect_int("corridor.first_thing", corridor.first_thing, 0x321,
                     "ReDMCSB DUNVIEW.C:8294 M550_FIRST_THING");
    ok &= expect_int("corridor.cell_order", corridor.f0115_cell_order, 0x0021,
                     "ReDMCSB DEFS.H:2662");
    ok &= expect_int("corridor.no_f0107", corridor.calls_f0107, 0,
                     "ReDMCSB DUNVIEW.C:8164-8310 excludes F0107");
    ok &= expect_int("corridor.no_f0111", corridor.calls_f0111, 0,
                     "ReDMCSB DUNVIEW.C:8164-8310 excludes F0111");

    ok &= expect_int("teleporter.route", (int)teleporter.route,
                     CSB_V1_D0C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD,
                     "ReDMCSB DUNVIEW.C:8302-8308");
    ok &= expect_int("teleporter.f0113", teleporter.calls_f0113, 1,
                     "ReDMCSB DUNVIEW.C:8308");
    ok &= expect_int("teleporter.field_zone", teleporter.field_zone, 715,
                     "ReDMCSB DEFS.H:4055");
    ok &= expect_int("teleporter.field_aspect", teleporter.field_aspect, 13,
                     "ReDMCSB DUNVIEW.C:377 G2035[0]");
    ok &= expect_int("teleporter.f0115_precedes_field", teleporter.calls_f0115, 1,
                     "ReDMCSB DUNVIEW.C:8294 before 8302-8308");

    ok &= expect_int("invalid.accepted", invalid.accepted, 0,
                     "contract rejects non-D0C elements");
    ok &= expect_int("invalid.no_tail_f0115", invalid.calls_f0115, 0,
                     "invalid route does not claim ReDMCSB tail");
    ok &= expect_contains("route.name.teleporter",
                          csb_v1_viewport_d0c_center_field_route_name_pc34(
                              CSB_V1_D0C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD),
                          "teleporter", "route helper");

    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_contract_metadata();
    ok &= test_door_stairs_and_pit_routes();
    ok &= test_common_tail_and_teleporter_route();

    if (!ok || g_failures) {
        printf("FAIL csb_v1_viewport_d0c_center_field_pc34_compat "
               "assertions=%d failures=%d\n", g_assertions, g_failures);
        return 1;
    }

    printf("PASS csb_v1_viewport_d0c_center_field_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
