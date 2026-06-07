#include "csb_v1_viewport_d3c_center_field_pc34_compat.h"

/*
 * Source-locked contract gate only; not a real-asset bitmap parity test.
 */

enum {
    /* ReDMCSB: DEFS.H:2607 defines I34 M600_VIEW_SQUARE_D3C. */
    CSB_RED_M600_VIEW_SQUARE_D3C = 11,
    /* ReDMCSB: DUNVIEW.C:371-377 maps I34 D3C to lane 0, depth 3, field aspect 2. */
    CSB_RED_D3C_LANE = 0,
    CSB_RED_D3C_DEPTH = 3,
    CSB_RED_D3C_FIELD_ASPECT = 2,
    /* ReDMCSB: DEFS.H:4044 defines C704_ZONE_WALL_D3C. */
    CSB_RED_C704_ZONE_WALL_D3C = 704,
    /* ReDMCSB: DEFS.H:4080-4081 define I34 D3C door-frame zones. */
    CSB_RED_C722_ZONE_DOOR_FRAME_LEFT_D3C = 722,
    CSB_RED_C723_ZONE_DOOR_FRAME_RIGHT_D3C = 723,
    /* ReDMCSB: DEFS.H:4253 defines I34 M625_ZONE_DOOR_D3C. */
    CSB_RED_M625_ZONE_DOOR_D3C = 3730,
    /* ReDMCSB: DEFS.H:4142/4155/4200 define I34 D3C stairs and pit zones. */
    CSB_RED_C803_ZONE_STAIRS_UP_FRONT_D3C = 803,
    CSB_RED_C816_ZONE_STAIRS_DOWN_FRONT_D3C = 816,
    CSB_RED_C853_ZONE_FLOORPIT_D3C = 853,
    /* ReDMCSB: DEFS.H:2658,2669,2672,2676 define D3C thing-pass orders. */
    CSB_RED_C0X0000_CELL_ORDER_ALCOVE = 0x0000,
    CSB_RED_C0X0218_CELL_ORDER_DOORPASS1 = 0x0218,
    CSB_RED_C0X0349_CELL_ORDER_DOORPASS2 = 0x0349,
    CSB_RED_C0X3421_CELL_ORDER_OPEN = 0x3421,
    /* ReDMCSB: DEFS.H:2088 defines C10_COLOR_FLESH. */
    CSB_RED_C10_COLOR_FLESH = 10
};

static const char s_non_lineage_marker[] =
    "NON_CSB_LINEAGE_REPLACEMENT: CSB-lineage Viewport.cpp is a cross-reference; "
    "ReDMCSB DUNVIEW.C remains the source-lock for this Firestaff gate.";

static const char s_source_evidence[] =
    "Source-locked contract gate only; not a real-asset bitmap parity test. "
    "ReDMCSB DUNVIEW.C:8490-8499 F0128 updates map coordinates by relative "
    "(3,0) and dispatches F0118_DUNGEONVIEW_DrawSquareD3C_CPSF. "
    "DUNVIEW.C:6642-6833 F0118 reads square aspect, draws D3C wall/stairs/"
    "door/pit/open/teleporter cases, uses C704 for wall and field, and uses "
    "I34 zones C722/C723, C803, C816, C853, and M625. "
    "DUNVIEW.C:6697-6720 wall returns unless F0107 finds an alcove; the "
    "alcove path uses C0x0000 at 6716-6718 then F0115 at 6815-6816. "
    "DUNVIEW.C:6721-6747 door front performs F0108 and F0115 with C0x0218 "
    "before door-frame/button/F0111 drawing, then performs a second F0115 "
    "with C0x0349. DUNVIEW.C:6748-6816 makes pit and teleporter fall through "
    "to the C0x3421 open thing pass, and DUNVIEW.C:6818-6833 draws the D3C "
    "teleporter field after that pass. DUNVIEW.C:4853-4920 includes I34 "
    "M600_VIEW_SQUARE_D3C objects in the visible object route. "
    "DEFS.H:2607,4044,4080-4081,4142,4155,4200,4253,2658,2669,2672,2676 "
    "anchor the constants. CSB-lineage Viewport.cpp:2057-2066 keeps F3 "
    "as a distinct draw-code slot, Viewport.cpp:4110-4240 mirrors the F3 "
    "open/pit/teleporter/door/stair cases, and Viewport.cpp:6972-6984 "
    "dispatches relative (3,0) to RF3/DrawCellF3. "
    "Gate marker parity-csb-d3c-center-field binds this CSB-specific D3C slice. "
    "NON_CSB_LINEAGE_REPLACEMENT marker: the lineage source cross-checks the "
    "CSB route but does not replace the ReDMCSB source-lock.";

static const CSB_V1_D3CCenterFieldContractPc34 s_contract = {
    true,
    CSB_RED_M600_VIEW_SQUARE_D3C,
    CSB_RED_D3C_LANE,
    CSB_RED_D3C_DEPTH,
    CSB_RED_D3C_FIELD_ASPECT,
    CSB_RED_C704_ZONE_WALL_D3C,
    CSB_RED_C722_ZONE_DOOR_FRAME_LEFT_D3C,
    CSB_RED_C723_ZONE_DOOR_FRAME_RIGHT_D3C,
    CSB_RED_M625_ZONE_DOOR_D3C,
    CSB_RED_C803_ZONE_STAIRS_UP_FRONT_D3C,
    CSB_RED_C816_ZONE_STAIRS_DOWN_FRONT_D3C,
    CSB_RED_C853_ZONE_FLOORPIT_D3C,
    CSB_RED_C0X0000_CELL_ORDER_ALCOVE,
    CSB_RED_C0X0218_CELL_ORDER_DOORPASS1,
    CSB_RED_C0X0349_CELL_ORDER_DOORPASS2,
    CSB_RED_C0X3421_CELL_ORDER_OPEN,
    CSB_RED_C10_COLOR_FLESH,
    { 74, 149, 25, 75, 64, 51, 18, 0 },
    "ReDMCSB DUNVIEW.C:6642-6833 F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
    "ReDMCSB DUNVIEW.C:8490-8499 F0128 relative (3,0) D3C dispatch",
    "ReDMCSB DUNVIEW.C:4853-4920 F0115 includes M600_VIEW_SQUARE_D3C",
    "ReDMCSB DEFS.H:2607,4044,4080-4081,4142,4155,4200,4253,2658,2669,2672,2676",
    "CSB-lineage Viewport.cpp:2057-2066; 4110-4240; 6972-6984",
    s_non_lineage_marker,
    s_source_evidence
};

const CSB_V1_D3CCenterFieldContractPc34 *
csb_v1_viewport_d3c_center_field_contract_pc34(void)
{
    return &s_contract;
}

const char *
csb_v1_viewport_d3c_center_field_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static CSB_V1_D3CCenterFieldPlanPc34 base_plan(
    CSB_V1_D3CCenterFieldInputPc34 input)
{
    CSB_V1_D3CCenterFieldPlanPc34 plan = {
        true,
        CSB_V1_D3C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS,
        CSB_RED_M600_VIEW_SQUARE_D3C,
        CSB_RED_D3C_FIELD_ASPECT,
        input.first_thing,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        CSB_RED_C0X3421_CELL_ORDER_OPEN,
        -1,
        1,
        false,
        false,
        false,
        false,
        true,
        false,
        false,
        false,
        false,
        false,
        false,
        s_source_evidence
    };
    return plan;
}

CSB_V1_D3CCenterFieldPlanPc34
csb_v1_viewport_d3c_center_field_plan_pc34(
    CSB_V1_D3CCenterFieldInputPc34 input)
{
    CSB_V1_D3CCenterFieldPlanPc34 plan = base_plan(input);

    switch (input.element) {
        case CSB_V1_D3C_CENTER_FIELD_ELEMENT_WALL:
            plan.calls_f0100_wall = true;
            plan.calls_f0107_alcove_probe = true;
            plan.wall_zone = CSB_RED_C704_ZONE_WALL_D3C;
            plan.calls_f0108_floor_ornament = false;
            plan.first_cell_order = -1;
            plan.f0115_call_count = 0;
            plan.wall_returns_before_floor_ornament = true;
            if (input.wall_ornament_is_alcove) {
                plan.route = CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_ALCOVE_THING_PASS;
                plan.first_cell_order = CSB_RED_C0X0000_CELL_ORDER_ALCOVE;
                plan.f0115_call_count = 1;
                plan.wall_returns_before_floor_ornament = false;
            } else {
                plan.route = CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_NO_ALCOVE;
            }
            break;
        case CSB_V1_D3C_CENTER_FIELD_ELEMENT_DOOR_FRONT:
            plan.route = CSB_V1_D3C_CENTER_FIELD_ROUTE_DOOR_FRONT_DOUBLE_PASS;
            plan.calls_f0104 = true;
            plan.calls_f0105 = true;
            plan.calls_f0110_door_button = input.door_has_button;
            plan.calls_f0111_door = true;
            plan.door_frame_left_zone = CSB_RED_C722_ZONE_DOOR_FRAME_LEFT_D3C;
            plan.door_frame_right_zone = CSB_RED_C723_ZONE_DOOR_FRAME_RIGHT_D3C;
            plan.door_zone = CSB_RED_M625_ZONE_DOOR_D3C;
            plan.first_cell_order = CSB_RED_C0X0218_CELL_ORDER_DOORPASS1;
            plan.second_cell_order = CSB_RED_C0X0349_CELL_ORDER_DOORPASS2;
            plan.f0115_call_count = 2;
            break;
        case CSB_V1_D3C_CENTER_FIELD_ELEMENT_STAIRS_FRONT:
            plan.route = input.stairs_up
                ? CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_UP_THING_PASS
                : CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_DOWN_THING_PASS;
            plan.calls_f0104 = true;
            plan.stairs_zone = input.stairs_up
                ? CSB_RED_C803_ZONE_STAIRS_UP_FRONT_D3C
                : CSB_RED_C816_ZONE_STAIRS_DOWN_FRONT_D3C;
            break;
        case CSB_V1_D3C_CENTER_FIELD_ELEMENT_PIT:
            plan.route = input.pit_or_teleporter_visible
                ? CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_INVISIBLE_THING_PASS
                : CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_VISIBLE_THING_PASS;
            plan.calls_f0104 = !input.pit_or_teleporter_visible;
            plan.pit_uses_floor_pit_bitmap = !input.pit_or_teleporter_visible;
            plan.floor_pit_zone = input.pit_or_teleporter_visible
                ? -1
                : CSB_RED_C853_ZONE_FLOORPIT_D3C;
            break;
        case CSB_V1_D3C_CENTER_FIELD_ELEMENT_TELEPORTER:
            plan.route = CSB_V1_D3C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD;
            plan.calls_f0113_field = true;
            plan.field_zone = CSB_RED_C704_ZONE_WALL_D3C;
            plan.teleporter_field_after_thing_pass = true;
            break;
        case CSB_V1_D3C_CENTER_FIELD_ELEMENT_CORRIDOR:
            break;
        default:
            plan.accepted = false;
            plan.route = CSB_V1_D3C_CENTER_FIELD_ROUTE_INVALID;
            plan.calls_f0108_floor_ornament = false;
            plan.first_cell_order = -1;
            plan.f0115_call_count = 0;
            break;
    }

    return plan;
}

int csb_v1_viewport_d3c_center_field_apply_synthetic_c10_field_pc34(
    const CSB_V1_D3CCenterFieldContractPc34 *contract,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;
    if (!contract || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C:6818-6833 reaches F0113 for D3C teleporter fields;
     * DEFS.H:2088 C10_COLOR_FLESH remains transparent in the synthetic gate. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)contract->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *
csb_v1_viewport_d3c_center_field_route_name_pc34(
    CSB_V1_D3CCenterFieldRoutePc34 route)
{
    switch (route) {
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_NO_ALCOVE:
            return "wall without alcove";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_WALL_ALCOVE_THING_PASS:
            return "wall alcove thing pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_CORRIDOR_THING_PASS:
            return "corridor thing pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_UP_THING_PASS:
            return "stairs up thing pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_STAIRS_DOWN_THING_PASS:
            return "stairs down thing pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_VISIBLE_THING_PASS:
            return "visible pit thing pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_PIT_INVISIBLE_THING_PASS:
            return "invisible pit thing pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_TELEPORTER_FIELD:
            return "teleporter field";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_DOOR_FRONT_DOUBLE_PASS:
            return "door front double pass";
        case CSB_V1_D3C_CENTER_FIELD_ROUTE_INVALID:
        default:
            return "invalid";
    }
}
