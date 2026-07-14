#include "redmcsb_f0550_video_fill_screen_box_pc34_compat.h"

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

static uint16_t read_be16(const uint8_t *address)
{
    return (uint16_t)(((uint16_t)address[0] << 8) | address[1]);
}

static unsigned int pixel_color(const uint8_t *bitmap, size_t byte_width,
                                int x, int y)
{
    const uint8_t *group = bitmap + (size_t)y * byte_width +
                           (size_t)(x / 16) * 8u;
    const uint16_t pixel_mask = (uint16_t)(0x8000u >> (x & 15));
    unsigned int color = 0u;
    unsigned int plane;

    for (plane = 0u; plane < 4u; ++plane) {
        if ((read_be16(group + plane * 2u) & pixel_mask) != 0u) {
            color |= 1u << plane;
        }
    }
    return color;
}

int main(void)
{
    uint8_t bitmap[33];
    uint8_t before[33];
    const int16_t word_box[4] = {15, 17, 0, 1};
    const uint8_t byte_box[4] = {2, 3, 1, 1};
    const int16_t invalid_box[4] = {0, 32, 0, 0};
    int ok = 1;

    memset(bitmap, 0, sizeof(bitmap));
    ok &= check(F0550_VIDEO_FillScreenBox_PC34(
                    bitmap, 32u, 16u, 2u, word_box, false, 0x000au),
                "accepts an inclusive word-coordinate box across plane groups");
    ok &= check(pixel_color(bitmap, 16u, 14, 0) == 0u &&
                    pixel_color(bitmap, 16u, 15, 0) == 10u &&
                    pixel_color(bitmap, 16u, 16, 0) == 10u &&
                    pixel_color(bitmap, 16u, 17, 1) == 10u &&
                    pixel_color(bitmap, 16u, 18, 1) == 0u,
                "writes the requested color to all four big-endian planes");
    ok &= check(bitmap[32] == 0u, "exact bounded fill preserves guard byte");

    memset(bitmap, 0, sizeof(bitmap));
    ok &= check(F0550_VIDEO_FillScreenBox_PC34(
                    bitmap, 32u, 16u, 2u, byte_box, true, 0x8005u),
                "accepts source byte-coordinate boxes");
    ok &= check(pixel_color(bitmap, 16u, 2, 1) == 5u &&
                    pixel_color(bitmap, 16u, 3, 1) == 0u,
                "shade phase is anchored to screen coordinates, not box origin");

    memset(bitmap, 0xa5, sizeof(bitmap));
    memcpy(before, bitmap, sizeof(bitmap));
    ok &= check(!F0550_VIDEO_FillScreenBox_PC34(
                    bitmap, 31u, 16u, 2u, word_box, false, 1u) &&
                    !F0550_VIDEO_FillScreenBox_PC34(
                    bitmap, sizeof(bitmap), 15u, 2u, word_box, false, 1u) &&
                    !F0550_VIDEO_FillScreenBox_PC34(
                    bitmap, sizeof(bitmap), 16u, 2u, invalid_box, false, 1u),
                "rejects undersized, non-planar, and out-of-bounds fills");
    ok &= check(memcmp(bitmap, before, sizeof(bitmap)) == 0,
                "rejected fills leave the bitmap untouched");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0550_video_fill_screen_box_pc34_compat");
    return 0;
}
