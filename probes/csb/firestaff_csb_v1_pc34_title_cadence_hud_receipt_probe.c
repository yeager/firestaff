/*
 * Opt-in PC34 CSB title-to-HUD receipt probe.
 *
 * ReDMCSB TITLE.C F0437 owns PRESENTS/CHAOS/STRIKES cadence; ENTRANCE.C
 * F0807 owns the C002/C003 terminal door sequence; PANEL.C F0346/F0347
 * owns C040/C017. This probe accepts only user-supplied hash-verified data.
 */

#include "csb_v1_boot.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(condition, message) do { \
    ++checks; \
    if (!(condition)) { ++failures; printf("FAIL: %s\n", message); } \
    else { printf("PASS: %s\n", message); } \
} while (0)

static const char *probe_data_dir(int argc, char **argv, char *fallback,
                                  size_t fallback_size)
{
    const char *env = getenv("FIRESTAFF_CSB_PC_DATA");
    const char *home;
    if (argc > 1 && argv[1] && argv[1][0]) return argv[1];
    if (env && env[0]) return env;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(fallback, fallback_size, "%s/.firestaff/data/csb", home);
    return fallback;
}

static int title_palette_for_stage(int stage)
{
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34)
        return VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS;
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34)
        return VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
    if (stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34)
        return VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES;
    return -1;
}

static void opening_plan(int step, CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int left_right = 100 - 4 * (step - 1);
    int right_left = 109 + 4 * (step - 1);
    memset(plan, 0, sizeof(*plan));
    plan->surface = CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34;
    plan->special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    plan->opening_door_valid = 1;
    plan->opening_door_step = step;
    plan->opening_composite_valid = 1;
    plan->opening_composite_screen_asset_id = 4;
    plan->opening_composite_left_asset_id = 2;
    plan->opening_composite_right_asset_id = 3;
    plan->opening_composite_animation_step = step;
    if (left_right >= 0) {
        plan->opening_left_source_x = (step & 0x00fc) << 2;
        plan->opening_left_dest_y = 30;
        plan->opening_left_w = left_right + 1;
        plan->opening_left_h = 161;
        plan->opening_composite_left_source_x = plan->opening_left_source_x;
        plan->opening_composite_left_box_w = plan->opening_left_w;
    }
    if (right_left <= 231) {
        plan->opening_right_source_x = (step & 3) << 2;
        plan->opening_right_dest_x = right_left;
        plan->opening_right_dest_y = 30;
        plan->opening_right_w = 232 - right_left;
        plan->opening_right_h = 161;
        plan->opening_composite_right_source_x = plan->opening_right_source_x;
        plan->opening_composite_right_box_x = plan->opening_right_dest_x;
        plan->opening_composite_right_box_w = plan->opening_right_w;
    }
}

int main(int argc, char **argv)
{
    char fallback[1024];
    const char *data_dir = probe_data_dir(argc, argv, fallback, sizeof(fallback));
    CSB_V1_BootProfile profile;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    CSB_V1_StartupRenderPlan_PC34 plan;
    CSB_V1_StartupAudioAction_PC34 audio;
    CSB_V1_StartupCompleteTimelineReceipt_PC34 terminal;
    CSB_V1_TerminalUiStateReceipt_PC34 terminal_ui;
    uint64_t play_ms_before;
    int first_door_rasterized = 0;
    int title_frame;
    int step;
    uint32_t tick = 0u;

    printf("=== CSB PC34 title cadence to HUD receipt ===\n");
    if (!data_dir) {
        puts("SKIP: no CSB PC34 data directory");
        return 0;
    }
    csb_v1_boot_profile_init(&profile);
    if (csb_v1_boot_scan_assets(&profile, data_dir) != 0 ||
        !profile.assets_verified || !profile.graphics_verified ||
        profile.variant_id != CSB_V1_VARIANT_PC34_EN) {
        puts("SKIP: FIRESTAFF_CSB_PC_DATA has no verified PC34 CSB pair");
        csb_v1_boot_cleanup(&profile);
        return 0;
    }
    csb_v1_boot_startup_runtime_asset_session_init_pc34(&session);
    CHECK(csb_v1_boot_startup_runtime_asset_session_open_pc34(&profile, &session) &&
              session.valid && session.rejects_legacy_wrappers,
          "verified PC34 data opens one wrapper-free startup session");
    if (!session.valid) {
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    CHECK(csb_v1_boot_startup_playback_begin_pc34(&session, &audio) &&
              csb_v1_boot_startup_playback_complete_swoosh_pc34(&session, &audio),
          "title playback starts in the owned session");
    for (title_frame = 0;
         title_frame < csb_v1_startup_title_total_ticks_pc34();
         ++title_frame) {
        memset(&plan, 0, sizeof(plan));
        memset(&frame, 0, sizeof(frame));
        if (!csb_v1_boot_startup_playback_title_frame_pc34(
                &session, title_frame, &plan, &audio) ||
            !csb_v1_boot_startup_runtime_asset_session_frame_pc34(
                &session, &plan, ++tick, &frame) ||
            !frame.valid || frame.special_palette !=
                title_palette_for_stage(plan.title_stage) ||
            frame.source_tick != tick || frame.session_generation != session.generation) {
            break;
        }
    }
    CHECK(title_frame == csb_v1_startup_title_total_ticks_pc34() &&
              csb_v1_boot_startup_playback_title_frame_pc34(
                  &session, title_frame, &plan, &audio) &&
              session.playback.title_phase_mask == 0x0b,
          "PRESENTS CHAOS STRIKES cadence and palettes complete before entrance");

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.special_palette = VGA_PALETTE_PC34_SPECIAL_ENTRANCE;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, ++tick, &frame) && frame.valid &&
              frame.special_palette == VGA_PALETTE_PC34_SPECIAL_ENTRANCE,
          "C004 enters with the original entrance palette");
    for (step = 1; step <= 31; ++step) {
        opening_plan(step, &plan);
        if (!csb_v1_boot_startup_runtime_asset_session_frame_pc34(
                &session, &plan, ++tick, &frame) || !frame.valid ||
            frame.opening_step != step ||
            frame.special_palette != VGA_PALETTE_PC34_SPECIAL_ENTRANCE) break;
        if (step == 1) {
            memset(&raster, 0, sizeof(raster));
            first_door_rasterized =
                csb_v1_boot_startup_runtime_frame_rasterize_pc34(
                    &frame, &plan, &raster) && raster.valid &&
                raster.door_composited && raster.source_surface_count >= 2;
            csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
        }
    }
    CHECK(first_door_rasterized && step == 32 &&
              session.playback.last_door_opening_step == 31 &&
              csb_v1_boot_startup_playback_complete_entrance_pc34(&session) &&
              csb_v1_boot_startup_playback_enter_hud_pc34(&session),
          "first and terminal F0807 door frames consume original C002/C003 material");

    memset(&plan, 0, sizeof(plan));
    plan.surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    plan.special_palette = -1;
    plan.title_stage = CSB_V1_STARTUP_STAGE_DUNGEON_RUNTIME_PC34;
    CHECK(csb_v1_boot_startup_runtime_asset_session_frame_pc34(
              &session, &plan, ++tick, &frame) && frame.valid &&
              frame.special_palette == -1 &&
              csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
                  &session, &terminal) && terminal.valid &&
              terminal.source_tick == tick &&
              terminal.session_generation == frame.session_generation,
          "fresh terminal receipt binds neutral C017/C040 to the same tick and generation");
    memset(&raster, 0, sizeof(raster));
    CHECK(csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
              &frame, 1, &raster) && raster.valid &&
              raster.source_surface_count == 2,
          "fresh receipt reaches original C017 plus C040 HUD raster");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    memset(&terminal_ui, 0, sizeof(terminal_ui));
    terminal_ui.valid = 1;
    terminal_ui.c040_cleared_once = 1;
    terminal_ui.post_c101_presented = 1;
    terminal_ui.live_c017_only_panel_base = 1;
    terminal_ui.palette_neutral = 1;
    terminal_ui.no_legacy_wrapper = 1;
    terminal_ui.no_cast_or_combat = 1;
    terminal_ui.c017_source_asset_id = 17;
    terminal_ui.c017_width = 224;
    terminal_ui.c017_height = 136;
    terminal_ui.c017_special_palette = -1;
    terminal_ui.source_tick = terminal.source_tick;
    terminal_ui.session_generation = terminal.session_generation;
    memset(&raster, 0, sizeof(raster));
    CHECK(csb_v1_boot_terminal_ui_state_host_raster_pc34(
              &terminal_ui, &session, &frame, 0, &raster) && raster.valid &&
              raster.source_surface_count == 1,
          "terminal receipt consumes a neutral first live C017 HUD raster");
    csb_v1_boot_startup_runtime_raster_release_pc34(&raster);
    frame.source_tick--;
    CHECK(!csb_v1_boot_terminal_ui_state_host_raster_pc34(
               &terminal_ui, &session, &frame, 0, &raster),
          "stale terminal tick fails closed before live HUD raster");
    frame.source_tick++;
    frame.session_generation--;
    CHECK(!csb_v1_boot_terminal_ui_state_host_raster_pc34(
               &terminal_ui, &session, &frame, 0, &raster),
          "stale terminal generation fails closed before live HUD raster");
    frame.session_generation++;
    session.generation++;
    CHECK(!csb_v1_boot_terminal_ui_state_host_raster_pc34(
               &terminal_ui, &session, &frame, 0, &raster),
          "stale startup session fails closed before live HUD raster");
    session.generation--;
    session.playback.last_door_opening_step = 30;
    CHECK(!csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
               &session, &terminal) && !terminal.valid,
          "stale terminal door receipt fails closed before HUD acceptance");

    CHECK(csb_v1_boot_enter_game(&profile) == 0,
          "verified terminal startup route enters the CSB runtime");
    play_ms_before = profile.runtime.total_play_ms;
    csb_v1_runtime_tick(&profile.runtime, CSB_V1_TICK_MS_NOMINAL);
    CHECK(profile.runtime.total_play_ms > play_ms_before,
          "first live dungeon tick follows the terminal HUD receipt");

    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    csb_v1_boot_cleanup(&profile);
    printf("checks=%d failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
