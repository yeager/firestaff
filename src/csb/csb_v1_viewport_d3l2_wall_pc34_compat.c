#include "csb_v1_viewport_d3l2_wall_pc34_compat.h"

enum {
    CSB_D3L2_VIEW_SQUARE = 14,             /* ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2 */
    CSB_D3R2_VIEW_SQUARE = 15,             /* ReDMCSB DEFS.H:2611 C15_VIEW_SQUARE_D3R2 */
    CSB_D3L2_WALL_ZONE = 702,              /* ReDMCSB DEFS.H:4042 C702_ZONE_WALL_D3L2 */
    CSB_D3R2_WALL_ZONE = 703,              /* ReDMCSB DEFS.H:4043 C703_ZONE_WALL_D3R2 */
    CSB_D3L2_ZONE_RECORD_TYPE = 11,        /* ReDMCSB COORD.C:1495-1498 G3019_s_LayoutData04 */
    CSB_D3R2_ZONE_RECORD_TYPE = 12,        /* ReDMCSB COORD.C:1495-1498 G3019_s_LayoutData04 */
    CSB_D3_WALL_ZONE_PARENT_RECORD = 4,    /* ReDMCSB COORD.C:1495-1498 parent record */
    CSB_D3_WALL_VIEWPORT_CLIP_RECORD = 3,  /* ReDMCSB COORD.C:1656-1658 viewport clip chain */
    CSB_D3_WALL_COORD_LAYOUT_RANGE = 4,    /* ReDMCSB COORD.C:1491-1494 G3019_s_LayoutData04 */
    CSB_C03_GAME_EXETYPE = 3,              /* ReDMCSB DEFS.H C03_GAME */
    CSB_D3L2_NATIVE_WALL_INDEX = 11,       /* ReDMCSB DEFS.H:3433-3434 C11_WALL_D3L2 */
    CSB_D3R2_NATIVE_WALL_INDEX = 10,       /* ReDMCSB DEFS.H:3433 C10_WALL_D3R2 */
    CSB_D3L2_FRAME_X1 = 0,                 /* ReDMCSB DUNVIEW.C:579 G0711 */
    CSB_D3L2_FRAME_X2 = 15,                /* ReDMCSB DUNVIEW.C:579 G0711 */
    CSB_D3R2_FRAME_X1 = 208,               /* ReDMCSB DUNVIEW.C:580 G0712 */
    CSB_D3R2_FRAME_X2 = 223,               /* ReDMCSB DUNVIEW.C:580 G0712 */
    CSB_D3_WALL_FRAME_Y1 = 25,             /* ReDMCSB DUNVIEW.C:579-580 G0711/G0712 */
    CSB_D3_WALL_FRAME_Y2 = 73,             /* ReDMCSB DUNVIEW.C:579-580 G0711/G0712 */
    CSB_D3_WALL_BYTE_WIDTH = 8,            /* ReDMCSB DUNVIEW.C:579-580 G0711/G0712 */
    CSB_D3_WALL_HEIGHT = 49,               /* ReDMCSB DUNVIEW.C:579-580 G0711/G0712 */
    CSB_D3_WALL_SOURCE_X = 0,              /* ReDMCSB DUNVIEW.C:579-580 G0711/G0712 */
    CSB_D3_WALL_SOURCE_Y = 0,              /* ReDMCSB DUNVIEW.C:579-580 G0711/G0712 */
    CSB_D3L2_WALL_BITMAP_COMMAND = 60258,  /* CSB Viewport.cpp:659 StdWallBitmapF3L2 */
    CSB_D3R2_WALL_BITMAP_COMMAND = 60262,  /* CSB Viewport.cpp:663 StdWallBitmapF3R2 */
    CSB_D3L2_WALL_BITMAP_INDEX = 5,        /* CSB Viewport.cpp:2267 pWallBitmaps[5] */
    CSB_D3R2_WALL_BITMAP_INDEX = 6,        /* CSB Viewport.cpp:2271 pWallBitmaps[6] */
    CSB_D3L2_WALL_RECT_COMMAND = 60082,    /* CSB Viewport.cpp:465 StdWallRectangleF3L2 */
    CSB_D3R2_WALL_RECT_COMMAND = 60086,    /* CSB Viewport.cpp:469 StdWallRectangleF3R2 */
    CSB_D3L2_WALL_RECT_INDEX = 13,         /* CSB Viewport.cpp:2304 wallRectangles[13] */
    CSB_D3R2_WALL_RECT_INDEX = 12,         /* CSB Viewport.cpp:2308 wallRectangles[12] */
    CSB_D3_WALL_BLIT_COMMAND = 60001,      /* CSB Viewport.cpp:374/956-963/2959-2962 StdBltShape */
    CSB_TRANSPARENT_COLOR = 10             /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset wall bitmap parity. "
    "ReDMCSB DUNVIEW.C:579-580 G0711/G0712 wall frame metadata; "
    "DUNVIEW.C:6253-6264 F0676 D3L2 wall path, C11_WALL_D3L2/C702 and F0107 return; "
    "DUNVIEW.C:6320-6331 F0677 D3R2 parity path, C10_WALL_D3R2/C703 and F0107 return; "
    "DUNVIEW.C:3113-3129 F0104 C10 wall blit; DUNVIEW.C:3185-3204 F0105 scratch flip "
    "with C10 wall blit; DUNVIEW.C:8446-8459 far-side wall route. "
    "DEFS.H:2088 C10_COLOR_FLESH, 2610/2611 C14/C15 view squares, "
    "3433-3434 wall ordinals, 4042/4043 zones. "
    "DEFS.H C03_GAME and COORD.C:1491-1498 G3019_s_LayoutData04 zone records 702/703; "
    "COORD.C:1656-1658 viewport clip parent; COORD.C:2390-2409 clip rejection. "
    "CSB Viewport.cpp:954-963 StdDrawF3L2Stone/StdDrawF3R2Stone; "
    "659-663 wall bitmap commands; 2267/2271 pWallBitmaps[5]/[6]; "
    "465/469 wall rect commands; 2304/2308 wallRectangles[13]/[12]; "
    "2959-2962 StdBltShape to BltShapeToViewport.";

static const CSB_V1_ViewportD3L2WallRouteSpec s_d3l2_wall_route = {
    CSB_TRANSPARENT_COLOR,
    1,
    1,
    "DUNVIEW.C F0676_DrawD3L2 / F0677_DrawD3R2 / F0104 / F0105",
    "Viewport.cpp StdDrawF3L2Stone / StdDrawF3R2Stone",
    s_source_evidence,
    {
        CSB_D3L2_VIEW_SQUARE,
        CSB_D3L2_WALL_ZONE,
        CSB_D3L2_ZONE_RECORD_TYPE,
        CSB_D3_WALL_ZONE_PARENT_RECORD,
        CSB_D3_WALL_VIEWPORT_CLIP_RECORD,
        CSB_D3_WALL_COORD_LAYOUT_RANGE,
        CSB_C03_GAME_EXETYPE,
        CSB_D3L2_NATIVE_WALL_INDEX,
        CSB_D3R2_NATIVE_WALL_INDEX,
        CSB_D3L2_FRAME_X1,
        CSB_D3L2_FRAME_X2,
        CSB_D3_WALL_FRAME_Y1,
        CSB_D3_WALL_FRAME_Y2,
        CSB_D3_WALL_BYTE_WIDTH,
        CSB_D3_WALL_HEIGHT,
        CSB_D3_WALL_SOURCE_X,
        CSB_D3_WALL_SOURCE_Y,
        CSB_D3L2_WALL_BITMAP_COMMAND,
        CSB_D3L2_WALL_BITMAP_INDEX,
        CSB_D3L2_WALL_RECT_COMMAND,
        CSB_D3L2_WALL_RECT_INDEX,
        CSB_D3_WALL_BLIT_COMMAND,
        1
    },
    {
        CSB_D3R2_VIEW_SQUARE,
        CSB_D3R2_WALL_ZONE,
        CSB_D3R2_ZONE_RECORD_TYPE,
        CSB_D3_WALL_ZONE_PARENT_RECORD,
        CSB_D3_WALL_VIEWPORT_CLIP_RECORD,
        CSB_D3_WALL_COORD_LAYOUT_RANGE,
        CSB_C03_GAME_EXETYPE,
        CSB_D3R2_NATIVE_WALL_INDEX,
        CSB_D3L2_NATIVE_WALL_INDEX,
        CSB_D3R2_FRAME_X1,
        CSB_D3R2_FRAME_X2,
        CSB_D3_WALL_FRAME_Y1,
        CSB_D3_WALL_FRAME_Y2,
        CSB_D3_WALL_BYTE_WIDTH,
        CSB_D3_WALL_HEIGHT,
        CSB_D3_WALL_SOURCE_X,
        CSB_D3_WALL_SOURCE_Y,
        CSB_D3R2_WALL_BITMAP_COMMAND,
        CSB_D3R2_WALL_BITMAP_INDEX,
        CSB_D3R2_WALL_RECT_COMMAND,
        CSB_D3R2_WALL_RECT_INDEX,
        CSB_D3_WALL_BLIT_COMMAND,
        1
    }
};

const CSB_V1_ViewportD3L2WallRouteSpec *
csb_v1_viewport_d3l2_wall_route_spec_pc34(void)
{
    return &s_d3l2_wall_route;
}

const CSB_V1_ViewportD3L2WallSideSpec *
csb_v1_viewport_d3l2_wall_side_spec_pc34(
    const CSB_V1_ViewportD3L2WallRouteSpec *spec,
    CSB_V1_ViewportD3L2WallSide side)
{
    if (!spec) return 0;
    if (side == CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2) return &spec->d3l2;
    if (side == CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2) return &spec->d3r2;
    return 0;
}

int csb_v1_viewport_d3l2_wall_resolve_zone_pc34(
    const CSB_V1_ViewportD3L2WallRouteSpec *spec,
    CSB_V1_ViewportD3L2WallSide side,
    int *out_x,
    int *out_y,
    int *out_width,
    int *out_height)
{
    const CSB_V1_ViewportD3L2WallSideSpec *side_spec =
        csb_v1_viewport_d3l2_wall_side_spec_pc34(spec, side);

    if (!side_spec || !out_x || !out_y || !out_width || !out_height) return -1;
    *out_x = side_spec->frame_x1;
    *out_y = side_spec->frame_y1;
    *out_width = side_spec->byte_width;
    *out_height = side_spec->height;
    return 0;
}

int csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_ViewportD3L2WallRouteSpec *spec,
    CSB_V1_ViewportD3L2WallSide side,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int flip_horizontal)
{
    int copied = 0;
    const CSB_V1_ViewportD3L2WallSideSpec *side_spec =
        csb_v1_viewport_d3l2_wall_side_spec_pc34(spec, side);

    if (!side_spec || !source || !destination) return -1;
    if (source_stride < side_spec->source_x + side_spec->byte_width) return -1;
    if (destination_width <= 0 || destination_height <= 0) return -1;

    for (int y = 0; y < side_spec->height; ++y) {
        const int dst_y = side_spec->frame_y1 + y;
        if (dst_y < 0 || dst_y >= destination_height) continue;

        for (int x = 0; x < side_spec->byte_width; ++x) {
            const int src_x = flip_horizontal
                ? side_spec->source_x + side_spec->byte_width - 1 - x
                : side_spec->source_x + x;
            const int src_y = side_spec->source_y + y;
            const int dst_x = side_spec->frame_x1 + x;
            const uint8_t pixel = source[(src_y * source_stride) + src_x];

            if (dst_x < 0 || dst_x >= destination_width) continue;
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(dst_y * destination_width) + dst_x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_d3l2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
