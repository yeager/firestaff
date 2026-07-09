#ifndef FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H
#define FIRESTAFF_DM2_V1_VIEWPORT_RENDERER_H
#include <stdint.h>

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
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT 0x00
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D1C 0x00
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_D2C 0x01
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_OPENING_SHIFT 4
#define DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_MASK 0x0F
#define DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE (-0x400)
#define DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED 0x00
#define DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_PUSHED 0x05
#define DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE (-0x500)
#define DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_MASK 0xFF
#define DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE (-0x20000)
#define DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT 8
#define DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_MASK 0xFF
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

int dm2_v1_viewport_wall_field_for_square(int view_square);
int dm2_v1_viewport_wall_graphic_index_for_square(int view_square);
int dm2_v1_viewport_door_frame_field_for_square(int view_square);
int dm2_v1_viewport_door_frame_graphic_index_for_square(int view_square);
int dm2_v1_viewport_door_panel_field_for_square(int view_square);
int dm2_v1_viewport_door_panel_graphic_index_for_square(int view_square);
int dm2_v1_viewport_door_panel_graphic_index_for_record(int view_square,
                                                        int door_gfx_index,
                                                        int opening_dir);
int dm2_v1_viewport_door_button_field_for_state(int pushed);
int dm2_v1_viewport_door_button_graphic_index_for_state(int pushed);
int dm2_v1_viewport_skproject_cell_for_square(int view_square);
int dm2_v1_viewport_door_button_rectno_for_square(int view_square);
int dm2_v1_viewport_door_button_clickable_for_square(int view_square);
int dm2_v1_viewport_wall_button_graphic_index(int wall_gfx_index,
                                              int wall_gfx_field);
int dm2_v1_viewport_creature_graphic_index(int creature_type,
                                           int frame_index);
int dm2_v1_viewport_item_category_for_db_pool(int db_pool);
int dm2_v1_viewport_item_graphic_index(int item_category,
                                       int item_type,
                                       int frame_index);
int dm2_v1_viewport_projectile_graphic_index(int projectile_category,
                                             int projectile_type,
                                             int frame_index);
int dm2_v1_viewport_hud_portrait_graphic_index(int portrait_index);
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

#define DM2_V1_DOOR_RENDER_MAX DM2_SQ_COUNT

typedef struct {
    int view_square;
    int skproject_cell;
    int panel_gdat_index;
    int frame_gdat_index;
    int button_gdat_index;
    DM2_V1_ViewportRect panel_rect;
    DM2_V1_ViewportRect panel_visible_rect;
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect button_rect;
    uint8_t door_open_pct;
    uint8_t fallback_color;
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

typedef struct {
    DM2_V1_ViewportRect frame_rect;
    DM2_V1_ViewportRect fill_rect;
    uint8_t fill_color;
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
    uint8_t portrait_fill_color;
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
} DM2_CreatureSprite;

typedef struct {
    int creature_index;
    int creature_type;
    int frame_index;
    int direction;
    int depth;
    int center_x;
    int center_y;
    int gdat_index;
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
    int render_frame;
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

    /* Weather */
    int weather;               /* 0=clear, 1=rain, 2=fog, 3=storm */
    int rain_intensity;        /* 0–100 */

    /* Outdoor state */
    int is_outdoor;
    float time_of_day;         /* 0.0–1.0 */

    /* Rendering state */
    int dirty;                 /* 1=viewport needs full redraw */
    int tick_count;           /* frame counter for weather animation */
    uint32_t random_seed;      /* skproject RAND02-compatible render seed */

    DM2_V1_ViewportAssetFetch asset_fetch;
    void *asset_user;
    int asset_floor_ceiling_drawn_count;
    int fallback_floor_ceiling_drawn_count;
    int asset_wall_drawn_count;
    int fallback_wall_drawn_count;
    int asset_door_panel_drawn_count;
    int asset_door_frame_drawn_count;
    int asset_door_button_drawn_count;
    int fallback_door_drawn_count;
    int asset_creature_drawn_count;
    int fallback_creature_drawn_count;
    int asset_item_drawn_count;
    int fallback_item_drawn_count;
    int asset_creature_possession_item_drawn_count;
    int fallback_creature_possession_item_drawn_count;
    int asset_carried_item_drawn_count;
    int fallback_carried_item_drawn_count;
    int asset_projectile_drawn_count;
    int fallback_projectile_drawn_count;
    int asset_hud_portrait_drawn_count;
    int fallback_hud_portrait_drawn_count;
} DM2_V1_ViewportState;

/* ── Initialization ────────────────────────────────────────────── */
void dm2_v1_viewport_init(DM2_V1_ViewportState *s, uint8_t *framebuffer, int stride);
void dm2_v1_viewport_set_party(DM2_V1_ViewportState *s, int dir, int x, int y);
void dm2_v1_viewport_set_outdoor(DM2_V1_ViewportState *s, int is_outdoor);
void dm2_v1_viewport_set_level(DM2_V1_ViewportState *s, int level);
void dm2_v1_viewport_set_weather(DM2_V1_ViewportState *s, int weather, int rain_intensity);
void dm2_v1_viewport_set_time(DM2_V1_ViewportState *s, float time_of_day);
void dm2_v1_viewport_set_hud_party(DM2_V1_ViewportState *s,
                                   const DM2_V1_HudPartyState *party);
void dm2_v1_viewport_set_asset_provider(DM2_V1_ViewportState *s,
                                        DM2_V1_ViewportAssetFetch fetch,
                                        void *user);
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
