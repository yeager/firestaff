#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void make_terminal_real_data_session(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const char *graphics_path,
    unsigned char *title_pixels,
    unsigned char *presents_pixels,
    unsigned char *chaos_pixels,
    unsigned char *strikes_pixels,
    unsigned char *left_door_pixels,
    unsigned char *right_door_pixels,
    unsigned char *entrance_pixels,
    unsigned char *inventory_pixels,
    unsigned char *resurrect_pixels,
    int use_real_geometry)
{
    CSB_V1_StartupRuntimeSurface_PC34 *inventory;
    CSB_V1_StartupRuntimeSurface_PC34 *resurrect;

    csb_v1_boot_startup_runtime_asset_session_init_pc34(session);
    session->valid = 1;
    session->real_asset_matched = 1;
    session->full_startup_ready = 1;
    session->rejects_legacy_wrappers = 1;
    session->title_assets_ready = 1;
    session->title_presents_ready = 1;
    session->title_chaos_ready = 1;
    session->title_strikes_back_ready = 1;
    session->entrance_assets_ready = 1;
    session->door_assets_ready = 1;
    session->hud_assets_bound = 1;
    session->source_tick = 1u;
    session->generation = 1u;
    session->surfaces.valid = 1;
    session->surfaces.title_regions_ready = 1;
    session->surfaces.opening_frame_ready = 1;
    session->surfaces.entrance_screen_ready = 1;
    session->surfaces.hud_surfaces_ready = 1;
    session->surfaces.real_asset_matched = 1;
    session->playback.no_fallback_routes = 1;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session->playback.entrance_music_active = 1;
    session->playback.title_phase_mask = 0x0f;
    session->playback.entrance_complete = 1;
    session->playback.entrance_scene_presented = 1;
    session->playback.door_frame_presented = 1;
    session->playback.last_door_opening_step = 31;
    session->playback.next_door_opening_step = 32;
    session->playback.entrance_special_palette =
        VGA_PALETTE_PC34_SPECIAL_CSB_ENTRANCE;
    session->hud_inventory_binding.verified = 1;
    session->hud_inventory_binding.source =
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34;
    session->hud_inventory_binding.graphic_index = 17u;
    snprintf(session->hud_inventory_binding.path,
             sizeof(session->hud_inventory_binding.path), "%s", graphics_path);
    session->hud_resurrect_binding.verified = 1;
    session->hud_resurrect_binding.source =
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34;
    session->hud_resurrect_binding.graphic_index = 40u;
    snprintf(session->hud_resurrect_binding.path,
             sizeof(session->hud_resurrect_binding.path), "%s", graphics_path);
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].pixels = title_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].source_asset_id = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].width = 320;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].height = 153;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].pixels = presents_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].source_asset_id = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].source_y = 137;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].width = 320;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].height = 16;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].pixels = chaos_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].source_asset_id = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].width = 320;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].height = 80;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].pixels = strikes_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].source_asset_id = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].source_y = 80;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].width = 320;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].height = 57;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34].transparent_color = 0;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].pixels = left_door_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].source_asset_id = 2;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].width = 105;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].height = 161;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].pixels = right_door_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].source_asset_id = 3;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].width = 128;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].height = 161;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].pixels = entrance_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].source_asset_id = 4;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].width = 320;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].height = 200;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].transparent_color = -1;
    inventory = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    resurrect = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    inventory->valid = 1;
    inventory->pixels = inventory_pixels;
    inventory->source_asset_id = 17;
    inventory->width =
        use_real_geometry ? CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 : 1;
    inventory->height =
        use_real_geometry ? CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34 : 1;
    inventory->transparent_color = -1;
    resurrect->valid = 1;
    resurrect->pixels = resurrect_pixels;
    resurrect->source_asset_id = 40;
    resurrect->width =
        use_real_geometry ? CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34 : 1;
    resurrect->height =
        use_real_geometry ? CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34 : 1;
    resurrect->transparent_color =
        use_real_geometry
            ? CSB_V1_STARTUP_HUD_RESURRECT_TRANSPARENT_COLOR_PC34
            : -1;
}

static void make_release_receipt_ready(
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *receipt)
{
    csb_v1_boot_startup_release_app_capture_receipt_init_pc34(receipt);
    receipt->valid = 1;
    receipt->release_app_capture_ready = 1;
    receipt->title_sequence_capture_ready = 1;
    receipt->title_sequence_host_consumer_ready = 1;
    receipt->title_sequence_same_capture_route = 1;
    receipt->title_host_consumer_ready = 1;
    receipt->closed_door_host_consumer_ready = 1;
    receipt->utility_host_consumer_ready = 1;
    receipt->door_opening_release_app_capture_ready = 1;
    receipt->credits_release_app_capture_ready = 1;
    receipt->credits_host_consumer_ready = 1;
    receipt->route_specific_host_consumers_ready = 1;
    receipt->hud_door_capture_ready = 1;
    receipt->hud_door_host_consumers_ready = 1;
    receipt->hud_door_same_capture_route = 1;
    receipt->runtime_host_routes_ready = 1;
    receipt->draw_consumes_receipt_only = 1;
    receipt->input_consumes_receipt_only = 1;
    receipt->no_fallback_callbacks = 1;
    receipt->no_wrapper_fallback_routes = 1;
    receipt->host_route_wrappers_retired = 1;
    receipt->no_loose_render_plan_exports = 1;
    receipt->release_app_real_asset_capture_ready = 1;
    receipt->real_startup_assets_bound = 1;
    receipt->title_runtime_phase_mask = 0x0b;
    receipt->title_runtime_expected_phase_mask = 0x0b;
    receipt->title_runtime_phase_hash_count =
        CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
    receipt->title_runtime_phase_hash = 0x11111111u;
    receipt->release_app_capture_hash = 0x22222222u;
    receipt->title_sequence_capture_hash = 0x33333333u;
    receipt->closed_door_packaged_capture_hash = 0x44444444u;
    receipt->utility_packaged_capture_hash = 0x55555555u;
    receipt->door_opening_packaged_capture_hash = 0x66666666u;
    receipt->credits_packaged_capture_hash = 0x77777777u;
    receipt->hud_door_capture_hash = 0x88888888u;
    receipt->release_app_real_asset_capture_hash = 0x99999999u;
    receipt->runtime_host_gate_hash = 0xaaaabbbbu;
    receipt->complete_support_hash = 0xbbbbccccu;
}

static void verify_presented_route_hash_gate(void)
{
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 release;
    CSB_V1_StartupPresentedAppCaptureFacts_PC34 facts;
    CSB_V1_StartupPresentedAppCaptureReceipt_PC34 receipt;

    make_release_receipt_ready(&release);
    memset(&facts, 0, sizeof(facts));
    facts.running_from_macos_app_bundle = 1;
    facts.mac_window_capture_ready = 1;
    facts.presented_frame_captured = 1;
    facts.presented_frame_width = 320;
    facts.presented_frame_height = 200;
    facts.presented_frame_indexed_pixels = 1;
    facts.presented_frame_uses_real_csb_assets = 1;
    facts.presented_frame_hash = 0x12345678u;
    facts.presented_frame_route_hash = release.hud_door_capture_hash;
    check(csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &release, &facts, &receipt) && receipt.valid &&
              receipt.presented_frame_route_hash_ready,
          "presented Mac/app capture accepts a receipt-owned HUD/door route hash");

    facts.presented_frame_route_hash = 0xdeadbeefu;
    check(!csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &release, &facts, &receipt) && !receipt.valid &&
              !receipt.presented_frame_route_hash_ready,
          "presented Mac/app capture rejects an unrelated frame route hash");

    facts.presented_frame_route_hash = release.hud_door_capture_hash;
    facts.presented_frame_uses_real_csb_assets = 0;
    check(!csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
              &release, &facts, &receipt) && !receipt.valid &&
              !receipt.presented_frame_real_asset_ready,
          "presented Mac/app capture rejects non-real CSB asset pixels");
}

int main(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_BootStartupDoorRuntimeReceipt_PC34 receipt;
    CSB_V1_DungeonData dungeon;
    unsigned char title_pixels = 1u;
    unsigned char presents_pixels = 2u;
    unsigned char chaos_pixels = 3u;
    unsigned char strikes_pixels = 4u;
    unsigned char left_door_pixels = 5u;
    unsigned char right_door_pixels = 6u;
    unsigned char entrance_pixels = 7u;
    static unsigned char inventory_pixels[
        CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 *
        CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34];
    static unsigned char resurrect_pixels[
        CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34 *
        CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34];
    unsigned char wrapper_inventory_pixel = 17u;
    unsigned char wrapper_resurrect_pixel = 40u;
    int handoff_ok;

    csb_v1_boot_profile_init(&profile);
    memset(&snapshot, 0, sizeof(snapshot));
    memset(&dungeon, 0, sizeof(dungeon));
    profile.assets_verified = 1;
    profile.graphics_verified = 1;
    profile.dungeon_verified = 1;
    snprintf(profile.graphics_path, sizeof(profile.graphics_path), "%s",
             "/verified/CSBGRAPHICS.DAT");
    profile.runtime.dungeon_handle = &dungeon;
    snapshot.boot_profile = &profile;
    snapshot.entrance_active = 1;
    snapshot.opening_active = 1;
    snapshot.opening_step = 31;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    memset(inventory_pixels, 17, sizeof(inventory_pixels));
    memset(resurrect_pixels, 40, sizeof(resurrect_pixels));

    make_terminal_real_data_session(&session, profile.graphics_path,
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    inventory_pixels, resurrect_pixels, 1);
    check(csb_v1_startup_session_full_surface_contract_pc34(&session),
          "fixture supplies the original C001/C004/C002/C003/C017/C040 surface contract");
    memset(&receipt, 0, sizeof(receipt));
    handoff_ok = csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
        &snapshot, &session, &receipt);
    if (!receipt.hud_session_ready) {
        fprintf(stderr,
                "diagnostic: route=%d door=%d runtime=%d hud=%d status=%s "
                "mirror=%d level=%d stage=%d\n",
                (int)receipt.route,
                receipt.door_opening_finished,
                receipt.runtime_view_ready,
                receipt.hud_session_ready,
                receipt.status ? receipt.status : "(null)",
                receipt.runtime_mirror.valid,
                receipt.runtime_mirror.view.level_loaded,
                (int)session.playback.stage);
    }
    check(handoff_ok && receipt.hud_session_ready &&
              receipt.route == CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_HUD_READY_PC34,
          "ReDMCSB entrance completion hands the verified C017/C040 pair to runtime");

    make_terminal_real_data_session(&session, profile.graphics_path,
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    &wrapper_inventory_pixel,
                                    &wrapper_resurrect_pixel, 0);
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) && receipt.door_opening_finished &&
              !receipt.runtime_view_ready && !receipt.hud_session_ready &&
              receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34 &&
              session.playback.stage ==
                  CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34,
          "legacy one-byte C017/C040 wrapper cannot cross the entrance runtime handoff");

    make_terminal_real_data_session(&session, profile.graphics_path,
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    inventory_pixels, resurrect_pixels, 1);
    session.playback.title_phase_mask = 0x03;
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) && receipt.door_opening_finished &&
              !receipt.runtime_view_ready && !receipt.hud_session_ready &&
              receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34 &&
              session.playback.stage ==
                  CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34,
          "incomplete PRESENTS/CHAOS/STRIKES playback cannot cross the handoff");

    make_terminal_real_data_session(&session, "/verified/other-graphics.dat",
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    inventory_pixels, resurrect_pixels, 1);
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) && receipt.door_opening_finished &&
              !receipt.runtime_view_ready && !receipt.hud_session_ready &&
              receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34 &&
              session.playback.stage ==
                  CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34,
          "a different real-data package cannot cross the entrance runtime handoff");

    make_terminal_real_data_session(&session, profile.graphics_path,
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    inventory_pixels, resurrect_pixels, 1);
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].source_asset_id = 4;
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) && receipt.door_opening_finished &&
              !receipt.runtime_view_ready && !receipt.hud_session_ready &&
              receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34,
          "a swapped C003 opening strip cannot cross the entrance runtime handoff");

    make_terminal_real_data_session(&session, profile.graphics_path,
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    inventory_pixels, resurrect_pixels, 1);
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].width = 143;
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) && receipt.door_opening_finished &&
              !receipt.runtime_view_ready && !receipt.hud_session_ready &&
              receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34,
          "a malformed C040 crop cannot cross the entrance runtime handoff");

    verify_presented_route_hash_gate();
    return failures ? 1 : 0;
}
