#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_DOOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int rear_f0115_order;
    int front_f0115_order;
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
    const char *redmcsb_function;
    const char *csb_source_function;
    const char *source_lines;
} CSB_V1_ViewportD3L2DoorRouteSpec;

const CSB_V1_ViewportD3L2DoorRouteSpec *
csb_v1_viewport_d3l2_door_route_spec_pc34(void);

int csb_v1_viewport_d3l2_door_resolve_zone_pc34(
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec,
    int zone_x,
    int zone_y,
    int *out_x,
    int *out_y);

int csb_v1_viewport_d3l2_door_apply_c03_frame_clip_pc34(
    const CSB_V1_ViewportD3L2DoorRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *csb_v1_viewport_d3l2_door_source_evidence_pc34(void);

#endif
