#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D3C_F0107_F0108_FIRST_BACKDROP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D3C_F0107_F0108_FIRST_BACKDROP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34 224
#define CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_HEIGHT_PC34 136
#define CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_TRANSPARENT_COLOR_PC34 10

typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
} CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34;

/* Routing/geometry evidence only. Real GRAPHICS.DAT material is required for
 * any D3C backdrop or ornament draw. */
typedef struct {
    int contract_only;
    int no_game_data_dependency;
    int viewport_width;
    int viewport_height;
    int view_square_d3c;
    int view_wall_d3c_front;
    int view_floor_d3c;
    int wall_ornament_zone_base;
    int wall_ornament_coordinate_set_stride;
    int floor_ornament_zone_base;
    int floor_ornament_coordinate_set_stride;
    int transparent_color;
    int first_backdrop_is_before_cell_routes;
    int f0107_before_f0108;
    int f0108_transparent_mask_preserves_destination;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 d3c_window;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 wall_ornament_window;
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 floor_ornament_window;
    const char *redmcsb_f0097_anchor;
    const char *redmcsb_f0098_f0128_backdrop_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0108_anchor;
    const char *redmcsb_f0118_anchor;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0127_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_anchor;
    const char *source_evidence;
} CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract;

const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_contract_pc34(void);

const char *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
