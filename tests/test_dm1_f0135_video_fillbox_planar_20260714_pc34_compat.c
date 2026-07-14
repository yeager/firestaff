#include "dm1_f0135_video_fillbox_planar_20260714_pc34_compat.h"

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

static unsigned int pixel_color(const uint8_t *bitmap, size_t row_bytes, int x, int y)
{
    const uint8_t *group = bitmap + (size_t)y * row_bytes +
                           (size_t)(x / 16) * 8u;
    const uint16_t mask = (uint16_t)(0x8000u >> (x & 15));
    unsigned int color = 0;
    unsigned int plane;

    for (plane = 0; plane < 4u; ++plane) {
        if ((read_be16(group + plane * 2u) & mask) != 0) {
            color |= 1u << plane;
        }
    }
    return color;
}

int main(void)
{
    uint8_t bitmap[32];
    uint8_t before[32];
    const int16_t inclusive_box[4] = {15, 17, 0, 1};
    const int16_t alternate_box[4] = {1, 4, 0, 1};
    const int16_t negative_box[4] = {-1, 1, 0, 0};
    const int16_t outside_box[4] = {0, 32, 0, 0};
    const int16_t reversed_box[4] = {3, 2, 0, 0};
    int ok = 1;

    memset(bitmap, 0, sizeof(bitmap));
    ok &= check(dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, inclusive_box, 0x0au) == 1,
                "accepts an inclusive rectangle spanning planar word groups");
    ok &= check(pixel_color(bitmap, 16u, 14, 0) == 0u &&
                    pixel_color(bitmap, 16u, 15, 0) == 10u &&
                    pixel_color(bitmap, 16u, 16, 0) == 10u &&
                    pixel_color(bitmap, 16u, 17, 1) == 10u &&
                    pixel_color(bitmap, 16u, 18, 1) == 0u,
                "writes color bits across all four interleaved big-endian planes");
    ok &= check(bitmap[0] == 0x00u && bitmap[1] == 0x00u &&
                    bitmap[2] == 0x00u && bitmap[3] == 0x01u &&
                    bitmap[4] == 0x00u && bitmap[5] == 0x00u &&
                    bitmap[6] == 0x00u && bitmap[7] == 0x01u,
                "plane one and plane three receive the first group's final pixel");

    memset(bitmap, 0, sizeof(bitmap));
    ok &= check(dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, alternate_box, 0x8005u) == 1,
                "accepts the source alternate-pixel color flag");
    ok &= check(pixel_color(bitmap, 16u, 1, 0) == 5u &&
                    pixel_color(bitmap, 16u, 2, 0) == 0u &&
                    pixel_color(bitmap, 16u, 3, 0) == 5u &&
                    pixel_color(bitmap, 16u, 4, 0) == 0u &&
                    pixel_color(bitmap, 16u, 1, 1) == 0u &&
                    pixel_color(bitmap, 16u, 2, 1) == 5u &&
                    pixel_color(bitmap, 16u, 3, 1) == 0u &&
                    pixel_color(bitmap, 16u, 4, 1) == 5u,
                "bit 15 alternates the fill phase on each source scanline");

    memset(bitmap, 0xa5, sizeof(bitmap));
    memcpy(before, bitmap, sizeof(bitmap));
    ok &= check(dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap) - 1u, 16u, 2u, inclusive_box, 1u) == 0 &&
                    dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, negative_box, 1u) == 0 &&
                    dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, outside_box, 1u) == 0 &&
                    dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, reversed_box, 1u) == 0 &&
                    dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 15u, 2u, inclusive_box, 1u) == 0,
                "rejects insufficient storage, invalid boxes, and non-planar strides");
    ok &= check(memcmp(bitmap, before, sizeof(bitmap)) == 0,
                "rejected calls leave the bitmap untouched");

    if (!ok) {
        return 1;
    }
    puts("PASS dm1_f0135_video_fillbox_planar_20260714_pc34_compat");
    return 0;
}
