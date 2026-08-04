/* skproject: c_gfx_bmp.h / c_gfx_bmp.cpp */
#ifndef FIRESTAFF_DM2_V1_GFX_BMP_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GFX_BMP_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include "dm2_v1_gfx_pixel_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Screen dimensions (skproject dm2global.h) --- */
#define DM2_V1_ORIG_SWIDTH  320
#define DM2_V1_ORIG_SHEIGHT 200

/* --- MK_EVEN: round up to even (skproject util.h) --- */
#define DM2_V1_MK_EVEN(x) (((x) + 1) & (int32_t)0xFFFFFFFEL)

/* --- s_dm2bmpheader: 6-byte packed bitmap header preceding pixel data --- */
#pragma pack(push, 1)
typedef struct {
    DM2_V1_Resolution res;   /* BPP_4 or BPP_8 */
    int8_t unused;
    int16_t width;
    int16_t height;
} DM2_V1_BmpHeader;
#pragma pack(pop)

/* --- t_bmp: pixel pointer type (header at negative offset) --- */
typedef DM2_V1_Pixel DM2_V1_Bmp;

/* --- s_screen256bmp: full 320x200 8bpp screen buffer --- */
typedef struct {
    DM2_V1_BmpHeader header;
    DM2_V1_Pixel256 pixel[DM2_V1_ORIG_SWIDTH * DM2_V1_ORIG_SHEIGHT];
} DM2_V1_Screen256Bmp;

/* --- Receipt: observable output from init_bitmaps --- */
typedef struct {
    bool initialized;
    int32_t screen_pixel_count;
} DM2_V1_GfxBmpInitReceipt;

/* --- Receipt: observable output from calc_image_byte_length --- */
typedef struct {
    int32_t byte_length;
    DM2_V1_Resolution res;
    int16_t width;
    int16_t height;
} DM2_V1_GfxBmpCalcLengthReceipt;

/* --- Functions --- */

/**
 * Get the bitmap header from a t_bmp pointer.
 * The header is stored at a negative offset before the pixel data.
 * skproject: getbmpheader()
 */
DM2_V1_BmpHeader *dm2_v1_gfx_bmp_get_header(DM2_V1_Bmp *bmp);

/**
 * Initialize the screen256 bitmap buffer to zero.
 * skproject: init_bitmaps()
 */
DM2_V1_GfxBmpInitReceipt dm2_v1_gfx_bmp_init(DM2_V1_Screen256Bmp *screen);

/**
 * Calculate the byte length of bitmap pixel data.
 * For 4bpp: width is rounded up to even then halved.
 * For 8bpp: width is used directly.
 * Result = height * adjusted_width.
 * skproject: DM2_CALC_IMAGE_BYTE_LENGTH()
 */
DM2_V1_GfxBmpCalcLengthReceipt dm2_v1_gfx_bmp_calc_image_byte_length(
    DM2_V1_BmpHeader *header);

/**
 * Source evidence string for parity tracking.
 */
const char *dm2_v1_gfx_bmp_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GFX_BMP_PC34_COMPAT_H */
