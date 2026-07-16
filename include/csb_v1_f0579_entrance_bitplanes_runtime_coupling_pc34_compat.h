#ifndef FIRESTAFF_CSB_V1_F0579_ENTRANCE_BITPLANES_RUNTIME_COUPLING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0579_ENTRANCE_BITPLANES_RUNTIME_COUPLING_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_f0437_f0438_f0580_f0581_startup_runtime_coupling_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0579_SOURCE_COMPOSITE_WIDTH_PC34 = 256,
    CSB_V1_F0579_SOURCE_COMPOSITE_HEIGHT_PC34 = 161,
    CSB_V1_F0579_TARGET_SCREEN_WIDTH_PC34 = 320,
    CSB_V1_F0579_TARGET_SCREEN_HEIGHT_PC34 = 200,
    CSB_V1_F0579_TARGET_UPPER_Y_PC34 = 30,
    CSB_V1_F0579_TARGET_LOWER_Y_PC34 = 110,
    CSB_V1_F0579_DOOR_HALF_PLANE_COUNT_PC34 = 4
};

typedef struct CSB_V1_F0579_EntranceBitplanesFacts_PC34 {
    int valid;
    int source_composite_real_asset_bound;
    int target_screen_real_asset_bound;
    int source_width;
    int source_height;
    int target_width;
    int target_height;
    int upper_half_target_y;
    int lower_half_target_y;
    int source_plane_count;
    int target_plane_count;
    int no_legacy_bitplane_wrapper;
    int no_synthetic_visuals;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 runtime_coupling;
} CSB_V1_F0579_EntranceBitplanesFacts_PC34;

typedef struct CSB_V1_F0579_EntranceBitplanesReceipt_PC34 {
    int valid;
    int source_composite_bound;
    int target_screen_bound;
    int geometry_source_locked;
    int runtime_coupling_consumed;
    int title_hud_door_runtime_ready;
    int no_legacy_bitplane_wrapper;
    int no_synthetic_visuals;
    const char *source_evidence;
} CSB_V1_F0579_EntranceBitplanesReceipt_PC34;

void csb_v1_f0579_entrance_bitplanes_receipt_init_pc34(
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 *receipt);

int F0579_ENTRANCE_InitializeBitPlanes(
    const CSB_V1_F0579_EntranceBitplanesFacts_PC34 *facts,
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 *out_receipt);

const char *csb_v1_f0579_entrance_initialize_bitplanes_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0579_ENTRANCE_BITPLANES_RUNTIME_COUPLING_PC34_COMPAT_H */
