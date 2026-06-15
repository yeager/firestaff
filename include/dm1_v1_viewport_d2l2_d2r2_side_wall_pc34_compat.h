#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_SIDE_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_SIDE_WALL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C:6837-6865 F0678_DrawD2L2 / DUNVIEW.C:6868-6896 F0679_DrawD2R2
 *   are the side-row (depth=2, lateral=+-2) wall dispatchers, which only
 *   handle C00_ELEMENT_WALL (via F0104/F0105 + return) and
 *   C05_ELEMENT_TELEPORTER (via F0113) and do not call F0107/F0108/F0111/
 *   F0115 in their bodies.
 * - DUNVIEW.C:8503-8508 F0128 dispatches F0678 at relative (2,-2) and
 *   F0679 at relative (2,+2) AFTER F0118 (D3C, depth 3, lateral 0) and
 *   BEFORE F0119 (D2L, depth 2, lateral -1).
 * - DUNVIEW.C:3113-3129 F0104 / 3185-3204 F0105 share the C10_COLOR_FLESH
 *   transparent blit contract used by the D2L2/D2R2 wall zones.
 * - DEFS.H:2088 anchors C10_COLOR_FLESH, 2605-2606 anchors C09/C10 view
 *   square ids, 3428-3429 anchors C05_WALL_D2R2/C06_WALL_D2L2, 4047-4048
 *   anchors C707/C708 zones.
 * - The PC 3.4 path uses MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M
 *   _F31E_F31J_X31J_P31J and PC_FIX_CODE_SIZE for the F0678/F0679 native
 *   bitmap path (C06_WALL_D2L2+2 and C05_WALL_D2R2+2 wall-set indices);
 *   the G0076 flipped branch is excluded on PC 3.4.
 *
 * This gate is intentionally disjoint from:
 * - test_dm1_v1_viewport_d2l2_d2r2_wall_pc34_compat (F0104/F0105 wall-set
 *   blit + view-square/zone/viewport pixel contract only)
 * - test_dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_pc34_compat
 *   (F0107 wall-ornament alcove/C10/C1004 zone math, M551/M553 wall
 *   ornament ordinals)
 * - test_dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat
 *   (F0108 floor-ornament composition guard for the D2L/D2R front pair
 *   guarded by D2L2/D2R2)
 * - test_dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat
 *   (F0111 door-front pair blit)
 * - test_dm1_v1_viewport_d2l2_d2r2_f0115_thing_pass_pc34_compat
 *   (F0115 thing pass)
 * - test_dm1_v1_viewport_d2l2_d2r2_stairs_pit_dispatch_pc34_compat
 *   (stairs/pit dispatch)
 * - test_dm1_v1_viewport_d2l2_d2r2_f0108_floor_ornament_pc34_compat
 *   (F0108 floor-ornament pixel contract)
 * - test_dm1_v1_viewport_d2l2_d2r2_f0108_floor_ceiling_ornament_pc34_compat
 *   (F0108 floor+ceiling ornament two-pass)
 * - the integrated D0L2/D0R2, D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, D3C
 *   and CSB-lineage viewport gates.
 */

#define DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L2_D2R2_SIDE_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_SLOT_CAPACITY_PC34 16
#define DM1_V1_D2L2_D2R2_SIDE_WALL_CASE_CAPACITY_PC34 8
#define DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_ORDER_CAPACITY_PC34 10

typedef enum {
    DM1_V1_D2L2_D2R2_SIDE_WALL_SIDE_D2L2_PC34 = 0,
    DM1_V1_D2L2_D2R2_SIDE_WALL_SIDE_D2R2_PC34 = 1
} DM1_V1_D2L2D2R2SideWallSidePc34;

typedef enum {
    DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0678_PC34 = 0,
    DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0679_PC34 = 1
} DM1_V1_D2L2D2R2SideWallDispatchPc34;

typedef enum {
    DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C00_WALL_PC34 = 0,
    DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C05_TELEPORTER_PC34 = 1
} DM1_V1_D2L2D2R2SideWallElementPc34;

typedef struct {
    DM1_V1_D2L2D2R2SideWallSidePc34 side;
    const char *side_name;
    int view_square_index;
    const char *view_square_symbol;
    int wall_zone;
    const char *wall_zone_symbol;
    int native_wall_set_index;
    const char *native_wall_set_symbol;
    int flipped_wall_set_index;
    const char *flipped_wall_set_symbol;
    int f0104_f0105_call_line;
    int f0113_teleporter_call_line;
    int switch_end_line;
    int relative_depth;
    int relative_lateral;
    int f0128_call_line;
    int f0128_call_order_index;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2SideWallLanePc34;

typedef struct {
    DM1_V1_D2L2D2R2SideWallDispatchPc34 dispatcher;
    const char *dispatcher_symbol;
    DM1_V1_D2L2D2R2SideWallElementPc34 element;
    const char *element_symbol;
    int routes_to_f0104_native;
    int routes_to_f0105_flipped;
    int routes_to_f0113_teleporter;
    int has_return_after_draw;
    int call_line;
    int wall_set_index_used;
    int wall_zone_used;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2SideWallCasePc34;

typedef struct {
    int order_index;
    const char *name;
    int side;
    int f0128_call_line;
    int pre_f0678_dispatch;
    int post_f0678_dispatch;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2SideWallDispatchOrderPc34;

typedef struct {
    const char *sibling_name;
    int side_index;
    int reject_f0107_route;
    int reject_f0108_route;
    int reject_f0111_route;
    int reject_f0115_route;
    int reject_stairs_route;
    int reject_pit_route;
    int reject_door_route;
    int reject_alcove_route;
    int reject_corridor_route;
    int reject_f0128_depth;
    int reject_f0128_lateral;
    int reject_view_wall;
    int reject_wall_zone;
    const char *redmcsb_anchor;
} DM1_V1_D2L2D2R2SideWallSiblingRejectPc34;

typedef struct {
    int side;
    const char *side_name;
    int switch_dispatch_count;
    int wall_case_count;
    int teleporter_case_count;
    int non_wall_non_teleporter_case_count;
    int has_default_case;
    int has_break_after_wall_case;
    int has_return_after_wall_case;
    int has_break_after_teleporter_case;
    int has_return_after_teleporter_case;
    int f0107_call_count;
    int f0108_call_count;
    int f0111_call_count;
    int f0115_call_count;
    int f0104_native_call_count;
    int f0105_flipped_call_count;
    int f0113_teleporter_call_count;
    int f0128_caller_line;
    int f0128_relative_depth;
    int f0128_relative_lateral;
    int f0128_call_order;
    int view_square_index;
    int wall_zone;
    int wall_set_native_index;
    int wall_set_flipped_index;
    int opaque_pixel_value;
    int transparent_color;
    int c10_transparent_preserves_destination;
    int c10_opaque_writes_source;
    int synthetic_probe_collision_count;
    int synthetic_pixel_writes;
    int synthetic_pixel_skips;
    int side_left_of_center;
    int draws_before_sibling_right;
    int f0128_dispatches_after_d3c;
    int f0128_dispatches_before_d2l;
    int f0128_dispatches_after_d3l2;
    int f0128_dispatches_after_d3r2;
    int f0128_dispatches_after_d3l;
    int f0128_dispatches_after_d3r;
    int f0128_dispatches_before_d2r;
    int f0128_dispatches_before_d2c;
    int f0128_dispatches_before_d1l;
    int f0128_dispatches_before_d1r;
    int f0128_dispatches_before_d1c;
    int f0128_dispatches_before_d0l;
    int f0128_dispatches_before_d0r;
    int f0128_dispatches_before_d0c;
    int pc_fix_code_size_native_wall_offset;
    int pc_3_4_media720_path_enabled;
    int flipped_wall_and_footprints_branch_excluded_pc34;
    int zone_pair_c707_c708;
    int native_flipped_wall_set_swap;
    int contract_only_no_real_asset_parity;
    int no_graphics_dat_reads;
    int no_original_dos_pixel_parity;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
    DM1_V1_D2L2D2R2SideWallLanePc34 lanes[2];
    DM1_V1_D2L2D2R2SideWallCasePc34 cases[DM1_V1_D2L2_D2R2_SIDE_WALL_CASE_CAPACITY_PC34];
    DM1_V1_D2L2D2R2SideWallDispatchOrderPc34
        dispatch_order[DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_ORDER_CAPACITY_PC34];
    DM1_V1_D2L2D2R2SideWallSiblingRejectPc34 sibling_rejects[8];
} DM1_V1_D2L2D2R2SideWallDispatchModelPc34;

bool dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_builder_pc34(
    DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *out_model);

const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34(void);

uint32_t dm1_v1_viewport_d2l2_d2r2_side_wall_hash_model_pc34(
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model);

uint32_t dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34(void);

const DM1_V1_D2L2D2R2SideWallLanePc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_lane_at_pc34(size_t index);

const DM1_V1_D2L2D2R2SideWallCasePc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_case_at_pc34(size_t index);

const DM1_V1_D2L2D2R2SideWallDispatchOrderPc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_order_at_pc34(size_t index);

const DM1_V1_D2L2D2R2SideWallSiblingRejectPc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_sibling_reject_at_pc34(size_t index);

int dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_switch_count_pc34(
    int side_index);

int dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(
    int side_index,
    int element_index);

int dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(
    int side_index,
    int target_function);

int dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_call_order_pc34(
    int side_index);

int dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(
    int side_index,
    int other_call_line);

int dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(
    int side_index,
    int other_call_line);

uint8_t dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int dm1_v1_viewport_d2l2_d2r2_side_wall_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size);

int dm1_v1_viewport_d2l2_d2r2_side_wall_render_dispatch_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size);

const char *dm1_v1_viewport_d2l2_d2r2_side_wall_source_evidence_pc34(void);

const char *dm1_v1_viewport_d2l2_d2r2_side_wall_disjointness_note_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2L2_D2R2_SIDE_WALL_PC34_COMPAT_H */
