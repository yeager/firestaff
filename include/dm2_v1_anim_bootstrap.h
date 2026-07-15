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

typedef struct {
    int valid;
    const char *symbol;
    const char *source_file;
    int source_line;
    int handle;
    uint32_t requested_bytes;
    uint32_t transferred_bytes;
    uint32_t chunk_count;
    uint32_t file_size;
} DM2_V1_AnimFileReceipt;

int dm2_v1_anim_bootstrap_swoosh(DM2_V1_AnimBootstrapReceipt *out_receipt);
int dm2_v1_anim_bootstrap_title(DM2_V1_AnimBootstrapReceipt *out_receipt);

int dm2_v1_anim_file_open(const char *filename,
                          DM2_V1_AnimFileReceipt *out_receipt);
uint32_t dm2_v1_anim_get_file_size(int handle,
                                   DM2_V1_AnimFileReceipt *out_receipt);
int dm2_v1_anim_read_huge_file(int handle,
                               uint32_t read_size,
                               uint8_t *buffer,
                               DM2_V1_AnimFileReceipt *out_receipt);
void dm2_v1_anim_file_close(int handle,
                            DM2_V1_AnimFileReceipt *out_receipt);
char *dm2_v1_anim_strcpy(char *dst,
                         const char *src,
                         DM2_V1_AnimFileReceipt *out_receipt);
int dm2_v1_anim_toupper(int value,
                        DM2_V1_AnimFileReceipt *out_receipt);

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
int dm2_v1_anim_blit_to_memory_row_4to4bpp(const uint8_t *src,
                                           size_t src_size,
                                           uint16_t off_src,
                                           uint8_t *dst,
                                           size_t dst_size,
                                           uint16_t off_dst,
                                           uint16_t width,
                                           DM2_V1_AnimFileReceipt *out_receipt);

#endif
