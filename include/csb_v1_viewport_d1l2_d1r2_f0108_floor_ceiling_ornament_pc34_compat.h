#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D1L2_D1R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D1L2_D1R2_F0108_FLOOR_CEILING_ORNAMENT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only CSB V1 D1L2/D1R2 floor+ceiling+ornament source lock.
 * ReDMCSB anchors used: DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament
 * lines 3940-4011, F0104 lines 3113-3156, F0105 lines 3185-3247,
 * F0107 lines 3502-3938, F0115 row guard lines 5668-5671,
 * F0122_DUNGEONVIEW_DrawSquareD1L lines 7391-7557,
 * F0123_DUNGEONVIEW_DrawSquareD1R lines 7559-7725, F0128 lines 8503-8533;
 * DUNGEON.C F0163 lines 1769-1838, F0164 lines 1840-1905, F0172 lines
 * 2466-2523; DEFS.H line 2088, 2443-2452, 2582-2583, 2596-2604,
 * 2662/2668-2677, 2681-2707, 4139-4153, and 4202-4207.
 * CSB-lineage anchor: Viewport.cpp lines 1192-1209, 1865-1879,
 * 1903-1915, and 6507-6548.
 *
 * PASS test_csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat
 */

typedef enum {
    CSB_V1_D1L2_D1R2_F0108_SIDE_D1L2_PC34 = 1,
    CSB_V1_D1L2_D1R2_F0108_SIDE_D1R2_PC34 = 2
} CSB_V1_D1L2D1R2F0108SidePc34;

typedef struct {
    int side;
    const char *name;
    int redmcsb_function_number;
    int f0128_dispatch_index;
    int relative_depth;
    int relative_lateral;
    int view_square;
    int view_lane;
    int view_depth;
    int g2028_row;
    int g2033_row;
    int g2034_row;
    int field_aspect_index;
    int floor_view;
    int floor_view_flipped_by_f0108;
    int floor_ornament_zone_base;
    int ceiling_graphic_id;
    int ceiling_zone;
    int ceiling_flip_horizontal;
    unsigned int thing_pass_order;
    int thing_pass_after_floor_ceiling;
    int f0115_row_guard_g2028_nonnegative;
    int wall_ornament_view;
    int view_wall_ordinal_after_m579;
    int custom_background_mask_after_floor_ceiling;
    int custom_background_uses_mask;
    int f0163_not_called;
    int f0164_not_called;
    int f0172_square_aspect_source;
    const char *redmcsb_anchor;
    const char *lineage_anchor;
} CSB_V1_D1L2D1R2F0108SpecPc34;

typedef struct {
    int ok;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int assertions;
    int failures;
    int floor_recursion_calls;
    int ceiling_copies;
    int thing_pass_calls;
    int dispatch_entries;
    int row_guard_rejections;
    int mutation_rejections;
    uint32_t deterministic_hash;
} CSB_V1_D1L2D1R2F0108SelfTestResultPc34;

size_t csb_v1_viewport_d1l2_d1r2_f0108_spec_count_pc34(void);

const CSB_V1_D1L2D1R2F0108SpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0108_spec_at_pc34(size_t index);

const CSB_V1_D1L2D1R2F0108SpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0108_spec_for_side_pc34(int side);

uint8_t csb_v1_viewport_d1l2_d1r2_f0108_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel);

int csb_v1_viewport_d1l2_d1r2_f0108_should_flip_floor_pc34(
    int floor_view,
    int floor_ornament_index,
    int use_flipped_wall_and_footprints_bitmaps);

int csb_v1_viewport_d1l2_d1r2_f0108_row_guard_accepts_pc34(int view_square);

int csb_v1_viewport_d1l2_d1r2_f0108_trace_order_pc34(
    const CSB_V1_D1L2D1R2F0108SpecPc34 *spec,
    unsigned int floor_ornament_ordinal,
    int apply_custom_background_mask,
    uint32_t seed,
    CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *out_result);

int run_csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_self_test(void);

const CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *
csb_v1_viewport_d1l2_d1r2_f0108_last_self_test_result_pc34(void);

const char *csb_v1_viewport_d1l2_d1r2_f0108_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
