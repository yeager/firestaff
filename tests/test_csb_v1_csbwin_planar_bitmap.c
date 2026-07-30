#include "csb_v1_csbwin_planar_bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
static void expect_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s: got %d, expected %d\n", name, actual, expected);
        failures++;
    }
}

int main(void)
{
    const uint8_t indexed[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    uint8_t *planar = NULL;
    size_t planar_count = 0u;
    CSB_V1_CSBWinPlanarBitmap source;
    uint8_t destination[6 * 4];
    uint8_t color = 0u;

    expect_int("pack", csb_v1_csbwin_planar_bitmap_pack_indexed(
                   indexed, 5u, 2u, &planar, &planar_count), 1);
    expect_int("planar_count", (int)planar_count, 16);
    /* CSBWin Graphics.cpp TAG0088b2 reads four big-endian words in plane
     * order 0..3 for each 16-pixel group. First row colours 1,2,3,4,5 give
     * plane masks A800,6000,1800,0000 respectively. */
    expect_int("plane0_be_hi", planar ? planar[0] : 0, 0xa8);
    expect_int("plane0_be_lo", planar ? planar[1] : 0, 0x00);
    expect_int("plane1_be_hi", planar ? planar[2] : 0, 0x60);
    expect_int("plane1_be_lo", planar ? planar[3] : 0, 0x00);
    expect_int("plane2_be_hi", planar ? planar[4] : 0, 0x18);
    expect_int("plane2_be_lo", planar ? planar[5] : 0, 0x00);
    expect_int("plane3_empty", planar ? planar[6] : 0, 0x00);
    memset(&source, 0, sizeof(source));
    source.bytes = planar;
    source.width = 5u;
    source.height = 2u;
    source.byte_stride = 8u;
    expect_int("pixel_odd", csb_v1_csbwin_planar_bitmap_pixel_at(
                   &source, 4u, 1u, &color), 1);
    expect_int("pixel_odd_value", color, 10);
    expect_int("pixel_padding_rejected", csb_v1_csbwin_planar_bitmap_pixel_at(
                   &source, 5u, 0u, &color), 0);

    memset(destination, 15, sizeof(destination));
    expect_int("blit", csb_v1_csbwin_planar_bitmap_blit_indexed(
                   &source, 0, 0, 5, 2, destination, 6, 4, 6,
                   -1, 1, 5), 1);
    expect_int("clipped_left", destination[6], 2);
    expect_int("transparent_preserved", destination[6 + 3], 15);
    expect_int("second_row", destination[12], 7);
    expect_int("outside_preserved", destination[0], 15);
    expect_int("bad_extent", csb_v1_csbwin_planar_bitmap_blit_indexed(
                   &source, 4, 0, 2, 1, destination, 6, 4, 6,
                   0, 0, -1), 0);
    free(planar);
    if (failures) return 1;
    puts("csbwin planar bitmap: PASS");
    return 0;
}
