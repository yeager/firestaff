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

static void verify_real_indexed_startup(CSB_V1_BootProfile *profile)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupFullRuntimeReceipt_PC34 receipt;
    CSB_V1_StartupRenderPlan_PC34 plan;

    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(profile, &session) == 1 &&
              session.valid && session.full_startup_ready &&
              session.rejects_legacy_wrappers,
          "verified GRAPHICS.DAT opens one owned indexed startup session");
    if (!session.valid) return;

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

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.surface_w = 320;
    plan.surface_h = 200;
    plan.closed_left_w = 105;
    plan.closed_left_h = 161;
    plan.closed_left_dest_y = 28;
    plan.closed_right_w = 127;
    plan.closed_right_h = 161;
    plan.closed_right_dest_x = 105;
    plan.closed_right_dest_y = 28;
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
    plan.opening_left_dest_y = 28;
    plan.opening_right_source_x = 8;
    plan.opening_right_w = 119;
    plan.opening_right_h = 161;
    plan.opening_right_dest_x = 113;
    plan.opening_right_dest_y = 28;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, 5u, &frame) == 1 &&
              csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                  &frame, &plan, &raster) == 1 && raster.valid &&
              raster.door_composited && raster.source_surface_count == 3 &&
              raster_rows_match_surface(&raster, frame.entrance_surface, 28),
          "C004 opening frame preserves its first 28 rows before C002/C003 strips");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);

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

    verify_real_indexed_startup(&profile);

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
