/*
 * firestaff_csb_v1_pc_real_asset_launch_probe.c
 *
 * PC-first CSB V1 real-data launch gate.  This probe is intentionally
 * separate from the Atari ST/Amiga real-asset probe: the 24h CSB finish lane
 * should first prove the canonical PC CSB pair can scan, hand off to the CSB
 * boot profile, load DUNGEON.DAT, and tick the V1 runtime.
 *
 * Source-lock boundary:
 *   - ReDMCSB ENTRANCE.C F0806 lines 409-441 selects the CSB entrance path.
 *   - ReDMCSB LOADSAVE.C F0435 lines 1936-1944 loads the new-game dungeon.
 *   - ReDMCSB DUNGEON.C F0237 owns dungeon-load entry.
 *
 * The probe is skip-safe on hosts without user-supplied game data.
 */

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(cond, msg) do { \
    ++checks; \
    if (cond) { \
        printf("  PASS: %s\n", msg); \
    } else { \
        ++failures; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

static const char *pc_data_dir(int argc, char **argv, char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];

    env = getenv("FIRESTAFF_CSB_PC_DATA");
    if (env && env[0] != '\0') return env;

    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data/csb", home);
    return buf;
}

static int pc_data_present(const char *dir)
{
    CSB_V1_BootProfile profile;
    if (!dir || dir[0] == '\0') return 0;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, dir) == 0;
}

static int raster_rows_match_surface(const CSB_V1_StartupRuntimeRaster_PC34 *raster,
                                     const CSB_V1_StartupRuntimeSurface_PC34 *surface,
                                     int row_count)
{
    size_t byte_count;

    if (!raster || !surface || !raster->pixels || !surface->pixels ||
        raster->width != 320 || surface->width != 320 || row_count <= 0 ||
        row_count > raster->height || row_count > surface->height) {
        return 0;
    }
    byte_count = (size_t)row_count * 320u;
    return memcmp(raster->pixels, surface->pixels, byte_count) == 0;
}

static int title_palette_matches_rgb(const unsigned char *rgb,
                                     unsigned char r,
                                     unsigned char g,
                                     unsigned char b)
{
    return rgb && rgb[0] == r && rgb[1] == g && rgb[2] == b;
}

static uint32_t real_surface_hash(const unsigned char *pixels, size_t count)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!pixels || count == 0u) return 0u;
    for (i = 0; i < count; ++i) {
        hash ^= pixels[i];
        hash *= 16777619u;
    }
    return hash ? hash : 2166136261u;
}

static void verify_real_c017_c040_hud_handoff(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host_surface;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupAudioAction_PC34 audio_action;
    const CSB_V1_StartupRuntimeSurface_PC34 *inventory;
    const CSB_V1_StartupRuntimeSurface_PC34 *resurrect;

    if (!session) return;
    inventory = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    resurrect = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    CHECK(inventory->valid && inventory->pixels &&
              inventory->width == 224 && inventory->height == 136 &&
              resurrect->valid && resurrect->pixels &&
              resurrect->width == 144 && resurrect->height == 73,
          "C017/C040 decode to their original PC HUD dimensions");
    CHECK(real_surface_hash(inventory->pixels,
                            (size_t)inventory->width * inventory->height) ==
                  0x7117c9c5u &&
              real_surface_hash(resurrect->pixels,
                                (size_t)resurrect->width * resurrect->height) ==
                  0x35fb6d05u,
          "C017/C040 captures match verified PC CSB GRAPHICS.DAT pixels");

    memset(&plan, 0, sizeof(plan));
    CHECK(csb_v1_boot_startup_playback_begin_pc34(session, &audio_action) == 1 &&
              audio_action == CSB_V1_STARTUP_AUDIO_ACTION_PLAY_FTL_SWOOSH_PC34 &&
              csb_v1_boot_startup_playback_complete_swoosh_pc34(
                  session, &audio_action) == 1 &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  session, 0, &plan, &audio_action) == 1 &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  session, 60, &plan, &audio_action) == 1 &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  session, 80, &plan, &audio_action) == 1 &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  session, 100, &plan, &audio_action) == 1 &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  session, csb_v1_startup_title_total_ticks_pc34(),
                  &plan, &audio_action) == 1 &&
              audio_action == CSB_V1_STARTUP_AUDIO_ACTION_PLAY_ENTRANCE_MUSIC_PC34 &&
              csb_v1_boot_startup_playback_complete_entrance_pc34(session) == 1 &&
              csb_v1_boot_startup_playback_enter_hud_pc34(session) == 1 &&
              session->playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34,
          "C001-C005 playback reaches the C017/C040 runtime HUD owner");

    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              session, &plan, 102u, &frame) == 1 && frame.valid &&
              frame.uses_verified_hud_bindings &&
              frame.hud_inventory_surface == inventory &&
              frame.hud_resurrect_surface == resurrect &&
              frame.hud_inventory_pixel_hash ==
                  real_surface_hash(inventory->pixels,
                                    (size_t)inventory->width * inventory->height) &&
              frame.hud_resurrect_pixel_hash ==
                  real_surface_hash(resurrect->pixels,
                                    (size_t)resurrect->width * resurrect->height) &&
              frame.hud_binding_hash != 0u,
          "runtime frame binds original C017/C040 pixels without a wrapper surface");
    CHECK(csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
              session, &plan, 102u, &host_surface) == 1 && host_surface.valid &&
              host_surface.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34 &&
              host_surface.runtime_hud_decision &&
              host_surface.uses_c017_inventory && host_surface.uses_c040_resurrect &&
              host_surface.raster.valid &&
              host_surface.raster.real_asset_matched &&
              host_surface.raster.source_surface_count == 2 &&
              host_surface.no_synthetic_surface,
          "runtime host captures the presented original C017/C040 HUD frame");
    memset(&raster, 0, sizeof(raster));
    CHECK(csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
              &frame, 1, &raster) == 1 && raster.valid &&
              raster.real_asset_matched && raster.source_surface_count == 2 &&
              raster.pixel_hash == host_surface.raster.pixel_hash &&
              raster.route_hash == host_surface.raster.route_hash,
          "C017/C040 presented HUD capture is stable across direct and host routes");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host_surface);
}

static int real_c001_title_plan(int title_frame,
                                CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_StartupPresentationReceipt_PC34 receipt;

    if (!out_plan) return 0;
    csb_v1_startup_host_facts_init_pc34(&facts);
    facts.title_active = 1;
    facts.entrance_active = 1;
    facts.title_frame = title_frame;
    facts.title_source_step = (int)
        csb_v1_startup_title_source_step_for_frame_pc34(title_frame);
    if (!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
            &facts, &receipt) || !receipt.valid) {
        return 0;
    }
    *out_plan = receipt.render_plan;
    return 1;
}

static void verify_real_c001_title_sequence(CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupRenderPlan_PC34 plan;
    const unsigned char *rgb;

    if (!session) return;

    CHECK(csb_v1_startup_title_presents_ticks_pc34() == 60 &&
              csb_v1_startup_title_chaos_zoom_ticks_pc34() == 20 &&
              csb_v1_startup_title_chaos_hold_ticks_pc34() == 20 &&
              csb_v1_startup_title_strikes_back_ticks_pc34() == 2 &&
              csb_v1_startup_title_total_ticks_pc34() == 102,
          "C001 title timing matches TITLE.C F0437's PC 20-frame cadence");

    CHECK(real_c001_title_plan(0, &plan) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34 &&
              plan.title_source_x == 0 && plan.title_source_y == 137 &&
              plan.title_source_w == 320 && plan.title_source_h == 16 &&
              plan.title_dest_x == 0 && plan.title_dest_y == 90 &&
              plan.title_dest_w == 320 && plan.title_dest_h == 16 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS,
          "C001 PRESENTS uses the source crop, destination, and palette phase");
    rgb = F9011_VGA_GetSpecialColorRgb_Compat(
        0, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS);
    CHECK(title_palette_matches_rgb(rgb, 0, 0, 109) &&
              title_palette_matches_rgb(
                  F9011_VGA_GetSpecialColorRgb_Compat(
                      15, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS),
                  255, 255, 255),
          "C001 PRESENTS keeps TITLE.C's dark-blue base and white index 15");
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              session, &plan, 0u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid,
          "C001 PRESENTS preserves the verified source raster order");
    if (raster.valid) {
        CHECK(raster.pixel_hash == 0x38c165c5u,
              "C001 PRESENTS raster hash matches verified PC CSB graphics");
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    }

    CHECK(real_c001_title_plan(60, &plan) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 2 &&
              plan.title_source_x == 0 && plan.title_source_y == 0 &&
              plan.title_source_w == 320 && plan.title_source_h == 80 &&
              plan.title_dest_x == 152 && plan.title_dest_y == 78 &&
              plan.title_dest_w == 16 && plan.title_dest_h == 4 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS &&
              csb_v1_boot_startup_runtime_asset_session_frame_pc34(
                  session, &plan, 60u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid,
          "C001 CHAOS first raster preserves TITLE.C source order and 16x4 geometry");
    if (raster.valid) {
        CHECK(raster.pixel_hash == 0xc57f9804u,
              "C001 CHAOS first raster capture matches verified PC CSB graphics");
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    }
    CHECK(title_palette_matches_rgb(
                  F9011_VGA_GetSpecialColorRgb_Compat(
                      3, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS),
                  188, 156, 60) &&
              title_palette_matches_rgb(
                  F9011_VGA_GetSpecialColorRgb_Compat(
                      10, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS),
                  0, 0, 109),
          "C001 CHAOS keeps gold artwork indices over the dark-blue source base");

    CHECK(real_c001_title_plan(100, &plan) &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34 &&
              plan.title_source_x == 0 && plan.title_source_y == 80 &&
              plan.title_source_w == 320 && plan.title_source_h == 57 &&
              plan.title_dest_x == 0 && plan.title_dest_y == 118 &&
              plan.title_dest_w == 320 && plan.title_dest_h == 57 &&
              plan.title_transparent_color == 0 &&
              plan.title_special_palette ==
                  VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES &&
              csb_v1_boot_startup_runtime_asset_session_frame_pc34(
                  session, &plan, 80u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid,
          "C001 STRIKES BACK retains source crop, black key, and final palette phase");
    if (raster.valid) {
        CHECK(raster.pixel_hash == 0x8e96cf09u,
              "C001 STRIKES BACK raster capture matches verified PC CSB graphics");
        csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    }
    CHECK(title_palette_matches_rgb(
                  F9011_VGA_GetSpecialColorRgb_Compat(
                      10, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES),
                  0, 0, 0) &&
              title_palette_matches_rgb(
                  F9011_VGA_GetSpecialColorRgb_Compat(
                      12, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES),
                  255, 0, 0),
          "C001 STRIKES BACK applies TITLE.C's final black/red slot changes");
}

static void verify_real_indexed_startup(
    CSB_V1_BootProfile *profile,
    const CSB_V1_StartupRealReceipt *real_asset_receipt)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host_surface;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 presents_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 chaos_host;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 strikes_host;
    CSB_V1_StartupFullRuntimeReceipt_PC34 receipt;
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package_receipt;
    CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 handoff_receipt;
    CSB_V1_RuntimeStartupPackageHandoffReceipt_PC34 door_handoff_receipt;
    CSB_V1_StartupSessionPackageTitleReceipt_PC34 title_package_receipt;
    CSB_V1_RuntimeStartupTitlePackageHandoffReceipt_PC34 title_handoff_receipt;
    CSB_V1_StartupSessionOpeningDoorReceipt_PC34 opening_door_receipt;
    CSB_V1_RuntimeStartupTitleDoorHandoffReceipt_PC34 title_door_handoff_receipt;
    CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 consumption_receipt;
    CSB_V1_RuntimeStartupTitleOpeningConsumptionHandoffReceipt_PC34
        consumption_handoff_receipt;
    CSB_V1_StartupSessionTerminalReceipt_PC34 terminal_receipt;
    CSB_V1_StartupSessionTerminalPackageReceipt_PC34 terminal_package_receipt;
    CSB_V1_StartupSessionLiveHudReceipt_PC34 live_hud_receipt;
    CSB_V1_StartupSessionDoorHudTickReceipt_PC34 first_door_receipt;
    CSB_V1_StartupSessionInputReceipt_PC34 first_input_receipt;
    CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 hud_door_input_receipt;
    CSB_V1_RuntimeStartupHudDoorInputHandoffReceipt_PC34
        hud_door_input_handoff_receipt;
    CSB_V1_StartupEntranceInputOutcome_PC34 input_outcome;
    CSB_V1_StartupRuntimeApplyReceipt_PC34 runtime_apply;
    CSB_V1_StartupCommandStateReceipt_PC34 state_receipt;
    CSB_V1_StartupRenderPlan_PC34 plan;

    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(profile, &session) == 1 &&
              session.valid && session.full_startup_ready &&
              session.rejects_legacy_wrappers,
          "verified GRAPHICS.DAT opens one owned indexed startup session");
    if (!session.valid) return;

    verify_real_c001_title_sequence(&session);
    verify_real_c017_c040_hud_handoff(&session);
    CHECK(csb_v1_startup_real_package_consumption_receipt_from_session_pc34(
              real_asset_receipt, &session, &package_receipt) == 1 &&
              package_receipt.valid && package_receipt.real_package_matched &&
              package_receipt.c001_title_consumed &&
              package_receipt.c001_presents_consumed &&
              package_receipt.c001_chaos_consumed &&
              package_receipt.c001_strikes_back_consumed &&
              package_receipt.c017_hud_consumed &&
              package_receipt.c040_hud_consumed &&
              package_receipt.title_to_hud_same_session &&
              package_receipt.no_legacy_wrappers &&
              package_receipt.no_fallback_routes &&
              package_receipt.real_asset_receipt_hash ==
                  real_asset_receipt->receipt_hash &&
              package_receipt.consumed_surface_hash != 0u,
          "PC34 package receipt binds C001 title and C017/C040 HUD pixels to one real session");

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
    plan.title_dest_y = 90;
    plan.title_dest_w = 320;
    plan.title_dest_h = 16;
    plan.title_transparent_color = -1;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 1u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid &&
              raster.title_composited && raster.source_surface_count == 1 &&
              raster.real_asset_matched,
          "C001 PRESENTS reaches a 320x200 indexed runtime raster");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    CHECK(csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
              &session, &plan, 1u, &presents_host) == 1 && presents_host.valid &&
              presents_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 &&
              presents_host.raster.title_composited,
          "runtime host consumes the real C001 PRESENTS surface");

    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34;
    plan.title_dest_x = 136;
    plan.title_dest_y = 74;
    plan.title_dest_w = 48;
    plan.title_dest_h = 12;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 2u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid &&
              raster.title_composited && raster.pixel_hash != 0u,
          "C001 CHAOS zoom reaches the indexed runtime raster");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    CHECK(csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
              &session, &plan, 2u, &chaos_host) == 1 && chaos_host.valid &&
              chaos_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 &&
              chaos_host.raster.title_composited,
          "runtime host consumes the real C001 CHAOS surface");

    plan.title_stage = CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
    plan.title_dest_x = 0;
    plan.title_dest_y = 118;
    plan.title_dest_w = 320;
    plan.title_dest_h = 57;
    plan.title_transparent_color = 0;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 3u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid &&
              raster.title_composited && raster.pixel_hash != 0u,
          "C001 STRIKES BACK reaches the indexed runtime raster");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    CHECK(csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
              &session, &plan, 3u, &strikes_host) == 1 && strikes_host.valid &&
              strikes_host.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 &&
              strikes_host.raster.title_composited,
          "runtime host consumes the real C001 STRIKES BACK surface");

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.surface_w = 320;
    plan.surface_h = 200;
    plan.closed_left_w = 105;
    plan.closed_left_h = 161;
    plan.closed_left_dest_y = 30;
    plan.closed_right_w = 127;
    plan.closed_right_h = 161;
    plan.closed_right_dest_x = 105;
    plan.closed_right_dest_y = 30;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 4u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid &&
              raster.entrance_composited && raster.door_composited &&
              raster.source_surface_count == 3,
          "C004 plus closed C002/C003 reaches one indexed entrance raster");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34;
    plan.surface_w = 320;
    plan.surface_h = 200;
    plan.opening_composite_valid = 1;
    plan.opening_left_source_x = 0;
    plan.opening_left_w = 97;
    plan.opening_left_h = 161;
    plan.opening_left_dest_y = 30;
    plan.opening_right_source_x = 8;
    plan.opening_right_w = 119;
    plan.opening_right_h = 161;
    plan.opening_right_dest_x = 113;
    plan.opening_right_dest_y = 30;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 5u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid &&
              raster.door_composited && raster.source_surface_count == 3 &&
              raster_rows_match_surface(&raster, frame.entrance_surface, 30),
          "C004 opening frame preserves its first 30 rows before C002/C003 strips");
    if (raster.valid) {
        CHECK(raster.pixel_hash == 0xde9c254eu,
              "C004/C002/C003 opening capture matches verified PC CSB graphics");
    }
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    CHECK(csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
              &session, &plan, 5u, &host_surface) == 1 && host_surface.valid &&
              host_surface.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34 &&
              host_surface.door_opening_decision &&
              host_surface.raster.door_composited &&
              host_surface.raster.source_surface_count == 3 &&
              host_surface.no_synthetic_surface,
          "runtime host opening decision consumes C004 plus original C002/C003 strips");
    memset(&input_outcome, 0, sizeof(input_outcome));
    memset(&runtime_apply, 0, sizeof(runtime_apply));
    memset(&state_receipt, 0, sizeof(state_receipt));
    input_outcome.result = CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
    runtime_apply.result = CSB_V1_STARTUP_RUNTIME_APPLY_REDRAW_PC34;
    state_receipt.entrance_active = 1;
    state_receipt.opening_active = 1;
    state_receipt.opening_step = 1;
    CHECK(csb_v1_runtime_startup_package_handoff_receipt_from_transition_pc34(
              &package_receipt, &host_surface, &input_outcome, &runtime_apply,
              &state_receipt, &handoff_receipt) == 1 && handoff_receipt.valid &&
              handoff_receipt.door_opening_transition &&
              handoff_receipt.same_session_generation &&
              handoff_receipt.no_synthetic_surface,
          "opening input/runtime transition retains the verified PC34 package receipt");
    CHECK(csb_v1_startup_session_opening_door_receipt_pc34(
              &session, &package_receipt, &host_surface,
              &opening_door_receipt) == 1 && opening_door_receipt.valid &&
              opening_door_receipt.c004_entrance_ready &&
              opening_door_receipt.c002_left_door_ready &&
              opening_door_receipt.c003_right_door_ready,
          "C004/C002/C003 opening frame remains in the verified C001 package session");
    CHECK(csb_v1_startup_session_title_opening_consumption_receipt_pc34(
              &session, &package_receipt, &presents_host, &chaos_host,
              &strikes_host, &host_surface, &consumption_receipt) == 1 &&
              consumption_receipt.valid && consumption_receipt.presents_consumed &&
              consumption_receipt.chaos_consumed &&
              consumption_receipt.strikes_back_consumed &&
              consumption_receipt.c004_c002_c003_consumed,
          "real C001 title hosts and C004/C002/C003 opening host share one package session");
    CHECK(csb_v1_runtime_startup_title_opening_consumption_handoff_receipt_pc34(
              &consumption_receipt, &handoff_receipt,
              &consumption_handoff_receipt) == 1 && consumption_handoff_receipt.valid &&
              consumption_handoff_receipt.real_title_opening_consumption &&
              consumption_handoff_receipt.same_session_generation &&
              consumption_handoff_receipt.no_legacy_wrappers &&
              consumption_handoff_receipt.no_synthetic_surface,
          "runtime bridge retains real C001 and C004/C002/C003 host consumption");
    door_handoff_receipt = handoff_receipt;
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host_surface);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&presents_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&chaos_host);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&strikes_host);

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    CHECK(csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
              &session, &plan, 102u, &host_surface) == 1 && host_surface.valid &&
              host_surface.host_surface ==
                  CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34,
          "terminal session exposes the original C017/C040 runtime host surface");
    memset(&state_receipt, 0, sizeof(state_receipt));
    state_receipt.entrance_dismissed = 1;
    CHECK(csb_v1_runtime_startup_package_handoff_receipt_from_transition_pc34(
              &package_receipt, &host_surface, &input_outcome, &runtime_apply,
              &state_receipt, &handoff_receipt) == 1 && handoff_receipt.valid &&
              handoff_receipt.hud_runtime_transition &&
              handoff_receipt.real_asset_receipt_hash ==
                  package_receipt.real_asset_receipt_hash &&
              handoff_receipt.consumed_surface_hash ==
                  package_receipt.consumed_surface_hash,
          "HUD input/runtime transition retains the same verified PC34 package receipt");
    CHECK(csb_v1_startup_session_package_title_receipt_pc34(
              &session, &package_receipt, &title_package_receipt) == 1 &&
              title_package_receipt.valid &&
              title_package_receipt.c001_presents_ready &&
              title_package_receipt.c001_chaos_ready &&
              title_package_receipt.c001_strikes_back_ready &&
              title_package_receipt.title_to_hud_same_session,
          "C001 PRESENTS/CHAOS/STRIKES remain bound to the terminal PC34 package session");
    CHECK(csb_v1_startup_session_terminal_receipt_pc34(
              &session, &terminal_receipt) == 1 && terminal_receipt.valid &&
              terminal_receipt.c017_ready && terminal_receipt.c040_ready,
          "terminal C017/C040 HUD session has an owned PC34 receipt");
    CHECK(csb_v1_startup_session_terminal_package_receipt_pc34(
              &session, &package_receipt, &terminal_package_receipt) == 1 &&
              terminal_package_receipt.valid &&
              terminal_package_receipt.c001_title_consumed &&
              terminal_package_receipt.c017_hud_consumed &&
              terminal_package_receipt.c040_hud_consumed &&
              terminal_package_receipt.terminal_f0807_complete &&
              terminal_package_receipt.real_asset_receipt_hash ==
                  real_asset_receipt->receipt_hash &&
              terminal_package_receipt.consumed_surface_hash ==
                  package_receipt.consumed_surface_hash,
          "terminal F0807 retains hash-verified C001/C017/C040 package pixels");
    CHECK(csb_v1_startup_session_live_hud_receipt_pc34(
              &session, &terminal_receipt, 1u, terminal_receipt.source_tick,
              terminal_receipt.session_generation, &live_hud_receipt) == 1 &&
              live_hud_receipt.valid && live_hud_receipt.c017_live_base_only,
          "C040 clear returns to the real C017 HUD before runtime input");
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, terminal_receipt.source_tick + 1u, &frame) == 1 &&
              frame.valid && frame.session_generation ==
                  terminal_receipt.session_generation,
          "first runtime tick retains the terminal PC34 HUD session");
    CHECK(csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
              &session, &live_hud_receipt, 0u, 1u,
              live_hud_receipt.source_tick + 1u,
              live_hud_receipt.session_generation, &first_door_receipt) == 1 &&
              first_door_receipt.valid && first_door_receipt.first_live_door_tick,
          "first live door frame follows the C017/C040 HUD session");
    CHECK(csb_v1_startup_session_first_input_receipt_pc34(
              &session, &live_hud_receipt,
              CSB_V1_STARTUP_SESSION_MOVEMENT_FORWARD_PC34,
              live_hud_receipt.source_tick + 1u,
              live_hud_receipt.session_generation, &first_input_receipt) == 1 &&
              first_input_receipt.valid && first_input_receipt.first_post_c040_input,
          "first runtime input follows the same C017 HUD session");
    CHECK(csb_v1_startup_session_hud_door_input_package_receipt_pc34(
              &session, &package_receipt, &host_surface, &live_hud_receipt,
              &first_door_receipt, &first_input_receipt,
              &hud_door_input_receipt) == 1 && hud_door_input_receipt.valid &&
              hud_door_input_receipt.c017_hud_consumed &&
              hud_door_input_receipt.c040_hud_consumed &&
              hud_door_input_receipt.first_live_door_frame &&
              hud_door_input_receipt.first_runtime_input,
          "C017/C040 HUD, first door frame, and input share one PC34 package host");
    CHECK(csb_v1_runtime_startup_hud_door_input_handoff_receipt_pc34(
              &hud_door_input_receipt, &handoff_receipt,
              &hud_door_input_handoff_receipt) == 1 &&
              hud_door_input_handoff_receipt.valid &&
              hud_door_input_handoff_receipt.real_hud_door_input_consumption &&
              hud_door_input_handoff_receipt.same_session_generation &&
              hud_door_input_handoff_receipt.no_legacy_wrappers &&
              hud_door_input_handoff_receipt.no_synthetic_surface,
          "runtime bridge retains real C017/C040 HUD through first door and input");
    CHECK(csb_v1_runtime_startup_title_door_handoff_receipt_pc34(
              &title_package_receipt, &opening_door_receipt,
              &door_handoff_receipt, &title_door_handoff_receipt) == 1 &&
              title_door_handoff_receipt.valid &&
              title_door_handoff_receipt.full_title_to_opening_package_bound &&
              title_door_handoff_receipt.same_session_generation &&
              title_door_handoff_receipt.no_legacy_wrappers &&
              title_door_handoff_receipt.no_synthetic_surface,
          "C001 title phases hand off to original C004/C002/C003 opening runtime");
    CHECK(csb_v1_runtime_startup_title_package_handoff_receipt_pc34(
              &title_package_receipt, &handoff_receipt,
              &title_handoff_receipt) == 1 && title_handoff_receipt.valid &&
              title_handoff_receipt.full_title_to_hud_package_bound &&
              title_handoff_receipt.same_session_generation &&
              title_handoff_receipt.no_legacy_wrappers &&
              title_handoff_receipt.no_synthetic_surface,
          "runtime handoff retains all C001 title phases and the same PC34 package identity");
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&host_surface);

    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session, &receipt) == 1 && receipt.valid &&
              receipt.title_to_hud_same_session && receipt.no_legacy_wrappers,
          "indexed title and entrance session hands off to the C017/C040 HUD owner");
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *dir = pc_data_dir(argc, argv, default_dir, sizeof(default_dir));
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRealReceipt real_asset_receipt;
    const CSB_V1_DungeonData *current;
    uint64_t before_play_ms;

    printf("=== CSB V1 PC real-asset launch probe ===\n\n");
    printf("data_dir=%s\n", dir ? dir : "(none)");

    if (!pc_data_present(dir)) {
        printf("SKIP: PC CSB GRAPHICS.DAT + DUNGEON.DAT not available; "
               "set FIRESTAFF_CSB_PC_DATA to enable the real-data gate.\n");
        return 0;
    }

    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_startup_real_scan_and_receipt(
              dir, 4, &real_asset_receipt) == CSB_V1_STARTUP_REAL_OK &&
              real_asset_receipt.matched &&
              real_asset_receipt.variant_id == CSB_V1_VARIANT_PC34_EN &&
              real_asset_receipt.receipt_hash != 0u,
          "PC CSB package metadata has a verified real-asset receipt");
    CHECK(csb_v1_boot_scan_assets(&profile, dir) == 0,
          "PC CSB assets scan by hash");
    CHECK(profile.assets_verified == 1, "boot profile marks assets verified");
    CHECK(profile.graphics_verified == 1, "GRAPHICS.DAT is verified");
    CHECK(profile.dungeon_verified == 1, "DUNGEON.DAT is verified");
    CHECK(strcmp(profile.graphics_md5, "61fbfd56887c94adc26888a9491c6611") == 0,
          "PC CSB graphics MD5 matches canonical PC 3.4 English");
    CHECK(strcmp(profile.dungeon_md5, "6695d2acebce49f95db1d8f3a5c733de") == 0,
          "CSB dungeon MD5 matches shared canonical dungeon");
    CHECK(profile.variant_id == CSB_V1_VARIANT_PC34_EN,
          "variant detection selects PC DOS 3.4 English");
    CHECK(profile.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS,
          "graphics archive kind is ordinary GRAPHICS.DAT");

    verify_real_indexed_startup(&profile, &real_asset_receipt);

    CHECK(csb_v1_boot_enter_game(&profile) == 0,
          "boot profile enters the CSB V1 runtime");
    CHECK(profile.state == CSB_V1_BOOT_STATE_RUNTIME_READY,
          "boot profile advances to runtime-ready");
    CHECK(profile.runtime.state == CSB_STATE_TITLE,
          "runtime starts at the title/entrance state");
    CHECK(profile.runtime.variant_id == CSB_V1_VARIANT_PC34_EN,
          "runtime carries the PC CSB variant");
    CHECK(profile.runtime.dungeon_handle != NULL,
          "runtime owns a loaded dungeon handle");
    current = csb_v1_dungeon_get_current();
    CHECK(current == profile.runtime.dungeon_handle,
          "current dungeon singleton points at the runtime-owned dungeon");
    CHECK(current && current->level_count > 0,
          "loaded PC dungeon exposes at least one level");
    CHECK(csb_v1_dungeon_get_current_level() == 0,
          "new-game dungeon level is map 0");
    CHECK(profile.runtime.party_x == CSB_V1_START_PARTY_X &&
          profile.runtime.party_y == CSB_V1_START_PARTY_Y &&
          profile.runtime.party_dir == CSB_V1_START_PARTY_DIR,
          "runtime keeps the source-locked CSB start pose");
    CHECK(profile.runtime.chaos_magic.magic_initialized == 1,
          "CSB chaos magic is initialized at handoff");

    before_play_ms = profile.runtime.total_play_ms;
    csb_v1_runtime_tick(&profile.runtime, CSB_V1_TICK_MS_NOMINAL);
    CHECK(profile.runtime.total_play_ms > before_play_ms,
          "one CSB V1 tick advances play time");

    csb_v1_boot_cleanup(&profile);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "boot cleanup clears the current dungeon context");

    printf("\nchecks=%d failures=%d\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
