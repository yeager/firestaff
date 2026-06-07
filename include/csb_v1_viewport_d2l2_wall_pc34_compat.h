#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L2_WALL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L2_WALL_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int source_locked_contract_only;
    int view_square;
    int square_aspect_element_slot;
    int wall_element;
    int teleporter_element;
    int wall_zone;
    int native_wall_index_base;
    int native_wall_index_pc_fix_delta;
    int native_wall_index_pc34_effective;
    int media709_flipped_wall_index;
    int f0104_wall_route;
    int f0105_media709_flipped_route;
    int f0113_teleporter_route;
    int f0107_wall_ornament_route;
    int f0111_door_route;
    int f0115_thing_pass_route;
    int transparent_color;
    int preserves_c10_transparency;
    int frame_x1;
    int frame_x2;
    int frame_y1;
    int frame_y2;
    int byte_width;
    int height;
    int source_x;
    int source_y;
    int teleporter_field_aspect_index;
    int f0128_draw_order_index;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int csb_viewport_pwallbitmap_left_pair_index;
    int csb_viewport_pwallbitmap_right_pair_index;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportD2L2WallRouteSpec;

const CSB_V1_ViewportD2L2WallRouteSpec *
csb_v1_viewport_d2l2_wall_route_spec_pc34(void);

const char *csb_v1_viewport_d2l2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
