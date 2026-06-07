#ifndef CSB_V1_VIEWPORT_F0108_FOOTPRINTS_PC34_COMPAT_H
#define CSB_V1_VIEWPORT_F0108_FOOTPRINTS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int contract_only;
    int footprint_mask;
    int footprint_index;
    int footprint_ordinal;
    int ordinal_zero_skips_blit;
    int clears_mask_before_base_draw;
    int mask_only_skips_base_draw;
    int footprints_recurse_after_base;
    int recursion_preserves_view_floor;
    int recursion_stops_after_footprints;
    int csb_i34_floor_view_d3l2;
    int csb_i34_floor_view_d3r2;
    int csb_i34_floor_view_d1c;
    int csb_i34_floor_view_d2c;
    int csb_i34_floor_view_d3c;
    int zone_base;
    int coordinate_set_stride;
    int coordinate_set_index;
    int transparent_color;
    int flip_horizontal_mask;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_dungeon_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_specific_anchor;
    const char *source_evidence;
} CSB_V1_ViewportF0108FootprintsPc34Contract;

typedef struct {
    int ok;
    int draw_count;
    int base_drawn;
    int footprints_drawn;
    int base_ornament_index;
    int footprints_ornament_index;
    int cleared_base_ordinal;
    int recursive_ordinal;
    int recursive_view_floor;
    int base_zone;
    int footprints_zone;
    int base_flip;
    int footprints_flip;
    int base_transparent_color;
    int footprints_transparent_color;
    int recursion_stops;
    const char *source_evidence;
} CSB_V1_ViewportF0108FootprintsPc34Plan;

const CSB_V1_ViewportF0108FootprintsPc34Contract *
csb_v1_viewport_f0108_footprints_contract_pc34(void);

const char *
csb_v1_viewport_f0108_footprints_source_evidence_pc34(void);

int csb_v1_viewport_f0108_footprints_plan_pc34(
    uint16_t floor_ornament_ordinal,
    int view_floor_index,
    int use_flipped_wall_and_footprints_bitmaps,
    CSB_V1_ViewportF0108FootprintsPc34Plan *out_plan);

#ifdef __cplusplus
}
#endif

#endif
