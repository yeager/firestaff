#ifndef FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/* CSB V1 Viewport — CSB-specific rendering differences
 *
 * CSB shares the DM1 viewport engine but has:
 * - Different wall sets (CSB dungeon themes)
 * - Custom room backgrounds (per DSA script)
 * - Extended creature graphics
 * - Prison door / intro sequence renderer
 *
 * Source: CSBWin/Viewport.cpp (7290 lines)
 * Source: CSBWin/Graphics.cpp (3186 lines)
 * Base: ReDMCSB DUNVIEW.C (shared viewport core)
 */

typedef struct {
    int wall_set_index;
    int custom_background;
    int prison_door_open;  /* 0-100 open percentage for intro */
    int has_custom_ceiling;
    uint32_t ambient_color;

    /* Viewport pixel buffer (224×136, the dungeon view area).
     * Points into the global g_framebuffer or a screen buffer.
     * Viewport occupies pixel rows [33..168] of a 320×200 screen. */
    uint8_t *viewport_pixels;
    int      viewport_stride;  /* bytes per row (320 for screen) */

    /* Dungeon data for rendering decisions.
     * When viewport_pixels is NULL, render calls are no-ops. */
    const uint8_t *dungeon_grid;
    int dungeon_width;
    int dungeon_height;
} CSB_V1_ViewportConfig;

typedef struct {
    int view_square;
    int wall_zone;
    int draws_wall_ornament;
    int ornament_ordinal_slot;
    int view_wall_index;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentRouteSpec;

typedef struct {
    int view_square;
    int floor_view_index;
    int draws_corridor_floor_ornament;
    int draws_pit_floor_ornament;
    int draws_door_front_floor_ornament;
    int door_front_pass1_order;
    int door_front_pass2_order;
    int door_zone;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportFloorOrnamentRouteSpec;

typedef struct {
    int view_square;
    int floor_view_index;
    int ordinal_zero_skips_blit;
    int ordinal_to_index_delta;
    int native_bitmap_index_increment;
    int coordinate_set_index;
    int zone_base;
    int coordinate_set_stride;
    int horizontal_flip;
    int transparent_color;
    const char *door_front_ordinal_slot;
    const char *corridor_pit_ordinal_slot;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportFloorOrnamentBlitSpec;

typedef struct {
    int view_square;
    int floor_view_index;
    int door_front_floor_ornament_order;
    int door_front_rear_f0115_order;
    int door_front_f0111_order;
    int door_front_front_f0115_order;
    uint16_t door_front_rear_cell_order;
    uint16_t door_front_front_cell_order;
    uint16_t corridor_cell_order;
    uint16_t side_cell_order;
    int f0115_objects_layer_order;
    int f0115_creatures_layer_order;
    int f0115_projectiles_layer_order;
    int f0115_explosions_layer_order;
    int f0115_explosions_after_all_cells;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportThingPassOrderSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int object_visibility_row;
    int requires_item_type_range;
    int requires_thing_cell_match;
    int suppresses_depth3_front_cells;
    int suppresses_depth0_back_cells;
    unsigned char first_visible_cell_ordinal;
    unsigned char last_visible_cell_ordinal;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportObjectVisibilitySpec;

typedef struct {
    int view_square;
    int door_zone_base;
    int closed_record_type;
    int closed_parent_record;
    int closed_parent_x;
    int closed_parent_y;
    int clip_record;
    int native_bitmap_width;
    int native_bitmap_height;
    int clipped_width;
    int clipped_height;
    int closed_dst_x;
    int closed_dst_y;
    int door_ornament_index;
    int skips_open_state;
    int shifts_zone_by_state;
    int horizontal_first_half_zone_offset;
    int horizontal_second_half_zone_offset;
    int transparent_color;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportDoorPanelBlitSpec;

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg);
void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set);
void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id);

/* Wire dungeon grid for element routing in back-wall rendering.
 * Must be called before render if using CSB back-wall squares (D3L2/D3R2).
 * Source: ReDMCSB DUNVIEW.C:6233 F0172_SetSquareAspect · dm1_viewport_3d_get_dungeon_element */
void csb_v1_viewport_set_dungeon_grid(CSB_V1_ViewportConfig *cfg,
                                       const uint8_t *grid,
                                       int width, int height);

/* Render one dungeon view frame using the DM1 viewport engine.
 * party_dir: facing direction (0=N, 1=E, 2=S, 3=W)
 * party_x, party_y: position on dungeon grid
 *
 * Uses dm1_viewport_3d_draw_frame internally; CSB config
 * selects CSB-specific wall set and custom backgrounds.
 *
 * No-op when cfg->viewport_pixels is NULL.
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y);

size_t csb_v1_viewport_wall_ornament_route_spec_count(void);
const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec(size_t index);
const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec_for_square(int view_square);

size_t csb_v1_viewport_floor_ornament_route_spec_count(void);
const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec(size_t index);
const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec_for_square(int view_square);

size_t csb_v1_viewport_floor_ornament_blit_spec_count(void);
const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec(size_t index);
const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec_for_square(int view_square);
int csb_v1_viewport_floor_ornament_blit_zone(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                             int coordinate_set);
int csb_v1_viewport_floor_ornament_native_bitmap_index(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                                       int base_native_bitmap_index);

size_t csb_v1_viewport_thing_pass_order_spec_count(void);
const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec(size_t index);
const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec_for_square(int view_square);

size_t csb_v1_viewport_object_visibility_spec_count(void);
const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec(size_t index);
const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec_for_square(int view_square);
int csb_v1_viewport_object_visibility_allows_cell(const CSB_V1_ViewportObjectVisibilitySpec *spec,
                                                  unsigned char cell_ordinal);

size_t csb_v1_viewport_door_panel_blit_spec_count(void);
const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index);
const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square);

const char *csb_v1_viewport_source_evidence(void);

#endif
