/*
 * Real CSB startup sequence gate:
 * GRAPHICS.DAT C001-C005/C017/C040 -> PRESENTS/CHAOS/STRIKES -> Entrance
 * closed/opening/credits -> terminal HUD/runtime handoff.
 *
 * This test uses only operator-staged real CSB media.  If no verified CSB
 * corpus is available it exits as SKIP and does not manufacture image bytes.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_startup_real_asset_receipt.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
    } else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static const char *data_dir(char *buffer, size_t buffer_size)
{
    const char *env = getenv("FIRESTAFF_CSB_STARTUP_DATA");
    const char *home;

    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0' || buffer_size == 0u) return NULL;
    snprintf(buffer, buffer_size, "%s/.firestaff/data/csb", home);
    return buffer;
}

static int receipt_for_plan(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt)
{
    return csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
        session, plan, source_tick, receipt) &&
        receipt->valid && receipt->real_asset_matched &&
        receipt->no_legacy_wrappers && receipt->no_synthetic_surface &&
        receipt->raster.valid && receipt->raster.pixel_hash != 0u &&
        receipt->host_surface_hash != 0u;
}

static int render_plan_from_state(
    int title_active,
    int title_frame,
    int credits_active,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    CSB_V1_StartupRenderPlan_PC34 *plan)
{
    CSB_V1_StartupRenderState_PC34 state;

    memset(&state, 0, sizeof(state));
    state.entrance_active = 1;
    state.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    state.title_active = title_active;
    state.title_frame = title_frame;
    state.credits_active = credits_active;
    state.opening_active = opening_active;
    state.opening_delay_ticks = opening_delay_ticks;
    state.opening_step = opening_step;
    return csb_v1_startup_source_render_plan_from_state_pc34(&state, plan);
}

static int panel_matches_source(
    const unsigned char *framebuffer,
    const CSB_V1_StartupRuntimeSurface_PC34 *surface)
{
    int row;

    if (!framebuffer || !surface || !surface->valid || !surface->pixels ||
        surface->width != CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 ||
        surface->height != CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34) {
        return 0;
    }
    for (row = 0; row < surface->height; ++row) {
        if (memcmp(framebuffer + (size_t)(33 + row) * 320u,
                   surface->pixels + (size_t)row * (size_t)surface->width,
                   (size_t)surface->width) != 0) {
            return 0;
        }
    }
    return 1;
}

static int surface_has_visible_pixels(
    const CSB_V1_StartupRuntimeSurface_PC34 *surface)
{
    size_t pixel_count;
    size_t pixel;

    if (!surface || !surface->pixels || surface->width <= 0 ||
        surface->height <= 0 ||
        (size_t)surface->height > SIZE_MAX / (size_t)surface->width) {
        return 0;
    }
    pixel_count = (size_t)surface->width * (size_t)surface->height;
    for (pixel = 0U; pixel < pixel_count; ++pixel) {
        if (surface->pixels[pixel] != 0U) return 1;
    }
    return 0;
}

int main(void)
{
    char default_data_dir[1024];
    const char *root = data_dir(default_data_dir, sizeof(default_data_dir));
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRealReceipt real_receipt;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupAudioAction_PC34 audio_action;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 presents_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 chaos_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 chaos_hold_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 strikes_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 closed_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 opening_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 credits_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 credits_return_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 hud_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 rejected_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 f0128_closed_host;
    CSB_V1_StartupRuntimeRaster_PC34 f0128_entrance_raster;
    CSB_V1_StartupRuntimeRaster_PC34 rejected_f0128_entrance_raster;
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 f0128_viewport_receipt;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 title_snapshot;
    CSB_V1_BootStartupRenderViewReceipt_PC34 title_view;
    CSB_V1_StartupDoorOpeningCaptureReceipt_PC34 opening_capture;
    CSB_V1_StartupRuntimeHudPanelReceipt_PC34 hud_panel;
    CSB_V1_StartupFullRuntimeReceipt_PC34 full_runtime;
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package_receipt;
    CSB_V1_StartupSessionTerminalPackageReceipt_PC34 terminal_package;
    CSB_V1_DungeonData dungeon;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupDoorRuntimeReceipt_PC34 door_receipt;
    unsigned char hud_panel_pixels[320 * 200];
    unsigned char f0128_viewport_pixels[224 * 136];
    uint32_t pre_capture_source_tick;
    uint32_t credits_return_source_tick;
    int pre_capture_last_door_opening_step;
    int pre_capture_next_door_opening_step;

    memset(&presents_host, 0, sizeof(presents_host));
    memset(&chaos_host, 0, sizeof(chaos_host));
    memset(&chaos_hold_host, 0, sizeof(chaos_hold_host));
    memset(&strikes_host, 0, sizeof(strikes_host));
    memset(&closed_host, 0, sizeof(closed_host));
    memset(&opening_host, 0, sizeof(opening_host));
    memset(&credits_host, 0, sizeof(credits_host));
    memset(&credits_return_host, 0, sizeof(credits_return_host));
    memset(&hud_host, 0, sizeof(hud_host));
    memset(&rejected_host, 0, sizeof(rejected_host));
    memset(&f0128_closed_host, 0, sizeof(f0128_closed_host));
    memset(&f0128_entrance_raster, 0, sizeof(f0128_entrance_raster));
    memset(&rejected_f0128_entrance_raster, 0,
           sizeof(rejected_f0128_entrance_raster));
    memset(&f0128_viewport_receipt, 0, sizeof(f0128_viewport_receipt));
    memset(&title_snapshot, 0, sizeof(title_snapshot));
    memset(&title_view, 0, sizeof(title_view));
    memset(&opening_capture, 0, sizeof(opening_capture));
    memset(&hud_panel, 0, sizeof(hud_panel));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&snapshot, 0, sizeof(snapshot));
    printf("data_dir=%s\n", root ? root : "(none)");

    csb_v1_boot_profile_init(&profile);
    if (!root || csb_v1_boot_scan_assets(&profile, root) != 0 ||
        !profile.assets_verified || !profile.graphics_verified ||
        !profile.dungeon_verified ||
        profile.variant_id != CSB_V1_VARIANT_PC34_EN) {
        printf("SKIP: no verified PC34 CSB startup corpus was found.\n");
        return 0;
    }
    if (csb_v1_startup_real_scan_and_receipt(root, 6, &real_receipt) !=
            CSB_V1_STARTUP_REAL_OK ||
        !real_receipt.matched ||
        real_receipt.variant_id != CSB_V1_VARIANT_PC34_EN) {
        printf("SKIP: CSB startup real-asset receipt did not match PC34.\n");
        return 0;
    }

    check(csb_v1_boot_startup_runtime_asset_session_open_pc34(
              &profile, &session),
          "real GRAPHICS.DAT opens one C001-C005/C017/C040 startup session");
    if (failures) return 1;
    check(csb_v1_startup_session_full_surface_contract_pc34(&session),
          "real session keeps C001 title, C004 entrance, C002/C003 doors and C017/C040 HUD");
    check(session.surfaces.surfaces[
              CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34]
                  .decode_receipt.valid &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34]
                      .decode_receipt.ended_at_record_boundary &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34]
                      .decode_receipt.valid &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34]
                      .decode_receipt.ended_at_record_boundary &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                      .decode_receipt.valid &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                      .decode_receipt.ended_at_record_boundary,
          "C002/C003/C004 Entrance surfaces retain CSBWin decoder-boundary receipts");
    check(session.surfaces.surfaces[
              CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34]
                  .decode_receipt.valid &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34]
                      .decode_receipt.ended_at_record_boundary &&
              surface_has_visible_pixels(&session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34]),
          "real C005 uses CSBWin ExpandGraphic and produces visible credits pixels");
    check(session.surfaces.surfaces[
              CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]
                  .decode_receipt.valid &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]
                      .decode_receipt.ended_at_record_boundary &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
                  .decode_receipt.valid &&
              session.surfaces.surfaces[
                  CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
                      .decode_receipt.ended_at_record_boundary,
          "real C017/C040 retain exact GRAPHICS.DAT decoder-boundary receipts");
    memset(hud_panel_pixels, 0xa5, sizeof(hud_panel_pixels));
    check(!csb_v1_boot_startup_runtime_hud_panel_blit_from_session_pc34(
              &session, 0, hud_panel_pixels, 320, 200, &hud_panel) &&
              hud_panel_pixels[33u * 320u] == 0xa5,
          "C017 HUD panel rejects before the F0807 terminal session");

    check(csb_v1_boot_startup_playback_begin_pc34(&session, &audio_action) &&
              audio_action ==
                  CSB_V1_STARTUP_AUDIO_ACTION_PLAY_FTL_SWOOSH_PC34,
          "startup begins through the owned FTL swoosh route");
    check(csb_v1_boot_startup_playback_complete_swoosh_pc34(
              &session, &audio_action) &&
              audio_action ==
                  CSB_V1_STARTUP_AUDIO_ACTION_RELEASE_FTL_SWOOSH_PC34,
          "swoosh release enters the real title route");

    check(render_plan_from_state(1, 0, 0, 0, 0, 0, &plan) &&
              csb_v1_boot_startup_playback_accepts_title_plan_pc34(
                  &session, &plan, 0),
          "real C001 PRESENTS plan is admitted directly after swoosh release");
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34;
    check(!csb_v1_boot_startup_playback_accepts_title_plan_pc34(
              &session, &plan, 0) &&
              session.playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34 &&
              session.playback.title_phase_mask == 0u,
          "a non-title plan cannot skip real PRESENTS into Entrance");

    check(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, 0, &plan, &audio_action) &&
              plan.surface == CSB_V1_STARTUP_RENDER_TITLE_PC34 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              plan.title_source_step == 1 &&
              receipt_for_plan(&session, &plan, 1u, &presents_host) &&
              presents_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 &&
              presents_host.frame.title_phase_mask == 0x01 &&
              presents_host.frame.title_surface->source_asset_id == 1 &&
              presents_host.frame.title_surface->decode_receipt.valid,
          "real C001 PRESENTS raster reaches host surface receipt");
    check(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, 60, &plan, &audio_action) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 2 &&
              receipt_for_plan(&session, &plan, 2u, &chaos_host) &&
              chaos_host.frame.title_phase_mask == 0x02 &&
              chaos_host.frame.title_surface->source_asset_id == 1,
          "real C001 CHAOS zoom raster reaches host surface receipt");
    check(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, 79, &plan, &audio_action) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 21 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS &&
              receipt_for_plan(&session, &plan, 3u, &chaos_hold_host) &&
              chaos_hold_host.frame.title_phase_mask == 0x04,
          "real C001 CHAOS hold raster reaches host surface receipt");
    title_snapshot.boot_profile = &profile;
    title_snapshot.entrance_active = 1;
    title_snapshot.entrance_source_step =
        csb_v1_startup_entrance_wait_stage_pc34();
    title_snapshot.title_active = 1;
    title_snapshot.title_frame = 79;
    title_snapshot.title_source_step = 21;
    check(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &title_snapshot, &title_view) && title_view.render_plan_valid &&
              title_view.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              title_view.render_plan.title_source_step == 21 &&
              title_view.render_plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS &&
              title_view.render_plan.title_source_y == 0 &&
              title_view.render_plan.title_source_h == 80,
          "M11 render-view preserves the real frame-79 C001 CHAOS raster and palette");
    check(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, 80, &plan, &audio_action) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 21 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS &&
              receipt_for_plan(&session, &plan, 4u, &chaos_hold_host),
          "real C001 full CHAOS raster remains held before STRIKES BACK");
    title_snapshot.title_frame = 80;
    check(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &title_snapshot, &title_view) && title_view.render_plan_valid &&
              title_view.render_plan.title_stage ==
                  CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              title_view.render_plan.title_source_step == 21 &&
              title_view.render_plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS &&
              title_view.render_plan.title_source_y == 0 &&
              title_view.render_plan.title_source_h == 80,
          "M11 render-view retains the real frame-80 C001 CHAOS raster and palette");
    check(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, 100, &plan, &audio_action) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              plan.title_source_step == 22 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES &&
              receipt_for_plan(&session, &plan, 5u, &strikes_host) &&
              strikes_host.frame.title_phase_mask == 0x08 &&
              strikes_host.frame.title_surface->source_asset_id == 1 &&
              session.playback.title_phase_mask == 0x0f,
          "real C001 STRIKES BACK follows the full CHAOS hold");
    check(presents_host.raster.pixel_hash != 0u &&
              chaos_host.raster.pixel_hash != 0u &&
              chaos_hold_host.raster.pixel_hash != 0u &&
              strikes_host.raster.pixel_hash != 0u &&
              presents_host.raster.pixel_hash != chaos_host.raster.pixel_hash &&
              chaos_host.raster.pixel_hash != chaos_hold_host.raster.pixel_hash &&
              chaos_hold_host.raster.pixel_hash != strikes_host.raster.pixel_hash &&
              session.playback.title_phase_mask == 0x0f,
          "real C001 capture retains PRESENTS, CHAOS zoom, CHAOS hold, and STRIKES in source order");
    check(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, csb_v1_startup_title_total_ticks_pc34(), &plan,
              &audio_action) &&
              audio_action ==
                  CSB_V1_STARTUP_AUDIO_ACTION_PLAY_ENTRANCE_MUSIC_PC34 &&
              session.playback.stage ==
                  CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34 &&
              session.playback.title_phase_mask == 0x0fu,
          "title completion hands the same session to Entrance with all title phases consumed");
    title_snapshot.title_active = 0;
    title_snapshot.title_frame = 0;
    title_snapshot.title_source_step = 0;
    title_snapshot.credits_active = 0;
    title_snapshot.opening_active = 0;
    check(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &title_snapshot, &title_view) && title_view.render_plan_valid &&
              title_view.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              title_view.render_plan.source_asset_id == 4 &&
              title_view.render_plan.fallback_text_row_count == 0 &&
              receipt_for_plan(&session, &title_view.render_plan, 6u,
                               &closed_host) &&
              closed_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34 &&
              closed_host.raster.source_surface_count == 3,
          "M11 receipt transitions the complete real C001 sequence to C004/C002/C003 without fallback");

    check(render_plan_from_state(0, 0, 0, 0, 0, 0, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34 &&
              receipt_for_plan(&session, &plan, 6u, &closed_host) &&
              closed_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34 &&
              closed_host.raster.source_surface_count == 3,
          "real C004+C002+C003 closed Entrance reaches host surface receipt");
    f0128_viewport_receipt.valid = 1;
    f0128_viewport_receipt.consumed_by_raster = 1;
    f0128_viewport_receipt.command_count = 1;
    f0128_viewport_receipt.combined_material_hash =
        session.surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
            .decode_receipt.indexed_pixel_fnv1a;
    f0128_viewport_receipt.raster_hash = closed_host.raster.pixel_hash;
    for (int row = 0; row < 136; ++row) {
        memcpy(f0128_viewport_pixels + (size_t)row * 224u,
               session.surfaces.surfaces[
                   CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34]
                   .pixels + (size_t)(33 + row) * 320u,
               224u);
    }
    check(csb_v1_boot_startup_entrance_f0128_raster_compose_pc34(
              &closed_host.frame, &plan, &f0128_viewport_receipt,
              f0128_viewport_pixels,
              224u * 136u, &f0128_entrance_raster) &&
              f0128_entrance_raster.valid &&
              f0128_entrance_raster.real_asset_matched &&
              f0128_entrance_raster.source_surface_count == 4 &&
              memcmp(f0128_entrance_raster.pixels, closed_host.raster.pixels,
                     320u * 200u) == 0,
          "real C004 admits a receipt-bound F0128 viewport before C002/C003");
    check(csb_v1_boot_startup_runtime_entrance_f0128_receipt_from_session_pc34(
              &session, &plan, 6u, &f0128_viewport_receipt,
              f0128_viewport_pixels, 224u * 136u, &f0128_closed_host) &&
              f0128_closed_host.valid &&
              f0128_closed_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34 &&
              f0128_closed_host.raster.source_surface_count == 4 &&
              f0128_closed_host.host_surface_hash != 0u &&
              memcmp(f0128_closed_host.raster.pixels,
                     closed_host.raster.pixels, 320u * 200u) == 0,
          "runtime Entrance host consumes F0128 viewport before decoded doors");
    f0128_viewport_receipt.rejected = 1;
    check(!csb_v1_boot_startup_entrance_f0128_raster_compose_pc34(
              &closed_host.frame, &plan, &f0128_viewport_receipt,
              f0128_viewport_pixels, 224u * 136u,
              &rejected_f0128_entrance_raster),
          "Entrance rejects an unadmitted F0128 viewport raster");
    f0128_viewport_receipt.rejected = 0;
    check(render_plan_from_state(0, 0, 0, 1, 0, 31, &plan) &&
              plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34 &&
              plan.special_palette == VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE &&
              plan.title_special_palette == -1 &&
              receipt_for_plan(&session, &plan, 7u, &opening_host) &&
              opening_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 &&
              opening_host.raster.source_surface_count == 2,
          "real C004+C003 final opening frame reaches host surface receipt with Entrance palette");
    plan.special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
    check(!receipt_for_plan(&session, &plan, 7u, &rejected_host),
          "real F0438 opening raster rejects a title-palette relabel before presentation");
    pre_capture_source_tick = session.source_tick;
    pre_capture_last_door_opening_step =
        session.playback.last_door_opening_step;
    pre_capture_next_door_opening_step =
        session.playback.next_door_opening_step;
    check(csb_v1_boot_startup_door_opening_capture_from_session_pc34(
              &session, 100u, &opening_capture) && opening_capture.valid &&
              opening_capture.real_asset_matched &&
              opening_capture.no_legacy_wrappers &&
              opening_capture.no_synthetic_surface &&
              opening_capture.source_step_count == 31 &&
              opening_capture.captured_frame_count == 31 &&
              opening_capture.first_step == 1 && opening_capture.last_step == 31 &&
              opening_capture.frame_route_hashes[0] != 0u &&
              opening_capture.frame_route_hashes[30] != 0u &&
              opening_capture.frame_route_hashes[0] !=
                  opening_capture.frame_route_hashes[30] &&
              opening_capture.raster_pixel_hashes[0] != 0u &&
              opening_capture.raster_pixel_hashes[30] != 0u &&
              opening_capture.capture_sequence_hash != 0u,
          "real C004/C002/C003 session captures all 31 door-opening pages");
    check(session.source_tick == pre_capture_source_tick &&
              session.playback.last_door_opening_step ==
                  pre_capture_last_door_opening_step &&
              session.playback.next_door_opening_step ==
                  pre_capture_next_door_opening_step,
          "real F0438 capture keeps the live Entrance session at its prior step");
    check(render_plan_from_state(0, 0, 1, 0, 0, 0, &plan) &&
              plan.surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34 &&
              receipt_for_plan(&session, &plan, 8u, &credits_host) &&
              credits_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_CREDITS_PC34 &&
              credits_host.special_palette == VGA_PALETTE_PC34_SPECIAL_CREDITS &&
              credits_host.title_special_palette == -1 &&
              credits_host.frame.entrance_surface &&
              credits_host.frame.entrance_surface->source_asset_id == 5 &&
              credits_host.frame.entrance_surface->decode_receipt.valid &&
              credits_host.frame.entrance_surface->decode_receipt.ended_at_record_boundary &&
              credits_host.raster.source_surface_count == 1 &&
              session.playback.credits_scene_presented &&
              !session.playback.credits_return_presented &&
              session.playback.credits_source_tick == 8u &&
              session.playback.credits_frame_route_hash ==
                  credits_host.frame.frame_route_hash &&
              session.playback.credits_raster_hash == credits_host.raster.pixel_hash &&
              session.playback.credits_return_source_tick == 0u &&
              session.playback.credits_return_frame_route_hash == 0u &&
              session.playback.credits_return_raster_hash == 0u,
          "real C005 credits reaches and remains bound to the host frame receipt");
    check(!csb_v1_boot_startup_playback_complete_entrance_pc34(&session),
          "F0807 rejects a C005 route until its real C004/C002/C003 return frame is presented");
    check(render_plan_from_state(0, 0, 0, 0, 0, 0, &plan) &&
              receipt_for_plan(&session, &plan, 9u, &credits_return_host) &&
              credits_return_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34 &&
              credits_return_host.raster.source_surface_count == 3 &&
              session.playback.credits_return_presented &&
              session.playback.credits_return_source_tick == 9u &&
              session.playback.credits_return_frame_route_hash ==
                  credits_return_host.frame.frame_route_hash &&
              session.playback.credits_return_raster_hash ==
                  credits_return_host.raster.pixel_hash &&
              credits_return_host.special_palette !=
                  VGA_PALETTE_PC34_SPECIAL_CREDITS,
          "C005 credits return restores the real C004/C002/C003 Entrance route");

    credits_return_source_tick = session.playback.credits_return_source_tick;
    session.playback.credits_return_source_tick = session.playback.credits_source_tick;
    check(!csb_v1_boot_startup_playback_complete_entrance_pc34(&session),
          "F0807 rejects a stale C004/C002/C003 receipt after C005 credits");
    session.playback.credits_return_source_tick = credits_return_source_tick;
    check(csb_v1_boot_startup_playback_complete_entrance_pc34(&session) &&
              csb_v1_boot_startup_playback_enter_hud_pc34(&session),
          "Entrance completion moves the same session to runtime HUD");
    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    plan.special_palette = -1;
    plan.title_special_palette = -1;
    check(receipt_for_plan(&session, &plan, 9u, &hud_host) &&
              hud_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34 &&
              hud_host.special_palette == -1 &&
              hud_host.title_special_palette == -1 &&
              hud_host.uses_c017_inventory && hud_host.uses_c040_resurrect &&
              hud_host.raster.source_surface_count == 2,
          "real first C017/C040 HUD frame reaches terminal runtime host surface with neutral palette");
    /* A retained C28 palette is invalid after F0807; use the distinct DM
     * C07 palette as a concrete foreign-palette regression input. */
    plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    check(!receipt_for_plan(&session, &plan, 8u, &rejected_host),
          "first C017/C040 HUD raster rejects a retained Entrance palette before presentation");
    {
        CSB_V1_StartupGraphicDecodeReceipt_PC34 saved_receipt =
            session.surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]
                .decode_receipt;

        session.surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]
            .decode_receipt.ended_at_record_boundary = 0;
        plan.special_palette = -1;
        check(!receipt_for_plan(&session, &plan, 9u, &rejected_host),
              "C017/C040 HUD raster rejects a stale GRAPHICS.DAT source plan");
        session.surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]
            .decode_receipt = saved_receipt;
    }
    {
        CSB_V1_StartupGraphicDecodeReceipt_PC34 saved_receipt =
            session.surfaces.surfaces[
                CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
                .decode_receipt;

        session.surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
            .decode_receipt.ended_at_record_boundary = 0;
        check(!receipt_for_plan(&session, &plan, 9u, &rejected_host),
              "C017/C040 HUD raster rejects a stale C040 GRAPHICS.DAT source plan");
        session.surfaces.surfaces[
            CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34]
            .decode_receipt = saved_receipt;
    }
    memset(hud_panel_pixels, 0, sizeof(hud_panel_pixels));
    check(csb_v1_boot_startup_runtime_hud_panel_blit_from_session_pc34(
              &session, 0, hud_panel_pixels, 320, 200, &hud_panel) &&
              hud_panel.valid && hud_panel.c017_presented &&
              !hud_panel.c040_presented && hud_panel.no_synthetic_surface &&
              panel_matches_source(hud_panel_pixels,
                  &session.surfaces.surfaces[
                      CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34]),
          "first live HUD consumes the real F0807 C017 panel raster");
    check(!csb_v1_boot_startup_door_opening_capture_from_session_pc34(
              &session, 200u, &opening_capture),
          "F0438 door capture rejects after the F0807 HUD transition");

    check(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session, &full_runtime) &&
              full_runtime.valid && full_runtime.playback_reaches_title &&
              full_runtime.playback_reaches_entrance &&
              full_runtime.playback_reaches_hud &&
              full_runtime.title_to_hud_same_session &&
              full_runtime.no_legacy_wrappers,
          "full runtime receipt joins title, Entrance, door and HUD in one session");
    check(csb_v1_startup_real_package_consumption_receipt_from_session_pc34(
              &real_receipt, &session, &package_receipt) &&
              package_receipt.valid &&
              package_receipt.c001_presents_consumed &&
              package_receipt.c001_chaos_consumed &&
              package_receipt.c001_chaos_hold_consumed &&
              package_receipt.c001_strikes_back_consumed &&
              package_receipt.c002_left_door_consumed &&
              package_receipt.c003_right_door_consumed &&
              package_receipt.c004_entrance_consumed &&
              package_receipt.c005_credits_consumed &&
              package_receipt.c017_hud_consumed &&
              package_receipt.c040_hud_consumed,
          "real package consumption receipt covers C001-C005/C017/C040");
    check(csb_v1_startup_session_terminal_package_receipt_pc34(
              &session, &package_receipt, &terminal_package) &&
              terminal_package.valid &&
              terminal_package.real_package_matched &&
              terminal_package.terminal_f0807_complete,
          "terminal package receipt accepts only the same verified session");

    session.playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session.playback.entrance_complete = 1;
    check(csb_v1_dungeon_load_from_file(&dungeon, profile.dungeon_path) == 0,
          "real DUNGEON.DAT loads for terminal runtime mirror");
    profile.runtime.dungeon_handle = &dungeon;
    profile.runtime.party_state_valid = 1;
    profile.runtime.current_level = 0;
    profile.runtime.party_x = 1;
    profile.runtime.party_y = 1;
    profile.runtime.party_dir = 0;
    profile.runtime.party_state.ChampionCount = 1;
    profile.runtime.party_state.PartyDirection = 0;
    profile.runtime.party_state.LeaderIndex = 0;
    csb_v1_dungeon_set_current(&dungeon);
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.boot_profile = &profile;
    snapshot.entrance_active = 1;
    snapshot.opening_active = 1;
    snapshot.opening_step = 31;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &door_receipt) &&
              door_receipt.valid &&
              door_receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_HUD_READY_PC34 &&
              door_receipt.hud_session_ready &&
              door_receipt.runtime_view_ready,
          "terminal F0807 handoff reaches stable runtime HUD only after full real startup");

    csb_v1_dungeon_free(&dungeon);
    csb_v1_dungeon_set_current(NULL);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&presents_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&chaos_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&chaos_hold_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&strikes_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&closed_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&opening_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&credits_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&hud_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&rejected_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
        &f0128_closed_host);
    csb_v1_boot_startup_runtime_raster_release_pc34(&f0128_entrance_raster);
    csb_v1_boot_startup_runtime_raster_release_pc34(
        &rejected_f0128_entrance_raster);
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);

    printf("Summary: %s\n", failures ? "FAILED" : "PASSED");
    return failures ? 1 : 0;
}
