#include "csb_v1_viewport_d2c_f0111_door_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_VIEW_SQUARE_D2C = 6,        /* ReDMCSB: DUNVIEW.C:7244-7389 F0121 route. */
    CSB_D2C_DEPTH = 2,              /* ReDMCSB: DUNVIEW.C:7244-7389, F0128 reaches D2C. */
    CSB_D2C_LATERAL = 0,            /* ReDMCSB: DUNVIEW.C:7244-7389, center lane. */
    CSB_C09_WALL_D2C = 9,           /* ReDMCSB: DEFS.H:3432 C09_WALL_D2C. */
    CSB_C707_ZONE_WALL_D2C = 707,   /* ReDMCSB: DEFS.H:4030 C707_ZONE_WALL_D2C. */
    CSB_C709_ZONE_WALL_D2C = 709,   /* ReDMCSB: DEFS.H:4049 C709_ZONE_WALL_D2C. */
    CSB_C3700_ZONE_DOOR_D3L2 = 3700,/* ReDMCSB: DEFS.H:4250 C3700_ZONE_DOOR_D3L2. */
    CSB_C10_COLOR_FLESH = 10,       /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_D2C_CELL_ORDER = 0x3421,    /* ReDMCSB: DUNVIEW.C:7356 no-wall order. */
    CSB_COORD_CLIP_RECORD = 126,    /* ReDMCSB: COORD.C:1556-1559 door record path. */
    CSB_COORD_PARENT_RECORD = 129,  /* ReDMCSB: COORD.C:1556-1559 door record path. */
    CSB_COORD_CLIP_WIDTH = 48,      /* ReDMCSB: COORD.C:1556 clipped width. */
    CSB_COORD_CLIP_HEIGHT = 40,     /* ReDMCSB: COORD.C:1556 clipped height. */
    CSB_COORD_FRAME_X = 24,         /* ReDMCSB: COORD.C:1559 frame x. */
    CSB_COORD_FRAME_Y = 28          /* ReDMCSB: COORD.C:1559 frame y. */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset D2C door bitmap parity. "
    "ReDMCSB DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C is the "
    "D2C center route. DUNVIEW.C:7289-7312 is the C00 wall case: it draws "
    "C09_WALL_D2C / C707_ZONE_WALL_D2C / C709_ZONE_WALL_D2C as applicable "
    "and returns before the F0111 door path. DUNVIEW.C:7353-7388 is the "
    "C05/C01 no-wall center-field path; it excludes F0100, F0105, F0107, "
    "and F0111, keeps F0115 room-object dispatch, then calls F0113 with "
    "C709_ZONE_WALL_D2C. DEFS.H:3432 anchors C09_WALL_D2C, DEFS.H:4030 "
    "anchors C707_ZONE_WALL_D2C, DEFS.H:4049 anchors C709_ZONE_WALL_D2C, "
    "DEFS.H:4250 anchors C3700_ZONE_DOOR_D3L2, and DEFS.H:2088 anchors "
    "C10_COLOR_FLESH transparency. COORD.C:1556-1559 is the C3700 D3 door "
    "record path and is explicitly rejected by this D2C non-route contract. "
    "CSB-lineage Viewport.cpp:1151-1156 binds F2 open to floor, ceiling, "
    "and room-object dispatch; Viewport.cpp:1414-1420 binds F2 teleporter "
    "to the same open prework plus teleporter drawing. This gate records "
    "the frame-blt/frame-rect binding as contract metadata only.";

static const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 s_spec = {
    CSB_ROUTE_PRESENT,
    CSB_VIEW_SQUARE_D2C,
    CSB_D2C_DEPTH,
    CSB_D2C_LATERAL,
    CSB_ROUTE_PRESENT,
    CSB_C09_WALL_D2C,
    CSB_C707_ZONE_WALL_D2C,
    CSB_C709_ZONE_WALL_D2C,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_D2C_CELL_ORDER,
    CSB_C709_ZONE_WALL_D2C,
    CSB_ROUTE_PRESENT,
    CSB_C10_COLOR_FLESH,
    CSB_C3700_ZONE_DOOR_D3L2,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_PRESENT,
    CSB_COORD_CLIP_RECORD,
    CSB_COORD_PARENT_RECORD,
    CSB_COORD_CLIP_WIDTH,
    CSB_COORD_CLIP_HEIGHT,
    CSB_COORD_FRAME_X,
    CSB_COORD_FRAME_Y,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    "ReDMCSB DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C",
    "ReDMCSB DUNVIEW.C:7289-7312 wall-case early-return",
    "ReDMCSB DUNVIEW.C:7353-7388 C01/C05 no-wall center field",
    "CSB-lineage Viewport.cpp:1151-1156,1414-1420 frame-blt/frame-rect bindings"
};

const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *
csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34(void)
{
    return &s_spec;
}

int csb_v1_viewport_d2c_f0111_door_zone_from_wall_spec_pc34(
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec)
{
    if (!spec) return -1;
    return spec->media720_wall_zone_c709;
}

int csb_v1_viewport_d2c_f0111_door_reject_c3700_panel_path_pc34(
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec,
    int zone_x,
    int zone_y,
    int *out_x,
    int *out_y)
{
    (void)zone_x;
    (void)zone_y;
    if (!spec || !out_x || !out_y) return -1;
    if (!spec->d2c_uses_c3700_door_zone) return -2;
    *out_x = spec->coord_frame_x + zone_x;
    *out_y = spec->coord_frame_y + zone_y;
    return 0;
}

int csb_v1_viewport_d2c_f0111_door_apply_c10_field_blit_pc34(
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;

    if (!spec || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;

    /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH is preserved by this synthetic
     * no-wall field helper; the D2C C3700 door-panel path is not rendered. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_d2c_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
