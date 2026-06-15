#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2L_D2R_F0107_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2L_D2R_F0107_ORNAMENT_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_VIEWPORT_D2L_D2R_F0107_SIDE_D2L_PC34 = 0,
    CSB_V1_VIEWPORT_D2L_D2R_F0107_SIDE_D2R_PC34 = 1
} CSB_V1_ViewportD2LD2RF0107SidePc34;

typedef enum {
    CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_WALL_PC34 = 0,
    CSB_V1_VIEWPORT_D2L_D2R_F0107_ELEMENT_DOOR_FRONT_PC34 = 17
} CSB_V1_ViewportD2LD2RF0107ElementPc34;

typedef struct {
    int source_locked_contract_only;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int wall_element;
    int door_front_element;
    int wall_set_bitmap_index;
    int wall_frame_view_square;
    int wall_zone;
    int f0100_wall_blit;
    int f0100_source_bitmap_resolved;
    int f0100_frame_resolved;
    int f0100_transparent_color;
    int f0100_destination_byte_width;
    int f0107_side_ordinal;
    int f0107_side_view_wall;
    int f0107_front_ordinal;
    int f0107_front_view_wall;
    int f0107_side_before_front;
    int f0107_front_conditional_branch;
    int f0108_door_front_floor_view;
    int f0111_wall_case_call_count;
    int f0111_door_case_only;
    int csb_viewport_script_side_wall_location;
    int csb_viewport_script_front_wall_location;
    int csb_viewport_script_alcove_jump;
    const char *route_name;
    const char *redmcsb_function;
    const char *redmcsb_lines;
    const char *csb_viewport_lines;
} CSB_V1_ViewportD2LD2RF0107RouteSpecPc34;

typedef struct {
    int ok;
    int wall_blit_calls;
    int wall_blit_before_f0107;
    int f0107_side_calls;
    int f0107_front_calls;
    int f0107_side_return_alcove;
    int f0107_front_return_alcove;
    int f0107_front_branch_taken;
    int f0107_front_branch_fallthrough;
    int f0107_calls_compose;
    int f0108_floor_ornament_calls;
    int f0108_floor_ornament_view;
    int f0111_wall_calls;
    int f0111_door_case_only;
    int f0100_transparent_color;
    int f0100_destination_byte_width;
    int source_bitmap_resolved;
    int frame_resolved;
    const char *source_lock_evidence;
} CSB_V1_ViewportD2LD2RF0107RunResultPc34;

size_t csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_count_pc34(void);

const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *
csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(size_t index);

const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *
csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_for_side_pc34(
    CSB_V1_ViewportD2LD2RF0107SidePc34 side);

int csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *spec,
    int element,
    int side_f0107_returns_alcove,
    int front_f0107_returns_alcove,
    CSB_V1_ViewportD2LD2RF0107RunResultPc34 *out_result);

const char *csb_v1_viewport_d2l_d2r_f0107_ornament_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
