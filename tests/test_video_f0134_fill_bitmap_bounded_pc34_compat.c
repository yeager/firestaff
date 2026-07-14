#include "video_f0134_fill_bitmap_bounded_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    uint8_t bitmap[17];
    const uint8_t expected_color_five[8] = {
        0xffu, 0xffu, 0x00u, 0x00u, 0xffu, 0xffu, 0x00u, 0x00u
    };
    const uint8_t expected_color_ten[8] = {
        0x00u, 0x00u, 0xffu, 0xffu, 0x00u, 0x00u, 0xffu, 0xffu
    };
    int ok = 1;

    memset(bitmap, 0xa5, sizeof(bitmap));
    ok &= check(video_f0134_fill_bitmap_bounded_pc34_compat(
                    bitmap, 16u, 0xf5u, 2u) == 1,
                "two complete units are accepted");
    ok &= check(memcmp(bitmap, expected_color_five, sizeof(expected_color_five)) == 0 &&
                    memcmp(bitmap + 8u, expected_color_five,
                           sizeof(expected_color_five)) == 0,
                "source plane order expands only color bits zero through three");
    ok &= check(bitmap[16] == 0xa5u, "bounded fill preserves trailing guard byte");

    memset(bitmap, 0xa5, sizeof(bitmap));
    ok &= check(video_f0134_fill_bitmap_bounded_pc34_compat(
                    bitmap, 8u, 0x0au, 1u) == 1 &&
                    memcmp(bitmap, expected_color_ten, sizeof(expected_color_ten)) == 0,
                "plane masks match source color bits one and three");
    ok &= check(bitmap[8] == 0xa5u, "exact capacity does not write beyond a unit");

    memset(bitmap, 0x3cu, sizeof(bitmap));
    ok &= check(video_f0134_fill_bitmap_bounded_pc34_compat(
                    bitmap, 15u, 0x0fu, 2u) == 0 &&
                    bitmap[0] == 0x3cu && bitmap[14] == 0x3cu,
                "undersized destination rejects without mutation");
    ok &= check(video_f0134_fill_bitmap_bounded_pc34_compat(
                    bitmap, sizeof(bitmap), 0x0fu, 0u) == 0 &&
                    bitmap[0] == 0x3cu,
                "zero source-unit count rejects without mutation");
    ok &= check(video_f0134_fill_bitmap_bounded_pc34_compat(
                    NULL, 8u, 0x0fu, 1u) == 0,
                "null destination rejects");

    if (!ok) {
        return 1;
    }
    puts("PASS video_f0134_fill_bitmap_bounded_pc34_compat");
    return 0;
}
