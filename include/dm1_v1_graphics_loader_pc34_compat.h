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
} M11_GFX_BitmapHeader;

/* Decoded bitmap */
typedef struct {
    uint8_t* data;          /* planar or chunky pixel data */
    uint16_t width;
    uint16_t height;
    uint16_t byte_width;    /* (width + 7) / 8 per bitplane */
    bool     allocated;
} M11_GFX_Bitmap;

/* LZW decompressor state — ReDMCSB LZW.C pattern */
typedef struct {
    uint16_t dict_prefix[DM1_GFX_LZW_MAX_CODE];
    uint8_t  dict_append[DM1_GFX_LZW_MAX_CODE];
    uint8_t  decode_stack[DM1_GFX_LZW_MAX_CODE];
    uint16_t next_code;
    uint8_t  code_bits;
    bool     flushed;
} M11_GFX_LZWState;

/* Graphics loader state */
typedef struct {
    FILE*               dat_file;
    M11_GFX_BitmapHeader headers[DM1_GFX_MAX_BITMAPS];
    uint16_t            bitmap_count;
    M11_GFX_LZWState   lzw;
    bool                loaded;
} M11_GFX_LoaderState;

void m11_gfx_init(M11_GFX_LoaderState* state);
bool m11_gfx_open_dat(M11_GFX_LoaderState* state, const char* path);
bool m11_gfx_load_bitmap(M11_GFX_LoaderState* state, uint16_t index,
                          M11_GFX_Bitmap* out);
void m11_gfx_free_bitmap(M11_GFX_Bitmap* bmp);
int  m11_gfx_lzw_decompress(M11_GFX_LZWState* lzw,
                             const uint8_t* input, size_t in_size,
                             uint8_t* output, size_t out_size);
void m11_gfx_close(M11_GFX_LoaderState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GRAPHICS_LOADER_PC34_COMPAT_H */
