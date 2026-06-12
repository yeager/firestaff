#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0107_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L_D0R_F0107_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_SIDE_COUNT_PC34 2
#define DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34 9
#define DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34 6
#define DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_DOOR_STATE_COUNT_PC34 3
#define DM1_V1_D0L_D0R_F0107_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0L_D0R_F0107_ALCOVE_CELL_CONTENT_MASK_PC34 0x0008u

typedef enum {
    DM1_V1_D0L_D0R_F0107_SIDE_D0L_PC34 = 1,
    DM1_V1_D0L_D0R_F0107_SIDE_D0R_PC34 = 2
} DM1_V1_D0LD0RF0107SidePc34;

typedef enum {
    DM1_V1_D0L_D0R_F0107_STEP_F0128_DISPATCH_D0L_PC34 = 0,
    DM1_V1_D0L_D0R_F0107_STEP_F0128_DISPATCH_D0R_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_F0125_D0L_BODY_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_F0126_D0R_BODY_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_D0_WALL_RETURNS_BEFORE_F0107_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_F0107_ALCOVE_BOOL_SOURCE_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_F0107_C10_TRANSPARENCY_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_F0108_F0115_ORDER_CONTRAST_PC34,
    DM1_V1_D0L_D0R_F0107_STEP_F0111_PARTLY_OPEN_RELATION_PC34
} DM1_V1_D0LD0RF0107StepKindPc34;

typedef struct {
    int side;
    const char *side_name;
    int view_square;
    int wall_zone;
    int relative_depth;
    int relative_lateral;
    int f0128_update_line;
    int f0128_draw_line;
    int dispatcher_line_start;
    int dispatcher_line_end;
    int wall_case_line;
    int wall_case_returns_before_f0107;
    int direct_f0107_call_present;
    int first_thing_slot;
    unsigned int thing_pass_order;
    int thing_pass_line;
    int ceiling_line;
    int thing_before_ceiling;
    int ceiling_before_thing;
    int f0108_keepout;
    int f0111_direct_call_present;
    const char *redmcsb_anchor;
} DM1_V1_D0LD0RF0107LanePc34;

typedef struct {
    int ordinal_slot;
    const char *slot_name;
    int reaches_d0l_d0r_directly;
    int nearest_source_view_wall;
    int helper_case_index;
    int expected_rejection;
    const char *redmcsb_anchor;
} DM1_V1_D0LD0RF0107OrdinalFlowPc34;

typedef struct {
    int step;
    int order_index;
    int expected_present;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D0LD0RF0107StepPc34;

typedef struct {
    int ordinal_index;
    uint8_t before;
    uint8_t source;
    uint8_t after;
    int transparent_skip;
    int writes_pixel;
    const char *anchor;
} DM1_V1_D0LD0RF0107PixelPc34;

typedef struct {
    int door_state;
    int f0111_partly_open;
    int horizontal_half_blit_uses_c10;
    int mask0x4000_shift_applied;
    int shares_f0107_c10_transparency;
    const char *anchor;
} DM1_V1_D0LD0RF0107DoorRelationPc34;

typedef struct {
    int view_square_d0l;
    int view_square_d0r;
    int wall_zone_d0l;
    int wall_zone_d0r;
    int c10_transparent_color;
    int m550_first_thing_slot;
    int m551_right_wall_ornament_slot;
    int m552_front_wall_ornament_slot;
    int m553_left_wall_ornament_slot;
    int f0128_d0l_then_d0r;
    int d0l_direct_f0107_calls;
    int d0r_direct_f0107_calls;
    int d0_wall_case_returns_before_f0107;
    int f0107_zero_ordinal_returns_false;
    int f0107_non_alcove_cell_returns_false;
    int f0107_alcove_cell_returns_true;
    int f0107_uses_cell_content_bits;
    int f0107_blit_uses_c10;
    int c10_transparent_preserves_destination;
    int f0108_floor_ceiling_keepout;
    int f0115_d0l_order_backright;
    int f0115_d0r_order_backleft;
    int d0l_thing_before_ceiling;
    int d0r_ceiling_before_thing;
    int f0111_partly_open_uses_c10_half_blits;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    int helper_f0107_slot_constants_reused;
    DM1_V1_D0LD0RF0107LanePc34 lanes[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_SIDE_COUNT_PC34];
    DM1_V1_D0LD0RF0107OrdinalFlowPc34 ordinals[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34];
    DM1_V1_D0LD0RF0107StepPc34 steps[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_STEP_COUNT_PC34];
    DM1_V1_D0LD0RF0107PixelPc34 pixels[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_PIXEL_COUNT_PC34];
    DM1_V1_D0LD0RF0107DoorRelationPc34
        door_states[DM1_V1_D0L_D0R_F0107_WALL_ORNAMENT_DOOR_STATE_COUNT_PC34];
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D0LD0RF0107WallOrnamentModelPc34;

bool dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *out_model);

const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_default_model_pc34(void);

uint32_t dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D0LD0RF0107WallOrnamentModelPc34 *model);

uint32_t dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_deterministic_hash_pc34(void);

const DM1_V1_D0LD0RF0107LanePc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_lane_at_pc34(size_t index);

const DM1_V1_D0LD0RF0107StepPc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_step_at_pc34(size_t index);

const DM1_V1_D0LD0RF0107OrdinalFlowPc34 *
dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_ordinal_at_pc34(size_t index);

bool dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    unsigned int cell_content_bits);

uint8_t dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

const char *dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_source_evidence_pc34(void);

const char *dm1_v1_viewport_d0l_d0r_f0107_wall_ornament_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
