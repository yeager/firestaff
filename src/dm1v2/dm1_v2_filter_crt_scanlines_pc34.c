#include "dm1v2/dm1_v2_filters.h"

int dm1_v2_filter_crt_scanlines_rgba(unsigned char* rgba, int w, int h, int strength_pct) {
    (void)strength_pct;
    if (!rgba || w <= 0 || h <= 0) return -1;
    /* PC34 presents its source-owned palette directly. No scanline overlay
     * may darken alternate rows of an authenticated framebuffer. */
    return 0;
}
