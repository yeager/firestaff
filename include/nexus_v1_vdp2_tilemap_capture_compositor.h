#ifndef NEXUS_V1_VDP2_TILEMAP_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_VDP2_TILEMAP_CAPTURE_COMPOSITOR_H

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_saturn_runtime_capture.h"

/* Capture-only VDP2 NBG1 tilemap consumer. Register meanings follow
 * Mednafen's TileFetcher::Fetch/T_DrawNBG implementation. */
#define NEXUS_V1_VDP2_TILEMAP_REGISTERS_BYTES 0xe8U
#define NEXUS_V1_VDP2_TILEMAP_BGON_OFFSET 0x20U
#define NEXUS_V1_VDP2_TILEMAP_CHCTLA_OFFSET 0x28U
#define NEXUS_V1_VDP2_TILEMAP_PNCN1_OFFSET 0x32U
#define NEXUS_V1_VDP2_TILEMAP_CRAOFA_OFFSET 0xe4U
#define NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH 320U
#define NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT 224U
#define NEXUS_V1_VDP2_CAPTURE_PIXELS \
    (NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH * NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT)
#define NEXUS_V1_VDP2_STACK_LAYER_VDP1 0U
#define NEXUS_V1_VDP2_STACK_LAYER_NBG0 1U
#define NEXUS_V1_VDP2_STACK_LAYER_NBG1 2U
#define NEXUS_V1_VDP2_STACK_LAYER_NBG2 3U
#define NEXUS_V1_VDP2_STACK_LAYER_EMPTY 255U

typedef struct {
    const uint8_t *capture_name_table;
    int capture_name_table_size;
    const uint8_t *capture_character_generator;
    int capture_character_generator_size;
    const uint8_t *capture_cram;
    int capture_cram_size;
    const uint8_t *vdp2_registers;
    int vdp2_registers_size;
    /* The raw capture producer's Saturn word serialization applies to the
     * register, name-table and CRAM spans as one unit.  UNKNOWN preserves
     * the old fixture scorer; a concrete capture must carry its receipt. */
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order;
    const uint8_t *source_name_table;
    int source_name_table_size;
    const uint8_t *source_character_generator;
    int source_character_generator_size;
    const uint8_t *source_cram;
    int source_cram_size;
    int map_columns;
    int map_rows;
    /* Reserved for a future source-map crop. The first admitted lane is
     * deliberately whole-span only, so these must remain zero. */
    int source_tile_x;
    int source_tile_y;
    int destination_x;
    int destination_y;
    int source_hash_verified;
    int original_saturn_capture_verified;
    int transparent_index_zero_verified;
} Nexus_V1_Vdp2TilemapCaptureInput;

typedef struct {
    int valid;
    int capture_only;
    int layer_registers_verified;
    int nbg1_tilemap_mode;
    int pnd_size_two_words;
    int colour_code_4_or_8bpp;
    int name_table_span_join_verified;
    int character_generator_span_join_verified;
    int cram_span_join_verified;
    int original_saturn_capture_verified;
    int renderer_permitted;
    int written_pixels;
    int transparent_pixels;
    int tiles_decoded;
    int bits_per_pixel;
    int destination_x;
    int destination_y;
    int map_columns;
    int map_rows;
} Nexus_V1_Vdp2TilemapCaptureReceipt;

/* Raw visible NBG2 palette-index plane for the exact zero-scroll 8bpp Ymir
 * frame lane. It is a capture measurement and carries no retail asset owner. */
typedef struct {
    uint8_t index_buffer[NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH *
                         NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT];
    uint32_t rgba_buffer[NEXUS_V1_VDP2_NBG2_CAPTURE_WIDTH *
                         NEXUS_V1_VDP2_NBG2_CAPTURE_HEIGHT];
} Nexus_V1_Vdp2CapturePlane;

typedef struct {
    int valid;
    int capture_only;
    int renderer_permitted;
    int registers_verified;
    int original_ymir_capture_verified;
    int nbg2_tilemap_mode;
    int pnd_size_two_words;
    int colour_code_256;
    int name_table_vram_offset;
    int map_page_columns;
    int map_page_rows;
    int visible_columns;
    int visible_rows;
    int tiles_decoded;
    int written_pixels;
    int transparent_pixels;
} Nexus_V1_Vdp2Nbg2CapturePlaneReceipt;

int nexus_v1_vdp2_capture_decode_ymir_nbg2_tilemap_plane(
    Nexus_V1_Vdp2CapturePlane *plane,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    Nexus_V1_Vdp2Nbg2CapturePlaneReceipt *out_receipt);

typedef struct {
    int valid;
    int capture_only;
    int renderer_permitted;
    int registers_verified;
    int original_ymir_capture_verified;
    int nbg0_bitmap_mode;
    int colour_code_256;
    int bitmap_vram_offset;
    int written_pixels;
    int transparent_pixels;
} Nexus_V1_Vdp2Nbg0CapturePlaneReceipt;

int nexus_v1_vdp2_capture_decode_ymir_nbg0_bitmap_plane(
    Nexus_V1_Vdp2CapturePlane *plane,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    Nexus_V1_Vdp2Nbg0CapturePlaneReceipt *out_receipt);

typedef struct {
    int valid;
    int capture_only;
    int renderer_permitted;
    int registers_verified;
    int original_ymir_capture_verified;
    int nbg1_tilemap_mode;
    int pnd_size_two_words;
    int colour_code_256;
    int name_table_vram_offset;
    int map_page_columns;
    int map_page_rows;
    int visible_columns;
    int visible_rows;
    int tiles_decoded;
    int written_pixels;
    int transparent_pixels;
} Nexus_V1_Vdp2Nbg1CapturePlaneReceipt;

int nexus_v1_vdp2_capture_decode_ymir_nbg1_tilemap_plane(
    Nexus_V1_Vdp2CapturePlane *plane,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    Nexus_V1_Vdp2Nbg1CapturePlaneReceipt *out_receipt);

/* Capture-only pixel-priority stack for the hash-pinned Ymir frame 239.
 * The VDP1 framebuffer is RGB555 when bit 15 is set and palette/type-6 data
 * otherwise. This lane rejects unsupported color-calculation and shadow
 * cases; it does not grant production renderer permission. */
typedef struct {
    uint32_t rgba_buffer[NEXUS_V1_VDP2_CAPTURE_PIXELS];
    uint8_t top_layer_buffer[NEXUS_V1_VDP2_CAPTURE_PIXELS];
    uint8_t priority_buffer[NEXUS_V1_VDP2_CAPTURE_PIXELS];
} Nexus_V1_Vdp2YmirFrame239StackPlane;

typedef struct {
    int valid;
    int capture_only;
    int renderer_permitted;
    int registers_verified;
    int vdp1_framebuffer_verified;
    int original_ymir_capture_verified;
    int pixel_priority_stack_verified;
    int color_calculation_cases_unsupported;
    int shadow_cases_unsupported;
    int vdp1_rgb_pixels;
    int vdp1_cram_pixels;
    int nbg0_pixels;
    int nbg1_pixels;
    int nbg2_pixels;
    int empty_pixels;
} Nexus_V1_Vdp2YmirFrame239StackReceipt;

int nexus_v1_vdp2_capture_compose_ymir_frame239_priority_stack(
    Nexus_V1_Vdp2YmirFrame239StackPlane *plane,
    const uint8_t *vdp1_framebuffer, size_t vdp1_framebuffer_size,
    const uint8_t *vdp2_vram, size_t vdp2_vram_size,
    const uint8_t *vdp2_cram, size_t vdp2_cram_size,
    const uint8_t *vdp2_registers, size_t vdp2_registers_size,
    Nexus_V1_SaturnVdp2RegisterByteOrder register_byte_order,
    int original_ymir_capture_verified,
    int vdp1_framebuffer_verified,
    Nexus_V1_Vdp2YmirFrame239StackReceipt *out_receipt);

/* Explicit source/VRAM binding for one raw Saturn frame. The caller supplies
 * canonical source bytes and the already-proven VDP2-VRAM offsets; this
 * adapter never infers a menu map, FONT256 owner, or crop from register data. */
typedef struct {
    uint32_t capture_name_table_offset;
    uint32_t capture_character_generator_offset;
    int capture_character_generator_size;
    const uint8_t *source_name_table;
    int source_name_table_size;
    const uint8_t *source_character_generator;
    int source_character_generator_size;
    const uint8_t *source_cram;
    int source_cram_size;
    int map_columns;
    int map_rows;
    int source_tile_x;
    int source_tile_y;
    int destination_x;
    int destination_y;
    int source_hash_verified;
    int transparent_index_zero_verified;
} Nexus_V1_Vdp2RuntimeTilemapBinding;

int nexus_v1_vdp2_capture_composite_nbg1_tilemap(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp2TilemapCaptureInput *input,
    Nexus_V1_Vdp2TilemapCaptureReceipt *out_receipt);

/* Bind one authenticated raw frame to the bounded NBG1 consumer. All source
 * ownership and VRAM offsets remain explicit inputs. */
int nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    const Nexus_V1_Vdp2RuntimeTilemapBinding *binding,
    Nexus_V1_Vdp2TilemapCaptureReceipt *out_receipt);

/* Decode the bounded NBG1 4bpp/8x8/2-word tilemap lane directly from one
 * authenticated raw frame. This is a capture consumer only: it does not
 * bind the bytes to MENU.BPK, FONT256, HUD, viewport, or a host crop. */
int nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_tilemap(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    int source_x, int source_y, int width, int height,
    int destination_x, int destination_y,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_SaturnVdp2RegisterReceipt *out_register_receipt,
    Nexus_V1_Vdp2TilemapCaptureReceipt *out_receipt);

#endif
