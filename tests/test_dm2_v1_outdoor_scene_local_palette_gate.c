/* Source: skproject SKWIN/SkWinCore.cpp T600 GRAPHICSSET scene material
 * lookup and QUERY_GDAT_IMAGE_LOCALPAL.
 *
 * Provenance: introduced in 232a21a1e with a viewport-side FNV re-hash of
 * the decoded palette bytes.  Re-anchored after 5c21e5561 ("Fix DM2 scene
 * local palette ownership"), which added the source-faithful UPDATE_GFXSET
 * ready gate and made the local-palette receipt hash opaque and
 * provider-owned: the viewport no longer re-derives it from decoded bytes.
 * The "altered palette" case below therefore exercises the contract the
 * callback route actually enforces — an invalid local-palette receipt on
 * any plane blocks the whole scene atomically before either plane draws —
 * while byte-relabel detection stays owned by the boot-owned plan route,
 * which re-verifies palette_hash against the decoded palette bytes. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int asset_fetches;
    int palette_fetches;
    int sky_index;
    int ground_index;
    int palette_seen[2];
    int corrupt_palette;
} FetchTrace;

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static uint32_t palette_hash(const uint8_t palette[16])
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < 16; ++i) {
        hash ^= palette[i];
        hash *= 16777619u;
    }
    return hash;
}

static int fetch_outdoor_scene(void *user,
                               int gdat_index,
                               const uint8_t **out_pixels,
                               int *out_w,
                               int *out_h,
                               int *out_stride)
{
    static const uint8_t sky[1] = { 1 };
    static const uint8_t ground[1] = { 2 };
    FetchTrace *trace = (FetchTrace *)user;

    if (gdat_index == trace->sky_index) {
        *out_pixels = sky;
    } else if (gdat_index == trace->ground_index) {
        *out_pixels = ground;
    } else {
        return -1;
    }
    ++trace->asset_fetches;
    *out_w = 1;
    *out_h = 1;
    *out_stride = 1;
    return 0;
}

static int fetch_outdoor_local_palette(void *user,
                                       int gdat_index,
                                       uint8_t out_palette16[16],
                                       uint32_t *out_hash)
{
    FetchTrace *trace = (FetchTrace *)user;

    ++trace->palette_fetches;
    memset(out_palette16, 0, 16);
    if (gdat_index == trace->sky_index) {
        out_palette16[1] = 0x21u;
        trace->palette_seen[0] = 1;
    } else if (gdat_index == trace->ground_index) {
        out_palette16[2] = 0x42u;
        trace->palette_seen[1] = 1;
    } else {
        return -1;
    }
    /* Under the 5c21e5561 ownership model the receipt hash is opaque and
     * provider-owned, so an altered source palette reaches the viewport as
     * an invalid receipt (hash 0) rather than as an FNV mismatch.  Only the
     * ground plane is invalidated here: the sky must be fully resolved and
     * valid yet still never drawn, which proves the gate rejects the scene
     * atomically before either plane. */
    if (out_hash) {
        *out_hash = (trace->corrupt_palette &&
                     gdat_index == trace->ground_index)
                        ? 0u
                        : palette_hash(out_palette16);
    }
    return 0;
}

static void setup_outdoor(DM2_V1_ViewportState *viewport,
                          uint8_t *framebuffer,
                          FetchTrace *trace,
                          int bind_palette)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_outdoor(viewport, 1);
    /* 5c21e5561 added the source-faithful UPDATE_GFXSET ready gate: a scene
     * control receipt with ready=0 blocks every source-required material, so
     * the fixture must present a ready receipt for the gate to open. */
    dm2_v1_viewport_set_gdat_scene_control(
        viewport, 1, 0, 0x53434e45u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_outdoor_scene, trace);
    if (bind_palette) {
        dm2_v1_viewport_set_asset_palette_provider(
            viewport, fetch_outdoor_local_palette, trace);
    }
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    FetchTrace trace;

    memset(&trace, 0, sizeof(trace));
    trace.sky_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    trace.ground_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_outdoor(&viewport, framebuffer, &trace, 1);
    dm2_v1_viewport_render(&viewport);
    CHECK("outdoor scene consumes each source IMG3 local palette",
          trace.asset_fetches == 2 && trace.palette_fetches == 2 &&
              trace.palette_seen[0] && trace.palette_seen[1] &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x21u &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x42u &&
              viewport.asset_outdoor_sky_drawn_count == 1 &&
              viewport.asset_outdoor_ground_drawn_count == 1 &&
              viewport.gdat_local_palette_consumed_count > 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) == 0u);

    memset(framebuffer, 0x3c, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.sky_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    trace.ground_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
    trace.corrupt_palette = 1;
    setup_outdoor(&viewport, framebuffer, &trace, 1);
    dm2_v1_viewport_render(&viewport);
    CHECK("outdoor scene rejects an invalid source palette receipt before either plane",
          trace.asset_fetches == 2 && trace.palette_fetches == 2 &&
              viewport.asset_outdoor_sky_drawn_count == 0 &&
              viewport.asset_outdoor_ground_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x3cu &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x3cu);

    memset(framebuffer, 0x7e, sizeof(framebuffer));
    memset(&trace, 0, sizeof(trace));
    trace.sky_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING);
    trace.ground_index = dm2_v1_viewport_scene_material_graphic_index(
        0, DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
    setup_outdoor(&viewport, framebuffer, &trace, 0);
    dm2_v1_viewport_render(&viewport);
    /* 5c21e5561 moved this fail-closed decision into the UPDATE_GFXSET
     * transaction gate itself: with no local-palette provider bound the
     * scene blocks before any asset is fetched, which is strictly earlier
     * than the original per-plane discovery (asset_fetches == 1). */
    CHECK("source-required outdoor scene fails closed without local palettes",
          trace.asset_fetches == 0 && trace.palette_fetches == 0 &&
              viewport.asset_outdoor_sky_drawn_count == 0 &&
              viewport.asset_outdoor_ground_drawn_count == 0 &&
              (viewport.blocked_material_mask &
               DM2_V1_VIEWPORT_BLOCKED_MATERIAL_FLOOR_CEILING) != 0u &&
              framebuffer[40 * DM2_VP_WIDTH + 100] == 0x7eu &&
              framebuffer[140 * DM2_VP_WIDTH + 100] == 0x7eu);

    printf("DM2 outdoor scene local palette gate: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
