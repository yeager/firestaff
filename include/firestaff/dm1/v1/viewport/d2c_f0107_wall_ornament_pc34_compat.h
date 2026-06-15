#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D2C_F0107_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D2C_F0107_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D2C_F0107_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2C_F0107_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2C_F0107_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34 6
#define DM1_V1_D2C_F0107_STEP_COUNT_PC34 8
#define DM1_V1_D2C_F0107_SISTER_COUNT_PC34 4

typedef enum {
    DM1_V1_D2C_F0107_STEP_F0128_D2C_DISPATCH_PC34 = 0,
    DM1_V1_D2C_F0107_STEP_F0121_D2C_BODY_PC34,
    DM1_V1_D2C_F0107_STEP_WALL_BODY_PC34,
    DM1_V1_D2C_F0107_STEP_F0107_FRONT_WALL_ORNAMENT_PC34,
    DM1_V1_D2C_F0107_STEP_F0115_ALCOVE_ONLY_PC34,
    DM1_V1_D2C_F0107_STEP_F0108_KEEP_OUT_PC34,
    DM1_V1_D2C_F0107_STEP_F0111_KEEP_OUT_PC34,
    DM1_V1_D2C_F0107_STEP_SYNTHETIC_FRAMEBUFFER_PC34
} DM1_V1_D2CF0107StepKindPc34;

typedef struct {
    DM1_V1_D2CF0107StepKindPc34 step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0107StepPc34;

typedef struct {
    int ordinal_index_c0_to_c5;
    int sensor_ordinal;
    int aspect_slot;
    int reaches_d2c_f0107;
    int accepted_by_f0107_body;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0107OrdinalPc34;

typedef struct {
    const char *name;
    int x;
    int y;
    int width;
    int height;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0107ProbeBoxPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *redmcsb_anchor;
} DM1_V1_D2CF0107PixelPc34;

typedef struct {
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int viewport_x_first;
    int viewport_y_first;
    int viewport_x_last;
    int viewport_y_last;
    int view_square_d2c;
    int relative_depth;
    int relative_lateral;
    int c_coordinate;
    int y_coordinate;
    int view_wall_d2c_front;
    int wall_zone_d2c;
    int wall_index_d2c;
    int floor_view_d2c;
    int front_wall_ornament_slot;
    int first_thing_slot;
    int f0128_update_line;
    int f0128_draw_line;
    int f0128_after_d2l_d2r;
    int f0128_before_d1_d0;
    int body_function_start_line;
    int body_function_end_line;
    int wall_case_line;
    int wall_draw_first_line;
    int wall_draw_last_line;
    int f0107_call_line;
    int f0107_alcove_order_line;
    int wall_case_return_line;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_returns_false;
    int f0107_alcove_returns_true;
    int f0107_blit_uses_c10;
    int c10_preserves_destination;
    int c0_to_c5_ordinals_pinned;
    int only_m552_reaches_d2c;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    int no_graphics_dat_reads;
    DM1_V1_D2CF0107StepPc34 steps[DM1_V1_D2C_F0107_STEP_COUNT_PC34];
    DM1_V1_D2CF0107OrdinalPc34 ordinals[DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34];
    DM1_V1_D2CF0107ProbeBoxPc34 d2c_probe_box;
    DM1_V1_D2CF0107ProbeBoxPc34 sister_boxes[DM1_V1_D2C_F0107_SISTER_COUNT_PC34];
    DM1_V1_D2CF0107PixelPc34 pixels[DM1_V1_D2C_F0107_ORDINAL_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D2CF0107WallOrnamentModelPc34;

typedef struct {
    int writes;
    int transparent_skips;
    int touched_pixels;
    int d2c_probe_overlaps_sister;
    uint32_t framebuffer_hash;
} DM1_V1_D2CF0107FramebufferProbePc34;

bool dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D2CF0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D2CF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d2c_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d2c_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D2CF0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d2c_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D2CF0107StepPc34 *
dm1_v1_viewport_d2c_f0107_wall_ornament_step_at_pc34(size_t index);

const DM1_V1_D2CF0107OrdinalPc34 *
dm1_v1_viewport_d2c_f0107_wall_ornament_ordinal_at_pc34(size_t index);

bool dm1_v1_viewport_d2c_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int ornament_index_c0_to_c5);

bool dm1_v1_viewport_d2c_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

uint8_t dm1_v1_viewport_d2c_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

bool dm1_v1_viewport_d2c_f0107_wall_ornament_boxes_overlap_pc34(
    const DM1_V1_D2CF0107ProbeBoxPc34 *a,
    const DM1_V1_D2CF0107ProbeBoxPc34 *b);

bool dm1_v1_viewport_d2c_f0107_wall_ornament_probe_framebuffer_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_len,
    DM1_V1_D2CF0107FramebufferProbePc34 *out_probe);

const char *dm1_v1_viewport_d2c_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d2c_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
