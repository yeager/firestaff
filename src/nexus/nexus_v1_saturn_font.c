
#include "nexus_v1_saturn_font.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}

int nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size) {
    if (!font || !data || size < 48) return -1;
    memset(font, 0, sizeof(*font));

    /* Verify SEGA SATURN SCR header */
    if (memcmp(data, "SEGA SATURN SCR", 15) != 0) return -1;

    /* Parse header fields (big-endian) */
    font->char_count = rb32(data + 16) & 0xFFFF;
    if (font->char_count <= 0 || font->char_count > 512)
        font->char_count = 256;

    /* Estimate char dimensions from data size */
    int glyph_data_size = size - 48;  /* skip header */
    int glyph_size = glyph_data_size / font->char_count;
    /* Common Saturn font: 12x12, 16x16, 8x16 */
    if (glyph_size >= 32) { font->char_width = 16; font->char_height = 16; }
    else if (glyph_size >= 18) { font->char_width = 12; font->char_height = 12; }
    else { font->char_width = 8; font->char_height = glyph_size; }

    font->bitmap_data = malloc(glyph_data_size);
    if (font->bitmap_data) {
        memcpy(font->bitmap_data, data + 48, glyph_data_size);
        font->bitmap_size = glyph_data_size;
    }

    printf("Saturn font: %d chars, %dx%d, %d bytes\n",
        font->char_count, font->char_width, font->char_height, glyph_data_size);
    return font->char_count;
}

void nexus_v1_font_free(Nexus_V1_Font *font) {
    if (font) { free(font->bitmap_data); font->bitmap_data = NULL; }
}

const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int idx) {
    int glyph_size;
    if (!font || !font->bitmap_data || idx < 0 || idx >= font->char_count)
        return NULL;
    glyph_size = font->bitmap_size / font->char_count;
    return font->bitmap_data + idx * glyph_size;
}

static int nexus_v1_font_glyph_layout(const Nexus_V1_Font *font,
                                      int *out_glyph_size,
                                      int *out_row_stride,
                                      int *out_required_bytes) {
    int glyph_size, row_stride, required_bytes;
    if (!font || !font->bitmap_data || font->char_count <= 0 ||
        font->char_width <= 0 || font->char_height <= 0 ||
        font->bitmap_size <= 0) {
        return 0;
    }

    glyph_size = font->bitmap_size / font->char_count;
    row_stride = (font->char_width + 7) / 8;
    required_bytes = row_stride * font->char_height;
    if (glyph_size < required_bytes) {
        return 0;
    }

    if (out_glyph_size) *out_glyph_size = glyph_size;
    if (out_row_stride) *out_row_stride = row_stride;
    if (out_required_bytes) *out_required_bytes = required_bytes;
    return 1;
}

int nexus_v1_font_get_glyph_pixel(const Nexus_V1_Font *font,
                                  int char_index,
                                  int x,
                                  int y) {
    int row_stride;
    const uint8_t *glyph;
    const uint8_t *row;

    if (!nexus_v1_font_glyph_layout(font, NULL, &row_stride, NULL) ||
        x < 0 || y < 0 || x >= font->char_width || y >= font->char_height) {
        return 0;
    }

    glyph = nexus_v1_font_get_glyph(font, char_index);
    if (!glyph) {
        return 0;
    }

    row = glyph + y * row_stride;
    return (row[x / 8] & (uint8_t)(0x80u >> (x & 7))) ? 1 : 0;
}

int nexus_v1_font_expand_glyph_bitmap(const Nexus_V1_Font *font,
                                      int char_index,
                                      uint8_t *out_bitmap,
                                      int out_width,
                                      int out_height,
                                      int out_stride) {
    int x, y;

    if (!font || !out_bitmap ||
        out_width < font->char_width ||
        out_height < font->char_height ||
        out_stride < out_width ||
        !nexus_v1_font_get_glyph(font, char_index) ||
        !nexus_v1_font_glyph_layout(font, NULL, NULL, NULL)) {
        return -1;
    }

    for (y = 0; y < font->char_height; ++y) {
        for (x = 0; x < font->char_width; ++x) {
            out_bitmap[y * out_stride + x] =
                (uint8_t)nexus_v1_font_get_glyph_pixel(font, char_index, x, y);
        }
    }

    return 1;
}

int nexus_v1_font_draw_glyph_indexed(const Nexus_V1_Font *font,
                                     uint8_t *framebuffer,
                                     int fb_width,
                                     int fb_height,
                                     int fb_stride,
                                     int dst_x,
                                     int dst_y,
                                     int char_index,
                                     uint8_t fg_index,
                                     int bg_index) {
    int x, y, writes;

    if (!font || !framebuffer ||
        fb_width <= 0 || fb_height <= 0 || fb_stride < fb_width ||
        bg_index > 255 ||
        !nexus_v1_font_get_glyph(font, char_index) ||
        !nexus_v1_font_glyph_layout(font, NULL, NULL, NULL)) {
        return -1;
    }

    writes = 0;
    for (y = 0; y < font->char_height; ++y) {
        int py = dst_y + y;
        if (py < 0 || py >= fb_height) {
            continue;
        }
        for (x = 0; x < font->char_width; ++x) {
            int px = dst_x + x;
            int pixel;
            if (px < 0 || px >= fb_width) {
                continue;
            }

            pixel = nexus_v1_font_get_glyph_pixel(font, char_index, x, y);
            if (pixel) {
                framebuffer[py * fb_stride + px] = fg_index;
                ++writes;
            } else if (bg_index >= 0) {
                framebuffer[py * fb_stride + px] = (uint8_t)bg_index;
                ++writes;
            }
        }
    }

    return writes;
}
