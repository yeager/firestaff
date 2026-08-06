/*
 * dm2_v22_inplace_draw_pc34.h
 *
 * DM2 V2.2 GPU render path: V22 modern-art IN-PLACE bitmap lookup.
 *
 * Local V2.2 art caches are not original DM2 data. This compatibility API is
 * intentionally no-draw: it neither opens operator-provided RGBA cache files
 * nor returns pixels. V1's verified GDAT route remains the only material
 * owner until a separately source-proven policy is available.
 *
 * Source-lock: dm2_v22_shape_cache_pc34.h (the cache),
 * dm2_v22_modern_assets_pc34.c (manifest lookup),
 * dm2_v22_render_overlay_pc34.c (sibling overlay path),
 * include/dm1_v2_shape_runtime_pc34.h (shape variant enum),
 * SKULL.ASM T520/T560/T600 (composition order).
 *
 * Module: src/dm1v2/dm2_v22_inplace_draw_pc34.c
 * Test:   tests/test_dm2_v22_inplace_draw_pc34.c
 */

#ifndef FIRESTAFF_DM2_V22_INPLACE_DRAW_PC34_H
#define FIRESTAFF_DM2_V22_INPLACE_DRAW_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compatibility no-op. Always returns 0 and does not open local art files. */
int dm2_v22_inplace_draw_init(void);

/* Compatibility no-op. */
void dm2_v22_inplace_draw_shutdown(void);

/* Always false: no unproven local RGBA art is admitted. */
int dm2_v22_inplace_draw_active(void);

/* Get the cached RGBA bitmap for a V22 cell. depth in {1,2,3},
 * lateral in {-1,0,1}. Sets *out_w, *out_h to the bitmap dimensions.
 * Returns NULL if the cell has no V22 shape, the shape has no
 * mapped asset_id, or in-place has not been initialized.
 *
 * The returned pointer is owned by the in-place cache and remains
 * valid until dm2_v22_inplace_draw_shutdown(). */
const uint32_t* dm2_v22_inplace_get_cell_bitmap(int depth, int lateral,
                                                 int* out_w, int* out_h);

/* Lookup the asset_id (in modern_asset_manifest.json) that the
 * current V22 cell at (depth, lateral) maps to. Returns NULL if
 * the cell has no mapping. The returned string is owned by the
 * static mapping table and remains valid for the program lifetime.
 *
 * This is the seam between the shape variant enum and the asset
 * pack. The mapping is intentionally conservative in this first
 * cut: walls all map to wall_dm2_temple_01 (the most common carved
 * stone), floors map by tile pattern, creatures map by silhouette
 * tag. Per-cell refinement (e.g., mossy walls for slime zones) is
 * a follow-up. */
const char* dm2_v22_inplace_get_cell_asset_id(int depth, int lateral);

/* dm2_v22_inplace_get_bitmap_by_id — direct category + asset_id
 * hash lookup against the loaded RGBA cache. Returns the bitmap
 * pointer + width + height on hit, NULL when the cache is not
 * loaded or the (category, asset_id) tuple is unknown.
 *
 * The category strings are the manifest categories documented in
 * include/dm2_v22_modern_assets_pc34.h: "wall_shapes",
 * "floor_shapes", "creature_shapes", "ui_chrome",
 * "champion_portraits", "door_shapes".
 *
 * The asset_id strings are the keys from the modern_asset_manifest.json
 * entries (e.g. "wall_dm2_temple_01", "floor_dm2_outdoor_01",
 * "creature_dm2_brigand_01", "sky_dm2_outdoor_01").
 *
 * The returned pointer is owned by the in-place cache and remains
 * valid until dm2_v22_inplace_draw_shutdown(). */
const uint32_t* dm2_v22_inplace_get_bitmap_by_id(const char* category,
                                                  const char* asset_id,
                                                  int* out_w, int* out_h);

/* dm2_v22_inplace_render_pass — paints the cached V22 bitmaps into
 * the framebuffer at the DM2 4x3 cell rectangles (same coords as the
 * overlay pass: D1/D2/D3 × L/C/R). For each V22-active cell with a
 * cached bitmap, nearest-neighbor scales the bitmap into the cell
 * rect and writes to framebuffer[y*fbW+x] (single-byte indexed mode).
 *
 * This is the V22 in-place equivalent of dm2_v22_render_overlay:
 * the caller (typically the M11 game view) invokes this AFTER V1
 * rendering, so the V22 art replaces the V1 sprite at the same
 * Z-order. The bitmap's color_tint-tinted average is mapped to the
 * nearest EGA palette index for the indexed framebuffer.
 *
 * Returns the number of cells painted. */
int dm2_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH);

/* Source evidence for tests/probes. */
const char* dm2_v22_inplace_draw_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V22_INPLACE_DRAW_PC34_H */
