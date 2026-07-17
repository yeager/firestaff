#include "csb_v1_startup_runtime_coupling_adapter_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

static void make_complete(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupCompleteSupportReceipt_PC34 *support)
{
    unsigned int i;
    memset(session, 0, sizeof(*session));
    memset(support, 0, sizeof(*support));
    session->valid = 1;
    session->real_asset_matched = 1;
    session->title_assets_ready = 1;
    session->title_presents_ready = 1;
    session->title_chaos_ready = 1;
    session->title_strikes_back_ready = 1;
    session->entrance_assets_ready = 1;
    session->door_assets_ready = 1;
    session->hud_assets_bound = 1;
    session->full_startup_ready = 1;
    session->rejects_legacy_wrappers = 1;
    session->generation = 17u;
    session->playback.no_fallback_routes = 1;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34;
    session->playback.entrance_complete = 1;
    session->playback.last_door_opening_step = 31;
    session->playback.next_door_opening_step = 32;

    support->valid = 1;
    support->full_runtime_valid = 1;
    support->host_capture_gate_valid = 1;
    support->real_asset_matched = 1;
    support->title_sequence_ready = 1;
    support->title_phase_route_complete = 1;
    support->title_presents_ready = 1;
    support->title_chaos_ready = 1;
    support->title_strikes_back_ready = 1;
    support->entrance_ready = 1;
    support->hud_ready = 1;
    support->door_ready = 1;
    support->draw_consumes_receipt_only = 1;
    support->input_consumes_receipt_only = 1;
    support->no_legacy_wrappers = 1;
    support->no_fallback_callbacks = 1;
    support->no_wrapper_fallback_routes = 1;
    support->real_startup_assets_bound = 1;
    support->session_generation = session->generation;
    support->title_runtime_phase_mask =
        CSB_V1_STARTUP_RUNTIME_TITLE_ALL_PHASES_PC34;
    support->title_runtime_expected_phase_mask =
        CSB_V1_STARTUP_RUNTIME_TITLE_ALL_PHASES_PC34;
    support->title_runtime_phase_hash_count = 4;
    for (i = 0; i < 4; ++i) support->title_runtime_phase_hashes[i] = 100u + i;
}

static void make_title_host(CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host,
                            unsigned int phase, unsigned int tick,
                            const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
                            const CSB_V1_StartupCompleteSupportReceipt_PC34 *support)
{
    memset(host, 0, sizeof(*host));
    host->valid = host->real_asset_matched = host->no_legacy_wrappers = 1;
    host->no_synthetic_surface = 1;
    host->host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34;
    host->frame.session_generation = session->generation;
    host->frame.source_tick = tick;
    host->raster.valid = host->raster.title_composited = 1;
    host->raster.source_surface_count = 1;
    host->raster.pixel_hash = support->title_runtime_phase_hashes[phase];
    if (phase == 0u) {
        host->frame.stage = CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34;
        host->special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS;
    } else if (phase < 3u) {
        host->frame.stage = CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34;
        host->frame.title_phase_mask = phase == 2u ? 0x04 : 0x02;
        host->special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
    } else {
        host->frame.stage = CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34;
        host->special_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES;
    }
}

int main(void)
{
    unsigned int i;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupCompleteSupportReceipt_PC34 support;
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts;
    CSB_V1_StartupRuntimeCouplingSessionReceipt_PC34 receipt;
    CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 presents, chaos_zoom;
    CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 chaos_hold, strikes;
    CSB_V1_StartupTitleTerminalLifecycleReceipt_PC34 terminal;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 host;
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 release_capture;
    CSB_V1_StartupReleaseLifecycleReceipt_PC34 release_first, release_next;

    make_complete(&session, &support);
    CHECK(csb_v1_startup_runtime_coupling_facts_from_session_pc34(
        &session, &support, &facts) == 1);
    CHECK(facts.title_runtime_phase_hash_count == 4);
    CHECK(facts.door_step_index == 30);
    CHECK(csb_v1_startup_runtime_coupling_consume_session_pc34(
        &session, &support, &receipt) == 1);
    CHECK(receipt.valid && receipt.title_consumed && receipt.entrance_consumed &&
        receipt.door_step_consumed && receipt.door_blit_consumed);

    CHECK(csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        &session, &support, CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34,
        10u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS, NULL, &presents) == 1);
    CHECK(csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        &session, &support, CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_ZOOM_PC34,
        11u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS, &presents, &chaos_zoom) == 1);
    CHECK(csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        &session, &support, CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_HOLD_PC34,
        12u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS, &chaos_zoom, &chaos_hold) == 1);
    CHECK(csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        &session, &support, CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34,
        13u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES, &chaos_hold, &strikes) == 1 && strikes.valid &&
        strikes.capture_identity == support.title_runtime_phase_hashes[3]);
    CHECK(csb_v1_startup_title_terminal_lifecycle_receipt_pc34(
        &session, &support, &strikes, 14u, &terminal) == 1 && terminal.valid &&
        terminal.terminal_session_bound);
    CHECK(csb_v1_startup_title_terminal_lifecycle_receipt_pc34(
        &session, &support, &strikes, 13u, &terminal) == 0 && !terminal.valid);
    CHECK(csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        &session, &support, CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34,
        13u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES, &chaos_zoom, &strikes) == 0 && !strikes.valid);
    CHECK(csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        &session, &support, CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34,
        10u, VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS, NULL, &presents) == 0 &&
        !presents.valid);
    make_title_host(&host, 0u, 20u, &session, &support);
    CHECK(csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
        &session, &support, &host, NULL, &presents) == 1);
    make_title_host(&host, 1u, 21u, &session, &support);
    CHECK(csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
        &session, &support, &host, &presents, &chaos_zoom) == 1);
    make_title_host(&host, 2u, 22u, &session, &support);
    CHECK(csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
        &session, &support, &host, &chaos_zoom, &chaos_hold) == 1);
    make_title_host(&host, 3u, 23u, &session, &support);
    CHECK(csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
        &session, &support, &host, &chaos_hold, &strikes) == 1);
    host.raster.pixel_hash++;
    CHECK(csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
        &session, &support, &host, &chaos_hold, &strikes) == 0 && !strikes.valid);
    memset(&release_capture, 0, sizeof(release_capture));
    release_capture.valid = release_capture.complete_support_valid = 1;
    release_capture.title_phase_route_complete = release_capture.runtime_host_routes_ready = 1;
    release_capture.no_fallback_callbacks = release_capture.no_wrapper_fallback_routes = 1;
    release_capture.complete_support = support;
    release_capture.complete_support_hash = 11u;
    release_capture.title_runtime_phase_hash = 12u;
    release_capture.runtime_host_gate_hash = 13u;
    release_capture.release_app_capture_hash = 14u;
    release_capture.title_runtime_phase_hash_count = 4;
    for (i = 0; i < 4; ++i) release_capture.title_runtime_phase_hashes[i] = 20u + i;
    CHECK(csb_v1_startup_release_lifecycle_advance_pc34(
        &release_capture, 30u, NULL, &release_first) == 1 && release_first.valid);
    CHECK(csb_v1_startup_release_lifecycle_advance_pc34(
        &release_capture, 31u, &release_first, &release_next) == 1 && release_next.valid);
    release_capture.title_runtime_phase_hashes[3]++;
    CHECK(csb_v1_startup_release_lifecycle_advance_pc34(
        &release_capture, 32u, &release_next, &release_first) == 0 && !release_first.valid);

    support.title_runtime_phase_hashes[3] = support.title_runtime_phase_hashes[2];
    CHECK(csb_v1_startup_runtime_coupling_facts_from_session_pc34(
        &session, &support, &facts) == 0);
    make_complete(&session, &support);
    support.session_generation++;
    CHECK(csb_v1_startup_runtime_coupling_consume_session_pc34(
        &session, &support, &receipt) == 0);
    return 0;
}
