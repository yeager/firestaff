/*
 * csb_v22_viewport_swap_pc34.h
 *
 * Retired CSB V2.2 per-cell modern-art compatibility surface for the
 * 9-square (3 depth x 3 lateral) CSB viewport.
 *
 * Historical versions inferred modern assets from raw cell bytes and then
 * painted generic rectangles after V1. That inference has no source proof
 * for F0128 raster selection, palette, projection, clip or draw order.
 * Every mapping and render entry point therefore fails closed. The raw-cell
 * update remains solely for the sibling inspector cache.
 *
 * CSB viewport layout (CSBWin/Viewport.cpp:7290):
 *   - 3x3 grid: D0/D1/D2 (closest .. farthest) x L/C/R
 *   - 320x200 logical cells (V1), rendered at 1920x1080 in V2.2 mode
 *
 *
 * Source-lock:
 *   CSBWin/Viewport.cpp:7290  (9-square viewport layout)
 *   ReDMCSB DUNVIEW.C F0128   (CSB viewport routing)
 *   ReDMCSB DUNGEON.C:35-44   (direction step tables)
 *   include/csb_v22_shapes.h  (CSB_V22_ShapeType enum)
 *   include/csb_v22_modern_assets_pc34.h (asset pack paths + flags)
 *   include/csb_v22_inplace_draw_pc34.h (cache bitmap lookup)
 *   include/csb_v22_shape_cache_pc34.h (raw cell type store)
 *   csb_v22_inplace_draw_pc34.c (cache file format + bitmap blit)
 *
 * Module: src/csb/csb_v22_viewport_swap_pc34.c
 * Test:   tests/test_csb_v22_viewport_swap_pc34.c
 * Probe:  probes/firestaff_csb_v22_viewport_swap_probe.c
 */

#ifndef FIRESTAFF_CSB_V22_VIEWPORT_SWAP_PC34_H
#define FIRESTAFF_CSB_V22_VIEWPORT_SWAP_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Historical CSB V22 shape-type values. Parallel to the existing
 * CSB_V22_ShapeType in csb_v22_shapes.h but kept local here so the
 * legacy API remains ABI-stable. These values no longer admit artwork.
 *
 * Values mirror the CSB_V22_ShapeType ordering where the slots
 * overlap (walls 0..9, floors 10..19, ceiling 20..21, creatures/items
 * 30..42, fields 50..53, ui 60..64, narrative 70..73). CSB-only
 * narrative shapes (prison door, DSA scroll, Lord Order statue,
 * chaos rune marker) get their own slots so the per-cell swap can
 * route them to CSB-specific modern art.
 *
 * SHAPE_NONE is the sentinel for "no V22 mapping" — used by the
 * cached outdoor path (CSB does not have a T600 outdoor viewport
 * the way DM2 does, but the sentinel is kept for source-lock
 * consistency with dm2_v22_viewport_swap_pc34.h). */
typedef enum {
    CSB_V22_SWAP_SHAPE_WALL_STRAIGHT          = 0,
    CSB_V22_SWAP_SHAPE_WALL_CORNER_INNER      = 1,
    CSB_V22_SWAP_SHAPE_WALL_CORNER_OUTER      = 2,
    CSB_V22_SWAP_SHAPE_WALL_DOORWAY           = 3,
    CSB_V22_SWAP_SHAPE_WALL_ALCOVE            = 4,
    CSB_V22_SWAP_SHAPE_WALL_INSCRIPTION       = 5,

    CSB_V22_SWAP_SHAPE_FLOOR_PLAIN            = 10,
    CSB_V22_SWAP_SHAPE_FLOOR_CRACKED          = 11,
    CSB_V22_SWAP_SHAPE_FLOOR_MOSSY            = 12,
    CSB_V22_SWAP_SHAPE_FLOOR_PIT              = 13,
    CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_UP        = 14,
    CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_DOWN      = 15,
    CSB_V22_SWAP_SHAPE_FLOOR_DOOR             = 16,

    CSB_V22_SWAP_SHAPE_CEILING_PLAIN          = 20,
    CSB_V22_SWAP_SHAPE_CEILING_VAULTED        = 21,

    CSB_V22_SWAP_SHAPE_CREATURE               = 30,
    CSB_V22_SWAP_SHAPE_CREATURE_PROJECTILE    = 31,
    CSB_V22_SWAP_SHAPE_ITEM                   = 40,
    CSB_V22_SWAP_SHAPE_ITEM_FLOOR             = 41,
    CSB_V22_SWAP_SHAPE_ITEM_PROJECTILE        = 42,

    CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER       = 50,
    CSB_V22_SWAP_SHAPE_FIELD_FLUXCAGE         = 51,
    CSB_V22_SWAP_SHAPE_FIELD_EXPLOSION        = 52,
    CSB_V22_SWAP_SHAPE_FIELD_CHAOS_RIFT       = 53,

    CSB_V22_SWAP_SHAPE_UI_CHROME              = 60,
    CSB_V22_SWAP_SHAPE_UI_PORTRAIT            = 61,
    CSB_V22_SWAP_SHAPE_UI_MESSAGE_LOG         = 62,
    CSB_V22_SWAP_SHAPE_UI_INVENTORY_GRID      = 63,
    CSB_V22_SWAP_SHAPE_UI_DSA_RUNE            = 64,

    CSB_V22_SWAP_SHAPE_PRISON_DOOR            = 70,
    CSB_V22_SWAP_SHAPE_DSA_SCROLL             = 71,
    CSB_V22_SWAP_SHAPE_LORD_ORDER             = 72,
    CSB_V22_SWAP_SHAPE_CHAOS_RUNE             = 73,

    CSB_V22_SWAP_SHAPE_NONE                   = 255
} CSB_V22_SwapShapeType;

/* Returns SHAPE_NONE. A raw CSB cell byte does not authenticate a source
 * raster transaction; direction is retained for ABI compatibility. */
CSB_V22_SwapShapeType csb_v22_swap_shape_for_cell(uint8_t raw_cell_type,
                                                    uint8_t direction);

/* Returns NULL. Shape names cannot select V2.2 artwork without a source
 * command receipt. */
const char* csb_v22_swap_asset_id_for_shape(CSB_V22_SwapShapeType shape);

/* Returns NULL. Categories cannot select V2.2 artwork without a source
 * command receipt. */
const char* csb_v22_swap_category_for_shape(CSB_V22_SwapShapeType shape);

/* csb_v22_viewport_swap_update — populate the bounded per-cell cache
 * from a 3x3 array of raw cell types (D0..D2, L/C/R order). direction
 * is the party facing (0..3 N/E/S/W).
 *
 * The update never authorises a renderer. */
void csb_v22_viewport_swap_update(int direction,
                                    const unsigned char raw_cells[3][3]);

/* Always returns 0. Only the F0128 command compositor can admit a live
 * source-derived replacement. */
int csb_v22_viewport_swap_active(void);

/* 1 if at least one observation update has been called. */
int csb_v22_viewport_swap_populated(void);

/* Always returns 0 and leaves the framebuffer untouched. */
int csb_v22_viewport_swap_render(unsigned char* framebuffer,
                                   int fbW, int fbH);

/* csb_v22_viewport_swap_cells_painted — accumulated painted-cell
 * counter (debug / test introspection). Resets on every
 * csb_v22_viewport_swap_update(). */
int csb_v22_viewport_swap_cells_painted(void);

/* Source evidence string for tests/probes. */
const char* csb_v22_viewport_swap_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_VIEWPORT_SWAP_PC34_H */
