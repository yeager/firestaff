#include "firestaff/csb/v1/viewport/d1c_f0108_floor_ceiling_ornament_pc34_compat.h"

uint8_t csb_v1_viewport_d1c_f0108_blend_c10_pc34(uint8_t destination_pixel,
                                                  uint8_t source_pixel)
{
    /* ReDMCSB: DEFS.H line 2088 names C10_COLOR_FLESH; DUNVIEW.C
     * F0108 lines 3989-4004 passes C10 to the transparent blitter. */
    return source_pixel == CSB_V1_D1C_F0108_C10_COLOR_FLESH_PC34
        ? destination_pixel
        : source_pixel;
}

int csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(
    int coordinate_set,
    int view_floor)
{
    /* ReDMCSB: DUNVIEW.C F0108 lines 3998/4004 use
     * C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloor. */
    return CSB_V1_D1C_F0108_FLOOR_ZONE_BASE_PC34 +
           coordinate_set * CSB_V1_D1C_F0108_FLOOR_ZONE_STRIDE_PC34 +
           view_floor;
}
