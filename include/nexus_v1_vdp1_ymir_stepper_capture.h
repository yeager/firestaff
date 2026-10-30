#ifndef NEXUS_V1_VDP1_YMIR_STEPPER_CAPTURE_H
#define NEXUS_V1_VDP1_YMIR_STEPPER_CAPTURE_H

#include <stdint.h>

#include "nexus_v1_dungeon.h"
#include "nexus_v1_rasterizer.h"

/* This is a standalone, capture-only port of Ymir's integer VDP1 quad,
 * line, texture, and Gouraud steppers. It is intentionally not linked to
 * Firestaff's production renderer and never grants renderer admission. */
typedef struct {
    uint32_t rgba[NEXUS_FB_W * NEXUS_FB_H];
} Nexus_V1_Vdp1YmirCaptureSurface;

typedef struct {
    const uint8_t *command;
    int command_size;
    const uint8_t *texture_bytes;
    int texture_size;
    const uint8_t *palette_bytes;
    int palette_size;
    const uint8_t *vdp1_vram;
    int vdp1_vram_size;
    int command_capture_verified;
    int source_image_join_verified;
    int palette_join_verified;
    int vdp1_vram_hash_verified;
    int local_origin_verified;
    int local_origin_x;
    int local_origin_y;
    int system_clip_verified;
    int sys_clip_h;
    int sys_clip_v;
    int user_clip_verified;
    int user_clip_x0;
    int user_clip_y0;
    int user_clip_x1;
    int user_clip_y1;
    int even_odd_coord_select_verified;
    int even_odd_coord_select;
    /* This module currently models Ymir's single-density path only. */
    int single_density_verified;
} Nexus_V1_Vdp1YmirCaptureInput;

typedef struct {
    int valid;
    int capture_only;
    int renderer_permitted;
    int command_verified;
    int source_image_join_verified;
    int palette_join_verified;
    int local_origin_verified;
    int system_clip_verified;
    int user_clip_verified;
    int single_density_verified;
    int gouraud_verified;
    int gouraud_applied;
    int high_speed_shrink_applied;
    int pixels_written;
    int transparent_texels;
    int end_code_stops;
    int system_or_host_clipped_pixels;
    int user_clipped_pixels;
} Nexus_V1_Vdp1YmirCaptureReceipt;

/* Render one authenticated VDP1 type-2, mode-1 distorted sprite using the
 * integer stepping rules in Ymir's vdp1_steppers.hpp and
 * SoftwareVDPRenderer::VDP1PlotTexturedQuad/VDP1PlotTexturedLine.
 *
 * Coordinates in the command are signed 13-bit values; local_origin_* is
 * Ymir's captured VDP1 local-coordinate state. sys_clip_h/v are inclusive.
 * texture_bytes and palette_bytes must be the captured command's exact spans,
 * with source/palette ownership established by the caller. If Gouraud is
 * enabled, the table is read from the hash-verified full VDP1 VRAM snapshot.
 * The destination is updated atomically only on a valid receipt.
 *
 * Unsupported command features (mesh rendering, double density, malformed
 * user clipping, unavailable Gouraud/VRAM facts) are rejected. */
int nexus_v1_vdp1_ymir_capture_rasterize_mode1_distorted(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *input,
    Nexus_V1_Vdp1YmirCaptureReceipt *out_receipt);

#endif
