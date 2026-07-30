#include "csb_v1_csbwin_packed_bitmap.h"

#include <limits.h>
#include <stdlib.h>

static int csb_v1_csbwin_packed_bitmap_valid(
    const CSB_V1_CSBWinPackedBitmap *bitmap)
{
    unsigned int minimum_stride;

    if (!bitmap || !bitmap->bytes || bitmap->width == 0u ||
        bitmap->height == 0u) return 0;
    minimum_stride = ((unsigned int)bitmap->width + 1u) / 2u;
    return bitmap->byte_stride >= minimum_stride;
}

int csb_v1_csbwin_packed_bitmap_pack_indexed(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height,
    uint8_t **out_bytes, size_t *out_byte_count)
{
    size_t stride, byte_count, x, y;
    uint8_t *bytes;

    if (out_bytes) *out_bytes = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!indexed_pixels || !out_bytes || !out_byte_count || width == 0u ||
        height == 0u) return 0;
    stride = ((size_t)width + 1u) / 2u;
    if (stride == 0u || stride > SIZE_MAX / height) return 0;
    byte_count = stride * height;
    bytes = (uint8_t *)calloc(byte_count, 1u);
    if (!bytes) return 0;
    for (y = 0u; y < height; ++y) {
        for (x = 0u; x < width; ++x) {
            uint8_t color = indexed_pixels[y * width + x];
            if (color > 15u) {
                free(bytes);
                return 0;
            }
            if ((x & 1u) == 0u)
                bytes[y * stride + x / 2u] = (uint8_t)(color << 4u);
            else
                bytes[y * stride + x / 2u] |= color;
        }
    }
    *out_bytes = bytes;
    *out_byte_count = byte_count;
    return 1;
}

int csb_v1_csbwin_packed_bitmap_pixel_at(
    const CSB_V1_CSBWinPackedBitmap *bitmap, uint16_t x, uint16_t y,
    uint8_t *out_color)
{
    uint8_t packed;

    if (out_color) *out_color = 0u;
    if (!out_color || !csb_v1_csbwin_packed_bitmap_valid(bitmap) ||
        x >= bitmap->width || y >= bitmap->height) return 0;
    packed = bitmap->bytes[(size_t)y * bitmap->byte_stride + x / 2u];
    *out_color = (x & 1u) == 0u ? packed >> 4u : packed & 0x0fu;
    return 1;
}

int csb_v1_csbwin_packed_bitmap_blit_indexed(
    const CSB_V1_CSBWinPackedBitmap *source,
    int source_x, int source_y, int width, int height,
    uint8_t *destination, int destination_width, int destination_height,
    int destination_stride, int destination_x, int destination_y,
    int transparent_color)
{
    int x, y, copied = 0;

    if (!csb_v1_csbwin_packed_bitmap_valid(source) || !destination ||
        source_x < 0 || source_y < 0 || width <= 0 || height <= 0 ||
        destination_width <= 0 || destination_height <= 0 ||
        destination_stride < destination_width || transparent_color < -1 ||
        transparent_color > 15 || source_x > (int)source->width - width ||
        source_y > (int)source->height - height) return 0;
    for (y = 0; y < height; ++y) {
        const int dst_y = destination_y + y;
        if (dst_y < 0 || dst_y >= destination_height) continue;
        for (x = 0; x < width; ++x) {
            const int dst_x = destination_x + x;
            uint8_t color;
            if (dst_x < 0 || dst_x >= destination_width ||
                !csb_v1_csbwin_packed_bitmap_pixel_at(
                    source, (uint16_t)(source_x + x),
                    (uint16_t)(source_y + y), &color)) continue;
            if (transparent_color >= 0 && color == (uint8_t)transparent_color)
                continue;
            destination[(size_t)dst_y * destination_stride + dst_x] = color;
            copied = 1;
        }
    }
    return copied;
}
