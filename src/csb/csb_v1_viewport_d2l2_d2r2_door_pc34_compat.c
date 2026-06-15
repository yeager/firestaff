#include "csb_v1_viewport_d2l2_d2r2_door_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_D2L2_VIEW_SQUARE = 9,              /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2 */
    CSB_D2R2_VIEW_SQUARE = 10,             /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2 */
    CSB_D2_VIEW_DEPTH = 2,                 /* ReDMCSB DUNVIEW.C:8503/8507 F0128 */
    CSB_D2L2_VIEW_LATERAL = -2,            /* ReDMCSB DUNVIEW.C:8503 F0128 */
    CSB_D2R2_VIEW_LATERAL = 2,             /* ReDMCSB DUNVIEW.C:8507 F0128 */
    CSB_D2L2_WALL_ZONE = 707,              /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    CSB_D2R2_WALL_ZONE = 708,              /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2 */
    CSB_D3L2_DOOR_ZONE = 3700,             /* ReDMCSB DUNVIEW.C F0111:4218, DEFS.H:4250 */
    CSB_D3L2_DOOR_ZONE_RECORD_TYPE = 1,    /* ReDMCSB COORD.C:788 */
    CSB_D3L2_DOOR_PARENT_RECORD = 129,     /* ReDMCSB COORD.C:788,1559 */
    CSB_D3L2_DOOR_CLIP_RECORD = 126,       /* ReDMCSB COORD.C:1556,1559 */
    CSB_D3L2_DOOR_FRAME_X = 24,            /* ReDMCSB COORD.C:1559 */
    CSB_D3L2_DOOR_FRAME_Y = 28,            /* ReDMCSB COORD.C:1559 */
    CSB_D3L2_DOOR_CLIPPED_WIDTH = 48,      /* ReDMCSB COORD.C:1556 */
    CSB_D3L2_DOOR_CLIPPED_HEIGHT = 40,     /* ReDMCSB COORD.C:1556 */
    CSB_D3L2_COORD_LAYOUT_C03 = 3,         /* ReDMCSB COORD.C:1546-1548 */
    CSB_F3L1_FRAME_BITMAP_COMMAND = 60200, /* CSB Viewport.cpp:592 */
    CSB_F3L1_FRAME_BITMAP_INDEX = 5,       /* CSB Viewport.cpp:2281 */
    CSB_F3L1_FRAME_RECT_COMMAND = 60250,   /* CSB Viewport.cpp:650 */
    CSB_F3L1_FRAME_RECT_INDEX = 7,         /* CSB Viewport.cpp:2386 */
    CSB_FRAME_BLIT_COMMAND = 60010,        /* CSB Viewport.cpp:385 */
    CSB_DOOR_GRAPHIC_COMMAND = 60223,      /* CSB Viewport.cpp:618 */
    CSB_DOOR_GRAPHIC_INDEX = 2,            /* CSB Viewport.cpp:2568 */
    CSB_DOOR_GRAPHIC_SIZE = 984,           /* CSB Viewport.cpp:2602-2604 */
    CSB_DOOR_NEARNESS = 0,                 /* CSB Viewport.cpp:2602-2604 */
    CSB_TRANSPARENT_COLOR = 10             /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset door-frame bitmap parity. "
    "ReDMCSB DUNVIEW.C:6837-6872 F0678_DrawD2L2 and DUNVIEW.C:6837-6896 "
    "F0678/F0679 dispatch the near-side D2L2/D2R2 wall/teleporter squares. "
    "DUNVIEW.C:8503-8508 F0128 reaches those squares at depth 2 lateral -2/+2. "
    "The wall-precedes-door zones are DEFS.H:4047-4048 C707_ZONE_WALL_D2L2=707 "
    "and C708_ZONE_WALL_D2R2=708; the D2 dispatchers expose no direct F0111 "
    "call and wall cases return before any thing pass. The door pixel slice "
    "itself is locked to ReDMCSB DUNVIEW.C F0111:4218 and C3700_ZONE_DOOR_D3L2 "
    "with COORD.C:1556-1559 C03 record 126 clip / record 129 parent math. "
    "DEFS.H:2088 C10_COLOR_FLESH=10 is the transparency key passed by "
    "DUNVIEW.C:4334 F0111. CSB-lineage Viewport.cpp:1813-1820 "
    "StdDrawF3L1DoorFacing draws the frame before StdDrawDoor, with "
    "Viewport.cpp:2281 pDoorBitmaps[5], 2386 DoorFrameRect[7], and "
    "2568/2602-2604 DoorGraphic[2]/984/nearness0.";

static const CSB_V1_ViewportD2L2D2R2DoorRouteSpec s_routes[] = {
    {
        CSB_ROUTE_PRESENT,
        CSB_D2L2_VIEW_SQUARE,
        CSB_D2_VIEW_DEPTH,
        CSB_D2L2_VIEW_LATERAL,
        CSB_D2L2_WALL_ZONE,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_D3L2_DOOR_ZONE,
        CSB_D3L2_DOOR_ZONE_RECORD_TYPE,
        CSB_D3L2_DOOR_PARENT_RECORD,
        CSB_D3L2_DOOR_CLIP_RECORD,
        CSB_D3L2_DOOR_FRAME_X,
        CSB_D3L2_DOOR_FRAME_Y,
        CSB_D3L2_DOOR_CLIPPED_WIDTH,
        CSB_D3L2_DOOR_CLIPPED_HEIGHT,
        CSB_D3L2_COORD_LAYOUT_C03,
        CSB_F3L1_FRAME_BITMAP_COMMAND,
        CSB_F3L1_FRAME_BITMAP_INDEX,
        CSB_F3L1_FRAME_RECT_COMMAND,
        CSB_F3L1_FRAME_RECT_INDEX,
        CSB_FRAME_BLIT_COMMAND,
        CSB_ROUTE_ABSENT,
        CSB_DOOR_GRAPHIC_COMMAND,
        CSB_DOOR_GRAPHIC_INDEX,
        CSB_DOOR_GRAPHIC_SIZE,
        CSB_DOOR_NEARNESS,
        CSB_TRANSPARENT_COLOR,
        CSB_ROUTE_PRESENT,
        "DUNVIEW.C F0678_DrawD2L2 / F0128 / F0111 contract",
        "CSB-lineage Viewport.cpp StdDrawF3L1DoorFacing",
        s_source_evidence
    },
    {
        CSB_ROUTE_PRESENT,
        CSB_D2R2_VIEW_SQUARE,
        CSB_D2_VIEW_DEPTH,
        CSB_D2R2_VIEW_LATERAL,
        CSB_D2R2_WALL_ZONE,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_D3L2_DOOR_ZONE,
        CSB_D3L2_DOOR_ZONE_RECORD_TYPE,
        CSB_D3L2_DOOR_PARENT_RECORD,
        CSB_D3L2_DOOR_CLIP_RECORD,
        CSB_D3L2_DOOR_FRAME_X,
        CSB_D3L2_DOOR_FRAME_Y,
        CSB_D3L2_DOOR_CLIPPED_WIDTH,
        CSB_D3L2_DOOR_CLIPPED_HEIGHT,
        CSB_D3L2_COORD_LAYOUT_C03,
        CSB_F3L1_FRAME_BITMAP_COMMAND,
        CSB_F3L1_FRAME_BITMAP_INDEX,
        CSB_F3L1_FRAME_RECT_COMMAND,
        CSB_F3L1_FRAME_RECT_INDEX,
        CSB_FRAME_BLIT_COMMAND,
        CSB_ROUTE_ABSENT,
        CSB_DOOR_GRAPHIC_COMMAND,
        CSB_DOOR_GRAPHIC_INDEX,
        CSB_DOOR_GRAPHIC_SIZE,
        CSB_DOOR_NEARNESS,
        CSB_TRANSPARENT_COLOR,
        CSB_ROUTE_PRESENT,
        "DUNVIEW.C F0679_DrawD2R2 / F0128 / F0111 contract",
        "CSB-lineage Viewport.cpp StdDrawF3L1DoorFacing",
        s_source_evidence
    }
};

size_t csb_v1_viewport_d2l2_d2r2_door_route_spec_count_pc34(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *
csb_v1_viewport_d2l2_d2r2_door_route_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l2_d2r2_door_route_spec_count_pc34()) return NULL;
    return &s_routes[index];
}

const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *
csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d2l2_d2r2_door_route_spec_count_pc34(); ++i) {
        if (s_routes[i].view_square == view_square) return &s_routes[i];
    }
    return NULL;
}

int csb_v1_viewport_d2l2_d2r2_door_resolve_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec,
    int zone_x,
    int zone_y,
    int *out_x,
    int *out_y)
{
    if (!spec || !out_x || !out_y) return -1;
    *out_x = spec->frame_x + zone_x;
    *out_y = spec->frame_y + zone_y;
    return 0;
}

int csb_v1_viewport_d2l2_d2r2_door_apply_c03_frame_clip_pc34(
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;

    if (!spec || !source || !destination) return -1;
    if (source_stride < width || destination_stride < width) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (width > spec->clipped_width || height > spec->clipped_height) return -1;

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

const char *csb_v1_viewport_d2l2_d2r2_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
