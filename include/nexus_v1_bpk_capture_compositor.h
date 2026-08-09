#ifndef NEXUS_V1_BPK_CAPTURE_COMPOSITOR_H
#define NEXUS_V1_BPK_CAPTURE_COMPOSITOR_H

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_rasterizer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* One explicitly bounded indexed BPK surface as observed in a Saturn
 * capture. This is a surface join, not a claim about MENU.BPK entry meaning,
 * VDP2 layer ownership, or menu placement. The caller supplies the exact
 * capture crop and destination rectangle; no crop is inferred. */
typedef struct {
    const uint8_t *archive;
    size_t archive_size;
    uint32_t entry_index;
    const uint8_t *capture_pixels;
    size_t capture_pixels_size;
    int capture_width;
    int capture_height;
    int capture_stride;
    const uint8_t *capture_palette;
    size_t capture_palette_size;
    int destination_x;
    int destination_y;
    int bpk_hash_verified;
    int original_saturn_capture_verified;
    int transparent_index_zero_verified;
} Nexus_V1_BpkCaptureSurfaceInput;

typedef struct {
    int valid;
    int capture_only;
    int bpk_hash_verified;
    int original_saturn_capture_verified;
    int prs3_pixel_join_verified;
    int palt_cram_join_verified;
    int explicit_placement_verified;
    int transparent_index_zero_verified;
    int menu_semantics_proven;
    int vdp2_layer_owner_proven;
    int renderer_permitted;
    uint32_t entry_index;
    uint16_t width;
    uint8_t height;
    int destination_x;
    int destination_y;
    int written_pixels;
    int transparent_pixels;
    uint64_t palt_fnv1a64;
} Nexus_V1_BpkCaptureSurfaceReceipt;

/* Replay one exact indexed BPK crop. Failure leaves the framebuffer
 * unchanged and the receipt non-rendering. A successful call still remains
 * capture-only: it does not authorize the ordinary launcher/menu route. */
int nexus_v1_bpk_capture_surface(
    Nexus_Framebuffer *framebuffer,
    const Nexus_V1_BpkCaptureSurfaceInput *input,
    Nexus_V1_BpkCaptureSurfaceReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
