#ifndef DM2_V1_FIRE_BLIT_ROWS_H
#define DM2_V1_FIRE_BLIT_ROWS_H

#include <stddef.h>
#include <stdint.h>

typedef struct DM2_V1_FireBlitRowsReceipt {
    int handled;
    int source_locked;
    int bounds_ok;
    int copied_pixels;
    int skipped_colorkey_pixels;
    const char* symbol;
    const char* source_path;
} DM2_V1_FireBlitRowsReceipt;

void dm2_v1_fire_blit_rows_receipt_clear(DM2_V1_FireBlitRowsReceipt* receipt);

int dm2_v1_fire_blit_to_memory_row_4to4bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t* dst4,
    size_t dst4_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    DM2_V1_FireBlitRowsReceipt* out_receipt);

int dm2_v1_fire_blit_to_memory_row_4to8bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t* dst8,
    size_t dst8_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    const uint8_t palette16[16],
    int colorkey,
    DM2_V1_FireBlitRowsReceipt* out_receipt);

int dm2_v1_fire_stretch_blit_to_memory_4to4bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint16_t src_stride_pixels,
    uint16_t src_width_pixels,
    uint16_t src_height_pixels,
    uint8_t* dst4,
    size_t dst4_size,
    uint16_t off_dst_pixels,
    uint16_t dst_stride_pixels,
    uint16_t dst_width_pixels,
    uint16_t dst_height_pixels,
    DM2_V1_FireBlitRowsReceipt* out_receipt);

int dm2_v1_ibmio_load_4to8bpp_pal(
    const uint8_t* pal16,
    size_t pal16_size,
    uint8_t out_palette16[16],
    DM2_V1_FireBlitRowsReceipt* out_receipt);

int dm2_v1_ibmio_blit_row_4to8bpp(
    const uint8_t* src4,
    size_t src4_size,
    uint16_t off_src_pixels,
    uint8_t* dst8,
    size_t dst8_size,
    uint16_t off_dst_pixels,
    uint16_t width_pixels,
    const uint8_t palette16[16],
    DM2_V1_FireBlitRowsReceipt* out_receipt);

const char* dm2_v1_fire_blit_rows_source_evidence(void);

#endif
