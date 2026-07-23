#include "csb_v1_viewport_f0219_projectile_handoff_consumer_pc34_compat.h"

#include <string.h>

int csb_v1_viewport_f0219_projectile_handoff_to_blit_pc34(
    const CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 *handoff,
    int party_dir,
    int party_x,
    int party_y,
    CSB_V1_ViewportRuntimeProjectileSpriteBlit *out_blit)
{
    CSB_V1_ViewportRuntimeProjectileOverlayPlacement placement;
    CSB_V1_ViewportRuntimeProjectileSpriteBlit blit;

    memset(&blit, 0, sizeof(blit));
    blit.graphic_index = -1;
    blit.aspect_index = -1;
    blit.relative_dir = -1;
    blit.relative_cell = -1;
    if (out_blit) *out_blit = blit;
    if (!handoff || !out_blit || !handoff->valid ||
        handoff->bitmapIndex < 454 || handoff->bitmapIndex > 670 ||
        handoff->projectileAspectOrdinal <= 0) {
        return 0;
    }

    if (!csb_v1_viewport_runtime_projectile_overlay_placement(
            party_dir, party_x, party_y,
            handoff->mapX, handoff->mapY, handoff->cell, &placement) ||
        !placement.visible) {
        return 0;
    }

    /* DUNVIEW.C F0115 has already received F0142's source aspect from the
     * admitted handoff.  Do not recreate direction, aspect, or a colored
     * marker: pass the authenticated native bitmap directly to F0791's
     * drawer contract. */
    blit.view_cell = placement.view_cell;
    blit.source_zone = placement.source_zone;
    blit.source_zone_row = placement.source_zone_row;
    blit.viewport_x = placement.viewport_x;
    blit.viewport_y = placement.viewport_y;
    blit.x = placement.sprite_x;
    blit.y = placement.sprite_y;
    blit.w = placement.sprite_w;
    blit.h = placement.sprite_h;
    blit.forward = placement.forward;
    blit.aspect_index = handoff->projectileAspectOrdinal;
    blit.graphic_index = handoff->bitmapIndex;
    blit.flip_flags = 0;
    blit.derived_bitmap_cache_slot = -1;
    blit.transparent_color = handoff->transparentFlag ? 10 : 0;
    blit.uses_f0791_blit = 1;
    *out_blit = blit;
    return 1;
}
