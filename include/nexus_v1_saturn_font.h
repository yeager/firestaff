
#ifndef NEXUS_V1_SATURN_FONT_H
#define NEXUS_V1_SATURN_FONT_H
#include <stdint.h>

/* SEGA SATURN SCR — Saturn Screen Resource font format.
 * FONT256.S2D contains 256 character glyphs for Japanese text.
 * Header: "SEGA SATURN SCR\0" (16 bytes) */

typedef struct {
    int char_count;
    int char_width;
    int char_height;
    uint8_t *bitmap_data;
    int bitmap_size;
} Nexus_V1_Font;

int nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size);
void nexus_v1_font_free(Nexus_V1_Font *font);
const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int char_index);

#endif

