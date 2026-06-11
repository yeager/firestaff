#include "csb_v1_viewport_d0c_f0111_door_panel_pc34_compat.h"

enum {
    CSB_DOOR_STATE_OPEN = 0,          /* ReDMCSB DEFS.H:1039. */
    CSB_DOOR_STATE_PARTLY_OPEN_1 = 1,
    CSB_DOOR_STATE_PARTLY_OPEN_2 = 2,
    CSB_DOOR_STATE_PARTLY_OPEN_3 = 3,
    CSB_DOOR_STATE_CLOSED = 4,        /* ReDMCSB DEFS.H:1043. */
    CSB_DOOR_STATE_DESTROYED = 5      /* ReDMCSB DEFS.H:1044. */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap parity and no CSB "
    "game-data load. ReDMCSB DUNVIEW.C:4218-4339 "
    "F0111_DUNGEONVIEW_DrawDoor is the door-panel state machine: line 4262 "
    "dispatches the base F0109 door ornament, lines 4291-4294 draw "
    "C16_DOOR_ORNAMENT_THIEVES_EYE_MASK only when P2084_i_ZoneIndex equals "
    "M631_ZONE_DOOR_D1C, lines 4317-4326 keep partly-open doors inside the "
    "F0111 zone-shift path, and line 4334 performs the C10_COLOR_FLESH "
    "transparent final blit. The local Common/Source D0C dispatch symbol is "
    "F0127_DUNGEONVIEW_DrawSquareD0C, prototype DUNVIEW.C:1983-1989 and body "
    "8164-8311; the requested F0118_DUNGEONVIEW_DrawSquareD0C_CPSF spelling "
    "is not present in this snapshot. DUNVIEW.C:4547-4581 is the F0115 "
    "thing-pass loop and DUNVIEW.C:8294 is the D0C F0115 call site with "
    "M609_VIEW_SQUARE_D0C/C0x0021, but this D0C "
    "F0111 closed-door panel path does not introduce an F0115 anchor inside "
    "F0111. DUNVIEW.C:8478-8508 and 8534-8542 keep F0128 post-dispatch "
    "ordering stable before D0C. DUNVIEW.C:3113-3156 F0104, 3185-3247 F0105, "
    "3502-3938 F0107, and 8164-8311 F0127 are wall/frame/field contrast "
    "routes, not the F0111 door-panel path. DEFS.H:2088 C10_COLOR_FLESH, "
    "4040-4057 wall zones, and 4249-4261 door zones including "
    "M631_ZONE_DOOR_D1C anchor the zone mismatch. CSB-lineage "
    "Viewport.cpp:1903-1906 is the stable center-door thing-pass anchor before "
    "StdDrawDoor.";

static const CSB_V1_D0CF0111DoorPanelContractPc34 s_contract = {
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    CSB_V1_D0C_F0111_LINE_START_PC34,
    CSB_V1_D0C_F0111_LINE_END_PC34,
    CSB_V1_D0C_F0127_DISPATCH_LINE_START_PC34,
    CSB_V1_D0C_F0127_DISPATCH_LINE_END_PC34,
    CSB_V1_D0C_F0115_LINE_START_PC34,
    CSB_V1_D0C_F0115_LINE_END_PC34,
    CSB_V1_D0C_F0128_WALL_FOLLOWUP_START_PC34,
    CSB_V1_D0C_F0128_WALL_FOLLOWUP_END_PC34,
    CSB_V1_D0C_F0128_POST_DISPATCH_START_PC34,
    CSB_V1_D0C_F0128_POST_DISPATCH_END_PC34,
    CSB_V1_D0C_F0111_DOOR_ZONE_PC34,
    CSB_V1_D0C_F0111_D1C_THIEVES_EYE_ZONE_PC34,
    CSB_V1_D0C_F0111_WALL_ZONE_D0C_PC34,
    CSB_V1_D0C_F0111_VIEW_SQUARE_D0C_PC34,
    CSB_V1_D0C_F0111_F0115_CELL_ORDER_PC34,
    CSB_V1_D0C_F0111_C10_COLOR_FLESH_PC34,
    CSB_V1_D0C_F0111_C6_UNKNOWN_PC34,
    CSB_V1_D0C_F0111_MASK0X4000_PC34,
    CSB_V1_D0C_F0111_C15_DESTROYED_MASK_PC34,
    CSB_V1_D0C_F0111_C16_THIEVES_EYE_MASK_PC34,
    0,
    5,
    "ReDMCSB DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor; "
        "4262 base ornament; 4291-4294 M631 Thieves-Eye mask branch",
    "ReDMCSB DUNVIEW.C:1983-1989 prototype and 8164-8311 body "
        "F0127_DUNGEONVIEW_DrawSquareD0C; requested F0118 D0C name absent",
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 thing-pass loop; D0C call at 8294",
    "ReDMCSB DUNVIEW.C:8478-8508 and 8534-8542 F0128 post-dispatch order",
    "ReDMCSB DEFS.H:2088 C10; 4040-4057 wall zones; 4249-4261 door zones",
    "CSB-lineage Viewport.cpp:1903-1906 F1Contents/DrawOrder218 thing pass",
    s_source_evidence
};

const CSB_V1_D0CF0111DoorPanelContractPc34 *
csb_v1_viewport_d0c_f0111_door_panel_contract_pc34(void)
{
    return &s_contract;
}

const char *
csb_v1_viewport_d0c_f0111_door_panel_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(int door_state)
{
    if (door_state == CSB_DOOR_STATE_OPEN) {
        return CSB_V1_D0C_F0111_BRANCH_OPEN_SKIP_PC34;
    }
    if (door_state >= CSB_DOOR_STATE_PARTLY_OPEN_1 &&
        door_state <= CSB_DOOR_STATE_PARTLY_OPEN_3) {
        return CSB_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34;
    }
    if (door_state == CSB_DOOR_STATE_CLOSED) {
        return CSB_V1_D0C_F0111_BRANCH_CLOSED_PC34;
    }
    if (door_state == CSB_DOOR_STATE_DESTROYED) {
        return CSB_V1_D0C_F0111_BRANCH_DESTROYED_PC34;
    }
    return CSB_V1_D0C_F0111_BRANCH_INVALID_PC34;
}

int csb_v1_viewport_d0c_f0111_door_panel_closed_ornament_pc34(
    int door_state,
    int ornament_ordinal)
{
    if (door_state != CSB_DOOR_STATE_CLOSED) {
        return -1;
    }
    if (ornament_ordinal < s_contract.closed_ornament_ordinal_first ||
        ornament_ordinal > s_contract.closed_ornament_ordinal_last) {
        return -1;
    }
    return ornament_ordinal;
}

CSB_V1_D0CF0111DoorPanelStatePc34
csb_v1_viewport_d0c_f0111_door_panel_dispatch_pc34(
    CSB_V1_D0CF0111DoorPanelInputPc34 input)
{
    const int branch =
        input.door_present
            ? csb_v1_viewport_d0c_f0111_door_panel_branch_pc34(input.door_state)
            : CSB_V1_D0C_F0111_BRANCH_NO_DOOR_PC34;
    CSB_V1_D0CF0111DoorPanelStatePc34 state = {
        0,
        0,
        0,
        0,
        0,
        1,
        1,
        branch,
        CSB_V1_D0C_F0111_DOOR_ZONE_PC34,
        -1,
        -1,
        -1,
        CSB_V1_D0C_F0111_C10_COLOR_FLESH_PC34,
        input.ornament_ordinal,
        -1,
        0,
        CSB_V1_D0C_F0111_WALL_ZONE_D0C_PC34,
        s_source_evidence
    };

    if (!input.door_present ||
        input.element == CSB_V1_D0C_F0111_ELEMENT_EMPTY_CENTER_FIELD_PC34) {
        state.branch = CSB_V1_D0C_F0111_BRANCH_NO_DOOR_PC34;
        state.no_door_center_field_sane = 1;
        return state;
    }

    if (input.element != CSB_V1_D0C_F0111_ELEMENT_DOOR_FRONT_PC34 ||
        branch == CSB_V1_D0C_F0111_BRANCH_INVALID_PC34 ||
        branch == CSB_V1_D0C_F0111_BRANCH_OPEN_SKIP_PC34) {
        return state;
    }

    state.capturedF0111 = 1;
    state.capturedBaseOrnament = 1;
    state.final_zone = CSB_V1_D0C_F0111_DOOR_ZONE_PC34;
    state.destroyed_mask_ordinal =
        branch == CSB_V1_D0C_F0111_BRANCH_DESTROYED_PC34
            ? CSB_V1_D0C_F0111_C15_DESTROYED_MASK_PC34
            : -1;
    state.capturedThievesEyeBranch =
        (input.event73_count_thieves_eye &&
         state.door_zone == CSB_V1_D0C_F0111_D1C_THIEVES_EYE_ZONE_PC34)
            ? 1
            : 0;

    if (branch == CSB_V1_D0C_F0111_BRANCH_PARTLY_OPEN_PC34) {
        state.first_half_zone = input.horizontal_door
            ? state.door_zone + input.door_state + CSB_V1_D0C_F0111_C6_UNKNOWN_PC34
            : -1;
        state.second_half_zone = input.horizontal_door
            ? state.door_zone + input.door_state +
                  (CSB_V1_D0C_F0111_MASK0X4000_PC34 | 3)
            : state.door_zone + input.door_state;
    }

    return state;
}

uint8_t csb_v1_viewport_d0c_f0111_door_panel_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    /* ReDMCSB: DUNVIEW.C F0111 line 4334 uses C10_COLOR_FLESH transparency. */
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}
