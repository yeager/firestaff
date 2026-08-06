#ifndef DM1_V1_FMTOWNS_FONT_ASSET_H
#define DM1_V1_FMTOWNS_FONT_ASSET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 font-asset identity, recovered by
 * decoding INIT_TEXT (EDM.EXP 0x1ae54) and GET_MY_DECODED
 * (EDM.EXP 0x9f04). Every value below is byte-verified from the
 * hash-verified HMA-240 English EDM.EXP.
 *
 * Evidence:
 *   parity-evidence/dm1_fmtowns_font_asset.md
 *   parity-evidence/dm1_fmtowns_text_rasteriser.md
 *
 * The compressed font raster is NOT inline in the executable; it
 * is decoded on demand from the FM Towns picture library via
 * GET_MY_DECODED. This module encodes the exact loader arguments
 * INIT_TEXT uses so a firestaff-side loader can request the same
 * asset without re-lifting them from the executable.
 */

/* Byte size of the decoded font raster allocation.
 * INIT_TEXT at EDM.EXP 0x1ae74:
 *   push 0x300 ; push 1 ; call PUSH_MEM ; store into TEXT_PIC. */
#define DM1_V1_FMTOWNS_FONT_RASTER_ALLOC_BYTES 0x300u  /* 768 */

/* Full 32-bit picture-library asset id INIT_TEXT passes to
 * GET_MY_DECODED as its first argument (0xffffc22d). GET_MY_DECODED
 * inspects the low 16 bits `di`:
 *   di & 0x8000  -> use caller's buffer directly (no DUNBUF stage)
 *   di & 0x4000  -> skip the [esi-4] size-header decoration
 *   di & 0x3fff  -> picture-library index
 *
 * The full dword the caller passes has all upper bytes set to 0xff
 * because Metaware High-C sign-extends the constant -0x3dd3 into a
 * 32-bit argument slot; only the low word is consumed. */
#define DM1_V1_FMTOWNS_FONT_MY_DECODED_ARG    0xffffc22du

/* The three fields the loader extracts from the low 16 bits. */
#define DM1_V1_FMTOWNS_FONT_MY_DECODED_ID_MASK       0x3fffu
#define DM1_V1_FMTOWNS_FONT_MY_DECODED_DIRECT_MASK   0x8000u
#define DM1_V1_FMTOWNS_FONT_MY_DECODED_NO_HDR_MASK   0x4000u

/* Byte-verified extraction from EDM.EXP:
 *   0xc22d & 0x3fff = 0x022d = 557  (picture-library index)
 *   0xc22d & 0x8000 = 0x8000        (direct-to-buffer decode)
 *   0xc22d & 0x4000 = 0x4000        (skip size header) */
#define DM1_V1_FMTOWNS_FONT_PIC_LIB_INDEX       557u
#define DM1_V1_FMTOWNS_FONT_DIRECT_TO_BUFFER    1
#define DM1_V1_FMTOWNS_FONT_SKIP_SIZE_HEADER    1

/* Return the picture-library index for the FM Towns DM1 menu font.
 * Always returns DM1_V1_FMTOWNS_FONT_PIC_LIB_INDEX. */
uint16_t dm1_v1_fmtowns_font_pic_library_index_pc34(void);

/* Given the encoded 32-bit `GET_MY_DECODED` argument that INIT_TEXT
 * passes, extract the actual picture-library index (low 14 bits of
 * the low word). Returns 0 for an obviously-invalid encoding. */
uint16_t dm1_v1_fmtowns_font_decode_pic_index_pc34(uint32_t my_decoded_arg);

/* Test bit 15 (direct-to-caller-buffer flag). */
int dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(uint32_t my_decoded_arg);

/* Test bit 14 (skip-size-header flag). */
int dm1_v1_fmtowns_font_is_no_size_header_pc34(uint32_t my_decoded_arg);

/* Return the byte size of the caller-owned buffer INIT_TEXT
 * allocates via PUSH_MEM before calling GET_MY_DECODED. */
size_t   dm1_v1_fmtowns_font_raster_alloc_bytes_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_FONT_ASSET_H */
