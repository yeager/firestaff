#include "csb_v1_viewport_d2c_center_field_pc34_compat.h"

/*
 * Source-locked contract gate only; not full real-asset field bitmap parity.
 */

enum {
    /* ReDMCSB: DEFS.H:2088 defines C10_COLOR_FLESH. */
    CSB_RED_C10_COLOR_FLESH = 10,
    /* ReDMCSB: DEFS.H:2602 defines the I34 M603_VIEW_SQUARE_D2C. */
    CSB_RED_M603_VIEW_SQUARE_D2C = 6,
    /* ReDMCSB: DUNVIEW.C:370-372 map I34 D2C to lane 0 and depth 2. */
    CSB_RED_D2C_VIEW_LANE = 0,
    CSB_RED_D2_VIEW_DEPTH = 2,
    /* ReDMCSB: DUNVIEW.C:377 maps I34 D2C to field aspect 7. */
    CSB_RED_D2C_FIELD_ASPECT = 7,
    /* ReDMCSB: DEFS.H:3432 defines C09_WALL_D2C. */
    CSB_RED_C09_WALL_D2C = 9,
    /* ReDMCSB: DEFS.H:4030 and 4049 define the D2C wall/field zones. */
    CSB_RED_MEDIA508_C707_ZONE_WALL_D2C = 707,
    CSB_RED_MEDIA720_C709_ZONE_WALL_D2C = 709,
    /* ReDMCSB: DEFS.H:4042-4043 anchor the I34 contiguous wall-zone base. */
    CSB_RED_MEDIA720_C702_ZONE_WALL_D3L2 = 702,
    CSB_RED_MEDIA720_C703_ZONE_WALL_D3R2 = 703,
    /* ReDMCSB: DUNVIEW.C:7356 uses this no-wall D2 center thing order. */
    CSB_RED_C0X3421_CELL_ORDER_D2_CENTER_NO_WALL = 0x3421,
    /* ReDMCSB: DUNVIEW.C:586 G0163 frame for the D2C wall metadata. */
    CSB_RED_D2C_FRAME_X1 = 60,
    CSB_RED_D2C_FRAME_X2 = 163,
    CSB_RED_D2C_FRAME_Y1 = 20,
    CSB_RED_D2C_FRAME_Y2 = 90,
    CSB_RED_D2C_FRAME_BYTE_WIDTH = 72,
    CSB_RED_D2C_FRAME_HEIGHT = 71,
    CSB_RED_D2C_FRAME_BLIT_X = 16,
    CSB_RED_D2C_FRAME_BLIT_Y = 0
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset field bitmap parity. "
    "ReDMCSB DUNVIEW.C:370-377 maps I34 M603_VIEW_SQUARE_D2C index 6 to "
    "G2026 lane 0, G2027 depth 2, and G2035 field aspect 7; "
    "DUNVIEW.C:7244-7389 local F0121_DUNGEONVIEW_DrawSquareD2C is the "
    "D2C center route reached by the I34 F0128 dispatcher at DUNVIEW.C:8520-8521; "
    "DUNVIEW.C:7289-7312 contains the wall case with F0100/F0107 and returns, "
    "so the C01/C05 no-wall center field route at DUNVIEW.C:7353-7388 excludes "
    "F0100_DUNGEONVIEW_DrawWallSetBitmap, "
    "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally, "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF, and "
    "F0111_DUNGEONVIEW_DrawDoor; "
    "DUNVIEW.C:7356-7368 keeps only ordinary floor/ceiling and F0115 thing "
    "prework before DUNVIEW.C:7386 calls only F0113_DUNGEONVIEW_DrawField "
    "for the I34 D2C center field; "
    "DUNVIEW.C:586 anchors the D2C wall frame; DEFS.H:3432 anchors "
    "C09_WALL_D2C; DEFS.H:4030/4049 anchor C707/C709_ZONE_WALL_D2C; "
    "DEFS.H:4042-4043 anchor C702_ZONE_WALL_D3L2 and C703_ZONE_WALL_D3R2, "
    "with C702 + field aspect 7 yielding C709 for the F0113 I34 D2C zone; "
    "DEFS.H:2088 anchors C10_COLOR_FLESH transparency. "
    "CSB-lineage Viewport.cpp:1151-1156 and 1414-1420 route F2 open/"
    "teleporter through floor/ceiling, DrawRoomObjects, and DrawTeleporter, "
    "while Viewport.cpp:2115-2123 maps F2 open/teleporter separately from "
    "F2 stone and door-facing routes.";

static const CSB_V1_D2CCenterFieldSpecPc34 s_spec = {
    CSB_V1_D2C_CENTER_FIELD_PC34_VIEW_SQUARE_D2C,
    CSB_RED_M603_VIEW_SQUARE_D2C,
    CSB_RED_M603_VIEW_SQUARE_D2C,
    CSB_RED_D2_VIEW_DEPTH,
    CSB_RED_D2C_VIEW_LANE,
    CSB_RED_D2C_FIELD_ASPECT,
    "F0121_DUNGEONVIEW_DrawSquareD2C (I34 F0128 D2C dispatch)",
    "DUNVIEW.C:8520-8521",
    "DUNVIEW.C:7353-7388",
    "DUNVIEW.C:7386",
    "CSB-lineage Viewport.cpp:1151-1156; 1414-1420; 2115-2123",
    {
        CSB_RED_D2C_FRAME_X1,
        CSB_RED_D2C_FRAME_X2,
        CSB_RED_D2C_FRAME_Y1,
        CSB_RED_D2C_FRAME_Y2,
        CSB_RED_D2C_FRAME_BYTE_WIDTH,
        CSB_RED_D2C_FRAME_HEIGHT,
        CSB_RED_D2C_FRAME_BLIT_X,
        CSB_RED_D2C_FRAME_BLIT_Y
    },
    CSB_RED_C09_WALL_D2C,
    CSB_RED_MEDIA508_C707_ZONE_WALL_D2C,
    CSB_RED_MEDIA720_C709_ZONE_WALL_D2C,
    CSB_RED_MEDIA720_C702_ZONE_WALL_D3L2,
    CSB_RED_MEDIA720_C703_ZONE_WALL_D3R2,
    CSB_RED_MEDIA720_C702_ZONE_WALL_D3L2 + CSB_RED_D2C_FIELD_ASPECT,
    CSB_RED_C10_COLOR_FLESH,
    CSB_RED_C0X3421_CELL_ORDER_D2_CENTER_NO_WALL,
    true,
    false,
    false,
    false,
    false,
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true
};

const CSB_V1_D2CCenterFieldSpecPc34 *
M11_GameView_ViewportD2CCenterFieldPc34Spec(void)
{
    return &s_spec;
}

int M11_GameView_ViewportD2CCenterFieldPc34ZoneFromC702Base(
    const CSB_V1_D2CCenterFieldSpecPc34 *spec)
{
    if (!spec) return -1;
    return spec->media720_base_zone_c702 + spec->field_aspect_index;
}

int M11_GameView_ViewportD2CCenterFieldPc34ApplySyntheticC10FieldBlit(
    const CSB_V1_D2CCenterFieldSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH is the transparent color used by
     * field-style viewport blits. This helper is a synthetic contract probe,
     * not a real-asset D2C teleporter-field renderer. */
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

const char *M11_GameView_ViewportD2CCenterFieldPc34SourceEvidence(void)
{
    return s_source_evidence;
}
