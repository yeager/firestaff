#include "csb_v1_startup_img3_decode_pc34_compat.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

/*
 * CSBWin CSBCode.cpp ExpandGraphic (TAG021470--TAG021570) expands the
 * PC C001--C005 stream into the Atari four-plane, 16-pixel-group layout.
 * The public compatibility name predates the discovery of that format; the
 * startup GRAPHICS.DAT records are not ReDMCSB IMG3 records.
 */
typedef struct {
    const uint8_t *source;
    size_t source_size;
    size_t source_offset;
    uint8_t *planes;
    size_t pitch;
    size_t physical_width;
    size_t physical_pixels;
    size_t emitted_pixels;
} csb_v1_startup_planar_decoder_pc34;

static uint16_t csb_v1_startup_read_be16_pc34(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t csb_v1_startup_fnv1a_pc34(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes || count == 0U) return 0U;
    for (index = 0U; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1U;
}

static uint16_t csb_v1_startup_plane_word_pc34(
    const csb_v1_startup_planar_decoder_pc34 *decoder, size_t pixel,
    unsigned int plane)
{
    size_t line = pixel / decoder->physical_width;
    size_t column = pixel % decoder->physical_width;
    size_t offset = line * decoder->pitch + (column / 16U) * 8U + plane * 2U;

    return csb_v1_startup_read_be16_pc34(decoder->planes + offset);
}

static int csb_v1_startup_planar_emit_pixel_pc34(
    csb_v1_startup_planar_decoder_pc34 *decoder, uint8_t color)
{
    size_t line;
    size_t column;
    size_t offset;
    uint16_t mask;
    unsigned int plane;

    if (!decoder || decoder->emitted_pixels >= decoder->physical_pixels) return 0;
    line = decoder->emitted_pixels / decoder->physical_width;
    column = decoder->emitted_pixels % decoder->physical_width;
    offset = line * decoder->pitch + (column / 16U) * 8U;
    mask = (uint16_t)(1U << (15U - (unsigned int)(column & 15U)));
    for (plane = 0U; plane < 4U; ++plane) {
        if ((color & (uint8_t)(1U << plane)) != 0U) {
            uint16_t word = csb_v1_startup_read_be16_pc34(
                decoder->planes + offset + plane * 2U);
            word = (uint16_t)(word | mask);
            decoder->planes[offset + plane * 2U] = (uint8_t)(word >> 8);
            decoder->planes[offset + plane * 2U + 1U] = (uint8_t)word;
        }
    }
    decoder->emitted_pixels++;
    return 1;
}

static int csb_v1_startup_planar_emit_repeat_pc34(
    csb_v1_startup_planar_decoder_pc34 *decoder, uint8_t color, size_t count)
{
    size_t index;
    if (!decoder || count > decoder->physical_pixels - decoder->emitted_pixels)
        return 0;
    for (index = 0U; index < count; ++index) {
        if (!csb_v1_startup_planar_emit_pixel_pc34(decoder, color)) return 0;
    }
    return 1;
}

static int csb_v1_startup_planar_emit_previous_line_pc34(
    csb_v1_startup_planar_decoder_pc34 *decoder, size_t count)
{
    size_t index;

    if (!decoder || decoder->emitted_pixels < decoder->physical_width ||
        count > decoder->physical_pixels - decoder->emitted_pixels) return 0;
    for (index = 0U; index < count; ++index) {
        size_t source_pixel = decoder->emitted_pixels - decoder->physical_width;
        uint8_t color = 0U;
        unsigned int plane;
        for (plane = 0U; plane < 4U; ++plane) {
            size_t column = source_pixel % decoder->physical_width;
            uint16_t word = csb_v1_startup_plane_word_pc34(
                decoder, source_pixel, plane);
            if ((word & (uint16_t)(1U << (15U - (unsigned int)(column & 15U)))) != 0U)
                color = (uint8_t)(color | (uint8_t)(1U << plane));
        }
        if (!csb_v1_startup_planar_emit_pixel_pc34(decoder, color)) return 0;
    }
    return 1;
}

static int csb_v1_startup_planar_emit_literal_pc34(
    csb_v1_startup_planar_decoder_pc34 *decoder, uint8_t first_color,
    size_t count)
{
    if (!decoder || count == 0U || count > decoder->physical_pixels - decoder->emitted_pixels)
        return 0;
    if ((count & 1U) != 0U) {
        if (!csb_v1_startup_planar_emit_pixel_pc34(decoder, first_color)) return 0;
        count--;
    }
    while (count != 0U) {
        uint8_t packed;
        if (decoder->source_offset >= decoder->source_size) return 0;
        packed = decoder->source[decoder->source_offset++];
        if (!csb_v1_startup_planar_emit_pixel_pc34(decoder, (uint8_t)(packed >> 4)) ||
            !csb_v1_startup_planar_emit_pixel_pc34(decoder, (uint8_t)(packed & 0x0fU)))
            return 0;
        count -= 2U;
    }
    return 1;
}

int csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_receipt)
{
    csb_v1_startup_planar_decoder_pc34 decoder;
    uint8_t *planes = NULL;
    size_t pitch;
    size_t physical_width;
    size_t physical_pixels;
    size_t plane_byte_count;
    size_t pixel_count;
    size_t pixel;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!graphic || !indexed_pixels || graphic_byte_count < 5U ||
        expected_width == 0U || expected_height == 0U ||
        csb_v1_startup_read_be16_pc34(graphic) != expected_width ||
        csb_v1_startup_read_be16_pc34(graphic + 2U) != expected_height ||
        expected_height > SIZE_MAX / expected_width) return 0;
    pitch = (((size_t)expected_width + 15U) >> 1U) & ~(size_t)7U;
    if (pitch == 0U || pitch > SIZE_MAX / expected_height ||
        (pitch / 8U) > SIZE_MAX / 16U) return 0;
    physical_width = (pitch / 8U) * 16U;
    if (physical_width == 0U || expected_height > SIZE_MAX / physical_width ||
        expected_height > SIZE_MAX / pitch) return 0;
    physical_pixels = physical_width * expected_height;
    plane_byte_count = pitch * expected_height;
    pixel_count = (size_t)expected_width * expected_height;
    if (pixel_count > indexed_pixel_byte_count) return 0;
    planes = (uint8_t *)calloc(plane_byte_count, 1U);
    if (!planes) return 0;
    memset(&decoder, 0, sizeof(decoder));
    decoder.source = graphic;
    decoder.source_size = graphic_byte_count;
    decoder.source_offset = 4U;
    decoder.planes = planes;
    decoder.pitch = pitch;
    decoder.physical_width = physical_width;
    decoder.physical_pixels = physical_pixels;

    while (decoder.emitted_pixels < decoder.physical_pixels) {
        uint8_t command;
        uint8_t color;
        size_t count;
        /* CSBWin's ExpandGraphic has no source-length argument. The verified
         * PC C001 stream ends before the descriptor's full 320x153 rectangle;
         * the zeroed planar destination supplies that package-defined blank
         * tail. Stop at the owned record boundary instead of reading past it. */
        if (decoder.source_offset >= decoder.source_size) break;
        command = decoder.source[decoder.source_offset++];
        color = (uint8_t)(command & 0x0fU);
        if ((command & 0x80U) == 0U) {
            count = (size_t)(command >> 4U);
            if (count == 0U) count = 1U;
            else count++;
            if (!csb_v1_startup_planar_emit_repeat_pc34(&decoder, color, count)) goto fail;
        } else {
            if (decoder.source_offset >= decoder.source_size) goto fail;
            count = decoder.source[decoder.source_offset++];
            if ((command & 0x40U) != 0U) {
                if (decoder.source_offset >= decoder.source_size) goto fail;
                count = (count << 8U) | decoder.source[decoder.source_offset++];
            }
            count++;
            if ((command & 0x10U) == 0U) {
                if (!csb_v1_startup_planar_emit_repeat_pc34(&decoder, color, count)) goto fail;
            } else if ((command & 0x20U) != 0U) {
                if (!csb_v1_startup_planar_emit_previous_line_pc34(&decoder, count)) goto fail;
            } else if (!csb_v1_startup_planar_emit_literal_pc34(&decoder, color, count)) {
                goto fail;
            }
        }
    }
    for (pixel = 0U; pixel < pixel_count; ++pixel) {
        uint8_t color = 0U;
        size_t column = pixel % expected_width;
        size_t plane_pixel = (pixel / expected_width) * physical_width + column;
        unsigned int plane;
        for (plane = 0U; plane < 4U; ++plane) {
            uint16_t word = csb_v1_startup_plane_word_pc34(&decoder, plane_pixel, plane);
            if ((word & (uint16_t)(1U << (15U - (unsigned int)(column & 15U)))) != 0U)
                color = (uint8_t)(color | (uint8_t)(1U << plane));
        }
        indexed_pixels[pixel] = color;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->width = expected_width;
        out_receipt->height = expected_height;
        out_receipt->stream_byte_count = graphic_byte_count;
        out_receipt->stream_bytes_consumed = decoder.source_offset;
        out_receipt->emitted_planar_pixels = decoder.emitted_pixels;
        out_receipt->physical_planar_pixels = decoder.physical_pixels;
        out_receipt->stream_fnv1a =
            csb_v1_startup_fnv1a_pc34(graphic, graphic_byte_count);
        out_receipt->indexed_pixel_fnv1a =
            csb_v1_startup_fnv1a_pc34(indexed_pixels, pixel_count);
        out_receipt->ended_at_record_boundary =
            decoder.source_offset == decoder.source_size;
        out_receipt->implicit_blank_tail =
            decoder.emitted_pixels < decoder.physical_pixels;
        out_receipt->indexed_colors_are_4bit = 1;
        if (out_receipt->stream_fnv1a == 0U ||
            out_receipt->indexed_pixel_fnv1a == 0U) goto fail;
    }
    free(planes);
    return 1;
fail:
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    free(planes);
    return 0;
}

int csb_v1_startup_img3_decode_to_indexed_pc34_compat(
    const uint8_t *graphic, size_t graphic_byte_count, uint16_t expected_width,
    uint16_t expected_height, uint8_t *indexed_pixels,
    size_t indexed_pixel_byte_count)
{
    return csb_v1_startup_img3_decode_to_indexed_with_receipt_pc34_compat(
        graphic, graphic_byte_count, expected_width, expected_height,
        indexed_pixels, indexed_pixel_byte_count, NULL);
}
