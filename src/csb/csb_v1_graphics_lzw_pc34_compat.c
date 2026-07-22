#include "csb_v1_graphics_lzw_pc34_compat.h"

#include <string.h>

#define CSB_V1_GRAPHICS_LZW_MAX_CODE 4096
#define CSB_V1_GRAPHICS_LZW_CLEAR_CODE 256
#define CSB_V1_GRAPHICS_LZW_FIRST_CODE 257

typedef struct {
    const uint8_t *bytes;
    size_t size;
    size_t byte_pos;
    uint8_t chunk[12];
    int chunk_bit_idx;
    int chunk_bit_count;
    int needs_refill;
} CSB_V1_GraphicsBitReader;

static int csb_v1_graphics_read_bits(CSB_V1_GraphicsBitReader *br,
                                     int bit_count,
                                     uint16_t *out_code)
{
    static const uint8_t lsb_masks[9] = {
        0x00u, 0x01u, 0x03u, 0x07u, 0x0fu, 0x1fu, 0x3fu, 0x7fu, 0xffu
    };
    uint32_t value = 0u;
    int bit_index;
    int required;
    const uint8_t *p;

    if (!br || !out_code || bit_count <= 0 || bit_count > 12) return -1;
    /* LZW.C refills on a code-width boundary, rather than consuming a flat
     * bit stream. That detail is necessary for the original C001-C005 data. */
    if (br->needs_refill || br->chunk_bit_idx >= br->chunk_bit_count) {
        int chunk_bytes = bit_count;
        if (br->byte_pos + (size_t)chunk_bytes > br->size)
            chunk_bytes = (int)(br->size - br->byte_pos);
        if (chunk_bytes <= 0) return -1;
        memset(br->chunk, 0, sizeof(br->chunk));
        memcpy(br->chunk, br->bytes + br->byte_pos, (size_t)chunk_bytes);
        br->byte_pos += (size_t)chunk_bytes;
        br->chunk_bit_idx = 0;
        br->chunk_bit_count = (chunk_bytes << 3) - (bit_count - 1);
        br->needs_refill = 0;
    }
    bit_index = br->chunk_bit_idx;
    required = bit_count;
    p = br->chunk + (bit_index >> 3);
    bit_index &= 7;
    value = (uint32_t)(*p++ >> bit_index);
    required -= 8 - bit_index;
    bit_index = 8 - bit_index;
    if (required >= 8) {
        value |= (uint32_t)(*p++) << bit_index;
        bit_index += 8;
        required -= 8;
    }
    if (required > 0) value |= (uint32_t)(*p & lsb_masks[required]) << bit_index;
    br->chunk_bit_idx += bit_count;
    *out_code = (uint16_t)value;
    return 0;
}

static void csb_v1_graphics_lzw_reset(uint16_t *prefix,
                                      uint8_t *append,
                                      int *next_code,
                                      int *code_bits)
{
    int i;
    for (i = 0; i < 256; i++) {
        prefix[i] = 0xffffu;
        append[i] = (uint8_t)i;
    }
    *next_code = CSB_V1_GRAPHICS_LZW_FIRST_CODE;
    *code_bits = 9;
}

typedef struct {
    uint8_t repeat_pending;
    uint8_t repeat_character;
} CSB_V1_GraphicsRleState;

static int csb_v1_graphics_lzw_write_byte(CSB_V1_GraphicsRleState *rle,
                                           uint8_t value, uint8_t *out,
                                           size_t out_capacity,
                                           size_t *out_pos)
{
    size_t repeat_count;

    if (!rle || !out || !out_pos) return -1;
    if (!rle->repeat_pending) {
        if (value == 0x90u) {
            rle->repeat_pending = 1u;
            return 0;
        }
        if (*out_pos >= out_capacity) return -1;
        out[(*out_pos)++] = value;
        rle->repeat_character = value;
        return 0;
    }
    rle->repeat_pending = 0u;
    if (value == 0u) {
        if (*out_pos >= out_capacity) return -1;
        out[(*out_pos)++] = 0x90u;
        return 0;
    }
    repeat_count = (size_t)value - 1u;
    if (repeat_count > out_capacity - *out_pos) return -1;
    memset(out + *out_pos, rle->repeat_character, repeat_count);
    *out_pos += repeat_count;
    return 0;
}

static int csb_v1_graphics_lzw_emit(uint16_t code,
                                    const uint16_t *prefix,
                                    const uint8_t *append,
                                    uint8_t *stack,
                                    CSB_V1_GraphicsRleState *rle,
                                    uint8_t *out,
                                    size_t out_capacity,
                                    size_t *out_pos,
                                    uint8_t *out_first)
{
    int stack_len = 0;
    uint16_t cursor = code;
    while (cursor >= 256u) {
        if (cursor >= CSB_V1_GRAPHICS_LZW_MAX_CODE || prefix[cursor] == 0xffffu ||
            stack_len >= CSB_V1_GRAPHICS_LZW_MAX_CODE) return -1;
        stack[stack_len++] = append[cursor];
        cursor = prefix[cursor];
    }
    if (out_first) *out_first = (uint8_t)cursor;
    if (csb_v1_graphics_lzw_write_byte(
            rle, (uint8_t)cursor, out, out_capacity, out_pos) != 0)
        return -1;
    while (stack_len > 0) {
        if (csb_v1_graphics_lzw_write_byte(
                rle, stack[--stack_len], out, out_capacity, out_pos) != 0)
            return -1;
    }
    return 0;
}

int csb_v1_graphics_lzw_decode_pc34_compat(const uint8_t *input,
                                            size_t input_size,
                                            uint8_t *out,
                                            size_t out_capacity,
                                            size_t *out_size)
{
    uint16_t prefix[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    uint8_t append[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    uint8_t stack[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    CSB_V1_GraphicsBitReader br;
    int next_code;
    int code_bits;
    int old_code = -1;
    uint8_t old_first = 0u;
    size_t out_pos = 0u;
    CSB_V1_GraphicsRleState rle;

    if (!input || !out || !out_size || input_size == 0u) return -1;
    br.bytes = input;
    br.size = input_size;
    br.byte_pos = 0u;
    br.chunk_bit_idx = 0;
    br.chunk_bit_count = 0;
    br.needs_refill = 1;
    csb_v1_graphics_lzw_reset(prefix, append, &next_code, &code_bits);
    memset(&rle, 0, sizeof(rle));
    for (;;) {
        uint16_t code;
        uint8_t first = 0u;
        if (csb_v1_graphics_read_bits(&br, code_bits, &code) != 0) {
            *out_size = out_pos;
            return rle.repeat_pending ? -1 : 0;
        }
        if (code == CSB_V1_GRAPHICS_LZW_CLEAR_CODE) {
            csb_v1_graphics_lzw_reset(prefix, append, &next_code, &code_bits);
            old_code = -1;
            /* CSBWin Graphics.cpp::LZWExpand restarts at code 256 after a
             * clear and consumes the following code as ordinary data. */
            next_code = CSB_V1_GRAPHICS_LZW_CLEAR_CODE;
            continue;
        }
        if (code < (uint16_t)next_code) {
            if (csb_v1_graphics_lzw_emit(code, prefix, append, stack, &rle, out,
                                         out_capacity, &out_pos, &first) != 0) return -1;
        } else if (code == (uint16_t)next_code && old_code >= 0) {
            first = old_first;
            if (csb_v1_graphics_lzw_emit((uint16_t)old_code, prefix, append,
                                         stack, &rle, out, out_capacity, &out_pos,
                                         NULL) != 0 || out_pos >= out_capacity) return -1;
            if (csb_v1_graphics_lzw_write_byte(
                    &rle, first, out, out_capacity, &out_pos) != 0) return -1;
        } else {
            return -1;
        }
        if (old_code >= 0 && next_code < CSB_V1_GRAPHICS_LZW_MAX_CODE) {
            prefix[next_code] = (uint16_t)old_code;
            append[next_code] = first;
            next_code++;
            if (next_code > ((1 << code_bits) - 1) && code_bits < 12) {
                code_bits++;
                br.needs_refill = 1;
            }
        }
        old_code = (int)code;
        old_first = first;
    }
}
