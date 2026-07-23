#include "dm1_v1_f0431_f0436_palette_step_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static void write_color(uint8_t *bytes, size_t index, uint16_t color)
{
    bytes[index * 2u] = (uint8_t)(color >> 8u);
    bytes[index * 2u + 1u] = (uint8_t)color;
}

static uint16_t read_color(const uint8_t *bytes, size_t index)
{
    return (uint16_t)((uint16_t)bytes[index * 2u] << 8u) | bytes[index * 2u + 1u];
}

static DM1_V1_RawPalettePc34 make_palette(const uint8_t *bytes)
{
    DM1_V1_RawPalettePc34 palette;
    memset(&palette, 0, sizeof(palette));
    palette.bytes = bytes;
    palette.byte_count = DM1_V1_PALETTE_BYTE_COUNT_PC34;
    palette.original_pc34_palette = 1;
    palette.source_bytes_verified = 1;
    palette.no_synthetic_palette = 1;
    return palette;
}

int main(void)
{
    uint8_t current[DM1_V1_PALETTE_BYTE_COUNT_PC34] = {0};
    uint8_t target[DM1_V1_PALETTE_BYTE_COUNT_PC34] = {0};
    DM1_V1_RawPalettePc34 current_palette;
    DM1_V1_RawPalettePc34 target_palette;
    DM1_V1_F0431DarkenedColorReceiptPc34 darkened;
    DM1_V1_F0436FadeStepReceiptPc34 fade;

    write_color(current, 0u, 0x0fffu);
    write_color(current, 1u, 0x0000u);
    write_color(current, 2u, 0x0421u);
    write_color(target, 0u, 0x0000u);
    write_color(target, 1u, 0x0fffu);
    write_color(target, 2u, 0x0532u);
    current_palette = make_palette(current);
    target_palette = make_palette(target);

    CHECK(strstr(dm1_v1_f0431_f0436_palette_step_source_evidence_pc34(), "F0436") != NULL);
    CHECK(dm1_v1_f0431_get_darkened_color_pc34(&current_palette, 0u, &darkened));
    CHECK(darkened.accepted && darkened.source_color == 0x0fffu &&
          darkened.darkened_color == 0x0eeeu && darkened.source_palette_bound &&
          darkened.suppress_synthetic_fallback);
    CHECK(dm1_v1_f0431_get_darkened_color_pc34(&current_palette, 1u, &darkened));
    CHECK(darkened.darkened_color == 0x0000u);
    CHECK(!dm1_v1_f0431_get_darkened_color_pc34(&current_palette, 16u, &darkened));

    CHECK(dm1_v1_f0436_fade_to_palette_step_pc34(
        &current_palette, &target_palette, 1, &fade));
    CHECK(fade.accepted && fade.source_palette_bound && fade.target_palette_bound &&
          fade.vertical_blank_authorized && fade.suppress_synthetic_fallback);
    CHECK(read_color(fade.next_palette, 0u) == 0x0eeeu);
    CHECK(read_color(fade.next_palette, 1u) == 0x0111u);
    CHECK(read_color(fade.next_palette, 2u) == 0x0532u);

    current_palette.source_bytes_verified = 0;
    CHECK(!dm1_v1_f0436_fade_to_palette_step_pc34(
        &current_palette, &target_palette, 1, &fade));
    current_palette.source_bytes_verified = 1;
    CHECK(!dm1_v1_f0436_fade_to_palette_step_pc34(
        &current_palette, &target_palette, 0, &fade));
    target[0] = 0xf0u;
    CHECK(!dm1_v1_f0436_fade_to_palette_step_pc34(
        &current_palette, &target_palette, 1, &fade));

    printf("test_dm1_v1_f0431_f0436_palette_step_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
