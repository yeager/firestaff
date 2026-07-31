/*
 * csb_v22_inplace_draw_pc34.h
 *
 * CSB V2.2 source-bound in-place bitmap lookup.
 *
 * This module never draws a generic cell overlay. It can replace only an
 * already-composed F0128 command that has an authenticated source raster,
 * source palette and route-specific projection receipt. Everything else
 * remains on the original V1 path.
 *
 * Architecture:
 *   authenticated F0128 command -> route-specific admission receipt
 *     -> (category, asset_id) -> FSV22C source-derived cache entry
 *     -> source palette quantization -> original command clip
 *     -> return replacement count only for the admitted command
 *
 * When V22 is not active, or its selected material lacks a complete source
 * receipt, lookup returns NULL and the V1 draw path is unchanged.
 *
 * Source-lock: csb_v22_shape_cache_pc34.h (the cache),
 * csb_v22_modern_assets_pc34.c (manifest lookup),
 * ReDMCSB DUNVIEW.C F0111/F0115/F0128 (composition order and clips),
 * CSBWin Viewport.cpp (command ownership), and
 * scripts/firestaff_artpack_studio.py (little-endian FSV22C writer).
 *
 * Module: src/csb/csb_v22_inplace_draw_pc34.c
 * Test:   tests/test_csb_v22_inplace_draw_pc34.c
 */

#ifndef FIRESTAFF_CSB_V22_INPLACE_DRAW_PC34_H
#define FIRESTAFF_CSB_V22_INPLACE_DRAW_PC34_H

#include <stdint.h>
#include "csb_v1_viewport_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize the in-place bitmap cache. Loads the selected pack's bounded
 * FSV22C cache, whose entries are keyed by (category, asset_id). Call once
 * at startup after csb_v22_set_manifest_path() and the finished-art gate.
 *
 * Returns 1 on success (at least one bitmap cached), 0 if no assets
 * are available (V22 not installed — fallback to V1). */
int csb_v22_inplace_draw_init(void);

/* Free all cached RGBA buffers. Call once at shutdown. */
void csb_v22_inplace_draw_shutdown(void);

/* True when in-place has at least one cached bitmap. */
int csb_v22_inplace_draw_active(void);

/* Supplies the active source-owned indexed palette for V2.2's final
 * indexed composition.  CSB's PC3.4 viewport uses palette indices, not an
 * EGA cube: RGBA art must therefore be quantized to this exact runtime
 * palette before it enters the original F0128 framebuffer.  The pointer is
 * copied, so callers may reuse their frame-local palette buffer. */
int csb_v22_inplace_draw_set_indexed_palette_rgb6(
    const uint8_t rgb6[256][3]);

/* Removes the source palette binding.  This is only for shutdown and
 * data-free tests; production CSB frames provide the live palette above. */
void csb_v22_inplace_draw_clear_indexed_palette(void);

/* Get a cached RGBA bitmap by a routing candidate. This is a lookup-only
 * helper: it does not admit a replacement or paint a cell. Sets *out_w and
 * *out_h to the bitmap dimensions. It returns NULL when the candidate has
 * no mapped asset, the cache is inactive, or its material remains unbound.
 *
 * The returned pointer is owned by the in-place cache and remains
 * valid until csb_v22_inplace_draw_shutdown(). */
const uint32_t* csb_v22_inplace_get_cell_bitmap(int depth, int lateral,
                                                 int* out_w, int* out_h);

/* Lookup a route candidate's asset id. This is not a finished-art or F0128
 * admission decision; callers must use the command-level draw entry point.
 * The returned string is owned by the static mapping table. */
const char* csb_v22_inplace_get_cell_asset_id(int depth, int lateral);

/* Direct manifest category + asset_id lookup against the loaded RGBA cache.
 * Returns NULL if the in-place cache is inactive or the tuple is unknown. */
const uint32_t* csb_v22_inplace_get_bitmap_by_id(const char* category,
                                                  const char* asset_id,
                                                  int* out_w, int* out_h);

/* Replace one already-composed, source-owned F0128 command in place. Only
 * commands admitted by csb_v22_admit_f0128_door_projection_pc34() can paint;
 * this currently means the proven D1/D2 door routes. The caller invokes this
 * immediately after the source command, preserving later F0115 overlays. */
int csb_v22_inplace_render_f0128_command(
    const CSB_V1_ViewportRuntimeDrawCommandPc34* source_command,
    unsigned char* framebuffer, int fbW, int fbH);

/* Source evidence for tests/probes. */
const char* csb_v22_inplace_draw_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V22_INPLACE_DRAW_PC34_H */
