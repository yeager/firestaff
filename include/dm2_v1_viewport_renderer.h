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
    int16_t  sprite_depth;    /* depth sort key */
} DM2_ViewSquare;

#define DM2_MAX_CREATURES_PER_SQ  4
#define DM2_MAX_ITEMS_PER_SQ      8
#define DM2_MAX_PROJECTILES       16

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
} DM2_CreatureSprite;

typedef struct {
    uint8_t  item_type;       /* GDAT item index */
    uint8_t  frame_index;     /* animation frame */
    int16_t  depth;           /* depth sort key */
    int16_t  screen_x;        /* viewport X position */
    int16_t  screen_y;        /* viewport Y position */
} DM2_ItemSprite;

typedef struct {
    uint8_t  projectile_type; /* spell/arrow/bolt type */
    uint8_t  frame_index;     /* animation frame */
    int16_t  depth;           /* depth sort key */
    int16_t  screen_x;
    int16_t  screen_y;
    int16_t  velocity_x;      /* pixel velocity */
    int16_t  velocity_y;
    uint8_t  palette_shift;   /* light/color modifier */
} DM2_Projectile;

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
    DM2_Projectile projectiles[DM2_MAX_PROJECTILES];
    int projectile_count;

    /* Weather */
    int weather;               /* 0=clear, 1=rain, 2=fog, 3=storm */
    int rain_intensity;        /* 0–100 */

    /* Outdoor state */
    int is_outdoor;
    float time_of_day;         /* 0.0–1.0 */

    /* Rendering state */
    int dirty;                 /* 1=viewport needs full redraw */
    int tick_count;           /* frame counter for weather animation */
} DM2_V1_ViewportState;

/* ── Initialization ────────────────────────────────────────────── */
void dm2_v1_viewport_init(DM2_V1_ViewportState *s, uint8_t *framebuffer, int stride);
void dm2_v1_viewport_set_party(DM2_V1_ViewportState *s, int dir, int x, int y);
void dm2_v1_viewport_set_outdoor(DM2_V1_ViewportState *s, int is_outdoor);
void dm2_v1_viewport_set_level(DM2_V1_ViewportState *s, int level);
void dm2_v1_viewport_set_weather(DM2_V1_ViewportState *s, int weather, int rain_intensity);
void dm2_v1_viewport_set_time(DM2_V1_ViewportState *s, float time_of_day);

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