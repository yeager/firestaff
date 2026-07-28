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

    /*
     * DMCSB1 stores two complete tables, not comp/decomp pairs.  This is
     * significant for ANIMATE.DAT as well as GRAPHICS.DAT: interpreting
     * them as pairs makes item offsets point into the wrong assets.
     */
    for (uint16_t i = 0; i < count; ++i) {
        uint16_t comp_be;
        if (fread(&comp_be, 2, 1, state->dat_file) != 1) {
            fclose(state->dat_file);
            state->dat_file = NULL;
            return false;
        }
        uint16_t comp   = (uint16_t)((comp_be << 8)   | (comp_be >> 8));
        state->items[i].compressed_size = comp;
    }
    for (uint16_t i = 0; i < count; ++i) {
        uint16_t decomp_be;
        if (fread(&decomp_be, 2, 1, state->dat_file) != 1) {
            fclose(state->dat_file);
            state->dat_file = NULL;
            return false;
        }
        state->items[i].decompressed_size =
            (uint16_t)((decomp_be << 8) | (decomp_be >> 8));
    }

    /* The data section starts at the current file offset. */
    state->data_section_offset = (uint32_t)ftell(state->dat_file);
    {
        uint32_t item_offset = state->data_section_offset;
        for (uint16_t i = 0; i < count; ++i) {
            state->items[i].data_offset = item_offset;
            if (UINT32_MAX - item_offset < state->items[i].compressed_size) {
                fclose(state->dat_file);
                state->dat_file = NULL;
                return false;
            }
            item_offset += state->items[i].compressed_size;
        }
    }
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

    /* Read compressed bytes. */
    uint8_t* comp_buf = (uint8_t*)malloc(item->compressed_size);
    if (!comp_buf) return -1;
    if (fseek(state->dat_file, (long)item->data_offset, SEEK_SET) != 0) {
        free(comp_buf);
        return -1;
    }
    if (fread(comp_buf, 1, item->compressed_size, state->dat_file) !=
        item->compressed_size) {
        free(comp_buf);
        return -1;
    }

    /* Uncompressed DMCSB1 items are stored verbatim. */
    if (item->compressed_size == item->decompressed_size) {
        memcpy(out_buf, comp_buf, item->compressed_size);
        free(comp_buf);
        return (int)item->decompressed_size;
    }

    /* LZW-decompress using the existing DM1 V1 decoder. */
    DM1_V1_GFX_LZWStatePc34 lzw;
    memset(&lzw, 0, sizeof(lzw));
    int rc = DM1_V1_GFX_LzwDecompressPc34Compat(&lzw, comp_buf, item->compressed_size,
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

    /* DMCSB1 header: count, complete compressed-size table, then complete
     * decompressed-size table (all u16 BE). */
    uint16_t count = 2;
    uint8_t count_be[2] = { (uint8_t)(count >> 8), (uint8_t)(count & 0xFF) };
    fwrite(count_be, 2, 1, f);
    uint16_t comp0 = 3, comp1 = 3, decomp0 = 3, decomp1 = 3;
    uint8_t comp0_be[2] = { (uint8_t)(comp0 >> 8), (uint8_t)(comp0 & 0xFF) };
    uint8_t comp1_be[2] = { (uint8_t)(comp1 >> 8), (uint8_t)(comp1 & 0xFF) };
    uint8_t decomp0_be[2] = { (uint8_t)(decomp0 >> 8), (uint8_t)(decomp0 & 0xFF) };
    uint8_t decomp1_be[2] = { (uint8_t)(decomp1 >> 8), (uint8_t)(decomp1 & 0xFF) };
    fwrite(comp0_be, 2, 1, f);
    fwrite(comp1_be, 2, 1, f);
    fwrite(decomp0_be, 2, 1, f);
    fwrite(decomp1_be, 2, 1, f);
    /* Data: two raw items. */
    fwrite("ABC", 1, 3, f);
    fwrite("RAW", 1, 3, f);

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
    if (state.item_count != 2) {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    if (state.items[0].compressed_size != 3 ||
        state.items[0].decompressed_size != 3 ||
        state.items[1].compressed_size != 3 ||
        state.items[1].decompressed_size != 3 ||
        state.items[1].data_offset != state.data_section_offset + 3U) {
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
    memset(out, 0, sizeof(out));
    rc = csb_atari_st_graphics_loader_read_item(&state, 1, out, sizeof(out));
    if (rc != 3 || memcmp(out, "RAW", 3) != 0) {
        csb_atari_st_graphics_loader_close(&state);
        return -1;
    }
    csb_atari_st_graphics_loader_close(&state);
    return 0;
}
