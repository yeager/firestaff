#include "csb/csb_v1_viewport_d0c_center_field_pc34_compat.h"

enum {
    CSB_D0C_VIEW_SQUARE = 0,
    CSB_D0C_VIEW_LANE = 0,
    CSB_D0C_VIEW_DEPTH = 0,
    CSB_D0C_FIELD_ASPECT = 13,
    CSB_D0C_TRANSPARENT_COLOR = 10,
    CSB_D0C_F0115_CELL_ORDER = 0x0021,
    CSB_D0C_DOOR_FRAME_ZONE = 728,
    CSB_D0C_STAIRS_UP_LEFT_ZONE = 811,
    CSB_D0C_STAIRS_UP_RIGHT_ZONE = 812,
    CSB_D0C_STAIRS_DOWN_LEFT_ZONE = 824,
    CSB_D0C_STAIRS_DOWN_RIGHT_ZONE = 825,
    CSB_D0C_FLOOR_PIT_ZONE = 862,
    CSB_D0C_CEILING_PIT_ZONE = 871,
    CSB_D0C_FIELD_ZONE = 715
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not real-asset bitmap parity. "
    "ReDMCSB DUNVIEW.C:8164-8310 F0127_DUNGEONVIEW_DrawSquareD0C reads "
    "the current cell with F0172, routes C16 door-side through the I34 "
    "C728 door-frame bitmap path at 8224-8235, routes C19 stairs-front "
    "through paired left F0104 and right F0105 zones C811/C812 or C824/C825 "
    "at 8248-8270, routes C02 pit through C862 at 8279-8282, always performs "
    "the D0C ceiling-pit probe through C871 at 8286-8292 and the F0115 "
    "thing pass through M609/C0x0021 at 8294, then draws C05 teleporter "
    "fields through field aspect G2035[M609] and C715 at 8302-8308. "
    "DUNVIEW.C:8537-8542 dispatches D0L, D0R, then D0C, so this D0C center "
    "gate does not duplicate the existing D0L/D0R absence boundary. "
    "DUNVIEW.C:4853-4920 keeps D0C objects on the visible/grabbable center "
    "route. DEFS.H:2596,2662,4055,4086,4150-4164,4209,4218 and "
    "CSBWin Viewport.cpp:7140-7157 bind the same current-cell slot.";

static const CSB_V1_ViewportD0CCenterFieldContractPc34 s_contract = {
    1,
    CSB_D0C_VIEW_SQUARE,
    CSB_D0C_VIEW_LANE,
    CSB_D0C_VIEW_DEPTH,
    CSB_D0C_FIELD_ASPECT,
    CSB_D0C_TRANSPARENT_COLOR,
    CSB_D0C_F0115_CELL_ORDER,
    CSB_D0C_DOOR_FRAME_ZONE,
    CSB_D0C_STAIRS_UP_LEFT_ZONE,
    CSB_D0C_STAIRS_UP_RIGHT_ZONE,
    CSB_D0C_STAIRS_DOWN_LEFT_ZONE,
    CSB_D0C_STAIRS_DOWN_RIGHT_ZONE,
    CSB_D0C_FLOOR_PIT_ZONE,
    CSB_D0C_CEILING_PIT_ZONE,
    CSB_D0C_FIELD_ZONE,
    "ReDMCSB DUNVIEW.C:8164-8310 F0127_DUNGEONVIEW_DrawSquareD0C",
    "ReDMCSB DUNVIEW.C:8537-8542 F0128 dispatches D0L/D0R before D0C",
    "ReDMCSB DUNVIEW.C:4853-4920 F0115 includes the D0C center object route",
    "ReDMCSB DEFS.H:2596,2662,4055,4086,4150-4164,4209,4218",
    "CSBWin Viewport.cpp:7140-7157 CustomBackgrounds room 15 then DrawCellF0",
    s_source_evidence
};

const CSB_V1_ViewportD0CCenterFieldContractPc34 *
csb_v1_viewport_d0c_center_field_contract_pc34(void)
{
    return &s_contract;
}

const char *
csb_v1_viewport_d0c_center_field_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static CSB_V1_ViewportD0CCenterFieldPlanPc34 base_plan(
    CSB_V1_ViewportD0CCenterFieldInputPc34 input)
{
    CSB_V1_ViewportD0CCenterFieldPlanPc34 plan = {
        1,
        CSB_V1_D0C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS,
        CSB_D0C_VIEW_SQUARE,
        CSB_D0C_FIELD_ASPECT,
        input.first_thing,
        0,
        0,
        1,
        0,
        1,
        0,
        0,
        -1,
        -1,
        -1,
        -1,
        CSB_D0C_CEILING_PIT_ZONE,
        -1,
        CSB_D0C_F0115_CELL_ORDER,
        0,
        s_source_evidence
    };
    return plan;
}

CSB_V1_ViewportD0CCenterFieldPlanPc34
csb_v1_viewport_d0c_center_field_plan_pc34(
    CSB_V1_ViewportD0CCenterFieldInputPc34 input)
{
    CSB_V1_ViewportD0CCenterFieldPlanPc34 plan = base_plan(input);

    switch (input.element) {
        case CSB_V1_D0C_CENTER_FIELD_ELEMENT_CORRIDOR:
            break;
        case CSB_V1_D0C_CENTER_FIELD_ELEMENT_DOOR_SIDE:
            plan.route = CSB_V1_D0C_CENTER_FIELD_ROUTE_DOOR_FRAME;
            plan.calls_f0104 = 1;
            plan.door_frame_zone = CSB_D0C_DOOR_FRAME_ZONE;
            break;
        case CSB_V1_D0C_CENTER_FIELD_ELEMENT_STAIRS_FRONT:
            plan.route = input.stairs_up
                ? CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_UP_PAIR
                : CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_DOWN_PAIR;
            plan.calls_f0104 = 1;
            plan.calls_f0105 = 1;
            plan.left_stairs_zone = input.stairs_up
                ? CSB_D0C_STAIRS_UP_LEFT_ZONE
                : CSB_D0C_STAIRS_DOWN_LEFT_ZONE;
            plan.right_stairs_zone = input.stairs_up
                ? CSB_D0C_STAIRS_UP_RIGHT_ZONE
                : CSB_D0C_STAIRS_DOWN_RIGHT_ZONE;
            break;
        case CSB_V1_D0C_CENTER_FIELD_ELEMENT_PIT:
            plan.route = input.pit_or_teleporter_visible
                ? CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_INVISIBLE
                : CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_VISIBLE;
            plan.calls_f0104 = 1;
            plan.floor_pit_zone = CSB_D0C_FLOOR_PIT_ZONE;
            plan.uses_invisible_pit_graphic = input.pit_or_teleporter_visible ? 1 : 0;
            break;
        case CSB_V1_D0C_CENTER_FIELD_ELEMENT_TELEPORTER:
            plan.route = CSB_V1_D0C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD;
            plan.calls_f0113 = 1;
            plan.field_zone = CSB_D0C_FIELD_ZONE;
            break;
        default:
            plan.accepted = 0;
            plan.route = CSB_V1_D0C_CENTER_FIELD_ROUTE_INVALID;
            plan.calls_f0112 = 0;
            plan.calls_f0115 = 0;
            plan.ceiling_pit_zone = -1;
            plan.f0115_cell_order = -1;
            break;
    }

    return plan;
}

const char *
csb_v1_viewport_d0c_center_field_route_name_pc34(
    CSB_V1_ViewportD0CCenterFieldRoutePc34 route)
{
    switch (route) {
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS:
            return "corridor thing pass";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_DOOR_FRAME:
            return "door frame";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_UP_PAIR:
            return "stairs up pair";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_STAIRS_DOWN_PAIR:
            return "stairs down pair";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_VISIBLE:
            return "visible floor pit";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_FLOOR_PIT_INVISIBLE:
            return "invisible floor pit";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD:
            return "teleporter field";
        case CSB_V1_D0C_CENTER_FIELD_ROUTE_INVALID:
        default:
            return "invalid";
    }
}
