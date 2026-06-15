#include "dm1_v1_viewport_d0c_center_field_pc34_compat.h"

/*
 * Source-locked contract gate only; not full real-asset wall/field parity.
 */

enum {
    /* ReDMCSB: DEFS.H:2088 defines C10_COLOR_FLESH. */
    DM1_RED_C10_COLOR_FLESH = 10,
    /* ReDMCSB: DEFS.H:2587 defines M609_VIEW_SQUARE_D0C. */
    DM1_RED_M609_VIEW_SQUARE_D0C = 9,
    /* ReDMCSB: DEFS.H:4036 defines the MEDIA508 D0C field/wall zone. */
    DM1_RED_C713_ZONE_WALL_D0C = 713,
    /* ReDMCSB: DEFS.H:4055 defines the MEDIA720 D0C field/wall zone. */
    DM1_RED_C715_ZONE_WALL_D0C = 715,
    /* ReDMCSB: DUNVIEW.C:8294 uses C0x0021 for D0C's ordinary F0115 pass. */
    DM1_RED_C0X0021_CELL_ORDER_BACKLEFT_BACKRIGHT = 0x0021,
    /* ReDMCSB: DUNVIEW.C:592 gives D0C a no-wall G0163 frame. */
    DM1_RED_D0C_FRAME_X1 = 0,
    DM1_RED_D0C_FRAME_X2 = 223,
    DM1_RED_D0C_FRAME_Y1 = 0,
    DM1_RED_D0C_FRAME_Y2 = 135,
    DM1_RED_D0C_FRAME_BYTE_WIDTH = 0,
    DM1_RED_D0C_FRAME_HEIGHT = 0,
    DM1_RED_D0C_FRAME_BLIT_X = 0,
    DM1_RED_D0C_FRAME_BLIT_Y = 0,
    /* ReDMCSB: DEFS.H has no C*_WALL ordinal for D0C. */
    DM1_RED_D0C_NO_C_WALL_ORDINAL = -1
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset wall/field parity. "
    "DUNVIEW.C:5675-5683 F0115 D0C back-cell/projectile no-wall cell gate; "
    "DUNVIEW.C:581-594 G0163 wall frames, with D0C line 592 byte width/height zero; "
    "DUNVIEW.C:8164-8310 F0127_DUNGEONVIEW_DrawSquareD0C has no wall case, "
    "no F0100_DUNGEONVIEW_DrawWallSetBitmap/F0100_DrawWallSetBitmap, "
    "no F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally scratch flip wall route, "
    "no F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF wall ornament, "
    "and no F0111_DUNGEONVIEW_DrawDoor door draw; "
    "DUNVIEW.C:8294 F0127 performs the ordinary pre-field "
    "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF thing pass once; "
    "DUNVIEW.C:8295-8310 F0113_DUNGEONVIEW_DrawField teleporter field route; "
    "DUNVIEW.C:8305 F0113 uses C713_ZONE_WALL_D0C; "
    "DUNVIEW.C:8308 F0113 uses C715_ZONE_WALL_D0C in MEDIA720; "
    "DUNVIEW.C:8538-8542 F0128 dispatches D0R then F0127_DUNGEONVIEW_DrawSquareD0C; "
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2587 M609_VIEW_SQUARE_D0C; "
    "DEFS.H:4036 C713_ZONE_WALL_D0C; DEFS.H:4055 C715_ZONE_WALL_D0C; "
    "D0C has no C*_WALL ordinal in DEFS.H and therefore no F0100/F0105/F0107 wall route.";

static const DM1_V1_D0CCenterFieldSpecPc34 s_spec = {
    DM1_V1_D0C_CENTER_FIELD_PC34_VIEW_SQUARE_D0C,
    DM1_RED_M609_VIEW_SQUARE_D0C,
    "F0127_DUNGEONVIEW_DrawSquareD0C",
    "DUNVIEW.C:8538-8542",
    "DUNVIEW.C:5675-5683; DUNVIEW.C:8164-8310; DUNVIEW.C:581-594",
    "DUNVIEW.C:8295-8310",
    "DUNVIEW.C:8294 pre-field F0115 only; no F0111 and no extra F0115 after F0113",
    {
        DM1_RED_D0C_FRAME_X1,
        DM1_RED_D0C_FRAME_X2,
        DM1_RED_D0C_FRAME_Y1,
        DM1_RED_D0C_FRAME_Y2,
        DM1_RED_D0C_FRAME_BYTE_WIDTH,
        DM1_RED_D0C_FRAME_HEIGHT,
        DM1_RED_D0C_FRAME_BLIT_X,
        DM1_RED_D0C_FRAME_BLIT_Y
    },
    DM1_RED_D0C_NO_C_WALL_ORDINAL,
    DM1_RED_C713_ZONE_WALL_D0C,
    DM1_RED_C715_ZONE_WALL_D0C,
    DM1_RED_C10_COLOR_FLESH,
    DM1_RED_C0X0021_CELL_ORDER_BACKLEFT_BACKRIGHT,
    true,
    false,
    false,
    false,
    true,
    false,
    false,
    true,
    true,
    false,
    false,
    true
};

const DM1_V1_D0CCenterFieldSpecPc34 *
dm1_v1_viewport_d0c_center_field_pc34_compat_spec(void)
{
    return &s_spec;
}

const char *dm1_v1_viewport_d0c_center_field_pc34_compat_source_evidence(void)
{
    return s_source_evidence;
}
