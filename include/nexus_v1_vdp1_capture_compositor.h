#ifndef NEXUS_V1_VDP1_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_VDP1_CAPTURE_COMPOSITOR_H

#include "nexus_v1_rasterizer.h"
#include "nexus_v1_dungeon.h"

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
    int screen_origin_x;
    int screen_origin_y;
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
    int palette_slot_base;
    int screen_origin_x;
    int screen_origin_y;
} Nexus_V1_Vdp1CaptureCompositeReceipt;

/* Composite one source-bound mode-1 quad in command order. The operation is
 * capture replay only; no source bytes are decoded unless both exact DGN
 * joins and the original-capture attestation are present. */
int nexus_v1_vdp1_capture_composite_mode1(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_Vdp1CaptureCompositeInput *input,
    Nexus_V1_Vdp1CaptureCompositeReceipt *out_receipt);

#endif
