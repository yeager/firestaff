/*
 * Production seam for Theron V1 tile rendering.
 *
 * Track 02 currently proves only the initial grid bytes, not tile-bank
 * semantics, so production exposes no tile pixels until that handoff is
 * decoded. The former inferred renderer and its synthetic probe have been
 * retired; this is the only implementation of the seam.
 */

#include "theron_v1_tile_renderer.h"
#include <string.h>

int tr_tile_for_square(int square_type, int depth, int is_wall)
{
    (void)square_type; (void)depth; (void)is_wall;
    return TR_TILE_FALLBACK;
}

void tr_render_dungeon(TQR_PlanarFramebuffer *fb,
                       const TQR_PaletteState *palette,
                       Theron_V1_World *world)
{
    (void)fb; (void)palette; (void)world;
}

void tr_decode_tile_row(uint8_t *out_row, const uint8_t *src_row, int bpp)
{
    /* The byte-level HuC6270 decoder is source-independent once the caller
     * supplies authenticated VRAM bytes.  Reuse the canonical decoder rather
     * than maintaining a second, potentially divergent implementation. */
    tqr_decode_tile_row(out_row, src_row, bpp);
}

void tr_decode_tile(uint8_t *out64, const uint8_t *src, int bpp)
{
    tqr_decode_tile(out64, src, bpp);
}

const uint8_t *tr_get_tile_data(const TQR_PaletteState *palette, int tile_index)
{
    return tqr_tile_get_data(palette, tile_index);
}

void tr_clear_fb(TQR_PlanarFramebuffer *fb, uint8_t color_index)
{
    if (!fb || !fb->data || fb->w <= 0 || fb->h <= 0 || fb->stride < fb->w)
        return;
    for (int y = 0; y < fb->h; ++y) {
        memset(fb->data + (size_t)y * (size_t)fb->stride,
               color_index, (size_t)fb->w);
    }
}

const char *tr_source_evidence(void)
{
    return "Track 02 tile-bank semantics not decoded; production viewport tile route blocked";
}
