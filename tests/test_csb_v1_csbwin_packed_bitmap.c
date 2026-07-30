#include "csb_v1_csbwin_packed_bitmap.h"

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
    uint8_t *packed = NULL;
    size_t packed_count = 0u;
    CSB_V1_CSBWinPackedBitmap source;
    uint8_t destination[6 * 4];
    uint8_t color = 0u;

    expect_int("pack", csb_v1_csbwin_packed_bitmap_pack_indexed(
                   indexed, 5u, 2u, &packed, &packed_count), 1);
    expect_int("packed_count", (int)packed_count, 6);
    expect_int("high_low_0", packed ? packed[0] : 0, 0x12);
    expect_int("high_low_1", packed ? packed[1] : 0, 0x34);
    expect_int("odd_high", packed ? packed[2] : 0, 0x50);
    expect_int("row_1", packed ? packed[3] : 0, 0x67);

    memset(&source, 0, sizeof(source));
    source.bytes = packed;
    source.width = 5u;
    source.height = 2u;
    source.byte_stride = 3u;
    expect_int("pixel_odd", csb_v1_csbwin_packed_bitmap_pixel_at(
                   &source, 4u, 1u, &color), 1);
    expect_int("pixel_odd_value", color, 10);
    expect_int("pixel_padding_rejected", csb_v1_csbwin_packed_bitmap_pixel_at(
                   &source, 5u, 0u, &color), 0);

    memset(destination, 15, sizeof(destination));
    expect_int("blit", csb_v1_csbwin_packed_bitmap_blit_indexed(
                   &source, 0, 0, 5, 2, destination, 6, 4, 6,
                   -1, 1, 5), 1);
    expect_int("clipped_left", destination[6], 2);
    expect_int("transparent_preserved", destination[6 + 3], 15);
    expect_int("second_row", destination[12], 7);
    expect_int("outside_preserved", destination[0], 15);
    expect_int("bad_extent", csb_v1_csbwin_packed_bitmap_blit_indexed(
                   &source, 4, 0, 2, 1, destination, 6, 4, 6,
                   0, 0, -1), 0);

    free(packed);
    if (failures) return 1;
    puts("csbwin packed bitmap: PASS");
    return 0;
}
