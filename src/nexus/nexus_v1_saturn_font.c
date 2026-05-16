
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

