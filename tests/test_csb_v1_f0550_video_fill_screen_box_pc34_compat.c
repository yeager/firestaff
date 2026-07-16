#include "redmcsb_f0550_video_fill_screen_box_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static int test_source_named_wrapper_fills_word_box_color_planes(void)
{
    uint8_t bitmap[16] = { 0u };
    const int16_t box[4] = { 0, 0, 0, 0 };

    CHECK(F0550_VIDEO_FillScreenBox(
        bitmap,
        sizeof(bitmap),
        8u,
        2u,
        box,
        false,
        0x0005u) == true);
    CHECK(read_be16(bitmap + 0u) == 0x8000u);
    CHECK(read_be16(bitmap + 2u) == 0x0000u);
    CHECK(read_be16(bitmap + 4u) == 0x8000u);
    CHECK(read_be16(bitmap + 6u) == 0x0000u);
    CHECK(read_be16(bitmap + 8u) == 0x0000u);
    return 0;
}

static int test_byte_box_coordinates_are_supported(void)
{
    uint8_t bitmap[16] = { 0u };
    const uint8_t box[4] = { 1u, 1u, 1u, 1u };

    CHECK(F0550_VIDEO_FillScreenBox(
        bitmap,
        sizeof(bitmap),
        8u,
        2u,
        box,
        true,
        0x0002u) == true);
    CHECK(read_be16(bitmap + 8u) == 0x0000u);
    CHECK(read_be16(bitmap + 10u) == 0x4000u);
    CHECK(read_be16(bitmap + 12u) == 0x0000u);
    CHECK(read_be16(bitmap + 14u) == 0x0000u);
    return 0;
}

static int test_shade_color_uses_alternating_screen_space_pattern(void)
{
    uint8_t bitmap[8] = { 0u };
    const int16_t box[4] = { 0, 1, 0, 0 };

    CHECK(F0550_VIDEO_FillScreenBox(
        bitmap,
        sizeof(bitmap),
        8u,
        1u,
        box,
        false,
        0x8001u) == true);
    CHECK(read_be16(bitmap + 0u) == 0x4000u);
    CHECK(read_be16(bitmap + 2u) == 0x0000u);
    CHECK(read_be16(bitmap + 4u) == 0x0000u);
    CHECK(read_be16(bitmap + 6u) == 0x0000u);
    return 0;
}

static int test_invalid_coordinates_reject_without_mutation(void)
{
    uint8_t bitmap[16] = {
        0x12u, 0x34u, 0x56u, 0x78u,
        0x9au, 0xbcu, 0xdeu, 0xf0u,
        0x11u, 0x22u, 0x33u, 0x44u,
        0x55u, 0x66u, 0x77u, 0x88u
    };
    const uint8_t before[16] = {
        0x12u, 0x34u, 0x56u, 0x78u,
        0x9au, 0xbcu, 0xdeu, 0xf0u,
        0x11u, 0x22u, 0x33u, 0x44u,
        0x55u, 0x66u, 0x77u, 0x88u
    };
    const int16_t box[4] = { 2, 1, 0, 0 };

    CHECK(F0550_VIDEO_FillScreenBox(
        bitmap,
        sizeof(bitmap),
        8u,
        2u,
        box,
        false,
        0x000fu) == false);
    CHECK(memcmp(bitmap, before, sizeof(bitmap)) == 0);
    return 0;
}

static int test_wrapper_matches_pc34_entrypoint(void)
{
    uint8_t wrapper_bitmap[16] = { 0u };
    uint8_t pc34_bitmap[16] = { 0u };
    const int16_t box[4] = { 0, 3, 0, 1 };

    CHECK(F0550_VIDEO_FillScreenBox(
        wrapper_bitmap,
        sizeof(wrapper_bitmap),
        8u,
        2u,
        box,
        false,
        0x0003u) == true);
    CHECK(F0550_VIDEO_FillScreenBox_PC34(
        pc34_bitmap,
        sizeof(pc34_bitmap),
        8u,
        2u,
        box,
        false,
        0x0003u) == true);
    CHECK(memcmp(wrapper_bitmap, pc34_bitmap, sizeof(wrapper_bitmap)) == 0);
    return 0;
}

static int test_source_evidence_names_f0550(void)
{
    const char *evidence =
        redmcsb_f0550_video_fill_screen_box_pc34_compat_source_evidence();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0550_VIDEO_FillScreenBox") != 0);
    CHECK(strstr(evidence, "AMIGA.H:351") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_fills_word_box_color_planes() == 0);
    CHECK(test_byte_box_coordinates_are_supported() == 0);
    CHECK(test_shade_color_uses_alternating_screen_space_pattern() == 0);
    CHECK(test_invalid_coordinates_reject_without_mutation() == 0);
    CHECK(test_wrapper_matches_pc34_entrypoint() == 0);
    CHECK(test_source_evidence_names_f0550() == 0);
    return 0;
}
