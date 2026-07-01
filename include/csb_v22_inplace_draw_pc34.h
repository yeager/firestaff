/*
 * csb_v22_inplace_draw_pc34.h
 *
 * CSB V2.2 GPU render path: V22 modern-art IN-PLACE bitmap lookup.
 *
 * This is the foundation for switching the V22 render mode from
 * "overlay" (placeholder colored rectangle on top of V1) to
 * "in-place" (replace V1 sprite with V22 PBR PNG at the same cell).
 *
 * Architecture:
 *   csb_v22_shape_cache_update -> per-cell V22 shape (params, variant)
 *     -> csb_v22_inplace_get_cell_bitmap(depth, lateral, &w, &h)
 *        -> variant -> asset_id in modern_asset_manifest.json
 *        -> asset_id -> file path (via csb_v22_get_shape_path)
 *        -> PNG decode to RGBA buffer (cached at init)
 *        -> return RGBA* + width + height
 *   csb_draw_* (9-square viewport) passes consult the bitmap and blit it instead of
 *   the V1 sprite when V22 is active.
 *
 * When V22 is NOT active (modern assets not installed or
 * presentation_mode != V22), every cell returns NULL and the
 * V1 draw path is used unchanged. This is the migration safety
 * path: in-place is opt-in.
 *
 * Source-lock: csb_v22_shape_cache_pc34.h (the cache),
 * csb_v22_modern_assets_pc34.c (manifest lookup),
 * csb_v22_render_overlay_pc34.c (sibling overlay path),
 * include/dm1_v2_shape_runtime_pc34.h (shape variant enum),
 * ReDMCSB DUNVIEW.C:6697-6816 (composition order).
 *
 * Module: src/dm1v2/csb_v22_inplace_draw_pc34.c
 * Test:   tests/test_csb_v22_inplace_draw_pc34.c
 */

#ifndef FIRESTAFF_CSB_V22_INPLACE_DRAW_PC34_H
#define FIRESTAFF_CSB_V22_INPLACE_DRAW_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the in-place bitmap cache. Loads every PNG referenced
 * by ~/.firestaff/assets/csb/modern/modern_asset_manifest.json into
 * RGBA buffers keyed by (category, asset_id). Call once at startup
 * after csb_v22_set_manifest_path() and csb_v22_validate_manifest().
 *
 * Returns 1 on success (at least one bitmap cached), 0 if no assets
 * are available (V22 not installed — fallback to V1). */
int csb_v22_inplace_draw_init(void);

/* Free all cached RGBA buffers. Call once at shutdown. */
void csb_v22_inplace_draw_shutdown(void);

/* True when in-place has at least one cached bitmap. */
int csb_v22_inplace_draw_active(void);

/* Get the cached RGBA bitmap for a V22 cell. depth in {0,1,2},
 * lateral in {-1,0,1}. Sets *out_w, *out_h to the bitmap dimensions.
 * Returns NULL if the cell has no V22 shape, the shape has no
 * mapped asset_id, or in-place has not been initialized.
 *
 * The returned pointer is owned by the in-place cache and remains
 * valid until csb_v22_inplace_draw_shutdown(). */
const uint32_t* csb_v22_inplace_get_cell_bitmap(int depth, int lateral,
                                                 int* out_w, int* out_h);

/* Lookup the asset_id (in modern_asset_manifest.json) that the
 * current V22 cell at (depth, lateral) maps to. Returns NULL if
 * the cell has no mapping. The returned string is owned by the
 * static mapping table and remains valid for the program lifetime.
 *
 * This is the seam between the shape variant enum and the asset
 * pack. The mapping is intentionally conservative in this first
 * cut: walls all map to wall_dungeon_01 (the most common carved
 * stone), floors map by tile pattern, creatures map by silhouette
 * tag. Per-cell refinement (e.g., mossy walls for slime zones) is
 * a follow-up. */
const char* csb_v22_inplace_get_cell_asset_id(int depth, int lateral);

/* Direct manifest category + asset_id lookup against the loaded RGBA cache.
 * Returns NULL if the in-place cache is inactive or the tuple is unknown. */
const uint32_t* csb_v22_inplace_get_bitmap_by_id(const char* category,
                                                  const char* asset_id,
                                                  int* out_w, int* out_h);

/* csb_v22_inplace_render_pass — paints the cached V22 bitmaps into
 * the framebuffer at the CSB 3x3 cell rectangles (D0/D1/D2 × L/C/R).
 * For each V22-active cell with a
 * cached bitmap, nearest-neighbor scales the bitmap into the cell
 * rect and writes to framebuffer[y*fbW+x] (single-byte indexed mode).
 *
 * This is the V22 in-place equivalent of csb_v22_render_overlay:
 * the caller (typically the M11 game view) invokes this AFTER V1
 * rendering, so the V22 art replaces the V1 sprite at the same
 * Z-order. The bitmap's color_tint-tinted average is mapped to the
 * nearest EGA palette index for the indexed framebuffer.
 *
 * Returns the number of cells painted. */
int csb_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH);

/* Source evidence for tests/probes. */
const char* csb_v22_inplace_draw_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_INPLACE_DRAW_PC34_H */
