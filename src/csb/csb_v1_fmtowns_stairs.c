#include "csb_v1_fmtowns_stairs.h"

enum {
    CSB_V1_FMTOWNS_M645_FIRST_STAIRS = 108,
    CSB_V1_FMTOWNS_M647_WALL_SET_GRAPHIC_COUNT = 40,
    CSB_V1_FMTOWNS_STAIRS_GRAPHIC_COUNT = 18,
    CSB_V1_FMTOWNS_MAX_WALL_SET = 15
};

int csb_v1_fmtowns_stairs_graphic_index(int wall_set,
                                       int requested_graphic_index,
                                       unsigned int *out_graphic_index)
{
    if (out_graphic_index) *out_graphic_index = 0u;
    if (!out_graphic_index || wall_set < 0 ||
        wall_set > CSB_V1_FMTOWNS_MAX_WALL_SET ||
        requested_graphic_index < CSB_V1_FMTOWNS_M645_FIRST_STAIRS ||
        requested_graphic_index >= CSB_V1_FMTOWNS_M645_FIRST_STAIRS +
            CSB_V1_FMTOWNS_STAIRS_GRAPHIC_COUNT) {
        return 0;
    }
    *out_graphic_index = (unsigned int)(wall_set *
        CSB_V1_FMTOWNS_M647_WALL_SET_GRAPHIC_COUNT +
        requested_graphic_index);
    return 1;
}
