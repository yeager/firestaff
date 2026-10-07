/*
 * csb_v1_graphics_atari_st_loader_pc34_compat.c
 *
 * Atari ST Chaos Strikes Back GRAPHICS.DAT item loader.
 * See header for the full contract.
 */

#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"
#include "csb_v1_graphics_lzw_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CSB_ATARI_ST_GRAPHICS_PATH_CAPACITY = 1024 };

/* Atari rendering opens GRAPHICS.DAT through this loader from several
 * source-owned layout decoders. When the media is nested in a 7z archive,
 * reopening it on every frame repeats the full LZMA decode. Retain one
 * authenticated in-memory source image for the currently selected path;
 * each short-lived loader still owns its own copy and can close normally. */
static char g_csb_atari_st_graphics_cached_path[
    CSB_ATARI_ST_GRAPHICS_PATH_CAPACITY];
static uint8_t *g_csb_atari_st_graphics_cached_bytes;
static size_t g_csb_atari_st_graphics_cached_size;

void csb_atari_st_graphics_loader_init(CSB_AtariStLoader* state)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

static bool csb_atari_st_graphics_loader_parse_loaded(CSB_AtariStLoader* state)
{
    size_t offset = 0u;
    if (!state || !state->dat_bytes || state->dat_byte_count < 2u) return false;

    /* Read count (u16 big-endian). */
    uint16_t count_be = (uint16_t)(((uint16_t)state->dat_bytes[offset] << 8) |
                                   state->dat_bytes[offset + 1u]);
    offset += 2u;
    /* Manual byte-swap to big-endian if the host is little-endian. */
    uint16_t count = count_be;
    if (count == 0 || count > CSB_ATARI_ST_GRAPHICS_MAX_ITEMS) {
        free(state->dat_bytes);
        state->dat_bytes = NULL;
        state->dat_byte_count = 0u;
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
        if (offset + 2u > state->dat_byte_count) {
            free(state->dat_bytes);
            state->dat_bytes = NULL;
            state->dat_byte_count = 0u;
            return false;
        }
        comp_be = (uint16_t)(((uint16_t)state->dat_bytes[offset] << 8) |
                             state->dat_bytes[offset + 1u]);
        offset += 2u;
        uint16_t comp = comp_be;
        state->items[i].compressed_size = comp;
    }
    for (uint16_t i = 0; i < count; ++i) {
        uint16_t decomp_be;
        if (offset + 2u > state->dat_byte_count) {
            free(state->dat_bytes);
            state->dat_bytes = NULL;
            state->dat_byte_count = 0u;
            return false;
        }
        decomp_be = (uint16_t)(((uint16_t)state->dat_bytes[offset] << 8) |
                               state->dat_bytes[offset + 1u]);
        offset += 2u;
        state->items[i].decompressed_size = decomp_be;
    }

    /* The data section starts at the current file offset. */
    state->data_section_offset = (uint32_t)offset;
    {
        uint32_t item_offset = state->data_section_offset;
        for (uint16_t i = 0; i < count; ++i) {
            state->items[i].data_offset = item_offset;
            if (UINT32_MAX - item_offset < state->items[i].compressed_size ||
                (size_t)item_offset + state->items[i].compressed_size >
                    state->dat_byte_count) {
                free(state->dat_bytes);
                state->dat_bytes = NULL;
                state->dat_byte_count = 0u;
                return false;
            }
            item_offset += state->items[i].compressed_size;
        }
    }
    state->loaded = true;
    return true;
}

bool csb_atari_st_graphics_loader_open(CSB_AtariStLoader* state, const char* path)
{
    uint8_t *fresh_bytes = NULL;
    uint8_t *candidate_cache = NULL;
    size_t fresh_byte_count = 0u;
    bool cache_hit;
    bool parsed;

    if (!state || !path) return false;

    /* A preserved Atari package can be ZIP -> ZIP -> STX -> GRAPHICS.DAT.
     * The simple path reader intentionally accepts only one archive member;
     * use the native virtual-media reader for the full authenticated chain.
     * Both routes keep source bytes in process memory and never extract a
     * replacement GRAPHICS.DAT to disk. */
    cache_hit = g_csb_atari_st_graphics_cached_bytes &&
        strcmp(g_csb_atari_st_graphics_cached_path, path) == 0;
    if (cache_hit) {
        state->dat_bytes = (uint8_t *)malloc(g_csb_atari_st_graphics_cached_size);
        if (!state->dat_bytes) return false;
        memcpy(state->dat_bytes, g_csb_atari_st_graphics_cached_bytes,
               g_csb_atari_st_graphics_cached_size);
        state->dat_byte_count = g_csb_atari_st_graphics_cached_size;
    } else {
        if (!(strstr(path, "::")
                  ? asset_read_virtual_path_alloc(path, &fresh_bytes,
                                                   &fresh_byte_count)
                  : asset_read_path_alloc(path, &fresh_bytes,
                                          &fresh_byte_count))) return false;
        state->dat_bytes = fresh_bytes;
        state->dat_byte_count = fresh_byte_count;

        if (strlen(path) < sizeof(g_csb_atari_st_graphics_cached_path)) {
            candidate_cache = (uint8_t *)malloc(fresh_byte_count);
            if (candidate_cache) {
                memcpy(candidate_cache, fresh_bytes, fresh_byte_count);
            }
        }
    }

    strncpy(state->dat_path, path, sizeof(state->dat_path) - 1);
    state->dat_path[sizeof(state->dat_path) - 1] = '\0';
    parsed = csb_atari_st_graphics_loader_parse_loaded(state);
    if (!cache_hit && parsed && candidate_cache) {
        free(g_csb_atari_st_graphics_cached_bytes);
        g_csb_atari_st_graphics_cached_bytes = candidate_cache;
        g_csb_atari_st_graphics_cached_size = fresh_byte_count;
        snprintf(g_csb_atari_st_graphics_cached_path,
                 sizeof(g_csb_atari_st_graphics_cached_path), "%s", path);
    } else {
        free(candidate_cache);
    }
    return parsed;
}

bool csb_atari_st_graphics_loader_open_bytes(CSB_AtariStLoader* state,
                                              const uint8_t* bytes,
                                              size_t byte_count)
{
    if (!state || !bytes || byte_count < 2u) return false;
    csb_atari_st_graphics_loader_close(state);
    state->dat_bytes = (uint8_t*)malloc(byte_count);
    if (!state->dat_bytes) return false;
    memcpy(state->dat_bytes, bytes, byte_count);
    state->dat_byte_count = byte_count;
    strncpy(state->dat_path, "<memory>", sizeof(state->dat_path) - 1);
    state->dat_path[sizeof(state->dat_path) - 1] = '\0';
    return csb_atari_st_graphics_loader_parse_loaded(state);
}

int csb_atari_st_graphics_loader_read_item(const CSB_AtariStLoader* state,
                                            uint16_t index,
                                            uint8_t* out_buf,
                                            size_t out_buf_size)
{
    if (!state || !state->loaded || !state->dat_bytes || !out_buf) return -1;
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
    if ((size_t)item->data_offset + item->compressed_size >
        state->dat_byte_count) {
        free(comp_buf);
        return -1;
    }
    memcpy(comp_buf, state->dat_bytes + item->data_offset,
           item->compressed_size);

    /* Uncompressed DMCSB1 items are stored verbatim. */
    if (item->compressed_size == item->decompressed_size) {
        memcpy(out_buf, comp_buf, item->compressed_size);
        free(comp_buf);
        return (int)item->decompressed_size;
    }

    /* CSB's Atari DMCSB1 records use the Graphics.cpp LZW stream, including
     * its source RLE escape handling. The DM1 reader happens to accept many
     * table shapes but produces a different byte stream for C001-C005. */
    size_t written = 0u;
    int rc = csb_v1_graphics_lzw_decode_pc34_compat(
        comp_buf, item->compressed_size, out_buf, item->decompressed_size,
        &written);
    free(comp_buf);
    if (rc != 0 || written != item->decompressed_size) return -1;

    /* Return the actual decompressed byte count. */
    return (int)item->decompressed_size;
}

void csb_atari_st_graphics_loader_close(CSB_AtariStLoader* state)
{
    if (!state) return;
    free(state->dat_bytes);
    state->dat_bytes = NULL;
    state->dat_byte_count = 0u;
    state->loaded = false;
    state->item_count = 0;
}
