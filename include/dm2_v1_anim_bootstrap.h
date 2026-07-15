#ifndef FIRESTAFF_DM2_V1_ANIM_BOOTSTRAP_H
#define FIRESTAFF_DM2_V1_ANIM_BOOTSTRAP_H

#include <stddef.h>
#include <stdint.h>

#define DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS 8

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    int argc;
    const char *argv[DM2_V1_ANIM_BOOTSTRAP_MAX_ARGS];
    char command_line[96];
} DM2_V1_AnimBootstrapReceipt;

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    uint16_t width;
    uint16_t height;
    uint16_t even_width;
    uint32_t decoded_pixels;
    uint32_t consumed_bytes;
    uint32_t fill_run_count;
    uint32_t literal_run_count;
    uint32_t skip_run_count;
    uint32_t previous_row_run_count;
} DM2_V1_AnimDecodeImg1Receipt;

int dm2_v1_anim_bootstrap_swoosh(DM2_V1_AnimBootstrapReceipt *out_receipt);
int dm2_v1_anim_bootstrap_title(DM2_V1_AnimBootstrapReceipt *out_receipt);

int dm2_v1_anim_setpixel_seq_4bpp(uint8_t *dst,
                                  size_t dst_size,
                                  uint16_t pixel_offset,
                                  uint8_t color);
int dm2_v1_anim_fill_seq_4bpp(uint8_t *dst,
                              size_t dst_size,
                              uint16_t pixel_offset,
                              uint8_t color,
                              uint16_t count);
int dm2_v1_anim_decode_img1(const uint8_t *src,
                            size_t src_size,
                            uint8_t *dst,
                            size_t dst_size,
                            DM2_V1_AnimDecodeImg1Receipt *out_receipt);

#endif
