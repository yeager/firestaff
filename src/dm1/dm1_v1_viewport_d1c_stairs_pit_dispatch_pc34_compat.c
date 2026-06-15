#include "dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat.h"

/*
 * Source-locked contract gate only; no real-asset loading and no F0100/F0111
 * rendering.  ReDMCSB anchors:
 * DUNVIEW.C:7727-7958 F0124_DUNGEONVIEW_DrawSquareD1C dispatch body.
 * DEFS.H:2547-2559 square-aspect indices for MEDIA720.
 * DEFS.H:2595-2600 M606_VIEW_SQUARE_D1C.
 * DEFS.H:2695-2710 M587_VIEW_WALL_D1C_FRONT.
 * DEFS.H:4148/4161/4206 stairs and pit zones.
 */

static const DM1_V1_D1CDispatchEvidencePc34 s_evidence = {
    "DUNVIEW.C:7727-7958 F0124_DUNGEONVIEW_DrawSquareD1C",
    "DEFS.H:1007-1017 elements; DEFS.H:2547-2559 MEDIA720 M550/M552/M555/M558; DEFS.H:2595-2600 M606; DEFS.H:2695-2710 M587",
    "DEFS.H:4148 C809; DEFS.H:4161 C822; DEFS.H:4206 C859; DEFS.H:4052 C712",
    "DUNVIEW.C:7753-7763 C19 stairs-up branch calls F0104 with G0079[C05] and C809",
    "DUNVIEW.C:7764-7781 C19 stairs-down branch calls F0104 with G0079[C12] and C822",
    "DUNVIEW.C:7912-7921 C02 floor-pit branch calls F0104 with M759/M765 and C859 in MEDIA720",
    "DUNVIEW.C:7784-7844 C00 wall branch blits D1C wall, probes F0107, then F0115 only for alcove",
    "DUNVIEW.C:7939-7957 F0113 teleporter field route uses M606_VIEW_SQUARE_D1C and C712_ZONE_WALL_D1C in MEDIA720",
    "DUNVIEW.C:8530-8535 F0128 advances to D1C and calls F0124_DUNGEONVIEW_DrawSquareD1C",
    "Contract excludes wall pixels/F0100, F0107 ornament ordinal internals, F0111 door, and real-asset bitmap parity."
};

static void init_output(DM1_V1_D1CDispatchOutputPc34 *output,
                        const DM1_V1_D1CDispatchInputPc34 *input)
{
    output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_UNSUPPORTED;
    output->native_bitmap_index = -1;
    output->zone_index = -1;
    output->cell_order_called = -1;
    output->view_square_index = DM1_V1_D1C_DISPATCH_PC34_VIEW_SQUARE_D1C;
    output->view_wall_index = DM1_V1_D1C_DISPATCH_PC34_VIEW_WALL_D1C_FRONT;
    output->floor_ornament_ordinal = input->floor_ornament_ordinal;
    output->front_wall_ornament_ordinal = input->front_wall_ornament_ordinal;
    output->first_thing_index = input->first_thing_index;
    output->cell_order_called_valid = false;
    output->used_f0104 = false;
    output->used_f0113 = false;
    output->used_f0107_alcove = false;
    output->used_f0115 = false;
    output->used_f0108_floor_ornament = false;
    output->used_f0112_ceiling_pit = false;
    output->used_f0115_after_dispatch = false;
    output->wall_blit_called = false;
    output->used_f0111_door = false;
    output->unsupported_element = false;
    output->evidence = s_evidence;
}

static void mark_open_floor_tail(DM1_V1_D1CDispatchOutputPc34 *output)
{
    output->cell_order_called = DM1_V1_D1C_DISPATCH_PC34_CELL_ORDER_OPEN;
    output->cell_order_called_valid = true;
    output->used_f0115 = true;
    output->used_f0115_after_dispatch = true;
    output->used_f0108_floor_ornament = true;
    output->used_f0112_ceiling_pit = true;
}

bool dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_probe(
    const DM1_V1_D1CDispatchInputPc34 *input,
    DM1_V1_D1CDispatchOutputPc34 *output)
{
    if (!input || !output) {
        return false;
    }

    init_output(output, input);

    switch (input->element) {
    case DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT:
        if (input->has_stairs_up_bit) {
            output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_UP_FRONT;
            output->native_bitmap_index = DM1_V1_D1C_DISPATCH_PC34_STAIRS_UP_SLOT_D1C;
            output->zone_index = DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_UP_D1C;
        } else {
            output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT;
            output->native_bitmap_index = DM1_V1_D1C_DISPATCH_PC34_STAIRS_DOWN_SLOT_D1C;
            output->zone_index = DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D1C;
        }
        output->used_f0104 = true;
        mark_open_floor_tail(output);
        return true;

    case DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR_PIT:
        output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_FLOOR_PIT;
        output->native_bitmap_index = DM1_V1_D1C_DISPATCH_PC34_FLOOR_PIT_D1C_GRAPHIC;
        output->zone_index = DM1_V1_D1C_DISPATCH_PC34_ZONE_FLOOR_PIT_D1C;
        output->used_f0104 = true;
        mark_open_floor_tail(output);
        return true;

    case DM1_V1_D1C_DISPATCH_PC34_ELEMENT_WALL:
        output->wall_blit_called = true;
        output->used_f0107_alcove = true;
        if (input->has_alcove) {
            output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_WALL_ALCOVE;
            output->cell_order_called = DM1_V1_D1C_DISPATCH_PC34_CELL_ORDER_ALCOVE;
            output->cell_order_called_valid = true;
            output->used_f0115 = true;
        } else {
            output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_WALL_NO_ALCOVE;
        }
        return true;

    case DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR:
        output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_OPEN_FLOOR;
        mark_open_floor_tail(output);
        return true;

    case DM1_V1_D1C_DISPATCH_PC34_ELEMENT_TELEPORTER:
        output->route_taken = DM1_V1_D1C_DISPATCH_PC34_ROUTE_TELEPORTER_FIELD;
        output->zone_index = DM1_V1_D1C_DISPATCH_PC34_ZONE_FIELD_D1C;
        output->used_f0113 = true;
        mark_open_floor_tail(output);
        return true;

    default:
        output->unsupported_element = true;
        return true;
    }
}

const DM1_V1_D1CDispatchEvidencePc34 *
dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_evidence(void)
{
    return &s_evidence;
}
