#include "csb_v1_startup_session_contract_pc34_compat.h"

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

static void make_opening_fixture(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface)
{
    static unsigned char entrance_pixels[320 * 200];
    static unsigned char left_door_pixels[105 * 161];
    static unsigned char right_door_pixels[128 * 161];
    CSB_V1_StartupRuntimeSurface_PC34 *entrance;
    CSB_V1_StartupRuntimeSurface_PC34 *left_door;
    CSB_V1_StartupRuntimeSurface_PC34 *right_door;

    memset(session, 0, sizeof(*session));
    memset(package_receipt, 0, sizeof(*package_receipt));
    memset(host_surface, 0, sizeof(*host_surface));

    session->valid = session->real_asset_matched = 1;
    session->generation = 37u;
    session->source_tick = 911u;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session->playback.title_phase_mask = 0x0f;
    session->playback.no_fallback_routes = 1;
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

    package_receipt->valid = package_receipt->real_package_matched = 1;
    package_receipt->c002_left_door_consumed = 1;
    package_receipt->c003_right_door_consumed = 1;
    package_receipt->c004_entrance_consumed = 1;
    package_receipt->title_to_entrance_same_session = 1;
    package_receipt->title_to_hud_same_session = 1;
    package_receipt->no_legacy_wrappers = 1;
    package_receipt->no_fallback_routes = 1;
    package_receipt->source_tick = session->source_tick;
    package_receipt->session_generation = session->generation;
    package_receipt->real_asset_receipt_hash = 0x405060708090a0b0ULL;
    package_receipt->consumed_surface_hash = 0x1020304050607080ULL;

    host_surface->valid = host_surface->real_asset_matched = 1;
    host_surface->no_legacy_wrappers = host_surface->no_synthetic_surface = 1;
    host_surface->host_surface =
        CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34;
    host_surface->door_opening_decision = 1;
    host_surface->host_surface_hash = 0x4004u;
    host_surface->frame.session_generation = session->generation;
    host_surface->frame.source_tick = session->source_tick;
    host_surface->frame.frame_route_hash = 0x4104u;
    host_surface->frame.opening_step = 1;
    host_surface->frame.entrance_surface = entrance;
    host_surface->frame.left_door_surface = left_door;
    host_surface->frame.right_door_surface = right_door;
    host_surface->raster.valid = host_surface->raster.real_asset_matched = 1;
    host_surface->raster.entrance_composited = 1;
    host_surface->raster.door_composited = 1;
    host_surface->raster.source_surface_count = 3;
    host_surface->raster.pixel_hash = 0x4204u;
    host_surface->raster.route_hash = 0x4304u;
}

static void advance_source_page(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface)
{
    ++session->source_tick;
    ++package_receipt->source_tick;
    ++host_surface->frame.source_tick;
    ++host_surface->frame.opening_step;
}

int main(void)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package_receipt;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host_surface;
    CSB_V1_StartupSessionOpeningDoorTickReceipt_PC34 first;
    CSB_V1_StartupSessionOpeningDoorTickReceipt_PC34 second;

    make_opening_fixture(&session, &package_receipt, &host_surface);
    check(csb_v1_startup_session_opening_door_tick_receipt_pc34(
              &session, &package_receipt, &host_surface, NULL, &first) &&
              first.valid && first.first_source_frame &&
              first.door_step == 1u && first.source_tick == 911u,
          "first real C004/C002/C003 door page is source tick 911");

    advance_source_page(&session, &package_receipt, &host_surface);
    check(csb_v1_startup_session_opening_door_tick_receipt_pc34(
              &session, &package_receipt, &host_surface, &first, &second) &&
              second.valid && !second.first_source_frame &&
              second.previous_door_step == 1u && second.door_step == 2u &&
              second.source_tick == 912u,
          "second real door page follows the first source VBlank");

    ++host_surface.frame.source_tick;
    check(!csb_v1_startup_session_opening_door_tick_receipt_pc34(
              &session, &package_receipt, &host_surface, &first, &second),
          "a stale C004/C002/C003 host tick cannot replay a door page");
    --host_surface.frame.source_tick;

    host_surface.frame.opening_step = 4;
    check(!csb_v1_startup_session_opening_door_tick_receipt_pc34(
              &session, &package_receipt, &host_surface, &first, &second),
          "a source-owned door sequence cannot skip a raster step");

    return failures != 0;
}
