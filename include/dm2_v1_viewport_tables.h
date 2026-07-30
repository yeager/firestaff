#ifndef FIRESTAFF_DM2_V1_VIEWPORT_TABLES_H
#define FIRESTAFF_DM2_V1_VIEWPORT_TABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t dm2_v1_vp_render_order[20];
extern const uint8_t dm2_v1_vp_column_count[23];
extern const int16_t dm2_v1_vp_wall_face_near[18];
extern const int8_t dm2_v1_vp_wall_ornament_near[18];
extern const int16_t dm2_v1_vp_wall_face_mid[32];
extern const int8_t dm2_v1_vp_wall_ornament_mid[32];
extern const int8_t dm2_v1_vp_wall_ornament_mid_alt[32];
extern const uint8_t dm2_v1_vp_wall_visible[16];
extern const int16_t dm2_v1_vp_wall_rect_id[16];
extern const int8_t dm2_v1_vp_depth_index[5];
extern const int16_t dm2_v1_vp_floor_item_near[14];

extern const int8_t dm2_v1_vp_tile_walk_dx[8][2];
extern const int16_t dm2_v1_vp_tile_scan_dx[4][2];
extern const int8_t dm2_v1_vp_facing_remap[32];
extern const int8_t dm2_v1_vp_facing_reverse[4];
extern const int8_t dm2_v1_vp_creature_order[8][4];
extern const int8_t dm2_v1_vp_creature_subpos[4];

extern const int16_t dm2_v1_vp_champion_pane_rect[8];
extern const int16_t dm2_v1_vp_champion_pane_rect2[10];

extern const int8_t dm2_v1_vp_light_curve[16];

extern const uint32_t dm2_v1_vp_palette_mask_5bit[8];
extern const uint32_t dm2_v1_vp_palette_mask_full[8];
extern const uint32_t dm2_v1_vp_palette_mask_rgb[8];
extern const uint32_t dm2_v1_vp_palette_alpha[4];

extern const int8_t dm2_v1_vp_column_offset[23];
extern const int8_t dm2_v1_vp_tile_neighbor[23][4];
extern const int8_t dm2_v1_vp_tile_pos[23][2];
extern const int8_t dm2_v1_vp_tile_mirror[23];
extern const int8_t dm2_v1_vp_tile_depth[23];
extern const int8_t dm2_v1_vp_tile_to_near[23];
extern const int8_t dm2_v1_vp_tile_to_far[23];
extern const int8_t dm2_v1_vp_depth_light[5];

extern const int8_t dm2_v1_vp_creature_cell_pos[16][2];
extern const int8_t dm2_v1_vp_creature_cell_visible[16];
extern const int8_t dm2_v1_vp_creature_cell_depth[16];

extern const int16_t dm2_v1_vp_door_face[16];
extern const int8_t dm2_v1_vp_door_flip[16];
extern const int8_t dm2_v1_vp_door_ornament[16];
extern const int8_t dm2_v1_vp_door_ornament_alt[16];
extern const int16_t dm2_v1_vp_door_rect[16];

extern const int8_t dm2_v1_vp_floor_ornament[14][2];
extern const int8_t dm2_v1_vp_floor_depth[14];
extern const int8_t dm2_v1_vp_floor_item_id[14];

const char *dm2_v1_viewport_tables_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
