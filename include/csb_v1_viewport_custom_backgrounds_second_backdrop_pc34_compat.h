#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_SECOND_BACKDROP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_SECOND_BACKDROP_PC34_COMPAT_H

#include <stddef.h>

typedef enum {
    CSB_V1_SECOND_BACKDROP_STEP_F0098_BASE_PIXELS = 0,
    CSB_V1_SECOND_BACKDROP_STEP_FIRST_CUSTOM_BACKGROUND = 1,
    CSB_V1_SECOND_BACKDROP_STEP_SECOND_CUSTOM_BACKGROUND = 2,
    CSB_V1_SECOND_BACKDROP_STEP_F0128_BACKDROP_KEEP_OUT = 3
} CSB_V1_CustomBackgroundsSecondBackdropStep;

typedef struct {
    int contract_only;
    int second_custom_background_call_index;
    int second_custom_background_room_num;
    int second_custom_background_is_unmasked_baseline;
    int f0098_base_pixels_drawn_first;
    int f0128_backdrop_keep_out_applies;
    int keep_out_only_masked_overlay_without_near_substitute;
    int has_csb_near_layer_substitute_for_keep_out;
    const char *redmcsb_f0128_viewport_anchor;
    const char *redmcsb_f0098_base_anchor;
    const char *redmcsb_f0128_keep_out_anchor;
    const char *redmcsb_f0128_second_backdrop_anchor;
    const char *csb_lineage_index_path;
    const char *source_summary;
} CSB_V1_CustomBackgroundsSecondBackdropContract;

const CSB_V1_CustomBackgroundsSecondBackdropContract *
csb_v1_viewport_custom_backgrounds_second_backdrop_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_second_backdrop_order_pc34(
    CSB_V1_CustomBackgroundsSecondBackdropStep *out_steps,
    size_t out_capacity);

const char *
csb_v1_viewport_custom_backgrounds_second_backdrop_source_evidence_pc34(void);

#endif
