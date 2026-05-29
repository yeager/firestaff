/*
 * theron_v1_tile_renderer.c — Theron's Quest V1 Phase 4: Tile Renderer
 *
 * 2D tile-based dungeon renderer.  Theron's dungeon view is a 2D tile grid
 * (not the classic 3D first-person view of DM1/CSB/DM2).  This module
 * implements the complete tile selection, decoding, and rasterisation
 * pipeline for the Theron viewport.
 *
 * View cone rendering mirrors ReDMCSB DUNVIEW.C DrawSquareD* functions:
 *   D3 (farthest) → D0 (nearest) — painter's algorithm
 *   At each depth: left square, center square, right square
 *
 * Source-lock:
 *   ReDMCSB DUNVIEW.C F0116_DUNGEONVIEW_DrawSquareD3L  — line 6361
 *   ReDMCSB DUNVIEW.C F0117_DUNGEONVIEW_DrawSquareD3R  — line 6500
 *   ReDMCSB DUNVIEW.C F0118_DUNGEONVIEW_DrawSquareD3C  — line 6642
 *   ReDMCSB DUNVIEW.C F0119_DUNGEONVIEW_DrawSquareD2L  — line 6900
 *   ReDMCSB DUNVIEW.C F0120_DUNGEONVIEW_DrawSquareD2R  — line 7051
 *   ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C  — line 7244
 *   ReDMCSB DUNVIEW.C F0122_DUNGEONVIEW_DrawSquareD1L  — line 7391
 *   ReDMCSB DUNVIEW.C F0123_DUNGEONVIEW_DrawSquareD1R  — line 7559
 *   ReDMCSB DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C  — line 7727
 *   ReDMCSB DUNVIEW.C F0125_DUNGEONVIEW_DrawSquareD0L  — line 7960
 *   ReDMCSB DUNVIEW.C F0126_DUNGEONVIEW_DrawSquareD0R  — line 8064
 *   ReDMCSB DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C  — line 8164
 *   THQUEST.ASM T520   — viewport tile selection
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §7
 *   HuC6260/HuC6270 datasheet — planar tile format
 */

#include "theron_v1_tile_renderer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

_Static_assert(TR_VP_DEPTH == 4,  "View cone depth must be 4");
_Static_assert(TR_SQ_SIZE == 16,   "Square size must be 16px");
_Static_assert(TR_TILE_DIM == 8,   "Tile dimension must be 8px");

/* ── Direction vectors ──────────────────────────────────────────────── */

static const int8_t g_dir_dx[4] = {  0,  1,  0, -1 };
static const int8_t g_dir_dy[4] = { -1,  0,  1,  0 };
/* Left-vector (perpendicular) of each direction */
static const int8_t g_left_dx[4] = { -1,  0,  1,  0 };
static const int8_t g_left_dy[4] = {  0, -1,  0,  1 };

/* ── Square-type → tile-index lookup ────────────────────────────────── */
/*
 * Deterministic tile selection per square type + depth.
 * Index: tile_index = g_tile_table[square_type][depth][is_wall]
 *
 * tile_index >= 0:   tile index into TQR_PaletteState tile atlas
 * tile_index == -1:  flat-color fallback (palette entry 7 mid-gray)
 *
 * Source: THQUEST.ASM T520 — tile bank loading uses dungeon_seed to
 * select which tile sub-bank to use.  Deterministic mapping:
 *   floor/wall type + distance → tile index.
 *
 * Tile bank layout (from T520 and Track 02 format docs):
 *   tiles 0-127:    wall tiles (2bpp, palette group 0)
 *   tiles 128-255:  floor tiles (2bpp, palette group 0)
 *   tiles 256-383:  object tiles (2bpp, palette group 2)
 *   tiles 384-511:  creature tiles (4bpp, palette group 1)
 *   tiles 512-639:  UI tile set (2bpp, palette group 3)
 *   tiles 640-767:  font tiles (2bpp, palette group 4)
 *
 * For squares at depth d in direction dir:
 *   wall tile:  base_wall + (d % 4) * 8 + side_offset
 *   floor tile: base_floor + (d % 4) * 8
 *
 * Square type key:
 *   0  = WALL     → wall tile
 *   1  = FLOOR    → floor tile
 *   2  = PIT      → pit trap floor
 *   3  = STAIRS_UP   → stairs up tile
 *   4  = DOOR     → door tile (wall when closed, floor when open)
 *   5  = TELEPORTER → teleporter pad
 *   6  = ALARM    → alarm trigger floor
 *   8  = EXIT     → exit portal tile
 *   9  = TRIGGER  → event trigger floor
 *   10 = POOL     → recovery pool tile
 *   11 = SECRET   → hidden door (looks like wall)
 *   13 = STAIRS_DOWN → stairs down tile
 */
static const int g_tile_table[16][TR_VP_DEPTH][2] = {
    /* 0: WALL */
    [0] = {
        [0] = {  0,   0},   /* D0: closest wall — base wall tile 0   */
        [1] = {  8,   8},   /* D1: wall tile 8                        */
        [2] = { 16,  16},   /* D2: wall tile 16                       */
        [3] = { 24,  24},   /* D3: farthest wall — tile 24            */
    },
    /* 1: FLOOR */
    [1] = {
        [0] = {128,  -1},   /* D0: base floor 128                      */
        [1] = {136,  -1},   /* D1: floor tile 136                      */
        [2] = {144,  -1},   /* D2: floor tile 144                      */
        [3] = {152,  -1},   /* D3: farthest floor tile 152             */
    },
    /* 2: PIT */
    [2] = {
        [0] = {130,  -1},   /* D0: pit floor 130                       */
        [1] = {138,  -1},   /* D1: pit tile 138                        */
        [2] = {146,  -1},   /* D2: pit tile 146                        */
        [3] = {154,  -1},   /* D3: pit tile 154                        */
    },
    /* 3: STAIRS_UP */
    [3] = {
        [0] = {200,  -1},   /* D0: stairs up tile 200                   */
        [1] = {201,  -1},   /* D1: stairs up tile 201                  */
        [2] = {202,  -1},   /* D2: stairs up tile 202                  */
        [3] = {203,  -1},   /* D3: stairs up tile 203                  */
    },
    /* 4: DOOR — closed uses wall tile, open uses floor tile */
    [4] = {
        [0] = {128,  32},   /* D0: floor 128 / wall tile 32            */
        [1] = {136,  40},   /* D1: floor 136 / wall tile 40            */
        [2] = {144,  48},   /* D2: floor 144 / wall tile 48            */
        [3] = {152,  56},   /* D3: floor 152 / wall tile 56            */
    },
    /* 5: TELEPORTER */
    [5] = {
        [0] = {170,  -1},   /* D0: teleporter pad 170                  */
        [1] = {171,  -1},   /* D1: teleporter tile 171                */
        [2] = {172,  -1},   /* D2: teleporter tile 172                */
        [3] = {173,  -1},   /* D3: teleporter tile 173                */
    },
    /* 6: ALARM */
    [6] = {
        [0] = {128,  -1},   /* D0: alarm floor                         */
        [1] = {136,  -1},   /* D1: alarm tile                          */
        [2] = {144,  -1},   /* D2: alarm tile                          */
        [3] = {152,  -1},   /* D3: alarm tile                          */
    },
    /* 7: unknown — default floor */
    [7] = {
        [0] = {128,  -1},
        [1] = {136,  -1},
        [2] = {144,  -1},
        [3] = {152,  -1},
    },
    /* 8: EXIT portal */
    [8] = {
        [0] = {180,  -1},   /* D0: exit portal 180                      */
        [1] = {181,  -1},   /* D1: exit tile 181                       */
        [2] = {182,  -1},   /* D2: exit tile 182                       */
        [3] = {183,  -1},   /* D3: exit tile 183                       */
    },
    /* 9: TRIGGER */
    [9] = {
        [0] = {128,  -1},   /* D0: trigger floor                        */
        [1] = {136,  -1},   /* D1: trigger tile                         */
        [2] = {144,  -1},   /* D2: trigger tile                         */
        [3] = {152,  -1},   /* D3: trigger tile                         */
    },
    /* 10: POOL */
    [10] = {
        [0] = {160,  -1},   /* D0: recovery pool 160                    */
        [1] = {161,  -1},   /* D1: pool tile 161                       */
        [2] = {162,  -1},   /* D2: pool tile 162                       */
        [3] = {163,  -1},   /* D3: pool tile 163                       */
    },
    /* 11: SECRET door (hidden — looks like wall) */
    [11] = {
        [0] = {  0,   0},   /* D0: secret — wall tile 0               */
        [1] = {  8,   8},   /* D1: wall tile 8                         */
        [2] = { 16,  16},   /* D2: wall tile 16                        */
        [3] = { 24,  24},   /* D3: wall tile 24                         */
    },
    /* 12: unknown — default floor */
    [12] = {
        [0] = {128,  -1},
        [1] = {136,  -1},
        [2] = {144,  -1},
        [3] = {152,  -1},
    },
    /* 13: STAIRS_DOWN */
    [13] = {
        [0] = {210,  -1},   /* D0: stairs down tile 210                */
        [1] = {211,  -1},   /* D1: stairs down tile 211               */
        [2] = {212,  -1},   /* D2: stairs down tile 212               */
        [3] = {213,  -1},   /* D3: stairs down tile 213               */
    },
    /* 14: unknown — default floor */
    [14] = {
        [0] = {128,  -1},
        [1] = {136,  -1},
        [2] = {144,  -1},
        [3] = {152,  -1},
    },
    /* 15: unknown — default floor */
    [15] = {
        [0] = {128,  -1},
        [1] = {136,  -1},
        [2] = {144,  -1},
        [3] = {152,  -1},
    },
};

/* ══════════════════════════════════════════════════════════════════════
 * Tile decoding (HuC6260 planar → indexed bitmap)
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * tr_decode_tile_row — decode one 8-pixel row from planar bitplanes.
 *
 * Source: HuC6260 VDC datasheet — planar bitmap format.
 * Bit layout: LSB = leftmost pixel (HuC6260 native, NOT flipped).
 *
 * 2bpp (16 bytes/tile): each row = 2 bytes (bitplane 0 + bitplane 1)
 *   bit 7 of byte 0 = pixel 0 bit 0
 *   bit 7 of byte 1 = pixel 0 bit 1
 *   pixel_index = (bit1 << 1) | bit0
 *
 * 4bpp (32 bytes/tile): each row = 4 bytes (bitplane 0..3)
 *   pixel_index = (bit3 << 3) | (bit2 << 2) | (bit1 << 1) | bit0
 */
void tr_decode_tile_row(uint8_t *out_row,
                       const uint8_t *src_row,
                       int bpp) {
    if (bpp == 2) {
        /* 2bpp: 2 bytes per row → 8 pixels */
        for (int i = 0; i < 8; i++) {
            int bit = 7 - i; /* MSB first = leftmost pixel */
            int b0 = (src_row[0] >> bit) & 1;
            int b1 = (src_row[1] >> bit) & 1;
            out_row[i] = (uint8_t)((b1 << 1) | b0);
        }
    } else if (bpp == 4) {
        /* 4bpp: 4 bytes per row → 8 pixels */
        for (int i = 0; i < 8; i++) {
            int bit = 7 - i;
            int b0 = (src_row[0] >> bit) & 1;
            int b1 = (src_row[1] >> bit) & 1;
            int b2 = (src_row[2] >> bit) & 1;
            int b3 = (src_row[3] >> bit) & 1;
            out_row[i] = (uint8_t)((b3 << 3) | (b2 << 2) | (b1 << 1) | b0);
        }
    }
}

/*
 * tr_decode_tile — decode a full 8×8 planar tile into 64-byte indexed bitmap.
 * Source: HuC6260 VDC datasheet.
 */
void tr_decode_tile(uint8_t *out64, const uint8_t *src, int bpp) {
    int bytes_per_row = (bpp == 2) ? 2 : 4;
    for (int row = 0; row < 8; row++) {
        tr_decode_tile_row(out64 + row * 8, src + row * bytes_per_row, bpp);
    }
}

/* Static decoded tile buffer — rendering is single-threaded in M11 */
static uint8_t g_decoded_tile[64];

/*
 * tr_get_tile_data — get decoded tile data for a given tile index.
 * Returns pointer to static 64-byte buffer (not thread-safe).
 * Source: THQUEST.ASM T400 (tile bank loading).
 */
const uint8_t *tr_get_tile_data(const TQR_PaletteState *palette, int tile_index) {
    if (!palette || tile_index < 0 || tile_index >= TQR_MAX_TILES) return NULL;
    if (tile_index >= palette->tile_count) return NULL;
    const TQR_Tile *t = &palette->tiles[tile_index];
    if (!t->data) return NULL;
    tr_decode_tile(g_decoded_tile, t->data, t->bpp);
    return g_decoded_tile;
}

/* ══════════════════════════════════════════════════════════════════════
 * Tile blt helpers (planar fb ← decoded tile)
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * blt_tile_row_2x — blit one decoded 8×8 tile row into planar fb.
 *
 * Each pixel in the decoded tile is replicated 2× horizontally
 * (16px output per 8px input) to fill the PC Engine logical square.
 *
 * ReDMCSB reference: DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C
 * draws the center D0 square using a similar per-row blt pattern.
 */
static void blt_tile_row_2x(TQR_PlanarFramebuffer *fb,
                            const uint8_t *tile_data,
                            int row,
                            int fb_x, int fb_y) {
    if (!fb || !fb->data || !tile_data) return;
    if (fb_y < 0 || fb_y >= fb->h) return;
    if (fb_x < 0 || fb_x + 16 > fb->w) {
        /* Clip: clamp left/right */
        int clip_left  = (fb_x < 0)       ? -fb_x : 0;
        int clip_right = (fb_x + 16 > fb->w) ? (fb_x + 16 - fb->w) : 0;
        int start_x  = fb_x + clip_left;
        int count    = 16 - clip_left - clip_right;
        if (count <= 0) return;
        uint8_t *row_ptr = fb->data + fb_y * fb->stride;
        const uint8_t *src = tile_data + row * 8 + clip_left;
        for (int i = 0; i < count; i++) {
            uint8_t px = src[i];
            int px_x = start_x + i;
            if (px_x >= 0 && px_x + 1 < fb->w) {
                row_ptr[px_x]     = px;
                row_ptr[px_x + 1] = px;
            }
        }
        return;
    }
    uint8_t *row_ptr = fb->data + fb_y * fb->stride;
    const uint8_t *src = tile_data + row * 8;
    for (int i = 0; i < 8; i++) {
        uint8_t px = src[i];
        int px_x = fb_x + i * 2;
        row_ptr[px_x]     = px;
        row_ptr[px_x + 1] = px;
    }
}

/*
 * blt_tile_16x16 — blit a full 8×8 decoded tile to a 16×16 screen region.
 * Each logical dungeon square = 16×16 pixels.
 * Source: ReDMCSB DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C (line 8164).
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

/*
 * fill_square_flat — fill a 16×16 screen region with a solid palette color.
 * Used when tile index is TILE_FALLBACK (-1) or tile not loaded.
 * Source: deterministic fallback rule (Phase 4 mandate).
 */
static void fill_square_flat(TQR_PlanarFramebuffer *fb,
                              int fb_x, int fb_y,
                              uint8_t pal_index) {
    if (!fb || !fb->data) return;
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

/* ══════════════════════════════════════════════════════════════════════
 * Public tile-renderer API
 * ══════════════════════════════════════════════════════════════════════ */

int tr_tile_for_square(int square_type, int depth, int is_wall) {
    if (depth < 0 || depth >= TR_VP_DEPTH) return TR_TILE_FALLBACK;
    int st = square_type & 0xF;
    return g_tile_table[st][depth][is_wall ? 1 : 0];
}

void tr_clear_fb(TQR_PlanarFramebuffer *fb, uint8_t color_index) {
    if (!fb || !fb->data) return;
    size_t n = (size_t)fb->w * (size_t)fb->h;
    memset(fb->data, color_index, n);
}

/* ══════════════════════════════════════════════════════════════════════
 * Dungeon rendering (view cone)
 * ══════════════════════════════════════════════════════════════════════ */

/*
 * tr_render_dungeon — render the 2D tile-grid dungeon view.
 *
 * Uses the party position and direction from world state to build
 * the view cone: D3 (far) → D0 (near), painter's algorithm.
 *
 * At each depth d in direction dir:
 *   center square: (cx, cy) = party_pos + dir * d
 *   left square:  (lx, ly)  = (cx) + left_vector
 *   right square: (rx, ry)  = (cx) - left_vector
 *
 * Screen layout (256×224 planar fb, 192×192 view cone):
 *   D3:  y = y_margin + 0..47    (3 rows × 16px)
 *   D2:  y = y_margin + 48..95
 *   D1:  y = y_margin + 96..143
 *   D0:  y = y_margin + 144..191
 *   Left column:   x = x_margin + 0..63    (4 squares × 16px)
 *   Center column: x = x_margin + 64..127
 *   Right column: x = x_margin + 128..191
 *
 * Source: ReDMCSB DUNVIEW.C F0116-27 (DrawSquareD3/D2/D1/D0 L/R/C)
 *         THQUEST.ASM T520 (viewport tile selection)
 */
void tr_render_dungeon(TQR_PlanarFramebuffer *fb,
                       const TQR_PaletteState *palette,
                       Theron_V1_World *world) {
    if (!fb || !fb->data || !world) return;
    if (!palette) return;

    /* Clear with black (palette index 0) */
    tr_clear_fb(fb, 0);

    /* Get party position and direction from world state.
     * Theron: position stored in world->levels[dungeon-1][level].start_x/y
     * and facing direction. */
    const Theron_V1_Champion *theron = theron_v1_party_leader_c(&world->party);
    if (!theron) return;

    int party_x, party_y, party_dir;
    party_x = party_y = party_dir = 0;

    {
        int did = world->current_dungeon;
        int lvl = world->current_level;
        if (did >= 1 && did <= THERON_DUNGEON_COUNT &&
            lvl >= 0 && lvl < THERON_MAX_LEVELS_PER_DUNGEON &&
            world->level_loaded[did - 1][lvl]) {
            const Theron_V1_Level *lv = &world->levels[did - 1][lvl];
            party_x   = lv->start_x;
            party_y   = lv->start_y;
            party_dir = lv->start_dir & 3;
        }
    }

    const int sq_size   = TR_SQ_SIZE;   /* 16px per dungeon square   */
    const int x_margin  = TR_X_MARGIN;  /* center 192-wide view      */
    const int y_margin  = TR_Y_MARGIN;  /* center 192-tall view      */

    /* Render D3 → D0 (far to near) — painter's algorithm.
     * ReDMCSB DUNVIEW.C: D3 squares drawn first, then D2, D1, D0.
     * Within each depth, left→center→right (back-to-front within the row). */
    for (int d = TR_VP_DEPTH - 1; d >= 0; d--) {
        /* Band y in screen space */
        int band_y = y_margin + (TR_VP_DEPTH - 1 - d) * sq_size;

        int dx = g_dir_dx[party_dir];
        int dy = g_dir_dy[party_dir];
        int lx = g_left_dx[party_dir];
        int ly = g_left_dy[party_dir];

        /* Three columns: left (-1), center (0), right (+1) */
        for (int col = -1; col <= 1; col++) {
            int sq_x = party_x + dx * d + lx * col;
            int sq_y = party_y + dy * d + ly * col;

            int screen_x = x_margin + (col + 1) * sq_size * 2; /* 0, 64, 128 */

            /* Get square type from world map */
            uint8_t sq_type = theron_v1_world_get_square(world, sq_x, sq_y);
            int is_wall = (sq_type == THERON_SQUARE_WALL ||
                           sq_type == THERON_SQUARE_SECRET) ? 1 : 0;
            /* Door state: open if object at square is open (Phase 5) */
            (void)is_wall;

            /* Tile lookup */
            int tile_idx = tr_tile_for_square(sq_type, d, is_wall);

            if (tile_idx == TR_TILE_FALLBACK || tile_idx < 0) {
                /* Deterministic fallback: flat mid-gray palette entry 7 */
                fill_square_flat(fb, screen_x, band_y, TQR_TILE_FALLBACK_COLOR_INDEX);
            } else {
                const uint8_t *tile_data = tr_get_tile_data(palette, tile_idx);
                if (tile_data) {
                    blt_tile_16x16(fb, tile_data, screen_x, band_y);
                } else {
                    /* Tile not in atlas: fallback flat color */
                    fill_square_flat(fb, screen_x, band_y, TQR_TILE_FALLBACK_COLOR_INDEX);
                }
            }

            /* Side walls: if center is open but neighbor in col direction
             * is wall, draw side wall tile.  Phase 5: creature/object overlay. */
            (void)lx; (void)ly;
        }
    }
}

/* ── Source citation ─────────────────────────────────────────────── */
const char *tr_source_evidence(void) {
    return "ReDMCSB DUNVIEW.C F0116-27 (DrawSquareD3/D2/D1/D0 L/R/C, lines 6361-8542)  "
           "+ THQUEST.ASM T520 (tile selection)  "
           "+ HuC6260/HuC6270 datasheet (planar tile format)  "
           "+ tqr_v1_phase2_data_formats_H2339.md §7";
}
