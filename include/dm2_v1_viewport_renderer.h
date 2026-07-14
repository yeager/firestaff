#ifndef FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H
#define FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H
#include <stdint.h>
#include "dm2_v1_weather_gdat.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"

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
#define DM2_V1_HUD_PORTRAIT_COUNT 8

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
#define DM2_V1_VIEWPORT_BLOCKED_MATERIAL_SCENE_CONTROL 0x400u
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
#define DM2_V1_VIEWPORT_GFX_WALL_GRAPHICSSET_BASE (-0xE00000)
#define DM2_V1_VIEWPORT_GFX_DOOR_FRAME_GRAPHICSSET_BASE (-0xF00000)
#define DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET 0x01
#define DM2_V1_GDAT_SCENE_FLAG_OUTDOOR 0x0020u

int dm2_v1_viewport_wall_field_for_square(int view_square);
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
    int door_record_type;
    int door_gfx_index;
    int door_opening_dir;
    int ornament_index;
    int door_button;
    int door_button_state;
    int panel_gdat_index;
    int ornate_gdat_index;
    int destroyed_mask_gdat_index;
    int frame_gdat_index;
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

typedef struct {
    DM2_V1_DoorRender doors[DM2_V1_DOOR_RENDER_MAX];
    int door_count;
} DM2_V1_DoorRenderPlan;

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
    int scene_control_presentation_stage;
    int creature_presentation_stage;
    int item_presentation_stage;
    int hud_presentation_stage;
    int door_command_consumed;
    int dungeon_ceiling_command_consumed;
    int dungeon_floor_command_consumed;
    int dungeon_wall_command_consumed;
    uint16_t dungeon_wall_material_required_mask;
    uint16_t dungeon_wall_material_consumed_mask;
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
    uint32_t palette_hash;
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
    char name[DM2_V1_HUD_CHAMPION_NAME_MAX + 1];
} DM2_V1_HudChampionState;

typedef struct {
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

int dm2_v1_viewport_door_panel_rect_for_square(int view_square,
                                               DM2_V1_ViewportRect *out_rect);
int dm2_v1_viewport_door_button_rect_for_square(int view_square,
                                                DM2_V1_ViewportRect *out_rect);

/* ── View square state ──────────────────────────────────────────── */
typedef struct {
    uint8_t  square_type;     /* 5-bit tile type */
    uint8_t  flags;           /* DM2_SquareFlags */
    uint8_t  wall_gfx_index;  /* GDAT wall graphic index */
    uint8_t  floor_gfx_index; /* GDAT floor graphic index */
    uint8_t  door_gfx_index;  /* GDAT door graphic index */
    uint8_t  ornament_index;   /* GDAT ornament/ornate index */
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
    uint8_t  health_pct;       /* 0–100 for health bar */
    uint8_t  light_radius;     /* light emitted by creature */
    uint8_t  direction;        /* 0=N, 1=E, 2=S, 3=W */
    uint8_t  source_kind;      /* 1=live runtime, 2=G1 DB4 record */
} DM2_CreatureSprite;

typedef struct {
    int creature_index;
    int creature_type;
    int source_kind;
    int frame_index;
    int direction;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
    int rect14_applied;
    int rect14_scale64;
    int rect14_lateral_offset;
    int rect14_flip_mirror;
    DM2_V1_ViewportRect fallback_rect;
    uint8_t fallback_color;
    DM2_V1_ViewportRect health_bg_rect;
    DM2_V1_ViewportRect health_fill_rect;
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
    uint8_t  item_category;   /* GDAT category, 0 = miscellaneous fallback */
    uint8_t  item_type;       /* GDAT item index */
    uint8_t  frame_index;     /* animation frame */
    int16_t  depth;           /* depth sort key */
    int16_t  screen_x;        /* viewport X position */
    int16_t  screen_y;        /* viewport Y position */
    uint8_t  direction;       /* source ObjectID::Dir(), for carried overlays */
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
    int flip_mirror;
    int fallback_radius;
    uint8_t fallback_color;
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
    uint8_t  projectile_category; /* GDAT category, 0 = spell-missile fallback */
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
    DM2_V1_ViewportDoorSurfaceViewFetch door_surface_view_fetch;
    void *door_surface_view_user;
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
    int gdat_scene_weather_consumed_count;
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
    /* skproject CHECK_RECOMPUTE_LIGHT: 0 disables the path; nonzero values
     * are bounded to eight walk-path steps. HIGHEST_LIGHT_LEVEL stays in
     * the original 0..5 global-light scale until a source palette route is
     * implemented. */
    uint8_t gdat_scene_light_floor;
    uint8_t gdat_scene_light_search_depth;
    int gdat_scene_light_recompute_enabled;
    int gdat_scene_material_index;
    int gdat_scene_material_consumed_count;
    int gdat_interface_palette_ready;
    int gdat_interface_palette_consumed_count;
    int gdat_material_palette_floor_ceiling_consumed_count;
    int gdat_material_palette_wall_consumed_count;
    int gdat_material_palette_door_frame_consumed_count;
    uint32_t gdat_interface_palette_hash;
    uint8_t gdat_interface_palette16[16];
    const uint8_t *gdat_interface_font_rows;
    uint32_t gdat_interface_font_hash;
    int gdat_interface_font_consumed_count;
    const DM2_V1_InterfaceHudLayout *gdat_interface_hud_layout;
    const uint8_t *gdat_interface_rect14_rows;
    uint32_t gdat_interface_rect14_row_count;
    uint32_t gdat_interface_rect14_hash;
    int gdat_interface_rect14_consumed_count;
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
    uint16_t last_dungeon_wall_material_required_mask;
    uint16_t last_dungeon_wall_material_consumed_mask;
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
    const DM2_V1_G1TextWallGfxRuntimeReceipt *g1_text_wall_gfx_materials;
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *g1_actuator_wall_gfx_materials;
    int asset_carried_item_drawn_count;
    int fallback_carried_item_drawn_count;
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
} DM2_V1_ViewportState;

/* ── Initialization ────────────────────────────────────────────── */
void dm2_v1_viewport_init(DM2_V1_ViewportState *s, uint8_t *framebuffer, int stride);
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
void dm2_v1_viewport_set_door_surface_view_provider(
    DM2_V1_ViewportState *s,
    DM2_V1_ViewportDoorSurfaceViewFetch fetch,
    void *user);
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
void dm2_v1_viewport_set_scene_map_load_token(
    DM2_V1_ViewportState *s, uint32_t source_map_load_token);
/* skproject SkWinCore map-load 44929-45030 initializes one active
 * GRAPHICSSET scene record. Bind its typed controls atomically; no image,
 * palette decode, or draw instruction is created here. */
int dm2_v1_viewport_bind_static_graphicsset_scene_record(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
int dm2_v1_viewport_bind_static_scene_light_control(
    DM2_V1_ViewportState *s,
    uint32_t source_map_load_token,
    uint32_t source_scene_control_hash);
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
/* skproject QUERY_FONT consumes the six 128-byte dt07/0 rows as 3x6 HUD
 * glyph pixels.  The caller retains ownership through the boot profile. */
void dm2_v1_viewport_set_gdat_interface_font(
    DM2_V1_ViewportState *s,
    const uint8_t *rows,
    uint32_t hash);
void dm2_v1_viewport_set_g1_creature_map_chip_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt);
void dm2_v1_viewport_set_g1_wall_gfx_materials(
    DM2_V1_ViewportState *s,
    const DM2_V1_G1TextWallGfxRuntimeReceipt *text_receipt,
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *actuator_receipt);
void dm2_v1_viewport_set_gdat_interface_hud_layout(
    DM2_V1_ViewportState *s,
    const DM2_V1_InterfaceHudLayout *layout);
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
void dm2_v1_render_walls(DM2_V1_ViewportState *s);
void dm2_v1_render_doors(DM2_V1_ViewportState *s);
void dm2_v1_render_creatures(DM2_V1_ViewportState *s);
void dm2_v1_render_items(DM2_V1_ViewportState *s);
void dm2_v1_render_creature_possession_items(DM2_V1_ViewportState *s);
void dm2_v1_render_carried_item(DM2_V1_ViewportState *s);
void dm2_v1_render_projectiles(DM2_V1_ViewportState *s);
void dm2_v1_render_weather_overlay(DM2_V1_ViewportState *s);
void dm2_v1_render_ui_chrome(DM2_V1_ViewportState *s);

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
