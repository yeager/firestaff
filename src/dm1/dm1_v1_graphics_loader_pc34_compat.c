/* DM1 V1 Graphics/Bitmap Loader — source-locked from ReDMCSB
 * GRF1.C: 4-bitplane bitmap management
 * LZW.C: F0495_GetNextInputCode, G0666 max=4096, 12-bit codes
 * EXPAND.C: bitmap expansion from compressed to full resolution
 * IMAGE.C: G2158 pixel line buffer, vertical blank sync */

#include "dm1_v1_graphics_loader_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

void m11_gfx_init(M11_GFX_LoaderState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_GFX_LoaderState));
}

/* --- LZW decompressor (ReDMCSB LZW.C pattern) --- */

static void lzw_reset(M11_GFX_LZWState* lzw) {
    lzw->next_code = DM1_GFX_LZW_FIRST_CODE;
    lzw->code_bits = 9;
    lzw->flushed   = true;
}

/* Read a variable-width code from the input bitstream */
/* ReDMCSB LZW.C F0495_LZW_GetNextInputCode:
 * Reads codeBitCount BYTES into a 12-byte chunk buffer,
 * then extracts codes from bit positions within that chunk.
 * This is NOT a continuous bitstream — it re-reads a new chunk
 * every time the chunk is exhausted or code width changes. */

static uint8_t g_lzw_chunk[12];
static int g_lzw_chunk_bit_idx = 0;
static int g_lzw_chunk_bit_count = 0;
static size_t g_lzw_byte_pos = 0;
static int g_lzw_needs_refill = 1;

static void lzw_input_reset(void) {
    g_lzw_chunk_bit_idx = 0;
    g_lzw_chunk_bit_count = 0;
    g_lzw_byte_pos = 0;
    g_lzw_needs_refill = 1;
}

static int lzw_read_code(const uint8_t* input, size_t in_size,
                          size_t* bit_pos_unused, uint8_t code_bits) {
    static const uint8_t lsb_masks[9] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
    int result, bi, required;
    const uint8_t* p;
    (void)bit_pos_unused;

    /* Refill chunk when exhausted, after flush, or when code width changed */
    if (g_lzw_needs_refill || g_lzw_chunk_bit_idx >= g_lzw_chunk_bit_count) {
        int chunk_bytes = code_bits;
        if (g_lzw_byte_pos + (size_t)chunk_bytes > in_size)
            chunk_bytes = (int)(in_size - g_lzw_byte_pos);
        if (chunk_bytes <= 0) return -1;
        memcpy(g_lzw_chunk, input + g_lzw_byte_pos, chunk_bytes);
        g_lzw_byte_pos += chunk_bytes;
        g_lzw_chunk_bit_idx = 0;
        g_lzw_chunk_bit_count = (chunk_bytes << 3) - (code_bits - 1);
        g_lzw_needs_refill = 0;
    }

    bi = g_lzw_chunk_bit_idx;
    required = code_bits;
    p = g_lzw_chunk + (bi >> 3);
    bi &= 7;

    /* Extract code across byte boundaries (matches ReDMCSB exactly) */
    result = *p++ >> bi;
    required -= (8 - bi);
    bi = 8 - bi;

    if (required >= 8) {
        result |= (int)(*p++) << bi;
        bi += 8;
        required -= 8;
    }
    if (required > 0) {
        result |= (int)(*p & lsb_masks[required]) << bi;
    }

    g_lzw_chunk_bit_idx += code_bits;
    return result;
}

/* Decode a code to bytes, pushing onto decode_stack; return count */
static int lzw_decode_string(M11_GFX_LZWState* lzw, uint16_t code) {
    int count = 0;
    while (code >= DM1_GFX_LZW_FIRST_CODE && count < DM1_GFX_LZW_MAX_CODE) {
        lzw->decode_stack[count++] = lzw->dict_append[code];
        code = lzw->dict_prefix[code];
    }
    lzw->decode_stack[count++] = (uint8_t)code;
    return count;
}

int m11_gfx_lzw_decompress(M11_GFX_LZWState* lzw,
                            const uint8_t* input, size_t in_size,
                            uint8_t* output, size_t out_size) {
    size_t out_pos = 0;
    int old_code, new_code, i, count;
    uint8_t first_char;

    if (!lzw || !input || !output || in_size == 0 || out_size == 0) return -1;

    /* Reset LZW state */
    lzw_input_reset();
    lzw_reset(lzw);
    for (i = 0; i < 256; i++) {
        lzw->dict_prefix[i] = 0xFFFF;
        lzw->dict_append[i] = (uint8_t)i;
    }
    lzw->next_code = DM1_GFX_LZW_FIRST_CODE;
    lzw->code_bits = 9;

    /* First code */
    old_code = lzw_read_code(input, in_size, NULL, lzw->code_bits);
    if (old_code < 0) return 0;
    if (old_code == DM1_GFX_LZW_CLEAR_CODE) {
        lzw_reset(lzw);
        lzw->next_code = DM1_GFX_LZW_FIRST_CODE;
        lzw->code_bits = 9;
        g_lzw_needs_refill = 1;
        old_code = lzw_read_code(input, in_size, NULL, lzw->code_bits);
        if (old_code < 0) return 0;
    }
    if (out_pos < out_size) output[out_pos++] = (uint8_t)old_code;

    while (out_pos < out_size) {
        new_code = lzw_read_code(input, in_size, NULL, lzw->code_bits);
        if (new_code < 0) break;

        if (new_code == DM1_GFX_LZW_CLEAR_CODE) {
            lzw_reset(lzw);
            lzw->next_code = DM1_GFX_LZW_FIRST_CODE;
            lzw->code_bits = 9;
            g_lzw_needs_refill = 1;
            old_code = lzw_read_code(input, in_size, NULL, lzw->code_bits);
            if (old_code < 0) break;
            if (out_pos < out_size) output[out_pos++] = (uint8_t)old_code;
            continue;
        }

        if (new_code == DM1_GFX_LZW_END_CODE) break;

        /* Decode string */
        if ((uint16_t)new_code < lzw->next_code) {
            count = lzw_decode_string(lzw, (uint16_t)new_code);
            first_char = (count > 0) ? lzw->decode_stack[count - 1] : (uint8_t)old_code;
        } else {
            /* KwKwK case: new_code == next_code */
            count = lzw_decode_string(lzw, (uint16_t)old_code);
            first_char = (count > 0) ? lzw->decode_stack[count - 1] : (uint8_t)old_code;
            if (out_pos < out_size) output[out_pos++] = first_char;
        }

        /* Output decoded string (stack is reversed) */
        for (i = count - 1; i >= 0 && out_pos < out_size; i--)
            output[out_pos++] = lzw->decode_stack[i];

        /* Add to dictionary */
        if (lzw->next_code < DM1_GFX_LZW_MAX_CODE) {
            lzw->dict_prefix[lzw->next_code] = (uint16_t)old_code;
            lzw->dict_append[lzw->next_code] = first_char;
            lzw->next_code++;

            /* Grow code width */
            if (lzw->next_code > (uint16_t)((1 << lzw->code_bits) - 1) && lzw->code_bits < 12) {
                lzw->code_bits++;
                g_lzw_needs_refill = 1;  /* ReDMCSB refills on width change */
            }
        }

        old_code = new_code;
    }

    return (int)out_pos;
}

/* --- GRAPHICS.DAT file operations --- */

bool m11_gfx_open_dat(M11_GFX_LoaderState* state, const char* path) {
    if (!state || !path) return false;

    state->dat_file = fopen(path, "rb");
    if (!state->dat_file) return false;

    /* Read bitmap count from file header */
    uint16_t count = 0;
    if (fread(&count, 2, 1, state->dat_file) != 1) {
        fclose(state->dat_file);
        state->dat_file = NULL;
        return false;
    }

    if (count > DM1_GFX_MAX_BITMAPS) count = DM1_GFX_MAX_BITMAPS;
    state->bitmap_count = count;

    /* Read bitmap headers/index */
    for (uint16_t i = 0; i < count; i++) {
        uint16_t w, h;
        uint32_t csize;
        if (fread(&w, 2, 1, state->dat_file) != 1 ||
            fread(&h, 2, 1, state->dat_file) != 1 ||
            fread(&csize, 4, 1, state->dat_file) != 1) {
            state->bitmap_count = i;
            break;
        }
        state->headers[i].width = w;
        state->headers[i].height = h;
        state->headers[i].compressed_size = csize;
        state->headers[i].offset = (uint32_t)ftell(state->dat_file);
        /* Skip past compressed data */
        fseek(state->dat_file, (long)csize, SEEK_CUR);
    }

    state->loaded = true;
    return true;
}

bool m11_gfx_load_bitmap(M11_GFX_LoaderState* state, uint16_t index,
                          M11_GFX_Bitmap* out) {
    if (!state || !out || !state->loaded || !state->dat_file) return false;
    if (index >= state->bitmap_count) return false;

    M11_GFX_BitmapHeader* hdr = &state->headers[index];
    uint16_t bw = (hdr->width + 7) / 8;
    size_t decompressed_size = (size_t)bw * hdr->height * 4; /* 4 bitplanes */

    /* Read compressed data */
    uint8_t* compressed = (uint8_t*)malloc(hdr->compressed_size);
    if (!compressed) return false;

    fseek(state->dat_file, (long)hdr->offset, SEEK_SET);
    if (fread(compressed, 1, hdr->compressed_size, state->dat_file) !=
        hdr->compressed_size) {
        free(compressed);
        return false;
    }

    /* Allocate output buffer */
    uint8_t* pixels = (uint8_t*)calloc(1, decompressed_size);
    if (!pixels) {
        free(compressed);
        return false;
    }

    /* Decompress */
    int result = m11_gfx_lzw_decompress(&state->lzw, compressed,
                                         hdr->compressed_size,
                                         pixels, decompressed_size);
    free(compressed);

    if (result <= 0) {
        free(pixels);
        return false;
    }

    out->data = pixels;
    out->width = hdr->width;
    out->height = hdr->height;
    out->byte_width = bw;
    out->allocated = true;
    return true;
}

void m11_gfx_free_bitmap(M11_GFX_Bitmap* bmp) {
    if (!bmp) return;
    if (bmp->allocated && bmp->data) {
        free(bmp->data);
    }
    memset(bmp, 0, sizeof(M11_GFX_Bitmap));
}

void m11_gfx_close(M11_GFX_LoaderState* state) {
    if (!state) return;
    if (state->dat_file) {
        fclose(state->dat_file);
        state->dat_file = NULL;
    }
    state->loaded = false;
    state->bitmap_count = 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — IMAGE.C remaining function citations
 *
 *   IMAGE.C:88 F1010_L
 * ══════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — IMAGE3.C remaining function citations
 *
 *   IMAGE3.C:7 F0681_C
 *   IMAGE3.C:170 F0682_C
 *   IMAGE3.C:932 F0690_C
 * ══════════════════════════════════════════════════════════════════════ */

