#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_D2_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_D2_F0108_FLOOR_ORNAMENT_OCCLUSION_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D3C/D2 F0108 floor-ornament occlusion gate.
 *
 * ReDMCSB anchors: DUNVIEW.C F0118/F0119/F0120/F0121 call
 * F0108_DUNGEONVIEW_DrawFloorOrnament after the pit bitmap and before
 * F0115, with the BUG0_64 comment that floor ornaments draw over open
 * pits. The four source lines pinned here are D3C:6814, D2L:7020,
 * D2R:7213, and D2C:7357. F0108:3940-4011 owns the M558 ordinal,
 * MASK0x8000 footprint recursion, C10 transparent blit, and PC 3.4
 * C1500 + coordinateSet * 11 + viewFloor zone math. F0128:8318-8542
 * dispatches these lanes far-to-near after D3L/D3R/D3L2/D3R2 and before
 * the D1/D0 foreground pass.
 *
 * This does not duplicate the existing D1C, D3L2/D3R2, or D3L/D3R
 * occlusion gates. It is asset-free and makes no real-asset or original
 * DOS pixel-parity claim.
 */

#define DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34 4
#define DM1_V1_D3C_D2_FOCCL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D3C_D2_FOCCL_FOOTPRINT_MASK_PC34 0x8000u
#define DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_BASE_PC34 1500
#define DM1_V1_D3C_D2_FOCCL_FLOOR_ZONE_STRIDE_PC34 11

typedef enum {
    DM1_V1_D3C_D2_FOCCL_LANE_D3C_PC34 = 0,
    DM1_V1_D3C_D2_FOCCL_LANE_D2L_PC34 = 1,
    DM1_V1_D3C_D2_FOCCL_LANE_D2R_PC34 = 2,
    DM1_V1_D3C_D2_FOCCL_LANE_D2C_PC34 = 3
} DM1_V1_D3CD2F0108FloorOrnamentOcclusionLanePc34;

typedef struct {
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionLanePc34 lane;
    const char *lane_name;
    const char *redmcsb_function;
    int view_square;
    int view_floor;
    int source_line;
    int cell_order;
    int floor_zone_at_coordinate_zero;
    const char *pit_source_lines;
    const char *f0108_source_lines;
    const char *f0115_source_lines;
} DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34;

typedef struct {
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34 anchors[DM1_V1_D3C_D2_FOCCL_ANCHOR_COUNT_PC34];
    int c10_transparent_color;
    int floor_zone_base;
    int floor_zone_stride;
    int floor_ornament_ordinal_slot;
    int first_thing_slot;
    int bug0_64_anchor_count;
    int f0108_ordinal_zero_skips_blit;
    int f0108_footprint_mask_recurses;
    int f0108_blit_uses_c10_transparent;
    int no_graphics_dat_reads;
    int source_locked_contract_only;
    int no_original_dos_pixel_parity;
    uint32_t deterministic_hash;
    const char *source_evidence;
    const char *disjointness_note;
} DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34;

typedef struct {
    int ok;
    int assertions;
    int failures;
    int model_builder_ok;
    int hash_stable;
    int anchor_count_four;
    int bug0_64_count_four;
    int zones_match_stride;
    int occlusion_accepts_nonzero_ordinals;
    int occlusion_rejects_zero_ordinal;
    int source_evidence_present;
    int disjointness_note_present;
    uint32_t deterministic_hash;
} DM1_V1_D3CD2F0108FloorOrnamentOcclusionSelfTestResultPc34;

bool dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_builder_pc34(
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *out_model);

const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *
dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_default_model_pc34(void);

uint32_t dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_hash_model_pc34(
    const DM1_V1_D3CD2F0108FloorOrnamentOcclusionModelPc34 *model);

uint32_t dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_deterministic_hash_pc34(void);

unsigned int dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_count_pc34(void);

const DM1_V1_D3CD2F0108FloorOrnamentOcclusionAnchorPc34 *
dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_anchor_at_pc34(size_t index);

int dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_zone_pc34(
    int coordinate_set,
    int view_floor);

bool dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_lane_occludes_pc34(
    DM1_V1_D3CD2F0108FloorOrnamentOcclusionLanePc34 lane,
    unsigned int floor_ornament_ordinal);

const char *dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_source_evidence_pc34(void);

const char *dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_disjointness_note_pc34(void);

int dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_self_test_pc34(void);

const DM1_V1_D3CD2F0108FloorOrnamentOcclusionSelfTestResultPc34 *
dm1_v1_viewport_d3c_d2_f0108_floor_ornament_occlusion_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
