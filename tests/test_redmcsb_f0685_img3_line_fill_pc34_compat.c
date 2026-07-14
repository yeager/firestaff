#include "redmcsb_f0685_img3_line_fill_pc34_compat.h"

#include <stdint.h>

int main(void) {
    uint8_t b[3] = {0x12, 0x34, 0x56};

    if (!redmcsb_f0685_img3_line_fill_pc34_compat(b, 3U, 1U, 0xAU, 4U) ||
        b[0] != 0x1AU || b[1] != 0xAAU || b[2] != 0xA6U) return 1;
    if (redmcsb_f0685_img3_line_fill_pc34_compat(b, 3U, 5U, 1U, 2U) ||
        b[2] != 0xA6U) return 1;
    if (redmcsb_f0685_img3_line_fill_pc34_compat(
            b, SIZE_MAX / 2U + 1U, 0U, 0U, 0U)) return 1;
    return 0;
}
