#ifndef NEXUS_V1_VDP1_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_VDP1_CAPTURE_COMPOSITOR_H

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_vdp1_command_sequence.h"
#include "nexus_v1_saturn_runtime_capture.h"

/* A narrowly scoped host presentation of one authenticated Saturn VDP1
 * command. It is not a DGN renderer: source image/palette joins and the
 * command's signed screen coordinates must all come from the same capture.
 *
 * Saturn VDP1 coordinates use the display centre as origin for the command
 * lane. The caller supplies that captured display origin explicitly because
 * local-coordinate and system-clip state are not recoverable from a command
 * record alone.
 */
typedef struct {
    const uint8_t *command;
    int command_size;
    const uint8_t *texture_span;
    int texture_span_size;
    const uint8_t *palette_state;
    int palette_state_size;
    const uint8_t *dgn_image;
    int dgn_image_size;
    const uint8_t *dgn_palette;
    int dgn_palette_size;
    int dgn_source_hash_verified;
    int original_saturn_capture_verified;
    /* A captured mode-1 source whose every texel is index zero and whose
     * draw mode keeps index zero transparent. This is a bounded clear/no-op
     * observation, not a DGN material owner. */
    int transparent_capture_noop_verified;
    int capture_allow_zero_pixel_command;
    int screen_origin_x;
    int screen_origin_y;
    int system_clip_state_verified;
    int system_clip_x;
    int system_clip_y;
    int palette_slot_base;
} Nexus_V1_Vdp1CaptureCompositeInput;

typedef struct {
    int valid;
    int source_join_verified;
    int palette_join_verified;
    int command_framed;
    int mode1_lookup;
    int coordinate_words_framed;
    int original_saturn_capture_verified;
    int renderer_permitted;
    int fallback_visuals_permitted;
    int written_pixels;
    int transparent_pixels;
    int end_code_pixels;
    int transparent_noop_verified;
    int palette_slot_base;
    int screen_origin_x;
    int screen_origin_y;
} Nexus_V1_Vdp1CaptureCompositeReceipt;

/* VDP1 colour mode 5 is not a CLUT texture.  It is a 16-bit, 32K-colour
 * source stream.  Keep this capture-only surface separate from the indexed
 * Nexus framebuffer: publishing a host palette for mode 5 would lose the
 * Saturn word semantics and could accidentally authorize an unowned DGN
 * material.  Mednafen 1.32.1 src/ss/vdp1.cpp::TexFetch<*> documents the
 * direct RGB fetch and its ECD transparency code. */
typedef struct {
    uint32_t rgba_buffer[NEXUS_FB_W * NEXUS_FB_H];
    int clear_color;
} Nexus_V1_Vdp1DirectColorFramebuffer;

typedef struct {
    int valid;
    int capture_only;
    int command_framed;
    int direct_color_mode;
    int source_word_order_verified;
    int coordinate_words_framed;
    int original_saturn_capture_verified;
    int renderer_permitted;
    int fallback_visuals_permitted;
    int written_pixels;
    int transparent_pixels;
    uint32_t command_byte_offset;
} Nexus_V1_Vdp1DirectColorCaptureReceipt;

/* A complete, bounded replay lane for one captured VDP1 command window.
 * The per-command source joins remain explicit; these additional facts bind
 * the state that one command cannot establish by itself. */
typedef struct {
    const Nexus_V1_Vdp1CaptureCompositeInput *commands;
    int command_count;
    int system_clip_state_verified;
    int local_coordinate_state_verified;
    int display_origin_state_verified;
    int display_origin_x;
    int display_origin_y;
    int command_order_verified;
    int end_record_verified;
    int system_clip_x;
    int system_clip_y;
} Nexus_V1_Vdp1CaptureSequenceInput;

typedef struct {
    int valid;
    int sequence_state_verified;
    int command_count;
    int command_frames_verified;
    int source_joins_verified;
    int palette_joins_verified;
    int transparent_noop_commands;
    int capture_gap_commands;
    int display_origin_verified;
    int display_origin_x;
    int display_origin_y;
    int command_order_verified;
    int end_record_verified;
    int renderer_permitted;
    int written_pixels;
    int transparent_pixels;
    int end_code_pixels;
    int system_clip_state_verified;
    int system_clip_x;
    int system_clip_y;
} Nexus_V1_Vdp1CaptureSequenceReceipt;

/* Resolves one command's captured VDP1 source/CLUT spans to the canonical
 * DGN bytes that own them. Returning zero rejects the complete frame. */
typedef int (*Nexus_V1_Vdp1CaptureSequenceMaterialResolver)(
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *command, int command_size,
    const Nexus_V1_Vdp1TextureCommand *parsed,
    uint32_t command_byte_offset,
    Nexus_V1_Vdp1CaptureCompositeInput *out_input,
    void *context);

typedef struct {
    const uint8_t *vdp1_vram;
    int vdp1_vram_size;
    uint32_t copr_word;
    int original_saturn_capture_verified;
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material;
    void *resolver_context;
    /* Permit a bounded mode-1-only replay to skip non-mode-1 draws whose
     * owner belongs to a separate capture lane (for example direct RGB). */
    int mode1_only_capture;
    int system_clip_state_present;
    uint32_t system_clip_x;
    uint32_t system_clip_y;
} Nexus_V1_Vdp1CaptureVramSequenceInput;

typedef struct {
    int valid;
    Nexus_V1_Vdp1CommandSequenceReceipt command_sequence;
    Nexus_V1_Vdp1CaptureSequenceReceipt replay;
    int draw_commands_seen;
    int draw_commands_resolved;
    int unresolved_draw_commands;
    int unowned_non_mode1_draw_commands;
    int unowned_mode1_draw_commands;
    int skipped_non_draw_commands;
    int system_clip_state_missing;
    int system_clip_state_verified;
    int system_clip_x;
    int system_clip_y;
    int semantic_admission_blocked;
} Nexus_V1_Vdp1CaptureVramSequenceReceipt;

/* Composite one source-bound mode-1 quad in command order. The operation is
 * capture replay only; no source bytes are decoded unless both exact DGN
 * joins and the original-capture attestation are present. */
int nexus_v1_vdp1_capture_composite_mode1(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureCompositeInput *input,
    Nexus_V1_Vdp1CaptureCompositeReceipt *out_receipt);

/* Decode one authenticated mode-5 command into an RGBA capture surface.
 * VDP1 VRAM is a little-endian word image; each source word is decoded as
 * Saturn 32K RGB.  This is deliberately capture-only: no DGN owner is
 * inferred and renderer_permitted remains zero until a separately verified
 * owner/material handoff exists. */
int nexus_v1_vdp1_capture_decode_direct_color(
    Nexus_V1_Vdp1DirectColorFramebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureCompositeInput *input,
    Nexus_V1_Vdp1DirectColorCaptureReceipt *out_receipt);

/* Replay a complete authenticated VDP1 window atomically. On any failed
 * command or missing state fact, the destination framebuffer is unchanged. */
int nexus_v1_vdp1_capture_composite_mode1_sequence(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureSequenceInput *input,
    Nexus_V1_Vdp1CaptureSequenceReceipt *out_receipt);

/* Parse and replay one complete authenticated VDP1 VRAM command sequence.
 * Every textured draw must be resolved by an exact DGN source/CLUT join. */
int nexus_v1_vdp1_capture_replay_vram_sequence(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureVramSequenceInput *input,
    Nexus_V1_Vdp1CaptureVramSequenceReceipt *out_receipt);

/* Parse one authenticated Saturn raw frame and feed its VDP1 VRAM/COPR
 * directly into the bounded replay lane. VDP2 data is exposed by the frame
 * receipt but is not silently composed here. */
int nexus_v1_vdp1_capture_replay_runtime_frame(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material,
    void *resolver_context,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CaptureVramSequenceReceipt *out_replay_receipt);

/* Replay every source-bound mode-1 draw in one authenticated frame, retaining
 * non-mode-1 draws as explicitly unowned capture gaps. */
int nexus_v1_vdp1_capture_replay_runtime_frame_mode1_sequence(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material,
    void *resolver_context,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CaptureVramSequenceReceipt *out_replay_receipt);

/* Decode the first authenticated colour-mode-5 draw in one raw Saturn frame.
 * This is capture-only: material ownership and renderer permission remain
 * deliberately absent until an exact source-owner handoff is verified. */
int nexus_v1_vdp1_capture_decode_direct_color_runtime_frame(
    Nexus_V1_Vdp1DirectColorFramebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CommandSequenceReceipt *out_sequence_receipt,
    Nexus_V1_Vdp1DirectColorCaptureReceipt *out_direct_receipt);

/* Resolve and replay the first authenticated colour-mode-1 draw whose
 * material callback succeeds. The result is a source-bound capture replay of
 * one command, not a complete DGN scene: camera transform, face selection,
 * culling and command-list scene ownership remain caller-visible facts. */
int nexus_v1_vdp1_capture_replay_runtime_frame_mode1_material(
    Nexus_Framebuffer *framebuffer,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    unsigned int frame_index,
    Nexus_V1_Vdp1CaptureSequenceMaterialResolver resolve_material,
    void *resolver_context,
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt *out_frame_receipt,
    Nexus_V1_Vdp1CommandSequenceReceipt *out_sequence_receipt,
    Nexus_V1_Vdp1CaptureCompositeReceipt *out_composite_receipt,
    uint32_t *out_command_byte_offset);

#endif
