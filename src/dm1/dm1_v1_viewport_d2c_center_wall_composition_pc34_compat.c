#include "dm1_v1_viewport_d2c_center_wall_composition_pc34_compat.h"

enum {
    DM1_RED_M603_VIEW_SQUARE_D2C = 6,
    DM1_RED_D2C_VIEW_DEPTH = 2,
    DM1_RED_D2C_VIEW_LANE = 0,
    DM1_RED_D2C_FIELD_ASPECT = 7,
    DM1_RED_C709_ZONE_WALL_D2C = 709,
    DM1_RED_C0X0000_CELL_ORDER_ALCOVE = 0x0000,
    DM1_RED_C0X0218_CELL_ORDER_DOORPASS1 = 0x0218,
    DM1_RED_C0X0349_CELL_ORDER_DOORPASS2 = 0x0349,
    DM1_RED_C0X3421_CELL_ORDER_OPEN = 0x3421
};

/*
 * ReDMCSB: DUNVIEW.C F0128 lines 8520-8521 dispatch D2C; F0121 lines
 * 7289-7388 define the D2C wall, door, open-square, and field routes;
 * F0100/F0101 lines 3048-3076 define center-wall blits; F0107 lines
 * 3502-3590 resolves wall ornaments/alcoves; F0108 lines 3940-3992 draws
 * floor ornaments; F0113 lines 4382-4474 draws fields; F0115 lines
 * 4547-4581 documents the ordered thing pass.
 */
static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset bitmap parity. "
    "ReDMCSB DUNVIEW.C:8520-8521 F0128 dispatches relative depth 2 lane 0 "
    "into F0121_DUNGEONVIEW_DrawSquareD2C. DUNVIEW.C:7244-7388 is the D2C "
    "body. DUNVIEW.C:7289-7312 draws the center wall via F0100/F0101/zone "
    "wall and probes F0107; line 7312 returns when the front ornament is not "
    "an alcove. DUNVIEW.C:7308-7310 sends a wall alcove to F0115 with "
    "C0x0000_CELL_ORDER_ALCOVE. DUNVIEW.C:7313-7342 composes a door-front "
    "route as F0108, rear F0115 C0x0218, door body/F0111, then front F0115 "
    "C0x0349. DUNVIEW.C:7353-7368 composes open/pit/teleporter center "
    "squares as F0108, F0112, and F0115 C0x3421. DUNVIEW.C:7370-7388 overlays "
    "the D2C teleporter field; DUNVIEW.C:7386 uses G2035[M603]=7 and "
    "C709_ZONE_WALL_D2C. DUNVIEW.C:370-377 maps M603 index 6 to lane 0, "
    "depth 2, and field aspect 7. DUNVIEW.C:4382-4474 F0113 uses the field "
    "transparent color; DEFS.H:2088 defines C10_COLOR_FLESH. DUNVIEW.C:3048-3058 "
    "F0100 keeps C10 transparency for D2C door-frame/wall-set blits. "
    "DUNVIEW.C:4547-4581 F0115 defines the door-front two-pass cell-order "
    "semantics used by the synthetic D2C door-front pixel trace. DEFS.H:2596-2606 "
    "defines M603_VIEW_SQUARE_D2C=6; DEFS.H:2656-2677 defines the packed "
    "cell orders; DEFS.H:4040-4052 defines C709_ZONE_WALL_D2C.";

static void append_step(DM1_V1_D2CCenterCompositionTracePc34 *trace,
                        DM1_V1_D2CCenterCompositionStepPc34 step,
                        uint16_t cell_order,
                        int zone,
                        const char *anchor)
{
    DM1_V1_D2CCenterCompositionOpPc34 *op;

    if (!trace ||
        trace->step_count >= DM1_V1_D2C_CENTER_COMPOSITION_PC34_MAX_STEPS) {
        return;
    }
    op = &trace->steps[trace->step_count++];
    op->step = step;
    op->cell_order = cell_order;
    op->view_square_index = DM1_RED_M603_VIEW_SQUARE_D2C;
    op->zone = zone;
    op->transparent_color = DM1_V1_D2C_CENTER_COMPOSITION_PC34_C10_COLOR_FLESH;
    op->anchor = anchor;
}

const char *dm1_v1_viewport_d2c_center_wall_composition_source_evidence_pc34(void)
{
    return s_source_evidence;
}

DM1_V1_D2CCenterCompositionTracePc34
dm1_v1_viewport_d2c_center_wall_composition_trace_pc34(
    DM1_V1_D2CCenterCompositionRoutePc34 route)
{
    DM1_V1_D2CCenterCompositionTracePc34 trace = {0};

    trace.route = route;
    trace.view_square_index = DM1_RED_M603_VIEW_SQUARE_D2C;
    trace.view_depth = DM1_RED_D2C_VIEW_DEPTH;
    trace.view_lane = DM1_RED_D2C_VIEW_LANE;
    trace.field_aspect_index = DM1_RED_D2C_FIELD_ASPECT;
    trace.wall_zone_pc34 = DM1_RED_C709_ZONE_WALL_D2C;
    trace.source_evidence =
        dm1_v1_viewport_d2c_center_wall_composition_source_evidence_pc34();

    append_step(&trace,
                DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_DISPATCH_D2C,
                0, -1, "ReDMCSB DUNVIEW.C:8520-8521 F0128");

    switch (route) {
        case DM1_V1_D2C_CENTER_COMPOSITION_PC34_WALL_PLAIN:
            trace.calls_f0107_front_wall_ornament = true;
            trace.wall_case_returns_before_f0108 = true;
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_WALL_BODY,
                        0, DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7289-7307 F0121");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0107_FRONT_WALL_ORNAMENT,
                        0, DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7308-7310 F0121/F0107");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_RETURN,
                        0, -1, "ReDMCSB DUNVIEW.C:7312 F0121");
            break;
        case DM1_V1_D2C_CENTER_COMPOSITION_PC34_WALL_ALCOVE:
            trace.calls_f0107_front_wall_ornament = true;
            trace.calls_f0115_open_or_alcove = true;
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_WALL_BODY,
                        0, DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7289-7307 F0121");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0107_FRONT_WALL_ORNAMENT,
                        0, DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7308-7310 F0121/F0107");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_OPEN_OR_ALCOVE,
                        DM1_RED_C0X0000_CELL_ORDER_ALCOVE,
                        DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7309-7310/7367-7368 F0121/F0115");
            break;
        case DM1_V1_D2C_CENTER_COMPOSITION_PC34_DOOR_FRONT:
            trace.calls_f0108_floor_ornament = true;
            trace.calls_f0115_rear = true;
            trace.calls_f0111_door_body = true;
            trace.calls_f0115_front = true;
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0108_FLOOR_ORNAMENT,
                        0, -1, "ReDMCSB DUNVIEW.C:7314 F0121/F0108");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_REAR,
                        DM1_RED_C0X0218_CELL_ORDER_DOORPASS1,
                        DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7315 F0121/F0115");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0111_DOOR_BODY,
                        0, DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7317-7339 F0121/F0111");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_FRONT,
                        DM1_RED_C0X0349_CELL_ORDER_DOORPASS2,
                        DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7341-7342/7367-7368 F0121/F0115");
            break;
        case DM1_V1_D2C_CENTER_COMPOSITION_PC34_TELEPORTER:
            trace.calls_f0113_center_field = true;
            trace.field_uses_c10_transparency = true;
        case DM1_V1_D2C_CENTER_COMPOSITION_PC34_CORRIDOR:
        default:
            trace.calls_f0108_floor_ornament = true;
            trace.calls_f0112_ceiling_pit = true;
            trace.calls_f0115_open_or_alcove = true;
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0108_FLOOR_ORNAMENT,
                        0, -1, "ReDMCSB DUNVIEW.C:7357 F0121/F0108");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0112_CEILING_PIT,
                        0, -1, "ReDMCSB DUNVIEW.C:7358-7365 F0121/F0112");
            append_step(&trace,
                        DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0115_OPEN_OR_ALCOVE,
                        DM1_RED_C0X3421_CELL_ORDER_OPEN,
                        DM1_RED_C709_ZONE_WALL_D2C,
                        "ReDMCSB DUNVIEW.C:7356-7368 F0121/F0115");
            if (route == DM1_V1_D2C_CENTER_COMPOSITION_PC34_TELEPORTER) {
                append_step(&trace,
                            DM1_V1_D2C_CENTER_COMPOSITION_PC34_STEP_F0113_CENTER_FIELD,
                            0, DM1_RED_C709_ZONE_WALL_D2C,
                            "ReDMCSB DUNVIEW.C:7370-7388 F0121/F0113");
            }
            break;
    }

    return trace;
}

int dm1_v1_viewport_d2c_center_wall_composition_apply_c10_layer_pc34(
    const uint8_t *source,
    uint8_t *destination,
    size_t count,
    uint8_t transparent_color)
{
    int writes = 0;
    size_t i;

    if (!source || !destination) return -1;
    for (i = 0; i < count; ++i) {
        if (source[i] == transparent_color) continue;
        destination[i] = source[i];
        ++writes;
    }
    return writes;
}

static uint8_t apply_transparent_pixel(uint8_t destination,
                                       uint8_t source,
                                       uint8_t transparent_color,
                                       bool *transparent)
{
    if (transparent) {
        *transparent = (source == transparent_color);
    }
    if (source == transparent_color) return destination;
    return source;
}

static int decode_door_cells(uint16_t order, int *cells)
{
    int count = 0;
    int shift;

    if (!cells || (order & 0x0008U) == 0U) return -1;
    for (shift = 4; shift < 16; shift += 4) {
        int cell = (int)((order >> shift) & 0x000FU);
        if (cell == 0) break;
        if (count >= 2) return -1;
        cells[count++] = cell;
    }
    return count;
}

int dm1_v1_viewport_d2c_door_front_compose_pixel_pc34(
    uint8_t initial_pixel,
    uint8_t floor_ornament_pixel,
    uint8_t rear_f0115_pixel,
    uint8_t f0111_door_pixel,
    uint8_t front_f0115_pixel,
    uint8_t transparent_color,
    DM1_V1_D2CDoorFrontPixelTracePc34 *out_trace)
{
    uint8_t pixel;

    if (!out_trace) return 0;

    *out_trace = (DM1_V1_D2CDoorFrontPixelTracePc34){0};
    out_trace->rear_cell_order = DM1_RED_C0X0218_CELL_ORDER_DOORPASS1;
    out_trace->front_cell_order = DM1_RED_C0X0349_CELL_ORDER_DOORPASS2;
    out_trace->rear_cell_count =
        decode_door_cells(out_trace->rear_cell_order, out_trace->rear_cells);
    out_trace->front_cell_count =
        decode_door_cells(out_trace->front_cell_order, out_trace->front_cells);
    if (out_trace->rear_cell_count != 2 || out_trace->front_cell_count != 2) {
        return 0;
    }

    out_trace->initial_pixel = initial_pixel;

    /* ReDMCSB: DUNVIEW.C F0121 lines 7314-7342 draws floor ornament,
     * rear F0115 door pass 1, F0111 door/front frame, then front F0115
     * door pass 2; F0100/F0115/F0111 blits preserve C10 pixels. */
    pixel = apply_transparent_pixel(initial_pixel, floor_ornament_pixel,
                                    transparent_color,
                                    &out_trace->floor_transparent);
    out_trace->after_floor_ornament = pixel;

    pixel = apply_transparent_pixel(pixel, rear_f0115_pixel,
                                    transparent_color,
                                    &out_trace->rear_transparent);
    out_trace->after_rear_f0115 = pixel;

    pixel = apply_transparent_pixel(pixel, f0111_door_pixel,
                                    transparent_color,
                                    &out_trace->door_transparent);
    out_trace->after_f0111_door = pixel;

    pixel = apply_transparent_pixel(pixel, front_f0115_pixel,
                                    transparent_color,
                                    &out_trace->front_transparent);
    out_trace->after_front_f0115 = pixel;
    return 1;
}
