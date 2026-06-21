/*
 * csb_v1_graphics_atari_st_loader_pc34_compat.h
 *
 * Atari ST Chaos Strikes Back GRAPHICS.DAT item loader.
 *
 * The Atari ST CSB GRAPHICS.DAT uses the DMCSB1 file format
 * (per dmweb Data Files page):
 *   - Big-endian (Atari ST is big-endian)
 *   - 563 items
 *   - 1 word (u16 BE): number of items
 *   - 1 word per item (u16 BE): compressed byte count
 *   - 1 word per item (u16 BE): decompressed byte count
 *   - Data section: items concatenated, each compressed_size bytes
 *   - Items 558-562 are LZW-compressed (the Atari ST HIDDEN CODE
 *     range that gets excluded from integrity checksums); the
 *     remaining items may use IMG1/IMG2/IMG3 encodings.
 *
 * This loader does NOT decode the IMG1/IMG2/IMG3 image formats
 * -- that is a Tier 3 follow-up. The loader does:
 *
 *   1. Parse the DMCSB1 header (count + comp/decomp pairs)
 *   2. LZW-decompress each item's data via the existing
 *      m11_gfx_lzw_decompress() from the DM1 V1 graphics loader
 *   3. Return the decompressed byte buffer for each item
 *
 * The decompressed buffer for items 21/538/548 (Atari ST CSB
 * hidden code) will contain 68k machine code. Callers must
 * consult csb_v1_graphics_hidden_should_skip_item() before
 * blitting the buffer into a framebuffer.
 *
 * Source-lock:
 *   - dmweb Data Files page: GRAPHICS.DAT CSB Atari ST v2.0/v2.1
 *     is BIG-endian DMCSB1 with 563 items, LZW-compressed.
 *   - ReDMCSB LZW.C F0497_LZW_Decompress (the actual LZW decoder).
 *   - ReDMCSB GRAPH21.C F0914 / GRAPH538.C F0915 / GRAPH548.C
 *     F0916 (the hidden-code 68k payloads for items 21/538/548).
 *   - dmweb Meynaf disassembly of CSB Atari ST hidden-code items.
 */
#ifndef FIRESTAFF_CSB_V1_GRAPHICS_ATARI_ST_LOADER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_GRAPHICS_ATARI_ST_LOADER_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_ATARI_ST_GRAPHICS_MAX_ITEMS 700

typedef struct {
    uint16_t compressed_size;
    uint16_t decompressed_size;
    uint32_t data_offset;  /* offset into the file where the item's compressed bytes start */
} CSB_AtariStItem;

typedef struct {
    FILE*               dat_file;
    char                dat_path[1024];
    uint16_t            item_count;
    CSB_AtariStItem     items[CSB_ATARI_ST_GRAPHICS_MAX_ITEMS];
    uint32_t            data_section_offset;
    bool                loaded;
} CSB_AtariStLoader;

/* Initialize the loader state. */
void csb_atari_st_graphics_loader_init(CSB_AtariStLoader* state);

/* Open + parse a DMCSB1-format GRAPHICS.DAT file. Returns true on success. */
bool csb_atari_st_graphics_loader_open(CSB_AtariStLoader* state, const char* path);

/* LZW-decompress item `index` into `out_buf` (sized to
 * items[index].decompressed_size bytes). Returns the number of
 * bytes written, or -1 on failure.
 *
 * NOTE: For hidden-code items (21/538/548 on Atari ST CSB), the
 * returned buffer contains 68k machine code. Do NOT blit it as
 * image data; consult csb_v1_graphics_hidden_should_skip_item(). */
int csb_atari_st_graphics_loader_read_item(const CSB_AtariStLoader* state,
                                            uint16_t index,
                                            uint8_t* out_buf,
                                            size_t out_buf_size);

/* Close + free the loader. */
void csb_atari_st_graphics_loader_close(CSB_AtariStLoader* state);

/* Self-test: parse a synthetic DMCSB1 file and verify LZW
 * round-trip on a single item. Returns 0 on success, -1 on
 * failure. */
int csb_atari_st_graphics_loader_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_GRAPHICS_ATARI_ST_LOADER_PC34_COMPAT_H */