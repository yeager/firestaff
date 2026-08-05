/*
 * Production seam for Theron V1 tile rendering.
 *
 * Track 02 currently proves only the initial grid bytes, not tile-bank
 * semantics, so production exposes no tile pixels until that handoff is
 * decoded. The former inferred renderer and its synthetic probe have been
 * retired; this is the only implementation of the seam.
 */

#include "theron_v1_tile_renderer.h"

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
    (void)out_row; (void)src_row; (void)bpp;
}

void tr_decode_tile(uint8_t *out64, const uint8_t *src, int bpp)
{
    (void)out64; (void)src; (void)bpp;
}

const uint8_t *tr_get_tile_data(const TQR_PaletteState *palette, int tile_index)
{
    (void)palette; (void)tile_index;
    return NULL;
}

void tr_clear_fb(TQR_PlanarFramebuffer *fb, uint8_t color_index)
{
    (void)fb; (void)color_index;
}

const char *tr_source_evidence(void)
{
    return "Track 02 tile-bank semantics not decoded; production viewport tile route blocked";
}
