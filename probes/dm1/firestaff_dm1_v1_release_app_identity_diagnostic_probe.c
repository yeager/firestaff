#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_true(int value, const char* label) {
    if (!value) {
        fprintf(stderr, "expected true: %s\n", label);
        return 0;
    }
    return 1;
}

static int expect_false(int value, const char* label) {
    if (value) {
        fprintf(stderr, "expected false: %s\n", label);
        return 0;
    }
    return 1;
}

static int expect_contains(const char* haystack,
                           const char* needle,
                           const char* label) {
    if (!haystack || !needle || !strstr(haystack, needle)) {
        fprintf(stderr, "missing %s: %s\n", label, needle ? needle : "(null)");
        return 0;
    }
    return 1;
}

static DM1_V1_StartupHoCBootProbeSummary_PC34 ready_summary(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 summary;

    memset(&summary, 0, sizeof(summary));
    summary.handled = 1;
    summary.full_graphics_ready = 1;
    summary.host_render_plan_ready = 1;
    summary.capture_proof_passed = 1;
    summary.runtime_apply_ready = 1;
    summary.production_consumer_ready = 1;
    summary.no_host_fallback_visuals = 1;
    summary.real_asset_capture = 1;
    summary.mac_window_capture = 1;
    summary.release_app_capture = 1;
    summary.release_app_identity_ready = 1;
    summary.release_app_identity_hash = 0x5a17d331u;
    summary.host_capture_route_matches = 1;
    summary.release_capture_ownership_ready = 1;
    summary.host_render_consumer_ready = 1;
    summary.m11_boot_probe_consumer_ready = 1;
    summary.launch_path_ready = 1;
    summary.required_asset_capture = 1;
    summary.receipt_only_consumer_ready = 1;
    summary.lower_level_helpers_ready = 1;
    summary.host_draw_uses_owned_receipt = 1;
    summary.host_draw_consumes_backing_asset = 1;
    summary.host_draw_rejects_backing_fallback = 1;
    summary.hoc_asset_capture = 1;
    summary.host_window_capture = 1;
    summary.presented_capture = 1;
    summary.presented_capture_width = 320;
    summary.presented_capture_height = 200;
    summary.presented_capture_geometry = 1;
    summary.presented_capture_pixels = 1;
    summary.presented_capture_bytes = 320 * 200 * 4;
    summary.presented_capture_hash = 0x0d1007f1u;
    summary.presented_capture_chain_ready = 1;
    summary.presented_capture_consumer_mask = 0x3u;
    summary.presented_capture_chain_hash = 0x5011ceedu;
    summary.host_capture_route_packaged = 1;
    summary.host_capture_route_mask = 0x7u;
    summary.host_capture_route_hash = 0x1a11cafeu;
    summary.presented_capture_route_packaged = 1;
    summary.opened_entrance_frame = 1;
    summary.hall_mirror_overlay = 1;
    summary.blocked_enter_until_champion = 1;
    summary.map_width = 32;
    summary.map_height = 32;
    summary.render_command_count = 4;
    summary.consumed_hoc_save_capture_host_readiness = 1;
    summary.hoc_save_capture_ready = 1;
    summary.hoc_original_save_capture_ready = 1;
    summary.complete_support_ready = 1;
    summary.complete_source_visible_startup = 1;
    summary.complete_entrance_to_hoc = 1;
    summary.complete_hoc_render_route = 1;
    summary.complete_host_app_capture_route = 1;
    summary.complete_save_corpus_route = 1;
    summary.complete_original_save_roundtrip_route = 1;
    summary.user_save_corpus_pc34_ready = 1;
    summary.user_save_corpus_part_envelope_ready = 1;
    summary.user_save_corpus_roundtrip_ready = 1;
    summary.user_save_corpus_roundtrip_verified = 1;
    summary.user_save_corpus_roundtrip_hash = 0x51a7e5u;
    summary.source_evidence =
        "ReDMCSB TITLE.C F0437; ENTRANCE.C F0797/F0441";
    return summary;
}

int main(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 summary = ready_summary();
    DM1_V1_StartupHoCBootProbeSummary_PC34 missing_identity = ready_summary();
    DM1_V1_StartupHoCBootProbeSummary_PC34 zero_hash = ready_summary();
    DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 expectation;
    DM1_V1_StartupHoCBootProbeLogReceipt_PC34 log_receipt;
    int ok = 1;

    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_release_app_capture_ready_pc34(
            &summary),
        "release app capture with identity");

    missing_identity.release_app_identity_ready = 0;
    ok &= expect_false(
        dm1_v1_startup_hoc_boot_probe_release_app_capture_ready_pc34(
            &missing_identity),
        "release app capture without identity");

    zero_hash.release_app_identity_hash = 0u;
    ok &= expect_false(
        dm1_v1_startup_hoc_boot_probe_release_app_capture_ready_pc34(
            &zero_hash),
        "release app capture without identity hash");

    memset(&expectation, 0, sizeof(expectation));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
            &summary,
            DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_RELEASE_APP_CAPTURE_PC34,
            &expectation),
        "release app expectation receipt");
    ok &= expect_true(expectation.ready, "release app expectation ready");
    ok &= expect_contains(expectation.diagnostic,
                          "releaseIdentity=1",
                          "release identity diagnostic");
    ok &= expect_contains(expectation.diagnostic,
                          "releaseIdentityHash=5a17d331",
                          "release identity hash diagnostic");

    memset(&expectation, 0, sizeof(expectation));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
            &missing_identity,
            DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_RELEASE_APP_CAPTURE_PC34,
            &expectation),
        "missing identity expectation receipt");
    ok &= expect_false(expectation.ready, "missing identity expectation ready");
    ok &= expect_contains(expectation.diagnostic,
                          "releaseIdentity=0",
                          "missing release identity diagnostic");

    memset(&log_receipt, 0, sizeof(log_receipt));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(&summary,
                                                       &log_receipt),
        "boot probe log receipt");
    ok &= expect_contains(log_receipt.fields,
                          "dm1HoCReleaseAppIdentity=1",
                          "log release identity");
    ok &= expect_contains(log_receipt.fields,
                          "dm1HoCReleaseAppIdentityHash=5a17d331",
                          "log release identity hash");

    return ok ? 0 : 1;
}
