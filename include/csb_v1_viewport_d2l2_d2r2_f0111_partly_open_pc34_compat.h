#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0111_PARTLY_OPEN_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_F0111_PARTLY_OPEN_PC34_COMPAT_H

#include "csb_v1_viewport_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_C10_COLOR_FLESH 10
#define CSB_V1_D2L2_D2R2_F0111_PARTLY_OPEN_MASK0x4000 0x4000

typedef struct {
    int source_locked_contract_only;
    int no_real_asset_pixel_parity;
    int view_square;
    int f0128_draw_order_index;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int wall_zone_binding;
    int direct_f0111_route_present;
    int direct_f0115_rear_front_route_present;
    int back_wall_ornament_f0107_route_present;
    int wall_case_returns_before_f0111;
    int c3700_d3_door_zone_metadata_excluded;
    int door_zone_base;
    int coord_closed_record_type;
    int coord_parent_record;
    int coord_clip_record;
    int coord_frame_x;
    int coord_frame_y;
    int native_bitmap_width;
    int native_bitmap_height;
    int clipped_width;
    int clipped_height;
    int open_state;
    int partly_state_one;
    int partly_state_two;
    int closed_state;
    int destroyed_state;
    int first_half_zone_offset;
    int final_half_zone_offset;
    int final_half_mask;
    int transparent_color;
    int c10_skip_enabled;
    int synthetic_blit_uses_d2_panel_clip;
    int lineage_f3l1_binding_present;
    int lineage_frame_before_door;
    int lineage_draw_order_rear;
    int lineage_draw_order_front;
    const char *route_name;
    const char *redmcsb_dispatcher_lines;
    const char *f0111_source_lines;
    const char *coord_source_lines;
    const char *defs_source_lines;
    const char *lineage_source_lines;
} CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34;

size_t csb_v1_viewport_d2l2_d2r2_f0111_partly_open_count_pc34(void);

const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_at_pc34(size_t index);

const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(int view_square);

const CSB_V1_ViewportDoorPanelBlitSpec *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_context_panel_pc34(void);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int door_state,
    int horizontal_door);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int zone,
    int *out_record_type,
    int *out_x,
    int *out_y);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_resolve_clip_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int zone,
    int *out_x,
    int *out_y);

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *csb_v1_viewport_d2l2_d2r2_f0111_partly_open_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
