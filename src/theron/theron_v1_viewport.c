/*
 * theron_v1_viewport.c — Theron's Quest V1 Phase 4: Rendering Pipeline
 *
 * Implementations for:
 *   1. Theron viewport rendering (PC Engine 256×224 planar framebuffer)
 *   2. UI chrome rendering (top bar, right panel, bottom champion slots, message)
 *   3. Asset selection wiring (TQR tile/palette system from Track 02)
 *   4. Planar-to-M11 framebuffer presentation
 *
 * Architecture mirrors nexus_v1_viewport.c:
 *   - Local planar framebuffer (indexed pixels)
 *   - View cone rendering (D0..D3 depth, left/center/right columns)
 *   - TQR tile/palette system for dungeon graphics
 *   - UI chrome composited from world state
 *
 * Source references:
 *   THQUEST.ASM T400   — tile bank loading
 *   THQUEST.ASM T520   — dungeon viewport tile selection
 *   THQUEST.ASM T600   — UI overlay zones
 *   HuC6260/HuC6270 datasheet — VDC/VCE rendering
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §7
 */

#include "theron_v1_viewport.h"
#include "theron_v1_palette.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Compile-time constants ─────────────────────────────────────── */

_Static_assert(TQR_FB_W == 256, "Planar framebuffer width must be 256");
_Static_assert(TQR_FB_H == 224, "Planar framebuffer height must be 224");

/* ── Direction helpers ───────────────────────────────────────────── */

static const int8_t g_dir_dx[4] = { 0,  1,  0, -1};
static const int8_t g_dir_dy[4] = {-1,  0,  1,  0};
/* Left-vector of each direction (perpendicular) */
static const int8_t g_left_dx[4] = {-1,  0,  1,  0};
static const int8_t g_left_dy[4] = { 0, -1,  0,  1};

/* ── Tile index tables ───────────────────────────────────────────── */
/*
 * Deterministic tile selection per square type + depth.
 * Index: tile_index = g_tile_table[square_type][depth][is_wall]
 *
 * tile_index meanings:
 *   >= 0:  tile index into TQR_PaletteState tile atlas
 *   -1:    flat-color fallback (palette entry 7 mid-gray)
 *
 * Source: THQUEST.ASM T520 — tile bank loading uses dungeon_seed to
 * select which tile sub-bank to use.  Here we use the simplified
 * deterministic mapping: floor/wall type + distance → tile index.
 *
 * Tile bank layout (from T520 and Track 02 format docs):
 *   tiles 0-127:    wall tiles (2bpp, palette group 0)
 *   tiles 128-255:  floor tiles (2bpp, palette group 0)
 *   tiles 256-383:  object tiles (2bpp, palette group 2)
 *   tiles 384-511:  creature tiles (4bpp, palette group 1)
 *   tiles 512-639:  UI tile set (2bpp, palette group 3)
 *   tiles 640-767:  font tiles (2bpp, palette group 4)
 *
 * For squares at depth d in direction dir, the wall tile index
 * formula:  base_wall + (d % 4) * 8 + side_offset
 * For floors: base_floor + (d % 4) * 8
 */

#define TILE_FALLBACK  (-1)

/* Square type → base tile index table.
 * Layout: [THERON_SQUARE_MAX][TQR_VP_DEPTH][2]
 * Last dimension: [not_wall=0, is_wall=1]
 *
 * Key:
 *   WALL    = 0  → wall tile
 *   FLOOR   = 1  → floor tile
 *   DOOR    = 4  → door tile (uses wall tile when closed, floor when open)
 *   PIT     = 2  → floor tile (pit trap)
 *   STAIRS  = 3,13 → stairs up/down tile
 *   TELEPORT= 5  → floor tile (teleporter pad)
 *   ALARM   = 6  → floor tile
 *   EXIT    = 8  → exit portal tile
 *   TRIGGER = 9  → floor tile
 *   POOL    = 10 → pool tile
 *   SECRET  = 11 → wall tile (hidden door)
 */
static const int g_tile_table[16][TQR_VP_DEPTH][2] = {
    /* 0: WALL */
    [0] = {
        [0] = { 0,  0},   /* D0: closest wall — base wall tile 0 */
        [1] = { 8,  8},   /* D1: wall tile 8 */
        [2] = {16, 16},   /* D2: wall tile 16 */
        [3] = {24, 24},   /* D3: farthest wall — tile 24 */
    },
    /* 1: FLOOR */
    [1] = {
        [0] = {128, TILE_FALLBACK},  /* D0: base floor 128 */
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
    /* 2: PIT */
    [2] = {
        [0] = {130, TILE_FALLBACK},  /* D0: pit floor 130 */
        [1] = {138, TILE_FALLBACK},
        [2] = {146, TILE_FALLBACK},
        [3] = {154, TILE_FALLBACK},
    },
    /* 3: STAIRS_UP */
    [3] = {
        [0] = {200, TILE_FALLBACK},  /* D0: stairs up */
        [1] = {201, TILE_FALLBACK},
        [2] = {202, TILE_FALLBACK},
        [3] = {203, TILE_FALLBACK},
    },
    /* 4: DOOR — closed uses wall tile, open uses floor tile */
    [4] = {
        [0] = {128, 32},   /* D0: floor 128 / wall tile 32 */
        [1] = {136, 40},
        [2] = {144, 48},
        [3] = {152, 56},
    },
    /* 5: TELEPORTER */
    [5] = {
        [0] = {170, TILE_FALLBACK},  /* D0: teleporter pad */
        [1] = {171, TILE_FALLBACK},
        [2] = {172, TILE_FALLBACK},
        [3] = {173, TILE_FALLBACK},
    },
    /* 6: ALARM */
    [6] = {
        [0] = {128, TILE_FALLBACK},
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
    /* 7: unknown */
    [7] = {
        [0] = {128, TILE_FALLBACK},
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
    /* 8: EXIT */
    [8] = {
        [0] = {180, TILE_FALLBACK},  /* D0: exit portal */
        [1] = {181, TILE_FALLBACK},
        [2] = {182, TILE_FALLBACK},
        [3] = {183, TILE_FALLBACK},
    },
    /* 9: TRIGGER */
    [9] = {
        [0] = {128, TILE_FALLBACK},
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
    /* 10: POOL */
    [10] = {
        [0] = {160, TILE_FALLBACK},  /* D0: recovery pool */
        [1] = {161, TILE_FALLBACK},
        [2] = {162, TILE_FALLBACK},
        [3] = {163, TILE_FALLBACK},
    },
    /* 11: SECRET */
    [11] = {
        [0] = { 0,  0},   /* D0: secret — looks like wall */
        [1] = { 8,  8},
        [2] = {16, 16},
        [3] = {24, 24},
    },
    /* 12-15: unknown / future */
    [12] = {
        [0] = {128, TILE_FALLBACK},
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
    [13] = {
        [0] = {210, TILE_FALLBACK},  /* D0: stairs down */
        [1] = {211, TILE_FALLBACK},
        [2] = {212, TILE_FALLBACK},
        [3] = {213, TILE_FALLBACK},
    },
    [14] = {
        [0] = {128, TILE_FALLBACK},
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
    [15] = {
        [0] = {128, TILE_FALLBACK},
        [1] = {136, TILE_FALLBACK},
        [2] = {144, TILE_FALLBACK},
        [3] = {152, TILE_FALLBACK},
    },
};

/* ── Tile blt helpers ─────────────────────────────────────────────── */
/*
 * Blt a decoded 8×8 tile row into the planar framebuffer.
 * dst:      pointer to row start in planar fb
 * tile_data: decoded 64-byte linear tile (indexed palette bytes)
 * row:      which row (0=top, 7=bottom)
 * x_offset: pixel x offset within the row (for sub-tile placement)
 *
 * Pixel replication: each source pixel is replicated 2× horizontally
 * to fill the PC Engine 256-wide viewport (each logical column = 2 pixels).
 */
static void blt_tile_row_2x(TQR_PlanarFramebuffer *fb,
                            const uint8_t *tile_data,
                            int row,
                            int fb_x, int fb_y) {
    if (!fb || !fb->data || !tile_data) return;
    if (fb_y < 0 || fb_y >= fb->h) return;
    if (fb_x < 0 || fb_x + 16 > fb->w) {
        /* Clip: clamp */
        int clip_left = (fb_x < 0) ? -fb_x : 0;
        int clip_right = (fb_x + 16 > fb->w) ? (fb_x + 16 - fb->w) : 0;
        int start_x = fb_x + clip_left;
        int count = 16 - clip_left - clip_right;
        if (count <= 0) return;
        uint8_t *row_ptr = fb->data + fb_y * fb->stride;
        const uint8_t *src = tile_data + row * 8 + clip_left;
        for (int i = 0; i < count; i++) {
            uint8_t px = src[i];
            int px_x = start_x + i;
            /* 2× horizontal replication */
            if (px_x >= 0 && px_x + 1 < fb->w) {
                row_ptr[px_x]     = px;
                row_ptr[px_x + 1] = px;
            }
        }
        return;
    }
    uint8_t *row_ptr = fb->data + fb_y * fb->stride;
    const uint8_t *src = tile_data + row * 8;
    /* 2× horizontal replication for each pixel */
    for (int i = 0; i < 8; i++) {
        uint8_t px = src[i];
        int px_x = fb_x + i * 2;
        row_ptr[px_x]     = px;
        row_ptr[px_x + 1] = px;
    }
}

/*
 * Blt a full 8×8 tile into the planar framebuffer at the given
 * top-left pixel position.  Tiles are placed at 16×16 screen-space
 * (each logical dungeon square = 16×16 pixels on screen).
 */
static void blt_tile_16x16(TQR_PlanarFramebuffer *fb,
                            const uint8_t *tile_data,
                            int fb_x, int fb_y) {
    if (!fb || !fb->data || !tile_data) return;
    for (int row = 0; row < 8; row++) {
        blt_tile_row_2x(fb, tile_data, row, fb_x, fb_y + row * 2);
        blt_tile_row_2x(fb, tile_data, row, fb_x, fb_y + row * 2 + 1);
    }
}

/* ── Fallback flat-color fill ─────────────────────────────────────── */
/*
 * Fill a 16×16 screen-space square with a flat palette color.
 * Used when tile index is TILE_FALLBACK (-1) or tile not loaded.
 * Source: deterministic fallback rule (Phase 4 mandate).
 */
static void fill_square_flat(TQR_PlanarFramebuffer *fb,
                              int fb_x, int fb_y,
                              uint8_t pal_index) {
    if (!fb || !fb->data) return;
    /* Clamp to framebuffer bounds */
    if (fb_x < 0) fb_x = 0;
    if (fb_y < 0) fb_y = 0;
    if (fb_x + 16 > fb->w) fb_x = fb->w - 16;
    if (fb_y + 16 > fb->h) fb_y = fb->h - 16;
    if (fb_x < 0 || fb_y < 0) return;

    for (int y = 0; y < 16; y++) {
        uint8_t *row_ptr = fb->data + (fb_y + y) * fb->stride;
        for (int x = 0; x < 16; x++) {
            row_ptr[fb_x + x] = pal_index;
        }
    }
}

/* ── Get tile data from palette state ─────────────────────────────── */
/*
 * Returns decoded 64-byte linear tile data, or NULL if not loaded.
 * Uses the palette state's tile pool.
 *
 * The tile_data must be copied out — returned pointer is not stable.
 * We decode into a per-call static buffer for simplicity (not thread-safe,
 * but rendering is single-threaded in M11).
 */
static const uint8_t *get_tile_data(const TQR_PaletteState *pal,
                                      int tile_index) {
    static uint8_t decoded[64];
    if (!pal || tile_index < 0 || tile_index >= TQR_MAX_TILES) return NULL;
    if (tile_index >= pal->tile_count) return NULL;
    const TQR_Tile *t = &pal->tiles[tile_index];
    if (!t->data) return NULL;
    tqr_decode_tile(decoded, t->data, t->bpp);
    return decoded;
}

/* ══════════════════════════════════════════════════════════════════════
 * Viewport lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

int theron_vp_init(Theron_V1_Viewport *vp) {
    if (!vp) return 0;
    memset(vp, 0, sizeof(*vp));

    /* Alloc planar framebuffer: 256×224 indexed bytes */
    vp->fb.w      = TQR_FB_W;
    vp->fb.h      = TQR_FB_H;
    vp->fb.stride = TQR_FB_W;
    vp->fb.data   = (uint8_t *)calloc((size_t)TQR_FB_W * TQR_FB_H, 1);
    if (!vp->fb.data) {
        vp->initialized = 0;
        return 0;
    }

    /* Init palette state with defaults */
    tqr_palette_init_defaults(&vp->palette);

    vp->viewport_x = 0;
    vp->viewport_y = 0;
    vp->initialized = 1;

    printf("[TQR] viewport initialized: %dx%d planar fb, palette ready\n",
           TQR_FB_W, TQR_FB_H);
    return 1;
}

void theron_vp_free(Theron_V1_Viewport *vp) {
    if (!vp) return;
    if (vp->fb.data) {
        free(vp->fb.data);
        vp->fb.data = NULL;
    }
    tqr_palette_free_tiles(&vp->palette);
    vp->initialized = 0;
}

void theron_vp_set_palette(Theron_V1_Viewport *vp, const TQR_PaletteState *palette) {
    if (!vp || !palette) return;
    vp->palette = *palette; /* copy */
}

/* ══════════════════════════════════════════════════════════════════════
 * Dungeon rendering
 * ══════════════════════════════════════════════════════════════════════ */

void theron_vp_render_dungeon(Theron_V1_Viewport *vp,
                               const Theron_V1_World *world) {
    if (!vp || !vp->initialized || !world) return;

    /* Clear planar framebuffer with black (palette index 0) */
    theron_vp_clear(vp, 0);

    /* Get party position and direction */
    const Theron_V1_Champion *theron = theron_v1_party_leader(&world->party);
    if (!theron) return;

    /* Party direction from world state (world tracks dungeon position).
     * Theron: position stored in world->levels[dungeon-1][level].start_x/y
     * and facing direction (default north = 0).  For Phase 4 we use
     * a simple placeholder: direction encoded in world.world_tick LSB.
     * Real direction tracking comes in Phase 5 mechanics. */
    int party_x = 0, party_y = 0;
    int party_dir = 0;

    /* Try to get party position from world state */
    {
        int did = world->current_dungeon;
        int lvl = world->current_level;
        if (did >= 1 && did <= THERON_DUNGEON_COUNT &&
            lvl >= 0 && lvl < THERON_MAX_LEVELS_PER_DUNGEON &&
            world->level_loaded[did - 1][lvl]) {
            const Theron_V1_Level *lv = &world->levels[did - 1][lvl];
            party_x = lv->start_x;
            party_y = lv->start_y;
            party_dir = lv->start_dir & 3;
        }
    }

    /*
     * View cone rendering: D0..D3 (depth 0=closest, 3=farthest)
     *
     * At each depth d in direction dir:
     *   - Center square: (cx, cy) = party_pos + dir * d
     *   - Left square:  (lx, ly)  = (cx) + left_vector
     *   - Right square: (rx, ry)  = (cx) - left_vector
     *
     * Screen layout (16px per square, 2× scale):
     *   D3:  48px tall, y=0..48
     *   D2:  48px tall, y=48..96
     *   D1:  48px tall, y=96..144
     *   D0:  48px tall, y=144..192
     *   Left column at x=0..47, Center at x=64..111, Right at x=128..175
     *
     * Viewport (256×224) is letterboxed: squares centered at:
     *   x_margin = (256 - 192) / 2 = 32
     *   y_margin = (224 - 192) / 2 = 16
     */

    const int sq_size = 16;   /* each dungeon square = 16px on screen */
    const int x_margin = 32;  /* center 192-wide view in 256-wide fb */
    const int y_margin = 16;  /* center 192-tall view in 224-tall fb  */

    /* Render each depth from far (D3) to near (D0) — painter's algorithm */
    for (int d = TQR_VP_DEPTH - 1; d >= 0; d--) {
        /* Center of the depth band in screen Y */
        int band_y = y_margin + (TQR_VP_DEPTH - 1 - d) * sq_size;

        /* Direction vectors */
        int dx = g_dir_dx[party_dir];
        int dy = g_dir_dy[party_dir];
        int lx = g_left_dx[party_dir];
        int ly = g_left_dy[party_dir];

        /* Three columns: left (-1), center (0), right (+1) */
        for (int col = -1; col <= 1; col++) {
            int cx = party_x + dx * d + lx * col;
            int cy = party_y + dy * d + ly * col;

            int screen_x = x_margin + (col + 1) * sq_size * 2; /* 0, 64, 128 */

            /* Get square type from world */
            uint8_t sq_type = theron_v1_world_get_square(world, cx, cy);
            int is_wall = (sq_type == THERON_SQUARE_WALL ||
                           sq_type == THERON_SQUARE_SECRET) ? 1 : 0;
            int is_open = (sq_type == THERON_SQUARE_DOOR &&
                           /* door state: open if object is open */ 0) ? 1 : 0;
            (void)is_open;

            /* Look up tile for this square at this depth */
            int tile_idx = g_tile_table[sq_type & 0xF][d][is_wall];

            if (tile_idx == TILE_FALLBACK || tile_idx < 0) {
                /* Deterministic fallback: flat mid-gray (palette entry 7) */
                fill_square_flat(&vp->fb, screen_x, band_y, TQR_TILE_FALLBACK_COLOR_INDEX);
            } else {
                /* Try to decode and blt the tile */
                const uint8_t *tile_data = get_tile_data(&vp->palette, tile_idx);
                if (tile_data) {
                    blt_tile_16x16(&vp->fb, tile_data, screen_x, band_y);
                } else {
                    /* Tile not in atlas: fallback flat color */
                    fill_square_flat(&vp->fb, screen_x, band_y, TQR_TILE_FALLBACK_COLOR_INDEX);
                }
            }

            /* Side walls: if center is open but neighbor in col direction is wall,
             * draw the side wall tile (handled separately in Phase 5). */

            /* Special squares at D0/D1: render object/creature overlay */
            if ((d == 0 || d == 1)) {
                /* Phase 5: creature/object sprite overlay at D0/D1 */
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * UI chrome rendering
 * ══════════════════════════════════════════════════════════════════════ */

/* Draw a horizontal bar in the planar framebuffer */
void theron_vp_draw_bar(TQR_PlanarFramebuffer *fb,
                        int x, int y, int w, int h,
                        int current, int max,
                        uint8_t pal_index,
                        uint8_t bg_index) {
    if (!fb || !fb->data) return;
    if (x < 0 || y < 0 || x + w > fb->w || y + h > fb->h) return;
    if (max <= 0) max = 1;
    if (current < 0) current = 0;
    int filled = (current * w) / max;
    if (filled > w) filled = w;

    for (int row = 0; row < h; row++) {
        uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
        /* Filled portion */
        for (int col = 0; col < filled; col++) {
            row_ptr[x + col] = pal_index;
        }
        /* Empty portion */
        for (int col = filled; col < w; col++) {
            row_ptr[x + col] = bg_index;
        }
    }
}

/* Get dungeon display name */
static const char *theron_dungeon_name(int dungeon_id) {
    static const char *names[THERON_DUNGEON_COUNT + 1] = {
        [1] = "Hall of Records",
        [2] = "Catacombs",
        [3] = "Caverns",
        [4] = "Castle",
        [5] = "Tower",
        [6] = "Temple",
        [7] = "Final Dungeon",
    };
    if (dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) return "Unknown";
    return names[dungeon_id] ? names[dungeon_id] : "Unknown";
}

/* Render the top bar: dungeon name + quest item count */
static void render_topbar(TQR_PlanarFramebuffer *fb,
                           const Theron_V1_World *world,
                           int y_offset) {
    if (!fb || !world) return;
    /* Top bar: y=0..TQR_TOPBAR_H within the planar fb (after y_margin) */
    int y = y_offset;
    uint8_t dark_gray = 12;   /* palette index for dark gray */
    uint8_t light_gray = 2;   /* palette index for light gray */

    /* Background */
    for (int row = 0; row < TQR_TOPBAR_H; row++) {
        uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
        for (int col = 0; col < fb->w; col++) {
            row_ptr[col] = dark_gray;
        }
    }

    /* Dungeon name text: rendered as raw pixel dump using font tiles.
     * Phase 5: full font rendering.  For Phase 4, we draw a simple
     * bar indicator showing dungeon number. */
    int dungeon_id = world->current_dungeon;
    int quest_items = world->quest_items_in_dungeon;

    /* Simple pixel indicator: draw colored squares representing dungeon/quest */
    int x = 8;
    /* Dungeon ID as colored pixel block */
    for (int i = 0; i < dungeon_id && x + 4 < fb->w; i++, x += 6) {
        for (int row = 4; row < 12 && y + row < y + TQR_TOPBAR_H; row++) {
            uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
            for (int col = 0; col < 4; col++) {
                row_ptr[x + col] = (uint8_t)(10 + (i % 4));  /* tan/skin color */
            }
        }
    }
    (void)quest_items;
    (void)light_gray;
}

/* Render the right panel: Theron stats + compass */
static void render_right_panel(TQR_PlanarFramebuffer *fb,
                                const Theron_V1_World *world,
                                int x_offset) {
    if (!fb || !world) return;
    int x = x_offset;
    uint8_t dark_gray = 12;

    /* Right panel: x = 256 - TQR_RIGHT_W = 160..255, y = y_margin..192 */
    /* Fill panel background */
    for (int row = 0; row < TQR_VP_H; row++) {
        uint8_t *row_ptr = fb->data + (16 + row) * fb->stride;
        for (int col = 0; col < TQR_RIGHT_W; col++) {
            if (x + col < fb->w) row_ptr[x + col] = dark_gray;
        }
    }

    /* Theron stats */
    const Theron_V1_Champion *theron = theron_v1_party_leader(&world->party);
    if (!theron) return;

    /* HP bar: x=165, y=24, w=80, h=6 */
    theron_vp_draw_bar(fb, x + 5, 28, 80, 6,
                       theron->health, theron->max_health,
                       8, /* red for HP */
                       dark_gray);

    /* Stamina bar: x=165, y=40, w=80, h=6 */
    theron_vp_draw_bar(fb, x + 5, 40, 80, 6,
                       theron->stamina, theron->max_stamina,
                       10, /* tan/skin for stamina */
                       dark_gray);

    /* Compass direction indicator: simple arrow in center-right */
    int dir = 0;
    {
        int did = world->current_dungeon;
        int lvl = world->current_level;
        if (did >= 1 && did <= THERON_DUNGEON_COUNT &&
            lvl >= 0 && lvl < THERON_MAX_LEVELS_PER_DUNGEON &&
            world->level_loaded[did - 1][lvl]) {
            dir = world->levels[did - 1][lvl].start_dir & 3;
        }
    }
    /* Draw a simple directional pixel marker at x+40, y=80 */
    {
        int cx = x + 40;
        int cy = 80;
        /* Arrow pointing in dir: 3×3 pixel cluster */
        static const int8_t arrow_disp[4][2] = {
            [0] = { 0, -3},  /* N: above center */
            [1] = { 3,  0},  /* E: right of center */
            [2] = { 0,  3},  /* S: below center */
            [3] = {-3,  0},  /* W: left of center */
        };
        int ax = cx + arrow_disp[dir][0];
        int ay = cy + arrow_disp[dir][1];
        if (ax >= 0 && ax < fb->w && ay >= 0 && ay < fb->h) {
            fb->data[ay * fb->stride + ax] = 11; /* yellow */
        }
    }
}

/* Champion name string (truncated to n chars) */
static void champ_slot_name(char *buf, size_t buf_size,
                            const Theron_V1_Champion *c) {
    if (!c || !buf || buf_size == 0) return;
    if (c->name[0] == '\0') {
        snprintf(buf, buf_size, "---");
        return;
    }
    snprintf(buf, buf_size, "%-12s", c->name);
}

/* Render a single champion slot (80×56 bottom panel) */
void theron_vp_draw_champion_slot(TQR_PlanarFramebuffer *fb,
                                   int slot_idx,
                                   int x, int y,
                                   const Theron_V1_Champion *champion) {
    if (!fb || !fb->data) return;
    if (slot_idx < 0 || slot_idx >= THERON_MAX_CHAMPIONS) return;

    uint8_t bg     = 12;  /* dark gray background */
    uint8_t frame  =  1;  /* gray border */
    uint8_t name_color = 15; /* white */
    uint8_t hp_col =  8;  /* red HP */
    uint8_t stam_col = 10; /* tan stamina */
    uint8_t mana_col = 14; /* blue mana */

    /* Slot background fill */
    for (int row = 0; row < TQR_CHAMP_SLOT_H; row++) {
        if (y + row >= fb->h) break;
        uint8_t *row_ptr = fb->data + (y + row) * fb->stride;
        for (int col = 0; col < TQR_CHAMP_SLOT_W; col++) {
            if (x + col < fb->w) row_ptr[x + col] = bg;
        }
    }

    if (!champion || !champion->alive) {
        /* Empty/dead slot: draw X mark */
        for (int i = 0; i < 16 && x + i < fb->w && y + i < fb->h; i++) {
            if (x + i < fb->w && y + i < fb->h)
                fb->data[(y + i) * fb->stride + x + i] = frame;
            if (x + i < fb->w && y + 16 - 1 - i < fb->h)
                fb->data[(y + 16 - 1 - i) * fb->stride + x + i] = frame;
        }
        return;
    }

    /* Icon area: 24×24 at top-left of slot */
    int icon_x = x + 4;
    int icon_y = y + 4;
    /* Class icon: simple colored square per class */
    {
        uint8_t class_col = 1; /* gray default */
        switch (champion->primary_class) {
            case THERON_CLASS_FIGHTER: class_col = 8;  break; /* red */
            case THERON_CLASS_NINJA:   class_col = 9;  break; /* orange */
            case THERON_CLASS_PRIEST:  class_col = 15; break; /* white */
            case THERON_CLASS_WIZARD:  class_col = 14; break; /* blue */
        }
        for (int r = 0; r < 16 && icon_y + r < fb->h; r++) {
            uint8_t *row_ptr = fb->data + (icon_y + r) * fb->stride;
            for (int c2 = 0; c2 < 16 && icon_x + c2 < fb->w; c2++) {
                row_ptr[icon_x + c2] = class_col;
            }
        }
    }

    /* Name: simple text bar (Phase 5: font tiles) */
    int name_x = x + 24;
    int name_y = y + 6;
    /* Draw name as colored bar (placeholder) */
    {
        char name_buf[16];
        champ_slot_name(name_buf, sizeof(name_buf), champion);
        (void)name_buf;
        /* For Phase 4: draw a colored name bar */
        for (int c2 = 0; c2 < 50 && name_x + c2 < fb->w; c2++) {
            uint8_t *row_ptr = fb->data + (name_y) * fb->stride;
            row_ptr[name_x + c2] = name_color;
        }
    }

    /* HP bar */
    theron_vp_draw_bar(fb, x + 24, y + 18, 50, 4,
                       champion->health, champion->max_health,
                       hp_col, bg);

    /* Stamina bar */
    theron_vp_draw_bar(fb, x + 24, y + 26, 50, 4,
                       champion->stamina, champion->max_stamina,
                       stam_col, bg);

    /* Mana bar (only for magic users) */
    if (champion->max_mana > 0) {
        theron_vp_draw_bar(fb, x + 24, y + 34, 50, 4,
                           champion->mana, champion->max_mana,
                           mana_col, bg);
    }
}

void theron_vp_render_ui(Theron_V1_Viewport *vp,
                          const Theron_V1_World *world,
                          uint32_t ui_flags) {
    if (!vp || !vp->initialized || !world) return;

    int x_margin = 32;   /* (256-192)/2 */
    int y_margin = 16;   /* (224-192)/2 */

    if (ui_flags & TQR_UI_TOPBAR) {
        render_topbar(&vp->fb, world, y_margin);
    }

    if (ui_flags & TQR_UI_RIGHT_PANEL) {
        render_right_panel(&vp->fb, world, 256 - TQR_RIGHT_W);
    }

    if (ui_flags & TQR_UI_BOTTOM_PANEL) {
        /* Bottom panel: 4 champion slots, each 80×56 */
        int slot_y = 192 + y_margin; /* y_margin puts vp at y=16, vp h=192 → bottom at 208 */
        /* Actually in the 224-tall fb, bottom panel starts at y=184 (within the vp) */
        slot_y = 184; /* absolute y in planar fb */
        for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
            int slot_x = i * TQR_CHAMP_SLOT_W;
            const Theron_V1_Champion *c = theron_v1_party_getChampion(&world->party, i);
            theron_vp_draw_champion_slot(&vp->fb, i, slot_x, slot_y, c);
        }
    }

    (void)x_margin;
}

/* ══════════════════════════════════════════════════════════════════════
 * Presentation (planar → M11 framebuffer)
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * Convert palette index → M11 VGA palette slot.
 * Theron PC Engine palette entries (0-255) are mapped to the M11
 * 16-color VGA palette as follows (approximate HuC6270 → VGA mapping):
 *
 * HuC6270 palette group 0 (dungeon stone):
 *   0-15   → VGA slots 0-15 (dungeon stone palette)
 * HuC6270 palette group 1 (creatures):
 *   16-31  → VGA slots 0-15 (creature palette)
 * etc.
 *
 * For the M11 framebuffer, we convert Theron's indexed pixel to
 * the closest M11 VGA palette index using a fixed mapping table.
 * The mapping is deterministic: same input index → same output slot.
 *
 * Simplified mapping (Phase 4): index % 16.
 * Full mapping comes in Phase 5 when Track 02 palette data is available.
 */
static uint8_t tqr_idx_to_m11_idx(uint8_t tqr_idx) {
    /* Deterministic: use lower 4 bits of the TQR palette index */
    return tqr_idx & 0xF;
}

void theron_vp_present(const Theron_V1_Viewport *vp,
                       const TQR_PaletteState *palette,
                       unsigned char *m11_fb,
                       int m11_fb_w,
                       int m11_fb_h) {
    if (!vp || !vp->initialized || !m11_fb) return;
    if (!palette) palette = &vp->palette;

    /*
     * Letterbox the 256×224 planar fb into the M11 320×200 framebuffer.
     * Planar fb is centered:
     *   dst_x = (320 - 256) / 2 = 32
     *   dst_y = (200 - 192) / 2 = 4  (viewport height 192 in 200)
     *
     * Actually for Theron, we use the full 224-tall fb but scale to fit:
     *   M11 viewport: x=32, y=24, w=256, h=192 (Theron letterboxed)
     *   M11 total: 320×200
     */
    int dst_x = 32;
    int dst_y = 24;

    /* Copy each row of the planar fb into the M11 framebuffer */
    for (int y = 0; y < vp->fb.h; y++) {
        int m11_y = dst_y + y;
        if (m11_y >= m11_fb_h) break;

        uint8_t *src_row = vp->fb.data + y * vp->fb.stride;
        unsigned char *dst_row = m11_fb + m11_y * m11_fb_w;

        for (int x = 0; x < vp->fb.w; x++) {
            int m11_x = dst_x + x;
            if (m11_x >= m11_fb_w) break;

            uint8_t tqr_idx = src_row[x];
            uint8_t m11_idx = tqr_idx_to_m11_idx(tqr_idx);
            dst_row[m11_x] = m11_idx;
        }
    }

    /* Black bars above and below viewport */
    /* Top bar: y=0..23 */
    for (int y = 0; y < dst_y && y < m11_fb_h; y++) {
        unsigned char *row = m11_fb + y * m11_fb_w;
        for (int x = 0; x < m11_fb_w; x++) {
            row[x] = 0; /* black */
        }
    }
    /* Bottom: y = dst_y + vp->fb.h .. m11_fb_h */
    for (int y = dst_y + vp->fb.h; y < m11_fb_h; y++) {
        unsigned char *row = m11_fb + y * m11_fb_w;
        for (int x = 0; x < m11_fb_w; x++) {
            row[x] = 0; /* black */
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Utility
 * ══════════════════════════════════════════════════════════════════════ */

int theron_vp_tile_for_square(int square_type, int depth, int is_wall) {
    if (depth < 0 || depth >= TQR_VP_DEPTH) return TILE_FALLBACK;
    int st = square_type & 0xF;
    return g_tile_table[st][depth][is_wall ? 1 : 0];
}

void theron_vp_clear(Theron_V1_Viewport *vp, uint8_t color_index) {
    if (!vp || !vp->fb.data) return;
    size_t n = (size_t)vp->fb.w * (size_t)vp->fb.h;
    memset(vp->fb.data, color_index, n);
}

const char *theron_v1_viewport_source_evidence(void) {
    return "THQUEST.ASM T400/T520/T600  "
           "+ tqr_v1_phase2_data_formats_H2339.md §7  "
           "+ HuC6260/HuC6270 datasheet";
}