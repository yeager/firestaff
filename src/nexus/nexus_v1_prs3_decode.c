#include "nexus_v1_prs3_decode.h"
#include <string.h>

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

int nexus_v1_prs3_parse_header(const uint8_t *data, int data_size,
                               Nexus_V1_Prs3Header *out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < 16) return 0;
    if (memcmp(data, "PRS3", 4) != 0) return 0;
    out->version = read_be32(data + 4);
    out->uncompressed_size = read_be32(data + 8);
    out->compressed_size = read_be32(data + 12);
    if (out->version != 1) return 0;
    if ((int)(16 + out->compressed_size) > data_size) return 0;
    out->stream = data + 16;
    out->valid = 1;
    return 1;
}

Nexus_V1_Prs3DecodeResult nexus_v1_prs3_decompress(
    const uint8_t *compressed, int compressed_size,
    uint8_t *output, int output_capacity,
    int uncompressed_size) {
    Nexus_V1_Prs3DecodeResult result;
    int sp = 0;
    int out_pos = 0;
    int target;

    memset(&result, 0, sizeof(result));
    if (!compressed || !output || compressed_size <= 0 ||
        output_capacity <= 0 || uncompressed_size <= 0) {
        return result;
    }

    target = uncompressed_size;
    if (target > output_capacity) target = output_capacity;

    while (out_pos < target && sp < compressed_size) {
        uint8_t ctrl = compressed[sp++];
        int bit_idx;

        for (bit_idx = 0; bit_idx < 8 && out_pos < target; ++bit_idx) {
            int bit = (ctrl >> bit_idx) & 1;

            if (bit == 1) {
                if (sp >= compressed_size) goto done;
                output[out_pos++] = compressed[sp++];
            } else {
                int b0, b1, num_pixels, raw_offset, offset, i;
                if (sp + 1 >= compressed_size) goto done;
                b0 = compressed[sp++];
                b1 = compressed[sp++];
                num_pixels = 3 + (b1 & 0x0F);
                raw_offset = ((b1 >> 4) << 8) | b0;
                offset = raw_offset - 4078;
                while (out_pos - offset > 4095) {
                    offset += 4096;
                }
                for (i = 0; i < num_pixels && out_pos < target; ++i) {
                    if (offset < 0) {
                        output[out_pos++] = 0;
                    } else if (offset < out_pos) {
                        output[out_pos++] = output[offset];
                    } else {
                        output[out_pos++] = 0;
                    }
                    offset++;
                }
            }
        }
    }

done:
    result.bytes_produced = (uint32_t)out_pos;
    result.bytes_consumed = (uint32_t)sp;
    result.success = (out_pos == uncompressed_size) ? 1 : 0;
    return result;
}
