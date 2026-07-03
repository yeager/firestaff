#ifndef FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "memory_projectile_pc34_compat.h"

typedef struct {
    int src_x;
    int src_y;
    int dst_x;
    int dst_y;
    int width;
    int height;
    const uint16_t *mask_words;
    size_t mask_word_count;
} CSB_V1_ViewportCustomBackgroundMask;

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

    /* Optional live runtime overlays.  CSB V1 runtime owns these lists;
     * the viewport treats NULL as "no live projectile/explosion overlay". */
    const struct ProjectileList_Compat *runtime_projectiles;
    const struct ExplosionList_Compat *runtime_explosions;

    /* Optional CSBgraphics.dat CustomBackgrounds bridge. The CSB boot layer
     * owns the plan/cache/skin-def bytes; the viewport can select a
     * cell/default skin, decode CSBWin room mask geometry, and still prefer
     * caller-supplied synthetic masks for focused tests. */
    const void *csbgraphics_plan;
    const void *csbgraphics_cache;
    const uint16_t *custom_background_skin_def_words;
    size_t custom_background_skin_def_word_count;
    const uint8_t *custom_background_cell_skins;
    int custom_background_cell_skin_width;
    int custom_background_cell_skin_height;
    int custom_background_loaded_level;
    int custom_background_default_skin;
    int custom_background_selected_skin_num;
    int custom_background_used_default_skin;
    int custom_background_room_num;
    int custom_background_last_room_num;
    uint32_t custom_background_applied_room_mask;
    CSB_V1_ViewportCustomBackgroundMask custom_background_layer_masks[3];
    uint8_t custom_background_layer_mask_valid[3];
    int custom_background_applied_count;
} CSB_V1_ViewportConfig;

typedef struct {
    int room_num;
    int relative_forward;
    int relative_side;
    int call_order;
    int applies_before_cell_draw;
    int source_mask_slots;
    int source_bitmap_slots;
    int large_bitmap_min_bytes;
    int middle_bitmap_min_bytes;
    int near_bitmap_min_bytes;
    const char *csbwin_function;
    const char *source_lines;
} CSB_V1_ViewportCustomBackgroundSlotSpec;

typedef struct {
    const CSB_V1_ViewportCustomBackgroundSlotSpec *room_slot;
    int uses_csbwin_relative_translation;
    int checks_map_bounds_before_skin_lookup;
    int uses_cell_skin_before_default_skin;
    int skin_def_background_graphic_id;
    int skin_def_min_bytes;
    int large_bitmap_skin_def_index;
    int large_mask_skin_def_index;
    int large_mask_min_bytes;
    int large_bitmap_min_bytes;
    int middle_bitmap_skin_def_index;
    int middle_mask_skin_def_index;
    int middle_mask_min_bytes;
    int middle_bitmap_min_bytes;
    int near_bitmap_skin_def_index;
    int near_mask_skin_def_index;
    int near_mask_min_bytes;
    int near_bitmap_min_bytes;
    int applies_near_layer;
    int near_layer_room_num_limit;
    int selects_csd_i34_background_bitmap;
    int selects_redmcsb_base_bitmap;
    int extends_redmcsb_floor_ceiling_baseline;
    int applybackground_masked_composite;
    const char *csbwin_function;
    const char *source_lines;
} CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec;

typedef enum {
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L2 = 0,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R2,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L2,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R2,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D1L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D1R,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D0L,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D0C,
    CSB_V1_CUSTOM_BACKGROUND_VIEW_D0R
} CSB_V1_ViewportCustomBackgroundViewIndex;

typedef struct {
    const uint8_t *bitmap;
    const uint8_t *mask;
    int byte_width;
    int height;
    int is_valid;
} CSB_V1_ViewportCustomBackgroundSelection;

typedef enum {
    CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE = 0,
    CSB_V1_CUSTOM_BACKGROUND_LAYER_MIDDLE = 1,
    CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR = 2
} CSB_V1_ViewportCustomBackgroundLayer;

typedef struct {
    CSB_V1_ViewportCustomBackgroundLayer layer;
    int bitmap_skin_def_index;
    int mask_skin_def_index;
    int bitmap_min_bytes;
    int mask_min_bytes;
    int applies_to_room;
} CSB_V1_ViewportCustomBackgroundLayerPlan;

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
    int view_wall_index;
    int ordinal_zero_skips_blit;
    int ordinal_to_index_delta;
    int native_bitmap_index_increment;
    int zone_base;
    int coordinate_set_stride;
    int scale_x;
    int scale_y;
    int horizontal_flip;
    int transparent_color;
    int uses_scaled_bitmap;
    int uses_f0791_blit;
    const char *ornament_ordinal_slot;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentBlitSpec;

typedef struct {
    int view_square;
    int view_wall_index;
    int evaluates_alcove_predicate;
    int updates_facing_alcove_state;
    int updates_facing_vi_altar_state;
    int updates_facing_fountain_state;
    int updates_wall_clickbox;
    int draws_champion_portrait_overlay;
    int uses_d3_i34_scaled_bitmap_path;
    int derived_bitmap_cache_slot;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentSideEffectSpec;

typedef struct {
    int view_square;
    int view_wall_index;
    int ornament_ordinal_slot;
    int returns_alcove_to_square_draw;
    int uses_d2_scaled_bitmap_path;
    int uses_d1_native_bitmap_path;
    int native_bitmap_index_increment;
    int derived_bitmap_index_increment;
    int scale;
    int horizontal_flip;
    int updates_d1_front_state;
    int updates_wall_clickbox;
    int draws_champion_portrait_overlay;
    int zone_base;
    int coordinate_set_stride;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportWallOrnamentD1D2PathSpec;

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
    int f0115_projectile_g2028_row;
    int f0115_projectile_zone_base;
    int f0115_projectile_zone_stride;
    int f0115_projectile_restarts_thing_list;
    int f0115_projectile_requires_cell_match;
    int f0115_projectile_suppresses_depth3_front_cells;
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
    int redmcsb_view_square_index;
    int view_depth;
    int object_visibility_row;
    int object_zone_base;
    int object_zone_cell_stride;
    int shifts_objects_and_creatures;
    int shift_set_index;
    int pile_shift_advances_per_object;
    int transparent_color;
    int uses_f0791_blit;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportObjectBlitSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int projectile_visibility_row;
    int projectile_zone_base;
    int projectile_zone_cell_stride;
    int requires_projectile_type;
    int requires_thing_cell_match;
    int restarts_thing_list;
    int suppresses_depth3_front_cells;
    int suppresses_depth0_back_cells;
    int derived_bitmap_cache_slot_for_scaled_path;
    int transparent_color;
    int uses_f0791_blit;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportProjectileBlitSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int creature_visibility_row;
    int requires_group_marker;
    int rejects_missing_creature_row;
    int creature_zone_base;
    int creature_coordinate_set_stride;
    int creature_zone_cell_stride;
    int shifts_objects_and_creatures;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportCreatureVisibilitySpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int view_depth;
    int explosion_row;
    int field_aspect_index;
    int restarts_thing_list_after_cells;
    int rebirth_requires_visible_row_and_cell_match;
    int fluxcage_defers_to_field;
    int fluxcage_field_zone;
    int rebirth_step1_zone_base;
    int rebirth_step2_zone_base;
    int centered_zone_base;
    int side_zone_base;
    int side_zone_cell_stride;
    int transparent_color;
    int uses_f0791_blit;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportExplosionBlitSpec;

typedef struct {
    int view_square;
    int redmcsb_view_square_index;
    int draws_only_for_teleporter;
    int after_thing_pass;
    int field_aspect_index;
    int field_zone;
    int uses_f0113_draw_field;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportTeleporterFieldSpec;

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
    int destroyed_state;
    int destroyed_mask_ornament_ordinal;
    int destroyed_mask_uses_view_ornament_index;
    int transparent_color;
    const char *redmcsb_function;
    const char *source_lines;
} CSB_V1_ViewportDoorPanelBlitSpec;

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg);
void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set);
void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id);
size_t csb_v1_viewport_custom_background_slot_spec_count(void);
const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec(size_t index);
const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec_for_room(int room_num);
size_t csb_v1_viewport_custom_background_bitmap_application_spec_count(void);
const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec(size_t index);
const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec_for_room(int room_num);
int csb_v1_viewport_custom_background_translate_cell(
    const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *spec,
    int party_x,
    int party_y,
    int facing,
    int *out_x,
    int *out_y);
CSB_V1_ViewportCustomBackgroundSelection
csb_v1_viewport_custom_background_load_and_select_pc34(
    const uint8_t *skin_def,
    size_t skin_def_size,
    CSB_V1_ViewportCustomBackgroundViewIndex view_index);
size_t csb_v1_viewport_custom_background_layer_plan_pc34(
    int room_num,
    CSB_V1_ViewportCustomBackgroundLayerPlan *out_layers,
    size_t out_capacity);
int csb_v1_viewport_custom_background_apply_aligned_mask_pc34(
    const CSB_V1_ViewportCustomBackgroundMask *mask,
    const uint32_t *bitmap_words,
    size_t bitmap_word_count,
    uint32_t *viewport_words,
    size_t viewport_word_count,
    int viewport_width_pixels);

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

size_t csb_v1_viewport_wall_ornament_blit_spec_count(void);
const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec(size_t index);
const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec_for_square(int view_square);
int csb_v1_viewport_wall_ornament_blit_zone(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                            int coordinate_set);
int csb_v1_viewport_wall_ornament_native_bitmap_index(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                                      int base_native_bitmap_index);
int csb_v1_viewport_wall_ornament_blit_pixels(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                              const uint8_t *source,
                                              int source_stride,
                                              uint8_t *destination,
                                              int destination_stride,
                                              int width,
                                              int height);

size_t csb_v1_viewport_wall_ornament_side_effect_spec_count(void);
const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec(size_t index);
const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec_for_square(int view_square);

size_t csb_v1_viewport_wall_ornament_d1d2_path_spec_count(void);
const CSB_V1_ViewportWallOrnamentD1D2PathSpec *csb_v1_viewport_get_wall_ornament_d1d2_path_spec(size_t index);
int csb_v1_viewport_wall_ornament_d1d2_path_zone(
    const CSB_V1_ViewportWallOrnamentD1D2PathSpec *spec,
    int coordinate_set);

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

size_t csb_v1_viewport_object_blit_spec_count(void);
const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec(size_t index);
const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec_for_square(int view_square);
int csb_v1_viewport_object_blit_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                     unsigned char view_cell);
int csb_v1_viewport_object_blit_layout_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                            unsigned char view_cell);

size_t csb_v1_viewport_projectile_blit_spec_count(void);
const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec(size_t index);
const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec_for_square(int view_square);
int csb_v1_viewport_projectile_blit_zone(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                         unsigned char view_cell);
int csb_v1_viewport_projectile_blit_pixels(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                           int flip_flags,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride,
                                           int width,
                                           int height);

size_t csb_v1_viewport_creature_visibility_spec_count(void);
const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec(size_t index);
const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec_for_square(int view_square);
int csb_v1_viewport_creature_visibility_zone(const CSB_V1_ViewportCreatureVisibilitySpec *spec,
                                             int coordinate_set,
                                             unsigned char view_cell);

size_t csb_v1_viewport_explosion_blit_spec_count(void);
const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec(size_t index);
const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec_for_square(int view_square);
int csb_v1_viewport_explosion_rebirth_step1_zone(const CSB_V1_ViewportExplosionBlitSpec *spec);
int csb_v1_viewport_explosion_rebirth_step2_zone(const CSB_V1_ViewportExplosionBlitSpec *spec);
int csb_v1_viewport_explosion_centered_zone(const CSB_V1_ViewportExplosionBlitSpec *spec);
int csb_v1_viewport_explosion_side_zone(const CSB_V1_ViewportExplosionBlitSpec *spec,
                                        unsigned char view_cell);

size_t csb_v1_viewport_teleporter_field_spec_count(void);
const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec(size_t index);
const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec_for_square(int view_square);

size_t csb_v1_viewport_door_panel_blit_spec_count(void);
const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index);
const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square);
int csb_v1_viewport_door_panel_first_half_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                               int door_state,
                                               int horizontal_door);
int csb_v1_viewport_door_panel_final_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                          int door_state,
                                          int horizontal_door);
int csb_v1_viewport_door_panel_blit_pixels(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                           int door_state,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride);

const char *csb_v1_viewport_source_evidence(void);

#endif
