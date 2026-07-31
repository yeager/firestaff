#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1C_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNVIEW.C F0108:3940-4011. These helpers are intentionally
 * limited to original-material composition: the caller must supply pixels
 * decoded from the verified CSB GRAPHICS.DAT session. */
#define CSB_V1_D1C_F0108_C10_COLOR_FLESH_PC34 10u
#define CSB_V1_D1C_F0108_FLOOR_ZONE_BASE_PC34 1500
#define CSB_V1_D1C_F0108_FLOOR_ZONE_STRIDE_PC34 11

uint8_t csb_v1_viewport_d1c_f0108_blend_c10_pc34(uint8_t destination_pixel,
                                                  uint8_t source_pixel);

int csb_v1_viewport_d1c_f0108_zone_for_coordinate_set_pc34(
    int coordinate_set,
    int view_floor);

#ifdef __cplusplus
}
#endif

#endif
