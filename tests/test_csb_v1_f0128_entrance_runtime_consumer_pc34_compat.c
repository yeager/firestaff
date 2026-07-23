#include "csb_v1_f0128_entrance_runtime_consumer_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static void make_session(CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    static unsigned char entrance_pixels[320 * 200];
    static unsigned char left_door_pixels[105 * 161];
    static unsigned char right_door_pixels[128 * 161];
    CSB_V1_StartupRuntimeSurface_PC34 *entrance;
    CSB_V1_StartupRuntimeSurface_PC34 *left_door;
    CSB_V1_StartupRuntimeSurface_PC34 *right_door;

    memset(session, 0, sizeof(*session));
    memset(entrance_pixels, 3, sizeof(entrance_pixels));
    memset(left_door_pixels, 5, sizeof(left_door_pixels));
    memset(right_door_pixels, 7, sizeof(right_door_pixels));
    session->valid = session->real_asset_matched = 1;
    session->title_presents_ready = session->title_chaos_ready = 1;
    session->title_strikes_back_ready = session->entrance_assets_ready = 1;
    session->door_assets_ready = session->full_startup_ready = 1;
    session->rejects_legacy_wrappers = 1;
    session->source_tick = 71u;
    session->generation = 23u;
    session->surfaces.valid = session->surfaces.real_asset_matched = 1;
    session->surfaces.opening_frame_ready = 1;
    session->surfaces.entrance_screen_ready = 1;
    entrance = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    left_door = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    right_door = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    entrance->valid = 1;
    entrance->pixels = entrance_pixels;
    entrance->source_asset_id = 4;
    entrance->width = 320;
    entrance->height = 200;
    entrance->transparent_color = -1;
    left_door->valid = 1;
    left_door->pixels = left_door_pixels;
    left_door->source_asset_id = 2;
    left_door->width = 105;
    left_door->height = 161;
    left_door->transparent_color = -1;
    right_door->valid = 1;
    right_door->pixels = right_door_pixels;
    right_door->source_asset_id = 3;
    right_door->width = 128;
    right_door->height = 161;
    right_door->transparent_color = -1;
}

static int make_closed_plan(CSB_V1_StartupRenderPlan_PC34 *plan)
{
    CSB_V1_StartupRenderState_PC34 state;

    memset(&state, 0, sizeof(state));
    state.entrance_active = 1;
    state.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    return csb_v1_startup_source_render_plan_from_state_pc34(&state, plan);
}

int main(void)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_ViewportFirstFrameMaterializationReceipt material;
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 raster;
    CSB_V1_F0128EntranceRuntimeBinding_PC34 binding;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host;
    unsigned char viewport_pixels[224 * 136];

    make_session(&session);
    check(make_closed_plan(&plan), "closed C004/C002/C003 Entrance plan exists");
    for (int row = 0; row < 136; ++row) {
        memcpy(viewport_pixels + (size_t)row * 224u,
               session.surfaces.surfaces[
                   CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].pixels +
                   (size_t)(33 + row) * 320u,
               224u);
    }
    memset(&material, 0, sizeof(material));
    material.valid = material.consumed_by_m11_render = 1;
    material.real_graphics_session = material.no_synthetic_pixels = 1;
    material.no_fallback_visuals = 1;
    material.combined_material_hash = 0x12345678u;
    memset(&raster, 0, sizeof(raster));
    raster.valid = raster.consumed_by_raster = 1;
    raster.command_count = 5;
    raster.combined_material_hash = material.combined_material_hash;
    raster.raster_hash = 0x87654321u;
    memset(&binding, 0, sizeof(binding));
    binding.valid = 1;
    binding.source_tick = session.source_tick;
    binding.session_generation = session.generation;
    binding.material_receipt = &material;
    binding.raster_receipt = &raster;
    binding.viewport_pixels = viewport_pixels;
    binding.viewport_pixel_count = sizeof(viewport_pixels);
    memset(&host, 0, sizeof(host));
    check(csb_v1_f0128_entrance_runtime_consume_pc34(
              &session, &plan, &binding, &host) && host.valid &&
              host.host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34 &&
              host.raster.source_surface_count == 4 &&
              host.raster.pixels[33u * 320u] == 5u,
          "F0128 raster replaces C004 interior before real C002/C003 doors");
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host);

    ++binding.source_tick;
    check(!csb_v1_f0128_entrance_runtime_consume_pc34(
              &session, &plan, &binding, &host),
          "stale F0128 source tick cannot enter the C004 door route");
    --binding.source_tick;
    material.no_synthetic_pixels = 0;
    check(!csb_v1_f0128_entrance_runtime_consume_pc34(
              &session, &plan, &binding, &host),
          "non-source F0128 raster cannot replace the Entrance interior");
    return failures != 0;
}
