#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3L2_F0115_PROJECTILE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3L2_F0115_PROJECTILE_PC34_COMPAT_H

#include <stdint.h>

typedef struct {
    int view_square;
    int view_depth;
    int view_lane;
    int projectile_g2028_row;
    int excluded_d3r2_g2028_row;
    int projectile_zone_base;
    int projectile_zone_cell_stride;
    int projectile_thing_type;
    int requires_projectile_type_c14;
    int requires_cell_match;
    int restarts_thing_list;
    int suppresses_depth3_front_cells;
    int suppresses_depth0_back_cells;
    int projectile_scale_index_depth_shift;
    int projectile_scale_index_cell_shift;
    int projectile_kinetic_minimum;
    int projectile_kinetic_floor;
    int projectile_derived_bitmap_none;
    int projectile_uses_f0791_blit;
    int dynamic_flip_flags_preserved;
    int transparent_color;
    int wall_f0121_f0104_route;
    int teleporter_f0113_route;
    int source_locked_contract_only;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34;

const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *
csb_v1_viewport_d3l2_f0115_projectile_route_spec_pc34(void);

int csb_v1_viewport_d3l2_f0115_projectile_accepts_thing_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    int thing_type,
    unsigned char thing_cell,
    unsigned char view_cell);

int csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    unsigned char view_cell);

int csb_v1_viewport_d3l2_f0115_projectile_scale_index_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    unsigned char view_cell);

int csb_v1_viewport_d3l2_f0115_projectile_apply_kinetic_scale_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    int base_scale,
    int kinetic_energy,
    int scales_with_kinetic_energy);

int csb_v1_viewport_d3l2_f0115_projectile_apply_c10_blit_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height,
    int flip_horizontal);

const char *csb_v1_viewport_d3l2_f0115_projectile_source_evidence_pc34(void);

#endif
