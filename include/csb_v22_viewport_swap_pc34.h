/*
 * csb_v22_viewport_swap_pc34.h
 *
 * CSB V2.2 GPU render path: bounded PER-CELL modern-art swap for the
 * 9-square (3 depth x 3 lateral) CSB viewport.
 *
 * This module is the next step after csb_v22_inplace_draw_pc34.{c,h}:
 * it pairs the per-cell V22 cache with the CSB viewport state and
 * provides a single render-pass seam the CSB V1 draw pipeline can
 * consult AFTER V1 has finished drawing its placeholder cells.
 *
 * CSB viewport layout (CSBWin/Viewport.cpp:7290):
 *   - 3x3 grid: D0/D1/D2 (closest .. farthest) x L/C/R
 *   - 320x200 logical cells (V1), rendered at 1920x1080 in V2.2 mode
 *
 * The 9-cell rect table is shared with csb_v22_inplace_draw_pc34.c
 * (CSB_V22_CellRect coords). The per-cell render uses the same
 * (depth, lateral) -> ShapeType -> asset_id mapping table as the
 * underlying shape cache, but adds a per-cell discriminator so the
 * renderer can resolve a raw CSB cell type (0..255) into a
 * CSB_V22_ShapeType WITHOUT going through the sibling shape cache.
 *
 * The render pass is gated on:
 *   - V22 modern pack installed (csb_v22_get_installed()==1)
 *   - best_available_shape_source(3) == CSB_V22_SHAPE_SOURCE_V2_MODERN
 *   - cache file loaded (csb_v22_inplace_draw_active()==1)
 *   - shape cache populated (csb_v22_viewport_swap_populated()==1)
 *
 * When ANY of those conditions fails, the render pass is a no-op
 * and the V1 placeholder path stays in charge — V1 source ownership
 * is preserved.
 *
 * Per-cell asset_id mapping (first cut, conservative):
 *   Walls (any)                 -> wall_dungeon_01
 *   Floor PLAIN                 -> floor_plain_01
 *   Floor CRACKED/MOSSY         -> floor_cracked_01
 *   Floor PIT                   -> floor_pit_01
 *   Stairs (up/down)            -> floor_stairs_01
 *   Ceiling PLAIN/VAULTED       -> ceiling_plain_01
 *   Creature (any)              -> creature_chaos_fiend_01
 *   Item (floor/projectile)     -> creature_chaos_fiend_01
 *   Door                        -> door_iron_portcullis_01
 *   Field TELEPORTER            -> field_teleporter_01
 *   Field FLUXCAGE/CHAOS_RIFT   -> field_chaos_rift_01
 *   Field EXPLOSION             -> field_explosion_01
 *   UI chrome                   -> ui_panel_01
 *   UI portrait                 -> champion_warrior_csb
 *   UI message log              -> ui_message_log_01
 *   UI inventory grid           -> ui_inventory_01
 *   UI DSA rune (CSB-only)      -> chaos_rune_01
 *   Narrative PRISON_DOOR       -> door_prison_01
 *   Narrative DSA_SCROLL        -> dsa_scroll_01
 *   Narrative LORD_ORDER        -> statue_lord_order_01
 *   Narrative CHAOS_RUNE        -> chaos_rune_marker_01
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

/* CSB V22 shape-type discriminator. Parallel to the existing
 * CSB_V22_ShapeType in csb_v22_shapes.h but kept local here so the
 * per-cell swap table is self-contained and the discriminator does
 * not depend on the full shape-book init (which needs V1 graphics).
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

/* csb_v22_swap_shape_for_cell — bounded discriminator that maps a
 * raw CSB cell type (0..255, mirroring CSB V1's square type encoding)
 * to a CSB_V22_SwapShapeType. The mapping is intentionally coarse in
 * this first cut: walls, plain/cracked/mossy/pit/stairs floors,
 * creatures, and CSB-only narrative shapes. Per-cell variant decoding
 * (e.g. corner vs straight wall) is a follow-up once a real CSB V22
 * shape book lands.
 *
 * The direction parameter is preserved in the API even though the
 * first cut does not consume it (per-direction shape variants are a
 * follow-up). */
CSB_V22_SwapShapeType csb_v22_swap_shape_for_cell(uint8_t raw_cell_type,
                                                    uint8_t direction);

/* csb_v22_swap_asset_id_for_shape — per-cell asset_id string for a
 * given shape type. Returns NULL for CSB_V22_SWAP_SHAPE_NONE.
 *
 * The returned string is owned by the static asset_id table in
 * csb_v22_viewport_swap_pc34.c and remains valid for the program
 * lifetime. */
const char* csb_v22_swap_asset_id_for_shape(CSB_V22_SwapShapeType shape);

/* csb_v22_swap_category_for_shape — manifest category ("wall_shapes"
 * / "floor_shapes" / "creature_shapes" / "door_shapes" / ...) used to
 * resolve a CSB_V22_SwapShapeType to a (category, asset_id) pair via
 * csb_v22_inplace_get_bitmap_by_id. Returns NULL for SHAPE_NONE.
 *
 * The returned string is owned by the static mapping table and
 * remains valid for the program lifetime. */
const char* csb_v22_swap_category_for_shape(CSB_V22_SwapShapeType shape);

/* csb_v22_viewport_swap_update — populate the bounded per-cell cache
 * from a 3x3 array of raw cell types (D0..D2, L/C/R order). direction
 * is the party facing (0..3 N/E/S/W).
 *
 * When V22 is not the active presentation mode (csb_v22_get_installed()
 * == 0 or best_available_shape_source(3) != V2_MODERN), the cache is
 * still updated but csb_v22_viewport_swap_active() returns 0 and the
 * render pass becomes a no-op. */
void csb_v22_viewport_swap_update(int direction,
                                    const unsigned char raw_cells[3][3]);

/* csb_v22_viewport_swap_active — 1 if the swap is loaded and the
 * presentation mode + asset state permit a render pass. 0 otherwise. */
int csb_v22_viewport_swap_active(void);

/* csb_v22_viewport_swap_populated — 1 if at least one update has
 * been called. Render pass still requires csb_v22_viewport_swap_active(). */
int csb_v22_viewport_swap_populated(void);

/* csb_v22_viewport_swap_render — paints the cached V22 bitmaps into
 * the framebuffer at the CSB 9-square cell rectangles (D0..D2 x
 * L/C/R). For each V22-active cell with a cached bitmap, nearest-
 * neighbor scales the bitmap into the cell rect and writes to
 * framebuffer[y*fbW+x] (single-byte indexed mode).
 *
 * This is the V22 in-place equivalent of the (sibling) overlay
 * path: the caller (typically the M11 game view) invokes this AFTER
 * V1 rendering, so the V22 art replaces the V1 sprite at the same
 * Z-order.
 *
 * Returns the number of cells painted. Returns 0 when V22 is not
 * active, the cache is not populated, the framebuffer is NULL, or
 * the dimensions are zero. */
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
