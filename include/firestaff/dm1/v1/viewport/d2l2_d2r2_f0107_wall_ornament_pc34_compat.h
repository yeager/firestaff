#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34 2
#define DM1_V1_D2L2_D2R2_F0107_CALL_COUNT_PC34 2
#define DM1_V1_D2L2_D2R2_F0107_STEP_COUNT_PC34 10
#define DM1_V1_D2L2_D2R2_F0107_ORDINAL_COUNT_PC34 6
#define DM1_V1_D2L2_D2R2_F0107_PIXEL_COUNT_PC34 6
#define DM1_V1_D2L2_D2R2_F0107_SIBLING_REJECT_COUNT_PC34 8
#define DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D2L2_D2R2_F0107_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2L2_D2R2_F0107_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34 10

typedef enum {
    DM1_V1_D2L2_D2R2_F0107_SIDE_D2L2_PC34 = 1,
    DM1_V1_D2L2_D2R2_F0107_SIDE_D2R2_PC34 = 2
} DM1_V1_D2L2D2R2F0107SidePc34;

typedef enum {
    DM1_V1_D2L2_D2R2_F0107_STEP_F0128_D2L2_PC34 = 0,
    DM1_V1_D2L2_D2R2_F0107_STEP_F0128_D2R2_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_F0119_D2L_BODY_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_F0120_D2R_BODY_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_D2L2_F0107_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_D2R2_F0107_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_F0108_BASELINE_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_ZONE_MATH_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_C10_PC34,
    DM1_V1_D2L2_D2R2_F0107_STEP_SIBLING_NON_OVERLAP_PC34
} DM1_V1_D2L2D2R2F0107StepKindPc34;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} DM1_V1_D2L2D2R2F0107RectPc34;

typedef struct {
    int side;
    const char *side_name;
    int guard_view_square;
    int carrier_view_square;
    int relative_depth;
    int relative_lateral;
    int guard_wall_zone;
    int carrier_wall_zone;
    int f0678_f0679_start_line;
    int f0678_f0679_end_line;
    int carrier_body_start_line;
    int carrier_body_end_line;
    int carrier_wall_case_line;
    int carrier_wall_draw_line;
    int f0107_line;
    int f0107_aspect_slot;
    int f0107_view_wall;
    int f0128_guard_update_line;
    int f0128_guard_draw_line;
    int f0128_carrier_update_line;
    int f0128_carrier_draw_line;
    DM1_V1_D2L2D2R2F0107RectPc34 probe_rect;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2F0107LanePc34;

typedef struct {
    int call_index;
    int side;
    int aspect_slot;
    const char *aspect_slot_name;
    int view_wall;
    const char *view_wall_name;
    int call_line;
    int coordinate_set;
    int zone;
    int accepts_c0_to_c5;
    int alcove_boolean_pinned;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2F0107CallPc34;

typedef struct {
    int step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2F0107StepPc34;

typedef struct {
    int ordinal_index_c0_to_c5;
    int sensor_ordinal;
    int accepted_at_d2l2;
    int accepted_at_d2r2;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2F0107OrdinalPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2F0107PixelPc34;

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
} DM1_V1_D2L2D2R2F0107SiblingRejectPc34;

typedef struct {
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int c10_transparent_color;
    int wall_ornament_zone_base;
    int wall_ornament_zone_stride;
    int wall_ornament_coordinate_set;
    int d2l2_wall_ornament_zone;
    int d2r2_wall_ornament_zone;
    int d2l2_guard_wall_zone;
    int d2r2_guard_wall_zone;
    int m550_first_thing_slot;
    int m551_right_wall_ornament_slot;
    int m552_front_wall_ornament_slot;
    int m553_left_wall_ornament_slot;
    int f0128_d2l2_before_d2r2;
    int f0128_d2r2_before_d2l_d2r_d2c;
    int f0119_f0120_body_pinned;
    int direct_f0107_call_count;
    int side_ornament_call_count;
    int front_ornament_call_count;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_returns_false;
    int f0107_alcove_returns_true;
    int f0107_blit_uses_c10;
    int c10_transparent_preserves_destination;
    int c0_to_c5_ordinals_pinned;
    int f0108_floor_ceiling_baseline_separate;
    int zone_math_pinned;
    int synthetic_probe_collision_count;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    int no_graphics_dat_reads;
    int redmcsb_c707_c708_zone_label_deviation_documented;
    DM1_V1_D2L2D2R2F0107LanePc34 lanes[DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34];
    DM1_V1_D2L2D2R2F0107CallPc34 calls[DM1_V1_D2L2_D2R2_F0107_CALL_COUNT_PC34];
    DM1_V1_D2L2D2R2F0107StepPc34 steps[DM1_V1_D2L2_D2R2_F0107_STEP_COUNT_PC34];
    DM1_V1_D2L2D2R2F0107OrdinalPc34 ordinals[DM1_V1_D2L2_D2R2_F0107_ORDINAL_COUNT_PC34];
    DM1_V1_D2L2D2R2F0107PixelPc34 pixels[DM1_V1_D2L2_D2R2_F0107_PIXEL_COUNT_PC34];
    DM1_V1_D2L2D2R2F0107SiblingRejectPc34
        sibling_rejects[DM1_V1_D2L2_D2R2_F0107_SIBLING_REJECT_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D2L2D2R2F0107LanePc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_lane_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0107CallPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_call_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0107StepPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_step_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0107OrdinalPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_ordinal_at_pc34(size_t index);

const DM1_V1_D2L2D2R2F0107SiblingRejectPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_sibling_reject_at_pc34(size_t index);

bool dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int side_index,
    int ornament_index_c0_to_c5);

bool dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

int dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall);

uint8_t dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size);

const char *dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
