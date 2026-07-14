#include "redmcsb_f0721_shrink_blt_sub2_pc34_compat.h"

#include <assert.h>
#include <stdint.h>

static void make_reverse_palette(uint8_t palette[16])
{
    uint8_t index;

    for (index = 0U; index < 16U; ++index) {
        palette[index] = (uint8_t)(15U - index);
    }
}

int main(void)
{
    const uint8_t source[] = { 0x12U, 0x34U, 0x56U, 0x78U };
    uint8_t palette[16];
    uint8_t destination[] = { 0xa5U, 0xa5U, 0xa5U, 0xa5U };

    make_reverse_palette(palette);
    redmcsb_f0721_shrink_blt_sub2_pc34_compat(
        source, destination, palette, 0, 0U, 64U, 4U);
    assert(destination[0] == 0xedU);
    assert(destination[1] == 0xcbU);
    assert(destination[2] == 0xa5U);

    redmcsb_f0721_shrink_blt_sub2_pc34_compat(
        source, destination, palette, 0, 4U, 128U, 4U);
    assert(destination[2] == 0xdbU);
    assert(destination[3] == 0x97U);

    return 0;
}
