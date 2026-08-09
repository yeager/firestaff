#ifndef NEXUS_V1_VDP2_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_VDP2_CAPTURE_COMPOSITOR_H

#include "nexus_v1_rasterizer.h"

#define NEXUS_V1_VDP2_NBG1_BITMAP_BYTES (512U * 256U)
#define NEXUS_V1_VDP2_NBG1_PALETTE_BYTES (256U * 2U)
#define NEXUS_V1_VDP2_REGISTERS_BYTES 0xe8U
#define NEXUS_V1_VDP2_CRAOFA_OFFSET 0xe4U

/* One source-bound NBG1 bitmap plane. The source and capture spans must be
 * exact byte matches; placement is explicit because registers alone do not
 * establish the host framebuffer crop. */
typedef struct {
    const uint8_t *capture_bitmap;
    int capture_bitmap_size;
    const uint8_t *capture_cram;
    int capture_cram_size;
    const uint8_t *vdp2_registers;
    int vdp2_registers_size;
    const uint8_t *source_bitmap;
    int source_bitmap_size;
    const uint8_t *source_palette;
    int source_palette_size;
    int source_hash_verified;
    int original_saturn_capture_verified;
    int transparent_index_zero_verified;
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    int width;
    int height;
    int palette_base;
} Nexus_V1_Vdp2CaptureCompositeInput;

typedef struct {
    int valid;
    int layer_registers_verified;
    int nbg1_bitmap_mode;
    int colour_code_256;
    int bitmap_span_join_verified;
    int palette_span_join_verified;
    int original_saturn_capture_verified;
    int renderer_permitted;
    int written_pixels;
    int transparent_pixels;
    int source_x;
    int source_y;
    int destination_x;
    int destination_y;
    int width;
    int height;
    int palette_base;
} Nexus_V1_Vdp2CaptureCompositeReceipt;

/* Replay one authenticated NBG1 8bpp bitmap span. This is deliberately a
 * capture consumer, not a VDP2 emulator or a general tilemap renderer. */
int nexus_v1_vdp2_capture_composite_nbg1_bitmap(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp2CaptureCompositeInput *input,
    Nexus_V1_Vdp2CaptureCompositeReceipt *out_receipt);

#endif
