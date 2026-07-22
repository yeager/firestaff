#include "redmcsb_f0692_fillbox_f0135_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static unsigned int pixel_color(const unsigned char *bitmap,
                                size_t row_bytes,
                                int x,
                                int y)
{
    const unsigned char *group = bitmap + (size_t)y * row_bytes +
                                 (size_t)(x / 16) * 8u;
    const unsigned int mask = 0x8000u >> (x & 15);
    unsigned int color = 0;
    unsigned int plane;

    for (plane = 0; plane < 4u; ++plane) {
        const unsigned int word = ((unsigned int)group[plane * 2u] << 8) |
                                  group[plane * 2u + 1u];
        if ((word & mask) != 0u) color |= 1u << plane;
    }
    return color;
}

int main(void)
{
    unsigned char bitmap[32];
    unsigned char before[32];
    const int16_t box[4] = { 15, 17, 0, 1 };
    const int16_t invalid_box[4] = { 0, 32, 0, 0 };
    int ok = 1;

    memset(bitmap, 0, sizeof(bitmap));
    ok &= check(redmcsb_f0692_fillbox_f0135_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, box, 0x000au) == 1,
                "F0692 admits the F0135 planar fill");
    ok &= check(pixel_color(bitmap, 16u, 14, 0) == 0u &&
                    pixel_color(bitmap, 16u, 15, 0) == 10u &&
                    pixel_color(bitmap, 16u, 16, 1) == 10u &&
                    pixel_color(bitmap, 16u, 18, 1) == 0u,
                "the full F0692 to F0135 chain preserves the inclusive box");

    memset(bitmap, 0x5au, sizeof(bitmap));
    memcpy(before, bitmap, sizeof(bitmap));
    ok &= check(redmcsb_f0692_fillbox_f0135_pc34_compat(
                    bitmap, sizeof(bitmap), 16u, 2u, invalid_box, 1u) == 0,
                "invalid F0692 input is rejected by F0135");
    ok &= check(memcmp(bitmap, before, sizeof(bitmap)) == 0,
                "rejected input cannot synthesize or alter bitmap data");
    ok &= check(strstr(redmcsb_f0692_fillbox_f0135_source_evidence_pc34(),
                       "F0135_VIDEO_FillBox") != NULL,
                "source evidence retains the F0135 route");

    if (!ok) return 1;
    puts("PASS redmcsb_f0692_fillbox_f0135_pc34_compat");
    return 0;
}
