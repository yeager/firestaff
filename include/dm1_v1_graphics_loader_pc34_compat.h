/* DM1 V1 Graphics/Bitmap Loader — source-locked from ReDMCSB
 * GRF1.C, EXPAND.C, LZW.C, IMAGE.C
 * LZW.C: 12-bit LZW decompression (G0666 max_code=4096)
 * F0495_GetNextInputCode, F0455_DecompressDungeon pattern */
#ifndef FIRESTAFF_DM1_V1_GRAPHICS_LOADER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GRAPHICS_LOADER_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_GFX_MAX_BITMAPS     600
#define DM1_GFX_LZW_MAX_CODE   4096
#define DM1_GFX_LZW_MAX_BITS   12
#define DM1_GFX_LZW_CLEAR_CODE 256
#define DM1_GFX_LZW_END_CODE   257
#define DM1_GFX_LZW_FIRST_CODE 258

/* Bitmap header as stored in GRAPHICS.DAT index */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t compressed_size;
    uint32_t offset;        /* file offset to compressed data */
} DM1_V1_GFX_BitmapHeaderPc34;

/* Decoded bitmap */
typedef struct {
    uint8_t* data;          /* planar or chunky pixel data */
    uint16_t width;
    uint16_t height;
    uint16_t byte_width;    /* (width + 7) / 8 per bitplane */
    bool     allocated;
} DM1_V1_GFX_BitmapPc34;

/* LZW decompressor state — ReDMCSB LZW.C pattern */
typedef struct {
    uint16_t dict_prefix[DM1_GFX_LZW_MAX_CODE];
    uint8_t  dict_append[DM1_GFX_LZW_MAX_CODE];
    uint8_t  decode_stack[DM1_GFX_LZW_MAX_CODE];
    uint16_t next_code;
    uint8_t  code_bits;
    bool     flushed;
    uint8_t  chunk[12];
    int      chunk_bit_idx;
    int      chunk_bit_count;
    size_t   byte_pos;
    int      needs_refill;
} DM1_V1_GFX_LZWStatePc34;

/* Graphics loader state */
typedef struct {
    FILE*               dat_file;
    DM1_V1_GFX_BitmapHeaderPc34 headers[DM1_GFX_MAX_BITMAPS];
    uint16_t            bitmap_count;
    DM1_V1_GFX_LZWStatePc34   lzw;
    bool                loaded;
} DM1_V1_GFX_LoaderStatePc34;

void DM1_V1_GFX_InitPc34Compat(DM1_V1_GFX_LoaderStatePc34* state);
bool DM1_V1_GFX_OpenDatPc34Compat(DM1_V1_GFX_LoaderStatePc34* state, const char* path);
bool DM1_V1_GFX_LoadBitmapPc34Compat(DM1_V1_GFX_LoaderStatePc34* state, uint16_t index,
                          DM1_V1_GFX_BitmapPc34* out);
void DM1_V1_GFX_FreeBitmapPc34Compat(DM1_V1_GFX_BitmapPc34* bmp);
int  DM1_V1_GFX_LzwDecompressPc34Compat(DM1_V1_GFX_LZWStatePc34* lzw,
                             const uint8_t* input, size_t in_size,
                             uint8_t* output, size_t out_size);
void DM1_V1_GFX_ClosePc34Compat(DM1_V1_GFX_LoaderStatePc34* state);

typedef DM1_V1_GFX_BitmapHeaderPc34 M11_GFX_BitmapHeader;
typedef DM1_V1_GFX_BitmapPc34 M11_GFX_Bitmap;
typedef DM1_V1_GFX_LZWStatePc34 M11_GFX_LZWState;
typedef DM1_V1_GFX_LoaderStatePc34 M11_GFX_LoaderState;

#define m11_gfx_init DM1_V1_GFX_InitPc34Compat
#define m11_gfx_open_dat DM1_V1_GFX_OpenDatPc34Compat
#define m11_gfx_load_bitmap DM1_V1_GFX_LoadBitmapPc34Compat
#define m11_gfx_free_bitmap DM1_V1_GFX_FreeBitmapPc34Compat
#define m11_gfx_lzw_decompress DM1_V1_GFX_LzwDecompressPc34Compat
#define m11_gfx_close DM1_V1_GFX_ClosePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GRAPHICS_LOADER_PC34_COMPAT_H */
