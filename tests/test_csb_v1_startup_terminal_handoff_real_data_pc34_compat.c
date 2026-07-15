#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"

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
    unsigned char *resurrect_pixels)
{
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
    session->surfaces.real_asset_matched = 1;
    session->surfaces.title_regions_ready = 1;
    session->surfaces.opening_frame_ready = 1;
    session->surfaces.entrance_screen_ready = 1;
    session->surfaces.hud_surfaces_ready = 1;
    session->playback.no_fallback_routes = 1;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session->playback.title_phase_mask = 0x0f;
    session->playback.entrance_music_active = 1;
    session->playback.entrance_complete = 1;
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
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].pixels =
        inventory_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].source_asset_id = 17;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].width = 224;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].height = 136;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].pixels =
        resurrect_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].source_asset_id = 40;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].width = 144;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].height = 73;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].transparent_color = 6;
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
    unsigned char inventory_pixels = 17u;
    unsigned char resurrect_pixels = 40u;
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

    make_terminal_real_data_session(&session, profile.graphics_path,
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    &inventory_pixels, &resurrect_pixels);
    check(csb_v1_startup_session_full_surface_contract_pc34(&session),
          "fixture supplies the original C001/C004/C002/C003/C017/C040 surface contract");
    handoff_ok = csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
        &snapshot, &session, &receipt);
    check(handoff_ok && receipt.hud_session_ready &&
              receipt.route == CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_HUD_READY_PC34,
          "ReDMCSB entrance completion hands the verified C017/C040 pair to runtime");

    make_terminal_real_data_session(&session, "/verified/other-graphics.dat",
                                    &title_pixels, &presents_pixels,
                                    &chaos_pixels, &strikes_pixels,
                                    &left_door_pixels, &right_door_pixels,
                                    &entrance_pixels,
                                    &inventory_pixels, &resurrect_pixels);
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
                                    &inventory_pixels, &resurrect_pixels);
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
                                    &inventory_pixels, &resurrect_pixels);
    session.surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34].width = 143;
    check(csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
              &snapshot, &session, &receipt) && receipt.door_opening_finished &&
              !receipt.runtime_view_ready && !receipt.hud_session_ready &&
              receipt.route ==
                  CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34,
          "a malformed C040 crop cannot cross the entrance runtime handoff");

    return failures ? 1 : 0;
}
