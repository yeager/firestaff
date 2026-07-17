#include <stdio.h>
#include <string.h>

#include "theron_v1_palette_runtime_admission.h"

static const Theron_RuntimeMediaSurface *theron_v1_palette_runtime_surface(
    const Theron_V1_World *world,
    Theron_RuntimeMediaSurfaceKind kind,
    unsigned int *expected_route_bit) {
    if (!world || !expected_route_bit) {
        return NULL;
    }
    switch (kind) {
    case THERON_RUNTIME_MEDIA_SURFACE_TITLE:
        *expected_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
        return &world->runtime_media.title;
    case THERON_RUNTIME_MEDIA_SURFACE_STAGE:
        *expected_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;
        return &world->runtime_media.stage;
    case THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM:
        *expected_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
        return &world->runtime_media.soul_room;
    case THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD:
        *expected_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
        return &world->runtime_media.forcefield;
    default:
        return NULL;
    }
}

int theron_v1_palette_runtime_admit_track02_surface(
    const Theron_V1Track02PaletteRouteReceipt *palette_route,
    const Theron_V1_World *world,
    Theron_RuntimeMediaSurfaceKind surface_kind,
    Theron_V1PaletteRuntimeAdmissionReceipt *out) {
    Theron_V1PaletteRuntimeAdmissionReceipt receipt = {0};
    const Theron_RuntimeMediaSurface *surface;
    unsigned int expected_route_bit = 0u;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    surface = theron_v1_palette_runtime_surface(world, surface_kind,
                                                &expected_route_bit);
    if (!palette_route || !world || !out || !surface ||
        !palette_route->accepted || !palette_route->real_cd_verified ||
        palette_route->render_allowed ||
        palette_route->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        theron_v1_track02_variant_for_md5(palette_route->track02_md5) !=
            palette_route->track02_variant ||
        !world->runtime_media.restored ||
        !world->runtime_media.identity.ready ||
        world->runtime_media.identity.track02_variant !=
            (int)palette_route->track02_variant ||
        !surface->ready || !surface->raw_source_verified ||
        strcmp(surface->track02_md5, palette_route->track02_md5) != 0 ||
        surface->route_bit != expected_route_bit ||
        surface->width == 0u || surface->height == 0u ||
        surface->first_raw_offset == 0u ||
        surface->first_user_data_offset == 0u || surface->checksum == 0u) {
        return 0;
    }

    receipt.valid = 1;
    receipt.authenticated_palette_route_consumed = 1;
    receipt.runtime_surface_consumed = 1;
    receipt.runtime_palette_admission_allowed = 1;
    receipt.track02_variant = palette_route->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             palette_route->track02_md5);
    receipt.surface_kind = surface_kind;
    receipt.bitmap_route_bit = surface->route_bit;
    receipt.bitmap_width = surface->width;
    receipt.bitmap_height = surface->height;
    receipt.bitmap_first_raw_offset = surface->first_raw_offset;
    receipt.bitmap_first_user_data_offset = surface->first_user_data_offset;
    receipt.bitmap_checksum = surface->checksum;
    receipt.vce_index_address = palette_route->vce_index_address;
    receipt.vce_low_address = palette_route->vce_low_address;
    receipt.vce_high_address = palette_route->vce_high_address;
    receipt.vce_index = palette_route->vce_index;
    receipt.vce_low = palette_route->vce_low;
    receipt.vce_high = palette_route->vce_high;
    receipt.bitmap_palette_relation_verified = 0;
    receipt.render_allowed = 0;
    receipt.dungeon_draw_allowed = 0;
    receipt.fallback_visuals_allowed = 0;
    *out = receipt;
    return 1;
}
