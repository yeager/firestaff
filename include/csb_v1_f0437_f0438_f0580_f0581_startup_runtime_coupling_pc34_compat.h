#ifndef FIRESTAFF_CSB_V1_F0437_F0438_F0580_F0581_STARTUP_RUNTIME_COUPLING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0437_F0438_F0580_F0581_STARTUP_RUNTIME_COUPLING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_STARTUP_RUNTIME_TITLE_PRESENTS_PHASE_PC34 = 0x01,
    CSB_V1_STARTUP_RUNTIME_TITLE_CHAOS_ZOOM_PHASE_PC34 = 0x02,
    CSB_V1_STARTUP_RUNTIME_TITLE_CHAOS_HOLD_PHASE_PC34 = 0x04,
    CSB_V1_STARTUP_RUNTIME_TITLE_STRIKES_BACK_PHASE_PC34 = 0x08,
    CSB_V1_STARTUP_RUNTIME_TITLE_ALL_PHASES_PC34 = 0x0f,
    CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34 = 4,
    CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 = 31
};

typedef enum CSB_V1_StartupRuntimeCouplingStage_PC34 {
    CSB_V1_STARTUP_RUNTIME_F0437_DRAW_TITLE_PC34 = 1u << 0,
    CSB_V1_STARTUP_RUNTIME_F0438_OPEN_ENTRANCE_DOORS_PC34 = 1u << 1,
    CSB_V1_STARTUP_RUNTIME_F0580_DRAW_DOOR_STEP_PC34 = 1u << 2,
    CSB_V1_STARTUP_RUNTIME_F0581_BLIT_DOORS_PC34 = 1u << 3
} CSB_V1_StartupRuntimeCouplingStage_PC34;

#define CSB_V1_STARTUP_RUNTIME_COUPLING_ALL_STAGES_PC34 \
    (CSB_V1_STARTUP_RUNTIME_F0437_DRAW_TITLE_PC34 | \
     CSB_V1_STARTUP_RUNTIME_F0438_OPEN_ENTRANCE_DOORS_PC34 | \
     CSB_V1_STARTUP_RUNTIME_F0580_DRAW_DOOR_STEP_PC34 | \
     CSB_V1_STARTUP_RUNTIME_F0581_BLIT_DOORS_PC34)

typedef struct CSB_V1_StartupRuntimeCouplingFacts_PC34 {
    int valid;
    int real_startup_assets_bound;
    int title_presents_runtime_captured;
    int title_chaos_zoom_runtime_captured;
    int title_chaos_hold_runtime_captured;
    int title_strikes_back_runtime_captured;
    int title_runtime_phase_mask;
    int title_runtime_phase_hash_count;
    int title_runtime_unique_sample_hash_count;
    int closed_door_hud_runtime_captured;
    int utility_hud_runtime_captured;
    int door_opening_delay_runtime_captured;
    int door_opening_frame_runtime_captured;
    int source_door_step_count;
    int door_step_index;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_fallback_callbacks;
    int no_wrapper_fallback_routes;
    int no_legacy_door_fallback_route;
    int no_synthetic_visuals;
} CSB_V1_StartupRuntimeCouplingFacts_PC34;

typedef struct CSB_V1_StartupRuntimeCouplingReceipt_PC34 {
    int valid;
    uint32_t accepted_stage_mask;
    uint32_t rejected_stage_mask;
    int real_startup_assets_bound;
    int title_phase_route_complete;
    int hud_runtime_coupled;
    int door_runtime_coupled;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_legacy_wrappers;
    int no_synthetic_visuals;
    const char *source_evidence;
} CSB_V1_StartupRuntimeCouplingReceipt_PC34;

void csb_v1_startup_runtime_coupling_receipt_init_pc34(
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *receipt);

int F0437_STARTEND_DrawTitle(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt);
int F0438_STARTEND_OpenEntranceDoors(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt);
int F0580_ENTRANCE_DrawDoorAnimationStep(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt);
int F0581_ENTRANCE_BlitDoors(
    const CSB_V1_StartupRuntimeCouplingFacts_PC34 *facts,
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 *out_receipt);

const char *csb_v1_f0437_startend_draw_title_source_evidence_pc34(void);
const char *csb_v1_f0438_startend_open_entrance_doors_source_evidence_pc34(void);
const char *csb_v1_f0580_entrance_draw_door_step_source_evidence_pc34(void);
const char *csb_v1_f0581_entrance_blit_doors_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0437_F0438_F0580_F0581_STARTUP_RUNTIME_COUPLING_PC34_COMPAT_H */
