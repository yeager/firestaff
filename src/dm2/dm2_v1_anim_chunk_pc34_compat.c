#include "dm2_v1_anim_chunk_pc34_compat.h"
#include "dm2_v1_anim_bootstrap.h"

#include <string.h>

static uint16_t rd16be(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}

int dm2_v1_anim_chunk_read(const uint8_t *data,
                           size_t data_size,
                           uint32_t offset,
                           DM2_V1_AnimChunk *out_chunk)
{
    uint16_t tag;
    uint16_t payload_size;

    if (!data || !out_chunk || (size_t)offset + DM2_V1_ANIM_CHUNK_OVERHEAD > data_size)
        return 0;

    tag = rd16be(data + offset);
    payload_size = rd16be(data + offset + 2);

    if ((size_t)offset + 4 + payload_size + 2 > data_size)
        return 0;

    out_chunk->tag = tag;
    out_chunk->payload_size = payload_size;
    out_chunk->payload = data + offset + 4;
    out_chunk->trailer = rd16be(data + offset + 4 + payload_size);
    out_chunk->file_offset = offset;
    return 1;
}

int dm2_v1_anim_chunk_scan(const uint8_t *data,
                           size_t data_size,
                           DM2_V1_AnimChunkScanReceipt *out_receipt)
{
    DM2_V1_AnimChunkScanReceipt r;
    uint32_t pos = 0;
    DM2_V1_AnimChunk chunk;

    memset(&r, 0, sizeof(r));
    if (!data || data_size < DM2_V1_ANIM_CHUNK_OVERHEAD) {
        if (out_receipt) *out_receipt = r;
        return 0;
    }

    while (dm2_v1_anim_chunk_read(data, data_size, pos, &chunk)) {
        r.chunk_count++;
        switch (chunk.tag) {
        case DM2_V1_ANIM_CHUNK_AN: r.an_count++; break;
        case DM2_V1_ANIM_CHUNK_PL: r.pl_count++; break;
        case DM2_V1_ANIM_CHUNK_EN: r.en_count++; break;
        case DM2_V1_ANIM_CHUNK_DL: r.dl_count++; break;
        case DM2_V1_ANIM_CHUNK_SD: r.sd_count++; break;
        case DM2_V1_ANIM_CHUNK_BR: r.br_count++; break;
        case DM2_V1_ANIM_CHUNK_SO: r.so_count++; break;
        case DM2_V1_ANIM_CHUNK_DO: r.do_count++; break;
        case DM2_V1_ANIM_CHUNK_FO: r.fo_count++; break;
        case DM2_V1_ANIM_CHUNK_NE: r.ne_count++; break;
        case DM2_V1_ANIM_CHUNK_BN: r.bn_count++; break;
        default: r.unknown_count++; break;
        }
        pos += 4 + chunk.payload_size + 2;
    }

    r.bytes_consumed = pos;
    r.valid = (pos == (uint32_t)data_size) ? 1 : 0;
    if (out_receipt) *out_receipt = r;
    return r.valid;
}

int dm2_v1_anim_parse_an_header(const DM2_V1_AnimChunk *chunk,
                                DM2_V1_AnimAnHeader *out_header)
{
    if (!chunk || !out_header || chunk->tag != DM2_V1_ANIM_CHUNK_AN ||
        chunk->payload_size < 8 || !chunk->payload)
        return 0;

    out_header->width = rd16be(chunk->payload + 2);
    out_header->height = rd16be(chunk->payload + 4);
    out_header->flags = rd16be(chunk->payload + 6);
    out_header->extra = chunk->trailer;
    return 1;
}

int dm2_v1_anim_parse_palette(const DM2_V1_AnimChunk *chunk,
                              DM2_V1_AnimPalette *out_palette)
{
    uint16_t start;
    uint16_t num;
    uint16_t i;

    if (!chunk || !out_palette || chunk->tag != DM2_V1_ANIM_CHUNK_PL ||
        chunk->payload_size < 4 || !chunk->payload)
        return 0;

    memset(out_palette, 0, sizeof(*out_palette));
    start = rd16be(chunk->payload);
    num = (uint16_t)((chunk->payload_size - 4) / 4);
    if (num > DM2_V1_ANIM_MAX_PALETTE_COLORS || num == 0)
        return 0;

    out_palette->start_color = start;
    out_palette->num_colors = num;
    for (i = 0; i < num; i++) {
        const uint8_t *e = chunk->payload + 4 + i * 4;
        out_palette->entries[i][0] = e[0];
        out_palette->entries[i][1] = e[1];
        out_palette->entries[i][2] = e[2];
        out_palette->entries[i][3] = e[3];
    }
    return 1;
}

static int decode_rle_partial(const uint8_t *src, size_t src_size,
                              uint8_t *dst, size_t dst_size,
                              uint16_t width, uint16_t height)
{
    uint16_t even_width = (uint16_t)((width + 1u) & 0xfffeu);
    uint32_t total_pixels = (uint32_t)even_width * (uint32_t)height;
    size_t pos = 0;
    uint32_t di = 0;

    if (((total_pixels + 1u) >> 1) > dst_size)
        return 0;

    while (di < total_pixels && pos < src_size) {
        uint8_t op = src[pos++];
        uint16_t count;

        if ((op & 0x80u) == 0u) {
            count = (uint16_t)((op >> 4) + 1u);
            if (di + count > total_pixels) return 0;
            dm2_v1_anim_fill_seq_4bpp(dst, dst_size, (uint16_t)di, op, count);
            di += count;
            continue;
        }
        switch (op & 0x30u) {
        case 0x00u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 1;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 1;
                count = (uint16_t)(((uint16_t)src[pos] << 8) | src[pos+1u]);
                pos += 2u; count++;
            }
            if (di + count > total_pixels) return 0;
            dm2_v1_anim_fill_seq_4bpp(dst, dst_size, (uint16_t)di, op, count);
            di += count;
            break;
        case 0x10u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 1;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 1;
                count = (uint16_t)(((uint16_t)src[pos] << 8) | src[pos+1u]);
                pos += 2u; count++;
            }
            if (count & 1u) {
                if (di >= total_pixels) return 0;
                dm2_v1_anim_setpixel_seq_4bpp(dst, dst_size, (uint16_t)di, op);
                di++; count--;
            }
            if (di + count > total_pixels || pos + ((size_t)count >> 1) > src_size)
                return (di + count > total_pixels) ? 0 : 1;
            {
                uint16_t i;
                for (i = 0; i < count; i++) {
                    uint8_t bv = src[pos + (i >> 1)];
                    uint8_t c = (i & 1) ? (bv & 0x0f) : ((bv >> 4) & 0x0f);
                    dm2_v1_anim_setpixel_seq_4bpp(dst, dst_size,
                                                  (uint16_t)(di + i), c);
                }
            }
            pos += (size_t)count >> 1;
            di += count;
            break;
        case 0x20u:
            count = (uint16_t)(((op >> 2) & 16u) | (op & 15u));
            if (count == 0x1du) {
                if (pos >= src_size) return 1;
                count = (uint16_t)(src[pos++] + 1u);
            } else if (count == 0x1eu) {
                if (pos >= src_size) return 1;
                count = (uint16_t)(src[pos++] + 0x101u);
            } else if (count == 0x1fu) {
                if (pos + 1u >= src_size) return 1;
                count = (uint16_t)(((uint16_t)src[pos] << 8) | src[pos+1u]);
                pos += 2u; count++;
            } else {
                count++;
            }
            if (di + count > total_pixels) return 0;
            di += count;
            break;
        case 0x30u:
            if ((op & 0x40u) == 0u) {
                if (pos >= src_size) return 1;
                count = (uint16_t)(src[pos++] + 1u);
            } else {
                if (pos + 1u >= src_size) return 1;
                count = (uint16_t)(((uint16_t)src[pos] << 8) | src[pos+1u]);
                pos += 2u; count++;
            }
            if (di < even_width || di + count + 1u > total_pixels)
                return 0;
            {
                uint16_t i;
                for (i = 0; i < count; i++) {
                    size_t pb = (size_t)(di - even_width + i) >> 1;
                    uint8_t c;
                    if (pb >= dst_size) return 0;
                    c = ((di - even_width + i) & 1)
                        ? (dst[pb] & 0x0f) : ((dst[pb] >> 4) & 0x0f);
                    dm2_v1_anim_setpixel_seq_4bpp(dst, dst_size,
                                                  (uint16_t)(di + i), c);
                }
            }
            di += count;
            dm2_v1_anim_setpixel_seq_4bpp(dst, dst_size, (uint16_t)di, op);
            di++;
            break;
        default:
            return 0;
        }
    }
    return 1;
}

int dm2_v1_anim_decode_en_keyframe(const DM2_V1_AnimChunk *chunk,
                                   uint8_t *dst,
                                   size_t dst_size,
                                   uint16_t *out_width,
                                   uint16_t *out_height)
{
    uint16_t w, h;

    if (!chunk || !dst || chunk->tag != DM2_V1_ANIM_CHUNK_EN ||
        chunk->payload_size < 6 || !chunk->payload)
        return 0;

    w = rd16be(chunk->payload + 2);
    h = rd16be(chunk->payload + 4);
    if (out_width) *out_width = w;
    if (out_height) *out_height = h;

    return decode_rle_partial(chunk->payload + 6,
                              chunk->payload_size - 6,
                              dst, dst_size, w, h);
}

int dm2_v1_anim_apply_dl_delta(const DM2_V1_AnimChunk *chunk,
                               uint8_t *framebuf,
                               size_t framebuf_size,
                               uint16_t frame_width,
                               uint16_t frame_height)
{
    if (!chunk || !framebuf || chunk->tag != DM2_V1_ANIM_CHUNK_DL ||
        chunk->payload_size < 8 || !chunk->payload)
        return 0;

    if (rd16be(chunk->payload + 4) != frame_width ||
        rd16be(chunk->payload + 6) != frame_height)
        return 0;

    return decode_rle_partial(chunk->payload + 8,
                              chunk->payload_size - 8,
                              framebuf, framebuf_size,
                              frame_width, frame_height);
}
