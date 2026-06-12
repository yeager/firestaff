#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_MASK_AFTER_FLOOR_CEILING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_MASK_AFTER_FLOOR_CEILING_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CSB_V1_MASK_AFTER_FLOOR_STEP_F0098_FLOOR_CEILING = 0,
    CSB_V1_MASK_AFTER_FLOOR_STEP_G0297_RESET = 1,
    CSB_V1_MASK_AFTER_FLOOR_STEP_APPLYBACKGROUND_MASK = 2,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_LARGE = 3,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_MIDDLE = 4,
    CSB_V1_MASK_AFTER_FLOOR_STEP_ROOM_BITMAP_NEAR = 5,
    CSB_V1_MASK_AFTER_FLOOR_STEP_STATE_DEFAULT = 6
} CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep;

typedef struct {
    int layer_ordinal;
    int room_num;
    int bitmap_skin_def_index;
    int mask_skin_def_index;
    int bitmap_graphic_id;
    int mask_graphic_id;
    int applies_for_room_num;
    const char *layer_name;
    const char *source_lines;
} CSB_V1_CustomBackgroundsMaskAfterFloorCeilingLayer;

typedef struct {
    int room_num;
    int had_custom_background;
    int state_default;
    int floor_drawn;
    int ceiling_drawn;
    int g0297_initial_requested;
    int g0297_after_floor_ceiling;
    int g0297_before_applybackground_mask;
    int g0297_after_room_bitmap;
    int applybackground_mask_applied;
    int room_bitmap_apply_count;
    int room_bitmap_large_applied;
    int room_bitmap_middle_applied;
    int room_bitmap_near_applied;
    int floor_ceiling_before_mask;
    int mask_before_room_bitmap;
    int reset_before_mask;
    size_t order_count;
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep order[8];
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingLayer layers[3];
    const char *source_lines;
} CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult;

typedef struct {
    int contract_only;
    int f0098_floor_ceiling_step;
    int g0297_reset_step;
    int applybackground_mask_step;
    int room_bitmap_large_step;
    int room_bitmap_middle_step;
    int room_bitmap_near_step;
    int state_default_step;
    int skin_def_min_words;
    int large_bitmap_skin_def_index;
    int large_mask_skin_def_index;
    int middle_bitmap_skin_def_index;
    int middle_mask_skin_def_index;
    int near_bitmap_skin_def_index;
    int near_mask_skin_def_index;
    int near_layer_room_num_limit;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_defs_view_square_anchor;
    const char *csb_lineage_applybackground_anchor;
    const char *csb_lineage_bitmap_application_anchor;
    const char *csbwin_viewport_url;
    const char *source_summary;
} CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract;

const CSB_V1_CustomBackgroundsMaskAfterFloorCeilingContract *
csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_contract_pc34(void);

size_t csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_order_pc34(
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingStep *out_steps,
    size_t out_capacity);

int csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_run_pc34(
    int room_num,
    int draw_floor_and_ceiling_requested,
    int has_custom_background,
    const uint16_t *skin_def_words,
    size_t skin_def_word_count,
    CSB_V1_CustomBackgroundsMaskAfterFloorCeilingResult *out_result);

const char *
csb_v1_viewport_custom_backgrounds_mask_after_floor_ceiling_source_evidence_pc34(void);

#endif
