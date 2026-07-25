#include "redmcsb_f0692_fill_box_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static void test_pc_zone_layout_and_odd_edges(void)
{
    int16_t zone[4] = { 1, 1, 3, 2 };
    (void)zone;
    uint8_t bitmap[16];

    memset(bitmap, 0xab, sizeof(bitmap));
    /* ReDMCSB rounds the supplied seven-pixel row width to eight. */
    assert(redmcsb_f0692_fill_box_pc34_compat(bitmap, sizeof(bitmap), zone,
                                               4, 7));
    assert(bitmap[0] == 0xab && bitmap[1] == 0xab);
    assert(bitmap[4] == 0xa4 && bitmap[5] == 0x44 &&
           bitmap[6] == 0xab && bitmap[7] == 0xab);
    assert(bitmap[8] == 0xa4 && bitmap[9] == 0x44 &&
           bitmap[10] == 0xab && bitmap[11] == 0xab);
}

static void test_color_uses_low_nibble_and_preserves_neighbours(void)
{
    int16_t zone[4] = { 2, 0, 2, 1 };
    (void)zone;
    uint8_t bitmap[4] = { 0xab, 0xcd, 0xef, 0x01 };
    (void)bitmap;

    assert(redmcsb_f0692_fill_box_pc34_compat(bitmap, sizeof(bitmap), zone,
                                               0x1e, 8));
    assert(bitmap[0] == 0xab);
    assert(bitmap[1] == 0xee);
    assert(bitmap[2] == 0xef && bitmap[3] == 0x01);
}

static void test_invalid_zone_does_not_modify_bitmap(void)
{
    int16_t zone[4] = { 7, 0, 2, 1 };
    (void)zone;
    uint8_t bitmap[4] = { 0x12, 0x34, 0x56, 0x78 };
    uint8_t expected[sizeof(bitmap)];

    memcpy(expected, bitmap, sizeof(bitmap));
    assert(!redmcsb_f0692_fill_box_pc34_compat(bitmap, sizeof(bitmap), zone,
                                                3, 8));
    assert(memcmp(bitmap, expected, sizeof(bitmap)) == 0);
}

int main(void)
{
    test_pc_zone_layout_and_odd_edges();
    test_color_uses_low_nibble_and_preserves_neighbours();
    test_invalid_zone_does_not_modify_bitmap();
    return 0;
}
