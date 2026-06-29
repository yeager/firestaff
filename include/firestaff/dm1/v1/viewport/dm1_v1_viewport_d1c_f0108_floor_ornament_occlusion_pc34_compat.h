#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1C F0108 floor-ornament occlusion source-lock gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0124:7727-7924 D1C dispatch body. The D1C column-center
 *   square is the only square where the F0124 wall case (C00_ELEMENT_WALL)
 *   is reached via F0107 alcove path, and the only square where the
 *   F0108 floor-ornament call targets M595_VIEW_FLOOR_D1C.
 * - DUNVIEW.C F0108:3940-4011 floor-ornament ordinal, MASK0x8000_FOOTPRINTS
 *   recursion at T0108005, C10_COLOR_FLESH transparency, and PC 3.4
 *   C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloor zone math.
 * - DUNVIEW.C F0124:7874 C17_ELEMENT_DOOR_FRONT: F0108 fires once before
 *   the F0115 thing pass with C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT.
 *   The C17 path is the *only* D1C path that does not exhibit the BUG0_64
 *   occlusion, because the F0104 floor-pit / F0105 flipped path is kept
 *   out and the door is drawn on top.
 * - DUNVIEW.C F0124:7926 T0124017 C02_ELEMENT_PIT / C05_ELEMENT_TELEPORTER /
 *   C01_ELEMENT_CORRIDOR with BUG0_64: F0108 fires *over* the open pit
 *   with no occlusion guard. F0112 ceiling-pit fires at 7929, F0115
 *   thing pass at 7937 with C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT.
 * - DUNVIEW.C F0124:7868 C19_ELEMENT_STAIRS_FRONT (M555_STAIRS_UP) goes to
 *   T0124017 via `goto T0124017`; F0104 stairs-front bitmap fires first
 *   (7836-7866) and then F0108 fires *over* the stairs-front bitmap at
 *   7926 with the BUG0_64 occlusion contract.
 * - DUNVIEW.C F0128:8318-8542 dispatches D1L, D1R, D1C, D0L, D0R, D0C in
 *   order, with D1C at line 8536 between D1L/D1R and D0L/D0R.
 * - DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:2533-2559 M550..M558 square
 *   aspect slots with M558_FLOOR_ORNAMENT_ORDINAL=5; DEFS.H:2596-2611
 *   view-square ordinals M603..M609 with M606_VIEW_SQUARE_D1C=3;
 *   DEFS.H:2668-2677 cell orders; DEFS.H:2746 M595_VIEW_FLOOR_D1C=7;
 *   DEFS.H:4045-4046 C705/C706 wall zones; DEFS.H:4052 C712_ZONE_WALL_D1C.
 *
 * This is synthetic metadata coverage for a 320x200 screen and a 224x136
 * viewport. It makes no real-asset or original-DOS pixel-parity claim and
 * intentionally does not duplicate the older D1C F0107 wall-ornament,
 * D0C/D2C/D3C F0108 floor+ceiling+ornament, D0C F0108 floor-ornament
 * keepout, D1C F0111 partly-open door, or D1C F0115 / stairs / pit /
 * center-field gates.
 */

#define DM1_V1_D1C_F0108_FOCCL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1C_F0108_FOCCL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1C_F0108_FOCCL_SCREEN_WIDTH_PC34 320
#define DM1_V1_D1C_F0108_FOCCL_SCREEN_HEIGHT_PC34 200
#define DM1_V1_D1C_F0108_FOCCL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D1C_F0108_FOCCL_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D1C_F0108_FOCCL_FOOTPRINT_INDEX_PC34 15
#define DM1_V1_D1C_F0108_FOCCL_FLOOR_ZONE_BASE_PC34 1500
#define DM1_V1_D1C_F0108_FOCCL_FLOOR_ZONE_STRIDE_PC34 11
#define DM1_V1_D1C_F0108_FOCCL_C1500_ZONE_FLOOR_ORNAMENT_PC34 1500
#define DM1_V1_D1C_F0108_FOCCL_C712_ZONE_WALL_D1C_PC34 712
#define DM1_V1_D1C_F0108_FOCCL_M587_VIEW_WALL_D1C_FRONT_PC34 14
#define DM1_V1_D1C_F0108_FOCCL_M595_VIEW_FLOOR_D1C_PC34 7
#define DM1_V1_D1C_F0108_FOCCL_M606_VIEW_SQUARE_D1C_PC34 3

typedef enum {
    DM1_V1_D1C_F0108_FOCCL_CONTEXT_CORRIDOR_PC34 = 0,
    DM1_V1_D1C_F0108_FOCCL_CONTEXT_OPEN_PIT_PC34 = 1,
    DM1_V1_D1C_F0108_FOCCL_CONTEXT_TELEPORTER_PC34 = 2,
    DM1_V1_D1C_F0108_FOCCL_CONTEXT_STAIRS_FRONT_PC34 = 3,
    DM1_V1_D1C_F0108_FOCCL_CONTEXT_DOOR_FRONT_PC34 = 4
} DM1_V1_D1CF0108FloorOrnamentOcclusionContextPc34;

typedef enum {
    DM1_V1_D1C_F0108_FOCCL_STEP_F0128_DISPATCH_D1C_PC34 = 0,
    DM1_V1_D1C_F0108_FOCCL_STEP_F0124_F0108_CALL_PC34,
    DM1_V1_D1C_F0108_FOCCL_STEP_F0108_ORDINAL_DECODE_PC34,
    DM1_V1_D1C_F0108_FOCCL_STEP_F0108_FOOTPRINT_RECURSION_PC34,
    DM1_V1_D1C_F0108_FOCCL_STEP_F0108_C10_BLIT_PC34,
    DM1_V1_D1C_F0108_FOCCL_STEP_F0112_CEILING_PIT_PC34,
    DM1_V1_D1C_F0108_FOCCL_STEP_F0115_THING_PASS_PC34,
    DM1_V1_D1C_F0108_FOCCL_STEP_BUG0_64_OCCLUSION_GUARD_PC34
} DM1_V1_D1CF0108FloorOrnamentOcclusionStepKindPc34;

typedef struct {
    DM1_V1_D1CF0108FloorOrnamentOcclusionContextPc34 context;
    int order_index;
    int calls_f0108;
    int f0108_occludes_cell;
    int bug0_64_occlusion_present;
    int calls_f0112;
    int calls_f0115;
    int expected_cell_order;
    int expected_thing_pass;
    int expected_zone_base;
    int expected_zone_coordinate_set;
    int expected_view_floor;
    const char *name;
    const char *redmcsb_anchor;
} DM1_V1_D1CF0108FloorOrnamentOcclusionStepPc34;

typedef struct {
    int view_square_d1c;
    int view_floor_d1c;
    int view_wall_d1c_front;
    int wall_zone_d1c;
    int c10_transparent_color;
    int floor_ornament_ordinal_slot;
    int first_thing_slot;
    int pit_or_teleporter_visible_slot;
    int stairs_up_slot;
    int door_thing_index_slot;
    int door_state_slot;
    int front_wall_ornament_slot;
    int f0128_dispatch_order_d1c;
    int f0128_dispatch_after_d1l;
    int f0128_dispatch_after_d1r;
    int f0128_dispatch_before_d0l;
    int f0124_door_front_calls_f0108;
    int f0124_door_front_calls_f0115;
    int f0124_door_front_cell_order;
    int f0124_corridor_calls_f0108;
    int f0124_corridor_calls_f0112;
    int f0124_corridor_calls_f0115;
    int f0124_corridor_cell_order;
    int f0124_pit_f0104_fires_first;
    int f0124_pit_f0108_occludes_pit;
    int f0124_teleporter_f0108_occludes_teleporter;
    int f0124_stairs_front_f0104_fires_first;
    int f0124_stairs_front_f0108_occludes_stairs;
    int f0108_ordinal_zero_skips_blit;
    int f0108_footprint_mask_recurses;
    int f0108_footprint_only_skips_primary;
    int f0108_blit_uses_c10_transparent;
    int f0108_d1c_in_horizontal_flip_branch;
    int f0108_d1c_zone_uses_11_stride;
    int f0108_d1c_zone_uses_9_stride;
    int f0108_zone_d1c;
    int bug0_64_occlusion_guard;
    int f0112_ceiling_pit_graphic;
    int f0112_ceiling_pit_zone_d1c;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34;

bool dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *out_model);

uint32_t dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *model);

uint32_t dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void);

const DM1_V1_D1CF0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_default_model_pc34(void);

unsigned int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_count_pc34(void);

const DM1_V1_D1CF0108FloorOrnamentOcclusionStepPc34 *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_step_at_pc34(size_t index);

bool dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_context_occludes_pc34(
    DM1_V1_D1CF0108FloorOrnamentOcclusionContextPc34 context,
    unsigned int floor_ornament_ordinal);

unsigned int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    bool *footprint_flag_set,
    unsigned int *cleared_ordinal,
    bool *primary_draws,
    int *primary_index,
    bool *recursive_footprints_draw,
    int *recursive_footprints_index);

uint8_t dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_zone_d1c_pc34(
    int coordinate_set,
    int view_floor);

const char *dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_source_evidence_pc34(void);

const char *dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

typedef struct {
    int ok;
    int model_builder_ok;
    int hash_stable;
    int decode_simple_primary;
    int decode_fp_only_recurses;
    int decode_fp_with_primary_both;
    int decode_zero_skips_blit;
    int context_corridor_occludes;
    int context_open_pit_occludes;
    int context_teleporter_occludes;
    int context_stairs_front_occludes;
    int context_door_front_occludes;
    int context_zero_ordinal_no_occlusion;
    int blend_c10_preserves_destination;
    int blend_opaque_writes_source;
    int zone_11_stride_correct;
    int zone_9_stride_correct;
    int bug0_64_guard_absent;
    int step_count_eight;
    int step_bug0_64_marker;
    int deterministic_hash_set;
    int assertions;
    int failures;
    uint32_t deterministic_hash;
} DM1_V1_D1CF0108FloorOrnamentOcclusionSelfTestResultPc34;

int dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_self_test_pc34(void);

const DM1_V1_D1CF0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d1c_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void);
