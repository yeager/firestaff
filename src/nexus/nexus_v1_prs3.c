#include "nexus_v1_prs3.h"
#include <string.h>

/* Read a 32-bit big-endian value from a byte pointer. */
static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}

#define PRS3_MAGIC      0x50525333u  /* 'PRS3' */
#define PRS3_HEADER_SIZE 16

size_t nexus_v1_prs3_decode(const uint8_t *src, size_t src_size,
                            uint8_t *dst, size_t dst_size) {
    if (!src || !dst || src_size < PRS3_HEADER_SIZE)
        return 0;

    /* Parse header (big-endian). */
    uint32_t magic           = read_be32(src + 0);
    uint32_t version          = read_be32(src + 4);
    uint32_t uncompressed_sz = read_be32(src + 8);
    uint32_t compressed_sz   = read_be32(src + 12);

    if (magic != PRS3_MAGIC)
        return 0;
    /* DMWeb's DMNDataFileDecoder accepts PRS3 version 1 only. */
    if (version != 1)
        return 0;
    if ((size_t)uncompressed_sz > dst_size)
        return 0;
    if ((size_t)compressed_sz + PRS3_HEADER_SIZE > src_size)
        return 0;

    const uint8_t *in  = src + PRS3_HEADER_SIZE;
    const uint8_t *in_end = in + compressed_sz;
    size_t out_pos = 0;
    size_t target  = uncompressed_sz;

    while (out_pos < target && in < in_end) {
        uint8_t control = *in++;
        for (int bit = 0; bit < 8 && out_pos < target; bit++) {
            if (control & (1u << bit)) {
                /* Literal byte. */
                if (in >= in_end) return 0;
                dst[out_pos++] = *in++;
            } else {
                /* Back-reference: two bytes encode offset + count. */
                if (in + 1 >= in_end) return 0;
                uint8_t byte0 = *in++;
                uint8_t byte1 = *in++;

                int count      = 3 + (byte1 & 0x0F);
                int raw_offset = ((byte1 >> 4) << 8) | byte0;

                /* DMWeb DMNDataFileDecoder.vbs::DecodePRS3 uses two
                 * 12-bit window regions: values below &HFDC address the
                 * forward window after a +18 bias, while &HFDC..&HFFF
                 * address the negative window after subtracting &HFEE. */
                int offset = raw_offset >= 0xFDC ?
                    raw_offset - 0xFEE : raw_offset + 18;
                while ((int)out_pos - offset > 4095)
                    offset += 4096;

                /* Copy count bytes from the absolute offset position.
                 * DMWeb defines only the negative prefix as zero-filled;
                 * a positive forward reference is malformed and must fail
                 * closed instead of manufacturing pixels. */
                for (int i = 0; i < count && out_pos < target; i++) {
                    int pos = offset + i;
                    if (pos < 0) {
                        dst[out_pos++] = 0x00;
                    } else if ((size_t)pos < out_pos) {
                        dst[out_pos++] = dst[pos];
                    } else {
                        return 0;
                    }
                }
            }
        }
    }

    return out_pos;
}
