#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_LANE_COUNT_PC34 2
#define DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_FLOW_COUNT_PC34 7
#define DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34 12
#define DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34 8
#define DM1_V1_D3L_D3R_F0107_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D3L_D3R_F0107_SIDE_D3L_PC34 = 1,
    DM1_V1_D3L_D3R_F0107_SIDE_D3R_PC34 = 2
} DM1_V1_D3LD3RF0107SidePc34;

typedef enum {
    DM1_V1_D3L_D3R_F0107_STEP_F0128_DISPATCH_D3L_PC34 = 0,
    DM1_V1_D3L_D3R_F0107_STEP_F0128_DISPATCH_D3R_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_F0116_D3L_WALL_BODY_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_F0117_D3R_WALL_BODY_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_D3L_SIDE_F0107_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_D3L_FRONT_F0107_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_D3R_SIDE_F0107_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_D3R_FRONT_F0107_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_F0108_BASELINE_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_F0112_BEFORE_F0115_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_F0113_AFTER_F0115_PC34,
    DM1_V1_D3L_D3R_F0107_STEP_TERMINAL_DEPTH_SIDE_PAIR_PC34
} DM1_V1_D3LD3RF0107StepKindPc34;

typedef struct {
    int side;
    const char *side_name;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int wall_zone;
    int floor_view;
    int field_aspect_index;
    int f0128_update_line;
    int f0128_draw_line;
    int dispatcher_line_start;
    int dispatcher_line_end;
    int wall_case_line;
    int wall_zone_draw_line;
    int side_f0107_line;
    int side_ornament_slot;
    int side_view_wall;
    int front_f0107_line;
    int front_ornament_slot;
    int front_view_wall;
    int alcove_order;
    int corridor_order;
    int door_side_order;
    int door_pass1_order;
    int door_pass2_order;
    int f0108_door_front_line;
    int f0108_open_path_line;
    int f0112_ceiling_line;
    int f0115_line;
    int f0113_field_line;
    int f0108_before_f0112;
    int f0112_before_f0115;
    int f0115_before_f0113;
    int f0111_door_zone;
    int f0111_line;
    const char *redmcsb_anchor;
} DM1_V1_D3LD3RF0107LanePc34;

typedef struct {
    int ordinal_position;
    const char *position_name;
    int aspect_slot;
    int view_wall;
    int reaches_d3l_d3r_f0107;
    int side;
    int is_front_wall;
    int sensor_provided;
    const char *redmcsb_anchor;
} DM1_V1_D3LD3RF0107OrdinalFlowPc34;

typedef struct {
    int step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D3LD3RF0107StepPc34;

typedef struct {
    int ordinal_position;
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *anchor;
} DM1_V1_D3LD3RF0107PixelPc34;

typedef struct {
    int view_square_d3l;
    int view_square_d3r;
    int wall_zone_d3l;
    int wall_zone_d3r;
    int floor_view_d3l;
    int floor_view_d3r;
    int field_aspect_d3l;
    int field_aspect_d3r;
    int c10_transparent_color;
    int c1004_wall_ornament_zone_base;
    int wall_ornament_zone_stride;
    int c1500_floor_ornament_zone_base;
    int g0205_wall_ornament_coordinate_sets;
    int g0206_floor_ornament_coordinate_sets;
    int g0207_door_ornament_coordinate_sets;
    int g0208_door_button_coordinate_sets;
    int f0128_d3l_then_d3r;
    int f0128_d3_pair_before_d3c;
    int f0128_d3_pair_before_d2_pair;
    int spatially_deeper_than_d2_pair;
    int direct_f0107_call_count;
    int sensor_position_count;
    int side_ornament_call_count;
    int front_ornament_call_count;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_returns_false;
    int f0107_alcove_returns_true;
    int f0107_blit_uses_c10;
    int c10_transparent_preserves_destination;
    int f0108_floor_baseline_before_f0115;
    int f0112_ceiling_pit_before_f0115;
    int f0113_teleporter_field_after_f0115;
    int d3l_cell_order_terminal_depth;
    int d3r_cell_order_terminal_depth;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    DM1_V1_D3LD3RF0107LanePc34 lanes[DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_LANE_COUNT_PC34];
    DM1_V1_D3LD3RF0107OrdinalFlowPc34 ordinals[DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_FLOW_COUNT_PC34];
    DM1_V1_D3LD3RF0107StepPc34 steps[DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34];
    DM1_V1_D3LD3RF0107PixelPc34 pixels[DM1_V1_D3L_D3R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D3LD3RF0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D3LD3RF0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D3LD3RF0107LanePc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_lane_at_pc34(size_t index);

const DM1_V1_D3LD3RF0107StepPc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_step_at_pc34(size_t index);

const DM1_V1_D3LD3RF0107OrdinalFlowPc34 *
dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_ordinal_at_pc34(size_t index);

bool dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

int dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall);

uint8_t dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int dm1_v1_viewport_d3l_d3r_f0107_decode_cell_order_pc34(
    unsigned int cell_order,
    int ordinal_index);

const char *dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
