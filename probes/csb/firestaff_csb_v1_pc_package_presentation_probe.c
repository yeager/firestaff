/*
 * Real PC CSB package presentation probe.
 *
 * This is deliberately an opt-in, no-fixture probe. It opens only a
 * hash-verified PC 3.4 GRAPHICS.DAT + DUNGEON.DAT pair and drives the
 * production startup session through ReDMCSB TITLE.C F0437 and ENTRANCE.C
 * F0438/F0807. The session owns all decoded pixels; this probe never creates
 * an image, palette, or fallback surface.
 *
 * CSBWin reference: Graphics.cpp ReadGraphic consumes the same indexed
 * graphics contract. Custom CSBgraphics.dat remains outside this path until
 * an independently hash-registered package is supplied.
 */

#include "csb_v1_boot.h"
#include "csb_v1_startup_real_asset_receipt.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(condition, message) do { \
    ++checks; \
    if (condition) { \
        printf("  PASS: %s\n", message); \
    } else { \
        ++failures; \
        printf("  FAIL: %s\n", message); \
    } \
} while (0)

static const char *data_dir_from_args(int argc, char **argv,
                                      char *default_dir, size_t default_size)
{
    const char *value;
    const char *home;

    if (argc > 1 && argv[1] && argv[1][0] != '\0') return argv[1];
    value = getenv("FIRESTAFF_CSB_PC_DATA");
    if (value && value[0] != '\0') return value;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(default_dir, default_size, "%s/.firestaff/data/csb", home);
    return default_dir;
}

static int host_surface(CSB_V1_StartupRuntimeAssetSession_PC34 *session,
                        const CSB_V1_StartupRenderPlan_PC34 *plan,
                        unsigned int tick,
                        int expected_sources,
                        const char *message)
{
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 receipt;
    int ok;

    memset(&receipt, 0, sizeof(receipt));
    ok = csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
             session, plan, tick, &receipt) == 1 && receipt.valid &&
         receipt.raster.valid && receipt.raster.real_asset_matched &&
         receipt.raster.source_surface_count == expected_sources &&
         receipt.no_synthetic_surface && receipt.raster.pixel_hash != 0u;
    CHECK(ok, message);
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&receipt);
    return ok;
}

static int title_plan(int title_frame,
                      const CSB_V1_StartupRenderPlan_PC34 *playback_plan)
{
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_StartupPresentationReceipt_PC34 receipt;

    if (!playback_plan) return 0;
    csb_v1_startup_host_facts_init_pc34(&facts);
    memset(&receipt, 0, sizeof(receipt));
    facts.title_active = 1;
    facts.entrance_active = 1;
    facts.title_frame = title_frame;
    /* The M11 playback boundary and source state adapter publish the C001
     * rectangle for the same title frame.  Comparing with the preceding
     * frame rejects valid CHAOS and STRIKES presentations at phase changes. */
    facts.title_source_step = (int)
        csb_v1_startup_title_source_step_for_frame_pc34(title_frame);
    if (!csb_v1_startup_presentation_receipt_from_host_facts_pc34(
            &facts, &receipt) || !receipt.valid) {
        return 0;
    }
    return playback_plan->surface == receipt.render_plan.surface &&
        playback_plan->source_asset_id == receipt.render_plan.source_asset_id &&
        playback_plan->title_stage == receipt.render_plan.title_stage &&
        playback_plan->title_source_step == receipt.render_plan.title_source_step &&
        playback_plan->title_blit_kind == receipt.render_plan.title_blit_kind &&
        playback_plan->title_source_x == receipt.render_plan.title_source_x &&
        playback_plan->title_source_y == receipt.render_plan.title_source_y &&
        playback_plan->title_source_w == receipt.render_plan.title_source_w &&
        playback_plan->title_source_h == receipt.render_plan.title_source_h &&
        playback_plan->title_dest_x == receipt.render_plan.title_dest_x &&
        playback_plan->title_dest_y == receipt.render_plan.title_dest_y &&
        playback_plan->title_dest_w == receipt.render_plan.title_dest_w &&
        playback_plan->title_dest_h == receipt.render_plan.title_dest_h &&
        playback_plan->special_palette == receipt.render_plan.special_palette &&
        playback_plan->asset_command_count == receipt.render_plan.asset_command_count;
}

int main(int argc, char **argv)
{
    char default_dir[1024];
    const char *data_dir = data_dir_from_args(argc, argv, default_dir,
                                              sizeof(default_dir));
    CSB_V1_StartupRealReceipt real_package;
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupAudioAction_PC34 audio;
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_StartupPresentationReceipt_PC34 entrance;
    CSB_V1_StartupFullRuntimeReceipt_PC34 runtime;
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 consumption;
    int scan_result;

    printf("=== CSB V1 PC package presentation probe ===\n");
    printf("data_dir=%s\n", data_dir ? data_dir : "(none)");
    if (!data_dir) {
        printf("SKIP: no PC CSB data directory is configured.\n");
        return 0;
    }

    csb_v1_startup_real_receipt_init(&real_package);
    scan_result = csb_v1_startup_real_scan_and_receipt(data_dir, 4,
                                                        &real_package);
    if (scan_result != CSB_V1_STARTUP_REAL_OK || !real_package.matched) {
        printf("SKIP: verified PC CSB package unavailable (result=%s).\n",
               csb_v1_startup_real_result_name(scan_result));
        return 0;
    }

    CHECK(real_package.variant_id == CSB_V1_VARIANT_PC34_EN &&
              real_package.graphics_kind == CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS &&
              real_package.receipt_hash != 0u,
          "hash-verified PC34 package is the only accepted presentation source");
    if (failures != 0) return 1;

    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, data_dir) == 0 &&
              profile.assets_verified &&
              profile.variant_id == CSB_V1_VARIANT_PC34_EN,
          "boot profile resolves the same verified PC package");
    CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(&profile,
                                                               &session) == 1 &&
              session.valid && session.real_asset_matched &&
              session.rejects_legacy_wrappers,
          "startup session admits only decoded package surfaces");
    if (failures != 0) return 1;

    memset(&audio, 0, sizeof(audio));
    CHECK(csb_v1_boot_startup_playback_begin_pc34(&session, &audio) == 1 &&
              csb_v1_boot_startup_playback_complete_swoosh_pc34(&session,
                                                                  &audio) == 1,
          "production playback enters TITLE.C without a replacement route");

    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 0, &plan,
                                                         &audio) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34,
          "PRESENTS selects its ReDMCSB title phase");
    CHECK(title_plan(0, &plan) == 1,
          "PRESENTS receives TITLE.C source geometry from the production receipt");
    host_surface(&session, &plan, 0u, 1, "PRESENTS presents decoded C001 only");

    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 60, &plan,
                                                         &audio) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
          "CHAOS selects its ReDMCSB title phase");
    CHECK(title_plan(60, &plan) == 1,
          "CHAOS receives TITLE.C source geometry from the production receipt");
    host_surface(&session, &plan, 60u, 1, "CHAOS presents decoded C001 only");

    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 80, &plan,
                                                         &audio) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34 &&
              plan.title_source_step == 21,
          "full CHAOS hold retains TITLE.C's distinct source step");
    CHECK(title_plan(80, &plan) == 1 && plan.title_source_step == 21,
          "full CHAOS hold receives TITLE.C source geometry from the production receipt");
    host_surface(&session, &plan, 80u, 1,
                 "full CHAOS hold presents decoded C001 only");

    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(&session, 100, &plan,
                                                         &audio) == 1 &&
              plan.title_stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34,
          "STRIKES BACK selects its ReDMCSB title phase");
    CHECK(title_plan(100, &plan) == 1,
          "STRIKES BACK receives TITLE.C source geometry from the production receipt");
    host_surface(&session, &plan, 100u, 1,
                 "STRIKES BACK presents decoded C001 only");
    CHECK(csb_v1_boot_startup_playback_title_frame_pc34(
              &session, csb_v1_startup_title_total_ticks_pc34(), &plan,
              &audio) == 1,
          "TITLE.C terminal tick hands ownership to ENTRANCE.C");

    csb_v1_startup_host_facts_init_pc34(&facts);
    facts.entrance_active = 1;
    facts.credits_active = 1;
    facts.credits_remaining_ticks = csb_v1_startup_entrance_credits_ticks_pc34();
    CHECK(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &entrance) == 1 && entrance.valid &&
              entrance.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34 &&
              entrance.render_plan.source_asset_id == 5,
          "ENTRANCE.C selects C005 for its credits phase");
    host_surface(&session, &entrance.render_plan, 101u, 1,
                 "credits presents decoded C005 package pixels only");

    facts.credits_active = 0;
    facts.credits_remaining_ticks = 0;
    facts.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    CHECK(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &entrance) == 1 && entrance.valid &&
              entrance.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34,
          "ENTRANCE.C selects the closed C004/C002/C003 composition");
    host_surface(&session, &entrance.render_plan, 102u, 3,
                 "closed entrance presents C004/C002/C003 package pixels");

    facts.opening_active = 1;
    facts.opening_delay_ticks = 0;
    facts.opening_step = 2;
    CHECK(csb_v1_startup_presentation_receipt_from_host_facts_pc34(
              &facts, &entrance) == 1 && entrance.valid &&
              entrance.render_plan.surface ==
                  CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34,
          "ENTRANCE.C selects the opening-door composition");
    host_surface(&session, &entrance.render_plan, 103u, 3,
                 "opening door presents C004/C002/C003 package pixels");

    CHECK(csb_v1_boot_startup_playback_complete_entrance_pc34(&session) == 1 &&
              csb_v1_boot_startup_playback_enter_hud_pc34(&session) == 1,
          "F0807 completion releases the package-backed HUD");
    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    plan.special_palette = -1;
    plan.title_special_palette = -1;
    host_surface(&session, &plan, 104u, 2,
                 "C017/C040 HUD presents package pixels without a wrapper");

    memset(&runtime, 0, sizeof(runtime));
    memset(&consumption, 0, sizeof(consumption));
    CHECK(csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
              &session, &runtime) == 1 && runtime.valid &&
              runtime.title_to_hud_same_session && runtime.no_legacy_wrappers,
          "one session reaches title, door, and HUD");
    CHECK(csb_v1_startup_real_package_consumption_receipt_from_session_pc34(
              &real_package, &session, &consumption) == 1 && consumption.valid &&
              consumption.no_fallback_routes && consumption.c001_presents_consumed &&
              consumption.c001_chaos_consumed &&
              consumption.c001_chaos_zoom_consumed &&
              consumption.c001_chaos_hold_consumed &&
              consumption.c001_strikes_back_consumed &&
              consumption.c002_left_door_consumed &&
              consumption.c003_right_door_consumed &&
              consumption.c004_entrance_consumed &&
              consumption.c005_credits_consumed &&
              consumption.title_to_entrance_same_session &&
              consumption.c017_hud_consumed && consumption.c040_hud_consumed,
          "terminal receipt binds C001-C005 and C017/C040 to one package");

    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
