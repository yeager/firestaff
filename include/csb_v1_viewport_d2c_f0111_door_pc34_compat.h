#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_DOOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0111_DOOR_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D2C_F0111_DOOR_PC34_C10_COLOR_FLESH 10

typedef struct {
    int source_locked_contract_only;
    int view_square;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int f0121_center_route_present;
    int wall_ordinal_c09_d2c;
    int media508_wall_zone_c707;
    int media720_wall_zone_c709;
    int wall_case_returns_before_f0111;
    int wall_case_calls_f0100;
    int wall_case_calls_f0105;
    int wall_case_calls_f0107;
    int wall_case_calls_f0111;
    int wall_case_calls_c3700_door_panel;
    int c01_corridor_enters_no_wall_path;
    int c05_teleporter_enters_no_wall_path;
    int no_wall_path_calls_f0100;
    int no_wall_path_calls_f0105;
    int no_wall_path_calls_f0107;
    int no_wall_path_calls_f0111;
    int no_wall_path_calls_f0113;
    int no_wall_path_draws_floor_ornament;
    int no_wall_path_draws_ceiling_pit;
    int no_wall_path_draws_f0115_before_field;
    int no_wall_path_cell_order;
    int no_wall_path_field_zone;
    int no_wall_path_preserves_c10_transparency;
    int transparent_color;
    int c3700_door_zone;
    int c3700_is_d3l2_door_zone;
    int d2c_uses_c3700_door_zone;
    int d2c_c3700_panel_path_rejected;
    int coord_clip_record;
    int coord_parent_record;
    int coord_clip_width;
    int coord_clip_height;
    int coord_frame_x;
    int coord_frame_y;
    int d2c_uses_coord_door_record_path;
    int lineage_open_binding_present;
    int lineage_teleporter_binding_present;
    int lineage_frame_blt_binding_present;
    int lineage_frame_rect_binding_present;
    const char *f0121_source_lines;
    const char *wall_case_source_lines;
    const char *no_wall_source_lines;
    const char *lineage_source_lines;
} CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34;

const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *
csb_v1_viewport_d2c_f0111_door_non_route_spec_pc34(void);

int csb_v1_viewport_d2c_f0111_door_zone_from_wall_spec_pc34(
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec);

int csb_v1_viewport_d2c_f0111_door_reject_c3700_panel_path_pc34(
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec,
    int zone_x,
    int zone_y,
    int *out_x,
    int *out_y);

int csb_v1_viewport_d2c_f0111_door_apply_c10_field_blit_pc34(
    const CSB_V1_ViewportD2CF0111DoorNonRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *csb_v1_viewport_d2c_f0111_door_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
