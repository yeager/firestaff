/*
 * dm2_v1_gfx_bmp_pc34_compat.c -- DM2 bitmap header and image length.
 *
 * Port of skproject c_gfx_bmp.cpp.
 * Functions: getbmpheader, init_bitmaps, DM2_CALC_IMAGE_BYTE_LENGTH.
 */

#include "dm2_v1_gfx_bmp_pc34_compat.h"
#include <string.h>

/* skproject: getbmpheader() -- negative offset from pixel data to header */
DM2_V1_BmpHeader *dm2_v1_gfx_bmp_get_header(DM2_V1_Bmp *bmp)
{
    return (DM2_V1_BmpHeader *)((uint8_t *)bmp - sizeof(DM2_V1_BmpHeader));
}

/* skproject: init_bitmaps() */
DM2_V1_GfxBmpInitReceipt dm2_v1_gfx_bmp_init(DM2_V1_Screen256Bmp *screen)
{
    DM2_V1_GfxBmpInitReceipt receipt;

    if (screen != NULL) {
        memset(screen, 0, sizeof(DM2_V1_Screen256Bmp));
        receipt.initialized = true;
        receipt.screen_pixel_count = DM2_V1_ORIG_SWIDTH * DM2_V1_ORIG_SHEIGHT;
    } else {
        receipt.initialized = false;
        receipt.screen_pixel_count = 0;
    }

    return receipt;
}

/* skproject: DM2_CALC_IMAGE_BYTE_LENGTH() */
DM2_V1_GfxBmpCalcLengthReceipt dm2_v1_gfx_bmp_calc_image_byte_length(
    DM2_V1_BmpHeader *header)
{
    DM2_V1_GfxBmpCalcLengthReceipt receipt;
    int32_t w;

    receipt.res = header->res;
    receipt.width = header->width;
    receipt.height = header->height;

    if (header->res != DM2_V1_BPP_4) {
        w = (uint16_t)header->width;
    } else {
        w = DM2_V1_MK_EVEN((uint16_t)header->width) >> 1;
    }

    receipt.byte_length = (int32_t)((uint16_t)header->height * (uint16_t)w);

    return receipt;
}

const char *dm2_v1_gfx_bmp_source_evidence(void)
{
    return "skproject c_gfx_bmp.cpp: 3 functions "
           "(getbmpheader, init_bitmaps, DM2_CALC_IMAGE_BYTE_LENGTH)";
}
