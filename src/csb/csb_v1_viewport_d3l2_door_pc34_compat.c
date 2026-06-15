#include "csb_v1_viewport_d3l2_door_pc34_compat.h"

enum {
    CSB_D3L2_VIEW_SQUARE = 14,          /* ReDMCSB DEFS.H C14_VIEW_SQUARE_D3L2 */
    CSB_D3L2_DOOR_PASS1 = 0x0218,       /* ReDMCSB DEFS.H:2669 */
    CSB_D3L2_DOOR_PASS2 = 0x0349,       /* ReDMCSB DEFS.H:2672 */
    CSB_D3L2_DOOR_ZONE = 3700,          /* ReDMCSB DEFS.H:4250 */
    CSB_D3L2_DOOR_ZONE_RECORD_TYPE = 1, /* ReDMCSB COORD.C:788 */
    CSB_D3L2_DOOR_PARENT_RECORD = 129,  /* ReDMCSB COORD.C:788,1559 */
    CSB_D3L2_DOOR_CLIP_RECORD = 126,    /* ReDMCSB COORD.C:1556,1559 */
    CSB_D3L2_DOOR_FRAME_X = 24,         /* ReDMCSB COORD.C:1559 */
    CSB_D3L2_DOOR_FRAME_Y = 28,         /* ReDMCSB COORD.C:1559 */
    CSB_D3L2_DOOR_CLIPPED_WIDTH = 48,   /* ReDMCSB COORD.C:1556 */
    CSB_D3L2_DOOR_CLIPPED_HEIGHT = 40,  /* ReDMCSB COORD.C:1556 */
    CSB_D3L2_COORD_LAYOUT_C03 = 3,      /* ReDMCSB COORD.C:1546-1548 */
    CSB_D3L2_FRAME_BITMAP_COMMAND = 60200, /* CSB Viewport.cpp:592 */
    CSB_D3L2_FRAME_BITMAP_INDEX = 5,       /* CSB Viewport.cpp:2281 */
    CSB_D3L2_FRAME_RECT_COMMAND = 60250,   /* CSB Viewport.cpp:650 */
    CSB_D3L2_FRAME_RECT_INDEX = 7,         /* CSB Viewport.cpp:2386 */
    CSB_D3L2_FRAME_BLIT_COMMAND = 60010,   /* CSB Viewport.cpp:385 */
    CSB_D3L2_DOOR_GRAPHIC_COMMAND = 60223, /* CSB Viewport.cpp:618 */
    CSB_D3L2_DOOR_GRAPHIC_INDEX = 2,       /* CSB Viewport.cpp:2568 */
    CSB_D3L2_DOOR_GRAPHIC_SIZE = 984,      /* CSB Viewport.cpp:2602-2604 */
    CSB_D3L2_DOOR_NEARNESS = 0,            /* CSB Viewport.cpp:2602-2604 */
    CSB_D3L2_TRANSPARENT_COLOR = 10        /* ReDMCSB DEFS.H:2088 */
};

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C:4218-4334 F0111_DUNGEONVIEW_DrawDoor; "
    "DUNVIEW.C:6271-6273 F0676 D3L2 F0115/F0111/F0115 route; "
    "DEFS.H:2088 C10_COLOR_FLESH, 2669/2672 door pass orders, "
    "4250 C3700_ZONE_DOOR_D3L2; COORD.C:788-797 zone 3700 records, "
    "1546-1560 C03 layout records 126/129. CSB Viewport.cpp:1813-1820 "
    "StdDrawF3L1DoorFacing frame-before-door route; 2281 pDoorBitmaps[5]; "
    "2386 DoorFrameRect[7]; 2568 and 2596-2616 DoorGraphic[2]/984/nearness0; "
    "CSBCode.cpp:2912-2929 BltShapeToViewport C10 transparent blit.";

static const CSB_V1_ViewportD3L2DoorRouteSpec s_d3l2_door_route = {
    CSB_D3L2_VIEW_SQUARE,
    CSB_D3L2_VIEW_SQUARE,
    CSB_D3L2_DOOR_PASS1,
    CSB_D3L2_DOOR_PASS2,
    CSB_D3L2_DOOR_ZONE,
    CSB_D3L2_DOOR_ZONE_RECORD_TYPE,
    CSB_D3L2_DOOR_PARENT_RECORD,
    CSB_D3L2_DOOR_CLIP_RECORD,
    CSB_D3L2_DOOR_FRAME_X,
    CSB_D3L2_DOOR_FRAME_Y,
    CSB_D3L2_DOOR_CLIPPED_WIDTH,
    CSB_D3L2_DOOR_CLIPPED_HEIGHT,
    CSB_D3L2_COORD_LAYOUT_C03,
    CSB_D3L2_FRAME_BITMAP_COMMAND,
    CSB_D3L2_FRAME_BITMAP_INDEX,
    CSB_D3L2_FRAME_RECT_COMMAND,
    CSB_D3L2_FRAME_RECT_INDEX,
    CSB_D3L2_FRAME_BLIT_COMMAND,
    0,
    CSB_D3L2_DOOR_GRAPHIC_COMMAND,
    CSB_D3L2_DOOR_GRAPHIC_INDEX,
    CSB_D3L2_DOOR_GRAPHIC_SIZE,
    CSB_D3L2_DOOR_NEARNESS,
    CSB_D3L2_TRANSPARENT_COLOR,
    1,
    "DUNVIEW.C F0676_DrawD3L2 / F0111_DUNGEONVIEW_DrawDoor",
    "Viewport.cpp StdDrawF3L1DoorFacing",
    s_source_evidence
};

const CSB_V1_ViewportD3L2DoorRouteSpec *
csb_v1_viewport_d3l2_door_route_spec_pc34(void)
{
    return &s_d3l2_door_route;
}

int csb_v1_viewport_d3l2_door_resolve_zone_pc34(
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec,
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

int csb_v1_viewport_d3l2_door_apply_c03_frame_clip_pc34(
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec,
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

const char *csb_v1_viewport_d3l2_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
