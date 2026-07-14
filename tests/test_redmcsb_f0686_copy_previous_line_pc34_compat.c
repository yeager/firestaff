#include "redmcsb_f0686_copy_previous_line_pc34_compat.h"

#include <stdint.h>

int main(void)
{
    uint8_t b[4] = {0x12, 0x34, 0x56, 0x78};

    if (!redmcsb_f0686_copy_previous_line_pc34_compat(b, 4U, 4U, 0U, 4U) ||
        b[2] != 0x12U || b[3] != 0x34U) return 1;
    if (redmcsb_f0686_copy_previous_line_pc34_compat(b, 4U, 7U, 0U, 2U) ||
        b[3] != 0x34U) return 1;
    if (redmcsb_f0686_copy_previous_line_pc34_compat(
            b, SIZE_MAX / 2U + 1U, 0U, 0U, 0U)) return 1;
    return 0;
}
