#include "dm1_v1_fmtowns_icon_geometry.h"

int dm1_v1_fmtowns_icon_geometry_is_self_consistent_pc34(void) {
    return DM1_V1_FMTOWNS_ICON_SIZE_BYTES ==
           (DM1_V1_FMTOWNS_ICON_X_SIZE * DM1_V1_FMTOWNS_ICON_Y_SIZE);
}

int dm1_v1_fmtowns_icon_geometry_get_pc34(
        uint16_t *out_bytes, uint16_t *out_x, uint16_t *out_y) {
    if (!out_bytes || !out_x || !out_y) return 0;
    if (!dm1_v1_fmtowns_icon_geometry_is_self_consistent_pc34()) return 0;
    *out_bytes = DM1_V1_FMTOWNS_ICON_SIZE_BYTES;
    *out_x     = DM1_V1_FMTOWNS_ICON_X_SIZE;
    *out_y     = DM1_V1_FMTOWNS_ICON_Y_SIZE;
    return 1;
}
