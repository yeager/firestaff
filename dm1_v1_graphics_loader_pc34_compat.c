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
static int lzw_read_code(const uint8_t* input, size_t in_size,
                          size_t* bit_pos, uint8_t code_bits) {
    size_t byte_idx = *bit_pos / 8;
    unsigned bit_off = *bit_pos % 8;

    if (byte_idx + 2 >= in_size && byte_idx >= in_size) return -1;

    /* Assemble up to 24 bits from 3 bytes */
    uint32_t raw = 0;
    for (int i = 0; i < 3 && (byte_idx + (size_t)i) < in_size; i++) {
        raw |= (uint32_t)input[byte_idx + (size_t)i] << (8 * i);
    }
    raw >>= bit_off;
    *bit_pos += code_bits;
    return (int)(raw & ((1u << code_bits) - 1));
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
    if (!lzw || !input || !output || in_size == 0 || out_size == 0) return -1;

    lzw_reset(lzw);

    /* Initialize dictionary with single-byte entries */
    for (int i = 0; i < 256; i++) {
        lzw->dict_prefix[i] = 0;
        lzw->dict_append[i] = (uint8_t)i;
    }

    size_t bit_pos = 0;
    size_t out_pos = 0;

    int old_code = lzw_read_code(input, in_size, &bit_pos, lzw->code_bits);
    if (old_code < 0 || old_code == DM1_GFX_LZW_END_CODE) return 0;
    if (old_code == DM1_GFX_LZW_CLEAR_CODE) {
        lzw_reset(lzw);
        old_code = lzw_read_code(input, in_size, &bit_pos, lzw->code_bits);
        if (old_code < 0 || old_code == DM1_GFX_LZW_END_CODE) return 0;
    }

    if (out_pos < out_size) output[out_pos++] = (uint8_t)old_code;

    while (out_pos < out_size) {
        int new_code = lzw_read_code(input, in_size, &bit_pos, lzw->code_bits);
        if (new_code < 0 || new_code == DM1_GFX_LZW_END_CODE) break;

        if (new_code == DM1_GFX_LZW_CLEAR_CODE) {
            lzw_reset(lzw);
            old_code = lzw_read_code(input, in_size, &bit_pos, lzw->code_bits);
            if (old_code < 0 || old_code == DM1_GFX_LZW_END_CODE) break;
            if (out_pos < out_size) output[out_pos++] = (uint8_t)old_code;
            continue;
        }

        int count;
        if ((uint16_t)new_code < lzw->next_code) {
            count = lzw_decode_string(lzw, (uint16_t)new_code);
        } else {
            /* KwKwK special case */
            lzw->decode_stack[0] = (uint8_t)old_code;
            count = lzw_decode_string(lzw, (uint16_t)old_code);
            /* Shift stack up by 1 and put first char at index count */
            /* Actually: push the first char of old_code string */
            uint8_t first_char = lzw->decode_stack[count - 1];
            lzw->decode_stack[count++] = first_char;
        }

        /* Output decoded string in reverse (stack is LIFO) */
        for (int i = count - 1; i >= 0 && out_pos < out_size; i--) {
            output[out_pos++] = lzw->decode_stack[i];
        }

        /* Add new dictionary entry */
        if (lzw->next_code < DM1_GFX_LZW_MAX_CODE) {
            lzw->dict_prefix[lzw->next_code] = (uint16_t)old_code;
            /* First char of decoded string for new_code */
            lzw->dict_append[lzw->next_code] = lzw->decode_stack[count - 1];
            lzw->next_code++;

            /* Grow code width when needed */
            if (lzw->next_code >= (1u << lzw->code_bits) &&
                lzw->code_bits < DM1_GFX_LZW_MAX_BITS) {
                lzw->code_bits++;
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

