#ifndef NEXUS_V1_FONT_S2D_H
#define NEXUS_V1_FONT_S2D_H

#include <stdint.h>

#define NEXUS_SCR_MAX_SECTIONS  16
#define NEXUS_SCR_HEADER_SIZE   16
#define NEXUS_SCR_DESC_SIZE     16
/* The authenticated European FONT256.S2D CG region contains 242 actual
 * 8x8/8bpp tiles after its 16-byte region prefix.  The SCR header still
 * declares 256 character codes; the missing 14 code-to-tile mappings remain
 * unproven and are never filled with synthetic tiles. */
#define NEXUS_V1_FONT_S2D_REAL_TILE_COUNT 242

typedef struct {
    uint32_t offset;
    uint32_t size;
} Nexus_V1_ScrSection;

typedef struct {
    int valid;
    int section_count;
    Nexus_V1_ScrSection sections[NEXUS_SCR_MAX_SECTIONS];
    /* DMWeb DecodeFONT256S2D header pairs, retained as named source
     * regions.  These are byte windows only; no tile/glyph semantics are
     * inferred until the page/tilemap consumer is bound. */
    uint32_t map_offset;
    uint32_t map_size;
    uint32_t page_offset;
    uint32_t page_size;
    uint32_t character_generator_offset;
    uint32_t character_generator_size;
    uint32_t palette_offset;
    uint32_t palette_size;
    uint32_t attribute_offset;
    uint32_t attribute_size;
    /* DMWeb DecodeFONT256S2D header facts; these are VDP2/SCR metadata only,
     * not a character-code mapping. */
    uint16_t map_horizontal_page;
    uint16_t map_vertical_page;
    uint16_t map_page_number;
    uint32_t page_character_control_data;
    uint16_t page_pattern_name_auxiliary_data;
    int tilemap_width;
    int tilemap_height;
    int tile_count;
    int palette_color_count;
    uint32_t data_hash;
} Nexus_V1_FontS2dDecodeResult;

/* DMWeb DecodeFONT256S2D: the Character Generator region contains a
 * 16-byte prefix followed by NEXUS_V1_FONT_S2D_REAL_TILE_COUNT raw 8x8,
 * 8-bit tile images. This copies one
 * source tile only; it does not infer a character/glyph mapping. */
int nexus_v1_font_s2d_copy_character_generator_tile(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int tile_index, uint8_t out_tile[64]);

/* DMWeb page data: 16-byte prefix followed by 4096 big-endian tilemap
 * words. Palette data: 16-byte prefix followed by 256 big-endian BGR555
 * words. These helpers expose source words only; they do not assign glyph
 * or menu meaning. */
int nexus_v1_font_s2d_page_tilemap_word(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int tilemap_index, uint16_t *out_word);
int nexus_v1_font_s2d_palette_word(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int palette_index, uint16_t *out_word);
int nexus_v1_font_s2d_attribute_word(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int tile_index, uint16_t *out_word);

int nexus_v1_font_s2d_decode(const uint8_t *data, int data_size,
                              Nexus_V1_FontS2dDecodeResult *out);

#endif
