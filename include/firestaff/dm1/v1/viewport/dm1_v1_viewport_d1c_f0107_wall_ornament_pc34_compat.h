#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D1C_F0107_WALL_ORNAMENT_SLOT_COUNT_PC34 3
#define DM1_V1_D1C_F0107_WALL_ORNAMENT_STEP_COUNT_PC34 7
#define DM1_V1_D1C_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34 6

typedef enum {
    DM1_V1_D1C_F0107_STEP_F0128_DISPATCH_D1C_PC34 = 0,
    DM1_V1_D1C_F0107_STEP_F0124_WALL_CASE_PC34,
    DM1_V1_D1C_F0107_STEP_F0765_OPAQUE_D1C_WALL_PC34,
    DM1_V1_D1C_F0107_STEP_F0107_FRONT_WALL_ORNAMENT_PC34,
    DM1_V1_D1C_F0107_STEP_F0115_ALCOVE_THING_PASS_PC34,
    DM1_V1_D1C_F0107_STEP_F0108_KEEP_OUT_PC34,
    DM1_V1_D1C_F0107_STEP_F0111_KEEP_OUT_PC34
} DM1_V1_D1CF0107StepKindPc34;

typedef struct {
    int aspect_slot;
    const char *slot_name;
    int view_wall;
    int reaches_d1c_f0107;
    int can_trigger_alcove_thing_pass;
    int side_slot_rejected_by_d1c;
    int expected_ord_flow_index;
    const char *redmcsb_anchor;
} DM1_V1_D1CF0107SlotFlowPc34;

typedef struct {
    DM1_V1_D1CF0107StepKindPc34 step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D1CF0107StepPc34;

typedef struct {
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *anchor;
} DM1_V1_D1CF0107PixelPc34;

typedef struct {
    int view_square_d1c;
    int view_wall_d1c_front;
    int wall_zone_d1c;
    int c10_transparent_color;
    int front_wall_ornament_slot;
    int right_wall_ornament_slot;
    int left_wall_ornament_slot;
    int first_thing_slot;
    unsigned int alcove_cell_order;
    int f0128_dispatch_order_d1c;
    int f0128_dispatch_after_d1l;
    int f0128_dispatch_after_d1r;
    int f0128_dispatch_before_d0l;
    int f0124_wall_case_uses_f0107;
    int f0124_wall_case_uses_f0108;
    int f0124_wall_case_uses_f0111;
    int f0124_wall_case_uses_f0115_only_for_alcove;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_returns_false;
    int f0107_alcove_returns_true;
    int f0107_sets_facing_alcove;
    int f0107_sets_vi_altar;
    int f0107_sets_fountain;
    int f0107_blit_uses_c10;
    int wall_ornament_zone_base;
    int wall_ornament_zone_stride;
    int wall_ornament_zone_d1c_front;
    int wall_ornament_coordinate_set;
    int wall_ornament_native_bitmap_incremented_for_front;
    int wall_ornament_palette_d1c_native;
    int f0115_alcove_uses_first_thing;
    int f0115_alcove_uses_d1c_view_square;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int helper_f0107_slot_constants_reused;
    DM1_V1_D1CF0107SlotFlowPc34 slots[DM1_V1_D1C_F0107_WALL_ORNAMENT_SLOT_COUNT_PC34];
    DM1_V1_D1CF0107StepPc34 steps[DM1_V1_D1C_F0107_WALL_ORNAMENT_STEP_COUNT_PC34];
    DM1_V1_D1CF0107PixelPc34 pixels[DM1_V1_D1C_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D1CF0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D1CF0107WallOrnamentModelPc34 *out_model);

uint32_t dm1_v1_viewport_d1c_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D1CF0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d1c_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D1CF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d1c_f0107_wall_ornament_default_model_pc34(void);

const DM1_V1_D1CF0107SlotFlowPc34 *
dm1_v1_viewport_d1c_f0107_wall_ornament_slot_flow_at_pc34(size_t index);

const DM1_V1_D1CF0107StepPc34 *
dm1_v1_viewport_d1c_f0107_wall_ornament_step_at_pc34(size_t index);

bool dm1_v1_viewport_d1c_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove);

uint8_t dm1_v1_viewport_d1c_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d1c_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d1c_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
