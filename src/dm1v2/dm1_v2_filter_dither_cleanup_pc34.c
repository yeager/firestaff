#include "dm1v2/dm1_v2_filters.h"

int dm1_v2_filter_dither_cleanup_indexed(unsigned char* fb, int w, int h) {
    if (!fb || w <= 0 || h <= 0) return -1;
    /* The indexed bytes are source-owned colour/brightness pairs. PC34 has
     * no host 3x3 mode-filter route, so keep every byte unchanged. */
    return 0;
}
