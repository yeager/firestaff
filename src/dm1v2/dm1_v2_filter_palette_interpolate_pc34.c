#include "dm1v2/dm1_v2_filters.h"

int dm1_v2_filter_palette_interpolate_indexed(unsigned char* fb, int w, int h, int strength_pct) {
    (void)strength_pct;
    if (!fb || w <= 0 || h <= 0) return -1;
    /* PC34 owns discrete palette selection. Do not invent intermediate
     * brightness levels in the source framebuffer. */
    return 0;
}
