#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3L2_D3R2_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D3L2/D3R2 F0108 floor-ornament occlusion gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0676:6226-6290 D3L2 dispatch body. The C17_ELEMENT_DOOR_FRONT
 *   case at 6268 calls F0108 once with M552_FRONT_WALL_ORNAMENT_ORDINAL
 *   (C00_VIEW_FLOOR_D3L2), then F0115 with C0x0218_DOORPASS1.
 *   The shared C02/C05/C01 tail at 6276-6286 calls F0108 with
 *   M558_FLOOR_ORNAMENT_ORDINAL and C00_VIEW_FLOOR_D3L2, and the BUG0_64
 *   comment ("Floor ornaments are drawn over open pits") sits at line
 *   6284. The shared tail uses C0x3421 cell order for T0676015 (corridor)
 *   and T0676016 (door-side/stairs-side C0x0321).
 * - DUNVIEW.C F0677:6293-6357 D3R2 dispatch body. The C17_ELEMENT_DOOR_FRONT
 *   case at 6333 calls F0108 once with M558_FLOOR_ORNAMENT_ORDINAL
 *   (C01_VIEW_FLOOR_D3R2), then F0115 with C0x0128_DOORPASS1_BACKRIGHT_BACKLEFT.
 *   The shared tail at 6342-6353 calls F0108 with C01_VIEW_FLOOR_D3R2
 *   and the BUG0_64 comment sits at line 6351. The shared tail uses
 *   C0x4312 (corridor) and C0x0412 (door-side/stairs-side) cell orders.
 * - DUNVIEW.C F0108:3940-4011 floor-ornament ordinal gate,
 *   MASK0x8000_FOOTPRINTS recursion at T0108005, C10_COLOR_FLESH
 *   transparent blit, and PC 3.4 C1500 + CoordinateSet*11 + ViewFloor
 *   zone math.
 * - DUNVIEW.C F0128:8318-8542 dispatch order: D3C, then F0676/F0677,
 *   then the rest of the far-to-near pass.
 * - DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2533-2559
 *   M550/M551/M552/M553/M554/M555/M556/M557/M558 slots; DEFS.H:2596-2611
 *   M610/M611 view-square ordinals for D3L2=14 / D3R2=15;
 *   DEFS.H:2668-2677 cell orders (C0x0218, C0x0321, C0x0349, C0x0412,
 *   C0x0439, C0x3421, C0x4312, C0x0128); DEFS.H:4045-4046
 *   C705/C706 (D3L/D3R walls, not used here); DEFS.H:4139-4153
 *   C702/C703 wall zones for D3L2/D3R2; DEFS.H:4205-4207
 *   D3L2/D3R2 floor-pit zones.
 *
 * This is synthetic metadata coverage for a 320x200 screen and a 224x136
 * viewport. It makes no real-asset or original-DOS pixel-parity claim and
 * intentionally does not duplicate the D1C F0108 floor-ornament
 * occlusion sibling (which locks the same BUG0_64 contract on the D1C
 * front-square column-center only), the F0676/F0677 thing-pass sibling
 * (`dm1_v1_viewport_d3l2_d3r2_f0115_thing_pass_pc34_compat.c`), the
 * F0108 floor+ceiling+ornament sibling
 * (`dm1_v1_viewport_d3l2_d3r2_f0108_floor_ceiling_ornament_pc34_compat.c`),
 * the F0108 wall-composition sibling
 * (`dm1_v1_viewport_d3l2_d3r2_f0108_wall_composition_pc34_compat.c`), the
 * D3L2/D3R2 wall sibling, the stairs/pit dispatch sibling, the F0111
 * door-front-pair sibling, the D1L/D1R door-frame sibling, the D3L/D3R
 * sibling, the D0C floor-ornament keepout, the F0111 partly-open door
 * family, or any CSB/Nexus/Theron/DM2 lane.
 */

#define DM1_V1_D3L2_D3R2_FOCCL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D3L2_D3R2_FOCCL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D3L2_D3R2_FOCCL_SCREEN_WIDTH_PC34 320
#define DM1_V1_D3L2_D3R2_FOCCL_SCREEN_HEIGHT_PC34 200
#define DM1_V1_D3L2_D3R2_FOCCL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3L2_D3R2_FOCCL_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D3L2_D3R2_FOCCL_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D3L2_D3R2_FOCCL_FLOOR_ZONE_BASE_PC34 1500
#define DM1_V1_D3L2_D3R2_FOCCL_FLOOR_ZONE_STRIDE_PC34 11

#define DM1_V1_D3L2_D3R2_FOCCL_VIEW_SQUARE_D3L2_PC34 14
#define DM1_V1_D3L2_D3R2_FOCCL_VIEW_SQUARE_D3R2_PC34 15
#define DM1_V1_D3L2_D3R2_FOCCL_VIEW_FLOOR_D3L2_PC34 0
#define DM1_V1_D3L2_D3R2_FOCCL_VIEW_FLOOR_D3R2_PC34 1

#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_WALL_D3L2_PC34 702
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_WALL_D3R2_PC34 703
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_FLOORPIT_D3L2_PC34 850
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_FLOORPIT_D3R2_PC34 851
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_DOOR_D3L2_PC34 3700
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_DOOR_D3R2_PC34 3710
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_STAIRS_UP_FRONT_D3L2_PC34 800
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_STAIRS_UP_FRONT_D3R2_PC34 801
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_STAIRS_DOWN_FRONT_D3L2_PC34 813
#define DM1_V1_D3L2_D3R2_FOCCL_ZONE_STAIRS_DOWN_FRONT_D3R2_PC34 814

#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS1_BLBR_PC34 0x0218u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS1_BRBL_PC34 0x0128u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS2_FLFR_PC34 0x0349u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_DOORPASS2_FRFL_PC34 0x0439u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FLFR_D3L2_PC34 0x3421u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BLBR_FRFL_D3L2_PC34 0x3249u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BRBL_FLFR_D3R2_PC34 0x4213u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_OPEN_BRBL_FRFL_D3R2_PC34 0x4312u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_SIDE_BLBR_FR_PC34 0x0321u
#define DM1_V1_D3L2_D3R2_FOCCL_CELL_ORDER_SIDE_BRBL_FL_PC34 0x0412u

typedef enum {
    DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3L2_PC34 = 0,
    DM1_V1_D3L2_D3R2_FOCCL_SIDE_D3R2_PC34 = 1
} DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSidePc34;

typedef enum {
    DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_CORRIDOR_PC34 = 0,
    DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_OPEN_PIT_PC34 = 1,
    DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_TELEPORTER_PC34 = 2,
    DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_STAIRS_SIDE_PC34 = 3,
    DM1_V1_D3L2_D3R2_FOCCL_CONTEXT_DOOR_FRONT_PC34 = 4
} DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionContextPc34;

typedef enum {
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0677_DISPATCH_PC34 = 0,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0676_F0108_DOOR_FRONT_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0676_F0108_SHARED_TAIL_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0676_BUG0_64_OCCLUSION_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0677_F0108_DOOR_FRONT_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0677_F0108_SHARED_TAIL_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0677_BUG0_64_OCCLUSION_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0108_ORDINAL_DECODE_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0108_FOOTPRINT_RECURSION_PC34,
    DM1_V1_D3L2_D3R2_FOCCL_STEP_F0108_C10_BLIT_PC34
} DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionStepKindPc34;

typedef struct {
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionContextPc34 context;
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
} DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionStepPc34;

typedef struct {
    int view_square_d3l2;
    int view_square_d3r2;
    int view_floor_d3l2;
    int view_floor_d3r2;
    int wall_zone_d3l2;
    int wall_zone_d3r2;
    int c10_transparent_color;
    int footprint_index;
    int floor_ornament_ordinal_slot;
    int front_wall_ornament_slot;
    int first_thing_slot;
    int door_state_slot;
    int door_thing_index_slot;
    int pit_or_teleporter_visible_slot;
    int stairs_up_slot;
    int f0128_dispatches_after_d3c;
    int f0676_dispatches_before_f0677;
    int f0677_dispatches_before_d2l;
    int f0676_door_front_calls_f0108_with_552;
    int f0676_door_front_calls_f0115_doorpass1;
    int f0676_shared_tail_calls_f0108_with_558;
    int f0676_shared_tail_open_pit_uses_f0104_first;
    int f0676_corridor_cell_order_d3l2;
    int f0676_side_cell_order_d3l2;
    int f0676_doorpass2_cell_order_d3l2;
    int f0677_door_front_calls_f0108;
    int f0677_door_front_calls_f0115_doorpass1;
    int f0677_shared_tail_calls_f0108;
    int f0677_shared_tail_open_pit_uses_f0104_first;
    int f0677_corridor_cell_order_d3r2;
    int f0677_side_cell_order_d3r2;
    int f0677_doorpass2_cell_order_d3r2;
    int f0108_ordinal_zero_skips_blit;
    int f0108_footprint_mask_recurses;
    int f0108_footprint_only_skips_primary;
    int f0108_blit_uses_c10_transparent;
    int f0108_zone_uses_11_stride;
    int f0108_zone_d3l2;
    int f0108_zone_d3r2;
    int bug0_64_occlusion_guard;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34;

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
    int zone_d3l2_stride_11;
    int zone_d3r2_stride_11;
    int step_count_ten;
    int bug0_64_marker_count;
    int source_evidence_present;
    int disjointness_note_present;
    int f0676_door_front_branch_present;
    int f0677_door_front_branch_present;
    uint32_t deterministic_hash;
} DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSelfTestResultPc34;

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *out_model);

uint32_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *model);

uint32_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void);

const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_default_model_pc34(void);

unsigned int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_count_pc34(void);

const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_step_at_pc34(size_t index);

bool dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSidePc34 side,
    DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionContextPc34 context,
    unsigned int floor_ornament_ordinal);

unsigned int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    bool *footprint_flag_set,
    unsigned int *cleared_ordinal,
    bool *primary_draws,
    int *primary_index,
    bool *recursive_footprints_draw,
    int *recursive_footprints_index);

uint8_t dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3l2_pc34(
    int coordinate_set,
    int view_floor);

int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_zone_d3r2_pc34(
    int coordinate_set,
    int view_floor);

const char *dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_source_evidence_pc34(void);

const char *dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_disjointness_note_pc34(void);

int dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_self_test_pc34(void);

const DM1_V1_D3L2D3R2F0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d3l2_d3r2_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
