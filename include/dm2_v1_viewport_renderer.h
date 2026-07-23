#ifndef FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H
#define FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H
#include <stdint.h>
#include "dm2_v1_weather_gdat.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"

typedef struct DM2_V1_GdatHudM11CommandPlan DM2_V1_GdatHudM11CommandPlan;
typedef struct DM2_V1_GdatWallM11CommandPlan DM2_V1_GdatWallM11CommandPlan;
typedef struct DM2_V1_GdatDoorOverlayM11CommandPlan DM2_V1_GdatDoorOverlayM11CommandPlan;

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V1 Viewport Renderer — Skullkeep rendering pipeline
 *
 * Phase 4: Source-lock wall/floor/door/ornament/item/creature/
 * projectile/cloud rendering, palette/light handling, UI surfaces,
 * title/intro assets, GDAT-backed animation frames.
 *
 * Architecture:
 *   DM2 viewport is 320×200 game pixels, same as DM1/CSB.
 *   Status bar: top 28px  (champion health/magic/conditions)
 *   Dungeon view: 320×144px (walls, floor, creatures, items)
 *   Action strip: bottom 28px (action icons: Attack/Cast/Use/Drop/Move)
 *   Portrait panel: right 80×144px (champion portraits)
 *
 * Draw order (c_gui_vp.cpp reference, same as DM1):
 *   1. Background: fill E_COL00 (black)
 *   2. Floor tiles: bottom-up tile rendering (near tiles first)
 *   3. Walls: per-distance-column vertical strip rendering
 *   4. Ceiling: ceiling graphics atop walls
 *   5. Door overlay: animated tweened open/close transitions
 *   6. Sprite pass: creatures and items, depth-sorted per square
 *   7. Weather overlay: rain/fog/storm (blitline_48 16→8-bit)
 *   8. UI pass: HUD, champion panels, dialogue via DM2_blit_specialeffects
 *
 * Source: SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULLWIN/SKWIN/c_gui_vp.cpp — viewport blit order
 *         SKULLWIN/SKWIN/c_gfx_blit.h — blitter function matrix
 *         SKULLWIN/SKWIN/c_gfx_main.cpp — DM2_FILL, DM2_blit
 *         docs/dm2_graphics.md — drawing pipeline audit
 *         docs/dm2_walls.md — wall/door/floor rendering specifics
 *         docs/dm2_creatures_gfx.md — creature/item rendering
 *         docs/dm2_palette.md — DM2 palette system
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Viewport geometry ──────────────────────────────────────────── */
#define DM2_VP_WIDTH   320
#define DM2_VP_HEIGHT  200
#define DM2_VP_STATUS_BAR   28
#define DM2_VP_DUNGEON_H  144
#define DM2_VP_ACTION_STRIP 28
#define DM2_VP_CHROME_TOP  DM2_VP_STATUS_BAR
#define DM2_VP_CHROME_BOT  DM2_VP_ACTION_STRIP
#define DM2_VP_DUNGEON_Y  DM2_VP_STATUS_BAR
#define DM2_V1_HUD_ACTION_ICON_COUNT 5
#define DM2_V1_HUD_CHAMPION_SLOT_COUNT 4
#define DM2_V1_HUD_CHAMPION_NAME_MAX 8
/* skproject Champion::HeroType is an 8-bit save-record field (bound to the
 * runtime HUD by 1da849469); the portrait packing must not narrow it. */
#define DM2_V1_HUD_PORTRAIT_COUNT 256

/* ── Depth / distance rows ─────────────────────────────────────── */
/* DM2 uses the same 4-row perspective as DM1:
 *   D3C/D3L/D3R = back wall row (depth 3, smallest strips)
 *   D2C/D2L/D2R = mid wall row  (depth 2)
 *   D1C/D1L/D1R = near wall row (depth 1)
 *   D0C/D0L/D0R = forward wall  (depth 0, full height) */
#define DM2_DEPTH_ROWS  4
enum {
    DM2_SQ_D3C = 0, DM2_SQ_D3L, DM2_SQ_D3R,   /* back row */
    DM2_SQ_D2C,     DM2_SQ_D2L, DM2_SQ_D2R,    /* mid row */
    DM2_SQ_D1C,     DM2_SQ_D1L, DM2_SQ_D1R,    /* near row */
    DM2_SQ_D0C,     DM2_SQ_D0L, DM2_SQ_D0R,    /* forward */
    DM2_SQ_COUNT
};

/* View square flags */
typedef enum {
    DM2_SQF_NONE        = 0,
    DM2_SQF_HAS_WALL    = 1 << 0,
    DM2_SQF_HAS_DOOR    = 1 << 1,
    DM2_SQF_HAS_FLOOR_ORNAMENT = 1 << 2,
    DM2_SQF_HAS_WALL_ORNAMENT  = 1 << 3,
    DM2_SQF_HAS_CREATURE      = 1 << 4,
    DM2_SQF_HAS_ITEM          = 1 << 5,
    DM2_SQF_HAS_PROJECTILE    = 1 << 6,
    DM2_SQF_TRANSPARENT_WALL  = 1 << 7,  /* wall with window/open */
} DM2_SquareFlags;

/* SKProject DME.h::tileTypeIndex encodes PC G1 squares as 0=wall, 1=floor,
 * and 4=door, whereas the renderer's DM2_SquareType uses different numeric
 * values.  This conversion is the only admitted G1 terrain bridge. Doors
 * still require their direct DB0 route before they are rendered. */
int dm2_v1_viewport_g1_tile_class_to_square_type(uint8_t tile_class);

/* ── Wall frame ─────────────────────────────────────────────────── */
/* Wall frame descriptor — source rectangle within wall bitmap.
 * Derived from ReDMCSB DUNVIEW.C wall frame tables.
 * Used for both blit geometry and clipping gates. */
typedef struct {
    uint8_t left_x;
    uint8_t right_x;
    uint8_t top_y;
    uint8_t bottom_y;
    uint8_t byte_width;   /* source byte width */
    uint8_t height;
    uint8_t blit_x;       /* source blit offset X */
    uint8_t blit_y;       /* source blit offset Y */
} DM2_WallFrame;

/* DM2 wall frame table — 12 view squares, D3C..D0R.
 * Sourced from SKULLWIN c_gui_vp.cpp wall geometry constants. */
extern const DM2_WallFrame g_dm2_wall_frames[DM2_SQ_COUNT];

#define DM2_V1_VIEWPORT_GFX_FLOOR   (-1)
#define DM2_V1_VIEWPORT_GFX_CEILING (-2)
#define DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE (-0x100)
#define DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST 0x22
#define DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE (-0x200)
#define DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT 0x06
#define DM2_V1_VIEWPORT_GFX_DOOR_FRAME_D1C 0x07
#define DM2_V1_VIEWPORT_GFX_DOOR_FRAME_D2C 0x09
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE (-0x300)
#define DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE (-0xA0000)
#define DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE (-0xB0000)
#define DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE (-0xC0000)
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT 0x00
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D1C 0x00
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING 0x01u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL          0x02u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_DOOR          0x04u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CREATURE      0x08u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_ITEM          0x10u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_POSSESSION    0x20u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_CARRIED_ITEM  0x40u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_PROJECTILE    0x80u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_CORE      0x100u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_HUD_PORTRAIT  0x200u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WEATHER       0x400u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_TELEPORTER    0x800u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_SCENE_CONTROL 0x1000u
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_WALL_ORNAMENT 0x2000u
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D2C 0x01
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_OPENING_SHIFT 4
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK 0x0F
#define DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_FIELD_MASK 0x0F
#define DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE (-0x400)
#define DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED 0x00
#define DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_PUSHED 0x05
#define DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE (-0x500)
#define DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE (-0x20000)
#define DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_CREATURE_DIRECT_FIELD_BASE (-0x220000)
#define DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE (-0x40000)
#define DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT 16
#define DM2_V1_VIEWPORT_GFX_ITEM_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_ITEM_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE (-0x60000)
#define DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT 16
#define DM2_V1_VIEWPORT_GFX_PROJECTILE_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE (-0x80000)
#define DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD 0x00
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE (-0x81000)
#define DM2_V1_VIEWPORT_GFX_HUD_HAND_ACTION_BASE (-0x82000)
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR 0x02
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_STRIP 0x0a
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL 0x07
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_GOLD_BOX 0x09
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE 0x20
#define DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_BASE (-0xD00000)
#define DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING 0x01
#define DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR   0x00
#define DM2_V1_VIEWPORT_GFX_WEATHER_ENVIRONMENT_BASE (-0xD10000)
#define DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE (-0xE00000)
#define DM2_V1_VIEWPORT_GFX_DOOR_FRAME_GRAPHICSSET_BASE (-0xF00000)
#define DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET 0x01
#define DM2_V1_GDAT_SCENE_FLAG_OUTDOOR 0x20u
#define DM2_V1_VIEWPORT_GFX_TELEPORTER_MAP_CHIP (-0x1100000)
#define DM2_V1_VIEWPORT_GFX_FLOOR_GFX_MAP_CHIP_BASE (-0x1200000)
#define DM2_V1_VIEWPORT_GFX_WALL_GFX_MAP_CHIP_BASE (-0x1300000)
#define DM2_V1_VIEWPORT_GFX_DOOR_MAP_CHIP_BASE (-0x1400000)
#define DM2_V1_VIEWPORT_GFX_DIALOGUE_BOX (-0x1500000)

int dm2_v1_viewport_wall_field_for_square(int view_square);
int dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(int skproject_cell);
/* Source: SKProject dm2data.cpp::table1d7029 consumed by
 * c_gui_vp.cpp::DM2_DRAW_DUNGEON_TILES. Returns the source pass for the
 * directly mapped wall cell, or -1 when that mapping is not admitted. */
int dm2_v1_viewport_draw_dungeon_tiles_pass_for_square(int view_square);
int dm2_v1_viewport_wall_graphic_index_for_square(int view_square);
int dm2_v1_viewport_wall_graphic_index_for_graphicsset(int graphicsset_index,
                                                        int view_square);
int dm2_v1_viewport_wall_graphic_address(int gdat_index,
                                         int *out_graphicsset_index,
                                         int *out_field);
int dm2_v1_viewport_door_frame_field_for_square(int view_square);
int dm2_v1_viewport_door_frame_graphic_index_for_square(int view_square);
int dm2_v1_viewport_door_frame_graphic_index_for_graphicsset(
    int graphicsset_index, int view_square);
/* skproject SKWIN/skval1.h binds the two side-frame IMG3 fields to the
 * viewport cell, not to Firestaff's legacy single-frame convenience slot.
 * `side` is 0=left, 1=right.  The returned rect is the source
 * QUERY_CREATURE_BLIT_RECTI(cell, 10/14, 0) key. */
int dm2_v1_viewport_door_side_frame_source(int view_square, int side,
                                           int *out_graphicsset_field,
                                           int *out_rect_number,
                                           int *out_mirror_flip,
                                           int *out_offset_x,
                                           int *out_offset_y);
/* DRAW_DOOR_FRAMES switches table1d6ee1 row/column when v1e12d0 is active,
 * while QUERY_CREATURE_BLIT_RECTI retains the original cell's RAW4 rect. */
int dm2_v1_viewport_door_side_frame_source_for_movement(
    int view_square, int side, int movement_active,
    int *out_graphicsset_field, int *out_rect_number, int *out_mirror_flip,
    int *out_offset_x, int *out_offset_y);
int dm2_v1_viewport_door_panel_field_for_square(int view_square);
int dm2_v1_viewport_door_panel_graphic_index_for_square(int view_square);
int dm2_v1_viewport_door_panel_graphic_index_for_record(int view_square,
                                                        int door_gfx_index,
                                                        int opening_dir);
int dm2_v1_viewport_door_ornate_graphic_index(int door_ornate_index,
                                              int view_square);
int dm2_v1_viewport_door_destroyed_mask_graphic_index(int door_gfx_index,
                                                      int view_square);
int dm2_v1_viewport_door_button_field_for_state(int pushed);
int dm2_v1_viewport_door_button_graphic_index_for_state(int pushed);
/* skproject DRAW_DOOR/DRAW_DOOR_FRAMES derive the visible panel percentage from
 * the tile door state.  The viewport layer reconciles an explicit runtime
 * open_pct with the source state table, blocking inconsistent synthetic values. */
int dm2_v1_viewport_door_open_pct_from_state(int door_state,
                                             int explicit_open_pct);
int dm2_v1_viewport_skproject_cell_for_square(int view_square);
int dm2_v1_viewport_door_button_rectno_for_square(int view_square);
int dm2_v1_viewport_door_button_clickable_for_square(int view_square);
int dm2_v1_viewport_wall_button_graphic_index(int wall_gfx_index,
                                              int wall_gfx_field);
int dm2_v1_viewport_creature_graphic_index(int creature_type,
                                           int frame_index);
int dm2_v1_viewport_creature_field_graphic_index(int creature_type,
                                                 int image_field);
int dm2_v1_viewport_item_category_for_db_pool(int db_pool);
int dm2_v1_viewport_item_graphic_index(int item_category,
                                       int item_type,
                                       int frame_index);
/* skproject SkWinCore.cpp::_2405_014a, used by DRAW_ITEM_IN_HAND. The caller
 * supplies the original item's dtWordValue(6) selector. Unsupported modes
 * require record state Firestaff does not yet own and fail closed. */
int dm2_v1_viewport_select_carried_item_image_field(uint16_t selector,
                                                    uint32_t object_index,
                                                    uint32_t game_tick,
                                                    int party_direction,
                                                    uint8_t *out_image_field);
int dm2_v1_viewport_projectile_graphic_index(int projectile_category,
                                             int projectile_type,
                                             int frame_index);
int dm2_v1_viewport_hud_portrait_graphic_index(int portrait_index);
int dm2_v1_viewport_hud_core_graphic_index(int field);
int dm2_v1_viewport_hud_hand_action_graphic_index(int possession_index,
                                                   int left_or_right);
int dm2_v1_viewport_hud_hand_action_graphic_address(
    int gdat_index,
    int *out_possession_index,
    int *out_left_or_right,
    int *out_entry);
int dm2_v1_viewport_hud_action_icon_graphic_index(int icon_index);
int dm2_v1_viewport_scene_material_graphic_index(int graphicsset_index,
                                                  int material_field);
int dm2_v1_viewport_scene_material_graphic_address(int gdat_index,
                                                    int *out_graphicsset_index,
                                                    int *out_material_field);
int dm2_v1_viewport_weather_environment_graphic_index(int graphicsset_index,
                                                       int environment_field);
int dm2_v1_viewport_weather_environment_graphic_address(
    int gdat_index, int *out_graphicsset_index, int *out_environment_field);
int dm2_v1_viewport_teleporter_map_chip_graphic_index(void);
/* skproject LOAD_LOCALLEVEL_DYN/DRAW_MAP_CHIP selects FLOOR_GFX/index/F9
 * from the active map's local list.  Keep the index reversible so boot can
 * prove the exact source asset before M11 accepts its material plan. */
int dm2_v1_viewport_floor_gfx_map_chip_graphic_index(int floor_gfx_index);
int dm2_v1_viewport_floor_gfx_map_chip_graphic_address(
    int gdat_index, int *out_floor_gfx_index);
int dm2_v1_viewport_wall_gfx_map_chip_graphic_index(int wall_gfx_index);
int dm2_v1_viewport_wall_gfx_map_chip_graphic_address(
    int gdat_index, int *out_wall_gfx_index);
int dm2_v1_viewport_door_map_chip_graphic_index(int door_gfx_index);
int dm2_v1_viewport_door_map_chip_graphic_address(
    int gdat_index, int *out_door_gfx_index);
int dm2_v1_viewport_dialogue_box_graphic_index(void);
int dm2_v1_viewport_map_chip_frame_width(int src_w, int src_h);
int dm2_v1_viewport_map_chip_frame_count(int src_w, int src_h);
int dm2_v1_viewport_map_chip_frame_index(int requested_frame,
                                         int frame_count);
int dm2_v1_viewport_projectile_frame_for_direction(int requested_frame,
                                                   int projectile_direction,
                                                   int party_direction,
                                                   int frame_count);
int dm2_v1_viewport_projectile_frame_for_map_chip(int requested_frame,
                                                  int projectile_direction,
                                                  int object_direction,
                                                  int party_direction,
                                                  int frame_count,
                                                  int frame_class);
int dm2_v1_viewport_projectile_flip_for_direction(int projectile_direction,
                                                  int party_direction);
int dm2_v1_viewport_map_chip_flip_for_object_direction(int object_direction,
                                                       int party_direction);
int dm2_v1_viewport_cloud_frame_for_tick(int tick_count,
                                         int frame_count);
int dm2_v1_viewport_cloud_flip_for_seed(uint32_t *seed);
int dm2_v1_viewport_creature_frame_for_direction(int requested_frame,
                                                 int creature_direction,
                                                 int party_direction,
                                                 int frame_count);

typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM2_V1_ViewportRect;

typedef struct {
    const uint8_t *pixels;
    int width;
    int height;
    int stride;
    DM2_ImageFormat format;
} DM2_V1_BootViewportSurfaceView;

typedef struct {
    int valid;
    int viewport_owned;
    uint32_t map_load_token;
    uint8_t gdat_category;
    uint8_t floor_ornate_source_index;
    int animated_frame_route;
} DM2_V1_FloorGfxViewportOwnershipReceipt;

#define DM2_V1_WALL_PANEL_RENDER_MAX DM2_SQ_COUNT

typedef struct {
    int render_step;
    int view_square;
    int skproject_cell;
    int gdat_index;
    DM2_V1_ViewportRect src_rect;
    DM2_V1_ViewportRect dst_rect;
    uint8_t fallback_color;
} DM2_V1_WallPanelRender;

typedef struct {
    DM2_V1_WallPanelRender panels[DM2_V1_WALL_PANEL_RENDER_MAX];
    int panel_count;
    /* c_gui_vp's cell order is view-relative.  The plan is rebuilt for the
     * current direction after G1 has classified the actual visible cells. */
    int party_direction;
    uint16_t selected_square_mask;
} DM2_V1_WallPanelRenderPlan;

/* skproject SkWinCore map-load scene state. It is a typed source record, not
 * a decoded material bundle: category/set/field addresses and control words
 * are the only data permitted to cross into a viewport plan. */
typedef struct {
    int valid;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint8_t graphicsset;
    uint16_t scene_colorkey;
    uint16_t scene_flags;
    uint16_t ambient_light;
    uint16_t highest_light_level;
    uint16_t ambient_darkness;
    uint8_t material_category;
    uint8_t floor_field;
    uint8_t ceiling_field;
    uint8_t door_frame_front_d1_field;
    uint8_t door_frame_d1c_field;
    uint8_t door_frame_d2c_field;
} DM2_V1_GraphicsSetStaticSceneReceipt;

/* Typed scene-material ownership that may reach the dungeon plan before any
 * image decode. It deliberately has no bitmap, GDAT field, or blit rect. */
typedef struct {
    int valid;
    int static_scene_control_owned;
    int static_light_control_owned;
    int static_ambient_light_control_owned;
    int static_ambient_darkness_control_owned;
    int static_scene_flags_control_owned;
    int static_scene_colorkey_control_owned;
    int static_scene_floor_material_owned;
    int static_scene_ceiling_material_owned;
    int static_scene_door_frame_material_owned;
    int static_scene_door_frame_d1c_material_owned;
    int static_scene_door_frame_d2c_material_owned;
    uint32_t map_load_token;
    uint8_t gdat_category;
    uint8_t floor_ornate_source_index;
    int animated_frame_route;
    uint32_t scene_control_hash;
    uint16_t scene_colorkey;
    uint16_t scene_flags;
    int outdoor_scene;
    uint8_t scene_floor_material_category;
    uint8_t scene_floor_material_graphicsset;
    uint8_t scene_floor_material_field;
    uint8_t scene_ceiling_material_category;
    uint8_t scene_ceiling_material_graphicsset;
    uint8_t scene_ceiling_material_field;
    uint8_t scene_door_frame_material_category;
    uint8_t scene_door_frame_material_graphicsset;
    uint8_t scene_door_frame_material_field;
    uint8_t scene_door_frame_d1c_material_category;
    uint8_t scene_door_frame_d1c_material_graphicsset;
    uint8_t scene_door_frame_d1c_material_field;
    uint8_t scene_door_frame_d2c_material_category;
    uint8_t scene_door_frame_d2c_material_graphicsset;
    uint8_t scene_door_frame_d2c_material_field;
    uint16_t ambient_light;
    uint16_t highest_light_level;
    uint16_t ambient_darkness;
} DM2_V1_ViewportFloorGfxRenderPlanReceipt;

#define DM2_V1_DOOR_RENDER_MAX DM2_SQ_COUNT

typedef struct {
    int view_square;
    int skproject_cell;
    /* table1d7029 pass that dispatched this source door, or -1 for a
     * compatibility-only caller. */
    int source_pass;
    int door_record_type;
    int door_gfx_index;
    int door_gfx_admitted;
    int door_opening_dir;
    int ornament_index;
    int door_ornate_gfx_index;
    int door_button;
    int door_button_state;
    int panel_gdat_index;
    int ornate_gdat_index;
    int destroyed_mask_gdat_index;
    int frame_gdat_index;
    int side_frame_gdat_index[2];
    int side_frame_graphicsset_field[2];
    int side_frame_rect_number[2];
    int side_frame_mirror_flip[2];
    int side_frame_offset_x[2];
    int side_frame_offset_y[2];
    /* skproject DRAW_DOOR_FRAMES reads glbMapGraphicsSet, not a global
     * default graphics set.  This value is carried into the M11 receipt. */
    int graphicsset_index;
    int button_gdat_index;
    int button_source_kind; /* 1=default door button, 2=wall-gfx button */
    int wall_button_index;
    int wall_button_field;
    int wall_button_x;
    int wall_button_y;
    uint16_t wall_button_object_id;
    DM2_V1_ViewportRect panel_rect;
    DM2_V1_ViewportRect panel_visible_rect;
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect button_rect;
    uint8_t door_open_pct;
    uint8_t fallback_color;
    uint8_t door_state;
} DM2_V1_DoorRender;

typedef struct DM2_V1_DoorRenderPlan {
    DM2_V1_DoorRender doors[DM2_V1_DOOR_RENDER_MAX];
    int door_count;
} DM2_V1_DoorRenderPlan;

#define DM2_V1_WALL_ORNAMENT_RENDER_MAX DM2_SQ_COUNT

typedef struct {
    int view_square;
    int gdat_index;
    DM2_V1_ViewportRect dst_rect;
    uint32_t material_hash;
} DM2_V1_WallOrnamentRender;

typedef struct DM2_V1_WallOrnamentRenderPlan {
    DM2_V1_WallOrnamentRender ornaments[DM2_V1_WALL_ORNAMENT_RENDER_MAX];
    int ornament_count;
    int valid;
} DM2_V1_WallOrnamentRenderPlan;

typedef struct {
    int gdat_index;
    DM2_V1_ViewportRect src_rect;
    DM2_V1_ViewportRect dst_rect;
    int src_stride;
    int transparent_color;
} DM2_V1_DoorAssetBlit;

/* Source-material gate observed by the production door-frame path. The
 * decoded-format bit is set only after the viewport provider has returned a
 * bitmap; dm2_v1_boot_viewport_asset_fetch admits only proven U4 formats. */
typedef struct {
    int valid;
    int original_material_required;
    int active_graphicsset_frame;
    int decoded_format_gate_ready;
    int scene_record_ready;
    int palette_ready;
    int colorkey_ready;
    int accepted;
    int gdat_index;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint16_t scene_colorkey;
} DM2_V1_OriginalMaterialGateReceipt;

/* A decoded original door frame may publish this borrowed payload view only
 * after the material gate has accepted it. It never owns or creates pixels. */
typedef struct {
    int valid;
    uint8_t gdat_category;
    uint8_t graphicsset;
    uint8_t field;
    int gdat_index;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    uint16_t scene_colorkey;
    const uint8_t *decoded_pixels;
    const uint8_t *palette16;
    int width;
    int height;
    int stride;
    DM2_ImageFormat format;
} DM2_V1_OriginalDoorSurfaceRequest;

/* Renderer binding for a source-owned U4 door frame. The rects describe the
 * existing payload view and destination only; this contract never rasterizes
 * or allocates a replacement surface. */
typedef struct {
    int valid;
    DM2_V1_OriginalDoorSurfaceRequest request;
    DM2_V1_DoorAssetBlit blit;
    int palette_stride;
    uint8_t colorkey_palette_index;
} DM2_V1_OriginalDoorSurfaceBinding;

/* skproject DRAW_DOOR_TILE/DRAW_DOOR_FRAMES selects one center cell before
 * it draws a moving panel and its GRAPHICSSET frame. This is the renderer's
 * borrowed request for that specific opening state; it never decodes or
 * rasterizes another surface. */
typedef struct {
    int valid;
    int view_square;
    int skproject_cell;
    uint8_t door_state;
    uint8_t door_open_pct;
    uint8_t door_opening_dir;
    DM2_V1_ViewportRect opening_visible_rect;
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
    DM2_V1_OriginalDoorSurfaceBinding material;
} DM2_V1_OriginalDoorOpeningFrameRequest;

/* Final viewport presentation boundary for a source-owned door frame. It
 * borrows the verified U4 payload and palette view; it never owns pixels or
 * provides an alternate bitmap when the source route is incomplete. */
typedef struct {
    int valid;
    DM2_V1_OriginalDoorOpeningFrameRequest opening_frame;
    const uint8_t *u4_pixels;
    const uint8_t *palette16;
    int palette_stride;
    uint8_t scene_colorkey;
    uint8_t colorkey_palette_index;
    DM2_ImageFormat format;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_ViewportDoorPresentationCommand;

/* skproject DM2_LOAD_GDAT_INTERFACE_00_02 selects this INTERFACE_GENERAL
 * material before HUD drawing. The request borrows a proven GDAT payload and
 * its active palette; it cannot manufacture a chrome surface. */
typedef struct {
    int valid;
    int gdat_index;
    uint8_t gdat_category;
    uint8_t gdat_subcategory;
    uint8_t gdat_entry;
    uint8_t field;
    const uint8_t *indexed_pixels;
    const uint8_t *palette16;
    uint32_t palette_hash;
    int palette_entry_count;
    int width;
    int height;
    int stride;
    int transparent_color;
    uint8_t colorkey_palette_index;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_ViewportHudMaterialRequest;

/* skproject SkWinCore::DRAW_DUNGEON_GRAPHIC selects the active
 * GRAPHICSSET/FLOOR entry before the indoor floor pass. This command borrows
 * the verified indexed payload and palette binding; it cannot synthesize a
 * floor surface when the map-owned material is absent. */
typedef struct {
    int valid;
    uint8_t gdat_category;
    uint8_t graphicsset;
    uint8_t field;
    int gdat_index;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    const uint8_t *indexed_pixels;
    const uint8_t *palette16;
    int palette_stride;
    int width;
    int height;
    int stride;
    int transparent_color;
    uint8_t colorkey_palette_index;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_ViewportDungeonMaterialCommand;

/* skproject CHECK_RECOMPUTE_LIGHT reads GRAPHICSSET/AMBIANT_DARKNESS after
 * dungeon geometry and before interface presentation. It is a typed control
 * handoff only: no Firestaff darkness overlay may stand in for this route. */
typedef struct {
    int valid;
    uint8_t gdat_category;
    uint8_t graphicsset;
    uint8_t field;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    uint16_t scene_colorkey;
    uint8_t colorkey_palette_index;
    uint16_t ambient_darkness;
    uint8_t light_floor;
    uint8_t walk_path_depth;
    int light_check_enabled;
} DM2_V1_ViewportSceneControlCommand;

/* skproject QUERY_DUNGEON_MAP_CHIP_PICT/DRAW_CHIP_OF_MAGIC_MAP consumes a
 * creature map-chip after scene control and before the interface. This is a
 * borrowed decoded-payload command, never a generated creature surface. */
typedef struct {
    int valid;
    uint8_t gdat_category;
    uint8_t creature_type;
    uint8_t field;
    int gdat_index;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    const uint8_t *indexed_pixels;
    const uint8_t *palette16;
    int palette_stride;
    int width;
    int height;
    int stride;
    int transparent_color;
    uint8_t colorkey_palette_index;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_ViewportCreatureMaterialCommand;

/* skproject DRAW_ITEM/DRAW_CHIP_OF_MAGIC_MAP consumes floor-object map chips
 * after creature sprites and before the interface. This borrows the verified
 * indexed payload; it never invents an item surface when GDAT is unavailable. */
typedef struct {
    int valid;
    uint8_t gdat_category;
    uint8_t item_type;
    uint8_t field;
    int gdat_index;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    const uint8_t *indexed_pixels;
    const uint8_t *palette16;
    int palette_stride;
    int width;
    int height;
    int stride;
    int transparent_color;
    uint8_t colorkey_palette_index;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_ViewportItemMaterialCommand;

/* Stage-11 presentation command for the verified INTERFACE_GENERAL top bar.
 * It retains source indexed bytes, palette and colorkey as borrowed views. */
typedef struct {
    int valid;
    DM2_V1_ViewportHudMaterialRequest material;
    const uint8_t *indexed_pixels;
    const uint8_t *palette16;
    int transparent_color;
    DM2_V1_ViewportRect source_rect;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_ViewportHudPresentationCommand;

/* skproject DRAW_HAND_ACTION_ICONS: entry=(possession<<1)+side+2 and
 * rectno=(possession==1 ? 0x46 : 0x4a)+((partypos+4-partydir)&3). */
typedef struct {
    int valid;
    uint8_t player_index;
    uint8_t possession_index;
    uint8_t left_or_right;
    uint8_t player_position;
    uint8_t party_direction;
    uint8_t gdat_category;
    uint8_t gdat_subcategory;
    uint8_t gdat_entry;
    uint8_t rectno;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    DM2_V1_ViewportRect destination_rect;
} DM2_V1_HudHandActionSource;

/* Bounded indoor frame composition around one source-owned door command.
 * skproject binds GRAPHICSSET light and dtPalette16 before DRAW_DOOR_FRAMES;
 * this receipt records that order without inventing a HUD or dungeon image. */
typedef struct {
    int valid;
    int indoor_viewport;
    int scene_record_owned;
    int scene_light_owned;
    int palette_owned;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t palette_hash;
    uint8_t light_floor;
    uint8_t light_search_depth;
    int door_presentation_stage;
    int dungeon_ceiling_presentation_stage;
    int dungeon_floor_presentation_stage;
    int dungeon_wall_presentation_stage;
    int wall_ornament_presentation_stage;
    int scene_control_presentation_stage;
    int creature_presentation_stage;
    int item_presentation_stage;
    int hud_presentation_stage;
    int door_command_consumed;
    int dungeon_ceiling_command_consumed;
    int dungeon_floor_command_consumed;
    int dungeon_wall_command_consumed;
    int wall_ornament_command_consumed;
    uint16_t dungeon_wall_material_required_mask;
    uint16_t dungeon_wall_material_consumed_mask;
    uint16_t wall_ornament_material_required_mask;
    uint16_t wall_ornament_material_consumed_mask;
    int scene_control_command_consumed;
    int creature_command_consumed;
    int item_command_consumed;
    int hud_top_bar_material_consumed;
    int hud_top_bar_command_consumed;
    int hud_status_panel_material_consumed;
    int hud_status_panel_command_consumed;
    int hud_top_bar_order;
    int hud_status_panel_order;
    int hud_hand_action_order;
    DM2_V1_ViewportDoorPresentationCommand door_command;
    DM2_V1_ViewportDungeonMaterialCommand dungeon_ceiling_command;
    DM2_V1_ViewportDungeonMaterialCommand dungeon_floor_command;
    DM2_V1_ViewportDungeonMaterialCommand dungeon_wall_command;
    DM2_V1_ViewportDungeonMaterialCommand wall_ornament_command;
    DM2_V1_ViewportSceneControlCommand scene_control_command;
    DM2_V1_ViewportCreatureMaterialCommand creature_command;
    DM2_V1_ViewportItemMaterialCommand item_command;
    DM2_V1_ViewportHudMaterialRequest hud_top_bar_request;
    DM2_V1_ViewportHudPresentationCommand hud_top_bar_command;
    DM2_V1_ViewportHudMaterialRequest hud_status_panel_request;
    DM2_V1_ViewportHudPresentationCommand hud_status_panel_command;
    int hud_hand_action_command_consumed;
    DM2_V1_ViewportHudMaterialRequest hud_hand_action_request;
    DM2_V1_ViewportHudPresentationCommand hud_hand_action_command;
} DM2_V1_ViewportFrameCompositionReceipt;

/* M11 consumes one atomic DM2 decision after it has presented the viewport.
 * skproject sequences DRAW_DUNGEON_GRAPHIC, DRAW_DOOR_FRAMES,
 * DRAW_CHIP_OF_MAGIC_MAP and the INTERFACE_GENERAL HUD through one frame;
 * this receipt prevents the host from re-deriving any individual GDAT gate.
 * It owns metadata only and borrows no framebuffer or synthetic surface. */
typedef struct {
    int valid;
    int m11_consume_frame;
    int source_materials_required;
    uint32_t map_load_token;
    uint32_t scene_control_hash;
    uint32_t scene_light_hash;
    uint16_t scene_ambient_light;
    uint32_t c_light_receipt_hash;
    uint32_t c_light_source_state_hash;
    uint8_t c_light_level;
    int weather_graphicsset_bound;
    uint8_t weather_graphicsset;
    uint32_t weather_source_receipt_hash;
    uint32_t weather_destination_receipt_hash;
    uint32_t presentation_state_hash;
    uint32_t floor_material_hash;
    uint32_t ceiling_material_hash;
    uint32_t wall_material_plan_hash;
    int wall_material_plan_command_count;
    int door_material_plan_required;
    uint32_t door_material_plan_hash;
    int door_material_plan_command_count;
    int door_material_plan_consumed;
    int hud_material_plan_required;
    uint32_t hud_material_plan_hash;
    uint32_t hud_scene_control_hash;
    int hud_material_plan_command_count;
    int hud_material_plan_consumed;
    int creature_material_plan_required;
    uint32_t creature_material_plan_hash;
    int creature_material_plan_command_count;
    int creature_material_plan_consumed;
    int projectile_material_plan_required;
    uint32_t projectile_material_plan_hash;
    int projectile_material_plan_command_count;
    int projectile_material_plan_consumed;
    int item_material_plan_required;
    uint32_t item_material_plan_hash;
    uint32_t item_scene_control_hash;
    int item_material_plan_command_count;
    int item_material_plan_consumed;
    int teleporter_material_plan_required;
    uint32_t teleporter_material_plan_hash;
    int teleporter_material_plan_consumed;
    int floor_gfx_map_chip_material_plan_required;
    uint32_t floor_gfx_map_chip_material_plan_hash;
    int floor_gfx_map_chip_material_plan_consumed;
    int wall_gfx_map_chip_material_plan_required;
    uint32_t wall_gfx_map_chip_material_plan_hash;
    int wall_gfx_map_chip_material_plan_consumed;
    int door_map_chip_material_plan_required;
    uint32_t door_map_chip_material_plan_hash;
    int door_map_chip_material_plan_consumed;
    int weather_material_plan_required;
    uint32_t weather_material_plan_hash;
    int weather_material_plan_command_count;
    int weather_material_plan_consumed;
    uint32_t palette_hash;
    uint32_t interface_action_palette_hash;
    int interface_action_palette_consumed;
    /* INTERFACE_GENERAL dt07/0x0A Rect14 placement table carried into M11. */
    int interface_rect14_required;
    int interface_rect14_consumed;
    uint32_t interface_rect14_table_hash;
    uint32_t interface_rect14_placement_hash;
    uint32_t interface_rect14_row_count;
    uint8_t floor_ceiling_material_required_mask;
    uint8_t floor_ceiling_material_consumed_mask;
    int floor_ceiling_materials_complete;
    int gdat_wall_material_plan_consumed;
    DM2_V1_ViewportFrameCompositionReceipt composition;
} DM2_V1_ViewportM11FrameReceipt;

typedef struct {
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect fill_rect;
    uint8_t fill_color;
    int gdat_index;
} DM2_V1_HudIconRender;

typedef struct {
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect fill_rect;
    uint8_t fill_color;
    int occupied;
    int leader;
    uint8_t hp_pct;
    uint8_t stamina_pct;
    uint8_t mana_pct;
    /* SKProject glbChampionColor[player], initialized by INIT. */
    uint8_t stat_bar_color;
    int stat_bar_color_source_bound;
    uint8_t portrait_index;
    int portrait_type_source_bound;
    int state_source_bound;
    uint8_t portrait_fill_color;
    char name[DM2_V1_HUD_CHAMPION_NAME_MAX + 1];
    DM2_V1_ViewportRect leader_mark_rect;
    DM2_V1_ViewportRect portrait_rect;
    DM2_V1_ViewportRect name_marker_rect;
    DM2_V1_ViewportRect hp_bar_rect;
    DM2_V1_ViewportRect hp_fill_rect;
    DM2_V1_ViewportRect stamina_bar_rect;
    DM2_V1_ViewportRect stamina_fill_rect;
    DM2_V1_ViewportRect mana_bar_rect;
    DM2_V1_ViewportRect mana_fill_rect;
} DM2_V1_HudChampionSlotRender;

typedef struct {
    int occupied;
    int leader;
    uint8_t portrait_index;
    int portrait_type_source_bound;
    int state_source_bound;
    uint8_t hp_pct;
    uint8_t stamina_pct;
    uint8_t mana_pct;
    /* Source-owned default from SkWinCore::INIT, not a resource-specific
     * Firestaff color choice. */
    uint8_t stat_bar_color;
    int stat_bar_color_source_bound;
    char name[DM2_V1_HUD_CHAMPION_NAME_MAX + 1];
} DM2_V1_HudChampionState;

typedef struct DM2_V1_HudPartyState {
    int champion_count;
    int leader_index;
    DM2_V1_HudChampionState champions[DM2_V1_HUD_CHAMPION_SLOT_COUNT];
} DM2_V1_HudPartyState;

typedef struct {
    int outdoor;
    DM2_V1_ViewportRect top_bar_rect;
    DM2_V1_ViewportRect top_divider_rect;
    DM2_V1_ViewportRect action_strip_rect;
    DM2_V1_ViewportRect action_divider_rect;
    DM2_V1_ViewportRect gold_box_rect;
    DM2_V1_ViewportRect gold_coin_rect;
    DM2_V1_ViewportRect gold_label_rect;
    int top_bar_gdat_index;
    int action_strip_gdat_index;
    int portrait_panel_gdat_index;
    int gold_box_gdat_index;
    DM2_V1_HudIconRender action_icons[DM2_V1_HUD_ACTION_ICON_COUNT];
    int action_icon_count;
    DM2_V1_ViewportRect portrait_separator_dark_rect;
    DM2_V1_ViewportRect portrait_separator_light_rect;
    DM2_V1_ViewportRect portrait_panel_rect;
    DM2_V1_HudChampionSlotRender champion_slots[
        DM2_V1_HUD_CHAMPION_SLOT_COUNT];
    int champion_slot_count;
} DM2_V1_HudChromeRenderPlan;

int dm2_v1_viewport_build_hud_chrome_plan(
    int is_outdoor,
    DM2_V1_HudChromeRenderPlan *out_plan);
int dm2_v1_viewport_build_hud_chrome_plan_for_party(
    int is_outdoor,
    const DM2_V1_HudPartyState *party,
    DM2_V1_HudChromeRenderPlan *out_plan);

typedef struct {
    int visible;
    int depth;
    int screen_x;
    int screen_y;
} DM2_V1_ViewportSpritePlacement;

typedef struct {
    int valid;
    uint8_t base_5x5;
    int8_t lateral_offset;
    uint16_t cell_pos;
    uint16_t blit_rect_id[4];
    uint8_t image_field[4];
    uint8_t stretch_source[4];
    uint16_t stretched_size[4];
    uint8_t flags[4];
} DM2_V1_InterfaceRect14Placement;

int dm2_v1_viewport_project_map_to_sprite(int map_x,
                                          int map_y,
                                          int party_dir,
                                          int party_x,
                                          int party_y,
                                          DM2_V1_ViewportSpritePlacement *out);
/* Maps the visible 4x3 grid to SKProject c_gui_vp.cpp::DM2_DRAW_STATIC_OBJECT
 * cell/pass pairs (table1d7029).  Cell 0 (party square) has no pass and is
 * not promoted; the side/deep cells it yields are admitted by
 * dm2_v1_viewport_static_object_source_plan through the source-owned
 * y-distance, stretch, display-order and visibility-mask tables. */
int dm2_v1_viewport_static_object_cell_for_map(int map_x,
                                               int map_y,
                                               int party_dir,
                                               int party_x,
                                               int party_y,
                                               int *out_cell,
                                               int *out_pass);

/* Exact SKWIN/SkWinCore.cpp DRAW_ITEM selection for the bounded floor-object
 * family on cells 1..15 (cell 0 has no table1d7029 pass; D4 cells are
 * rejected by DRAW_PUT_DOWN_ITEM's distance guard).  This describes source
 * selection only: callers must still provide the expanded clipping rectangle
 * and dtImageOffset receipt before pixels may be drawn. */
typedef struct {
    int source_cell;
    int source_pass;
    int position_5x5;
    int clip_rect_id;
    int y_distance;
    int stretch_factor64;
    int image_field;
    int flip_mirror;
    int slot_x_offset;
    int slot_y_offset;
    int object_direction;
    /* SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT filters by a 5x5 visibility mask
     * before calling DRAW_PUT_DOWN_ITEM.  The mask is source-owned: the runtime
     * binds it from the record scan (SkWinCore.cpp:45360-45370) through
     * dm2_v1_viewport_static_object_visibility_bit.  A zero mask or a mask
     * that does not contain this object's position keeps the plan
     * evidence-only and fail-closed. */
    uint32_t visibility_mask_5x5;
    /* Record-list ordinal within the source square's object chain.  Zero is
     * unavailable and blocks M11 delivery until the runtime supplies it. */
    uint16_t record_list_ordinal;
    /* INTERFACE_GENERAL dt07/0x0A Rect14 row that governs this static object's
     * placement, scale and image-field selection.  When a matching row exists,
     * the render plan is gated by that row instead of by synthetic geometry. */
    int rect14_applied;
    uint8_t rect14_image_field;
    int rect14_scale64;
    int rect14_lateral_offset;
    int rect14_flip_mirror;
    uint32_t rect14_row_hash;
    uint32_t rect14_placement_hash;
} DM2_V1_StaticObjectSourcePlan;

/* object_direction is the record's absolute ObjectID::Dir(); view_dir is the
 * absolute party viewing direction (skproject _4976_5aa0).  The source DRAW_ITEM
 * anchor is QUERY_OBJECT_5x5_POS(rl, _4976_5aa0), i.e. the direction anchor
 * rotated into view space, so position_5x5/clip_rect_id/flip are derived from
 * (object_direction - view_dir) & 3. */
int dm2_v1_viewport_static_object_source_plan(int source_cell,
                                              int source_pass,
                                              int item_category,
                                              int object_direction,
                                              int container_open,
                                              int draw_slot,
                                              int view_dir,
                                              uint16_t record_list_ordinal,
                                              uint32_t visibility_mask_5x5,
                                              DM2_V1_StaticObjectSourcePlan *out);

/* SKWIN/SkWinCore.cpp QUERY_OBJECT_5x5_POS for floor objects (dbWeapon ..
 * dbMiscellaneous_item): ROTATE_5x5_POS(_4976_4a04[dir], view_dir).
 * Returns -1 for out-of-range input. */
int dm2_v1_viewport_object_5x5_pos(int object_direction, int view_dir);

/* SKWIN/SkWinCore.cpp line 45370: the per-cell 5x5 visibility mask bit that
 * DRAW_STATIC_OBJECT tests ((*_4976_5be2)[cellPos]) before calling
 * DRAW_PUT_DOWN_ITEM.  Returns 0 for out-of-range input. */
uint32_t dm2_v1_viewport_static_object_visibility_bit(int object_direction,
                                                      int view_dir);

/* SKWIN/SkWinCore.cpp DIR_FROM_5x5_POS: corner 5x5 positions map to
 * 0=N/1=E/2=S/3=W, centre 12 maps to 4, everything else to -1. */
int dm2_v1_viewport_dir_from_5x5_pos(int pos5x5);

/* SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT lines 47160-47174: the 5x5 display
 * order is tlbDisplayOrderLeft/Center/Right selected by the sign of
 * glbTabXAxisDistance[cell_pos]; cell 0 iterates only the first 15 entries.
 * out_order receives up to 25 source table entries; returns the entry count
 * (0 for an unknown cell). */
int dm2_v1_viewport_static_object_display_order(int cell_pos,
                                                uint8_t out_order[25]);

/* SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT lines 47174-47190: the 5x5
 * positions, in source display order, whose visibility-mask bit is set; each
 * fires DRAW_PUT_DOWN_ITEM for the cell.  Returns the position count. */
int dm2_v1_viewport_static_object_draw_positions(
    int cell_pos, uint32_t visibility_mask_5x5, uint8_t out_positions[25]);

/* SKWIN/SkWinCore.cpp QUERY_CREATURE_5x5_POS: a creature whose info slot is
 * 0xff occupies the centre (12); otherwise its 5x5 anchor rotates by
 * (party_dir - creature_dir) & 3 through ROTATE_5x5_POS.  Returns -1 for an
 * invalid anchor. */
int dm2_v1_viewport_creature_occupancy_5x5(int anchor5x5,
                                           int creature_dir,
                                           int party_dir);

/* SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT lines 47179-47185 and SkGlobal.cpp
 * _4976_43f5/_4976_4415: the _4976_5aa4 occupancy grid coordinate for a 5x5
 * position inside a viewport cell.  Returns 1 when the coordinate is bound,
 * 0 for an unknown cell or position. */
int dm2_v1_viewport_occupancy_grid_coords(int cell_pos,
                                          int pos5x5,
                                          int *out_x,
                                          int *out_y);

/* Index of a 5x5 position in the source tlbDisplayOrder* walk for the cell
 * (DRAW_STATIC_OBJECT draws the creature whose _4976_5aa4 slot matches at
 * that index).  Returns -1 when the position is not iterated. */
int dm2_v1_viewport_static_object_display_index(int cell_pos,
                                                int pos5x5);

/* SKWIN/SkWinCore.cpp DRAW_FLYING_ITEM lines 47010-47017: the missile scale
 * is _4976_41a9[(y_distance << 1) - (dir_from_5x5 >> 1)]; a negative index
 * blocks the draw.  Returns the factor64 or -1 when the source blocks. */
int dm2_v1_viewport_flying_item_scale64(int y_distance,
                                        int dir_from_5x5);

/* SKWIN/SkWinCore.cpp DRAW_FLYING_ITEM lines 47024-47096: image field
 * selection (8/9/10/12) and mirror bits from the _48ae_011a frame class,
 * the missile timer direction parity, the tile parity, the cell x distance
 * and the 5x5 direction.  Returns the field (8, 9, 10 or 12), or -1 when
 * the class has no source field.  out_flip may be NULL; it receives the
 * unmasked source mirror bits (si, masked by bp06 by the caller). */
int dm2_v1_viewport_flying_item_image_field(int frame_class,
                                            int timer_direction,
                                            int view_dir,
                                            int tile_x,
                                            int tile_y,
                                            int x_distance,
                                            int dir_from_5x5,
                                            int cls1_is_spell,
                                            int *out_flip);
int dm2_v1_viewport_possession_slot_placement(
    const DM2_V1_ViewportSpritePlacement *base,
    int possession_slot,
    DM2_V1_ViewportSpritePlacement *out);
int dm2_v1_viewport_calc_stretched_size(int value, int factor64);
int dm2_v1_viewport_rotate_5x5_pos(int pos5x5, int dir);
int dm2_v1_viewport_creature_blit_rect_id(int cell_pos,
                                          int pos5x5,
                                          int dir);
int dm2_v1_viewport_interface_rect14_placement(
    const uint8_t row14[14],
    int cell_pos,
    int distance_stretch_factor64,
    DM2_V1_InterfaceRect14Placement *out);

/* Bind a real INTERFACE_GENERAL dt07/0x0A Rect14 row to a static-object source
 * plan.  When the table is present and a row matches the source cell, 5x5
 * anchor and view-relative direction, the plan carries the original image
 * field, scale, flip and placement hashes instead of synthetic geometry.
 * Source: SKWIN/SkWinCore.cpp QUERY_CREATURE_BLIT_RECTI and
 * SKULLWIN/c_gui_vp.cpp DM2_DRAW_PUT_DOWN_ITEM. */
int dm2_v1_viewport_enrich_static_object_source_plan_with_rect14(
    const uint8_t *rows,
    uint32_t row_count,
    uint32_t table_hash,
    int object_direction,
    int party_dir,
    DM2_V1_StaticObjectSourcePlan *plan);

int dm2_v1_viewport_door_panel_rect_for_square(int view_square,
                                               DM2_V1_ViewportRect *out_rect);
int dm2_v1_viewport_door_button_rect_for_square(int view_square,
                                                DM2_V1_ViewportRect *out_rect);

/* ── View square state ──────────────────────────────────────────── */
typedef struct {
    uint8_t  square_type;     /* 5-bit tile type */
    uint8_t  flags;           /* DM2_SquareFlags */
    uint8_t  wall_gfx_index;  /* GDAT wall graphic index */
    uint8_t  wall_ornate_gfx_index; /* GDAT WALL_GFX ornament index (0=none) */
    uint8_t  floor_gfx_index; /* GDAT floor graphic index */
    uint8_t  door_gfx_index;  /* GDAT door graphic index */
    uint8_t  door_gfx_admitted; /* map UseDoor0/UseDoor1 source gate */
    uint8_t  ornament_index;   /* DB0 Door::OrnateIndex() ordinal */
    uint8_t  door_ornate_gfx_index; /* map-local DOOR_GFX entry, or zero */
    uint8_t  creature_type;   /* creature type or 0 */
    uint8_t  item_type;       /* item type or 0 */
    uint8_t  light_level;     /* 0–15 per-tile illumination */
    uint8_t  wall_parity;     /* 0=normal, 1=flipped (odd parity) */
    uint8_t  door_open_pct;    /* 0–100 door open percentage */
    uint8_t  door_button;      /* 0=no default button, 1=draw default button */
    uint8_t  door_button_state;/* skproject door->ButtonState(): 0=released, 1=pushed */
    uint8_t  door_wall_button; /* 1=draw custom wall-gfx button when no default button */
    uint8_t  door_wall_button_index; /* skproject tblCellTilesRoom[cell].w6[2] low byte */
    uint8_t  door_wall_button_field; /* skproject tblCellTilesRoom[cell].w6[2] high byte + 1 */
    uint8_t  door_wall_button_state; /* 0=released, 1=pushed; selects field+state */
    uint8_t  door_direct_g1_root; /* 1=DB0 root metadata came from G1 map */
    uint8_t  door_record_type; /* skproject door->DoorType() */
    uint8_t  door_opening_dir; /* skproject door->OpeningDir() */
    uint8_t  door_state;       /* low 3 bits of tile door state */
    int16_t  door_wall_button_x;
    int16_t  door_wall_button_y;
    uint16_t door_wall_button_object_id;
    int16_t  sprite_depth;    /* depth sort key */
} DM2_ViewSquare;

#define DM2_MAX_CREATURES_PER_SQ  4
#define DM2_MAX_ITEMS_PER_SQ      8
#define DM2_MAX_CREATURE_POSSESSION_ITEMS 8
#define DM2_MAX_PRESENTED_ITEM_MATERIALS \
    (DM2_MAX_ITEMS_PER_SQ + DM2_MAX_CREATURE_POSSESSION_ITEMS + 1)
#define DM2_MAX_PROJECTILES       16

#define DM2_V1_PROJECTILE_RENDER_MISSILE 0
#define DM2_V1_PROJECTILE_RENDER_CLOUD   1

#define DM2_V1_PROJECTILE_FRAME_CLASS_MISSING      0xffu
#define DM2_V1_PROJECTILE_FRAME_CLASS_FRONT_ONLY   0u
#define DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL  1u
#define DM2_V1_PROJECTILE_FRAME_CLASS_FLAT         2u
#define DM2_V1_PROJECTILE_FRAME_CLASS_BASE_FRONT   3u

/* ── Sprite / creature record ───────────────────────────────────── */
typedef struct {
    uint8_t  creature_type;   /* GDAT creature index */
    uint8_t  frame_index;      /* current animation frame */
    uint8_t  anim_phase;       /* animation phase (walk/attack/idle/death) */
    int16_t  depth;            /* depth sort key */
    int16_t  screen_x;         /* viewport X position */
    int16_t  screen_y;         /* viewport Y position */
    uint8_t  health_pct;       /* runtime state; no synthetic scene overlay */
    uint8_t  light_radius;     /* light emitted by creature */
    uint8_t  direction;        /* 0=N, 1=E, 2=S, 3=W */
    uint8_t  source_kind;      /* 1=live runtime, 2=G1 DB4 record */
    uint8_t  source_material_proven; /* real GDAT owner admitted */
    uint8_t  gdat_image_field; /* direct CREATURES dtImage field for live route */
    /* 1 = the direct dtImage field was selected through the real FB/FC/FD
     * V5 animation chain (SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST's live
     * non-static route) rather than the DB4 map-chip F9 route. */
    uint8_t  source_v5_field;
    uint32_t source_material_hash;
    uint16_t object_id;        /* G1 DB4 ObjectID; zero for non-dungeon sprites */
    int16_t  map_x;            /* source map coordinate for G1 material ownership */
    int16_t  map_y;
} DM2_CreatureSprite;

/* Real FB/FC/FD V5 material for one G1 creature record: the exact decoded
 * CREATURES/type/dtImage identity resolved through the live animation chain
 * (SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST's live non-static route).
 * Objects without that evidence keep the map-chip gate. */
typedef struct {
    uint16_t object_id;
    int16_t map_x;
    int16_t map_y;
    uint8_t creature_type;
    uint8_t image_field;
    int gdat_index;
    int width;
    int height;
    int stride;
    uint32_t palette_hash;
    uint32_t decoded_hash;
    uint32_t raw_material_hash;
    uint32_t raw_material_receipt_hash;
} DM2_V1_G1CreatureV5Material;

#define DM2_V1_G1_CREATURE_V5_MAX 8

typedef struct {
    int valid;
    int map;
    int count;
    DM2_V1_G1CreatureV5Material materials[DM2_V1_G1_CREATURE_V5_MAX];
} DM2_V1_G1CreatureV5RuntimeReceipt;

typedef struct {
    int creature_index;
    int creature_type;
    int source_kind;
    uint16_t object_id;
    int frame_index;
    int material_frame_index;
    int direction;
    int map_x;
    int map_y;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
    int rect14_applied;
    int rect14_scale64;
    int rect14_lateral_offset;
    int rect14_flip_mirror;
    /* Source-owned FB/FC/FD V5 field route: the sprite's dtImage field came
     * from the real animation chain, so the row draws CREATURES/type/field
     * and never the F9 map chip. */
    int source_v5_field;
    /* _4976_5aa4 occupancy evidence: the creature's 5x5 position inside its
     * cell (QUERY_CREATURE_5x5_POS) and its index in the source
     * tlbDisplayOrder* walk (DRAW_STATIC_OBJECT).  -1 when unproven. */
    int occupancy_5x5;
    int occupancy_display_index;
    /* table1d7029 draw pass of the creature's viewport cell, or -1. */
    int source_pass;
} DM2_V1_CreatureRender;

typedef struct {
    DM2_V1_CreatureRender creatures[DM2_MAX_CREATURES_PER_SQ];
    int creature_count;
} DM2_V1_CreatureRenderPlan;

typedef struct {
    int gdat_index;
    int frame_x;
    int frame_y;
    int frame_w;
    int frame_h;
    DM2_V1_ViewportRect dst_rect;
    int src_stride;
    int transparent_color;
    int flip_mirror;
    int render_frame;
    int draw_order;
} DM2_V1_CreatureAssetBlit;

typedef struct {
    uint8_t  item_category;   /* source-owned GDAT category; 0 = unavailable */
    uint8_t  item_type;       /* GDAT item index */
    uint8_t  frame_index;     /* animation frame */
    int16_t  depth;           /* depth sort key */
    int16_t  screen_x;        /* viewport X position */
    int16_t  screen_y;        /* viewport Y position */
    uint8_t  direction;       /* source ObjectID::Dir(), for carried overlays */
    /* A direct G1 DB5 root is admitted only through DRAW_MAP_CHIP's
     * WEAPONS/type/F9 selector.  Generic runtime items leave this clear. */
    uint16_t object_id;
    int16_t map_x;
    int16_t map_y;
    uint8_t source_gdat_field;
    uint8_t source_g1_weapon;
    uint8_t source_g1_container;
    uint8_t source_static_object_admitted;
    uint8_t source_static_object_cell;
    int8_t source_static_object_pass;
    uint16_t source_static_object_clip_rect_id;
    uint32_t source_static_object_raw_gfx256_hash;
    uint32_t source_static_object_raw_gfx256_receipt_hash;
    uint32_t source_static_object_raw4_hash;
    uint32_t source_static_object_raw4_receipt_hash;
    /* Record-owned GDAT dtImageOffset for the DRAW_ITEM image field (the
     * signed high byte shifts x, the signed low byte shifts y).
     * Source: SKWIN/SkWinCore.cpp DRAW_ITEM lines 23973-23977. */
    uint16_t source_static_object_image_offset;
} DM2_ItemSprite;

typedef struct {
    int item_index;
    int item_category;
    int item_type;
    int frame_index;
    int direction;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
    uint16_t object_id;
    int map_x;
    int map_y;
    int source_gdat_field;
    int source_g1_weapon;
    int source_g1_container;
    int source_static_object_admitted;
    int source_static_object_cell;
    int source_static_object_pass;
    int source_static_object_clip_rect_id;
    uint32_t source_static_object_raw_gfx256_hash;
    uint32_t source_static_object_raw_gfx256_receipt_hash;
    uint32_t source_static_object_raw4_hash;
    uint32_t source_static_object_raw4_receipt_hash;
    int flip_mirror;
    int fallback_radius;
    uint8_t fallback_color;
    /* INTERFACE_GENERAL dt07/0x0A Rect14 row placement for this item, when the
     * runtime has bound the table and the frame index selects a valid row.
     * Source: SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST / DRAW_ITEM. */
    int rect14_applied;
    int rect14_scale64;
    int rect14_lateral_offset;
    int rect14_flip_mirror;
    uint32_t rect14_row_hash;
    uint32_t rect14_placement_hash;
    /* Source-owned DRAW_ITEM placement from DM2_V1_StaticObjectSourcePlan.
     * Used when the runtime has admitted a DB5/DB9 static object through the
     * source cell/pass/clip route but no INTERFACE_GENERAL Rect14 row matched.
     * Source: SKWIN/SkWinCore.cpp DRAW_ITEM / DRAW_PUT_DOWN_ITEM. */
    int source_static_object_placement_valid;
    int source_static_object_stretch_factor64;
    int source_static_object_slot_x_offset;
    int source_static_object_slot_y_offset;
    int source_static_object_position_5x5;
    int source_static_object_image_field;
    int source_static_object_flip_mirror;
    int source_static_object_image_offset;
} DM2_V1_ItemRender;

typedef struct {
    DM2_V1_ItemRender items[DM2_MAX_ITEMS_PER_SQ];
    int item_count;
} DM2_V1_ItemRenderPlan;

typedef struct {
    DM2_V1_ItemRender item;
    int item_present;
} DM2_V1_CarriedItemRenderPlan;

typedef struct {
    DM2_V1_ItemRender items[DM2_MAX_CREATURE_POSSESSION_ITEMS];
    int item_count;
} DM2_V1_CreaturePossessionItemRenderPlan;

typedef struct {
    int gdat_index;
    int frame_x;
    int frame_y;
    int frame_w;
    int frame_h;
    DM2_V1_ViewportRect dst_rect;
    int src_stride;
    int transparent_color;
    int flip_mirror;
    int render_frame;
    int draw_order;
} DM2_V1_ItemAssetBlit;

typedef struct {
    uint8_t  projectile_category; /* source-owned GDAT category; 0 = unavailable */
    uint8_t  projectile_type; /* spell/arrow/bolt type */
    uint8_t  frame_index;     /* animation frame */
    int16_t  depth;           /* depth sort key */
    int16_t  screen_x;
    int16_t  screen_y;
    int16_t  velocity_x;      /* pixel velocity */
    int16_t  velocity_y;
    uint8_t  direction;       /* 0=N, 1=E, 2=S, 3=W */
    uint8_t  palette_shift;   /* light/color modifier */
    uint8_t  render_kind;     /* DM2_V1_PROJECTILE_RENDER_* */
    uint8_t  object_direction;/* skproject ObjectID::Dir() source direction */
    uint8_t  frame_class;     /* DM2_V1_PROJECTILE_FRAME_CLASS_* */
} DM2_Projectile;

typedef struct {
    int projectile_index;
    int projectile_category;
    int projectile_type;
    int frame_index;
    int render_frame;
    int direction;
    int object_direction;
    int frame_class;
    int render_kind;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
    int flip_mirror;
    int cloud_flip_from_seed;
    int fallback_dx;
    int fallback_dy;
    int fallback_len;
    uint8_t fallback_color;
    /* INTERFACE_GENERAL dt07/0x0A Rect14 row placement for this projectile,
     * when the runtime has bound the table and the frame index selects a valid
     * row. Source: SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST / DRAW_TEMP_PICST. */
    int rect14_applied;
    int rect14_scale64;
    int rect14_lateral_offset;
    int rect14_flip_mirror;
    uint32_t rect14_row_hash;
    uint32_t rect14_placement_hash;
} DM2_V1_ProjectileRender;

typedef struct {
    int gdat_index;
    int frame_x;
    int frame_y;
    int frame_w;
    int frame_h;
    DM2_V1_ViewportRect dst_rect;
    int src_stride;
    int transparent_color;
    int flip_mirror;
    int render_frame;
    int draw_order;
    uint32_t random_seed_before;
    uint32_t random_seed_after;
} DM2_V1_ProjectileAssetBlit;

typedef struct {
    DM2_V1_ProjectileRender projectiles[DM2_MAX_PROJECTILES];
    int projectile_count;
} DM2_V1_ProjectileRenderPlan;

typedef enum {
    DM2_V1_WEATHER_OVERLAY_NONE = 0,
    DM2_V1_WEATHER_OVERLAY_RAIN = 1,
    DM2_V1_WEATHER_OVERLAY_FOG = 2,
    DM2_V1_WEATHER_OVERLAY_STORM = 3
} DM2_V1_WeatherOverlayKind;

typedef struct {
    DM2_V1_WeatherOverlayKind kind;
    int intensity;
    int density;
    int scroll;
    int alpha;
    int streak_step;
    int lightning_flash;
    uint8_t rain_color;
    uint8_t fog_target_color;
    uint8_t lightning_color;
} DM2_V1_WeatherOverlayRenderPlan;

typedef enum {
    DM2_V1_WEATHER_COMMAND_NONE = 0,
    DM2_V1_WEATHER_COMMAND_RAIN_STREAKS = 1,
    DM2_V1_WEATHER_COMMAND_FOG_BLEND = 2,
    DM2_V1_WEATHER_COMMAND_LIGHTNING_FILL = 3
} DM2_V1_WeatherOverlayCommandKind;

typedef struct {
    DM2_V1_WeatherOverlayCommandKind kind;
    int density;
    int scroll;
    int streak_step;
    int alpha;
    uint8_t color;
    uint8_t target_color;
} DM2_V1_WeatherOverlayCommand;

#define DM2_V1_WEATHER_OVERLAY_COMMAND_MAX 3

typedef struct {
    DM2_V1_WeatherOverlayCommand commands[
        DM2_V1_WEATHER_OVERLAY_COMMAND_MAX];
    int command_count;
} DM2_V1_WeatherOverlayCommandPlan;

typedef int (*DM2_V1_ViewportAssetFetch)(
    void *user,
    int gdat_index,
    const uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride);
typedef int (*DM2_V1_ViewportDoorSurfaceViewFetch)(
    void *user,
    int gdat_index,
    DM2_V1_BootViewportSurfaceView *out_view);
typedef int (*DM2_V1_ViewportAssetPaletteFetch)(
    void *user,
    int gdat_index,
    uint8_t out_palette16[16],
    uint32_t *out_hash);

#include "dm2_v1_surface_snapshot.h"

/* ── Viewport state ────────────────────────────────────────────── */
typedef struct {
    /* View geometry */
    int party_dir;             /* 0=N, 1=E, 2=S, 3=W */
    int party_x;
    int party_y;
    int dungeon_level;

    /* Framebuffer output */
    uint8_t *framebuffer;      /* 320×200 pixel buffer */
    int      fb_stride;        /* bytes per row */
    DM2_V1_ViewportSurfaceSnapshot surface_snapshot;

    /* View squares — populated by the world model (Phase 3) */
    DM2_ViewSquare squares[DM2_SQ_COUNT];

    /* Sprite pools */
    DM2_CreatureSprite creatures[DM2_MAX_CREATURES_PER_SQ];
    int creature_count;
    DM2_ItemSprite items[DM2_MAX_ITEMS_PER_SQ];
    int item_count;
    DM2_ItemSprite creature_possession_items[DM2_MAX_CREATURE_POSSESSION_ITEMS];
    int creature_possession_item_count;
    DM2_ItemSprite carried_item;
    int carried_item_present;
    DM2_Projectile projectiles[DM2_MAX_PROJECTILES];
    int projectile_count;
    DM2_V1_HudPartyState hud_party;
    int hud_party_valid;
    DM2_V1_HudHandActionSource hud_hand_action_source;

    /* Weather */
    int weather;               /* 0=clear, 1=rain, 2=fog, 3=storm */
    int rain_intensity;        /* 0–100 */

    /* Outdoor state */
    int is_outdoor;
    DM2_V1_G1FirstMapRuntimeReceipt g1_first_map_runtime;
    DM2_V1_G1TeleporterTransitionReceipt g1_map0_teleporter_transition;
    float time_of_day;         /* 0.0–1.0 */

    /* Rendering state */
    int dirty;                 /* 1=viewport needs full redraw */
    int tick_count;           /* frame counter for weather animation */
    uint32_t random_seed;      /* skproject RAND02-compatible render seed */

    DM2_V1_ViewportAssetFetch asset_fetch;
    void *asset_user;
    DM2_V1_ViewportAssetPaletteFetch asset_palette_fetch;
    void *asset_palette_user;
    int active_asset_palette_ready;
    uint32_t active_asset_palette_hash;
    uint8_t active_asset_palette16[16];
    DM2_V1_ViewportDoorSurfaceViewFetch door_surface_view_fetch;
    void *door_surface_view_user;
    /* Source-owned GDAT loader for RAW4 placement tables and image metadata.
     * Decoded pixel fetches still go through asset_fetch; this pointer is only
     * used when source_materials_required is active. */
    const DM2_V1_AssetLoader *asset_loader;
    /* A boot-owned, hash-verified GDAT provider must never be replaced by
     * aggregate paint.  skproject SKWIN/SkWinCore.cpp (DRAW_MAP_CHIP) resolves
     * GRAPHICSSET/WALL_GFX imagery before its blit; a failed decode is a
     * blocked source-material frame, not a new Firestaff wall or plane. */
    int source_materials_required;
    int blocked_material_draw_count;
    uint32_t blocked_material_mask;
    int asset_floor_ceiling_drawn_count;
    int fallback_floor_ceiling_drawn_count;
    int asset_outdoor_sky_drawn_count;
    int asset_outdoor_ground_drawn_count;
    int asset_wall_drawn_count;
    int fallback_wall_drawn_count;
    int asset_wall_ornament_drawn_count;
    int fallback_wall_ornament_drawn_count;
    int gdat_scene_control_ready;
    uint32_t gdat_scene_map_load_token;
    DM2_V1_GraphicsSetStaticSceneReceipt gdat_static_scene_record;
    uint32_t gdat_static_light_map_load_token;
    uint32_t gdat_static_light_scene_control_hash;
    int gdat_static_light_control_owned;
    uint32_t gdat_static_ambient_light_map_load_token;
    uint32_t gdat_static_ambient_light_scene_control_hash;
    int gdat_static_ambient_light_control_owned;
    uint32_t gdat_static_ambient_darkness_map_load_token;
    uint32_t gdat_static_ambient_darkness_scene_control_hash;
    int gdat_static_ambient_darkness_control_owned;
    uint32_t gdat_static_scene_flags_map_load_token;
    uint32_t gdat_static_scene_flags_scene_control_hash;
    int gdat_static_scene_flags_control_owned;
    uint32_t gdat_static_scene_colorkey_map_load_token;
    uint32_t gdat_static_scene_colorkey_scene_control_hash;
    int gdat_static_scene_colorkey_control_owned;
    uint32_t gdat_static_scene_floor_material_map_load_token;
    uint32_t gdat_static_scene_floor_material_scene_control_hash;
    int gdat_static_scene_floor_material_owned;
    uint32_t gdat_static_scene_ceiling_material_map_load_token;
    uint32_t gdat_static_scene_ceiling_material_scene_control_hash;
    int gdat_static_scene_ceiling_material_owned;
    uint32_t gdat_static_scene_wall_material_map_load_token;
    uint32_t gdat_static_scene_wall_material_scene_control_hash;
    int gdat_static_scene_wall_material_owned;
    /* One bit per DM2_SQ_* panel whose GRAPHICSSET wall field has been
     * source-bound for the active map generation.  This is deliberately a
     * viewport-square mask rather than a compacted field number: skproject
     * addresses field `viewportCell + 0x22`, and D3C has no wall payload. */
    uint16_t gdat_static_scene_wall_material_mask;
    uint8_t gdat_static_scene_wall_material_view_square;
    uint8_t gdat_static_scene_wall_material_field;
    uint32_t gdat_static_scene_door_frame_material_map_load_token;
    uint32_t gdat_static_scene_door_frame_material_scene_control_hash;
    int gdat_static_scene_door_frame_material_owned;
    uint32_t gdat_static_scene_door_frame_d1c_material_map_load_token;
    uint32_t gdat_static_scene_door_frame_d1c_material_scene_control_hash;
    int gdat_static_scene_door_frame_d1c_material_owned;
    uint32_t gdat_static_scene_door_frame_d2c_material_map_load_token;
    uint32_t gdat_static_scene_door_frame_d2c_material_scene_control_hash;
    int gdat_static_scene_door_frame_d2c_material_owned;
    DM2_V1_FloorGfxViewportOwnershipReceipt floor_gfx_viewport_ownership;
    int gdat_scene_control_consumed_count;
    int gdat_scene_light_consumed_count;
    /* `c_light.cpp::DM2_RECALC_LIGHT_LEVEL` is a separate runtime result
     * from GRAPHICSSET control words.  Keep its authenticated transaction
     * explicit so a later raw save/live-state bridge cannot be replaced by a
     * host brightness value. */
    int gdat_c_light_receipt_ready;
    uint8_t gdat_c_light_level;
    uint32_t gdat_c_light_scene_control_hash;
    uint32_t gdat_c_light_source_state_hash;
    uint32_t gdat_c_light_receipt_hash;
    int gdat_c_light_consumed_count;
    int gdat_scene_weather_consumed_count;
    const DM2_V1_WeatherRendererReceipt *gdat_weather_renderer_receipt;
    uint8_t gdat_weather_renderer_graphicsset;
    uint32_t gdat_weather_renderer_consumed_hash;
    unsigned int gdat_weather_renderer_consumed_command_count;
    int asset_weather_drawn_count;
    int asset_teleporter_drawn_count;
    int gdat_sprite_palette_consumed_count;
    uint32_t gdat_scene_control_hash;
    uint16_t gdat_scene_colorkey;
    uint16_t gdat_scene_flags;
    uint16_t gdat_ambient_light;
    uint16_t gdat_highest_light_level;
    uint16_t gdat_void_random_fall;
    uint16_t gdat_animated_floor;
    uint16_t gdat_scene_rain;
    uint16_t gdat_misty_map;
    uint16_t gdat_thunder_position;
    uint16_t gdat_ambient_darkness;
    uint8_t gdat_scene_light_floor;
    uint8_t gdat_scene_light_search_depth;
    int gdat_scene_light_recompute_enabled;
    /* SKProject DRAW_DUNGEON_GRAPHIC offsets only the 700/701 planes while
     * glbIsPlayerMoving is live. This is a boolean rather than a host-provided
     * displacement: the two source offsets are fixed. */
    int gdat_scene_movement_active;
    /* G1 map-header coordinates belong to the original plane-flip predicate. */
    int gdat_scene_map_offset_x;
    int gdat_scene_map_offset_y;
    int gdat_scene_material_index;
    /* The active MAP's decoded GRAPHICSSET planes are retained by the runtime
     * plan. These are never synthesized and are preferred over a second GDAT
     * lookup during the same M11 frame. */
    const DM2_V1_GdatSceneM11CommandPlan *gdat_scene_material_plan;
    int gdat_scene_material_plan_rejected;
    const DM2_V1_GdatDoorOverlayM11CommandPlan *gdat_door_overlay_material_plan;
    int gdat_door_overlay_material_plan_consumed_count;
    const DM2_V1_GdatWallM11CommandPlan *gdat_wall_material_plan;
    /* Bound when UPDATE_GFXSET installs the plan.  A later G1 scene-control
     * transaction invalidates the pointer before any M10 wall draw. */
    uint32_t gdat_wall_material_plan_scene_control_hash;
    int gdat_scene_material_consumed_count;
    /* Validated c_gui_vp ceiling/floor draws consumed in source order. */
    int gdat_scene_draw_order_consumed_count;
    /* Counts only wall blits supplied by the boot-owned GRAPHICSSET plan. */
    int gdat_wall_material_plan_consumed_count;
    /* Source-owned WALL_GFX ornament placement plan bound by the runtime.
     * Without this plan a visible wall ornament blocks the frame. */
    const DM2_V1_WallOrnamentRenderPlan *gdat_wall_ornament_material_plan;
    int gdat_interface_palette_ready;
    int gdat_interface_palette_consumed_count;
    int gdat_material_palette_floor_ceiling_consumed_count;
    int gdat_material_palette_wall_consumed_count;
    int gdat_material_palette_door_frame_consumed_count;
    uint32_t gdat_interface_palette_hash;
    uint8_t gdat_interface_palette16[16];
    int gdat_interface_text_palette_ready;
    uint32_t gdat_interface_text_palette_hash;
    uint8_t gdat_interface_text_palette16[16];
    const uint8_t *gdat_interface_font_rows;
    uint32_t gdat_interface_font_hash;
    int gdat_interface_action_palette_consumed_count;
    int gdat_interface_font_consumed_count;
    const DM2_V1_InterfaceHudLayout *gdat_interface_hud_layout;
    const uint8_t *gdat_interface_rect14_rows;
    uint32_t gdat_interface_rect14_row_count;
    uint32_t gdat_interface_rect14_hash;
    int gdat_interface_rect14_consumed_count;
    int gdat_local_palette_consumed_count;
    int asset_door_panel_drawn_count;
    int asset_door_overlay_drawn_count;
    int asset_door_frame_drawn_count;
    int asset_door_button_drawn_count;
    int fallback_door_drawn_count;
    int last_door_panel_asset_blit_valid;
    int last_door_panel_asset_src_w;
    int last_door_panel_asset_src_h;
    int last_door_panel_asset_src_stride;
    DM2_V1_DoorAssetBlit last_door_panel_asset_blit;
    int last_door_ornate_asset_blit_valid;
    int last_door_ornate_asset_src_w;
    int last_door_ornate_asset_src_h;
    int last_door_ornate_asset_src_stride;
    DM2_V1_DoorAssetBlit last_door_ornate_asset_blit;
    int last_door_destroyed_mask_asset_blit_valid;
    int last_door_destroyed_mask_asset_src_w;
    int last_door_destroyed_mask_asset_src_h;
    int last_door_destroyed_mask_asset_src_stride;
    DM2_V1_DoorAssetBlit last_door_destroyed_mask_asset_blit;
    int last_door_frame_asset_blit_valid;
    int last_door_frame_asset_src_w;
    int last_door_frame_asset_src_h;
    int last_door_frame_asset_src_stride;
    DM2_V1_DoorAssetBlit last_door_frame_asset_blit;
    DM2_V1_OriginalMaterialGateReceipt last_original_material_gate;
    DM2_V1_OriginalDoorSurfaceRequest last_original_door_surface_request;
    DM2_V1_OriginalDoorSurfaceBinding last_original_door_surface_binding;
    DM2_V1_OriginalDoorOpeningFrameRequest
        last_original_door_opening_frame_request;
    DM2_V1_ViewportDoorPresentationCommand
        last_original_door_presentation_command;
    DM2_V1_ViewportHudMaterialRequest last_hud_top_bar_material_request;
    DM2_V1_ViewportHudPresentationCommand
        last_hud_top_bar_presentation_command;
    DM2_V1_ViewportHudMaterialRequest
        last_hud_status_panel_material_request;
    DM2_V1_ViewportHudPresentationCommand
        last_hud_status_panel_presentation_command;
    DM2_V1_ViewportHudMaterialRequest last_hud_hand_action_material_request;
    DM2_V1_ViewportHudPresentationCommand
        last_hud_hand_action_presentation_command;
    DM2_V1_ViewportDungeonMaterialCommand
        last_dungeon_ceiling_presentation_command;
    DM2_V1_ViewportDungeonMaterialCommand
        last_dungeon_floor_presentation_command;
    DM2_V1_ViewportDungeonMaterialCommand
        last_dungeon_wall_presentation_command;
    DM2_V1_ViewportDungeonMaterialCommand
        last_wall_ornament_presentation_command;
    uint16_t last_floor_ceiling_material_required_mask;
    uint16_t last_floor_ceiling_material_consumed_mask;
    uint16_t last_door_material_required_mask;
    uint16_t last_door_material_consumed_mask;
    uint16_t last_outdoor_scene_material_required_mask;
    uint16_t last_outdoor_scene_material_consumed_mask;
    uint16_t last_dungeon_wall_material_required_mask;
    uint16_t last_dungeon_wall_material_consumed_mask;
    uint16_t last_wall_ornament_material_required_mask;
    uint16_t last_wall_ornament_material_consumed_mask;
    DM2_V1_ViewportSceneControlCommand last_scene_control_presentation_command;
    DM2_V1_ViewportCreatureMaterialCommand
        last_creature_presentation_command;
    DM2_V1_ViewportItemMaterialCommand last_item_presentation_command;
    DM2_V1_ViewportFrameCompositionReceipt last_frame_composition;
    DM2_V1_ViewportM11FrameReceipt last_m11_frame_receipt;
    int last_door_button_asset_blit_valid;
    int last_door_button_asset_src_w;
    int last_door_button_asset_src_h;
    int last_door_button_asset_src_stride;
    DM2_V1_DoorAssetBlit last_door_button_asset_blit;
    int asset_creature_drawn_count;
    int fallback_creature_drawn_count;
    /* Every successful DRAW_MAP_CHIP/QUERY_CREATURE_PICST blit records its
     * source GDAT key in draw order. Runtime folds this exact list into the
     * M11 material receipt; it must never infer missing creature material
     * from the final blit alone. */
    int creature_material_drawn_count;
    int creature_material_gdat_indices[DM2_MAX_CREATURES_PER_SQ];
    int last_creature_asset_blit_valid;
    int last_creature_render_valid;
    int last_creature_draw_order;
    int last_creature_asset_src_w;
    int last_creature_asset_src_h;
    int last_creature_asset_src_stride;
    DM2_V1_CreatureRender last_creature_asset_render;
    DM2_V1_CreatureAssetBlit last_creature_asset_blit;
    DM2_V1_CreatureRender last_creature_render;
    int asset_item_drawn_count;
    int fallback_item_drawn_count;
    int asset_creature_possession_item_drawn_count;
    int fallback_creature_possession_item_drawn_count;
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *g1_creature_map_chip_materials;
    const DM2_V1_G1CreatureV5RuntimeReceipt *g1_creature_v5_materials;
    const DM2_V1_G1WeaponMapChipRuntimeReceipt *g1_weapon_map_chip_materials;
    const DM2_V1_G1ContainerMapChipRuntimeReceipt *g1_container_map_chip_materials;
    int g1_scene_creature_material_ready;
    int g1_scene_creature_material_map_x;
    int g1_scene_creature_material_map_y;
    int g1_scene_creature_material_type;
    int g1_scene_creature_material_gdat_index;
    int g1_scene_creature_material_width;
    int g1_scene_creature_material_height;
    int g1_scene_creature_material_stride;
    const uint8_t *g1_scene_creature_material_pixels;
    uint32_t g1_scene_creature_material_pixel_hash;
    uint8_t g1_scene_creature_material_palette16[16];
    uint32_t g1_scene_creature_material_palette_hash;
    int g1_scene_creature_material_consumed_count;
    /* One source-admitted DB5/DB9 DRAW_MAP_CHIP material can cross the M11
     * frame directly.  It is intentionally separate from creature state:
     * c_gui_vp.cpp chooses WEAPONS/CONTAINERS by the record class/type/F9. */
    int g1_scene_item_material_ready;
    int g1_scene_item_material_category;
    int g1_scene_item_material_type;
    int g1_scene_item_material_gdat_index;
    uint16_t g1_scene_item_material_object_id;
    int g1_scene_item_material_map_x;
    int g1_scene_item_material_map_y;
    int g1_scene_item_material_width;
    int g1_scene_item_material_height;
    int g1_scene_item_material_stride;
    const uint8_t *g1_scene_item_material_pixels;
    uint32_t g1_scene_item_material_pixel_hash;
    uint8_t g1_scene_item_material_palette16[16];
    uint32_t g1_scene_item_material_palette_hash;
    uint32_t g1_scene_item_material_raw_gfx256_hash;
    uint32_t g1_scene_item_material_raw_gfx256_receipt_hash;
    uint32_t g1_scene_item_material_raw4_hash;
    uint32_t g1_scene_item_material_raw4_receipt_hash;
    int g1_scene_item_material_consumed_count;
    /* A DB2/DB3 WALL_GFX field-1 image can be selected by
     * DRAW_DEFAULT_DOOR_BUTTON. Keep that exact decoded surface with the
     * M11 frame so the button never resolves a second, possibly stale asset. */
    int g1_scene_wall_button_material_ready;
    int g1_scene_wall_button_material_gdat_index;
    int g1_scene_wall_button_material_wall_gfx_index;
    int g1_scene_wall_button_material_field;
    int g1_scene_wall_button_material_map_x;
    int g1_scene_wall_button_material_map_y;
    uint16_t g1_scene_wall_button_material_object_id;
    int g1_scene_wall_button_material_width;
    int g1_scene_wall_button_material_height;
    int g1_scene_wall_button_material_stride;
    const uint8_t *g1_scene_wall_button_material_pixels;
    uint32_t g1_scene_wall_button_material_pixel_hash;
    uint8_t g1_scene_wall_button_material_palette16[16];
    uint32_t g1_scene_wall_button_material_palette_hash;
    uint16_t g1_scene_wall_button_material_raw_index;
    const uint8_t *g1_scene_wall_button_material_raw_bytes;
    size_t g1_scene_wall_button_material_raw_byte_count;
    uint32_t g1_scene_wall_button_material_raw_hash;
    uint32_t g1_scene_wall_button_material_receipt_hash;
    int g1_scene_wall_button_material_consumed_count;
    const DM2_V1_G1TextWallGfxRuntimeReceipt *g1_text_wall_gfx_materials;
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *g1_actuator_wall_gfx_materials;
    int asset_carried_item_drawn_count;
    int fallback_carried_item_drawn_count;
    /* All successful visible object map-chip blits in presentation order.
     * source_kind is 1=floor, 2=creature possession, 3=leader hand. */
    int item_material_drawn_count;
    int item_material_gdat_indices[DM2_MAX_PRESENTED_ITEM_MATERIALS];
    uint8_t item_material_source_kinds[DM2_MAX_PRESENTED_ITEM_MATERIALS];
    int last_item_render_valid;
    int last_item_asset_blit_valid;
    int last_item_source_kind;
    int last_item_draw_order;
    int last_item_asset_src_w;
    int last_item_asset_src_h;
    int last_item_asset_src_stride;
    DM2_V1_ItemRender last_item_render;
    DM2_V1_ItemAssetBlit last_item_asset_blit;
    int asset_projectile_drawn_count;
    int fallback_projectile_drawn_count;
    /* Every successful DRAW_CHIP_OF_MAGIC_MAP source blit in presentation
     * order. M11 must not reduce a mixed missile/cloud frame to its final
     * GDAT key. */
    int projectile_material_drawn_count;
    int projectile_material_gdat_indices[DM2_MAX_PROJECTILES];
    int last_projectile_render_valid;
    int last_projectile_asset_blit_valid;
    int last_projectile_draw_order;
    int last_projectile_asset_src_w;
    int last_projectile_asset_src_h;
    int last_projectile_asset_src_stride;
    DM2_V1_ProjectileRender last_projectile_render;
    DM2_V1_ProjectileAssetBlit last_projectile_asset_blit;
    int asset_hud_core_drawn_count;
    int fallback_hud_core_drawn_count;
    uint32_t last_hud_core_gdat_hash;
    uint32_t last_hud_core_pixel_count;
    int asset_hud_portrait_drawn_count;
    int fallback_hud_portrait_drawn_count;
    const DM2_V1_GdatHudM11CommandPlan *gdat_hud_material_plan;
    /* One successful blit per exact command in the boot-owned HUD plan.
     * A source-required frame must not let M11 promote a partial plan. */
    int gdat_hud_material_plan_consumed_count;
    /* c_dialog.cpp::DM2_dialog_2066_3820 owns this image and RECT_453.
     * Admission alone must not create a host dialogue. */
    DM2_V1_DialogueBoxHostCommand gdat_dialogue_box_command;
    int gdat_dialogue_box_active;
    int gdat_dialogue_box_consumed_count;
    uint32_t gdat_dialogue_box_consumed_hash;
    DM2_V1_DialogueOpenPanelHostCommand gdat_dialogue_open_panel_command;
    int gdat_dialogue_open_panel_active;
    int gdat_dialogue_open_panel_consumed_count;
    uint32_t gdat_dialogue_open_panel_consumed_hash;
} DM2_V1_ViewportState;

/* ── Initialization ────────────────────────────────────────────── */
void dm2_v1_viewport_init(DM2_V1_ViewportState *s, uint8_t *framebuffer, int stride);
int dm2_v1_viewport_bind_surface(DM2_V1_ViewportState *s, uint8_t *framebuffer,
                                 int stride);
int dm2_v1_viewport_surface_snapshot(const DM2_V1_ViewportState *s,
                                     DM2_V1_ViewportSurfaceSnapshot *out);
void dm2_v1_viewport_set_party(DM2_V1_ViewportState *s, int dir, int x, int y);
void dm2_v1_viewport_set_outdoor(DM2_V1_ViewportState *s, int is_outdoor);
void dm2_v1_viewport_set_g1_first_map_runtime(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1FirstMapRuntimeReceipt *receipt);
void dm2_v1_viewport_set_g1_map0_teleporter_transition(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TeleporterTransitionReceipt *receipt);
void dm2_v1_viewport_set_level(DM2_V1_ViewportState *s, int level);
void dm2_v1_viewport_set_weather(DM2_V1_ViewportState *s, int weather, int rain_intensity);
void dm2_v1_viewport_set_time(DM2_V1_ViewportState *s, float time_of_day);
void dm2_v1_viewport_set_hud_party(DM2_V1_ViewportState *s,
                                   const DM2_V1_HudPartyState *party);
void dm2_v1_viewport_set_hud_hand_action_source(
    DM2_V1_ViewportState *s,
    const DM2_V1_HudHandActionSource *source);
void dm2_v1_viewport_set_asset_provider(DM2_V1_ViewportState *s,
                                         DM2_V1_ViewportAssetFetch fetch,
                                         void *user);
void dm2_v1_viewport_set_gdat_door_overlay_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan);
void dm2_v1_viewport_set_asset_palette_provider(
    DM2_V1_ViewportState *s,
    DM2_V1_ViewportAssetPaletteFetch fetch,
    void *user);
void dm2_v1_viewport_set_door_surface_view_provider(
    DM2_V1_ViewportState *s,
    DM2_V1_ViewportDoorSurfaceViewFetch fetch,
    void *user);
void dm2_v1_viewport_set_asset_loader(
    DM2_V1_ViewportState *s,
    const DM2_V1_AssetLoader *loader);
void dm2_v1_viewport_set_source_materials_required(
    DM2_V1_ViewportState *s, int required);
void dm2_v1_viewport_set_gdat_scene_control(
    DM2_V1_ViewportState *s,
    int ready,
    int graphicsset_index,
    uint32_t hash,
    uint16_t scene_colorkey,
    uint16_t scene_flags,
    uint16_t ambient_light,
    uint16_t highest_light_level,
    uint16_t void_random_fall,
    uint16_t animated_floor,
    uint16_t scene_rain,
    uint16_t misty_map,
    uint16_t thunder_position,
    uint16_t ambient_darkness);
/* Bind the current map-load identity before any static GRAPHICSSET controls
 * can be retained by the viewport. */
void dm2_v1_viewport_set_scene_map_load_token(
    DM2_V1_ViewportState *s, uint32_t source_map_load_token);
/* skproject UPDATE_GFXSET admits the selected GRAPHICSSET control record
 * only under the current map-load token and its source control hash. */
int dm2_v1_viewport_bind_static_graphicsset_scene_record(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject static light control is map-local and must repeat the same
 * source GRAPHICSSET identity as the scene record. */
int dm2_v1_viewport_bind_static_scene_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* c_light.cpp's terminal result may enter a source-required dungeon frame
 * only when it names this exact UPDATE_GFXSET transaction.  It carries view
 * metadata only; no palette or pixel transform is implied by this bind. */
void dm2_v1_viewport_set_c_light_receipt(
    DM2_V1_ViewportState *s,
    const DM2_V1_CLightM11Receipt *receipt);
/* c_gui_vp consumes UPDATE_GFXSET's already decoded floor/ceiling pair.
 * The caller retains the plan through this viewport render; an incomplete or
 * differently addressed plan is rejected by the source-required plane gate. */
void dm2_v1_viewport_set_gdat_scene_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatSceneM11CommandPlan *plan);
/* Source-locked 700/701 movement presentation. When active, the ceiling is
 * drawn at -2 and the floor at +3 pixels, matching SKProject's initialized
 * _4976_00fa/_4976_00fc values. */
void dm2_v1_viewport_set_gdat_scene_movement_active(
    DM2_V1_ViewportState *s, int active);
void dm2_v1_viewport_set_gdat_scene_map_origin(
    DM2_V1_ViewportState *s, int map_offset_x, int map_offset_y);
void dm2_v1_viewport_set_gdat_wall_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatWallM11CommandPlan *plan);
void dm2_v1_viewport_set_gdat_wall_ornament_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_WallOrnamentRenderPlan *plan);
void dm2_v1_viewport_set_gdat_hud_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatHudM11CommandPlan *plan);
void dm2_v1_viewport_set_gdat_dialogue_box_host_command(
    DM2_V1_ViewportState *s,
    const DM2_V1_DialogueBoxHostCommand *command,
    int active);
void dm2_v1_viewport_set_gdat_dialogue_open_panel_host_command(
    DM2_V1_ViewportState *s,
    const DM2_V1_DialogueOpenPanelHostCommand *command,
    int active);
/* ReDMCSB/skproject SKWIN/SkWinCore.cpp RECALC_LIGHT_LEVEL 5093: AMBIANT_LIGHT
 * belongs to the active GRAPHICSSET control before its bounded level floor.
 * This binds source ownership only; it does not calculate or draw light. */
int dm2_v1_viewport_bind_static_scene_ambient_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp CHECK_RECOMPUTE_LIGHT 30416-30439 uses the
 * active GRAPHICSSET AMBIANT_DARKNESS word as map-local light-check control.
 * The viewport retains ownership only, never a darkness overlay. */
int dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp IS_MAP_INSIDE 56859 reads SCENE_FLAGS bit
 * 0x20 from the active GRAPHICSSET. The receipt publishes only that source
 * scene classification, never an outdoor image or palette route. */
int dm2_v1_viewport_bind_static_scene_flags_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp map load 45029 assigns SCENE_COLORKEY before
 * later dungeon-graphic calls. This binds the source word only; it must not
 * create a new colorkey blit, image decode, or visual fallback. */
int dm2_v1_viewport_bind_static_scene_colorkey_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp map load 45041 and dungeon draw 48022 use
 * GRAPHICSSET/FLOOR. This publishes its typed address only, never pixels. */
int dm2_v1_viewport_bind_static_scene_floor_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject DRAW_WALL (0x32CB:4F3B) selects GRAPHICSSET field
 * viewportCell+0x22 with the active map set. Bind one verified visible panel
 * so the real source route can enter the indoor composition without a wall
 * fallback. */
int dm2_v1_viewport_bind_static_scene_wall_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash,
    int view_square);
/* Bind every real GRAPHICSSET wall field which the viewport can render.
 * skproject DRAW_WALL performs the same per-cell lookup before blitting; a
 * partial field set must not authorize a source-required wall frame. */
int dm2_v1_viewport_bind_static_scene_all_wall_materials(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp map load 45046 and dungeon draw 48011 use
 * GRAPHICSSET/CEIL. This publishes its typed address only, never pixels. */
int dm2_v1_viewport_bind_static_scene_ceiling_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp 47762 directly addresses GRAPHICSSET's
 * front D1 door frame. This publishes the typed address only, never pixels. */
int dm2_v1_viewport_bind_static_scene_door_frame_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES 46311-46334 resolves the
 * D1 side-frame route. This publishes field 0x07 only, never pixels. */
int dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
/* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_FRAMES 46311-46334 resolves the
 * D2 side-frame route. This publishes field 0x09 only, never pixels. */
int dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
int dm2_v1_viewport_set_floor_gfx_viewport_ownership(
    DM2_V1_ViewportState *s,
    const DM2_V1_FloorGfxViewportOwnershipReceipt *ownership);
int dm2_v1_viewport_floor_gfx_render_plan_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportFloorGfxRenderPlanReceipt *out_receipt);
/* skproject/SKWIN/SkWinCore.cpp RECALC_LIGHT_LEVEL and
 * CHECK_RECOMPUTE_LIGHT: derive only the bounded control plan, never a
 * procedural brightness/palette substitute. */
void dm2_v1_viewport_scene_light_control(uint16_t highest_light_level,
                                         uint16_t ambient_darkness,
                                         uint8_t *out_light_floor,
                                         uint8_t *out_search_depth,
                                         int *out_recompute_enabled);
/* skproject SkWinCore::INIT loads dtPalIRGB/dtPalette16 before HUD drawing.
 * The viewport accepts only the already validated logical-index table owned by
 * the DM2 boot profile; HUD colours remain logical until this bind occurs. */
void dm2_v1_viewport_set_gdat_interface_palette(
    DM2_V1_ViewportState *s,
    int ready,
    uint32_t hash,
    const uint8_t palette16[16]);
void dm2_v1_viewport_set_gdat_scene_material_plan(
    DM2_V1_ViewportState *s,
    const DM2_V1_GdatSceneM11CommandPlan *plan);
void dm2_v1_viewport_set_gdat_weather_renderer_receipt(
    DM2_V1_ViewportState *s,
    uint8_t graphicsset_index,
    const DM2_V1_WeatherRendererReceipt *receipt);
void dm2_v1_viewport_set_gdat_interface_text_palette(
    DM2_V1_ViewportState *s,
    int ready,
    uint32_t hash,
    const uint8_t palette16[16]);
/* skproject QUERY_FONT consumes the six 128-byte dt07/0 rows as 3x6 HUD
 * glyph pixels.  The caller retains ownership through the boot profile. */
void dm2_v1_viewport_set_gdat_interface_font(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t hash);
void dm2_v1_viewport_set_g1_creature_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt);
void dm2_v1_viewport_set_g1_creature_v5_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1CreatureV5RuntimeReceipt *receipt);
int dm2_v1_g1_creature_v5_material_matches(
    const DM2_V1_G1CreatureV5RuntimeReceipt *receipt,
    uint16_t object_id, int map_x, int map_y, int creature_type,
    int image_field, int width, int height,
    uint32_t palette_hash, uint32_t decoded_hash);
void dm2_v1_viewport_set_g1_weapon_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt);
void dm2_v1_viewport_set_g1_container_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt);
void dm2_v1_viewport_set_g1_scene_creature_material(
    DM2_V1_ViewportState *s,
    int ready,
    int map_x,
    int map_y,
    int creature_type,
    int gdat_index,
    int width,
    int height,
    int stride,
    uint32_t palette_hash);
/* M10 binds the already decoded G1/GDAT handoff directly.  This path owns no
 * pixels; their boot provider must outlive the current viewport frame. */
void dm2_v1_viewport_set_g1_scene_creature_material_direct(
    DM2_V1_ViewportState *s, int ready, int map_x, int map_y,
    int creature_type, int gdat_index, const uint8_t *pixels,
    int width, int height, int stride, const uint8_t palette16[16],
    uint32_t palette_hash, uint32_t expected_pixel_hash);
void dm2_v1_viewport_set_g1_scene_item_material_direct(
    DM2_V1_ViewportState *s, int ready, int item_category, int item_type,
    int gdat_index, uint16_t object_id, int map_x, int map_y,
    const uint8_t *pixels, int width, int height, int stride,
    const uint8_t palette16[16], uint32_t palette_hash,
    uint32_t expected_pixel_hash);
void dm2_v1_viewport_set_g1_scene_static_item_material_direct(
    DM2_V1_ViewportState *s, int ready, int item_category, int item_type,
    int gdat_index, uint16_t object_id, int map_x, int map_y,
    const uint8_t *pixels, int width, int height, int stride,
    const uint8_t palette16[16], uint32_t palette_hash,
    uint32_t expected_pixel_hash, uint32_t raw_gfx256_hash,
    uint32_t raw_gfx256_receipt_hash, uint32_t raw4_hash,
    uint32_t raw4_receipt_hash);
void dm2_v1_viewport_set_g1_scene_wall_button_material_direct(
    DM2_V1_ViewportState *s, int ready, int gdat_index,
    int wall_gfx_index, int field, int map_x, int map_y,
    uint16_t object_id, const uint8_t *pixels, int width, int height,
    int stride, const uint8_t palette16[16], uint32_t palette_hash,
    uint32_t expected_pixel_hash, uint16_t raw_index,
    const uint8_t *raw_bytes, size_t raw_byte_count, uint32_t raw_hash,
    uint32_t raw_receipt_hash);
void dm2_v1_viewport_set_g1_wall_gfx_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TextWallGfxRuntimeReceipt *text_receipt,
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *actuator_receipt);
void dm2_v1_viewport_set_gdat_interface_hud_layout(
    DM2_V1_ViewportState *s,
    const DM2_V1_InterfaceHudLayout *layout);
/* skproject draws the live name/status overlay only after the original
 * INTERFACE_GENERAL geometry, palette, font and champion state are owned.
 * Callers leave the overlay untouched when this proof is incomplete. */
int dm2_v1_viewport_hud_dynamic_overlay_ready(
    const DM2_V1_ViewportState *s,
    const DM2_V1_HudChampionSlotRender *champion);
void dm2_v1_viewport_set_gdat_interface_rect14(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t row_count,
    uint32_t hash);
int dm2_v1_viewport_build_wall_panel_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WallPanelRenderPlan *out_plan);
int dm2_v1_viewport_build_door_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_DoorRenderPlan *out_plan);
int dm2_v1_viewport_door_panel_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit);
int dm2_v1_viewport_door_frame_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit);
int dm2_v1_viewport_last_original_material_gate_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalMaterialGateReceipt *out_receipt);
int dm2_v1_viewport_last_original_door_surface_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalDoorSurfaceRequest *out_request);
int dm2_v1_viewport_last_original_door_surface_binding(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalDoorSurfaceBinding *out_binding);
int dm2_v1_viewport_last_original_door_opening_frame_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_OriginalDoorOpeningFrameRequest *out_request);
int dm2_v1_viewport_last_original_door_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDoorPresentationCommand *out_command);
int dm2_v1_viewport_last_hud_top_bar_material_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudMaterialRequest *out_request);
int dm2_v1_viewport_last_hud_top_bar_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudPresentationCommand *out_command);
int dm2_v1_viewport_last_hud_status_panel_material_request(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudMaterialRequest *out_request);
int dm2_v1_viewport_last_hud_status_panel_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudPresentationCommand *out_command);
int dm2_v1_viewport_last_hud_hand_action_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportHudPresentationCommand *out_command);
int dm2_v1_viewport_last_dungeon_floor_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDungeonMaterialCommand *out_command);
int dm2_v1_viewport_last_dungeon_ceiling_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDungeonMaterialCommand *out_command);
int dm2_v1_viewport_last_dungeon_wall_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportDungeonMaterialCommand *out_command);
int dm2_v1_viewport_last_scene_control_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportSceneControlCommand *out_command);
int dm2_v1_viewport_last_creature_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportCreatureMaterialCommand *out_command);
int dm2_v1_viewport_last_item_presentation_command(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportItemMaterialCommand *out_command);
int dm2_v1_viewport_last_frame_composition_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportFrameCompositionReceipt *out_receipt);
/* M11-facing atomic source-required frame decision. It is absent unless all
 * requested dungeon and HUD passes share the active scene/palette ownership. */
int dm2_v1_viewport_last_m11_frame_receipt(
    const DM2_V1_ViewportState *s,
    DM2_V1_ViewportM11FrameReceipt *out_receipt);
int dm2_v1_viewport_door_button_asset_blit(
    const DM2_V1_DoorRender *render,
    int src_w,
    int src_h,
    int src_stride,
    DM2_V1_DoorAssetBlit *out_blit);
int dm2_v1_viewport_build_creature_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CreatureRenderPlan *out_plan);
int dm2_v1_viewport_creature_asset_blit(
    const DM2_V1_CreatureRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    DM2_V1_CreatureAssetBlit *out_blit);
int dm2_v1_viewport_build_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_ItemRenderPlan *out_plan);
int dm2_v1_viewport_build_carried_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CarriedItemRenderPlan *out_plan);
int dm2_v1_viewport_build_creature_possession_item_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_CreaturePossessionItemRenderPlan *out_plan);
int dm2_v1_viewport_item_asset_blit(
    const DM2_V1_ItemRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    int scale_base,
    int scale_max,
    DM2_V1_ItemAssetBlit *out_blit);
int dm2_v1_viewport_build_projectile_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_ProjectileRenderPlan *out_plan);
int dm2_v1_viewport_projectile_asset_blit(
    const DM2_V1_ProjectileRender *render,
    int src_w,
    int src_h,
    int src_stride,
    int party_direction,
    int tick_count,
    uint32_t *random_seed,
    DM2_V1_ProjectileAssetBlit *out_blit);
int dm2_v1_viewport_build_weather_overlay_render_plan(
    const DM2_V1_ViewportState *s,
    DM2_V1_WeatherOverlayRenderPlan *out_plan);
int dm2_v1_viewport_build_weather_overlay_commands(
    const DM2_V1_WeatherOverlayRenderPlan *plan,
    DM2_V1_WeatherOverlayCommandPlan *out_commands);

/* ── Lighting helpers ─────────────────────────────────────────── */
/* dm2_v1_viewport_object_light_level — compute object light intensity
 * for a tile at `distance_tiles`.
 * Boundary rule: distance >= source radius extinguishes to 0.
 */
uint8_t dm2_v1_viewport_object_light_level(uint8_t base_light_level,
                                           int distance_tiles,
                                           const DM2_CreatureSprite *source);

/* ── Main render entry ─────────────────────────────────────────── */
/* dm2_v1_viewport_render — render one complete viewport frame.
 * Calls the appropriate indoor (T560) or outdoor (T600) path. */
void dm2_v1_viewport_render(DM2_V1_ViewportState *s);

/* ── Per-pass render functions ────────────────────────────────── */
void dm2_v1_render_background(DM2_V1_ViewportState *s);
void dm2_v1_render_floor_ceiling(DM2_V1_ViewportState *s);
void dm2_v1_render_teleporter_fields(DM2_V1_ViewportState *s);
void dm2_v1_render_walls(DM2_V1_ViewportState *s);
void dm2_v1_render_wall_ornaments(DM2_V1_ViewportState *s);
void dm2_v1_render_doors(DM2_V1_ViewportState *s);
void dm2_v1_render_creatures(DM2_V1_ViewportState *s);
void dm2_v1_render_items(DM2_V1_ViewportState *s);
void dm2_v1_render_creature_possession_items(DM2_V1_ViewportState *s);
void dm2_v1_render_carried_item(DM2_V1_ViewportState *s);
void dm2_v1_render_projectiles(DM2_V1_ViewportState *s);
void dm2_v1_render_weather_overlay(DM2_V1_ViewportState *s);
void dm2_v1_render_ui_chrome(DM2_V1_ViewportState *s);
void dm2_v1_render_dialogue_box(DM2_V1_ViewportState *s);
void dm2_v1_render_dialogue_open_panel(DM2_V1_ViewportState *s);

/* ── GDAT-backed graphic fetch ─────────────────────────────────── */
/* Fetches a GDAT graphic as a decompressed 8-bit bitmap.
 * gdat_index: GDAT category<<8 | entry index (see dm2_v1_gfx_asset_loader.h)
 * Returns 0 on success, -1 if not found/not yet loaded.
 * Width/height filled for clipping. */
int dm2_v1_gfx_fetch(int gdat_index, const uint8_t **out_pixels,
                     int *out_w, int *out_h, int *out_stride);

/* ── Wall frame lookup ─────────────────────────────────────────── */
const DM2_WallFrame *dm2_v1_get_wall_frame(int view_square);

/* ── Source evidence ───────────────────────────────────────────── */
const char *dm2_v1_viewport_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H */
