#include "dm1_v1_f0431_f0436_palette_step_pc34_compat.h"

#include <string.h>

static int dm1_v1_raw_palette_is_valid_pc34(const DM1_V1_RawPalettePc34 *palette)
{
    size_t index;

    if (!palette || !palette->bytes ||
        palette->byte_count != DM1_V1_PALETTE_BYTE_COUNT_PC34 ||
        !palette->original_pc34_palette || !palette->source_bytes_verified ||
        !palette->no_synthetic_palette) {
        return 0;
    }
    for (index = 0u; index < DM1_V1_PALETTE_ENTRY_COUNT_PC34; ++index) {
        const uint16_t color = (uint16_t)((uint16_t)palette->bytes[index * 2u] << 8u) |
            palette->bytes[index * 2u + 1u];
        if ((color & 0xf000u) != 0u) return 0;
    }
    return 1;
}

static uint16_t dm1_v1_read_color_pc34(const uint8_t *bytes, size_t index)
{
    return (uint16_t)((uint16_t)bytes[index * 2u] << 8u) | bytes[index * 2u + 1u];
}

static void dm1_v1_write_color_pc34(uint8_t *bytes, size_t index, uint16_t color)
{
    bytes[index * 2u] = (uint8_t)(color >> 8u);
    bytes[index * 2u + 1u] = (uint8_t)color;
}

static uint16_t dm1_v1_f0431_darken_color_pc34(uint16_t color)
{
    if ((color & 0x000fu) != 0u) --color;
    if ((color & 0x00f0u) != 0u) color = (uint16_t)(color - 0x0010u);
    if ((color & 0x0f00u) != 0u) color = (uint16_t)(color - 0x0100u);
    return color;
}

static uint16_t dm1_v1_f0436_advance_color_pc34(uint16_t current, uint16_t target)
{
    if ((current & 0x000fu) > (target & 0x000fu)) --current;
    if ((current & 0x000fu) < (target & 0x000fu)) ++current;
    if ((current & 0x00f0u) > (target & 0x00f0u)) current = (uint16_t)(current - 0x0010u);
    if ((current & 0x00f0u) < (target & 0x00f0u)) current = (uint16_t)(current + 0x0010u);
    if ((current & 0x0f00u) > (target & 0x0f00u)) current = (uint16_t)(current - 0x0100u);
    if ((current & 0x0f00u) < (target & 0x0f00u)) current = (uint16_t)(current + 0x0100u);
    return current;
}

int dm1_v1_f0431_get_darkened_color_pc34(
    const DM1_V1_RawPalettePc34 *palette,
    uint8_t color_index,
    DM1_V1_F0431DarkenedColorReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_raw_palette_is_valid_pc34(palette) ||
        color_index >= DM1_V1_PALETTE_ENTRY_COUNT_PC34) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->source_color = dm1_v1_read_color_pc34(palette->bytes, color_index);
        out_receipt->darkened_color = dm1_v1_f0431_darken_color_pc34(out_receipt->source_color);
        out_receipt->source_palette_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence = dm1_v1_f0431_f0436_palette_step_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0436_fade_to_palette_step_pc34(
    const DM1_V1_RawPalettePc34 *current_palette,
    const DM1_V1_RawPalettePc34 *target_palette,
    int vertical_blank_authorized,
    DM1_V1_F0436FadeStepReceiptPc34 *out_receipt)
{
    size_t index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_raw_palette_is_valid_pc34(current_palette) ||
        !dm1_v1_raw_palette_is_valid_pc34(target_palette) ||
        !vertical_blank_authorized) {
        return 0;
    }
    if (out_receipt) {
        for (index = 0u; index < DM1_V1_PALETTE_ENTRY_COUNT_PC34; ++index) {
            dm1_v1_write_color_pc34(
                out_receipt->next_palette,
                index,
                dm1_v1_f0436_advance_color_pc34(
                    dm1_v1_read_color_pc34(current_palette->bytes, index),
                    dm1_v1_read_color_pc34(target_palette->bytes, index)));
        }
        out_receipt->accepted = 1;
        out_receipt->source_palette_bound = 1;
        out_receipt->target_palette_bound = 1;
        out_receipt->vertical_blank_authorized = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence = dm1_v1_f0431_f0436_palette_step_source_evidence_pc34();
    }
    return 1;
}

const char *dm1_v1_f0431_f0436_palette_step_source_evidence_pc34(void)
{
    return "ReDMCSB DARKCOLR.C:2-21 F0431 decrements non-zero 0x0RGB "
           "components; PALETTE.C:209-323 F0436 MEDIA108 changes each of 16 "
           "raw PC34 palette entries by exactly one component unit per VBlank.";
}
