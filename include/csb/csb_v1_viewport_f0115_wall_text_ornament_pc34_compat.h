#ifndef FIRESTAFF_CSB_V1_VIEWPORT_F0115_WALL_TEXT_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_F0115_WALL_TEXT_ORNAMENT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 320
#define CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34 200
#define CSB_V1_F0115_WALL_TEXT_ORNAMENT_VIEWPORT_WIDTH_PC34 224
#define CSB_V1_F0115_WALL_TEXT_ORNAMENT_VIEWPORT_HEIGHT_PC34 136

typedef struct {
    int source_locked_contract_only;
    int no_original_dos_pixel_parity_claim;
    int no_game_data_load;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int view_square_d1c;
    int view_depth_d1;
    int view_lane_center;
    int wall_element;
    int front_wall_ornament_ordinal_slot;
    int d1c_front_wall_view_index;
    int wall_text_ornament_ordinal;
    int wall_text_ornament_index;
    int wall_text_coordinate_set;
    int coordinate_height_index_c5;
    int transparent_color_c10;
    int f0107_reports_alcove;
    int f0115_cell_order_alcove;
    int f0115_first_nibble_alcove;
    int text_box_x;
    int text_box_y;
    int text_box_width;
    int text_box_height;
    const char *redmcsb_f0115_anchor;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0124_anchor;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_viewport_anchor;
    const char *source_evidence;
} CSB_V1_ViewportF0115WallTextOrnamentPc34Spec;

typedef struct {
    int ok;
    int route_enabled;
    int wall_pixels;
    int text_pixels_non_zero;
    int transparent_pixels_preserved;
    int outside_viewport_preserved;
    int f0107_order;
    int f0115_order;
    int first_text_x;
    int first_text_y;
    uint32_t framebuffer_hash;
    const char *source_evidence;
} CSB_V1_ViewportF0115WallTextOrnamentPc34Trace;

const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *
csb_v1_viewport_f0115_wall_text_ornament_pc34_spec(void);

const char *
csb_v1_viewport_f0115_wall_text_ornament_pc34_source_evidence(void);

int csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
    const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *spec,
    int wall_ornament_ordinal,
    int view_wall_index);

int csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size,
    CSB_V1_ViewportF0115WallTextOrnamentPc34Trace *out_trace);

int csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
    const uint8_t *framebuffer,
    size_t framebuffer_size,
    int x,
    int y);

uint32_t csb_v1_viewport_f0115_wall_text_ornament_hash_pc34(
    const uint8_t *framebuffer,
    size_t framebuffer_size);

#ifdef __cplusplus
}
#endif

#endif
