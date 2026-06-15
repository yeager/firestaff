#include "dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat.h"

enum {
    DM1_V1_F0098_VIEWPORT_WIDTH_PC34 = 224,
    DM1_V1_F0098_VIEWPORT_HEIGHT_PC34 = 136,
    DM1_V1_F0098_VIEWPORT_BYTE_WIDTH_PC34 = 112,
    DM1_V1_F0098_BLACK_AREA_HEIGHT_PC34 = 37,
    DM1_V1_F0098_CEILING_HEIGHT_PC34 = 29,
    DM1_V1_F0098_FLOOR_Y_PC34 = 66,
    DM1_V1_F0098_FLOOR_HEIGHT_PC34 = 70,
    DM1_V1_F0098_C10_COLOR_FLESH_PC34 = 10,      /* ReDMCSB: DEFS.H line 2088 C10_COLOR_FLESH */
    DM1_V1_F0098_CM1_NO_TRANSPARENCY_PC34 = -1, /* ReDMCSB: DEFS.H line 2076 CM1_COLOR_NO_TRANSPARENCY */
    DM1_V1_F0098_FLOOR_BITMAP_PC34 = -1,        /* ReDMCSB: DUNVIEW.C line 159 G2108_Floor */
    DM1_V1_F0098_CEILING_BITMAP_PC34 = -2,      /* ReDMCSB: DUNVIEW.C line 160 G2109_Ceiling */
    DM1_V1_F0098_M610_D0L_PC34 = 10,            /* ReDMCSB: DEFS.H line 2588 M610_VIEW_SQUARE_D0L */
    DM1_V1_F0098_M611_D0R_PC34 = 11,            /* ReDMCSB: DEFS.H line 2589 M611_VIEW_SQUARE_D0R */
    DM1_V1_F0098_C716_D0L_PC34 = 716,           /* ReDMCSB: DEFS.H line 4056 C716_ZONE_WALL_D0L */
    DM1_V1_F0098_C717_D0R_PC34 = 717,           /* ReDMCSB: DEFS.H line 4057 C717_ZONE_WALL_D0R */
    DM1_V1_F0098_C01_WALL_D0L_PC34 = 1,         /* ReDMCSB: DEFS.H line 3424 C01_WALL_D0L */
    DM1_V1_F0098_C00_WALL_D0R_PC34 = 0          /* ReDMCSB: DEFS.H line 3423 C00_WALL_D0R */
};

/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0098 lines 2962-3002: function-level floor/ceiling refresh,
 *   with black-area clear, G2109_Ceiling/G2108_Floor copy, viewport size set,
 *   and G0297_B_DrawFloorAndCeilingRequested cleared on exit.
 * - DUNVIEW.C F0128 lines 8337-8338 and 8611-8615: F0098 is entered from
 *   the dirty-flag guard and later called again to prefill the next viewport.
 * - DUNVIEW.C F0125/F0126 lines 8005/8115 and 8033/8139: F0128's viewport
 *   enumeration reaches M610/M611 and the PC34 C716/C717 overlay fallback.
 * - DUNVIEW.C F0104 lines 3113-3151 preserves C10_COLOR_FLESH as
 *   transparency for overlay bitmaps; F0792 lines 3288-3301 draws floor and
 *   ceiling refresh bitmaps with CM1_COLOR_NO_TRANSPARENCY.
 * - DUNVIEW.C F0108 lines 3940-4008: zero floor-ornament ordinal is a no-draw.
 */

static const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 s_dispatch[] = {
    {
        DM1_V1_F0098_FALLBACK_STEP_F0128_DIRTY_GUARD_PC34,
        10,
        "F0128_DUNGEONVIEW_Draw_CPSF",
        "DUNVIEW.C:8337-8338",
        "Enter F0098 only when G0297_B_DrawFloorAndCeilingRequested is true."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0098_CLEAR_BLACK_AREA_PC34,
        20,
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "DUNVIEW.C:2962-2989",
        "Clear the 224x37 viewport black area before floor/ceiling copies."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0098_COPY_CEILING_PC34,
        30,
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "DUNVIEW.C:2995",
        "Copy G2109_Ceiling into the viewport ceiling band."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0098_COPY_FLOOR_PC34,
        40,
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "DUNVIEW.C:2996",
        "Copy G2108_Floor into the viewport floor band."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0098_SET_VIEWPORT_SIZE_PC34,
        50,
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "DUNVIEW.C:2999-3000",
        "Restore the viewport bitmap width and height after the copies."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0098_CLEAR_DIRTY_FLAG_PC34,
        60,
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "DUNVIEW.C:3002",
        "Clear G0297_B_DrawFloorAndCeilingRequested on F0098 exit."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0128_ENUMERATE_D0L_PC34,
        170,
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "DUNVIEW.C:8564-8567/8005/8033",
        "F0128 enumeration reaches D0L: M610 objects and C716 wall fallback."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0128_ENUMERATE_D0R_PC34,
        180,
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "DUNVIEW.C:8568-8571/8115/8139",
        "F0128 enumeration reaches D0R: M611 objects and C717 wall fallback."
    },
    {
        DM1_V1_F0098_FALLBACK_STEP_F0128_PRESENT_AND_PREFILL_PC34,
        900,
        "F0128_DUNGEONVIEW_Draw_CPSF",
        "DUNVIEW.C:8606-8615",
        "Present the viewport, then prefill floor/ceiling for the next draw."
    }
};

static const char s_source_evidence[] =
    "contract_only=1; function_level=1; direction_specific=0; "
    "DUNVIEW.C:F0098:2962-3002 entry/exit floor-ceiling refresh clears "
    "G0297_B_DrawFloorAndCeilingRequested; "
    "DUNVIEW.C:F0128:8337-8338 dirty guard enters F0098; "
    "DUNVIEW.C:F0128:8564-8571 enumerates D0L/D0R after F0098; "
    "DUNVIEW.C:F0125:8005 uses M610_VIEW_SQUARE_D0L and line 8033 uses "
    "G2107_WallSet[C01_WALL_D0L] with C716_ZONE_WALL_D0L; "
    "DUNVIEW.C:F0126:8115 uses M611_VIEW_SQUARE_D0R and line 8139 uses "
    "G2107_WallSet[C00_WALL_D0R] with C717_ZONE_WALL_D0R; "
    "DUNVIEW.C:F0104:3113-3151 preserves C10_COLOR_FLESH transparency; "
    "DUNVIEW.C:F0792:3288-3301 floor/ceiling bitmap refresh uses "
    "CM1_COLOR_NO_TRANSPARENCY; "
    "DUNVIEW.C:F0108:3940-4008 zero floor ornament ordinal is no-draw; "
    "DUNVIEW.C:F0128:8606-8615 presents then pre-fills with F0098; "
    "DEFS.H:2076 CM1_COLOR_NO_TRANSPARENCY=-1; "
    "DEFS.H:2088 C10_COLOR_FLESH=10; "
    "DEFS.H:2588-2589 M610/M611; "
    "DEFS.H:4056-4057 C716/C717.";

static const DM1_V1_F0098FloorCeilingFallbackSpecPc34 s_spec = {
    true,
    true,
    false,
    false,
    DM1_V1_F0098_VIEWPORT_WIDTH_PC34,
    DM1_V1_F0098_VIEWPORT_HEIGHT_PC34,
    DM1_V1_F0098_VIEWPORT_BYTE_WIDTH_PC34,
    DM1_V1_F0098_BLACK_AREA_HEIGHT_PC34,
    DM1_V1_F0098_CEILING_HEIGHT_PC34,
    DM1_V1_F0098_FLOOR_Y_PC34,
    DM1_V1_F0098_FLOOR_HEIGHT_PC34,
    10,
    20,
    60,
    170,
    900,
    DM1_V1_F0098_C10_COLOR_FLESH_PC34,
    DM1_V1_F0098_CM1_NO_TRANSPARENCY_PC34,
    DM1_V1_F0098_FLOOR_BITMAP_PC34,
    DM1_V1_F0098_CEILING_BITMAP_PC34,
    DM1_V1_F0098_M610_D0L_PC34,
    DM1_V1_F0098_M611_D0R_PC34,
    DM1_V1_F0098_C716_D0L_PC34,
    DM1_V1_F0098_C717_D0R_PC34,
    DM1_V1_F0098_C01_WALL_D0L_PC34,
    DM1_V1_F0098_C00_WALL_D0R_PC34,
    "G2108_Floor",
    "G2109_Ceiling",
    "M610_VIEW_SQUARE_D0L",
    "M611_VIEW_SQUARE_D0R",
    "C716_ZONE_WALL_D0L",
    "C717_ZONE_WALL_D0R",
    s_source_evidence
};

const DM1_V1_F0098FloorCeilingFallbackSpecPc34 *
dm1_v1_viewport_f0098_floor_ceiling_fallback_spec_pc34(void)
{
    return &s_spec;
}

const DM1_V1_F0098FloorCeilingFallbackDispatchPc34 *
dm1_v1_viewport_f0098_floor_ceiling_fallback_dispatch_pc34(size_t *count)
{
    if (count) {
        *count = sizeof(s_dispatch) / sizeof(s_dispatch[0]);
    }
    return s_dispatch;
}

bool dm1_v1_viewport_f0098_floor_ceiling_should_enter_pc34(
    bool draw_floor_and_ceiling_requested)
{
    return draw_floor_and_ceiling_requested;
}

bool dm1_v1_viewport_f0098_floor_ceiling_dirty_after_exit_pc34(void)
{
    return false;
}

bool dm1_v1_viewport_f0098_floor_ceiling_zero_ordinal_draws_pc34(
    unsigned int ordinal)
{
    return ordinal != 0U;
}

int dm1_v1_viewport_f0098_floor_ceiling_blit_pixel_pc34(
    int destination_color,
    int source_color,
    int transparent_color)
{
    return source_color == transparent_color ? destination_color : source_color;
}

const char *dm1_v1_viewport_f0098_floor_ceiling_fallback_source_evidence_pc34(void)
{
    return s_source_evidence;
}
