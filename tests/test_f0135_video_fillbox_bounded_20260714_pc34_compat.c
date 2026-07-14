#include "f0135_video_fillbox_bounded_20260714_pc34_compat.h"

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
    uint8_t bitmap[12];
    uint8_t before[12];
    const int16_t center_box[4] = {1, 3, 1, 2};
    const int16_t clipped_box[4] = {-3, 2, -1, 1};
    const int16_t empty_box[4] = {4, 3, 0, 1};
    const int16_t outside_box[4] = {9, 10, 0, 1};
    int ok = 1;

    memset(bitmap, 0xa5, sizeof(bitmap));
    ok &= check(f0135_video_fillbox_bounded_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 5, 4, center_box, 0x0cu) == 6u,
                "inclusive center box reports every written pixel");
    ok &= check(bitmap[3] == 0xacu && bitmap[4] == 0xccu &&
                    bitmap[6] == 0xacu && bitmap[7] == 0xccu,
                "even pixels use the high nibble and odd pixels use the low");
    ok &= check(bitmap[0] == 0xa5u && bitmap[1] == 0xa5u &&
                    bitmap[2] == 0xa5u && bitmap[5] == 0xa5u &&
                    bitmap[8] == 0xa5u && bitmap[11] == 0xa5u,
                "fill leaves pixels outside the source box intact");

    memset(bitmap, 0xa5, sizeof(bitmap));
    ok &= check(f0135_video_fillbox_bounded_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 5, 4, clipped_box, 0x21u) == 6u,
                "partly visible box clips to the raster");
    ok &= check(bitmap[0] == 0x11u && bitmap[1] == 0x15u &&
                    bitmap[3] == 0x11u && bitmap[4] == 0x15u,
                "clipped fill preserves the unaddressed trailing nibble");

    memcpy(before, bitmap, sizeof(bitmap));
    ok &= check(f0135_video_fillbox_bounded_20260714_pc34_compat(
                    bitmap, sizeof(bitmap) - 1u, 5, 4, center_box, 2u) == 0u,
                "undersized packed raster rejects the fill");
    ok &= check(memcmp(bitmap, before, sizeof(bitmap)) == 0,
                "undersized packed raster remains unmodified");
    ok &= check(f0135_video_fillbox_bounded_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 5, 4, empty_box, 2u) == 0u &&
                    f0135_video_fillbox_bounded_20260714_pc34_compat(
                    bitmap, sizeof(bitmap), 5, 4, outside_box, 2u) == 0u,
                "empty and off-raster boxes do not mutate the buffer");
    ok &= check(memcmp(bitmap, before, sizeof(bitmap)) == 0,
                "rejected boxes leave the buffer unchanged");

    if (!ok) {
        return 1;
    }
    puts("PASS f0135_video_fillbox_bounded_20260714_pc34_compat");
    return 0;
}
