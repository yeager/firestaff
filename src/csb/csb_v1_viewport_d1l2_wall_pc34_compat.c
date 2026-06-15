#include "csb_v1_viewport_d1l2_wall_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_D1L2_REQUESTED_ADDRESSABLE = 0,
    CSB_D1L_CLOSEST_ANALOGUE = 1,
    CSB_D1L_VIEW_SQUARE = 4,             /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    CSB_D1L_RELATIVE_DEPTH = 1,          /* ReDMCSB DUNVIEW.C:8524 F0150 depth */
    CSB_D1L_RELATIVE_LATERAL = -1,       /* ReDMCSB DUNVIEW.C:8524 F0150 lateral */
    CSB_ELEMENT_WALL = 0,                /* ReDMCSB DEFS.H:1007 C00_ELEMENT_WALL */
    CSB_ELEMENT_TELEPORTER = 5,          /* ReDMCSB DEFS.H:1012 C05_ELEMENT_TELEPORTER */
    CSB_D1L_WALL_ZONE = 713,             /* ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L */
    CSB_D1C_WALL_ZONE = 712,             /* ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C */
    CSB_D1R_WALL_ZONE = 714,             /* ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R */
    CSB_D1L_NATIVE_WALL_INDEX = 3,       /* ReDMCSB DEFS.H:3426 C03_WALL_D1L */
    CSB_D1R_FLIPPED_WALL_INDEX = 2,      /* ReDMCSB DEFS.H:3425 C02_WALL_D1R */
    CSB_D1L_FRAME_ARRAY_INDEX = 4,       /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L */
    CSB_D1L_FRAME_X1 = 0,                /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_X2 = 63,               /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_Y1 = 9,                /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_Y2 = 119,              /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_BYTE_WIDTH = 128,      /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_HEIGHT = 111,          /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_SOURCE_X = 192,        /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_FRAME_SOURCE_Y = 0,          /* ReDMCSB DUNVIEW.C:590 G0163 D1L */
    CSB_D1L_CLIP_WIDTH = 64,             /* X1..X2 inclusive */
    CSB_D1L_CLIP_HEIGHT = 111,           /* Y1..Y2 inclusive */
    CSB_TRANSPARENT_COLOR = 10           /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no_asset_parity; not full real-asset "
    "wall bitmap parity. ReDMCSB DUNVIEW.C has no named D1L2 view square in "
    "the I34/CSB table: DEFS.H:2599-2601 exposes only M606 D1C, M607 D1L, "
    "and M608 D1R for row D1. This requested d1l2_wall gate is therefore "
    "anchored to the closest D1-column wall analogue, "
    "F0122_DUNGEONVIEW_DrawSquareD1L. DUNVIEW.C:581-590 G0163 frame metadata "
    "selects G0163[M607] = {0,63,9,119,128,111,192,0}; DUNVIEW.C:7436-7460 "
    "F0122 handles C00_ELEMENT_WALL with ST F0100(G0700, G0163[M607]), I34 "
    "F0105(G2107[C02_WALL_D1R], C713_ZONE_WALL_D1L) when flipped, otherwise "
    "F0104(G2107[C03_WALL_D1L], C713_ZONE_WALL_D1L), then F0107 and return. "
    "DUNVIEW.C:3113-3129 F0104 and DUNVIEW.C:3185-3204 F0105 preserve "
    "C10_COLOR_FLESH transparency. DUNVIEW.C:7538-7555 reaches F0113 only "
    "for C05_ELEMENT_TELEPORTER. DUNVIEW.C:8524-8525 dispatches relative "
    "depth 1 lateral -1 to F0122. DEFS.H:2088 C10_COLOR_FLESH=10; "
    "DEFS.H:2600 M607_VIEW_SQUARE_D1L=4; DEFS.H:3425-3426 C02/C03 wall "
    "ordinals; DEFS.H:4052-4054 neighboring C712/C713/C714 wall zones. "
    "non-overlap: CSB D1L2 wall route not yet covered; CSB D1L/D1R door and "
    "D1C center field already covered; this gate covers D1L2 wall route "
    "specifically. Limitation: D1L2 is not addressable by that name in "
    "ReDMCSB, so the asserted route is the D1L wall analogue.";

static const CSB_V1_ViewportD1L2WallRouteSpecPc34 s_route = {
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_D1L2_REQUESTED_ADDRESSABLE,
    CSB_D1L_CLOSEST_ANALOGUE,
    CSB_D1L_VIEW_SQUARE,
    CSB_D1L_RELATIVE_DEPTH,
    CSB_D1L_RELATIVE_LATERAL,
    CSB_ELEMENT_WALL,
    CSB_ELEMENT_TELEPORTER,
    CSB_D1L_WALL_ZONE,
    CSB_D1C_WALL_ZONE,
    CSB_D1R_WALL_ZONE,
    CSB_D1L_NATIVE_WALL_INDEX,
    CSB_D1R_FLIPPED_WALL_INDEX,
    CSB_D1L_FRAME_ARRAY_INDEX,
    CSB_D1L_FRAME_X1,
    CSB_D1L_FRAME_X2,
    CSB_D1L_FRAME_Y1,
    CSB_D1L_FRAME_Y2,
    CSB_D1L_FRAME_BYTE_WIDTH,
    CSB_D1L_FRAME_HEIGHT,
    CSB_D1L_FRAME_SOURCE_X,
    CSB_D1L_FRAME_SOURCE_Y,
    CSB_D1L_CLIP_WIDTH,
    CSB_D1L_CLIP_HEIGHT,
    CSB_TRANSPARENT_COLOR,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    "G0700_puc_Bitmap_WallSet_Wall_D1LCR / G2107_WallSet[C03_WALL_D1L]",
    "G0163_aauc_Graphic558_Frame_Walls[M607_VIEW_SQUARE_D1L]",
    "DUNVIEW.C F0122_DUNGEONVIEW_DrawSquareD1L / F0104 / F0105 / F0113",
    s_source_evidence
};

const CSB_V1_ViewportD1L2WallRouteSpecPc34 *
csb_v1_viewport_d1l2_wall_route_spec_pc34(void)
{
    return &s_route;
}

int csb_v1_viewport_d1l2_wall_resolve_clip_pc34(
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec,
    int *out_x,
    int *out_y,
    int *out_width,
    int *out_height)
{
    if (!spec || !out_x || !out_y || !out_width || !out_height) return -1;
    *out_x = spec->frame_x1;
    *out_y = spec->frame_y1;
    *out_width = spec->clip_width;
    *out_height = spec->clip_height;
    return 0;
}

int csb_v1_viewport_d1l2_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_ViewportD1L2WallRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int flip_horizontal,
    CSB_V1_ViewportD1L2WallBlitStatsPc34 *stats)
{
    CSB_V1_ViewportD1L2WallBlitStatsPc34 local = { 0, 0, 0, 0 };

    if (stats) *stats = local;
    if (!spec || !source || !destination ||
        source_width <= 0 || source_height <= 0 ||
        destination_width <= 0 || destination_height <= 0) {
        local.rejected = 1;
        if (stats) *stats = local;
        return -1;
    }
    if (source_width < spec->frame_source_x + spec->clip_width ||
        source_height < spec->frame_source_y + spec->clip_height) {
        local.rejected = 1;
        if (stats) *stats = local;
        return -1;
    }

    for (int y = 0; y < spec->clip_height; ++y) {
        const int dst_y = spec->frame_y1 + y;
        const int src_y = spec->frame_source_y + y;

        for (int x = 0; x < spec->clip_width; ++x) {
            const int src_offset = flip_horizontal ? spec->clip_width - 1 - x : x;
            const int src_x = spec->frame_source_x + src_offset;
            const int dst_x = spec->frame_x1 + x;
            const uint8_t pixel = source[(src_y * source_width) + src_x];

            if (dst_x < 0 || dst_x >= destination_width ||
                dst_y < 0 || dst_y >= destination_height) {
                ++local.clipped_pixels;
                continue;
            }
            if (pixel == (uint8_t)spec->transparent_color) {
                ++local.transparent_pixels;
                continue;
            }
            destination[(dst_y * destination_width) + dst_x] = pixel;
            ++local.copied_pixels;
        }
    }

    if (stats) *stats = local;
    return local.copied_pixels;
}

const char *csb_v1_viewport_d1l2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
