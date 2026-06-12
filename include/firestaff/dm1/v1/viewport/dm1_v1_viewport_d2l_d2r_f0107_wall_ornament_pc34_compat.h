#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2L_D2R_F0107_SIDE_COUNT_PC34 2
#define DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34 4
#define DM1_V1_D2L_D2R_F0107_STEP_COUNT_PC34 10
#define DM1_V1_D2L_D2R_F0107_CELL_COUNT_PC34 4
#define DM1_V1_D2L_D2R_F0107_ZONE_COUNT_PC34 8
#define DM1_V1_D2L_D2R_F0107_PIXEL_COUNT_PC34 6
#define DM1_V1_D2L_D2R_F0107_DOOR_STATE_COUNT_PC34 6
#define DM1_V1_D2L_D2R_F0107_ORDINAL_COUNT_PC34 6
#define DM1_V1_D2L_D2R_F0107_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D2L_D2R_F0107_SIDE_D2L_PC34 = 1,
    DM1_V1_D2L_D2R_F0107_SIDE_D2R_PC34 = 2
} DM1_V1_D2LD2RF0107SidePc34;

typedef enum {
    DM1_V1_D2L_D2R_F0107_STEP_F0128_D2L_PC34 = 0,
    DM1_V1_D2L_D2R_F0107_STEP_F0128_D2R_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_F0119_D2L_BODY_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_F0120_D2R_BODY_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_F0108_BASELINE_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_F0107_SIDE_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_F0107_FRONT_ALCOVE_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_F0111_DOOR_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_ZONE_MATH_PC34,
    DM1_V1_D2L_D2R_F0107_STEP_C10_PC34
} DM1_V1_D2LD2RF0107StepKindPc34;

typedef struct {
    DM1_V1_D2LD2RF0107SidePc34 side;
    const char *name;
    const char *body_name;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int f0128_update_line;
    int f0128_draw_line;
    int body_start_line;
    int body_end_line;
    int wall_case_line;
    int wall_zone;
    int wall_draw_line;
    int side_f0107_line;
    int front_f0107_line;
    int side_slot;
    int front_slot;
    int side_view_wall;
    int front_view_wall;
    int f0108_baseline_line;
    int f0111_line;
    int door_zone;
    unsigned int corridor_order;
    unsigned int door_pass1_order;
    unsigned int door_pass2_order;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107SideSpecPc34;

typedef struct {
    int call_index;
    DM1_V1_D2LD2RF0107SidePc34 side;
    int aspect_slot;
    const char *slot_name;
    int view_wall;
    const char *view_wall_name;
    int call_line;
    int zone;
    int accepts_c0_to_c5;
    int alcove_enables_f0115;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107CallPc34;

typedef struct {
    DM1_V1_D2LD2RF0107StepKindPc34 step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107StepPc34;

typedef struct {
    int requested_cell_index;
    const char *cell_name;
    int f0115_nibble;
    int is_front;
    int is_back;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107CellPc34;

typedef struct {
    const char *source_table;
    int base_zone;
    int coordinate_set;
    int view_index;
    int stride;
    int expected_zone;
    const char *purpose;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107ZonePc34;

typedef struct {
    int ordinal_index_c0_to_c5;
    int sensor_ordinal;
    int accepted_at_all_call_sites;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107OrdinalPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *anchor;
} DM1_V1_D2LD2RF0107PixelPc34;

typedef struct {
    int door_state;
    int open_rejects_blit;
    int draws_c10_blit;
    int partly_open_half_blit_uses_c10;
    int accepted_for_d2l;
    int accepted_for_d2r;
    const char *redmcsb_anchor;
} DM1_V1_D2LD2RF0107DoorStatePc34;

typedef struct {
    int view_square_d2l;
    int view_square_d2r;
    int view_wall_d2l_right;
    int view_wall_d2l_front;
    int view_wall_d2r_left;
    int view_wall_d2r_front;
    int wall_zone_d2l;
    int wall_zone_d2r;
    int door_zone_d2l;
    int door_zone_d2r;
    int c10_transparent_color;
    int first_thing_slot;
    int right_wall_ornament_slot;
    int front_wall_ornament_slot;
    int left_wall_ornament_slot;
    int f0128_d2l_before_d2r;
    int f0108_baseline_before_f0107_contract;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_returns_false;
    int f0107_alcove_returns_true;
    int f0107_blit_uses_c10;
    int c10_preserves_destination;
    int f0111_open_rejects_blit;
    int f0111_non_open_accepts_blit;
    int f0111_partly_open_uses_c10;
    int all_call_sites_accept_c0_to_c5;
    int front_cells_are_0_1;
    int back_cells_are_2_3;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    int no_graphics_dat_reads;
    int disjoint_from_d0l_d0r_and_d1c;
    int helper_f0107_slot_constants_reused;
    DM1_V1_D2LD2RF0107SideSpecPc34 sides[DM1_V1_D2L_D2R_F0107_SIDE_COUNT_PC34];
    DM1_V1_D2LD2RF0107CallPc34 calls[DM1_V1_D2L_D2R_F0107_CALL_COUNT_PC34];
    DM1_V1_D2LD2RF0107StepPc34 steps[DM1_V1_D2L_D2R_F0107_STEP_COUNT_PC34];
    DM1_V1_D2LD2RF0107CellPc34 cells[DM1_V1_D2L_D2R_F0107_CELL_COUNT_PC34];
    DM1_V1_D2LD2RF0107ZonePc34 zones[DM1_V1_D2L_D2R_F0107_ZONE_COUNT_PC34];
    DM1_V1_D2LD2RF0107OrdinalPc34 ordinals[DM1_V1_D2L_D2R_F0107_ORDINAL_COUNT_PC34];
    DM1_V1_D2LD2RF0107PixelPc34 pixels[DM1_V1_D2L_D2R_F0107_PIXEL_COUNT_PC34];
    DM1_V1_D2LD2RF0107DoorStatePc34 door_states[DM1_V1_D2L_D2R_F0107_DOOR_STATE_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D2LD2RF0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D2LD2RF0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D2LD2RF0107SideSpecPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_side_at_pc34(size_t index);

const DM1_V1_D2LD2RF0107CallPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_call_at_pc34(size_t index);

const DM1_V1_D2LD2RF0107StepPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_step_at_pc34(size_t index);

const DM1_V1_D2LD2RF0107CellPc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_cell_at_pc34(size_t index);

const DM1_V1_D2LD2RF0107ZonePc34 *
dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_zone_at_pc34(size_t index);

bool dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

bool dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int call_index,
    int ornament_index_c0_to_c5);

uint8_t dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d2l_d2r_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
