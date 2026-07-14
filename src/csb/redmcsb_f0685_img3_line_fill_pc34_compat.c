#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"

bool redmcsb_f0685_img3_line_fill_pc34_compat(
    uint8_t *d, size_t n, size_t p, uint8_t c, size_t count) {
    size_t i;
    size_t pixel_capacity;

    if (d == NULL || n > SIZE_MAX / 2U) return false;
    pixel_capacity = n * 2U;
    if (p > pixel_capacity || count > pixel_capacity - p) return false;
    for (i = 0U; i < count; ++i, ++p) {
        size_t b = p >> 1U;
        if ((p & 1U) == 0U)
            d[b] = (uint8_t)((d[b] & 0x0FU) | ((c & 0x0FU) << 4U));
        else
            d[b] = (uint8_t)((d[b] & 0xF0U) | (c & 0x0FU));
    }
    return true;
}
const char *redmcsb_f0685_img3_line_fill_source_evidence_pc34(void) { return "ReDMCSB IMAGE4.C F0685_IMG3_LineColorFilling (48-67), PC I34E/I34M packed-nibble destination fill"; }
