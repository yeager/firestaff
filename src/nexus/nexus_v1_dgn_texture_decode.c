#include "nexus_v1_dgn_texture_decode.h"

#include <string.h>

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static uint32_t rd32(const uint8_t *p) { return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }

int nexus_v1_dgn_texture_decode(const uint8_t *dgn, int dgn_size,
                                int image_id, uint8_t *pixels,
                                int pixel_capacity, uint16_t *palette,
                                int palette_capacity,
                                Nexus_V1_DgnTextureDecodeReceipt *out)
{
    uint32_t block, blocks, useful, descriptor_count, cursor, descriptor_bytes;
    uint16_t encoding, width, height, palette_id;
    uint32_t image_rel, palette_rel, image_bytes, image_abs, palette_abs;
    int x, y;
    if (!out) return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_INPUT;
    memset(out, 0, sizeof(*out));
    out->image_id = image_id;
    if (!dgn || dgn_size < NEXUS_DGN_BLOCK_SIZE || !pixels || image_id < 0 ||
        pixel_capacity <= 0) return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_INPUT;
    block = rd16(dgn + 0x14); blocks = rd16(dgn + 0x16); useful = rd32(dgn + 0x18);
    if (!block || !blocks || useful < 20U || block * NEXUS_DGN_BLOCK_SIZE > (uint32_t)dgn_size ||
        blocks * NEXUS_DGN_BLOCK_SIZE > (uint32_t)dgn_size - block * NEXUS_DGN_BLOCK_SIZE ||
        useful > blocks * NEXUS_DGN_BLOCK_SIZE) return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_BOUNDS;
    descriptor_count = 0U; cursor = 0U;
    while (cursor + 20U <= useful && rd16(dgn + block * NEXUS_DGN_BLOCK_SIZE + cursor) != 0xffffU) {
        if (rd16(dgn + block * NEXUS_DGN_BLOCK_SIZE + cursor) != descriptor_count)
            return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_DESCRIPTOR;
        ++descriptor_count; cursor += 20U;
    }
    descriptor_bytes = cursor;
    if (cursor + 2U > useful || (uint32_t)image_id >= descriptor_count)
        return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_DESCRIPTOR;
    cursor = block * NEXUS_DGN_BLOCK_SIZE + (uint32_t)image_id * 20U;
    encoding = rd16(dgn + cursor + 2); palette_id = rd16(dgn + cursor + 4);
    width = rd16(dgn + cursor + 6); height = rd16(dgn + cursor + 8);
    image_rel = rd32(dgn + cursor + 12); palette_rel = rd32(dgn + cursor + 16);
    if (!width || !height || (encoding != 0x0008U && encoding != 0x0028U))
        return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_DESCRIPTOR;
    image_bytes = encoding == 0x0008U ? ((uint32_t)width * height + 1U) / 2U : (uint32_t)width * height * 2U;
    image_abs = block * NEXUS_DGN_BLOCK_SIZE + image_rel;
    if (image_rel < descriptor_bytes + 2U || image_rel > useful || image_bytes > useful - image_rel || image_abs > (uint32_t)dgn_size || image_bytes > (uint32_t)dgn_size - image_abs)
        return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_BOUNDS;
    if ((encoding == 0x0008U ? (uint32_t)width * height :
         (uint32_t)width * height * 2U) > (uint32_t)pixel_capacity)
        return NEXUS_V1_DGN_TEXTURE_DECODE_OUTPUT_TOO_SMALL;
    if (encoding == 0x0008U) {
        if (!palette || palette_capacity < 16 || !palette_rel || useful < 32U ||
            palette_rel > useful - 32U)
            return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_BOUNDS;
        palette_abs = block * NEXUS_DGN_BLOCK_SIZE + palette_rel;
        if (palette_abs > (uint32_t)dgn_size || 32U > (uint32_t)dgn_size - palette_abs)
            return NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_BOUNDS;
        for (x = 0; x < 16; ++x) palette[x] = rd16(dgn + palette_abs + (uint32_t)x * 2U);
        for (y = 0; y < height; ++y) for (x = 0; x < width; ++x) {
            uint8_t v = dgn[image_abs + (uint32_t)y * ((width + 1U) / 2U) + (uint32_t)x / 2U];
            pixels[(size_t)y * width + (size_t)x] =
                (uint8_t)((x & 1) ? (v & 0x0fU) : (v >> 4));
        }
        out->indexed4 = out->palette_decoded = 1; out->palette_entries = 16;
    } else {
        for (y = 0; y < height; ++y) for (x = 0; x < width; ++x) {
            uint32_t o = ((uint32_t)y * width + (uint32_t)x) * 2U;
            pixels[o] = dgn[image_abs + o]; pixels[o + 1U] = dgn[image_abs + o + 1U];
        }
        out->direct_color_555 = 1;
    }
    out->encoding = encoding; out->width = width; out->height = height;
    out->image_offset = image_rel; out->image_bytes = image_bytes; out->palette_offset = palette_rel;
    out->source_verified = 1; out->decoded = 1;
    (void)palette_id;
    return NEXUS_V1_DGN_TEXTURE_DECODE_OK;
}

const char *nexus_v1_dgn_texture_decode_status_name(Nexus_V1_DgnTextureDecodeStatus s)
{
    switch (s) { case NEXUS_V1_DGN_TEXTURE_DECODE_OK: return "ok"; case NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_INPUT: return "blocked-input"; case NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_DESCRIPTOR: return "blocked-descriptor"; case NEXUS_V1_DGN_TEXTURE_DECODE_BLOCKED_BOUNDS: return "blocked-bounds"; case NEXUS_V1_DGN_TEXTURE_DECODE_OUTPUT_TOO_SMALL: return "output-too-small"; }
    return "blocked-input";
}
