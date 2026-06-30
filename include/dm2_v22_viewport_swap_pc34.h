/*
 * dm2_v22_viewport_swap_pc34.h
 *
 * DM2 V2.2 GPU render path: bounded PER-CELL modern-art swap.
 *
 * This module is the next step after dm2_v22_inplace_draw_pc34.{c,h}:
 * it pairs the per-cell V22 cache with the DM2 viewport state and
 * provides a render-pass seam that the V1 DM2 draw pipeline can
 * consult AFTER V1 has finished drawing its placeholder cells.
 *
 * Two viewport types are explicitly supported, matching the two
 * SKULL.ASM ticks that draw the DM2 view:
 *
 *   - T560 indoor dungeon (DM2 4x3 grid projection: D0..D2 x L/C/R)
 *     -> uses dm2_v22_kCellRects (640x360 cells in 1920x1080)
 *
 *   - T600 outdoor (sky top half + ground bottom half + per-cell
 *     ground variations L/C/R)
 *     -> uses dm2_v22_kOutdoorCellRects (sky band + 3 ground cells)
 *
 * Both paths share the same per-cell asset_id resolution table
 * (Dm2_V22_ShapeType -> asset_id) but expose a different rect
 * layout to the renderer. The render pass is gated on:
 *   - V22 modern pack installed (dm2_v22_get_installed()==1)
 *   - presentation_mode_index == 3 (M12_PRESENTATION_V22_MODERN)
 *   - best_available_shape_source(3) == DM2_V22_SHAPE_SOURCE_V2_MODERN
 *   - cache file loaded (dm2_v22_inplace_draw_active()==1)
 *   - shape cache populated (dm2_v22_viewport_swap_populated()==1)
 *
 * When ANY of those conditions fails, the render pass is a no-op
 * and the V1 placeholder path stays in charge — V1 source ownership
 * is preserved.
 *
 * Per-cell asset_id mapping (first cut, conservative):
 *   Walls (any) -> wall_dm2_temple_01
 *   Floor PLAIN/CRACKED/MOSSY -> floor_dm2_outdoor_01
 *   Floor pit -> floor_dm2_pit_01
 *   Stairs (up/down) -> floor_dm2_stairs_01
 *   Creatures (any) -> creature_dm2_brigand_01
 *   Outdoor sky -> sky_dm2_outdoor_01
 *   Outdoor ground (any) -> ground_dm2_outdoor_01
 *   Outdoor building/wall -> wall_dm2_outdoor_01
 *   Outdoor tree -> tree_dm2_outdoor_01
 *
 * Source-lock:
 *   SKULL.ASM T520/T560/T600 (DM2 viewport ticks: indoor T560 + outdoor T600)
 *   ReDMCSB DUNVIEW.C:2962-3070 (outdoor sky/ground composition order)
 *   ReDMCSB DUNVIEW.C:4351-4382 F0112 (ceiling pit — outdoor has no ceiling)
 *   include/dm2_v22_modern_assets_pc34.h (asset pack paths + flags)
 *   include/dm2_v22_inplace_draw_pc34.h (cache bitmap lookup)
 *   include/dm2_v22_shape_cache_pc34.h (raw cell type store)
 *   dm2_v22_inplace_draw_pc34.c (cache file format + bitmap blit)
 *
 * Module: src/dm2/dm2_v22_viewport_swap_pc34.c
 * Probe:  probes/firestaff_dm2_v22_inplace_render_probe.c
 */

#ifndef FIRESTAFF_DM2_V22_VIEWPORT_SWAP_PC34_H
#define FIRESTAFF_DM2_V22_VIEWPORT_SWAP_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2 V22 shape-type discriminator. Parallel to CSB_V22_ShapeType in
 * csb_v22_shapes.h. Drives the per-cell asset_id selection; the
 * enum is kept narrow so the per-cell swap table stays readable.
 *
 * Values are grouped:
 *   0..15  walls
 *   16..31 floors (plain/cracked/mossy/pit/stairs)
 *   32..47 creatures / items
 *   48..63 doors / fields / openings
 *   64..79 outdoor (sky / ground / building / tree) */
typedef enum {
    DM2_V22_SHAPE_WALL_STRAIGHT      = 0,
    DM2_V22_SHAPE_WALL_CORNER_INNER  = 1,
    DM2_V22_SHAPE_WALL_CORNER_OUTER  = 2,
    DM2_V22_SHAPE_WALL_ALCOVE        = 3,
    DM2_V22_SHAPE_WALL_DOORWAY       = 4,
    DM2_V22_SHAPE_WALL_INSCRIPTION   = 5,

    DM2_V22_SHAPE_FLOOR_PLAIN        = 16,
    DM2_V22_SHAPE_FLOOR_CRACKED      = 17,
    DM2_V22_SHAPE_FLOOR_MOSSY        = 18,
    DM2_V22_SHAPE_FLOOR_PIT          = 19,
    DM2_V22_SHAPE_FLOOR_STAIRS_UP    = 20,
    DM2_V22_SHAPE_FLOOR_STAIRS_DOWN  = 21,
    DM2_V22_SHAPE_CEILING_PLAIN      = 22,

    DM2_V22_SHAPE_CREATURE           = 32,
    DM2_V22_SHAPE_CREATURE_PROJECTILE = 33,

    DM2_V22_SHAPE_DOOR               = 48,
    DM2_V22_SHAPE_FIELD_TELEPORTER   = 49,
    DM2_V22_SHAPE_FIELD_OPENING      = 50,

    DM2_V22_SHAPE_OUTDOOR_SKY        = 64,
    DM2_V22_SHAPE_OUTDOOR_GROUND     = 65,
    DM2_V22_SHAPE_OUTDOOR_BUILDING   = 66,
    DM2_V22_SHAPE_OUTDOOR_TREE       = 67,
    DM2_V22_SHAPE_OUTDOOR_HORIZON    = 68,

    DM2_V22_SHAPE_NONE               = 255
} Dm2_V22_ShapeType;

/* Outdoor cell rect — same DM2 1920x1080 canvas as the indoor
 * 4x3 grid, but uses a sky/ground split:
 *
 *   - sky band         : top half of the viewport (sky gradient)
 *   - horizon strip    : 1-line transition at the sky/ground boundary
 *   - ground L/C/R     : three ground cells below the horizon
 *
 * The L/C/R cells are intended to receive different ground modern
 * art (grass L, dirt C, grass R by default) so the per-cell swap
 * is visible on the outdoor path too. */
typedef struct {
    int x, y, w, h;
} DM2_V22_OutdoorCellRect;

/* 1920x1080 outdoor layout (matches dm2_v1_outdoor_renderer sky/ground
 * split). The exact pixel values match the V1 sky-half / ground-half
 * split in dm2_v1_viewport_render (DM2_VP_HEIGHT/2). */
extern const DM2_V22_OutdoorCellRect dm2_v22_kOutdoorCellRects[3];

/* dm2_v22_shape_for_cell — bounded discriminator that maps the
 * DM2 raw cell type (0..255, indoor) or outdoor flag (T600) to
 * a Dm2_V22_ShapeType. The mapping is intentionally coarse in this
 * first cut: walls, plain/cracked/mossy/pit/stairs floors, and
 * creatures. Sub-cell variant decoding (e.g. corner vs straight
 * wall) is a follow-up once a real DM2 V22 shape book lands.
 *
 * This discriminator is intentionally POSITION-AGNOSTIC: it does
 * not look at depth/lateral. The position-aware sibling
 * (dm2_v22_shape_for_cell_pos) does that. Both are kept because
 * some callers (debug inspectors, future offline mappers) want a
 * per-raw_cell_type shape only. */
Dm2_V22_ShapeType dm2_v22_shape_for_cell(uint8_t raw_cell_type,
                                          uint8_t direction);

/* dm2_v22_shape_for_cell_pos — bounded position-aware indoor T560
 * discriminator. Maps (raw_cell_type, direction, depth, lateral)
 * into a Dm2_V22_ShapeType.
 *
 * depth is the DM2 4×3 depth index: 0 = D0 (closest), 1 = D1 (middle),
 * 2 = D2 (farthest). lateral is the column index: -1 = left, 0 = center,
 * +1 = right. direction is the party facing (0..3) and is preserved
 * for future per-direction shape book usage; the bounded first cut
 * uses it only to remember the facing but does not flip variants on
 * facing because DUNVIEW.C's per-cell variant is governed by which
 * of the 9 cells the party is looking at (D{n}L{R} / D{n}C), not by
 * the party facing itself.
 *
 * Refinements vs dm2_v22_shape_for_cell:
 *   - Walls in the LEFT column (lateral == -1)  -> WALL_CORNER_INNER
 *   - Walls in the RIGHT column (lateral == +1) -> WALL_CORNER_OUTER
 *   - Walls in the CENTER column (lateral == 0) -> WALL_STRAIGHT
 *   - Floors in D0 (closest depth)  -> FLOOR_PLAIN
 *   - Floors in D1 (middle depth)   -> FLOOR_CRACKED
 *   - Floors in D2 (farthest depth) -> FLOOR_MOSSY
 *   - Pit / stairs / creatures / doors / fields keep the
 *     position-agnostic discriminator so the dungeon feel of those
 *     gameplay markers is not lost when direction changes.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:6239-6675 (per-cell D3L2/D3C/D3R2/
 * D2L/D2R/D1L2/D1C/D1R2/D0L/D0R wall/floor zone tables) + DUNVIEW.C:
 * 2962-3070 (the per-cell composition order that owns the 4×3 grid).
 *
 * This function is read-only — it never touches V1 state. */
Dm2_V22_ShapeType dm2_v22_shape_for_cell_pos(uint8_t raw_cell_type,
                                               uint8_t direction,
                                               int depth,
                                               int lateral);

/* Per-cell asset_id string for a given shape type. Returns NULL
 * for DM2_V22_SHAPE_NONE. The returned string is owned by the
 * static asset_id table in dm2_v22_viewport_swap_pc34.c and
 * remains valid for the program lifetime. */
const char* dm2_v22_asset_id_for_shape(Dm2_V22_ShapeType shape);

/* dm2_v22_viewport_swap_update — populate the bounded per-cell
 * cache from a 3x3 array of raw cell types (D0..D2, L/C/R order)
 * for indoor T560 cells. direction is the party facing (0..3).
 *
 * When is_outdoor is non-zero, the raw_cells argument is ignored
 * (outdoor cells are filled with sky/ground/horizon asset_ids
 * directly) and the cache is marked outdoor-populated.
 *
 * When V22 is not the active presentation mode (dm2_v22_get_installed()
 * == 0 or best_available_shape_source(3) != V2_MODERN), the cache
 * is updated but dm2_v22_viewport_swap_active() returns 0 and the
 * render pass becomes a no-op. */
void dm2_v22_viewport_swap_update(int direction,
                                   const unsigned char raw_cells[3][3],
                                   int is_outdoor);

/* dm2_v22_viewport_swap_active — 1 if the swap is loaded and the
 * presentation mode + asset state permit a render pass. 0 otherwise. */
int dm2_v22_viewport_swap_active(void);

/* dm2_v22_viewport_swap_populated — 1 if at least one update has
 * been called. Render pass still requires dm2_v22_viewport_swap_active(). */
int dm2_v22_viewport_swap_populated(void);

/* dm2_v22_viewport_swap_render — paints the cached V22 bitmaps into
 * the framebuffer at the DM2 cell rectangles. For is_outdoor == 0
 * uses the indoor T560 4x3 grid (D0..D2 x L/C/R). For is_outdoor != 0
 * uses the outdoor T600 sky/ground split.
 *
 * Returns the number of cells painted. Returns 0 when V22 is not
 * active, the cache is not populated, the framebuffer is NULL, or
 * the dimensions are zero. */
int dm2_v22_viewport_swap_render(unsigned char* framebuffer,
                                   int fbW, int fbH,
                                   int is_outdoor);

/* Per-viewport painted-cell counters (debug / test introspection).
 * Resets on dm2_v22_viewport_swap_update(). */
int dm2_v22_viewport_swap_cells_painted_indoor(void);
int dm2_v22_viewport_swap_cells_painted_outdoor(void);

/* Source evidence string for tests/probes. */
const char* dm2_v22_viewport_swap_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V22_VIEWPORT_SWAP_PC34_H */
