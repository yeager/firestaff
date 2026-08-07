#include "dm1_v1_fmtowns_icon_category.h"

const uint16_t
dm1_v1_fmtowns_icon_category_thresholds[DM1_V1_FMTOWNS_ICON_CATEGORY_COUNT] = {
    0, 32, 64, 96, 128, 160, 192
};

int dm1_v1_fmtowns_icon_classify_pc34(
        uint16_t icon_index, uint16_t *out_category, uint16_t *out_offset) {
    unsigned int bx;
    if (!out_category || !out_offset) return 0;
    if (icon_index >= DM1_V1_FMTOWNS_ICON_INDEX_MAX) return 0;
    /* Match LOAD_ICON 0xbb98..0xbbb3 exactly:
     *   for (bx = 0; bx < 7; ++bx)
     *       if (edi < table[bx]) break;
     *   --bx;
     * Result: bx is the largest bucket whose threshold is <= edi. */
    for (bx = 0; bx < DM1_V1_FMTOWNS_ICON_CATEGORY_COUNT; ++bx) {
        if (icon_index < dm1_v1_fmtowns_icon_category_thresholds[bx]) break;
    }
    /* Loop always advances past bx=0 for any uint16_t (table[0]==0
     * so the icon_index<table[0] check is never true on the first
     * iteration), so bx after the loop is guaranteed to be >= 1. */
    --bx;
    *out_category = (uint16_t)bx;
    *out_offset   =
        (uint16_t)(icon_index - dm1_v1_fmtowns_icon_category_thresholds[bx]);
    return 1;
}
