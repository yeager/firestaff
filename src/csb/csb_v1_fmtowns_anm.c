#include "csb_v1_fmtowns_anm.h"
#include <string.h>

static uint16_t rd16be(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static CSB_V1_FmtownsAnmChunkType classify_chunk(uint8_t a, uint8_t b) {
    if (a == 'P' && b == 'L') return CSB_FMTOWNS_ANM_CHUNK_PL;
    if (a == 'S' && b == 'D') return CSB_FMTOWNS_ANM_CHUNK_SD;
    if (a == 'E' && b == 'N') return CSB_FMTOWNS_ANM_CHUNK_EN;
    if (a == 'D' && b == 'L') return CSB_FMTOWNS_ANM_CHUNK_DL;
    if (a == 'K' && b == 'D') return CSB_FMTOWNS_ANM_CHUNK_KD;
    if (a == 'B' && b == 'R') return CSB_FMTOWNS_ANM_CHUNK_BR;
    if (a == 'A' && b == 'N') return CSB_FMTOWNS_ANM_CHUNK_AN;
    return CSB_FMTOWNS_ANM_CHUNK_UNKNOWN;
}

static uint32_t fnv1a32(const uint8_t *data, size_t size) {
    uint32_t hash = 2166136261u;
    size_t i;
    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static int anm_header(const uint8_t *data, size_t size, size_t *out_pos,
                      uint16_t *out_width, uint16_t *out_height) {
    size_t pos = 0;
    if (!data || !out_pos || !out_width || !out_height || size < 14u)
        return 0;
    if (data[0] == 'B' && data[1] == 'R') {
        if (size < 20u)
            return 0;
        pos = 6u;
    }
    if (pos + 14u > size || data[pos] != 'A' || data[pos + 1u] != 'N')
        return 0;
    *out_width = rd16be(data + pos + 6u);
    *out_height = rd16be(data + pos + 8u);
    if (*out_width == 0u || *out_height == 0u ||
        *out_width > CSB_FMTOWNS_ANM_MAX_WIDTH ||
        *out_height > CSB_FMTOWNS_ANM_MAX_HEIGHT ||
        rd16be(data + pos + 10u) != 4u)
        return 0;
    *out_pos = pos + 14u;
    return 1;
}

/* ReDMCSB ANIMIMG.C F8288.  The original stores a packed nibble bitmap;
 * Firestaff keeps one index per pixel so the draw code can use the authentic
 * palette without a second, host-specific unpacking rule. */
static int decode_f8288(const uint8_t *source, size_t source_size,
                        uint8_t *pixels, size_t pixel_count,
                        uint16_t expected_width, uint16_t expected_height) {
    size_t cursor = 4u;
    size_t dst = 0u;
    size_t total;
    uint16_t width, height;
    size_t stride;
    if (!source || !pixels || source_size < 5u) return 0;
    width = rd16be(source);
    height = rd16be(source + 2u);
    if (width != expected_width || height != expected_height) return 0;
    stride = ((size_t)width + 1u) & ~(size_t)1u;
    total = stride * (size_t)height;
    if (total == 0u || total > pixel_count) return 0;
    while (dst < total) {
        uint8_t command;
        size_t count;
        if (cursor >= source_size) return 0;
        command = source[cursor++];
        if ((command & 0x80u) == 0u) {
            count = (size_t)(command >> 4u) + 1u;
            if (count > total - dst) return 0;
            memset(pixels + dst, command & 0x0fu, count);
            dst += count;
            continue;
        }
        switch (command & 0x30u) {
        case 0x00u:
        case 0x30u:
            if ((command & 0x40u) == 0u) {
                if (cursor >= source_size) return 0;
                count = (size_t)source[cursor++] + 1u;
            } else {
                if (cursor + 2u > source_size) return 0;
                count = (size_t)rd16be(source + cursor) + 1u;
                cursor += 2u;
            }
            if (count > total - dst) return 0;
            if ((command & 0x30u) == 0x00u) {
                memset(pixels + dst, command & 0x0fu, count);
                dst += count;
            } else {
                size_t i;
                if (dst < stride || count >= total - dst) return 0;
                for (i = 0u; i < count; ++i)
                    pixels[dst + i] = pixels[dst - stride + i];
                dst += count;
                if (dst >= total) return 0;
                pixels[dst++] = command & 0x0fu;
            }
            break;
        case 0x10u:
            if ((command & 0x40u) == 0u) {
                if (cursor >= source_size) return 0;
                count = (size_t)source[cursor++] + 1u;
            } else {
                if (cursor + 2u > source_size) return 0;
                count = (size_t)rd16be(source + cursor) + 1u;
                cursor += 2u;
            }
            if (count > total - dst) return 0;
            if (count & 1u) {
                pixels[dst++] = command & 0x0fu;
                --count;
            }
            if (cursor + count / 2u > source_size) return 0;
            while (count != 0u) {
                uint8_t packed = source[cursor++];
                pixels[dst++] = (uint8_t)(packed >> 4u);
                pixels[dst++] = (uint8_t)(packed & 0x0fu);
                count -= 2u;
            }
            break;
        case 0x20u:
            count = (size_t)(command & 0x0fu) |
                    (size_t)((command >> 2u) & 0x10u);
            if (count == 29u) {
                if (cursor >= source_size) return 0;
                count = (size_t)source[cursor++] + 1u;
            } else if (count == 30u) {
                if (cursor >= source_size) return 0;
                count = (size_t)source[cursor++] + 257u;
            } else if (count == 31u) {
                if (cursor + 2u > source_size) return 0;
                count = (size_t)rd16be(source + cursor) + 1u;
                cursor += 2u;
            } else {
                ++count;
            }
            if (count > total - dst) return 0;
            dst += count;
            break;
        default:
            return 0;
        }
    }
    return 1;
}

int csb_v1_fmtowns_anm_probe(const uint8_t *data, size_t size) {
    if (!data || size < 12) return 0;
    if (data[0] == 'A' && data[1] == 'N') return 1;
    if (data[0] == 'B' && data[1] == 'R') return 1;
    return 0;
}

int csb_v1_fmtowns_anm_parse(const uint8_t *data, size_t size,
                               CSB_V1_FmtownsAnmReceipt *out) {
    size_t pos = 0;
    size_t an_start;

    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!csb_v1_fmtowns_anm_probe(data, size)) return -1;

    out->file_size = (uint32_t)size;

    /* Handle BR wrapper */
    if (data[0] == 'B' && data[1] == 'R') {
        out->has_br_wrapper = 1;
        pos = 6;
        if (pos >= size || data[pos] != 'A' || data[pos + 1] != 'N')
            return -1;
    }

    /* Parse AN header: "AN" + version + flags + 2 unknown + width(BE16) + height(BE16) + bpp(BE16) + 2 unknown = 14 bytes */
    if (pos + 14 > size) return -1;
    if (data[pos] != 'A' || data[pos + 1] != 'N') return -1;

    out->version = data[pos + 2];
    out->flags = data[pos + 3];
    an_start = pos;

    out->width = rd16be(data + pos + 6);
    out->height = rd16be(data + pos + 8);
    out->bpp = (uint8_t)rd16be(data + pos + 10);
    pos += 14;

    /* Parse subchunks */
    /* Chunk format: 2-byte ID + 2-byte BE16 data size + 2-byte flags/unknown + data */
    while (pos + 6 <= size) {
        CSB_V1_FmtownsAnmChunkType type;
        uint16_t chunk_size;

        if (data[pos] < 'A' || data[pos] > 'Z' ||
            data[pos + 1] < 'A' || data[pos + 1] > 'Z')
            break;

        type = classify_chunk(data[pos], data[pos + 1]);
        chunk_size = rd16be(data + pos + 2);

        if (pos + 6 + chunk_size > size) break;

        switch (type) {
        case CSB_FMTOWNS_ANM_CHUNK_PL: {
            const uint8_t *pal = data + pos + 6;
            int i;
            int pal_count = (chunk_size >= 2) ? rd16be(pal) : 0;
            if (pal_count > CSB_FMTOWNS_ANM_PALETTE_SIZE)
                pal_count = CSB_FMTOWNS_ANM_PALETTE_SIZE;
            if (chunk_size >= 2 + (uint16_t)pal_count * 4) {
                for (i = 0; i < pal_count; i++) {
                    out->palette[pal[2 + i * 4]].r = pal[2 + i * 4 + 1];
                    out->palette[pal[2 + i * 4]].g = pal[2 + i * 4 + 2];
                    out->palette[pal[2 + i * 4]].b = pal[2 + i * 4 + 3];
                }
                out->has_palette = 1;
            }
            out->palette_count++;
            break;
        }
        case CSB_FMTOWNS_ANM_CHUNK_EN:
            out->frame_count++;
            break;
        case CSB_FMTOWNS_ANM_CHUNK_DL:
            out->delta_count++;
            break;
        case CSB_FMTOWNS_ANM_CHUNK_KD:
            out->keyframe_count++;
            break;
        default:
            break;
        }

        out->chunk_count++;
        pos += 6 + chunk_size;
    }

    out->valid = (out->width > 0 && out->height > 0 &&
                  out->chunk_count > 0) ? 1 : 0;
    (void)an_start;
    return out->valid ? 0 : -1;
}

int csb_v1_fmtowns_anm_decode_frame(const uint8_t *data, size_t size,
                                    uint32_t frame_index,
                                    uint8_t *out_pixels,
                                    size_t out_pixel_capacity,
                                    CSB_V1_FmtownsAnmFrameReceipt *out) {
    size_t pos;
    uint16_t width, height;
    size_t pixel_count;
    uint32_t seen_frames = 0u;
    int palette_seen = 0;
    CSB_V1_FmtownsAnmColor palette[CSB_FMTOWNS_ANM_PALETTE_SIZE];
    if (!out || !out_pixels) return -1;
    memset(out, 0, sizeof(*out));
    memset(palette, 0, sizeof(palette));
    if (!anm_header(data, size, &pos, &width, &height)) return -1;
    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > out_pixel_capacity) return -1;
    memset(out_pixels, 0, pixel_count);
    while (pos + 6u <= size) {
        CSB_V1_FmtownsAnmChunkType type;
        uint16_t bytes;
        const uint8_t *payload;
        if (data[pos] < 'A' || data[pos] > 'Z' ||
            data[pos + 1u] < 'A' || data[pos + 1u] > 'Z') return -1;
        type = classify_chunk(data[pos], data[pos + 1u]);
        bytes = rd16be(data + pos + 2u);
        if ((size_t)bytes > size - pos - 6u) return -1;
        payload = data + pos + 6u;
        if (type == CSB_FMTOWNS_ANM_CHUNK_PL) {
            unsigned int i;
            if (bytes < 2u || rd16be(payload) != 16u || bytes < 66u)
                return -1;
            for (i = 0u; i < CSB_FMTOWNS_ANM_PALETTE_SIZE; ++i) {
                uint8_t index = payload[2u + i * 4u];
                if (index >= CSB_FMTOWNS_ANM_PALETTE_SIZE) return -1;
                palette[index].r = payload[3u + i * 4u];
                palette[index].g = payload[4u + i * 4u];
                palette[index].b = payload[5u + i * 4u];
            }
            palette_seen = 1;
        } else if (type == CSB_FMTOWNS_ANM_CHUNK_EN ||
                   type == CSB_FMTOWNS_ANM_CHUNK_DL) {
            size_t payload_offset = 0u;
            if (bytes >= 2u && payload[0] == 0xffu && payload[1] == 0x81u)
                payload_offset = 2u;
            if ((size_t)bytes < payload_offset + 4u ||
                !decode_f8288(payload + payload_offset,
                               (size_t)bytes - payload_offset,
                               out_pixels, pixel_count, width, height))
                return -1;
            if (seen_frames == frame_index) {
                out->valid = 1;
                out->width = width;
                out->height = height;
                out->frame_index = frame_index;
                out->source_chunk_offset = (uint32_t)pos;
                out->source_chunk_bytes = bytes;
                out->source_was_delta = type == CSB_FMTOWNS_ANM_CHUNK_DL;
                out->palette_applied = palette_seen;
                memcpy(out->palette, palette, sizeof(palette));
                out->pixel_fnv1a = fnv1a32(out_pixels, pixel_count);
                return 0;
            }
            ++seen_frames;
        }
        pos += 6u + (size_t)bytes;
    }
    return -1;
}
