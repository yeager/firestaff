/*
 * dm2_v22_inplace_draw_pc34.c
 *
 * DM2 V2.2 GPU render compatibility boundary.
 *
 * The former in-place cache accepted locally generated RGBA art and invented
 * a wall/floor/creature mapping for it. Those bytes have no original DM2
 * GDAT category/index/field, palette or placement receipt, so they must not
 * be read into the running game. Every public entry point is retained as an
 * explicit no-draw boundary until a separately source-proven material policy
 * exists.
 *
 * Source-lock: dm2_v22_shape_cache_pc34.h (the cache),
 * dm2_v22_modern_assets_pc34.c (manifest path resolution),
 * dm2_v22_shape_cache_pc34.c (sibling cache + DM2_V22_CellRect coords),
 * include/dm1_v2_shape_runtime_pc34.h (shape variant enum),
 * SKULL.ASM T520/T560/T600 (composition order).
 */

#include "dm2_v22_inplace_draw_pc34.h"

#include <stddef.h>

int dm2_v22_inplace_draw_init(void) {
    /* A locally authored cache is neither original game data nor a verified
     * transformation of it. Do not open it. */
    return 0;
}

void dm2_v22_inplace_draw_shutdown(void) {
}

int dm2_v22_inplace_draw_active(void) {
    return 0;
}

const uint32_t* dm2_v22_inplace_get_cell_bitmap(int depth, int lateral,
                                                 int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    (void)depth;
    (void)lateral;
    /* DM2-GDAT-FB-07: synthetic V22 RGBA assets are not dungeon material.
     * skproject routes the active map's MapGraphicsStyle through GDAT
     * GRAPHICSSET/WALL_GFX/FLOOR_GFX/DOORS/CREATURES before it draws. The
     * V1 viewport owns that route, so this post-V1 cache has no drawable
     * cell until a separately selected, non-original policy exists. */
    return NULL;
}

const char* dm2_v22_inplace_get_cell_asset_id(int depth, int lateral) {
    (void)depth;
    (void)lateral;
    return NULL;
}

/* dm2_v22_inplace_get_bitmap_by_id — direct category + asset_id
 * hash lookup against the loaded cache. Used by
 * dm2_v22_viewport_swap_pc34.c to resolve per-cell asset_ids
 * without going through the sibling shape cache.
 *
 * The category strings are the manifest categories documented in
 * include/dm2_v22_modern_assets_pc34.h. The asset_id strings are
 * the keys from modern_asset_manifest.json entries. */
const uint32_t* dm2_v22_inplace_get_bitmap_by_id(const char* category,
                                                  const char* asset_id,
                                                  int* out_w, int* out_h) {
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    (void)category;
    (void)asset_id;
    /* DM2-GDAT-FB-07: keeping this lookup live would let a later caller
     * bypass the no-draw swap pass and install local cache pixels directly.
     * SKProject selects live viewport material from GRAPHICS.DAT GDAT; this
     * cache has no category/index/field or raw-byte receipt and is therefore
     * diagnostic storage only. */
    return NULL;
}

int dm2_v22_inplace_render_pass(unsigned char* framebuffer, int fbW, int fbH) {
    (void)framebuffer;
    (void)fbW;
    (void)fbH;
    return 0;
}

const char* dm2_v22_inplace_draw_source_evidence(void) {
    return "dm2_v22_shape_cache_pc34.c (per-cell V22 shape cache); "
           "dm1_v2_modern_assets_pc34.c (manifest path resolution); "
           "skproject/SKWINSPX/src/v4/skcore.cpp:2284-2334 (MapGraphicsStyle "
           "and DRAW_MAP_CHIP); skguidrw.cpp:6621-6781 (GRAPHICSSET draw); "
           "dm2_v1_boot_viewport_asset_fetch (active-map GDAT provider); "
           "DM2-GDAT-FB-07 (synthetic RGBA cache is explicit no-draw).";
}
