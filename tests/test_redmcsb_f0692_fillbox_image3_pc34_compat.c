#include "redmcsb_f0692_fillbox_image3_pc34_compat.h"

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

static unsigned int pixel_at(const uint8_t *bitmap, size_t row_bytes, int x, int y)
{
    const uint8_t packed = bitmap[(size_t)y * row_bytes + (size_t)x / 2u];

    return (x & 1) == 0 ? packed >> 4 : packed & 0x0fu;
}

int main(void)
{
    uint8_t bitmap[24];
    uint8_t before[24];
    const int16_t box[4] = {1, 6, 1, 2};
    const int16_t single_line_box[4] = {2, 5, 0, 0};
    const int16_t high_bit_color_box[4] = {1, 4, 0, 1};
    const int16_t bad_box[4] = {7, 8, 0, 0};
    int ok = 1;

    memset(bitmap, 0xa5, sizeof(bitmap));
    ok &= check(redmcsb_f0692_fillbox_image3_pc34_compat(
                    bitmap, sizeof(bitmap), 4u, 3u, box, 0x000cu) == 1,
                "fills an inclusive multi-line box in the caller buffer");
    ok &= check(pixel_at(bitmap, 4u, 0, 1) == 10u &&
                    pixel_at(bitmap, 4u, 1, 1) == 12u &&
                    pixel_at(bitmap, 4u, 6, 2) == 12u &&
                    pixel_at(bitmap, 4u, 7, 2) == 5u,
                "preserves edge nibbles and fills whole packed bytes");
    ok &= check(bitmap[5] == 0xccu && bitmap[9] == 0xccu,
                "uses the same packed color for complete byte spans");

    memset(bitmap, 0xa5, sizeof(bitmap));
    ok &= check(redmcsb_f0692_fillbox_image3_pc34_compat(
                    bitmap, sizeof(bitmap), 4u, 3u, single_line_box, 0x0003u) == 1,
                "accepts a full-byte fill on one line");
    ok &= check(bitmap[1] == 0x33u && bitmap[2] == 0x33u &&
                    bitmap[0] == 0xa5u && bitmap[3] == 0xa5u,
                "does not modify pixels or rows outside the box");

    memset(bitmap, 0x00, sizeof(bitmap));
    ok &= check(redmcsb_f0692_fillbox_image3_pc34_compat(
                    bitmap, sizeof(bitmap), 4u, 3u, high_bit_color_box, 0x8009u) == 1,
                "fills every pixel when upper color bits are present");
    ok &= check(pixel_at(bitmap, 4u, 1, 0) == 9u &&
                    pixel_at(bitmap, 4u, 2, 0) == 9u &&
                    pixel_at(bitmap, 4u, 3, 0) == 9u &&
                    pixel_at(bitmap, 4u, 4, 0) == 9u &&
                    pixel_at(bitmap, 4u, 1, 1) == 9u &&
                    pixel_at(bitmap, 4u, 2, 1) == 9u &&
                    pixel_at(bitmap, 4u, 3, 1) == 9u &&
                    pixel_at(bitmap, 4u, 4, 1) == 9u,
                "uses F0685's low-nibble color semantics without color flags");

    memcpy(before, bitmap, sizeof(bitmap));
    ok &= check(redmcsb_f0692_fillbox_image3_pc34_compat(
                    bitmap, 11u, 4u, 3u, box, 1u) == 0 &&
                    redmcsb_f0692_fillbox_image3_pc34_compat(
                    bitmap, sizeof(bitmap), 4u, 3u, bad_box, 1u) == 0,
                "rejects undersized and out-of-raster caller buffers");
    ok &= check(memcmp(bitmap, before, sizeof(bitmap)) == 0,
                "rejected fills leave the caller buffer unchanged");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0692_fillbox_image3_pc34_compat");
    return 0;
}
