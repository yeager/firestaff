#include "redmcsb_f0134_f0135_presentation_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int check(int condition, const char *label)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static uint32_t fnv1a(const uint8_t *bytes, size_t byte_count)
{
    uint32_t value = 2166136261u;
    size_t index;
    for (index = 0u; index < byte_count; ++index) {
        value ^= bytes[index];
        value *= 16777619u;
    }
    return value == 0u ? 1u : value;
}

int main(void)
{
    uint8_t pixels[8 * 6];
    uint8_t before[sizeof(pixels)];
    uint8_t palette[48];
    const uint8_t source[12] = {
        10u, 3u, 10u, 4u,
        5u, 6u, 7u, 8u,
        9u, 10u, 11u, 12u
    };
    Redmcsb_F0134F0135_PresentationTargetPc34 target;
    const int16_t clipped_box[4] = { -2, 2, -1, 1 };
    const int16_t alternate_box[4] = { -1, 3, 2, 3 };
    const int16_t outside_box[4] = { 10, 12, 0, 1 };
    int ok = 1;
    size_t index;

    for (index = 0u; index < sizeof(palette); ++index) {
        palette[index] = (uint8_t)(index & 0x3fu);
    }
    memset(pixels, 1, sizeof(pixels));
    memset(&target, 0, sizeof(target));
    ok &= check(redmcsb_f0134_f0135_presentation_target_bind_pc34(
                    &target, pixels, sizeof(pixels), 6, 4, 8, UINT32_C(0x7134),
                    palette, sizeof(palette), 1, 1) == 1,
                "binds a verified source frame and its exact VGA palette");
    ok &= check(redmcsb_f0134_fill_bitmap_pc34(&target, 12u) == 1 &&
                    pixels[0] == 12u && pixels[5] == 12u &&
                    pixels[8 + 5] == 12u && pixels[6] == 1u,
                "F0134 fills only indexed target pixels and preserves pitch guards");
    ok &= check(redmcsb_f0135_fill_box_pc34(&target, clipped_box, 5u) == 1 &&
                    pixels[0] == 5u && pixels[2] == 5u &&
                    pixels[8] == 5u && pixels[8 + 2] == 5u &&
                    pixels[8 + 3] == 12u,
                "F0135 clips an inclusive caller box to the source-bound target");
    ok &= check(redmcsb_f0135_fill_box_pc34(&target, alternate_box,
                                              UINT16_C(0x800e)) == 1 &&
                    pixels[2 * 8] == 12u && pixels[2 * 8 + 1] == 14u &&
                    pixels[2 * 8 + 2] == 12u && pixels[2 * 8 + 3] == 14u,
                "F0135 retains bit-15 alternate pixels with caller-box phase");
    memcpy(before, pixels, sizeof(pixels));
    ok &= check(redmcsb_f0135_fill_box_pc34(&target, outside_box, 3u) == 1 &&
                    memcmp(before, pixels, sizeof(pixels)) == 0,
                "fully clipped F0135 boxes do not fabricate a surface write");
    ok &= check(redmcsb_f0134_f0135_blit_indexed_pc34(
                    &target, source, sizeof(source), 4, 3,
                    target.palette_fingerprint, -1, 1, 10) == 1 &&
                    pixels[1 * 8] == 3u && pixels[1 * 8 + 1] == 5u &&
                    pixels[1 * 8 + 2] == 4u &&
                    pixels[2 * 8] == 6u && pixels[2 * 8 + 1] == 7u &&
                    pixels[2 * 8 + 2] == 8u,
                "source blit clips at target edges and preserves C10 transparency");
    memcpy(before, pixels, sizeof(pixels));
    ok &= check(redmcsb_f0134_f0135_blit_indexed_pc34(
                    &target, source, sizeof(source), 4, 3,
                    fnv1a((const uint8_t *)"wrong", 5u), 0, 0, -1) == 0 &&
                    memcmp(before, pixels, sizeof(pixels)) == 0,
                "palette drift rejects before a source blit writes pixels");
    memset(&target, 0, sizeof(target));
    ok &= check(redmcsb_f0134_f0135_presentation_target_bind_pc34(
                    &target, pixels, sizeof(pixels), 6, 4, 8, UINT32_C(0x7134),
                    palette, sizeof(palette), 0, 1) == 0,
                "unverified source material cannot create a fallback target");
    if (!ok) return 1;
    puts("PASS redmcsb_f0134_f0135_presentation_pc34_compat");
    return 0;
}
