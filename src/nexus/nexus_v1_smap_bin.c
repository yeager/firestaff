#include "nexus_v1_smap_bin.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t bgr555_to_rgba(uint16_t c) {
    /* Saturn Color RAM BGR555 stores R in bits 14..10, G in 9..5 and
     * B in 4..0; keep this aligned with nexus_v1_palette.c. */
    int r = ((c >> 10) & 0x1F) << 3;
    int g = ((c >> 5) & 0x1F) << 3;
    int b = (c & 0x1F) << 3;
    return 0xFF000000U | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

int nexus_v1_smap_parse_header(const uint8_t *data, int data_size,
                                Nexus_V1_SmapHeader *out) {
    uint64_t tilemap_end;
    uint64_t palette_end;
    uint64_t tileset_end;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 0x20) return 0;
    if (memcmp(data, "LVMP", 4) != 0) return 0;
    out->file_size = read_be32(data + 4);
    out->tilemap_offset = read_be32(data + 8);
    out->tilemap_size = read_be32(data + 12);
    out->palette_offset = read_be32(data + 16);
    out->palette_size = read_be32(data + 20);
    out->tileset_offset = read_be32(data + 24);
    out->tileset_size = read_be32(data + 28);
    /* DMWeb's LVMP corpus uses one fixed 80x76, 16-bit tilemap and a
     * 256-entry BGR555 palette. Reject a plausible-looking header that would
     * make decode walk into an adjacent section as if it were map data. */
    if (out->file_size != (uint32_t)data_size ||
        out->tilemap_size != (uint32_t)(NEXUS_SMAP_TILE_WIDTH *
                                        NEXUS_SMAP_TILE_HEIGHT * 2) ||
        out->palette_size != (uint32_t)(NEXUS_SMAP_PALETTE_COLORS * 2) ||
        out->tileset_size == 0U ||
        out->tileset_size % (NEXUS_SMAP_TILE_SIZE * NEXUS_SMAP_TILE_SIZE) != 0U) {
        return 0;
    }
    out->tile_count = (int)(out->tileset_size / (NEXUS_SMAP_TILE_SIZE *
                            NEXUS_SMAP_TILE_SIZE));
    tilemap_end = (uint64_t)out->tilemap_offset + out->tilemap_size;
    palette_end = (uint64_t)out->palette_offset + out->palette_size;
    tileset_end = (uint64_t)out->tileset_offset + out->tileset_size;
    if (tilemap_end > (uint64_t)data_size ||
        palette_end > (uint64_t)data_size ||
        tileset_end > (uint64_t)data_size ||
        out->tilemap_offset < 0x20U ||
        out->palette_offset < tilemap_end ||
        out->tileset_offset < palette_end) return 0;

    /* DMWeb Dungeon Master Nexus/file-formats: tilemap bit 0 is unused and
     * always zero; palette bit 15 is always one.  Rejecting violations here
     * keeps a padded or synthetic LVMP-looking blob from reaching the map
     * decoder as if it were an authentic Saturn automap. */
    {
        uint32_t index;
        for (index = 0U; index < NEXUS_SMAP_TILE_WIDTH *
                                  NEXUS_SMAP_TILE_HEIGHT; ++index) {
            uint16_t tile_word = read_be16(data + out->tilemap_offset +
                                           index * 2U);
            if ((tile_word & 1U) != 0U ||
                ((tile_word >> 1) & 0x1FFFU) >= (uint32_t)out->tile_count) {
                return 0;
            }
        }
        for (index = 0U; index < NEXUS_SMAP_PALETTE_COLORS; ++index) {
            if ((read_be16(data + out->palette_offset + index * 2U) &
                 0x8000U) == 0U) {
                return 0;
            }
        }
    }
    out->valid = 1;
    return 1;
}

int nexus_v1_smap_decode(const uint8_t *data, int data_size,
                          uint32_t *rgba_out, int rgba_capacity,
                          Nexus_V1_SmapDecodeResult *out) {
    Nexus_V1_SmapHeader hdr;
    uint32_t palette[NEXUS_SMAP_PALETTE_COLORS];
    int total_pixels;
    int tx, ty, px, py, i;
    uint32_t fnv;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!nexus_v1_smap_parse_header(data, data_size, &hdr)) return 0;

    total_pixels = NEXUS_SMAP_PIXEL_WIDTH * NEXUS_SMAP_PIXEL_HEIGHT;
    if (!rgba_out || rgba_capacity < total_pixels) return 0;

    for (i = 0; i < NEXUS_SMAP_PALETTE_COLORS; ++i) {
        uint16_t c = read_be16(data + hdr.palette_offset + i * 2);
        palette[i] = bgr555_to_rgba(c);
    }
    memcpy(out->palette_rgba, palette, sizeof(palette));

    memset(rgba_out, 0, (size_t)total_pixels * 4);

    for (ty = 0; ty < NEXUS_SMAP_TILE_HEIGHT; ++ty) {
        for (tx = 0; tx < NEXUS_SMAP_TILE_WIDTH; ++tx) {
            int map_idx = ty * NEXUS_SMAP_TILE_WIDTH + tx;
            uint16_t tile_word = read_be16(data + hdr.tilemap_offset +
                                           map_idx * 2);
            int h_flip = (tile_word >> 15) & 1;
            int v_flip = (tile_word >> 14) & 1;
            int tile_idx = (tile_word >> 1) & 0x1FFF;
            const uint8_t *tile_data;

            if (tile_idx >= hdr.tile_count) continue;
            tile_data = data + hdr.tileset_offset +
                        tile_idx * NEXUS_SMAP_TILE_SIZE *
                        NEXUS_SMAP_TILE_SIZE;

            for (py = 0; py < NEXUS_SMAP_TILE_SIZE; ++py) {
                for (px = 0; px < NEXUS_SMAP_TILE_SIZE; ++px) {
                    int src_x = h_flip ? (7 - px) : px;
                    int src_y = v_flip ? (7 - py) : py;
                    uint8_t pal_idx = tile_data[src_y *
                        NEXUS_SMAP_TILE_SIZE + src_x];
                    int dst_x = tx * NEXUS_SMAP_TILE_SIZE + px;
                    int dst_y = ty * NEXUS_SMAP_TILE_SIZE + py;
                    rgba_out[dst_y * NEXUS_SMAP_PIXEL_WIDTH + dst_x] =
                        palette[pal_idx];
                }
            }
        }
    }

    fnv = 0x811C9DC5U;
    for (i = 0; i < total_pixels; ++i) {
        uint32_t v = rgba_out[i];
        fnv = (fnv ^ (v & 0xFF)) * 0x01000193U;
        fnv = (fnv ^ ((v >> 8) & 0xFF)) * 0x01000193U;
        fnv = (fnv ^ ((v >> 16) & 0xFF)) * 0x01000193U;
        fnv = (fnv ^ ((v >> 24) & 0xFF)) * 0x01000193U;
    }

    out->pixel_hash = fnv;
    out->rendered_width = NEXUS_SMAP_PIXEL_WIDTH;
    out->rendered_height = NEXUS_SMAP_PIXEL_HEIGHT;
    out->valid = 1;
    return 1;
}
