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

typedef struct {
    const uint8_t *capture_name_table;
    int capture_name_table_size;
    const uint8_t *capture_character_generator;
    int capture_character_generator_size;
    const uint8_t *capture_cram;
    int capture_cram_size;
    const uint8_t *vdp2_registers;
    int vdp2_registers_size;
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
