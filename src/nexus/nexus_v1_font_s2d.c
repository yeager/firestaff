#include "nexus_v1_font_s2d.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static const uint8_t scr_magic[16] = {
    'S','E','G','A',' ','S','A','T','U','R','N',' ','S','C','R','\0'
};

int nexus_v1_font_s2d_copy_character_generator_tile(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int tile_index, uint8_t out_tile[64])
{
    uint32_t offset;

    if (!data || data_size <= 0 || !decoded || !out_tile ||
        !decoded->valid || tile_index < 0 || tile_index >= 242 ||
        decoded->character_generator_size < 16U + 242U * 64U) {
        return -1;
    }
    offset = decoded->character_generator_offset + 16U +
        (uint32_t)tile_index * 64U;
    if (offset > (uint32_t)data_size ||
        64U > (uint32_t)data_size - offset ||
        offset < decoded->character_generator_offset ||
        offset + 64U > decoded->character_generator_offset +
            decoded->character_generator_size) {
        return -1;
    }
    memcpy(out_tile, data + offset, 64U);
    return 0;
}

static int copy_region_be16(const uint8_t *data, int data_size,
                            uint32_t region_offset, uint32_t region_size,
                            int prefix_size, int index, int count,
                            uint16_t *out_word)
{
    uint32_t offset;
    if (!data || data_size <= 0 || !out_word || index < 0 || index >= count ||
        region_size < (uint32_t)prefix_size + (uint32_t)count * 2U) {
        return -1;
    }
    offset = region_offset + (uint32_t)prefix_size + (uint32_t)index * 2U;
    if (offset > (uint32_t)data_size || 2U > (uint32_t)data_size - offset ||
        offset < region_offset || offset + 2U > region_offset + region_size) {
        return -1;
    }
    *out_word = (uint16_t)(((uint16_t)data[offset] << 8) | data[offset + 1U]);
    return 0;
}

int nexus_v1_font_s2d_page_tilemap_word(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int tilemap_index, uint16_t *out_word)
{
    if (!decoded) return -1;
    return copy_region_be16(data, data_size, decoded->page_offset,
                            decoded->page_size, 16, tilemap_index, 4096,
                            out_word);
}

int nexus_v1_font_s2d_palette_word(
    const uint8_t *data, int data_size,
    const Nexus_V1_FontS2dDecodeResult *decoded,
    int palette_index, uint16_t *out_word)
{
    if (!decoded) return -1;
    return copy_region_be16(data, data_size, decoded->palette_offset,
                            decoded->palette_size, 16, palette_index, 256,
                            out_word);
}

int nexus_v1_font_s2d_decode(const uint8_t *data, int data_size,
                              Nexus_V1_FontS2dDecodeResult *out) {
    int i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < NEXUS_SCR_HEADER_SIZE + NEXUS_SCR_DESC_SIZE) return 0;
    if (memcmp(data, scr_magic, NEXUS_SCR_HEADER_SIZE) != 0) return 0;

    for (i = 0; i < NEXUS_SCR_MAX_SECTIONS; i++) {
        int desc_off = NEXUS_SCR_HEADER_SIZE + i * NEXUS_SCR_DESC_SIZE;
        uint32_t sec_offset, sec_size;

        if (desc_off + NEXUS_SCR_DESC_SIZE > data_size) break;
        sec_offset = read_be32(data + desc_off);
        sec_size = read_be32(data + desc_off + 4);
        if (sec_offset == 0 && sec_size == 0) break;
        if ((int)(sec_offset + sec_size) > data_size) break;

        out->sections[i].offset = sec_offset;
        out->sections[i].size = sec_size;
        out->section_count++;
    }

    if (out->section_count < 3) return 0;

    /* DMWeb DecodeFONT256S2D names the first five 32-bit offset/size
     * pairs Map, Page, Character Generator, Palette and Attributes.  Keep
     * those source windows explicit so later tilemap/CG work cannot mistake
     * a raw section ordinal for a glyph table. */
    if (out->section_count >= 5) {
        out->map_offset = out->sections[0].offset;
        out->map_size = out->sections[0].size;
        out->page_offset = out->sections[1].offset;
        out->page_size = out->sections[1].size;
        out->character_generator_offset = out->sections[2].offset;
        out->character_generator_size = out->sections[2].size;
        out->palette_offset = out->sections[3].offset;
        out->palette_size = out->sections[3].size;
        out->attribute_offset = out->sections[4].offset;
        out->attribute_size = out->sections[4].size;
    }

    /* Section 1: tilemap — 16-byte header + W*H*2 tile references */
    if (out->sections[1].size >= 16) {
        int map_off = (int)out->sections[1].offset;
        uint32_t map_data_size = out->sections[1].size - 16;

        out->tilemap_width = (int)read_be16(data + map_off);
        out->tilemap_height = (int)(map_data_size / (2 * (uint32_t)out->tilemap_width));

        /* Count unique tile indices */
        {
            int n_entries = (int)(map_data_size / 2);
            uint16_t max_idx = 0;
            int j;
            for (j = 0; j < n_entries; j++) {
                uint16_t idx = read_be16(data + map_off + 16 + j * 2);
                if (idx > max_idx) max_idx = idx;
            }
            out->tile_count = (max_idx / 2) + 1;
        }
    }

    /* Section 3: palette — 16-byte header + N*2 BGR555 colors */
    if (out->section_count >= 4 && out->sections[3].size >= 16) {
        uint32_t pal_data = out->sections[3].size - 16;
        out->palette_color_count = (int)(pal_data / 2);
    }

    fnv = 0x811C9DC5U;
    for (i = 0; i < data_size; i++) {
        fnv = (fnv ^ data[i]) * 0x01000193U;
    }
    out->data_hash = fnv;

    out->valid = 1;
    return 1;
}
