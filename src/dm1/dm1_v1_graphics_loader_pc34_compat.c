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

/*
 * Reset the LZW dictionary and code-width state but PRESERVE
 * the input stream position AND chunk state. This is the
 * correct behaviour for a CLEAR_CODE handler in standard LZW
 * (per Welch 1984 + GIF spec): the dictionary resets, the
 * bitstream position stays where it was, and the next code is
 * read from the chunk we were already in.
 *
 * An earlier version of this function also cleared
 * chunk_bit_idx / chunk_bit_count / needs_refill / byte_pos,
 * which forced an unwanted chunk refill from a stale byte_pos
 * after every CLEAR_CODE -- producing garbage codes instead of
 * the actual next code. The fix splits stream-init from
 * dict-reset: see lzw_init() and lzw_reset_dict().
 *
 * Verified by tests/test_dm1_lzw_round_trip.c (the pre-fix
 * decoder decoded 'ABC' + END as 4 bytes with a leading 0x00,
 * because the stale refill grabbed bytes from the middle of
 * the input; the post-fix decoder returns 3 bytes 'A','B','C').
 */
static void lzw_reset_dict(M11_GFX_LZWState* lzw) {
    lzw->next_code = DM1_GFX_LZW_FIRST_CODE;
    lzw->code_bits = 9;
    /* NOTE: deliberately do NOT touch byte_pos, chunk_bit_idx,
       chunk_bit_count, or needs_refill here. We want the next
       lzw_read_code() to keep reading from the same chunk at
       the same bit position where the CLEAR_CODE left off. */
}

/*
 * Full stream-and-dict initialization. Called once at the
 * start of m11_gfx_lzw_decompress. Resets byte_pos to 0 so
 * reading begins at the start of the input buffer.
 */
static void lzw_init(M11_GFX_LZWState* lzw) {
    lzw->flushed = true;
    lzw->byte_pos = 0;
    lzw->chunk_bit_idx = 0;
    lzw->chunk_bit_count = 0;
    lzw->needs_refill = 1;
    lzw_reset_dict(lzw);
}

/* Read a variable-width code from the input bitstream */
/* ReDMCSB LZW.C F0495_LZW_GetNextInputCode:
 * Reads codeBitCount BYTES into a 12-byte chunk buffer,
 * then extracts codes from bit positions within that chunk.
 * This is NOT a continuous bitstream — it re-reads a new chunk
 * every time the chunk is exhausted or code width changes. */

static int lzw_read_code(M11_GFX_LZWState* lzw,
                          const uint8_t* input, size_t in_size,
                          size_t* bit_pos_unused, uint8_t code_bits) {
    static const uint8_t lsb_masks[9] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0x7F,0xFF};
    int result, bi, required;
    const uint8_t* p;
    (void)bit_pos_unused;

    /* Refill chunk when exhausted, after flush, or when code width changed */
    if (lzw->needs_refill || lzw->chunk_bit_idx >= lzw->chunk_bit_count) {
        int chunk_bytes = code_bits;
        if (lzw->byte_pos + (size_t)chunk_bytes > in_size)
            chunk_bytes = (int)(in_size - lzw->byte_pos);
        if (chunk_bytes <= 0) return -1;
        memcpy(lzw->chunk, input + lzw->byte_pos, chunk_bytes);
        lzw->byte_pos += chunk_bytes;
        lzw->chunk_bit_idx = 0;
        lzw->chunk_bit_count = (chunk_bytes << 3) - (code_bits - 1);
        lzw->needs_refill = 0;
    }

    bi = lzw->chunk_bit_idx;
    required = code_bits;
    p = lzw->chunk + (bi >> 3);
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

    lzw->chunk_bit_idx += code_bits;
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
    lzw_init(lzw);
    for (i = 0; i < 256; i++) {
        lzw->dict_prefix[i] = 0xFFFF;
        lzw->dict_append[i] = (uint8_t)i;
    }
    lzw->next_code = DM1_GFX_LZW_FIRST_CODE;
    lzw->code_bits = 9;

    /* First code */
    old_code = lzw_read_code(lzw, input, in_size, NULL, lzw->code_bits);
    if (old_code < 0) return 0;
    if (old_code == DM1_GFX_LZW_CLEAR_CODE) {
        lzw_reset_dict(lzw);
        old_code = lzw_read_code(lzw, input, in_size, NULL, lzw->code_bits);
        if (old_code < 0) return 0;
    }
    if (out_pos < out_size) output[out_pos++] = (uint8_t)old_code;

    while (out_pos < out_size) {
        new_code = lzw_read_code(lzw, input, in_size, NULL, lzw->code_bits);
        if (new_code < 0) break;

        if (new_code == DM1_GFX_LZW_CLEAR_CODE) {
            lzw_reset_dict(lzw);
            old_code = lzw_read_code(lzw, input, in_size, NULL, lzw->code_bits);
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
            /* KwKwK case: new_code == next_code.
             *
             * The new string is defined as: string(old_code) +
             * first_char_of(string(old_code)). After
             * lzw_decode_string(old_code), the stack holds
             * string(old_code) reversed (so stack[count-1] is the
             * FIRST character). To produce the KwKwK output we
             * emit the stack reversed and then append first_char.
             *
             * Previously this code emitted first_char FIRST and
             * then the reversed stack, which produces
             * "AAB" instead of "ABA" for old_code -> "AB".
             * That bug made round-trip fail on any input
             * exercising the KwKwK edge case (alternating
             * two-byte repeats is the canonical trigger).
             * Verified by tests/test_dm1_lzw_round_trip.c. */
            count = lzw_decode_string(lzw, (uint16_t)old_code);
            first_char = (count > 0) ? lzw->decode_stack[count - 1] : (uint8_t)old_code;
            /* Note: do NOT emit first_char here. The for-loop
               below will emit stack reversed (which is the
               forward string of old_code), and we append
               first_char at the end. */
        }

        /* Output decoded string (stack is reversed, so iterating
           from count-1 down to 0 produces the forward string) */
        for (i = count - 1; i >= 0 && out_pos < out_size; i--)
            output[out_pos++] = lzw->decode_stack[i];

        /* For the KwKwK case (new_code == next_code), append the
           first_char of the old_code's string. This produces
           string(old_code) + first_char, which is the standard
           LZW KwKwK output. */
        if ((uint16_t)new_code >= lzw->next_code) {
            if (out_pos < out_size) output[out_pos++] = first_char;
        }

        /* Add to dictionary */
        if (lzw->next_code < DM1_GFX_LZW_MAX_CODE) {
            lzw->dict_prefix[lzw->next_code] = (uint16_t)old_code;
            lzw->dict_append[lzw->next_code] = first_char;
            lzw->next_code++;

            /* Grow code width */
            if (lzw->next_code > (uint16_t)((1 << lzw->code_bits) - 1) && lzw->code_bits < 12) {
                lzw->code_bits++;
                lzw->needs_refill = 1;  /* ReDMCSB refills on width change */
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
