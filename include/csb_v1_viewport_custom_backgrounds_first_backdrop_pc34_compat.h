#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_FIRST_BACKDROP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_FIRST_BACKDROP_PC34_COMPAT_H

/* CSB V1 CustomBackgrounds first-backdrop contract-only API.
 * Anchors: ReDMCSB DUNVIEW.C F0128:8318-8486, F0098:2962-3002,
 * DEFS.H:2088 and 2596-2614; CSB-lineage Viewport.cpp:6451-6505,
 * 6574-6622, 6599-6619, and first dispatch at 6924-6927. */

#include <stddef.h>

typedef enum {
    CSB_V1_FIRST_BACKDROP_STEP_F0098_BASE_PIXELS = 0,
    CSB_V1_FIRST_BACKDROP_STEP_FIRST_CUSTOM_BACKGROUND = 1,
    CSB_V1_FIRST_BACKDROP_STEP_SECOND_CUSTOM_BACKGROUND_FALLBACK = 2,
    CSB_V1_FIRST_BACKDROP_STEP_F0128_BACKDROP_KEEP_OUT = 3
} CSB_V1_CustomBackgroundsFirstBackdropStep;

typedef struct {
    int contract_only;
    int first_custom_background_call_index;
    int first_custom_background_room_num;
    int second_custom_background_fallback_room_num;
    int first_custom_background_selected_before_second;
    int second_custom_background_is_fallback_after_first;
    int first_relative_forward;
    int first_relative_side;
    int second_relative_forward;
    int second_relative_side;
    int first_keep_out_region_ordinal;
    int second_keep_out_region_ordinal;
    int keep_out_region_differs_from_second_backdrop;
    int explicitly_non_overlapping_with_second_backdrop_gate;
    int f0098_base_pixels_drawn_first;
    int f0128_backdrop_keep_out_applies;
    int c10_transparent_color;
    int c10_transparency_preserved_for_later_routes;
    int routes_through_f0107;
    int routes_through_f0108;
    int routes_through_f0111;
    int routes_through_f0115;
    int custom_background_count;
    int skin_def_graphic_id;
    int skin_def_min_bytes;
    int large_bitmap_skin_def_index;
    int large_mask_skin_def_index;
    int large_bitmap_min_bytes;
    int large_mask_min_bytes;
    int middle_bitmap_skin_def_index;
    int middle_mask_skin_def_index;
    int middle_bitmap_min_bytes;
    int middle_mask_min_bytes;
    int near_bitmap_skin_def_index;
    int near_mask_skin_def_index;
    int near_bitmap_min_bytes;
    int near_mask_min_bytes;
    int near_layer_room_num_limit;
    const char *redmcsb_f0128_viewport_anchor;
    const char *redmcsb_f0098_base_anchor;
    const char *redmcsb_f0128_keep_out_anchor;
    const char *redmcsb_defs_c10_anchor;
    const char *redmcsb_defs_view_square_anchor;
    const char *csb_lineage_first_dispatch_anchor;
    const char *csb_lineage_required_anchors;
    const char *csb_lineage_default_skin_anchor;
    const char *non_overlap_note;
    const char *source_summary;
} CSB_V1_CustomBackgroundsFirstBackdropContract;

const CSB_V1_CustomBackgroundsFirstBackdropContract *
csb_v1_viewport_custom_backgrounds_first_backdrop_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_first_backdrop_order_pc34(
    CSB_V1_CustomBackgroundsFirstBackdropStep *out_steps,
    size_t out_capacity);

const char *
csb_v1_viewport_custom_backgrounds_first_backdrop_source_evidence_pc34(void);

int csb_v1_viewport_custom_backgrounds_first_backdrop_preserve_c10_pc34(
    const unsigned char *source,
    unsigned char *destination,
    size_t count);

#endif
