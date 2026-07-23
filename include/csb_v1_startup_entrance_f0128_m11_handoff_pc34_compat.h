#ifndef FIRESTAFF_CSB_V1_STARTUP_ENTRANCE_F0128_M11_HANDOFF_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_ENTRANCE_F0128_M11_HANDOFF_PC34_COMPAT_H

#include "csb_v1_boot.h"

int csb_v1_startup_entrance_f0128_produce_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_ViewportFirstFrameMaterializationReceipt *out_material,
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 *out_raster,
    uint8_t *out_pixels, size_t out_pixel_count);

#endif
