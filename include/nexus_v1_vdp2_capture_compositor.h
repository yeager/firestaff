#ifndef NEXUS_V1_VDP2_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_VDP2_CAPTURE_COMPOSITOR_H

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_saturn_runtime_capture.h"

#define NEXUS_V1_VDP2_NBG1_BITMAP_BYTES (512U * 256U)
#define NEXUS_V1_VDP2_NBG1_PALETTE_BYTES (256U * 2U)
#define NEXUS_V1_VDP2_REGISTERS_BYTES 0xe8U
#define NEXUS_V1_VDP2_CRAOFA_OFFSET 0xe4U
#define NEXUS_V1_VDP2_NBG1_BITMAP_WIDTH 512U
#define NEXUS_V1_VDP2_NBG1_BITMAP_HEIGHT 256U
#define NEXUS_V1_VDP2_NBG0_BITMAP_BYTES (512U * 256U)
#define NEXUS_V1_VDP2_NBG0_BITMAP_WIDTH 512U
#define NEXUS_V1_VDP2_NBG0_BITMAP_HEIGHT 256U
#define NEXUS_V1_VDP2_YMIR_MENU_WIDTH 320U
#define NEXUS_V1_VDP2_YMIR_MENU_HEIGHT 224U
#define NEXUS_V1_VDP1_YMIR_FRAMEBUFFER_BYTES (512U * 256U * 2U)

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
    /* Set from Nexus_V1_SaturnRuntimeCaptureFrameReceipt when the input came
     * from a raw producer. UNKNOWN preserves older fixture compatibility. */
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order;
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
    int nbg0_bitmap_mode;
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

/* Raw NBG1 bitmap output, kept separate from the indexed production
 * framebuffer. This surface records Saturn's captured pixels and does not
 * imply a MENU.BPK/FONT256/HUD owner or a host placement. */
typedef struct {
    uint32_t rgba_buffer[NEXUS_V1_VDP2_NBG1_BITMAP_WIDTH *
                        NEXUS_V1_VDP2_NBG1_BITMAP_HEIGHT];
} Nexus_V1_Vdp2BitmapCaptureFramebuffer;

typedef struct {
    int valid;
    int capture_only;
    int layer_registers_verified;
    int nbg1_bitmap_mode;
    int nbg0_bitmap_mode;
    int colour_code_256;
    int bitmap_span_framed;
    int cram_span_framed;
    int cram_word_order_verified;
    int original_saturn_capture_verified;
    int renderer_permitted;
    int written_pixels;
    int transparent_pixels;
    uint32_t bitmap_vram_offset;
    uint32_t cram_offset;
} Nexus_V1_Vdp2BitmapCaptureReceipt;

/* Inputs for the observed Ymir Nexus title-menu capture lane. The two
 * caller attestations bind the independently retained Ymir dump bundle and
 * complete VDP1 trace; this function never authenticates files itself. */
typedef struct {
    const uint8_t *vdp1_display_framebuffer;
    size_t vdp1_display_framebuffer_size;
    const uint8_t *vdp2_vram;
    size_t vdp2_vram_size;
    const uint8_t *vdp2_cram;
    size_t vdp2_cram_size;
    const uint8_t *vdp2_registers;
    size_t vdp2_registers_size;
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order;
    /* Optional source-bound VDP1 mode-0 normal-sprite overlay. The command
     * record must be normalized for the C parser, while texture bytes retain
     * their captured byte lanes so packed 4bpp texels are not reordered. */
    const uint8_t *vdp1_vram;
    size_t vdp1_vram_size;
    uint32_t vdp1_command_byte_offset;
    int vdp1_command_source_join_verified;
    int original_ymir_capture_verified;
    int command_trace_complete_verified;
} Nexus_V1_Vdp2YmirMenuCaptureInput;

typedef struct {
    uint32_t rgba_buffer[NEXUS_V1_VDP2_YMIR_MENU_WIDTH *
                         NEXUS_V1_VDP2_YMIR_MENU_HEIGHT];
} Nexus_V1_Vdp2YmirMenuCaptureFramebuffer;

typedef struct {
    int valid;
    int capture_only;
    int renderer_permitted;
    int registers_verified;
    int command_trace_complete_verified;
    int original_ymir_capture_verified;
    int width;
    int height;
    int vdp1_direct_pixels;
    int vdp1_palette_pixels;
    int nbg0_overlay_pixels;
    int vdp1_command_overlaid;
    int vdp1_command_source_join_verified;
    int vdp1_command_pixels;
} Nexus_V1_Vdp2YmirMenuCaptureReceipt;

/* Replay one authenticated NBG1 8bpp bitmap span. This is deliberately a
 * capture consumer, not a VDP2 emulator or a general tilemap renderer. */
int nexus_v1_vdp2_capture_composite_nbg1_bitmap(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp2CaptureCompositeInput *input,
    Nexus_V1_Vdp2CaptureCompositeReceipt *out_receipt);

/* Decode one authenticated raw Saturn frame's NBG1 8bpp bitmap and CRAM.
 * Only the unambiguous 512x256/BMPNA-zero/CRAOFA-zero lane is admitted. */
int nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_bitmap(
    Nexus_V1_Vdp2BitmapCaptureFramebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_SaturnVdp2RegisterReceipt *out_register_receipt,
    Nexus_V1_Vdp2BitmapCaptureReceipt *out_receipt);

/* Decode an authenticated NBG0 8bpp bitmap capture. This is a bounded
 * capture consumer only; it does not establish VDP1 priority, menu ownership
 * or host presentation placement. */
int nexus_v1_vdp2_capture_decode_runtime_frame_nbg0_bitmap(
    Nexus_V1_Vdp2BitmapCaptureFramebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_SaturnVdp2RegisterReceipt *out_register_receipt,
    Nexus_V1_Vdp2BitmapCaptureReceipt *out_receipt);

/* Reconstruct the retained 320x224 Ymir title-menu capture from its raw
 * display framebuffer and VDP2 snapshots. This exact-register lane is
 * capture-only: a screenshot comparison is still required before any live
 * renderer may consume it. */
int nexus_v1_vdp2_capture_replay_ymir_title_menu(
    Nexus_V1_Vdp2YmirMenuCaptureFramebuffer *framebuffer,
    const Nexus_V1_Vdp2YmirMenuCaptureInput *input,
    Nexus_V1_Vdp2YmirMenuCaptureReceipt *out_receipt);

#endif
