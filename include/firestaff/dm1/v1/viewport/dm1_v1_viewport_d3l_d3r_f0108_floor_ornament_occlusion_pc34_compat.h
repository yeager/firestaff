#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L_D3R_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D3L/D3R F0108 floor-ornament occlusion gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0116:6361-6499 D3L dispatch body. The C17_ELEMENT_DOOR_FRONT
 *   case at 6443 calls F0108 once with M558_FLOOR_ORNAMENT_ORDINAL
 *   (M588_VIEW_FLOOR_D3L=2), then F0115 with C0x0218_DOORPASS1_BLBR.
 *   The shared C16/C18 door-side/stairs-side tail at T0116016 falls into
 *   the open-pit / corridor / teleporter shared tail that calls F0108
 *   with M558_FLOOR_ORNAMENT_ORDINAL + M588_VIEW_FLOOR_D3L at line 6478
 *   with the BUG0_64 source comment ("Floor ornaments are drawn over
 *   open pits. There is no check to prevent drawing floor ornaments
 *   over open pits"). The corridor cell order is C0x3421, the
 *   door-side/stairs-side cell order is C0x0321. The C19_ELEMENT_STAIRS_FRONT
 *   branch handles stairs up/down before falling into T0116016; the
 *   C00_ELEMENT_WALL branch returns early via F0107 alcove test.
 * - DUNVIEW.C F0117:6500-6641 D3R dispatch body. The C17_ELEMENT_DOOR_FRONT
 *   case at 6579 calls F0108 once with M558_FLOOR_ORNAMENT_ORDINAL
 *   (M590_VIEW_FLOOR_D3R=4), then F0115 with C0x0128_DOORPASS1_BRBL.
 *   The shared tail T0117017 calls F0108 with M558 + M590_VIEW_FLOOR_D3R
 *   and the BUG0_64 source comment sits at line 6620. The corridor cell
 *   order is C0x4312, the door-side/stairs-side cell order is C0x0412.
 * - DUNVIEW.C F0108:3940-4011 floor-ornament ordinal gate,
 *   MASK0x8000_FOOTPRINTS recursion at T0108005, C10_COLOR_FLESH
 *   transparent blit, and PC 3.4 C1500 + CoordinateSet*11 + ViewFloor
 *   zone math.
 * - DUNVIEW.C F0128:8318-8542 dispatch order: D3C, then F0676/F0677,
 *   then F0116 (D3L), F0117 (D3R), then the rest of the far-to-near pass.
 * - DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2533-2559 M550/M551/M552/M553/
 *   M554/M555/M556/M557/M558 slots (PC 3.4 / I34E: M550=2, M551=4,
 *   M552=5, M553=6, M554=3, M555=3, M556=3, M557=4, M558=5);
 *   DEFS.H:2608-2609 M601_VIEW_SQUARE_D3L=12 / M602_VIEW_SQUARE_D3R=13;
 *   DEFS.H:2668-2677 cell orders (C0x0218, C0x0321, C0x0349, C0x0412,
 *   C0x0439, C0x3421, C0x4312, C0x0128); DEFS.H:2698-2702 wall-ornament
 *   view ordinals; DEFS.H:2739-2754 floor-ornament view ordinals
 *   M588_VIEW_FLOOR_D3L=2 / M589_VIEW_FLOOR_D3C=3 / M590_VIEW_FLOOR_D3R=4;
 *   DEFS.H:4045-4046 C705_ZONE_WALL_D3L / C706_ZONE_WALL_D3R;
 *   DEFS.H:4141-4143 C802/C803/C804 stairs-up-front D3L/D3C/D3R zones;
 *   DEFS.H:4154-4156 C815/C816/C817 stairs-down-front zones;
 *   DEFS.H:4199-4201 C852/C853/C854 floor-pit D3L/D3C/D3R zones.
 *
 * This is synthetic metadata coverage for a 320x200 screen and a 224x136
 * viewport. It makes no real-asset or original-DOS pixel-parity claim and
 * intentionally does not duplicate the D1C F0108 floor-ornament
 * occlusion sibling (which locks the same BUG0_64 contract on the D1C
 * front-square column-center only), the D3L2/D3R2 F0108 floor-ornament
 * occlusion sibling (which locks BUG0_64 on the D3L2 + D3R2 *side-squares*
 * via F0676/F0677 in `dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_
 * occlusion_pc34_compat.c`), the F0116/F0117 thing-pass sibling
 * (`dm1_v1_viewport_d3l_d3r_f0115_thing_pass_pc34_compat.c`), the
 * F0108 floor+ceiling+ornament sibling
 * (`dm1_v1_viewport_d3l_d3r_f0108_floor_ceiling_ornament_pc34_compat.c`),
 * the F0107 wall-ornament sibling
 * (`dm1_v1_viewport_d3l_d3r_f0107_wall_ornament_pc34_compat.c`), the
 * D3L/D3R wall sibling
 * (`dm1_v1_viewport_d3l_d3r_wall_pc34_compat.c`), the sidewall-pair
 * sibling, the stairs/pit dispatch sibling, the F0111 door-front-pair
 * sibling, the D1L/D1R door-frame sibling, the D0C floor-ornament
 * keepout, the F0111 partly-open door family, the D2L/D2R F0098
 * fallback, the D2C F0108 floor+ceiling+ornament, the D0C F0108
 * floor-ornament, the CSB/Nexus/Theron/DM2 lanes, the F0098
 * floor+ceiling fallback, the F0095 floor-ornament aggregate, the
 * F0107 wall-ornament alcove helper, or any other PC34 viewport
 * ornament sibling.
 */

#define DM1_V1_D3L_D3R_FOCCL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L_D3R_FOCCL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L_D3R_FOCCL_SCREEN_WIDTH_PC34 320
#define DM1_V1_D3L_D3R_FOCCL_SCREEN_HEIGHT_PC34 200
#define DM1_V1_D3L_D3R_FOCCL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3L_D3R_FOCCL_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D3L_D3R_FOCCL_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D3L_D3R_FOCCL_FLOOR_ZONE_BASE_PC34 1500
#define DM1_V1_D3L_D3R_FOCCL_FLOOR_ZONE_STRIDE_PC34 11

#define DM1_V1_D3L_D3R_FOCCL_VIEW_SQUARE_D3L_PC34 12
#define DM1_V1_D3L_D3R_FOCCL_VIEW_SQUARE_D3R_PC34 13
#define DM1_V1_D3L_D3R_FOCCL_VIEW_FLOOR_D3L_PC34 2
#define DM1_V1_D3L_D3R_FOCCL_VIEW_FLOOR_D3R_PC34 4

#define DM1_V1_D3L_D3R_FOCCL_ZONE_WALL_D3L_PC34 705
#define DM1_V1_D3L_D3R_FOCCL_ZONE_WALL_D3R_PC34 706
#define DM1_V1_D3L_D3R_FOCCL_ZONE_FLOORPIT_D3L_PC34 852
#define DM1_V1_D3L_D3R_FOCCL_ZONE_FLOORPIT_D3R_PC34 854
#define DM1_V1_D3L_D3R_FOCCL_ZONE_DOOR_D3L_PC34 624
#define DM1_V1_D3L_D3R_FOCCL_ZONE_DOOR_D3R_PC34 626
#define DM1_V1_D3L_D3R_FOCCL_ZONE_STAIRS_UP_FRONT_D3L_PC34 802
#define DM1_V1_D3L_D3R_FOCCL_ZONE_STAIRS_UP_FRONT_D3R_PC34 804
#define DM1_V1_D3L_D3R_FOCCL_ZONE_STAIRS_DOWN_FRONT_D3L_PC34 815
#define DM1_V1_D3L_D3R_FOCCL_ZONE_STAIRS_DOWN_FRONT_D3R_PC34 817

#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_DOORPASS1_BLBR_PC34 0x0218u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_DOORPASS1_BRBL_PC34 0x0128u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_DOORPASS2_FLFR_PC34 0x0349u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_DOORPASS2_FRFL_PC34 0x0439u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L_PC34 0x3421u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_OPEN_BRBL_FRFL_D3R_PC34 0x4312u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_SIDE_BLBR_FR_PC34 0x0321u
#define DM1_V1_D3L_D3R_FOCCL_CELL_ORDER_SIDE_BRBL_FL_PC34 0x0412u

typedef enum {
    DM1_V1_D3L_D3R_FOCCL_SIDE_D3L_PC34 = 0,
    DM1_V1_D3L_D3R_FOCCL_SIDE_D3R_PC34 = 1
} DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionSidePc34;

typedef enum {
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_CORRIDOR_PC34 = 0,
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_OPEN_PIT_PC34 = 1,
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_TELEPORTER_PC34 = 2,
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_STAIRS_SIDE_PC34 = 3,
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_DOOR_FRONT_PC34 = 4,
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_STAIRS_FRONT_PC34 = 5,
    DM1_V1_D3L_D3R_FOCCL_CONTEXT_DOOR_SIDE_PC34 = 6
} DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionContextPc34;

typedef enum {
    DM1_V1_D3L_D3R_FOCCL_STEP_F0117_DISPATCH_PC34 = 0,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0116_F0108_DOOR_FRONT_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0116_F0108_SHARED_TAIL_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0116_BUG0_64_OCCLUSION_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0117_F0108_DOOR_FRONT_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0117_F0108_SHARED_TAIL_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0117_BUG0_64_OCCLUSION_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0108_ORDINAL_DECODE_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0108_FOOTPRINT_RECURSION_PC34,
    DM1_V1_D3L_D3R_FOCCL_STEP_F0108_C10_BLIT_PC34
} DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionStepKindPc34;

typedef struct {
    DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionContextPc34 context;
    int order_index;
    int calls_f0108;
    int f0108_occludes_cell;
    int bug0_64_occlusion_present;
    int calls_f0115;
    int expected_cell_order;
    int expected_view_floor;
    int expected_zone_wall;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionStepPc34;

typedef struct {
    int view_square_d3l;
    int view_square_d3r;
    int view_floor_d3l;
    int view_floor_d3r;
    int wall_zone_d3l;
    int wall_zone_d3r;
    int c10_transparent_color;
    int footprint_index;
    int floor_ornament_ordinal_slot;
    int first_thing_slot;
    int door_state_slot;
    int door_thing_index_slot;
    int pit_or_teleporter_visible_slot;
    int stairs_up_slot;
    int f0128_dispatches_after_d3c;
    int f0116_dispatches_before_f0117;
    int f0117_dispatches_before_d2l;
    int f0116_door_front_calls_f0108_with_558;
    int f0116_door_front_calls_f0115_doorpass1;
    int f0116_door_front_drops_to_doorpass2;
    int f0116_wall_branch_returns_via_alcove;
    int f0116_stairs_front_calls_f0104_first;
    int f0116_corridor_cell_order_d3l;
    int f0116_side_cell_order_d3l;
    int f0116_doorpass2_cell_order_d3l;
    int f0117_door_front_calls_f0108_with_558;
    int f0117_door_front_calls_f0115_doorpass1;
    int f0117_door_front_drops_to_doorpass2;
    int f0117_wall_branch_returns_via_alcove;
    int f0117_stairs_front_calls_f0104_first;
    int f0117_corridor_cell_order_d3r;
    int f0117_side_cell_order_d3r;
    int f0117_doorpass2_cell_order_d3r;
    int f0108_ordinal_zero_skips_blit;
    int f0108_footprint_mask_recurses;
    int f0108_footprint_only_skips_primary;
    int f0108_blit_uses_c10_transparent;
    int f0108_zone_uses_11_stride;
    int f0108_zone_d3l;
    int f0108_zone_d3r;
    int bug0_64_occlusion_guard;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionModelPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int model_builder_ok;
    int hash_stable;
    int decode_simple_primary;
    int decode_fp_only_recurses;
    int decode_fp_with_primary_both;
    int decode_zero_skips_blit;
    int context_occlusion_paths;
    int context_zero_ordinal_no_occlusion;
    int blend_c10_preserves_destination;
    int blend_opaque_writes_source;
    int zone_d3l_stride_11;
    int zone_d3r_stride_11;
    int step_count_ten;
    int bug0_64_marker_count;
    int source_evidence_present;
    int disjointness_note_present;
    int f0116_door_front_branch_present;
    int f0117_door_front_branch_present;
    uint32_t deterministic_hash;
} DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionSelfTestResultPc34;

bool dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionModelPc34 *out_model);

uint32_t dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionModelPc34 *model);

uint32_t dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void);

const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_default_model_pc34(void);

unsigned int dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_count_pc34(void);

const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_step_at_pc34(size_t index);

bool dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionSidePc34 side,
    DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionContextPc34 context,
    unsigned int floor_ornament_ordinal);

unsigned int dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    bool *footprint_flag_set,
    unsigned int *cleared_ordinal,
    bool *primary_draws,
    int *primary_index,
    bool *recursive_footprints_draw,
    int *recursive_footprints_index);

uint8_t dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3l_pc34(
    int coordinate_set,
    int view_floor);

int dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_zone_d3r_pc34(
    int coordinate_set,
    int view_floor);

const char *dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_source_evidence_pc34(void);

const char *dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_disjointness_note_pc34(void);

int dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_self_test_pc34(void);

const DM1_V1_D3L_D3RF0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d3l_d3r_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
