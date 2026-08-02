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
