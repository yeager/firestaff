#include "redmcsb_f0952_japanese_print_pc34_compat.h"

#include "redmcsb_f0949_japanese_pc34_compat.h"

#include <stddef.h>

static const uint8_t redmcsb_f0952_7423_pattern[
    REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT] = {
    0x40U, 0x80U, 0x21U, 0xFCU, 0x0EU, 0x88U, 0x02U, 0x50U,
    0x41U, 0x60U, 0x2FU, 0x80U, 0x00U, 0xFCU, 0x01U, 0x40U,
    0xE0U, 0x40U, 0x2FU, 0xFEU, 0x20U, 0x40U, 0x24U, 0x44U,
    0x24U, 0x44U, 0x57U, 0xFCU, 0x88U, 0x00U, 0x07U, 0xFEU
};

static bool redmcsb_f0952_is_shift_jis_lead(uint8_t character)
{
    return (character >= UINT8_C(0x81) && character <= UINT8_C(0x9f)) ||
           (character >= UINT8_C(0xe0) && character <= UINT8_C(0xfc));
}

static bool redmcsb_f0952_uses_7423_pattern(
    redmcsb_f0952_japanese_print_profile_pc34_compat profile)
{
    return profile == REDMCSB_F0952_JAPANESE_PRINT_P20JB ||
           profile == REDMCSB_F0952_JAPANESE_PRINT_P31J_GAME;
}

static uint16_t redmcsb_f0952_bytes_per_row(
    const redmcsb_f0952_japanese_print_bitmap_pc34_compat *bitmap,
    redmcsb_f0952_japanese_print_profile_pc34_compat profile)
{
    if (profile == REDMCSB_F0952_JAPANESE_PRINT_P31J_CEDT) {
        return UINT16_C(32);
    }

    return (uint16_t)(bitmap->pixel_width >> 1);
}

static void redmcsb_f0952_write_pixels(uint8_t *destination, uint16_t bits,
                                        uint8_t pixel_count,
                                        int16_t text_color,
                                        int16_t background_color)
{
    uint8_t pixel;

    for (pixel = 0U; pixel < pixel_count; ++pixel) {
        uint8_t high = (bits & UINT16_C(0x8000)) != 0U
                           ? (uint8_t)text_color
                           : (uint8_t)background_color;
        uint8_t low = (bits & UINT16_C(0x4000)) != 0U
                          ? (uint8_t)text_color
                          : (uint8_t)background_color;

        *destination++ = (uint8_t)((high << 4) | low);
        bits = (uint16_t)(bits << 2);
    }
}

int16_t redmcsb_f0952_japanese_print_pc34_compat(
    const uint8_t *string,
    int16_t byte_index,
    const redmcsb_f0952_japanese_print_bitmap_pc34_compat *bitmap,
    int16_t *width,
    int16_t *height,
    int16_t text_color,
    int16_t background_color,
    redmcsb_f0952_japanese_print_profile_pc34_compat profile,
    const redmcsb_f0952_japanese_print_io_pc34_compat *io,
    void *context)
{
    uint8_t character = string[byte_index];
    uint8_t pattern[REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT];
    uint8_t bytes_per_character;
    uint16_t bytes_per_row;
    uint8_t row;

    if (character == UINT8_C(0x1b) || character == UINT8_C(0x7c)) {
        *width = 0;
        *height = 0;
        return (int16_t)(byte_index + 1);
    }

    if (redmcsb_f0952_is_shift_jis_lead(character)) {
        bytes_per_character = 2U;
        *width = 16;
    } else {
        bytes_per_character = 1U;
        *width = 8;
    }
    *height = 16;

    if (bitmap == NULL) {
        return (int16_t)(byte_index + bytes_per_character);
    }

    bytes_per_row = redmcsb_f0952_bytes_per_row(bitmap, profile);
    if (bytes_per_character == 2U) {
        int16_t character_code = redmcsb_f0949_japanese_pc34_compat(
            (int16_t)(((uint16_t)character << 8) | string[byte_index + 1]));

        if (redmcsb_f0952_uses_7423_pattern(profile) &&
            (uint16_t)character_code == UINT16_C(0x7423)) {
            for (row = 0U;
                 row < REDMCSB_F0952_JAPANESE_PRINT_KANJI_PATTERN_BYTE_COUNT;
                 ++row) {
                pattern[row] = redmcsb_f0952_7423_pattern[row];
            }
        } else {
            io->pc98_pattern(context, character_code, pattern);
        }

        for (row = 0U; row < 16U; ++row) {
            redmcsb_f0952_write_pixels(bitmap->pixels +
                                            (uint16_t)(row * bytes_per_row),
                                        (uint16_t)(((uint16_t)pattern[row * 2U]
                                                    << 8) |
                                                   pattern[row * 2U + 1U]),
                                        8U, text_color, background_color);
        }
    } else {
        io->ank_pattern(context, (int16_t)character, pattern);
        for (row = 0U; row < 16U; ++row) {
            redmcsb_f0952_write_pixels(bitmap->pixels +
                                            (uint16_t)(row * bytes_per_row),
                                        (uint16_t)((uint16_t)pattern[row] << 8),
                                        4U, text_color, background_color);
        }
    }

    return (int16_t)(byte_index + bytes_per_character);
}

const char *redmcsb_f0952_japanese_print_source_evidence_pc34(void)
{
    return "ReDMCSB JAPANESE.C:270-381 defines F0952_JAPANESE_Print for "
           "MEDIA459_P20JA_P20JB_P31J: 0x1B/0x7C are zero-size controls, "
           "Shift-JIS leads 0x81-0x9F and 0xE0-0xFC are 16x16, remaining "
           "bytes are 8x16 ANK glyphs, and each packed glyph bit becomes "
           "a high/low 4-bit text or background pixel. JAPANESE.C:299-323 "
           "supplies the literal 0x7423 pattern for P20JB and P31J game; "
           "JAPANESE.C:328-377 supplies the profile-specific row stride.";
}
