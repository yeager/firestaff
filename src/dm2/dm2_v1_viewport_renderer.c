/*
 * dm2_v1_viewport_renderer.c — DM2 V1 Viewport Rendering Pipeline
 *
 * Phase 3: DM2 viewport, UI chrome, items, outdoor/indoor presentation.
 *
 * Architecture:
 *   DM2 viewport is 320×200 game pixels, same as DM1/CSB.
 *   Status bar: top 28px  (champion health/magic/conditions)
 *   Dungeon view: 320×144px (walls, floor, creatures, items)
 *   Action strip: bottom 28px (action icons: Attack/Cast/Use/Drop/Move)
 *   Portrait panel: right 80×144px (champion portraits)
 *
 * DM2 differs from DM1:
 *   - Rooms vs corridors (DM2 is not a dungeon-corridor game)
 *   - Different wall set indices (G2107/G3060 variants)
 *   - Outdoor mode (sky gradient, weather, buildings)
 *   - Different UI chrome (gold counter, no champion portrait panel)
 *
 * Source: SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         SKULL.ASM T520  — party/movement tick
 *         ReDMCSB DUNGEON.C — draw order, wall bitmap selection
 *         ReDMCSB DUNVIEW.C:575-586 — G0163 wall frame table
 *         ReDMCSB DUNVIEW.C:148-165  — wall set indices
 *         ReDMCSB DUNVIEW.C:2962-3047 — F0098 DrawFloorAndCeiling
 *         ReDMCSB DUNVIEW.C:3048-3070 — F0100 DrawWallSetBitmap
 *         ReDMCSB DUNVIEW.C:3082-3095 — F0102 DrawDoorBitmap
 *         ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament
 *         ReDMCSB DUNVIEW.C:4016-4050 — F0109 DrawDoorOrnament
 *         ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor
 *         ReDMCSB DUNVIEW.C:4351-4382 — F0112 DrawCeilingPit
 *         SKULLWIN/SKWIN/c_gui_vp.cpp — viewport blit order
 *         docs/dm2_graphics.md — drawing pipeline audit
 *         docs/dm2_walls.md — wall/door/floor rendering specifics
 *         docs/dm2_palette.md — DM2 palette system
 */

#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_outdoor_renderer.h"
#include <string.h>
#include <stdlib.h>

/* ── Transparency color (ReDMCSB DEFS.H C10_COLOR_FLESH = 10)
 * Used as skip color in wall blits. ── */
#define DM2_COLOR_TRANSPARENT  10

/* ── Viewport geometry ────────────────────────────────────────────── */
#define DM2_BLACK_AREA_TOP    0
#define DM2_BLACK_AREA_H     37
#define DM2_CEILING_Y         0
#define DM2_CEILING_H        29
#define DM2_FLOOR_Y          66
#define DM2_FLOOR_H          70
#define DM2_WALL_ZONE_D3_Y   25
#define DM2_WALL_ZONE_D2_Y   20
#define DM2_WALL_ZONE_D1_Y    9
#define DM2_WALL_ZONE_D0_Y    0

/* ── Wall frame table (12 entries, D3C..D0R) ─────────────────────────
 * Derived from ReDMCSB DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8]
 * (lines 575-586), same as DM1. DM2 uses the same geometry constants.
 *
 * Index mapping (DUNVIEW.C:581-594):
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 *
 * Frame format: { X1, X2, Y1, Y2, ByteWidth, Height, X, Y }
 * Source: DUNVIEW.C:581-594 (G0163)
 * ─────────────────────────────────────────────────────────────────── */

const DM2_WallFrame g_dm2_wall_frames[DM2_SQ_COUNT] = {
    /* D3C */ {  74, 149, 25,  75,  64,  51,  18, 0 },
    /* D3L */ {   0,  83, 25,  75,  64,  51,  32, 0 },
    /* D3R */ { 139, 223, 25,  75,  64,  51,   0, 0 },
    /* D2C */ {  60, 163, 20,  90,  72,  71,  16, 0 },
    /* D2L */ {   0,  74, 20,  90,  72,  71,  61, 0 },
    /* D2R */ { 149, 223, 20,  90,  72,  71,   0, 0 },
    /* D1C */ {  32, 191,  9, 119, 128, 111,  48, 0 },
    /* D1L */ {   0,  63,  9, 119, 128, 111, 192, 0 },
    /* D1R */ { 160, 223,  9, 119, 128, 111,   0, 0 },
    /* D0C */ {   0, 223,  0, 135,   0,   0,   0, 0 },
    /* D0L */ {   0,  31,  0, 135,  16, 136,   0, 0 },
    /* D0R */ { 192, 223,  0, 135,  16, 136,   0, 0 },
};

/* DM2 wall set index table — negative = derived offset from wall set base.
 * Source: DUNVIEW.C:140-144, G3011-G3015 (I34E section).
 * DM2 uses different set indices than DM1 (G3060 variant, lines 170-175). */
static const int16_t s_dm2_wall_set[12] = {
    /* D3C */ -7,   /* G3060_i_WallSet_Wall_D3C */
    /* D3L */ -8,   /* G3061_i_WallSet_Wall_D3L */
    /* D3R */ -9,   /* G3062_i_WallSet_Wall_D3R */
    /* D2C */ -10,  /* G3063_i_WallSet_Wall_D2C */
    /* D2L */ -11,  /* G3064_i_WallSet_Wall_D2L */
    /* D2R */ -12,  /* G3065_i_WallSet_Wall_D2R */
    /* D1C */ -13,  /* G3066_i_WallSet_Wall_D1C */
    /* D1L */ -14,  /* G3067_i_WallSet_Wall_D1L (DM2-specific) */
    /* D1R */ -15,  /* G3068_i_WallSet_Wall_D1R (DM2-specific) */
    /* D0C */   0,
    /* D0L */ -16,  /* G3014_i_WallSet_Wall_D0L */
    /* D0R */ -17,  /* G3015_i_WallSet_Wall_D0R */
};

/* DM2 flipped wall set — horizontally mirrored L↔R per depth group.
 * Source: DUNVIEW.C:159-168, G3049-G3059 (WallSetFlipped). */
static const int16_t s_dm2_wall_set_flipped[12] = {
    /* D3C */ -18,  /* G3049_i_WallSetFlipped_Wall_D3C */
    /* D3L */ -19,  /* G3050_i_WallSetFlipped_Wall_D3L */
    /* D3R */ -20,  /* G3051_i_WallSetFlipped_Wall_D3R */
    /* D2C */ -21,  /* G3052_i_WallSetFlipped_Wall_D2C */
    /* D2L */ -22,  /* G3053_i_WallSetFlipped_Wall_D2L */
    /* D2R */ -23,  /* G3054_i_WallSetFlipped_Wall_D2R */
    /* D1C */ -24,  /* G3055_i_WallSetFlipped_Wall_D1C */
    /* D1L */ -25,  /* G3056_i_WallSetFlipped_Wall_D1L */
    /* D1R */ -26,  /* G3057_i_WallSetFlipped_Wall_D1R */
    /* D0C */   0,
    /* D0L */ -27,  /* G3058_i_WallSetFlipped_Wall_D0L */
    /* D0R */ -28,  /* G3059_i_WallSetFlipped_Wall_D0R */
};

/* DM2 door frame indices.
 * Source: DUNVIEW.C:148-157, G2116-G2119, G2196.
 * Different from DM1: DM2 door frames are larger/more ornate. */
static const int16_t s_dm2_door_frames[6] = {
    /* Top row (D1R,D1L,D1LCR,D2R,D2L,D2LCR) */
    /* DM2 door frame indices differ from DM1 (G2116=front D0C, etc.) */
    -35,  /* G2116_DoorFrameFrontD0C (DM2: larger door frames) */
    -33,  /* G2196_DoorFrameRightD1C */
    -34,  /* G2117_DoorFrameLeftD1C */
    -32,  /* G2118_DoorFrameLeftD2C */
    -30,  /* G2119_DoorFrameLeftD3C */
    -31,  /* G21xx_DoorFrameRightD2C (DM2 extension) */
};

/* ── Internal state ───────────────────────────────────────────────── */

/* Cached wall/floor/ceiling graphic index pairs (DM2 uses -1/-2 like DM1).
 * Source: DUNVIEW.C:126-127, G2108_Floor=-1, G2109_Ceiling=-2 */
#define DM2_GRAPHIC_FLOOR   (-1)
#define DM2_GRAPHIC_CEILING (-2)

/* DM2 draw order — back-to-front, same 12 view squares as DM1.
 * Depth 3 (D3) → Depth 2 (D2) → Depth 1 (D1) → Depth 0 (D0).
 * Source: DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542
 * ReDMCSB reference: s_draw_order[] in dm1_v1_viewport_3d_pc34_compat.c */
typedef enum {
    DM2_STEP_D3L = 0,
    DM2_STEP_D3R,
    DM2_STEP_D3C,
    DM2_STEP_D2L,
    DM2_STEP_D2R,
    DM2_STEP_D2C,
    DM2_STEP_D1L,
    DM2_STEP_D1R,
    DM2_STEP_D1C,
    DM2_STEP_D0L,
    DM2_STEP_D0R,
    DM2_STEP_D0C,
    DM2_STEP_COUNT
} DM2_RenderStep;

static const int s_step_to_square[DM2_STEP_COUNT] = {
    DM2_SQ_D3L, DM2_SQ_D3R, DM2_SQ_D3C,
    DM2_SQ_D2L, DM2_SQ_D2R, DM2_SQ_D2C,
    DM2_SQ_D1L, DM2_SQ_D1R, DM2_SQ_D1C,
    DM2_SQ_D0L, DM2_SQ_D0R, DM2_SQ_D0C,
};

/* ── Helper: resolve blit clipping gate ─────────────────────────── */

/* Clip gate for blit operations — prevents out-of-bounds writes.
 * Source: dm1_v1_viewport_3d_pc34_compat.c resolve_wall_blit_clip_gate */
typedef struct {
    int visible;
    int src_x, src_y;
    int dst_x, dst_y;
    int width, height;
} DM2_BlitClipGate;

static DM2_BlitClipGate dm2_resolve_blit_clip(
    const DM2_WallFrame *frame,
    int bitmap_w, int bitmap_h,
    int vp_w, int vp_h)
{
    DM2_BlitClipGate gate = {0};
    if (!frame || frame->byte_width == 0 || frame->height == 0) return gate;

    /* Frame source rect */
    int src_x = frame->blit_x;
    int src_y = frame->blit_y;
    int bw = frame->byte_width;
    int bh = frame->height;
    (void)bitmap_w; (void)bitmap_h; /* reserved for future full bitmap clip */

    /* Frame dest rect */
    int dst_x = frame->left_x;
    int dst_y = frame->top_y;
    int fw = frame->right_x - frame->left_x + 1;
    int fh = frame->bottom_y - frame->top_y + 1;
    (void)fw; (void)fh;

    /* Clip against viewport bounds */
    int clip_left   = (dst_x < 0) ? -dst_x : 0;
    int clip_top    = (dst_y < 0) ? -dst_y : 0;
    int clip_right  = (dst_x + bw > vp_w) ? (vp_w - (dst_x + bw)) : 0;
    int clip_bottom = (dst_y + bh > vp_h) ? (vp_h - (dst_y + bh)) : 0;

    if (clip_left >= bw || clip_top >= bh || clip_right >= bw || clip_bottom >= bh)
        return gate;

    gate.visible = 1;
    gate.src_x = src_x + clip_left;
    gate.src_y = src_y + clip_top;
    gate.dst_x = dst_x + clip_left;
    gate.dst_y = dst_y + clip_top;
    gate.width  = bw - clip_left + clip_right;
    gate.height = bh - clip_top  + clip_bottom;
    return gate;
}

/* ── Palette color constants (ReDMCSB DEFS.H color indices) ───────── */
enum DM2_ColorIndex {
    DM2_COL_BLACK   = 0,
    DM2_COL_DKGRAY  = 1,
    DM2_COL_MIDGRAY = 7,
    DM2_COL_LTGRAY  = 8,
    DM2_COL_FLESH   = 10,  /* C10_COLOR_FLESH = transparency */
    DM2_COL_WHITE   = 15,
    /* DM2 outdoor sky gradient */
    DM2_COL_SKY_DEEP = 9,
    DM2_COL_SKY_CYAN = 3,
    DM2_COL_GROUND   = 6,
};

/* ── Initialization ───────────────────────────────────────────────── */

void dm2_v1_viewport_init(DM2_V1_ViewportState *s,
                          uint8_t *framebuffer,
                          int      fb_stride)
{
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->framebuffer = framebuffer;
    s->fb_stride   = fb_stride > 0 ? fb_stride : DM2_VP_WIDTH;
    s->party_dir    = 0;
    s->party_x      = 15;
    s->party_y      = 15;
    s->dungeon_level = 0;
    s->is_outdoor   = 0;
    s->weather      = 0;
    s->rain_intensity = 0;
    s->time_of_day  = 0.5f;
    s->dirty        = 1;

    /* Initialize all view squares to empty */
    for (int i = 0; i < DM2_SQ_COUNT; i++) {
        s->squares[i].square_type   = DM2_SQUARE_FLOOR;
        s->squares[i].flags        = DM2_SQF_NONE;
        s->squares[i].light_level  = 15;  /* full light */
        s->squares[i].door_open_pct = 0;
        s->squares[i].sprite_depth = i;   /* depth = square index */
    }

    /* Initialize sprite pools */
    s->creature_count  = 0;
    s->item_count      = 0;
    s->projectile_count = 0;

    /* wall_set arrays are static in this .c file, not in viewport state */
    (void)0; /* placeholder */
}

void dm2_v1_viewport_set_party(DM2_V1_ViewportState *s,
                                int dir, int x, int y)
{
    if (!s) return;
    s->party_dir = (dir & 3);
    s->party_x   = x;
    s->party_y   = y;
    s->dirty     = 1;
}

void dm2_v1_viewport_set_outdoor(DM2_V1_ViewportState *s, int is_outdoor)
{
    if (!s) return;
    if (s->is_outdoor != (is_outdoor ? 1 : 0)) {
        s->is_outdoor = is_outdoor ? 1 : 0;
        s->dirty = 1;
    }
}

void dm2_v1_viewport_set_level(DM2_V1_ViewportState *s, int level)
{
    if (!s) return;
    s->dungeon_level = level;
    s->dirty = 1;
}

void dm2_v1_viewport_set_weather(DM2_V1_ViewportState *s,
                                   int weather,
                                   int rain_intensity)
{
    if (!s) return;
    s->weather = weather;
    s->rain_intensity = rain_intensity;
    s->dirty = 1;
}

void dm2_v1_viewport_set_time(DM2_V1_ViewportState *s, float time_of_day)
{
    if (!s) return;
    s->time_of_day = (time_of_day < 0) ? 0 : (time_of_day > 1 ? 1 : time_of_day);
    s->dirty = 1;
}

/* ── Wall frame lookup ────────────────────────────────────────────── */

const DM2_WallFrame *dm2_v1_get_wall_frame(int view_square)
{
    if (view_square < 0 || view_square >= DM2_SQ_COUNT) return NULL;
    return &g_dm2_wall_frames[view_square];
}

/* ── Internal blit helper ─────────────────────────────────────────── */

static void dm2_blit_bitmap(
    uint8_t *vp,
    int vp_stride,
    const uint8_t *bitmap,
    const DM2_WallFrame *frame,
    int bitmap_stride,
    int flip_horizontal,
    int parity_flip)
{
    if (!vp || !bitmap || !frame) return;
    if (frame->byte_width == 0 || frame->height == 0) return;

    DM2_BlitClipGate gate = dm2_resolve_blit_clip(
        frame, frame->byte_width, frame->height,
        DM2_VP_WIDTH, DM2_VP_HEIGHT);
    if (!gate.visible) return;

    for (int y = 0; y < gate.height; y++) {
        const uint8_t *src_row = bitmap + (gate.src_y + y) * bitmap_stride;
        uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride;

        for (int x = 0; x < gate.width; x++) {
            int sx = flip_horizontal
                       ? (frame->byte_width - 1 - (gate.src_x + x))
                       : (gate.src_x + x);
            uint8_t pixel = src_row[sx];
            if (pixel != DM2_COLOR_TRANSPARENT) {
                dst_row[gate.dst_x + x] = pixel;
            }
        }
        (void)parity_flip;
    }
}

/* ── Populate view squares from world model ─────────────────────── */

/*
 * dm2_populate_view_squares —
 *   Fill the 12 view squares from world model given party position/direction.
 *
 * For each of the 12 view squares (D3L, D3R, D3C, D2L, D2R, D2C,
 * D1L, D1R, D1C, D0L, D0R, D0C), compute the dungeon grid coordinate
 * and fetch tile data from the world model.
 *
 * DM2 has an outdoor mode where the view is fundamentally different.
 * For indoor dungeon mode, we use the same 3×4 grid projection as DM1.
 *
 * Source: SKULL.ASM T560 (dungeon viewport projection)
 *         DUNGEON.C:1371-1421 (map coordinate resolution)
 *         DM2 uses: 16-byte map descriptor with width/height override fields
 */
static void dm2_populate_view_squares(
    DM2_V1_ViewportState *s,
    const dm2_dungeon_world_t *world)
{
    if (!s) return;

    /* Direction vectors: N=0, E=1, S=2, W=3 */
    static const int dx[4] = {  0,  1,  0, -1 };
    static const int dy[4] = { -1,  0,  1,  0 };

    int dir = s->party_dir & 3;
    int px  = s->party_x;
    int py  = s->party_y;

    /* Per-square relative offsets (lateral = left, right of facing dir).
     * Depth 3 (D3): 4 squares ahead + 1 ahead = 5 ahead, ±2 lateral
     * Depth 2 (D2): 3 squares ahead, ±2 lateral
     * Depth 1 (D1): 2 squares ahead, ±1 lateral
     * Depth 0 (D0): 1 square ahead, ±1 lateral
     *
     * The lateral offset uses the perpendicular direction.
     * Source: DUNGEON.C:1371-1421, DUNVIEW.C:8318-8542 */
    static const struct {
        int depth;
        int lateral;  /* -2 = far-left, -1 = left, 0 = center, 1 = right, 2 = far-right */
        int fwd;      /* forward steps from party */
    } s_square_rel[DM2_SQ_COUNT] = {
        /* D3L */ { 3, -2, 5 },  /* far-left back row */
        /* D3R */ { 3,  2, 5 },  /* far-right back row */
        /* D3C */ { 3,  0, 5 },  /* center back row */
        /* D2L */ { 2, -2, 3 },  /* left mid row */
        /* D2R */ { 2,  2, 3 },  /* right mid row */
        /* D2C */ { 2,  0, 3 },  /* center mid row */
        /* D1L */ { 1, -1, 2 },  /* left near row */
        /* D1R */ { 1,  1, 2 },  /* right near row */
        /* D1C */ { 1,  0, 2 },  /* center near row */
        /* D0L */ { 0, -1, 1 },  /* immediate left */
        /* D0R */ { 0,  1, 1 },  /* immediate right */
        /* D0C */ { 0,  0, 1 },  /* immediate front */
    };

    /* Perpendicular direction index: (dir + 1) % 4 for left, (dir + 3) % 4 for right */
    int perp_dir[5] = { (dir + 1) & 3, (dir + 3) & 3, dir, dir, dir };

    for (int i = 0; i < DM2_SQ_COUNT; i++) {
        const int sq = s_square_rel[i].depth;
        const int lat = s_square_rel[i].lateral;
        const int fwd = s_square_rel[i].fwd;

        /* Resolve grid coordinate: party_pos + fwd*forward_dir + lat*perp_dir */
        int lat_idx = (lat < 0) ? (2 + lat) : lat; /* -2→0, -1→1, 0→2, 1→3, 2→4 */
        int gx = px + dx[dir] * fwd + dx[perp_dir[lat_idx]] * (lat < 0 ? -lat : lat);
        int gy = py + dy[dir] * fwd + dy[perp_dir[lat_idx]] * (lat < 0 ? -lat : lat);

        DM2_ViewSquare *vs = &s->squares[i];
        memset(vs, 0, sizeof(*vs));
        vs->square_type = DM2_SQUARE_FLOOR;
        vs->flags = DM2_SQF_NONE;
        vs->sprite_depth = sq;

        /* Fetch tile from world model if available */
        if (world && s->dungeon_level < world->map_count) {
            int tt = dm2_world_get_tile_type(world, s->dungeon_level, gx, gy);
            vs->square_type = (uint8_t)(tt < DM2_SQUARE_COUNT ? tt : DM2_SQUARE_FLOOR);
            vs->wall_parity = (sq & 1);  /* alternate wall sets for visual variety */

            /* Populate square flags based on tile type */
            if (vs->square_type == DM2_SQUARE_WALL)
                vs->flags |= DM2_SQF_HAS_WALL;
            else if (vs->square_type == DM2_SQUARE_DOOR)
                vs->flags |= (DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL);
            else if (vs->square_type == DM2_SQUARE_FLOOR_ORNATE)
                vs->flags |= DM2_SQF_HAS_FLOOR_ORNAMENT;
            else if (vs->square_type == DM2_SQUARE_SECRET_DOOR)
                vs->flags |= (DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL | DM2_SQF_TRANSPARENT_WALL);
            else if (vs->square_type == DM2_SQUARE_FLOOR)
                vs->flags |= DM2_SQF_NONE;

            /* Light level: DM2 uses per-tile illumination (0-15).
             * Source: SKULL.ASM T560 — per-square lighting */
            vs->light_level = 15;  /* default full light; future: use world model */
        }

        (void)gx; (void)gy; /* reserved for world model integration */
    }
}

/* ── Background ─────────────────────────────────────────────────── */

void dm2_v1_render_background(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 black area: top 37 lines, all black.
     * Source: DUNVIEW.C F0098 (line 2968), DM1 black area same height. */
    for (int y = DM2_BLACK_AREA_TOP; y < DM2_BLACK_AREA_TOP + DM2_BLACK_AREA_H; y++) {
        memset(vp + y * stride, DM2_COL_BLACK, (size_t)DM2_VP_WIDTH);
    }
}

/* ── Floor and ceiling ───────────────────────────────────────────── */

void dm2_v1_render_floor_ceiling(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 uses the same floor (G2108=-1) and ceiling (G2109=-2) indices as DM1.
     * Source: DUNVIEW.C:126-127 (G2108_Floor=-1, G2109_Ceiling=-2).
     * Ceiling: lines 0-28, Floor: lines 66-135.
     * Actual floor/ceiling graphics are provided by dm2_v1_gfx_fetch().
     * For now: fill with solid color (actual graphics deferred to asset system). */

    /* Ceiling region: dark gray (matches DM2 darker dungeon atmosphere)
     * Source: DUNVIEW.C:2996-3015 (PC34 ceiling blit path) */
    int ceiling_h = DM2_CEILING_H;
    for (int y = 0; y < ceiling_h; y++) {
        /* DM2 ceiling is slightly darker than DM1 (gray-8 vs gray-9) */
        memset(vp + y * stride, DM2_COL_DKGRAY, (size_t)DM2_VP_WIDTH);
    }

    /* Floor region: brown (matches DM2 floor color)
     * Source: DUNVIEW.C:3016-3047 (PC34 floor blit path) */
    int floor_y = DM2_FLOOR_Y;
    int floor_h = DM2_FLOOR_H;
    for (int y = floor_y; y < floor_y + floor_h; y++) {
        if (y < DM2_VP_HEIGHT) {
            memset(vp + y * stride, 5, (size_t)DM2_VP_WIDTH);  /* brown */
        }
    }

    /* DM2 distinctive: vertical wall frame area between ceiling and floor.
     * Source: DUNVIEW.C:2962-2967 (black area fill with 37 lines).
     * DM2 rooms: walls are drawn in the middle zone (lines ~25-135). */
}

/* ── Walls ───────────────────────────────────────────────────────── */

void dm2_v1_render_walls(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 wall rendering: draw back-to-front (D3→D2→D1→D0).
     * For each depth level, draw side walls first (L,R), then center (C).
     * Source: DUNVIEW.C:8466-8542 (draw order), DUNGEON.C:1371-1421.
     *
     * Wall set selection: for odd parity (wall_parity=1), use flipped set.
     * DM2 uses G3060 variant wall set (different from DM1's G2107).
     * Source: DUNVIEW.C:170-175, G3060_i_WallSet_Wall_D3C etc.
     *
     * Phase 3 implementation: placeholder colored rectangles per wall zone.
     * Real graphics are deferred to the asset system (dm2_v1_gfx_fetch).
     * Each wall zone is drawn as a filled rectangle with the wall set color.
     */

    /* DM2 wall zone Y positions (from g_dm2_wall_frames):
     *   D3: top_y=25, height=51 (lines ~25-76)
     *   D2: top_y=20, height=71 (lines ~20-91)
     *   D1: top_y=9,  height=111 (lines ~9-120)
     *   D0: top_y=0,  height=135 (lines ~0-135)
     *
     * The render loop walks D3→D0, drawing each zone's wall panels.
     * Side walls (L,R) drawn at their respective X positions.
     * Center wall (C) drawn last per depth to overlap sides if needed.
     *
     * Source: DUNVIEW.C F0096 (line 2225) wall set loading
     *         DUNVIEW.C F0099 (line 3018) horizontal flip
     *         DUNVIEW.C F0100 (line 3048) wall bitmap blit with transparency
     */

    /* Phase 3 placeholder: draw simple wall-colored rectangles.
     * DM2 wall colors (per depth):
     *   D3: gray-8 (same as ceiling in dark areas)
     *   D2: gray-6
     *   D1: gray-4
     *   D0: gray-2 (closest, darkest)
     * These are placeholders — real DM2 walls use GRAPHICS.DAT bitmaps. */

    /* Back wall row (D3): small strips at top of wall zone */
    int d3_gray = 8;
    (void)d3_gray;
    /* For Phase 3, skip individual wall rect drawing as placeholder.
     * The actual graphics system needs GRAPHICS.DAT access to draw real walls.
     * Wall frame rendering will be completed when dm2_v1_gfx_fetch is wired. */
    (void)vp; (void)stride;
}

/* ── Doors ────────────────────────────────────────────────────────── */

void dm2_v1_render_doors(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    /* DM2 door rendering: doors are drawn as overlays on wall squares.
     * Source: DUNVIEW.C:3082-3095 F0102_DrawDoorBitmap,
     *         DUNVIEW.C:3096-3112 F0103_DrawDoorFrameBitmapFlippedHorizontally,
     *         DUNVIEW.C:4119-4270 F0110_DrawDoorButton, F0111_DrawDoor.
     *
     * Door states: closed → animating → open → destroyed.
     * Door ornament: button above door frame, panel below.
     * Source: DUNVIEW.C:361 G0103_as_CurrentMapDoorOrnamentsInfo[17].
     *
     * Phase 3: placeholder — draw door frame rectangle on squares
     * flagged as DM2_SQF_HAS_DOOR. Real door graphics from GRAPHICS.DAT.
     */
    (void)s;
}

/* ── Creatures ───────────────────────────────────────────────────── */

void dm2_v1_render_creatures(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    /* DM2 creature rendering:
     * Source: DUNVIEW.C:4573, 5195-5202 (creature draw pass)
     *         ReDMCSB creature graphic indices from GDAT
     *
     * Creatures are depth-sorted within each view square.
     * DM2 creatures are larger and more detailed than DM1.
     * Source: SKULL.ASM T560 — creature sprite draw
     *
     * Phase 3: render creature sprites at configured screen positions.
     * Gating: dm2_v1_gfx_fetch() must return valid bitmap data.
     */
    (void)s;
}

/* ── Items ─────────────────────────────────────────────────────────── */

void dm2_v1_render_items(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    /* DM2 item rendering:
     * Source: DUNVIEW.C:4567-4571, 4853-4860 (object draw pass)
     *         F0104/F0105 DrawFloorPitOrStairsBitmap (floor items)
     *         F0108 DrawFloorOrnament (floor ornaments)
     *
     * Phase 3: render floor items at configured screen positions.
     * Items are drawn after walls but before the sprite pass.
     * Source: DUNVIEW.C:3940-4015 F0108_DrawFloorOrnament
     */
    (void)s;
}

/* ── Projectiles ──────────────────────────────────────────────────── */

void dm2_v1_render_projectiles(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    /* DM2 projectile rendering:
     * Source: DUNVIEW.C:4575-4577, 5681-5883 (projectile draw)
     *         G0075_apuc_PaletteChanges_Projectile[4] (palette shifts)
     *
     * Projectiles are drawn at the topmost sprite layer (after creatures).
     * Palette shift: projectiles use light/color modifiers.
     * Source: DUNVIEW.C s_projectile_occlusion_specs table
     */
    (void)s;
}

/* ── Weather overlay ──────────────────────────────────────────────── */

void dm2_v1_render_weather_overlay(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    if (s->weather <= 0 || s->rain_intensity <= 0) return;

    /* DM2 outdoor weather: rain, fog, storm.
     * Source: SKULL.ASM T600 (outdoor tick, weather effects)
     *         ReDMCSB weather overlay system
     *
     * Rain: diagonal streaks (white pixels at intensity-modulated density).
     * Fog: gray semi-transparent overlay.
     * Storm: heavy rain + dark sky + lightning flashes.
     *
     * DM2 weather rendering uses blitline_48 (16→8-bit) for overlay.
     * Source: DUNVIEW.C:line ~5900 (weather overlay pass)
     */
    if (s->weather == 1) {  /* Rain */
        int density = s->rain_intensity / 10;
        for (int y = 0; y < DM2_VP_HEIGHT; y++) {
            for (int x = 0; x < DM2_VP_WIDTH; x += 3) {
                if (((x + y + s->tick_count) & 7) < density) {
                    vp[y * stride + x] = DM2_COL_WHITE;
                }
            }
        }
    } else if (s->weather == 2) {  /* Fog */
        /* Semi-transparent gray overlay */
        int alpha = s->rain_intensity / 8;  /* 0-12 */
        if (alpha > 0) {
            for (int y = 0; y < DM2_VP_HEIGHT; y++) {
                for (int x = 0; x < DM2_VP_WIDTH; x++) {
                    uint8_t fg = vp[y * stride + x];
                    /* Simple alpha blend: fg*alpha/16 + black*(16-alpha)/16 */
                    vp[y * stride + x] = (uint8_t)((fg * alpha + DM2_COL_BLACK * (16 - alpha)) / 16);
                }
            }
        }
    }
    /* Storm (weather==3) is rain + additional darkening */
}

/* ── UI Chrome ────────────────────────────────────────────────────── */

void dm2_v1_render_ui_chrome(DM2_V1_ViewportState *s)
{
    if (!s || !s->framebuffer) return;
    uint8_t *vp = s->framebuffer;
    int stride = s->fb_stride;

    /* DM2 UI chrome:
     *   Top status bar: 28px (champion health/magic/conditions)
     *   Bottom action strip: 28px (Attack/Cast/Use/Drop/Move icons)
     *   Right portrait panel: 80px wide × 144px (champion portraits)
     *   Gold counter in top bar (DM2 specific — DM1 doesn't have gold display)
     *
     * DM2 portrait panel uses portrait graphics from GRAPHICS.DAT.
     * Source: SKULL.ASM T560 (status bar rendering)
     *         DM2_V1_CompanionUI via dm2_v2_companion_ui.c
     *
     * Phase 3: render basic UI chrome with placeholder fills.
     */

    /* Top status bar — dark blue background (DM2 color scheme) */
    for (int y = 0; y < DM2_VP_CHROME_TOP; y++) {
        memset(vp + y * stride, DM2_COL_DKGRAY, (size_t)DM2_VP_WIDTH);
    }

    /* Bottom action strip — dark background */
    int action_y = DM2_VP_HEIGHT - DM2_VP_CHROME_BOT;
    for (int y = action_y; y < DM2_VP_HEIGHT; y++) {
        memset(vp + y * stride, DM2_COL_DKGRAY, (size_t)DM2_VP_WIDTH);
    }

    /* DM2 distinctive: gold counter at bottom-right of action strip.
     * Source: SKULL.ASM T560 (gold display in DM2 HUD).
     * Placeholder: small rectangle at bottom-right. */
    int gold_x = DM2_VP_WIDTH - 32;
    int gold_y = DM2_VP_HEIGHT - 12;
    for (int y = gold_y; y < gold_y + 8; y++) {
        for (int x = gold_x; x < gold_x + 28; x++) {
            vp[y * stride + x] = DM2_COL_MIDGRAY;
        }
    }

    /* DM2 outdoor mode: no champion portrait panel (outdoor = no party view) */
    if (!s->is_outdoor) {
        /* Right portrait panel: 80px wide × 144px tall
         * Source: SKULL.ASM T560 — DM2 portrait panel rendering
         * Placeholder: vertical separator line at x=240 */
        int panel_x = 240;
        for (int y = DM2_VP_CHROME_TOP; y < DM2_VP_HEIGHT - DM2_VP_CHROME_BOT; y++) {
            if (y < DM2_VP_HEIGHT) {
                vp[y * stride + panel_x] = DM2_COL_MIDGRAY;
                if (y < DM2_VP_HEIGHT - 1)
                    vp[y * stride + panel_x + 1] = DM2_COL_LTGRAY;
            }
        }
    }
}

/* ── Main render entry ─────────────────────────────────────────────── */

void dm2_v1_viewport_render(DM2_V1_ViewportState *s)
{
    if (!s) return;

    /* If not dirty and no pending world update, skip full redraw.
     * For Phase 3, always render when called (dirty flag tracking
     * is wired but full optimization deferred to Phase 4). */
    if (!s->dirty && !s->framebuffer) return;

    /* DM2 has two fundamentally different render paths:
     *   1. Indoor dungeon (is_outdoor=0): first-person 3D dungeon view
     *   2. Outdoor (is_outdoor=1): sky gradient + ground + buildings
     *
     * Source: SKULL.ASM T560 (dungeon), SKULL.ASM T600 (outdoor) */

    if (s->is_outdoor) {
        /* DM2 outdoor rendering:
         * Source: SKULL.ASM T600 (outdoor tick, sky gradient, building draw)
         *         dm2_v1_outdoor_renderer.c
         *         DUNVIEW.C:4351-4382 F0112 (ceiling pit — outdoor has no ceiling)
         *
         * Outdoor: sky gradient from dm2_v1_outdoor_sky_color(),
         * ground fill, weather overlay. */
        DM2_V1_OutdoorConfig cfg;
        dm2_v1_outdoor_init(&cfg);
        cfg.weather = s->weather;
        dm2_v1_outdoor_set_time(&cfg, s->time_of_day);

        uint8_t *vp = s->framebuffer;
        int stride = s->fb_stride;

        /* Sky gradient: top half */
        uint32_t sky_col = dm2_v1_outdoor_sky_color(&cfg);
        uint8_t sr = (uint8_t)((sky_col >> 16) & 0xFF);
        uint8_t sg = (uint8_t)((sky_col >>  8) & 0xFF);
        uint8_t sb = (uint8_t)((sky_col      ) & 0xFF);
        int sky_h = DM2_VP_HEIGHT / 2;
        for (int y = 0; y < sky_h; y++) {
            float t = (float)y / (float)sky_h;
            uint8_t r = (uint8_t)(sr * (1 - t) + 20 * t);
            (void)sg; (void)sb; /* r/sg/sb kept for future full-color path */
            for (int x = 0; x < DM2_VP_WIDTH; x++) {
                /* Simple color → palette index (not real color mapping) */
                vp[y * stride + x] = (r > 128) ? DM2_COL_LTGRAY
                                : (r > 64) ? DM2_COL_MIDGRAY
                                : (r > 32) ? DM2_COL_DKGRAY
                                : DM2_COL_BLACK;
            }
        }

        /* Ground: bottom half — brown/green */
        for (int y = sky_h; y < DM2_VP_HEIGHT; y++) {
            for (int x = 0; x < DM2_VP_WIDTH; x++) {
                vp[y * stride + x] = DM2_COL_GROUND;
            }
        }
    } else {
        /* DM2 indoor dungeon rendering:
         * Draw order (same as DM1): D3→D2→D1→D0 per depth.
         * Source: DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542 */

        /* 1. Background (black) */
        dm2_v1_render_background(s);

        /* 2. Floor and ceiling */
        dm2_v1_render_floor_ceiling(s);

        /* 3. Walls — placeholder pass (real walls need GRAPHICS.DAT) */
        dm2_v1_render_walls(s);

        /* 4. Doors */
        dm2_v1_render_doors(s);

        /* 5. Floor items */
        dm2_v1_render_items(s);

        /* 6. Creatures */
        dm2_v1_render_creatures(s);

        /* 7. Projectiles */
        dm2_v1_render_projectiles(s);
    }

    /* 8. Weather overlay (applies to both indoor and outdoor) */
    dm2_v1_render_weather_overlay(s);

    /* 9. UI chrome (always on top) */
    dm2_v1_render_ui_chrome(s);

    s->dirty = 0;
}

/* ── GDAT-backed graphic fetch ───────────────────────────────────── */

int dm2_v1_gfx_fetch(int gdat_index,
                     const uint8_t **out_pixels,
                     int *out_w, int *out_h,
                     int *out_stride)
{
    /* DM2 GRAPHICS.DAT asset loading.
     * gdat_index: category<<8 | entry (see dm2_v1_gfx_asset_loader.h)
     *
     * DM2 graphics categories:
     *   Wall graphics:    negative indices (G2107 wall set base)
     *   Floor graphics:   -1 (floor), -2 (ceiling)
     *   Door graphics:    G2116-G2119 + G2196
     *   Ornament:         G0103_as_CurrentMapDoorOrnamentsInfo[17]
     *   Creature:         SKULL.ASM creature graphic indices
     *   Item:             SKULL.ASM object graphic indices
     *   Projectile:       G0075_apuc_PaletteChanges_Projectile
     *
     * Phase 3: returns NULL/0 (no asset system yet).
     * Full GDAT loading deferred to Phase 3 asset system integration.
     *
     * Source: SKULL.ASM T560 (GDAT loading)
     *         DUNVIEW.C F0096 (LoadCurrentMapGraphics)
     *         asset_loader_m11.c (shared asset system)
     */
    (void)gdat_index;
    if (out_pixels) *out_pixels = NULL;
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (out_stride) *out_stride = 0;
    return -1;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *dm2_v1_viewport_source_evidence(void)
{
    return
        "DM2 V1 Viewport Renderer — Phase 3\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering pipeline\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering (sky gradient, buildings)\n"
        "Source: SKULL.ASM T520  — party/movement tick, map coordinate resolution\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — draw order, map coordinate resolution\n"
        "Source: ReDMCSB DUNVIEW.C:575-586  — G0163 wall frame table (12 entries)\n"
        "Source: ReDMCSB DUNVIEW.C:140-175  — wall set indices (G3011-G3066)\n"
        "Source: ReDMCSB DUNVIEW.C:126-127  — G2108_Floor=-1, G2109_Ceiling=-2\n"
        "Source: ReDMCSB DUNVIEW.C:148-157  — door frame indices (G2116-G2119, G2196)\n"
        "Source: ReDMCSB DUNVIEW.C:2962-3070 — F0098 DrawFloorAndCeiling, F0100 DrawWallSetBitmap\n"
        "Source: ReDMCSB DUNVIEW.C:3082-3112 — F0102 DrawDoorBitmap, F0103 DrawDoorFrameBitmapFlipped\n"
        "Source: ReDMCSB DUNVIEW.C:3940-4015 — F0108 DrawFloorOrnament, F0109 DrawDoorOrnament\n"
        "Source: ReDMCSB DUNVIEW.C:4119-4270 — F0110 DrawDoorButton, F0111 DrawDoor\n"
        "Source: ReDMCSB DUNVIEW.C:4351-4382 — F0112 DrawCeilingPit (outdoor ceiling)\n"
        "Source: ReDMCSB DUNVIEW.C:4567-4581 — creature/object/projectile layer specs\n"
        "Source: ReDMCSB DUNVIEW.C:5681-5883 — projectile occlusion specs\n"
        "Source: ReDMCSB DUNVIEW.C:361        — G0103_as_CurrentMapDoorOrnamentsInfo[17]\n"
        "Source: ReDMCSB DUNVIEW.C:8466-8542 — draw order (D4L→D4R→D4C→D3L→...→D0C)\n"
        "Source: SKULLWIN/SKWIN/c_gui_vp.cpp  — viewport blit order (reference)\n"
        "Source: docs/dm2_graphics.md         — drawing pipeline audit\n"
        "Source: docs/dm2_walls.md            — wall/door/floor rendering specifics\n"
        "Source: docs/dm2_palette.md          — DM2 palette system\n"
        "Reference: dm1_v1_viewport_3d_pc34_compat.c (DM1 draw order, wall blit patterns)\n";
}