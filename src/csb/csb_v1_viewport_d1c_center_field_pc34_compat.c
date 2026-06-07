#include "csb_v1_viewport_d1c_center_field_pc34_compat.h"

/*
 * Source-locked contract gate only; not full real-asset wall, door, or field
 * bitmap parity.
 */

enum {
    CSB_RED_ROUTE_REJECTED_ZONE = -1
};

static const CSB_V1_D1CCenterFieldEvidencePc34 s_evidence = {
    "ReDMCSB DUNVIEW.C:370-377 maps I34 M606_VIEW_SQUARE_D1C index 3 to "
    "G2026 lane 0, G2027 depth 1, and G2035 field aspect 10",
    "ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C=3; DEFS.H:2710 "
    "M587_VIEW_WALL_D1C_FRONT=14",
    "ReDMCSB DUNVIEW.C:8532-8533 dispatches relative depth 1 lane 0 into "
    "F0124_DUNGEONVIEW_DrawSquareD1C",
    "ReDMCSB DUNVIEW.C:7727-7958 contains the D1C body; the local source names "
    "this body F0124, while F0122 at DUNVIEW.C:8524-8525 is the D1L dispatch",
    "ReDMCSB DUNVIEW.C:7784-7872 handles C00_ELEMENT_WALL; DUNVIEW.C:7825 "
    "uses F0100 with M606_VIEW_SQUARE_D1C, while the I34 branches at "
    "DUNVIEW.C:7833-7840 use the C712_ZONE_WALL_D1C bitmap route",
    "ReDMCSB DUNVIEW.C:7842-7843 calls F0107 with M587_VIEW_WALL_D1C_FRONT "
    "and sends alcove objects through C0x0000_CELL_ORDER_ALCOVE",
    "ReDMCSB DUNVIEW.C:7873-7911 handles C17_ELEMENT_DOOR_FRONT; "
    "DUNVIEW.C:7875 sends the first pass through "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
    "ReDMCSB DUNVIEW.C:7922-7956 handles the no-wall corridor/teleporter path; "
    "the I34 F0113 field call at DUNVIEW.C:7955 is gated to C05_ELEMENT_TELEPORTER",
    "ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C, DEFS.H:4054 C714_ZONE_WALL_D1R, "
    "DEFS.H:4148 C809_ZONE_STAIRS_UP_FRONT_D1C, DEFS.H:4149 "
    "C810_ZONE_STAIRS_UP_FRONT_D1R, and DEFS.H:4161 "
    "C822_ZONE_STAIRS_DOWN_FRONT_D1C",
    "ReDMCSB DEFS.H:2658 C0x0000_CELL_ORDER_ALCOVE and DEFS.H:2669 "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
    "The requested CSB D1C center-field slice is source-locked to the local "
    "ReDMCSB I34 path; constants that were ambiguous in the task are resolved "
    "from DEFS.H and G2026/G2027/G2035."
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset wall, door, or field "
    "bitmap parity. ReDMCSB DUNVIEW.C:370-377 maps I34 "
    "M606_VIEW_SQUARE_D1C to index 3, lane 0, depth 1, field aspect 10. "
    "DEFS.H:2599 anchors M606_VIEW_SQUARE_D1C=3; DEFS.H:2710 anchors "
    "M587_VIEW_WALL_D1C_FRONT=14; DEFS.H:2658/2669 anchor C0x0000 and "
    "C0x0218 cell orders. DUNVIEW.C:8532-8533 dispatches the I34 depth-1 "
    "center square into F0124_DUNGEONVIEW_DrawSquareD1C; the local source has "
    "F0122 at DUNVIEW.C:8524-8525 as the D1L dispatch, not D1C. "
    "DUNVIEW.C:7784-7872 is the wall route: F0100/M606 at DUNVIEW.C:7825 "
    "and I34 C712 wall bitmap branches at DUNVIEW.C:7833-7840, then "
    "F0107/M587 at DUNVIEW.C:7842-7843, with alcove objects sent through "
    "C0x0000_CELL_ORDER_ALCOVE and no F0113 field before return. "
    "DUNVIEW.C:7873-7911 is the door-front route; DUNVIEW.C:7875 uses "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT. DUNVIEW.C:7922-7956 "
    "is the no-wall route; C01 corridor performs floor/ceiling/thing prework, "
    "while C05 teleporter additionally reaches the I34 F0113 field call at "
    "DUNVIEW.C:7955 using C712_ZONE_WALL_D1C. DEFS.H:4052 anchors C712, "
    "DEFS.H:4054 anchors neighboring C714, DEFS.H:4148-4149 anchor C809/C810, "
    "and DEFS.H:4161 anchors C822.";

static bool input_matches_i34_d1c(CSB_V1_D1CCenterFieldInputPc34 input)
{
    return input.view_square_index ==
               CSB_V1_D1C_CENTER_FIELD_PC34_VIEW_SQUARE_INDEX &&
           input.lane == CSB_V1_D1C_CENTER_FIELD_PC34_LANE &&
           input.depth == CSB_V1_D1C_CENTER_FIELD_PC34_DEPTH &&
           input.field_aspect == CSB_V1_D1C_CENTER_FIELD_PC34_FIELD_ASPECT;
}

CSB_V1_D1CCenterFieldOutputPc34
csb_v1_viewport_d1c_center_field_pc34_compat_probe(
    CSB_V1_D1CCenterFieldInputPc34 input)
{
    CSB_V1_D1CCenterFieldOutputPc34 out = {
        CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_INVALID,
        CSB_RED_ROUTE_REJECTED_ZONE,
        CSB_RED_ROUTE_REJECTED_ZONE,
        0,
        false,
        false,
        false,
        false,
        s_evidence
    };

    if (!input_matches_i34_d1c(input)) return out;

    switch (input.element) {
        case CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL:
            out.route_taken = input.has_alcove
                ? CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_FRONT_ALCOVE
                : CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_FRONT_NO_ALCOVE;
            out.wall_zone_index =
                CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_WALL_D1C;
            out.used_f0100 = true;
            out.used_f0107_alcove = true;
            if (input.has_alcove) {
                out.cell_order =
                    CSB_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_ALCOVE;
                out.used_f0115_thing_pass = true;
            }
            break;
        case CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR:
            out.route_taken = CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_CORRIDOR_OPEN;
            out.used_f0115_thing_pass = true;
            break;
        case CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER:
            out.route_taken = CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_TELEPORTER_FIELD;
            out.field_zone_index =
                CSB_V1_D1C_CENTER_FIELD_PC34_MEDIA720_ZONE_WALL_D1C;
            out.used_f0113_field = true;
            out.used_f0115_thing_pass = true;
            break;
        case CSB_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR_FRONT:
            out.route_taken = CSB_V1_D1C_CENTER_FIELD_PC34_ROUTE_DOOR_FRONT;
            out.cell_order =
                CSB_V1_D1C_CENTER_FIELD_PC34_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT;
            out.used_f0115_thing_pass = true;
            break;
        default:
            break;
    }

    return out;
}

const CSB_V1_D1CCenterFieldEvidencePc34 *
csb_v1_viewport_d1c_center_field_pc34_compat_evidence(void)
{
    return &s_evidence;
}

const char *
csb_v1_viewport_d1c_center_field_pc34_compat_source_evidence(void)
{
    return s_source_evidence;
}
