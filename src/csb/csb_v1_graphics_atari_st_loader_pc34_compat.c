/*
 * csb_v1_graphics_atari_st_loader_pc34_compat.c
 *
 * Atari ST Chaos Strikes Back GRAPHICS.DAT item loader.
 * See header for the full contract.
 */

#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

void csb_atari_st_graphics_loader_init(CSB_AtariStLoader* state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

bool csb_atari_st_graphics_loader_open(CSB_AtariStLoader* state, const char* path)
{
    if (!state || !path) return false;

    state->dat_file = fopen(path, "rb");
    if (!state->dat_file) return false;

    strncpy(state->dat_path, path, sizeof(state->dat_path) - 1);
    state->dat_path[sizeof(state->dat_path) - 1] = '\0';

    /* Read count (u16 big-endian). */
    uint16_t count_be;
    if (fread(&count_be, 2, 1, state->dat_file) != 1) {
        fclose(state->dat_file);
        state->dat_file = NULL;
        return false;
    }
    /* Manual byte-swap to big-endian if the host is little-endian. */
    uint16_t count = (uint16_t)((count_be << 8) | (count_be >> 8));
    if (count == 0 || count > CSB_ATARI_ST_GRAPHICS_MAX_ITEMS) {
        fclose(state->dat_file);
        state->dat_file = NULL;
        return false;
    }
    state->item_count = count;

    /* Read (comp, decomp) word pairs. */
    for (uint16_t i = 0; i < count; ++i) {
        uint16_t comp_be, decomp_be;
        if (fread(&comp_be, 2, 1, state->dat_file) != 1 ||
            fread(&decomp_be, 2, 1, state->dat_file) != 1) {
            fclose(state->dat_file);
            state->dat_file = NULL;
            return false;
        }
        uint16_t comp   = (uint16_t)((comp_be << 8)   | (comp_be >> 8));
        uint16_t decomp = (uint16_t)((decomp_be << 8) | (decomp_be >> 8));
        state->items[i].compressed_size   = comp;
        state->items[i].decompressed_size = decomp;
    }

    /* The data section starts at the current file offset. */
    state->data_section_offset = (uint32_t)ftell(state->dat_file);
    state->loaded = true;
    return true;
}

int csb_atari_st_graphics_loader_read_item(const CSB_AtariStLoader* state,
                                            uint16_t index,
                                            uint8_t* out_buf,
                                            size_t out_buf_size)
{
    if (!state || !state->loaded || !state->dat_file || !out_buf) return -1;
    if (index >= state->item_count) return -1;

    const CSB_AtariStItem* item = &state->items[index];
    if (item->compressed_size == 0) {
        /* Empty item: zero out the buffer (if any) and return 0. */
        if (out_buf_size > 0) memset(out_buf, 0, out_buf_size);
        return 0;
    }
    if (out_buf_size < item->decompressed_size) return -1;

    /* Compute the byte offset of this item in the file:
     * data_section_offset + sum(comp[0..index-1]). */
    uint32_t offset = state->data_section_offset;
    for (uint16_t i = 0; i < index; ++i) {
        offset += state->items[i].compressed_size;
    }

    /* Read compressed bytes. */
    uint8_t* comp_buf = (uint8_t*)malloc(item->compressed_size);
    if (!comp_buf) return -1;
    if (fseek(state->dat_file, (long)offset, SEEK_SET) != 0) {
        free(comp_buf);
        return -1;
    }
    if (fread(comp_buf, 1, item->compressed_size, state->dat_file) !=
        item->compressed_size) {
        free(comp_buf);
        return -1;
    }

    /* LZW-decompress using the existing DM1 V1 decoder. */
    M11_GFX_LZWState lzw;
    memset(&lzw, 0, sizeof(lzw));
    int rc = m11_gfx_lzw_decompress(&lzw, comp_buf, item->compressed_size,
                                     out_buf, item->decompressed_size);
    free(comp_buf);
    if (rc <= 0) return -1;

    /* Return the actual decompressed byte count. */
    return (int)item->decompressed_size;
}

void csb_atari_st_graphics_loader_close(CSB_AtariStLoader* state)
{
    if (!state) return;
    if (state->dat_file) {
        fclose(state->dat_file);
        state->dat_file = NULL;
    }
    state->loaded = false;
    state->item_count = 0;
}

/* Self-test: build a tiny synthetic DMCSB1 file with one item
 * containing a known LZW payload, open + read it back, and
 * verify the round-trip. */
static int build_synth_atari_dat(const char* path)
{
    FILE* f = fopen(path, "wb");
    if (!f) return -1;

    /* Build an LZW-encoded "ABCABCABC" payload (9 bytes).
     * We rely on the m11_gfx_lzw_decompress round-trip semantics:
     * we encode the bytes ourselves into LZW codes that the
     * decoder can recover. Simplest: encode a single byte stream
     * that the LZW will turn into literal codes 65/66/67/256/...
     * To avoid re-implementing an LZW encoder here, just use the
     * existing decoder's "literal only" path: write three 9-bit
     * codes (65, 66, 67) and an END_CODE (257).
     *
     * 9-bit code 65 = 0x00041 (bits 0..8 of byte 0..1)
     * 9-bit code 66 = 0x00042
     * 9-bit code 67 = 0x00043
     * 9-bit END_CODE = 0x00101 (257)
     *
     * Packed little-endian bit stream (LSB first per the LZW
     * decoder's read_code logic):
     *  byte 0 bits 0..7: code 65 low 8 bits = 0x41
     *  byte 1 bits 0..0: code 65 bit 8 = 0, code 66 bits 0..7 = 0x42
     *  byte 2 bits 0..0: code 66 bit 8 = 0, code 67 bits 0..7 = 0x43
     *  byte 3 bits 0..0: code 67 bit 8 = 0, code 257 low 8 bits = 0x01
     *  byte 4 bits 0..0: code 257 bit 8 = 1
     *
     * The LZW read_code reads `code_bits` bytes at a time into a
     * chunk and extracts codes from that chunk. With code_bits=9,
     * chunk_bytes=9, chunk_bit_count=72 bits = 8 codes. So we
     * need at least 9 bytes in the input even though we only
     * need 5 bytes of data. Pad with zeros.
     */
    uint8_t lzw_payload[16] = {0};
    lzw_payload[0] = 0x41;  /* code 65 low 8 */
    lzw_payload[1] = 0x42;  /* code 65 bit 8 + code 66 low 7 */
    lzw_payload[2] = 0x43;  /* code 66 bit 8 + code 67 low 7 */
    lzw_payload[3] = 0x01;  /* code 67 bit 8 + END_CODE low 7 */
    lzw_payload[4] = 0x01;  /* END_CODE bit 8 */
    /* Padding to 16 bytes (the chunk reads up to 9 bytes per fill). */

    /* DMCSB1 header: count(u16 BE) + items[count] of { comp(u16 BE), decomp(u16 BE) }. */
    uint16_t count = 1;
    uint8_t count_be[2] = { (uint8_t)(count >> 8), (uint8_t)(count & 0xFF) };
    fwrite(count_be, 2, 1, f);
    uint16_t comp = 16, decomp = 3;
    uint8_t comp_be[2]   = { (uint8_t)(comp   >> 8), (uint8_t)(comp   & 0xFF) };
    uint8_t decomp_be[2] = { (uint8_t)(decomp >> 8), (uint8_t)(decomp & 0xFF) };
    fwrite(comp_be, 2, 1, f);
    fwrite(decomp_be, 2, 1, f);
    /* Data: one item. */
    fwrite(lzw_payload, 1, 16, f);

    fclose(f);
    return 0;
}

int csb_atari_st_graphics_loader_self_test(void)
{
    const char* path = "/tmp/test_csb_atari_st_graphics_loader_synth.dat";
    if (build_synth_atari_dat(path) != 0) return -1;

    CSB_AtariStLoader state;
    csb_atari_st_graphics_loader_init(&state);
    if (!csb_atari_st_graphics_loader_open(&state, path)) {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    if (state.item_count != 1) {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    if (state.items[0].compressed_size != 16 ||
        state.items[0].decompressed_size != 3) {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    uint8_t out[16] = {0};
    int rc = csb_atari_st_graphics_loader_read_item(&state, 0, out, sizeof(out));
    if (rc != 3) {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    if (out[0] != 'A' || out[1] != 'B' || out[2] != 'C') {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    csb_atari_st_graphics_loader_close(&state);
    return 0;
}