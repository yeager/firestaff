#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>

static uint32_t fnv1a32(const unsigned char *pixels, size_t pixel_count)
{
    uint32_t hash = 2166136261u;
    size_t index;

    for (index = 0u; index < pixel_count; ++index) {
        hash ^= pixels[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static void bind_surface(CSB_V1_StartupRuntimeSurface_PC34 *surface,
                         unsigned char *pixels,
                         int width,
                         int height,
                         int graphic_index,
                         int transparent_color)
{
    memset(surface, 0, sizeof(*surface));
    surface->pixels = pixels;
    surface->width = width;
    surface->height = height;
    surface->source_asset_id = graphic_index;
    surface->transparent_color = transparent_color;
    surface->valid = 1;
    surface->source_kind =
        CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_CSBGRAPHICS_DAT_PC34;
    surface->decoded_pixel_fnv1a =
        fnv1a32(pixels, (size_t)width * (size_t)height);
}

int main(void)
{
    unsigned char inventory[224 * 136];
    unsigned char resurrect[144 * 73];
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupRuntimeSurface_PC34 inventory_surface;
    CSB_V1_StartupRuntimeSurface_PC34 resurrect_surface;
    int ok = 1;

    memset(inventory, 3, sizeof(inventory));
    memset(resurrect, 7, sizeof(resurrect));
    resurrect[0] = 6; /* C040's source-owned transparent key. */
    memset(&frame, 0, sizeof(frame));
    memset(&raster, 0, sizeof(raster));
    frame.valid = 1;
    frame.real_asset_matched = 1;
    frame.no_legacy_wrappers = 1;
    frame.hud_binding_hash = 0x71539a5du;
    bind_surface(&inventory_surface, inventory, 224, 136, 17, -1);
    bind_surface(&resurrect_surface, resurrect, 144, 73, 40, 6);
    frame.hud_inventory_surface = &inventory_surface;
    frame.hud_resurrect_surface = &resurrect_surface;

    if (!csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
            &frame, 1, &raster) || !raster.valid ||
        raster.source_surface_count != 2 ||
        raster.pixels[33u * 320u] != 3u ||
        raster.pixels[85u * 320u + 80u] != 3u) {
        ok = 0;
    }
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);

    inventory[0] = 4u;
    if (csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
            &frame, 0, &raster)) {
        ok = 0;
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    }
    inventory[0] = 3u;
    inventory_surface.decoded_pixel_fnv1a = fnv1a32(inventory, sizeof(inventory));
    inventory_surface.source_kind =
        CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_NONE_PC34;
    if (csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
            &frame, 0, &raster)) {
        ok = 0;
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    }

    printf("%s csb_v1_startup_hud_csbgraphics_surface_pc34_compat\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
