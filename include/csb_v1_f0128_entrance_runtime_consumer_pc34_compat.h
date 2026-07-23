#ifndef FIRESTAFF_CSB_V1_F0128_ENTRANCE_RUNTIME_CONSUMER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0128_ENTRANCE_RUNTIME_CONSUMER_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_boot.h"

typedef struct CSB_V1_F0128EntranceRuntimeBinding_PC34 {
    int valid;
    uint32_t source_tick;
    uint32_t session_generation;
    const CSB_V1_ViewportFirstFrameMaterializationReceipt *material_receipt;
    const CSB_V1_ViewportFirstFrameRasterReceiptPc34 *raster_receipt;
    const uint8_t *viewport_pixels;
    size_t viewport_pixel_count;
} CSB_V1_F0128EntranceRuntimeBinding_PC34;

/* Consumes an already materialized, original-data F0128 viewport in the
 * C004/C002/C003 Entrance path. It never builds a substitute viewport. */
int csb_v1_f0128_entrance_runtime_consume_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_F0128EntranceRuntimeBinding_PC34 *binding,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *out_receipt);

#endif
