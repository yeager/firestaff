#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0107_WALL_ORNAMENT_WITH_F0111_DOOR_FRONT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2C_F0107_WALL_ORNAMENT_WITH_F0111_DOOR_FRONT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 112
#define CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34 61
#define CSB_V1_D2C_F0107_F0111_TRANSPARENT_COLOR_PC34 10

typedef struct {
    int source_locked_contract_only;
    int no_game_data_load;
    int view_square_d2c;
    int relative_depth;
    int relative_lateral;
    int wall_element;
    int door_front_element;
    int front_wall_ornament_ordinal_slot;
    int front_wall_view_index;
    int wall_bitmap_index;
    int wall_zone;
    int wall_frame_view_square;
    int f0107_before_f0111;
    int f0107_alcove_cell_order;
    int f0111_doorpass1_cell_order;
    int f0111_doorpass2_cell_order;
    int f0111_closed_door_state;
    int f0111_door_zone;
    int f0111_door_ornament_view;
    int f0111_door_bitmap_width;
    int f0111_door_bitmap_height;
    int f0111_transparent_color;
    int ornament_x1;
    int ornament_x2;
    int ornament_center_x1;
    int ornament_center_x2;
    int door_x1;
    int door_x2;
    int left_visible_x1;
    int left_visible_x2;
    int right_visible_x1;
    int right_visible_x2;
    const char *redmcsb_f0107_anchor;
    const char *redmcsb_f0111_anchor;
    const char *redmcsb_f0121_anchor;
    const char *redmcsb_defs_anchor;
    const char *csb_lineage_viewport_anchor;
    const char *source_evidence;
} CSB_V1_ViewportD2CF0107F0111SpecPc34;

typedef struct {
    int ok;
    int wall_pixels;
    int ornament_pixels_before_door;
    int door_pixels;
    int ornament_center_pixels_covered_by_door;
    int ornament_left_pixels_visible_after_door;
    int ornament_right_pixels_visible_after_door;
    int center_samples_opaque;
    int left_samples_visible;
    int right_samples_visible;
    int draw_order_f0107;
    int draw_order_f0111;
    const char *source_evidence;
} CSB_V1_ViewportD2CF0107F0111TracePc34;

const CSB_V1_ViewportD2CF0107F0111SpecPc34 *
csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34(void);

int csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
    uint8_t *canvas,
    size_t canvas_size,
    CSB_V1_ViewportD2CF0107F0111TracePc34 *out_trace);

int csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
    const uint8_t *canvas,
    size_t canvas_size,
    int x,
    int y);

const char *
csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
