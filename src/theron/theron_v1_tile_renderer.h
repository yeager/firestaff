/*
 * theron_v1_tile_renderer.h — Theron's Quest V1 tile-rendering seam.
 *
 * The original tile decoder/rasterizer is intentionally not part of the
 * repository until Track 02 supplies an authenticated tile-bank/material
 * binding.  The production no-op seam keeps these declarations available to
 * the runtime and to source-lock gates without authorizing generated pixels.
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

#define TR_VP_DEPTH          4
#define TR_SQ_SIZE           16
#define TR_TILE_DIM          8
#define TR_X_MARGIN          32
#define TR_Y_MARGIN          16
#define TR_TILE_FALLBACK     (-1)

int tr_tile_for_square(int square_type, int depth, int is_wall);

void tr_render_dungeon(TQR_PlanarFramebuffer *fb,
                       const TQR_PaletteState *palette,
                       Theron_V1_World *world);

void tr_decode_tile_row(uint8_t *out_row,
                        const uint8_t *src_row,
                        int bpp);

void tr_decode_tile(uint8_t *out64,
                    const uint8_t *src,
                    int bpp);

const uint8_t *tr_get_tile_data(const TQR_PaletteState *palette,
                                int tile_index);

void tr_clear_fb(TQR_PlanarFramebuffer *fb, uint8_t color_index);

const char *tr_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TILE_RENDERER_H */
