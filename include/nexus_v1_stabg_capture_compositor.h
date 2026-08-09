#ifndef NEXUS_V1_STABG_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_STABG_CAPTURE_COMPOSITOR_H

#include "nexus_v1_rasterizer.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Join the exact DMWeb STABG first-map decode with one explicitly supplied
 * Saturn capture crop. This is a capture-only HUD surface route: the caller
 * supplies the crop and destination, so no VDP2 placement is inferred. */
typedef struct {
    const uint8_t *stabg;
    size_t stabg_size;
    const uint8_t *capture_pixels;
    size_t capture_pixels_size;
    int capture_width;
    int capture_height;
    int capture_stride;
    /* Raw CRAM bytes in the exact byte order used by STABG.BIN's source
     * palette region. No host-endian palette conversion is accepted here. */
    const uint8_t *capture_palette;
    size_t capture_palette_size;
    int destination_x;
    int destination_y;
    int source_hash_verified;
    int original_saturn_capture_verified;
    int transparent_index_zero_verified;
} Nexus_V1_StabgCaptureInput;

typedef struct {
    int valid;
    int capture_only;
    int source_hash_verified;
    int original_saturn_capture_verified;
    int dmweb_decode_verified;
    int pixel_join_verified;
    int palette_join_verified;
    int explicit_placement_verified;
    int transparent_index_zero_verified;
    int vdp2_layer_owner_proven;
    int renderer_permitted;
    int destination_x;
    int destination_y;
    int width;
    int height;
    int written_pixels;
    int transparent_pixels;
    uint64_t palette_fnv1a64;
} Nexus_V1_StabgCaptureReceipt;

/* Failure leaves the framebuffer unchanged and never promotes the ordinary
 * STABG host render wrapper. A success is still only a capture witness. */
int nexus_v1_stabg_capture_composite(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_StabgCaptureInput *input,
    Nexus_V1_StabgCaptureReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
