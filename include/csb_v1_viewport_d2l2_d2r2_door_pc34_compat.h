#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_D2R2_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int source_locked_contract_only;
    int view_square;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int wall_precedes_door_zone;
    int f0678_f0679_has_direct_f0111_route;
    int wall_case_returns_before_f0111;
    int door_zone_base;
    int door_zone_record_type;
    int door_panel_parent_record;
    int door_panel_clip_record;
    int frame_x;
    int frame_y;
    int clipped_width;
    int clipped_height;
    int c03_layout_range;
    int frame_bitmap_command;
    int frame_bitmap_index;
    int frame_rect_command;
    int frame_rect_index;
    int frame_blit_command;
    int frame_blit_is_mirrored;
    int door_graphic_command;
    int door_graphic_index;
    int door_graphic_size;
    int door_nearness;
    int transparent_color;
    int preserves_c10_transparency;
    const char *redmcsb_dispatcher;
    const char *csb_source_function;
    const char *source_lines;
} CSB_V1_ViewportD2L2D2R2DoorRouteSpec;

size_t csb_v1_viewport_d2l2_d2r2_door_route_spec_count_pc34(void);

const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *
csb_v1_viewport_d2l2_d2r2_door_route_spec_at_pc34(size_t index);

const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *
csb_v1_viewport_d2l2_d2r2_door_route_spec_for_square_pc34(int view_square);

int csb_v1_viewport_d2l2_d2r2_door_resolve_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec,
    int zone_x,
    int zone_y,
    int *out_x,
    int *out_y);

int csb_v1_viewport_d2l2_d2r2_door_apply_c03_frame_clip_pc34(
    const CSB_V1_ViewportD2L2D2R2DoorRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *csb_v1_viewport_d2l2_d2r2_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
