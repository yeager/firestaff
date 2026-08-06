#ifndef DM1_V1_FMTOWNS_PIC_LIBRARY_H
#define DM1_V1_FMTOWNS_PIC_LIBRARY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 picture-library container decoder.
 *
 * The FM Towns DM1 CD-ROM ships every graphic (viewport tiles,
 * champion portraits, item glyphs, menu font, splash screens, ...)
 * inside a single file `DATA/GRAPHICS.DAT`. The runtime loads that
 * file through `INIT_PIC_LIB` (EDM.EXP 0x92f8), keeps it open with
 * a refcount via `OPEN_PIC_LIB` (0x9290) / `CLOSE_PIC_LIB` (0x92d0),
 * reads a compressed asset span with the helper at 0x901c
 * (`READ_ASSET`), and decodes it through `_DECODE` (0x1f940) into
 * either the caller's buffer or the shared DUNBUF staging area
 * (0x29590). The pixel-level decoder is `DECODEGRAPHIC` (0x1f63c).
 *
 * INDEX_TO_FILE_OFFSET (0x8d04) computes the byte offset of asset
 * `i` in the on-disk file as:
 *     offset(i) = 2 + count*4 + sum(size_table[0..i-1])
 * That formula pins the container header down to three fields:
 *
 *   +0      : u16 asset_count            (little-endian)
 *   +2      : u16 size_table_primary[count]   (per-asset byte size)
 *   +2+2N   : u16 size_table_secondary[count] (mirror of primary)
 *   +2+4N   : raw payload, N asset spans back-to-back
 *
 * `INIT_PIC_LIB` reads BOTH tables from the file and then calls
 * memcpy(0x1c9a4) with (primary, secondary, count*2) — so the
 * runtime always uses the secondary table as authoritative. In
 * every shipped English FM Towns `GRAPHICS.DAT` we have hash-
 * verified the two tables are byte-identical, so either table
 * decodes the container correctly.
 *
 * Every asset span begins with a 4-byte `DECODEGRAPHIC` header:
 *   +0 : u16 width_pixels    (may be padded up to a multiple of 32)
 *   +2 : u16 height_pixels
 * The remaining bytes are either raw 4-bpp pixel rows (when
 * `width_pixels` already equals `padded_width`, which triggers the
 * fast-copy path at DECODEGRAPHIC 0x1f85f) or an RLE-compressed
 * stream (the loop starting at 0x1f6a0). This decoder ships the
 * container parse + header inspection; the RLE decoder is the next
 * step and lives outside this module.
 *
 * Evidence: parity-evidence/dm1_fmtowns_pic_library_format.md
 */

/* Header field sizes in bytes, byte-verified from INDEX_TO_FILE_OFFSET
 * (EDM.EXP 0x8d04) which starts its offset accumulator at `count*4+2`
 * — the fixed prefix is exactly six bytes plus two per-asset u16
 * tables. */
#define DM1_V1_FMTOWNS_PIC_LIB_COUNT_BYTES         2u
#define DM1_V1_FMTOWNS_PIC_LIB_SIZE_ENTRY_BYTES    2u
#define DM1_V1_FMTOWNS_PIC_LIB_TABLE_COUNT         2u  /* primary + secondary */

/* Per-asset DECODEGRAPHIC header (EDM.EXP 0x1f656) is 4 bytes. */
#define DM1_V1_FMTOWNS_PIC_LIB_ASSET_HEADER_BYTES  4u

/* Container-level errors returned by the parse helpers. */
typedef enum {
    DM1_V1_FMTOWNS_PIC_LIB_OK                = 0,
    DM1_V1_FMTOWNS_PIC_LIB_ERR_NULL          = 1,
    DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED     = 2,
    DM1_V1_FMTOWNS_PIC_LIB_ERR_INDEX         = 3,
    DM1_V1_FMTOWNS_PIC_LIB_ERR_ASSET_HEADER  = 4
} dm1_v1_fmtowns_pic_library_status_t;

/* View onto a picture-library file that already lives in a
 * caller-owned buffer. No dynamic allocation, no I/O, no globals. */
typedef struct {
    const uint8_t *data;          /* pointer to byte 0 of the file */
    size_t         data_size;     /* total container byte count */
    uint16_t       asset_count;   /* value at file offset 0 */
    const uint8_t *size_table_primary;    /* file offset 2 */
    const uint8_t *size_table_secondary;  /* file offset 2 + count*2 */
    size_t         payload_offset;        /* first payload byte */
} dm1_v1_fmtowns_pic_library_view_t;

/* Zero-copy view over an already-loaded picture-library file. Reads
 * the count word, splits the two size tables, and locates the
 * payload. Does not touch any asset bytes. Returns
 * DM1_V1_FMTOWNS_PIC_LIB_OK on success. */
dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_open_pc34(
    const uint8_t *data,
    size_t         data_size,
    dm1_v1_fmtowns_pic_library_view_t *out_view);

/* Return the byte size of asset `index` as stored in the primary
 * size table (the same table `READ_ASSET` at 0x901c consults). */
dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_asset_size_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t  index,
    uint16_t *out_size_bytes);

/* Return the file byte offset of asset `index`, computed with the
 * exact formula from EDM.EXP 0x8d04. */
dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_asset_offset_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t  index,
    size_t   *out_file_offset);

/* Return a read-only pointer to asset `index` inside the buffer.
 * On success `*out_ptr` points at the asset's first byte and
 * `*out_size_bytes` is filled in with the primary-table size. */
dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_asset_bytes_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t         index,
    const uint8_t **out_ptr,
    uint16_t        *out_size_bytes);

/* Return non-zero iff the two on-disk size tables are byte-identical
 * for every asset. Every hash-verified English shipped `GRAPHICS.DAT`
 * satisfies this. */
int
dm1_v1_fmtowns_pic_library_tables_are_mirror_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view);

/* DECODEGRAPHIC header (EDM.EXP 0x1f63c prologue). */
typedef struct {
    uint16_t width_pixels;    /* file offset asset+0 */
    uint16_t height_pixels;   /* file offset asset+2 */
    uint16_t padded_width_pixels;  /* (width + 0x1f) & ~0x1f */
    uint16_t row_bytes;            /* padded_width / 2 (4 bpp) */
    uint32_t decoded_total_bytes;  /* row_bytes * height */
    int      is_uncompressed;      /* padded_width == width */
} dm1_v1_fmtowns_pic_library_gfx_header_t;

/* Parse the 4-byte DECODEGRAPHIC header at the start of an asset
 * and compute the derived fields DECODEGRAPHIC uses at 0x1f665..
 * 0x1f68c. `asset_bytes` must be at least
 * DM1_V1_FMTOWNS_PIC_LIB_ASSET_HEADER_BYTES bytes long. */
dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(
    const uint8_t *asset_bytes,
    size_t         asset_size,
    dm1_v1_fmtowns_pic_library_gfx_header_t *out_header);

/* Compute `padded_width = (width + 0x1f) & ~0x1f` — the exact
 * arithmetic DECODEGRAPHIC uses at EDM.EXP 0x1f665. */
uint16_t
dm1_v1_fmtowns_pic_library_padded_width_pc34(uint16_t width_pixels);

/* Compute `row_bytes = padded_width / 2` (4 bits per pixel). */
uint16_t
dm1_v1_fmtowns_pic_library_row_bytes_pc34(uint16_t width_pixels);

/* GET_MY_DECODED (EDM.EXP 0x9f04) direct-copy path: for a raw
 * uncompressed asset (like the menu font, index 557), copy the
 * stored byte span verbatim into a caller buffer. Refuses to copy
 * a size that does not match the primary-table entry. */
dm1_v1_fmtowns_pic_library_status_t
dm1_v1_fmtowns_pic_library_load_raw_asset_pc34(
    const dm1_v1_fmtowns_pic_library_view_t *view,
    uint16_t  index,
    uint8_t  *dst_buffer,
    size_t    dst_capacity,
    size_t   *out_bytes_written);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_PIC_LIBRARY_H */
