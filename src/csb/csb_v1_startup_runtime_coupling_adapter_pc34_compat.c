#include "csb_v1_startup_runtime_coupling_adapter_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <string.h>

static int csb_v1_startup_title_hashes_unique_pc34(
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *receipt)
{
    unsigned int i;
    unsigned int j;

    if (!receipt || receipt->title_runtime_phase_hash_count !=
            CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34) return 0;
    for (i = 0; i < CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34; ++i) {
        if (receipt->title_runtime_phase_hashes[i] == 0u) return 0;
        for (j = 0; j < i; ++j) {
            if (receipt->title_runtime_phase_hashes[i] ==
                receipt->title_runtime_phase_hashes[j]) return 0;
        }
    }
    return 1;
}

static int csb_v1_startup_session_is_terminal_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session)
{
    return session && session->valid && session->real_asset_matched &&
        session->full_startup_ready && session->title_assets_ready &&
        session->title_presents_ready && session->title_chaos_ready &&
        session->title_strikes_back_ready && session->entrance_assets_ready &&
        session->door_assets_ready && session->hud_assets_bound &&
        session->rejects_legacy_wrappers && session->playback.no_fallback_routes &&
        session->playback.stage == CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34 &&
        session->playback.entrance_complete &&
        session->playback.last_door_opening_step ==
            CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 &&
        session->playback.next_door_opening_step ==
            CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 + 1 &&
        session->generation != 0u;
}

static uint32_t csb_v1_startup_title_phase_palette_pc34(
    CSB_V1_StartupTitleRuntimePhase_PC34 phase)
{
    if (phase == CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34)
        return VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS;
    if (phase == CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_ZOOM_PC34 ||
        phase == CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_HOLD_PC34)
        return VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
    if (phase == CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34)
        return VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES;
    return 0u;
}

int csb_v1_startup_runtime_coupling_facts_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupRuntimeCouplingFacts_PC34 *out_facts)
{
    int title_ready;

    if (out_facts) memset(out_facts, 0, sizeof(*out_facts));
    if (!out_facts || !csb_v1_startup_session_is_terminal_pc34(session) ||
        !complete_support || !complete_support->valid ||
        !complete_support->full_runtime_valid ||
        !complete_support->host_capture_gate_valid ||
        !complete_support->real_asset_matched ||
        !complete_support->title_sequence_ready ||
        !complete_support->title_phase_route_complete ||
        !complete_support->entrance_ready || !complete_support->hud_ready ||
        !complete_support->door_ready ||
        !complete_support->draw_consumes_receipt_only ||
        !complete_support->input_consumes_receipt_only ||
        !complete_support->no_legacy_wrappers ||
        !complete_support->no_fallback_callbacks ||
        !complete_support->no_wrapper_fallback_routes ||
        !complete_support->real_startup_assets_bound ||
        complete_support->session_generation != session->generation ||
        complete_support->title_runtime_phase_mask !=
            complete_support->title_runtime_expected_phase_mask ||
        !csb_v1_startup_title_hashes_unique_pc34(complete_support)) return 0;

    title_ready = complete_support->title_presents_ready &&
        complete_support->title_chaos_ready &&
        complete_support->title_strikes_back_ready;
    if (!title_ready) return 0;

    out_facts->valid = 1;
    out_facts->real_startup_assets_bound = 1;
    out_facts->title_presents_runtime_captured = 1;
    out_facts->title_chaos_zoom_runtime_captured = 1;
    out_facts->title_chaos_hold_runtime_captured = 1;
    out_facts->title_strikes_back_runtime_captured = 1;
    out_facts->title_runtime_phase_mask =
        CSB_V1_STARTUP_RUNTIME_TITLE_ALL_PHASES_PC34;
    out_facts->title_runtime_phase_hash_count =
        CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34;
    out_facts->title_runtime_unique_sample_hash_count =
        CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34;
    out_facts->closed_door_hud_runtime_captured = 1;
    out_facts->utility_hud_runtime_captured = 1;
    out_facts->door_opening_delay_runtime_captured = 1;
    out_facts->door_opening_frame_runtime_captured = 1;
    out_facts->source_door_step_count =
        CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34;
    out_facts->door_step_index =
        CSB_V1_STARTUP_RUNTIME_DOOR_STEP_COUNT_PC34 - 1;
    out_facts->draw_consumes_receipt_only = 1;
    out_facts->input_consumes_receipt_only = 1;
    out_facts->no_fallback_callbacks = 1;
    out_facts->no_wrapper_fallback_routes = 1;
    out_facts->no_legacy_door_fallback_route = 1;
    out_facts->no_synthetic_visuals = 1;
    return 1;
}

int csb_v1_startup_runtime_coupling_consume_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupRuntimeCouplingSessionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !csb_v1_startup_runtime_coupling_facts_from_session_pc34(
            session, complete_support, &facts)) return 0;

    out_receipt->facts_from_verified_session = 1;
    out_receipt->title_consumed = F0437_STARTEND_DrawTitle(
        &facts, &out_receipt->title);
    out_receipt->entrance_consumed = F0438_STARTEND_OpenEntranceDoors(
        &facts, &out_receipt->entrance);
    out_receipt->door_step_consumed = F0580_ENTRANCE_DrawDoorAnimationStep(
        &facts, &out_receipt->door_step);
    out_receipt->door_blit_consumed = F0581_ENTRANCE_BlitDoors(
        &facts, &out_receipt->door_blit);
    out_receipt->valid = out_receipt->title_consumed &&
        out_receipt->entrance_consumed && out_receipt->door_step_consumed &&
        out_receipt->door_blit_consumed;
    out_receipt->source_evidence =
        csb_v1_startup_runtime_coupling_adapter_source_evidence_pc34();
    return out_receipt->valid;
}

int csb_v1_startup_title_runtime_lifecycle_advance_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupTitleRuntimePhase_PC34 phase, unsigned int source_tick,
    uint32_t palette_identity,
    const CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *previous,
    CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts;
    uint32_t capture_identity;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || phase < CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34 ||
        phase > CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34 ||
        source_tick == 0u ||
        !csb_v1_startup_runtime_coupling_facts_from_session_pc34(
            session, complete_support, &facts)) return 0;
    capture_identity = complete_support->title_runtime_phase_hashes[phase];
    if (capture_identity == 0u ||
        palette_identity != csb_v1_startup_title_phase_palette_pc34(phase) ||
        (previous &&
        (!previous->valid || previous->session_generation != session->generation ||
         previous->phase + 1 != phase || previous->source_tick >= source_tick))) return 0;
    if (!previous && phase != CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34) return 0;
    out_receipt->valid = 1;
    out_receipt->real_asset_matched = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->phase = phase;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session->generation;
    out_receipt->palette_identity = palette_identity;
    out_receipt->capture_identity = capture_identity;
    return 1;
}

int csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_receipt,
    const CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *previous,
    CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupTitleRuntimePhase_PC34 phase;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!host_receipt || !host_receipt->valid ||
        !host_receipt->real_asset_matched || !host_receipt->no_legacy_wrappers ||
        !host_receipt->no_synthetic_surface ||
        host_receipt->host_surface != CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34 ||
        !host_receipt->raster.valid || !host_receipt->raster.title_composited ||
        host_receipt->raster.source_surface_count != 1 ||
        host_receipt->frame.session_generation != (session ? session->generation : 0u)) return 0;
    if (host_receipt->frame.stage == CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34)
        phase = CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34;
    else if (host_receipt->frame.stage == CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34)
        phase = host_receipt->frame.title_phase_mask == 0x04
            ? CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_HOLD_PC34
            : CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_ZOOM_PC34;
    else if (host_receipt->frame.stage == CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34)
        phase = CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34;
    else return 0;
    if (!complete_support || host_receipt->raster.pixel_hash == 0u ||
        host_receipt->raster.pixel_hash != complete_support->title_runtime_phase_hashes[phase]) return 0;
    return csb_v1_startup_title_runtime_lifecycle_advance_pc34(
        session, complete_support, phase, host_receipt->frame.source_tick,
        (uint32_t)host_receipt->special_palette, previous, out_receipt);
}

int csb_v1_startup_title_terminal_lifecycle_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    const CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *title_receipt,
    unsigned int terminal_source_tick,
    CSB_V1_StartupTitleTerminalLifecycleReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRuntimeCouplingFacts_PC34 facts;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !title_receipt || !title_receipt->valid ||
        title_receipt->phase != CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34 ||
        terminal_source_tick <= title_receipt->source_tick ||
        !csb_v1_startup_runtime_coupling_facts_from_session_pc34(
            session, complete_support, &facts) ||
        title_receipt->session_generation != session->generation ||
        title_receipt->capture_identity !=
            complete_support->title_runtime_phase_hashes[
                CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34] ||
        title_receipt->palette_identity !=
            VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_STRIKES) return 0;
    out_receipt->valid = 1;
    out_receipt->title_lifecycle_bound = 1;
    out_receipt->terminal_session_bound = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->title_source_tick = title_receipt->source_tick;
    out_receipt->terminal_source_tick = terminal_source_tick;
    out_receipt->session_generation = session->generation;
    out_receipt->strikes_palette_identity = title_receipt->palette_identity;
    out_receipt->strikes_capture_identity = title_receipt->capture_identity;
    return 1;
}

int csb_v1_startup_release_lifecycle_advance_pc34(
    const CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *release_capture,
    uint32_t capture_tick,
    const CSB_V1_StartupReleaseLifecycleReceipt_PC34 *previous,
    CSB_V1_StartupReleaseLifecycleReceipt_PC34 *out_receipt)
{
    unsigned int i;
    uint32_t title_phase_set_hash = 2166136261u;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !release_capture || !release_capture->valid ||
        !release_capture->complete_support_valid ||
        !release_capture->title_phase_route_complete ||
        !release_capture->runtime_host_routes_ready ||
        !release_capture->no_fallback_callbacks ||
        !release_capture->no_wrapper_fallback_routes || capture_tick == 0u ||
        release_capture->complete_support_hash == 0u ||
        release_capture->runtime_host_gate_hash == 0u ||
        release_capture->release_app_capture_hash == 0u ||
        release_capture->title_runtime_phase_hash == 0u ||
        release_capture->title_runtime_phase_hash_count !=
            CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34) return 0;
    for (i = 0u; i < CSB_V1_STARTUP_RUNTIME_TITLE_SAMPLE_COUNT_PC34; ++i) {
        if (release_capture->title_runtime_phase_hashes[i] == 0u) return 0;
        title_phase_set_hash = (title_phase_set_hash ^
            release_capture->title_runtime_phase_hashes[i]) * 16777619u;
    }
    if (previous && (!previous->valid ||
        previous->session_generation != release_capture->complete_support.session_generation ||
        previous->complete_support_hash != release_capture->complete_support_hash ||
        previous->title_phase_hash != release_capture->title_runtime_phase_hash ||
        previous->title_phase_set_hash != title_phase_set_hash ||
        previous->runtime_route_hash != release_capture->runtime_host_gate_hash ||
        previous->release_capture_hash != release_capture->release_app_capture_hash ||
        previous->capture_tick >= capture_tick)) return 0;
    out_receipt->valid = 1;
    out_receipt->complete_support_bound = 1;
    out_receipt->title_phases_bound = 1;
    out_receipt->no_fallback_routes = 1;
    out_receipt->session_generation = release_capture->complete_support.session_generation;
    out_receipt->capture_tick = capture_tick;
    out_receipt->complete_support_hash = release_capture->complete_support_hash;
    out_receipt->title_phase_hash = release_capture->title_runtime_phase_hash;
    out_receipt->title_phase_set_hash = title_phase_set_hash;
    out_receipt->runtime_route_hash = release_capture->runtime_host_gate_hash;
    out_receipt->release_capture_hash = release_capture->release_app_capture_hash;
    return 1;
}

const char *csb_v1_startup_runtime_coupling_adapter_source_evidence_pc34(void)
{
    return "ReDMCSB TITLE.C F0437 and ENTRANCE.C F0438/F0580/F0581; "
           "the CSB PC34 adapter consumes only one generation-matched, "
           "real-asset startup session and complete-support capture receipt";
}
