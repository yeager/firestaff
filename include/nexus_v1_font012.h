#ifndef NEXUS_V1_FONT012_H
#define NEXUS_V1_FONT012_H

#include <stddef.h>
#include <stdint.h>

/* Nexus FONT012 2bpp glyph resource (RLOWFIX.BIN RES* archive).
 * Glyph packing proven (pass 216): 2bpp MSB-first, inverted (3-val).
 * Palette: FFFF/DEF7/B9CE/8000 (white/light-gray/dark-gray/black BGR555).
 * VDP2 text layer: CHCTLA at 0x25F00006, CRAM upload at 0x25F80000. */
typedef struct {
    int valid;
    uint32_t resource_index;
    uint32_t header_word12;
    uint32_t format_word;
    uint16_t character_count;
    uint16_t character_width;
    uint16_t character_height;
    uint32_t resource_size;
} Nexus_V1_Font012Receipt;

int nexus_v1_font012_parse(const uint8_t *data, size_t size,
                           uint32_t resource_index,
                           Nexus_V1_Font012Receipt *out);

/* DMWeb DMNDataFileDecoder.vbs FONT branch: decode one bounded 2bpp glyph
 * into palette indices. This does not assign a character code or draw it. */
int nexus_v1_font012_decode_glyph(const uint8_t *data, size_t size,
                                  uint32_t resource_index,
                                  uint16_t glyph_index,
                                  uint8_t *pixels, size_t pixel_capacity);

#endif
