#ifndef FIRESTAFF_CSB_V1_SOURCE_BOUND_FILLBOX_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_SOURCE_BOUND_FILLBOX_PC34_COMPAT_H

#include "csb_v1_boot.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB BLITFILL.C F0134/F0135, restricted to a verified PC34 CSB
 * GRAPHICS.DAT host raster. The target owns a copy of the admitted raster;
 * neither operation may draw a generated panel, door, or source surface. */
typedef struct CSB_V1_SourceBoundFillTarget_PC34 {
    uint8_t *pixels;
    size_t pixel_count;
    int width;
    int height;
    int valid;
    int real_graphics_dat;
    int no_fallback_route;
    CSB_V1_StartupRuntimeHostSurface_PC34 host_surface;
    uint32_t source_pixel_hash;
    uint32_t source_route_hash;
    uint32_t source_host_surface_hash;
    uint32_t result_pixel_hash;
    uint32_t operation_count;
} CSB_V1_SourceBoundFillTarget_PC34;

/* ENTRANCE.C F0438 has 31 C002/C003 phases. Keep phase validation beside the
 * source-bound consumer so it verifies an existing host frame and never
 * changes TITLE/ENTRANCE plan selection. */
typedef struct CSB_V1_DoorOpeningPhaseReceipt_PC34 {
    int valid;
    int opening_step;
    int f0128_viewport_bound;
    int source_surface_count;
    uint32_t source_tick;
    uint32_t session_generation;
    uint32_t frame_route_hash;
    uint32_t raster_pixel_hash;
    uint32_t host_surface_hash;
} CSB_V1_DoorOpeningPhaseReceipt_PC34;

void csb_v1_source_bound_fill_target_init_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target);
void csb_v1_source_bound_fill_target_release_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target);

/* Accepts only a current C017/C040 HUD or C002/C003 opening-door host
 * receipt from a session whose HUD bindings are the original GRAPHICS.DAT.
 * It copies the exact indexed raster before any mutation. */
int csb_v1_source_bound_fill_target_from_host_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt,
    CSB_V1_SourceBoundFillTarget_PC34 *out_target);

/* Validates one already-produced F0438 runtime/M11 host frame. This routine
 * has no render-plan input and cannot alter title or Entrance state. */
int csb_v1_door_opening_phase_receipt_from_host_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_receipt,
    int f0128_viewport_bound,
    CSB_V1_DoorOpeningPhaseReceipt_PC34 *out_receipt);

/* F0134 fills the complete source-owned indexed target with the original
 * four-bit color. The indexed representation is the post-planar equivalent
 * of ReDMCSB's packed-nibble fill. */
int csb_v1_source_bound_f0134_fill_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target,
    uint8_t color);

/* F0135 fills an inclusive box. Bit 15 preserves F0692's every-other-pixel
 * mode; all coordinates must be within the source-owned raster. */
int csb_v1_source_bound_f0135_fill_box_pc34(
    CSB_V1_SourceBoundFillTarget_PC34 *target,
    const int16_t box[4],
    uint16_t color);

const char *csb_v1_source_bound_fillbox_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
