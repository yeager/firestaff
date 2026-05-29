/*
 * theron_v1_tile_renderer.h — Theron's Quest V1 Phase 4: Tile Renderer
 *
 * 2D tile-based dungeon renderer for Theron.  Theron uses a tile grid
 * (not the classic 3D perspective view used in DM1/CSB/DM2).  This module
 * implements the tile selection, decoding, and rasterisation pipeline.
 *
 * Architecture:
 *   - View cone: 4 depths (D0..D3), 3 columns (left/center/right)
 *   - Square type → tile index via deterministic lookup table
 *   - PC Engine 8×8 planar tiles decoded to indexed bitmap
 *   - 2× horizontal replication to fill 16px logical dungeon squares
 *   - Deterministic fallback: missing tile → palette entry 7 (mid-gray)
 *
 * Source-lock references:
 *   ReDMCSB DUNVIEW.C F0116_DUNGEONVIEW_DrawSquareD3L (line 6361)
 *   ReDMCSB DUNVIEW.C F0124_DUNGEONVIEW_DrawSquareD1C (line 7727)
 *   ReDMCSB DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C (line 8164)
 *   THQUEST.ASM T520   — viewport tile selection
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §7
 */

#ifndef THERON_V1_TILE_RENDERER_H
#define THERON_V1_TILE_RENDERER_H

#include "theron_v1_palette.h"
#include "theron_v1_viewport.h"
#include "theron_v1_world.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Tile renderer configuration ─────────────────────────────────── */
#define TR_VP_DEPTH          4  /* D0..D3 (near to far)              */
#define TR_SQ_SIZE          16  /* logical dungeon square in pixels */
#define TR_TILE_DIM          8  /* raw PC Engine tile dimension      */
#define TR_X_MARGIN         32  /* center 192-wide view in 256-wide */
#define TR_Y_MARGIN         16  /* center 192-tall view in 224-tall */

/* ── Tile atlas layout ────────────────────────────────────────────── */
/*
 * Tile bank layout (from THQUEST.ASM T520 and Track 02 format):
 *   0-127:    wall tiles (2bpp, palette group 0)
 *   128-255:  floor tiles (2bpp, palette group 0)
 *   256-383:  object tiles (2bpp, palette group 2)
 *   384-511:  creature tiles (4bpp, palette group 1)
 *   512-639:  UI tile set (2bpp, palette group 3)
 *   640-767:  font tiles (2bpp, palette group 4)
 */

/* ── Square-type → tile-index lookup ──────────────────────────────── */
/*
 * Deterministic tile index per square type, depth, and wall flag.
 * Source: THQUEST.ASM T520 tile bank selection.
 * Fallback: -1 = flat-color (palette entry 7).
 */
#define TR_TILE_FALLBACK (-1)

int tr_tile_for_square(int square_type, int depth, int is_wall);

/* ── Tile rendering ──────────────────────────────────────────────── */

/* Render a 2D tile grid dungeon view into the given planar framebuffer.
 *
 * Uses the party position and direction from world state to build
 * the view cone: D3 (far) → D0 (near) drawn in painter's algorithm order.
 *
 * Screen layout (256×224 planar fb, letterboxed 192×192 view):
 *   D3:  y = y_margin + 0..47    (48px = 3 squares × 16px)
 *   D2:  y = y_margin + 48..95
 *   D1:  y = y_margin + 96..143
 *   D0:  y = y_margin + 144..191
 *   Left col: x = x_margin + 0..63    (4 squares × 16px wide)
 *   Center:   x = x_margin + 64..127
 *   Right:    x = x_margin + 128..191
 *
 * Source: ReDMCSB DUNVIEW.C F0116-27 (DrawSquareD3/D2/D1/D0 L/R/C).
 */
void tr_render_dungeon(TQR_PlanarFramebuffer *fb,
                       const TQR_PaletteState *palette,
                       Theron_V1_World *world);

/* Decode an 8×8 planar tile row into 8 indexed pixels.
 * bpp = 2: src_row has 2 bytes (bitplane 0, bitplane 1)
 * bpp = 4: src_row has 4 bytes (bitplane 0..3)
 * LSB = leftmost pixel (HuC6260 native, NOT flipped).
 *
 * Source: HuC6260 VDC datasheet — planar bitmap format.
 */
void tr_decode_tile_row(uint8_t *out_row,
                       const uint8_t *src_row,
                       int bpp);

/* Decode a full 8×8 planar tile into a 64-byte linear indexed bitmap.
 * Calls tr_decode_tile_row() for each of the 8 rows.
 */
void tr_decode_tile(uint8_t *out64,
                    const uint8_t *src,
                    int bpp);

/* Get decoded tile data for a given tile index in the palette state.
 * Returns pointer to a static 64-byte buffer (not thread-safe).
 * Returns NULL if tile is out of range or not loaded.
 *
 * Source: THQUEST.ASM T400 (tile bank loading).
 */
const uint8_t *tr_get_tile_data(const TQR_PaletteState *palette,
                                  int tile_index);

/* Clear the planar framebuffer with a solid palette index. */
void tr_clear_fb(TQR_PlanarFramebuffer *fb, uint8_t color_index);

/* ── Source citation ─────────────────────────────────────────────── */
const char *tr_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TILE_RENDERER_H */
