#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3C_F0107_F0108_FIRST_BACKDROP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3C_F0107_F0108_FIRST_BACKDROP_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34 224
#define CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_HEIGHT_PC34 136
#define CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_TRANSPARENT_COLOR_PC34 10

typedef enum {
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_NONE = 0,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_BACKDROP = 1,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0107_WALL_ORNAMENT = 2,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0108_FLOOR_ORNAMENT = 3
} CSB_V1_ViewportD3cF0107F0108FirstBackdropStepPc34;

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34;

typedef struct {
    int contract_only;
    int no_game_data_dependency;
    int viewport_width;
    int viewport_height;
    int view_square_d3c;
    int view_wall_d3c_front;
    int view_floor_d3c;
    int wall_ornament_zone_base;
    int wall_ornament_coordinate_set_stride;
    int floor_ornament_zone_base;
    int floor_ornament_coordinate_set_stride;
    int transparent_color;
    int wall_ornament_ordinal;
    int wall_ornament_index;
    int floor_ornament_ordinal;
    int floor_ornament_index;
    int first_backdrop_room_slot;
    int first_backdrop_is_before_cell_routes;
    int f0107_before_f0108;
    int f0108_transparent_mask_preserves_destination;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 d3c_window;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 wall_ornament_window;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 floor_ornament_window;
    const char *redmcsb_f0097_anchor;
    const char *redmcsb_f0098_f0128_backdrop_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0118_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract;

typedef struct {
    int ok;
    int draw_step_count;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropStepPc34 draw_steps[3];
    int wall_ornament_ordinal;
    int wall_ornament_index;
    int floor_ornament_ordinal;
    int floor_ornament_index;
    int first_backdrop_room_slot;
    int wall_ornament_zone;
    int floor_ornament_zone;
    int backdrop_color;
    int wall_ornament_color;
    int floor_ornament_color;
    int masked_floor_source_color;
    int f0107_before_f0108;
    int f0108_mask_preserves_f0107;
    int distinct_layer_colors;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 d3c_window;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 wall_ornament_window;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 floor_ornament_window;
    int backdrop_only_x;
    int backdrop_only_y;
    int wall_only_x;
    int wall_only_y;
    int floor_opaque_x;
    int floor_opaque_y;
    int overlap_masked_x;
    int overlap_masked_y;
    const char *source_evidence;
} CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34;

typedef struct {
    int ok;
    int final_backdrop_only_pixel;
    int final_wall_only_pixel;
    int final_floor_opaque_pixel;
    int final_overlap_masked_pixel;
    int pixel_before_f0108_at_masked_overlap;
    int pixel_after_f0108_at_masked_overlap;
    int pixel_before_f0108_at_opaque_floor;
    int pixel_after_f0108_at_opaque_floor;
    int f0108_mask_did_not_erase_f0107;
    int f0108_opaque_pixel_overwrote_destination;
    int backdrop_pixels;
    int wall_ornament_pixels;
    int floor_ornament_opaque_pixels;
    int floor_ornament_masked_pixels;
    int overlap_pixels;
    int draw_step_count;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropStepPc34 draw_steps[3];
    const char *source_evidence;
} CSB_V1_ViewportD3cF0107F0108FirstBackdropResultPc34;

const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_contract_pc34(void);

const char *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34(void);

int csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(
    int wall_ornament_ordinal,
    int floor_ornament_ordinal,
    int first_backdrop_room_slot,
    CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 *out_plan);

int csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
    const CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 *plan,
    uint8_t *viewport,
    size_t viewport_len,
    CSB_V1_ViewportD3cF0107F0108FirstBackdropResultPc34 *out_result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D3C_F0107_F0108_FIRST_BACKDROP_PC34_COMPAT_H */
