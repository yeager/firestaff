#include "nexus_v1_face_bin.h"
#include "nexus_v1_prs3_decode.h"
#include <string.h>

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint32_t bgr555_to_rgba(uint16_t c) {
    int r = ((c >> 10) & 0x1F) << 3;
    int g = ((c >> 5) & 0x1F) << 3;
    int b = (c & 0x1F) << 3;
    return 0xFF000000U | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static uint32_t fnv1a32(const uint8_t *data, int len) {
    uint32_t h = 0x811C9DC5U;
    int i;
    for (i = 0; i < len; ++i)
        h = (h ^ data[i]) * 0x01000193U;
    return h;
}

int nexus_v1_face_bin_parse_header(const uint8_t *data, int data_size,
                                   Nexus_V1_FaceBinHeader *out) {
    int i;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 16) return 0;
    if (memcmp(data, "FACE", 4) != 0) return 0;
    out->file_size = read_be32(data + 4);
    out->portrait_count = read_be16(data + 8);
    if (out->portrait_count > NEXUS_FACE_BIN_PORTRAIT_COUNT) return 0;
    if (data_size < 16 + out->portrait_count * 2) return 0;
    for (i = 0; i < out->portrait_count; ++i)
        out->offsets[i] = read_be16(data + 16 + i * 2);
    out->valid = 1;
    return 1;
}

int nexus_v1_face_bin_decode_portrait(const uint8_t *data, int data_size,
                                      const Nexus_V1_FaceBinHeader *header,
                                      int portrait_index,
                                      Nexus_V1_FaceBinPortrait *out) {
    const uint8_t *pal_data;
    const uint8_t *prs3_data;
    int prs3_offset, prs3_size;
    int next_offset;
    Nexus_V1_Prs3Header prs3_hdr;
    uint8_t tile_buf[NEXUS_FACE_BIN_TILE_COUNT * NEXUS_FACE_BIN_TILE_SIZE *
                     NEXUS_FACE_BIN_TILE_SIZE];
    Nexus_V1_Prs3DecodeResult dec;
    int i, tx, ty, px, py;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || !header || !header->valid) return 0;
    if (portrait_index < 0 || portrait_index >= header->portrait_count) return 0;

    out->index = portrait_index;

    pal_data = data + header->offsets[portrait_index];
    if (header->offsets[portrait_index] + 128 > data_size) return 0;

    for (i = 0; i < NEXUS_FACE_BIN_PALETTE_COLORS; ++i)
        out->palette_rgba[i] = bgr555_to_rgba(read_be16(pal_data + i * 2));

    prs3_offset = header->offsets[portrait_index] + 128;
    if (portrait_index + 1 < header->portrait_count)
        next_offset = header->offsets[portrait_index + 1];
    else
        next_offset = data_size;
    prs3_size = next_offset - prs3_offset;
    if (prs3_offset + prs3_size > data_size || prs3_size < 16) return 0;

    prs3_data = data + prs3_offset;
    if (!nexus_v1_prs3_parse_header(prs3_data, prs3_size, &prs3_hdr))
        return 0;

    if ((int)prs3_hdr.uncompressed_size != (int)sizeof(tile_buf)) return 0;

    dec = nexus_v1_prs3_decompress(prs3_hdr.stream,
                                    (int)prs3_hdr.compressed_size,
                                    tile_buf, (int)sizeof(tile_buf),
                                    (int)prs3_hdr.uncompressed_size);
    if (!dec.success) return 0;

    for (ty = 0; ty < NEXUS_FACE_BIN_TILES_PER_ROW; ++ty) {
        for (tx = 0; tx < NEXUS_FACE_BIN_TILES_PER_ROW; ++tx) {
            int tile_idx = ty * NEXUS_FACE_BIN_TILES_PER_ROW + tx;
            const uint8_t *tile = tile_buf +
                tile_idx * NEXUS_FACE_BIN_TILE_SIZE * NEXUS_FACE_BIN_TILE_SIZE;
            for (py = 0; py < NEXUS_FACE_BIN_TILE_SIZE; ++py) {
                for (px = 0; px < NEXUS_FACE_BIN_TILE_SIZE; ++px) {
                    int dst_x = tx * NEXUS_FACE_BIN_TILE_SIZE + px;
                    int dst_y = ty * NEXUS_FACE_BIN_TILE_SIZE + py;
                    out->pixels[dst_y * NEXUS_FACE_BIN_PORTRAIT_WIDTH +
                                dst_x] =
                        tile[py * NEXUS_FACE_BIN_TILE_SIZE + px];
                }
            }
        }
    }

    out->pixel_hash = fnv1a32(out->pixels, NEXUS_FACE_BIN_PORTRAIT_WIDTH *
                              NEXUS_FACE_BIN_PORTRAIT_HEIGHT);
    out->valid = 1;
    return 1;
}

int nexus_v1_face_bin_decode_all(const uint8_t *data, int data_size,
                                 Nexus_V1_FaceBinDecodeResult *out) {
    Nexus_V1_FaceBinHeader header;
    int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!nexus_v1_face_bin_parse_header(data, data_size, &header)) return 0;

    out->portrait_count = header.portrait_count;
    for (i = 0; i < header.portrait_count; ++i) {
        if (nexus_v1_face_bin_decode_portrait(data, data_size, &header, i,
                                              &out->portraits[i]))
            ++out->decoded_count;
        else
            ++out->failed_count;
    }
    out->valid = (out->decoded_count > 0) ? 1 : 0;
    return out->valid;
}
