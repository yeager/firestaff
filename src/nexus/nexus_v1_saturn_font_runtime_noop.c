/* Nexus V1 FONT256 source-retention boundary.
 *
 * The production link retains the authenticated Character Generator bytes so
 * a later Saturn page/attribute consumer can use the same source object. It
 * deliberately does not expose glyph-code mapping, text placement, or
 * framebuffer writes; the caller keeps font_loaded closed until those facts
 * are captured.
 */

#include "nexus_v1_saturn_font.h"
#include "nexus_v1_font_s2d.h"
#include <stdlib.h>
#include <string.h>

int nexus_v1_font_load_from_s2d(Nexus_V1_Font *font,
                                const uint8_t *data, int data_size,
                                const Nexus_V1_FontS2dDecodeResult *decoded) {
    int tile_count;
    int i;

    if (!font || !data || data_size <= 0 || !decoded || !decoded->valid) {
        return -1;
    }
    if (decoded->character_generator_size < 16U ||
        (decoded->character_generator_size - 16U) % 64U != 0U) {
        return -1;
    }
    tile_count = (int)((decoded->character_generator_size - 16U) / 64U);
    if (tile_count <= 0 || tile_count > NEXUS_V1_FONT_S2D_REAL_TILE_COUNT) {
        return -1;
    }
    memset(font, 0, sizeof(*font));
    font->char_count = tile_count;
    font->char_width = 8;
    font->char_height = 8;
    font->bytes_per_pixel = 1;
    font->bitmap_size = tile_count * 64;
    font->bitmap_data = (uint8_t *)malloc((size_t)font->bitmap_size);
    if (!font->bitmap_data) {
        memset(font, 0, sizeof(*font));
        return -1;
    }
    for (i = 0; i < tile_count; ++i) {
        if (nexus_v1_font_s2d_copy_character_generator_tile(
                data, data_size, decoded, i,
                font->bitmap_data + i * 64) != 0) {
            free(font->bitmap_data);
            memset(font, 0, sizeof(*font));
            return -1;
        }
    }
    return tile_count;
}

void nexus_v1_font_free(Nexus_V1_Font *font) {
    if (!font) return;
    free(font->bitmap_data);
    memset(font, 0, sizeof(*font));
}
