#include "csb_v1_csbwin_planar_bitmap.h"

#include <stdlib.h>

static uint16_t csb_v1_csbwin_read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static void csb_v1_csbwin_write_be16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value >> 8u);
    bytes[1] = (uint8_t)value;
}

static int csb_v1_csbwin_planar_bitmap_valid(
    const CSB_V1_CSBWinPlanarBitmap *bitmap)
{
    unsigned int minimum_stride;

    if (!bitmap || !bitmap->bytes || bitmap->width == 0u ||
        bitmap->height == 0u) return 0;
    minimum_stride = (((unsigned int)bitmap->width + 15u) / 16u) * 8u;
    return bitmap->byte_stride >= minimum_stride;
}

int csb_v1_csbwin_planar_bitmap_pack_indexed(
    const uint8_t *indexed_pixels, uint16_t width, uint16_t height,
    uint8_t **out_bytes, size_t *out_byte_count)
{
    size_t stride, byte_count, x, y;
    uint8_t *bytes;

    if (out_bytes) *out_bytes = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!indexed_pixels || !out_bytes || !out_byte_count || width == 0u ||
        height == 0u) return 0;
    stride = (((size_t)width + 15u) / 16u) * 8u;
    if (stride == 0u || stride > SIZE_MAX / height) return 0;
    byte_count = stride * height;
    bytes = (uint8_t *)calloc(byte_count, 1u);
    if (!bytes) return 0;
    for (y = 0u; y < height; ++y) {
        for (x = 0u; x < width; ++x) {
            uint8_t color = indexed_pixels[y * width + x];
            unsigned int plane;
            size_t base = y * stride + (x / 16u) * 8u;
            uint16_t mask = (uint16_t)(1u << (15u - (x & 15u)));
            if (color > 15u) {
                free(bytes);
                return 0;
            }
            for (plane = 0u; plane < 4u; ++plane) {
                uint16_t word;
                if ((color & (uint8_t)(1u << plane)) == 0u) continue;
                word = csb_v1_csbwin_read_be16(bytes + base + plane * 2u);
                csb_v1_csbwin_write_be16(bytes + base + plane * 2u,
                                          (uint16_t)(word | mask));
            }
        }
    }
    *out_bytes = bytes;
    *out_byte_count = byte_count;
    return 1;
}

int csb_v1_csbwin_planar_bitmap_pixel_at(
    const CSB_V1_CSBWinPlanarBitmap *bitmap, uint16_t x, uint16_t y,
    uint8_t *out_color)
{
    unsigned int plane;
    uint8_t color = 0u;
    size_t base;
    uint16_t mask;

    if (out_color) *out_color = 0u;
    if (!out_color || !csb_v1_csbwin_planar_bitmap_valid(bitmap) ||
        x >= bitmap->width || y >= bitmap->height) return 0;
    base = (size_t)y * bitmap->byte_stride + (x / 16u) * 8u;
    mask = (uint16_t)(1u << (15u - (x & 15u)));
    for (plane = 0u; plane < 4u; ++plane) {
        if ((csb_v1_csbwin_read_be16(bitmap->bytes + base + plane * 2u) &
             mask) != 0u) color |= (uint8_t)(1u << plane);
    }
    *out_color = color;
    return 1;
}

int csb_v1_csbwin_planar_bitmap_blit_indexed(
    const CSB_V1_CSBWinPlanarBitmap *source,
    int source_x, int source_y, int width, int height,
    uint8_t *destination, int destination_width, int destination_height,
    int destination_stride, int destination_x, int destination_y,
    int transparent_color)
{
    int x, y, copied = 0;

    if (!csb_v1_csbwin_planar_bitmap_valid(source) || !destination ||
        source_x < 0 || source_y < 0 || width <= 0 || height <= 0 ||
        destination_width <= 0 || destination_height <= 0 ||
        destination_stride < destination_width || transparent_color < -1 ||
        transparent_color > 15 || source_x > (int)source->width - width ||
        source_y > (int)source->height - height) return 0;
    for (y = 0; y < height; ++y) {
        int dst_y = destination_y + y;
        if (dst_y < 0 || dst_y >= destination_height) continue;
        for (x = 0; x < width; ++x) {
            int dst_x = destination_x + x;
            uint8_t color;
            if (dst_x < 0 || dst_x >= destination_width ||
                !csb_v1_csbwin_planar_bitmap_pixel_at(
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

int csb_v1_csbwin_planar_bitmap_blit_wall_projection(
    const CSB_V1_CSBWinPlanarBitmap *source,
    const CSB_V1_CSBWinViewportProjectionRectangle *projection,
    int mirrored, uint8_t *destination, int destination_width,
    int destination_height, int destination_stride)
{
    int width;
    int height;
    int source_width;
    int x;
    int y;
    int copied = 0;

    if (!csb_v1_csbwin_planar_bitmap_valid(source) || !projection ||
        !csb_v1_csbwin_viewport_projection_rectangle_is_valid(projection) ||
        !destination || destination_width <= 0 || destination_height <= 0 ||
        destination_stride < destination_width ||
        projection->source_stride == 0u || projection->source_height == 0u ||
        (mirrored != 0 && mirrored != 1)) return 0;
    width = (int)projection->x2 - (int)projection->x1 + 1;
    height = (int)projection->y2 - (int)projection->y1 + 1;
    source_width = (int)projection->source_stride * 2;
    if (width <= 0 || height <= 0 ||
        (int)projection->source_x + width > source_width ||
        (int)projection->source_y + height > (int)projection->source_height ||
        source_width > (int)source->width ||
        projection->source_height > source->height) return 0;
    for (y = 0; y < height; ++y) {
        const int destination_y = (int)projection->y1 + y;
        if (destination_y < 0 || destination_y >= destination_height) continue;
        for (x = 0; x < width; ++x) {
            const int destination_x = (int)projection->x1 + x;
            const int source_x = (int)projection->source_x +
                (mirrored ? width - 1 - x : x);
            uint8_t color;

            if (destination_x < 0 || destination_x >= destination_width ||
                !csb_v1_csbwin_planar_bitmap_pixel_at(
                    source, (uint16_t)source_x,
                    (uint16_t)((int)projection->source_y + y), &color)) {
                continue;
            }
            if (color == 10u) continue;
            destination[(size_t)destination_y * destination_stride +
                        destination_x] = color;
            copied = 1;
        }
    }
    return copied;
}

static int csb_v1_csbwin_planar_bitmap_plane0_at(
    const CSB_V1_CSBWinPlanarBitmap *bitmap, int x, int y, int *out_bit)
{
    size_t base;
    uint16_t mask;

    if (out_bit) *out_bit = 0;
    if (!out_bit || !csb_v1_csbwin_planar_bitmap_valid(bitmap) || x < 0 ||
        y < 0 || x >= (int)bitmap->width || y >= (int)bitmap->height) {
        return 0;
    }
    base = (size_t)y * bitmap->byte_stride + (size_t)(x / 16) * 8u;
    mask = (uint16_t)(1u << (15u - ((unsigned int)x & 15u)));
    *out_bit = (csb_v1_csbwin_read_be16(bitmap->bytes + base) & mask) != 0u;
    return 1;
}

int csb_v1_csbwin_planar_bitmap_blit_teleporter(
    const CSB_V1_CSBWinPlanarBitmap *field,
    const CSB_V1_CSBWinPlanarBitmap *shape_mask,
    const uint8_t recipe[8],
    const CSB_V1_CSBWinViewportProjectionRectangle *projection,
    uint8_t random_period_bit, uint8_t random_start_row,
    uint8_t *destination, int destination_width, int destination_height,
    int destination_stride)
{
    const int has_mask = recipe && recipe[3] != 0xffu;
    const int field_period = recipe ? (int)recipe[1] +
        (int)(random_period_bit & 1u) : 0;
    const int transparent = recipe ? (int)(recipe[2] & 0x7fu) : -1;
    const int width = projection ? (int)projection->x2 - (int)projection->x1 + 1 : 0;
    const int height = projection ? (int)projection->y2 - (int)projection->y1 + 1 : 0;
    const int word_count = (width + 15) / 16;
    int x;
    int y;
    int copied = 0;

    /* DrawTeleporter only has two legal forms in the original layout:
     * y2=FF means no mask; otherwise the copied mask is exactly b4*b5 and
     * TAG008c98 reads its plane-0 word once per destination word. */
    if (!csb_v1_csbwin_planar_bitmap_valid(field) || !recipe || !projection ||
        !csb_v1_csbwin_viewport_projection_rectangle_is_valid(projection) ||
        !destination || destination_width <= 0 || destination_height <= 0 ||
        destination_stride < destination_width || width <= 0 || height <= 0 ||
        field->width != 16u || field_period <= 0 ||
        field_period > (int)field->height ||
        random_start_row >= (uint8_t)field_period || transparent < 0 ||
        transparent > 15 || (has_mask &&
            (!csb_v1_csbwin_planar_bitmap_valid(shape_mask) ||
             shape_mask->byte_stride != recipe[4] ||
             shape_mask->height != recipe[5] ||
             (int)shape_mask->width != (int)recipe[4] * 2 ||
             shape_mask->height != (uint16_t)height ||
             shape_mask->width != (uint16_t)(word_count * 16)))) {
        return 0;
    }
    for (y = 0; y < height; ++y) {
        const int dst_y = (int)projection->y1 + y;
        if (dst_y < 0 || dst_y >= destination_height) continue;
        for (x = 0; x < width; ++x) {
            const int dst_x = (int)projection->x1 + x;
            const int word_index = y * word_count + x / 16;
            const int field_y = ((int)random_start_row + word_index) % field_period;
            int selected = 1;
            uint8_t color;

            if (dst_x < 0 || dst_x >= destination_width ||
                !csb_v1_csbwin_planar_bitmap_pixel_at(
                    field, (uint16_t)(x & 15), (uint16_t)field_y, &color)) {
                continue;
            }
            if (has_mask && !csb_v1_csbwin_planar_bitmap_plane0_at(
                    shape_mask, (recipe[3] & 0x80u) != 0u
                        ? word_count * 16 - 1 - x : x,
                    y, &selected)) {
                return 0;
            }
            /* TAG008c98 makes unselected mask pixels transparent before
             * TAG0088b2's per-colour transparency dispatch. */
            if (!selected || color == (uint8_t)transparent) continue;
            destination[(size_t)dst_y * destination_stride + dst_x] = color;
            copied = 1;
        }
    }
    return copied;
}
