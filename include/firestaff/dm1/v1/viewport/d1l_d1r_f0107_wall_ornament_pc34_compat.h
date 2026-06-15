#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34 2
#define DM1_V1_D1L_D1R_F0107_STEP_COUNT_PC34 10
#define DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34 6
#define DM1_V1_D1L_D1R_F0107_PIXEL_COUNT_PC34 6
#define DM1_V1_D1L_D1R_F0107_SIBLING_REJECT_COUNT_PC34 4
#define DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D1L_D1R_F0107_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1L_D1R_F0107_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D1L_D1R_F0107_SIDE_D1L_PC34 = 1,
    DM1_V1_D1L_D1R_F0107_SIDE_D1R_PC34 = 2
} DM1_V1_D1LD1RF0107SidePc34;

typedef enum {
    DM1_V1_D1L_D1R_F0107_STEP_F0128_D1L_PC34 = 0,
    DM1_V1_D1L_D1R_F0107_STEP_F0128_D1R_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_F0122_D1L_BODY_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_F0123_D1R_BODY_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_D1L_F0107_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_D1R_F0107_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_F0108_BASELINE_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_F0115_CELL_ORDER_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_F0107_C10_PC34,
    DM1_V1_D1L_D1R_F0107_STEP_SIBLING_NON_OVERLAP_PC34
} DM1_V1_D1LD1RF0107StepKindPc34;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} DM1_V1_D1LD1RF0107RectPc34;

typedef struct {
    int side;
    const char *side_name;
    int view_square;
    int relative_depth;
    int relative_lateral;
    int wall_zone;
    int floor_view;
    int door_zone;
    int body_start_line;
    int body_end_line;
    int wall_case_line;
    int wall_draw_line;
    int f0107_line;
    int f0107_aspect_slot;
    int f0107_view_wall;
    int f0128_update_line;
    int f0128_draw_line;
    unsigned int corridor_order;
    unsigned int door_side_order;
    unsigned int door_pass1_order;
    unsigned int door_pass2_order;
    int f0108_open_line;
    int f0115_line;
    DM1_V1_D1LD1RF0107RectPc34 probe_rect;
    const char *redmcsb_anchor;
} DM1_V1_D1LD1RF0107LanePc34;

typedef struct {
    int step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D1LD1RF0107StepPc34;

typedef struct {
    int ordinal_index_c0_to_c5;
    int sensor_ordinal;
    int accepted_at_d1l;
    int accepted_at_d1r;
    const char *redmcsb_anchor;
} DM1_V1_D1LD1RF0107OrdinalPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *redmcsb_anchor;
} DM1_V1_D1LD1RF0107PixelPc34;

typedef struct {
    const char *sibling_name;
    int reject_cell_position;
    int reject_carrier_zone;
    int reject_view_wall;
    int reject_aspect_ratio;
    int relative_depth;
    int left_lateral;
    int right_lateral;
    int left_carrier_zone;
    int right_carrier_zone;
    int left_view_wall;
    int right_view_wall;
    int aspect_width;
    int aspect_height;
    const char *redmcsb_anchor;
} DM1_V1_D1LD1RF0107SiblingRejectPc34;

typedef struct {
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int viewport_x;
    int viewport_y;
    int c10_transparent_color;
    int wall_ornament_zone_base;
    int wall_ornament_zone_stride;
    int wall_ornament_coordinate_set;
    int d1l_wall_ornament_zone;
    int d1r_wall_ornament_zone;
    int m550_first_thing_slot;
    int m551_right_wall_ornament_slot;
    int m552_front_wall_ornament_slot;
    int m553_left_wall_ornament_slot;
    int f0128_d1l_before_d1r;
    int f0128_d1r_before_d1c;
    int direct_f0107_call_count;
    int side_ornament_call_count;
    int front_ornament_call_count;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_returns_false;
    int f0107_alcove_returns_true;
    int f0107_blit_uses_c10;
    int c10_transparent_preserves_destination;
    int c0_to_c5_ordinals_pinned;
    int f0108_baseline_pinned;
    int f0115_cell_order_pinned;
    int synthetic_probe_collision_count;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    int no_graphics_dat_reads;
    DM1_V1_D1LD1RF0107LanePc34 lanes[DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34];
    DM1_V1_D1LD1RF0107StepPc34 steps[DM1_V1_D1L_D1R_F0107_STEP_COUNT_PC34];
    DM1_V1_D1LD1RF0107OrdinalPc34 ordinals[DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34];
    DM1_V1_D1LD1RF0107PixelPc34 pixels[DM1_V1_D1L_D1R_F0107_PIXEL_COUNT_PC34];
    DM1_V1_D1LD1RF0107SiblingRejectPc34
        sibling_rejects[DM1_V1_D1L_D1R_F0107_SIBLING_REJECT_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D1LD1RF0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D1LD1RF0107LanePc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_lane_at_pc34(size_t index);

const DM1_V1_D1LD1RF0107StepPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_step_at_pc34(size_t index);

const DM1_V1_D1LD1RF0107OrdinalPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_ordinal_at_pc34(size_t index);

const DM1_V1_D1LD1RF0107SiblingRejectPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_sibling_reject_at_pc34(size_t index);

bool dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int side_index,
    int ornament_index_c0_to_c5);

bool dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

int dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall);

uint8_t dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size);

const char *dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
