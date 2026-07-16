#ifndef FIRESTAFF_CSB_V1_F0807_ENTRANCE_ANIMATION_STEP_RUNTIME_COUPLING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0807_ENTRANCE_ANIMATION_STEP_RUNTIME_COUPLING_PC34_COMPAT_H

#include "csb_v1_f0579_entrance_bitplanes_runtime_coupling_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0807_ENTRANCE_DOOR_STEP_FIRST_PC34 = 1,
    CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34 = 31,
    CSB_V1_F0807_TARGET_SCREEN_WIDTH_PC34 = 320,
    CSB_V1_F0807_TARGET_SCREEN_HEIGHT_PC34 = 200
};

typedef struct CSB_V1_F0807_EntranceAnimationStepFacts_PC34 {
    int valid;
    int animation_step_real_asset_bound;
    int target_screen_real_asset_bound;
    int source_door_step_count;
    int animation_step_index;
    int target_screen_width;
    int target_screen_height;
    int blit_box_source_locked;
    int no_transparency_mode;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_legacy_animation_wrapper;
    int no_synthetic_visuals;
    CSB_V1_F0579_EntranceBitplanesReceipt_PC34 bitplanes;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 runtime_coupling;
} CSB_V1_F0807_EntranceAnimationStepFacts_PC34;

typedef struct CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 {
    int valid;
    int animation_step_bound;
    int target_screen_bound;
    int source_step_range_locked;
    int bitplanes_consumed;
    int runtime_coupling_consumed;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_legacy_animation_wrapper;
    int no_synthetic_visuals;
    int accepted_animation_step_index;
    const char *source_evidence;
} CSB_V1_F0807_EntranceAnimationStepReceipt_PC34;

void csb_v1_f0807_entrance_animation_step_receipt_init_pc34(
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 *receipt);

int F0807_ENTRANCE_DrawAnimationStep(
    const CSB_V1_F0807_EntranceAnimationStepFacts_PC34 *facts,
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 *out_receipt);

const char *csb_v1_f0807_entrance_draw_animation_step_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0807_ENTRANCE_ANIMATION_STEP_RUNTIME_COUPLING_PC34_COMPAT_H */
