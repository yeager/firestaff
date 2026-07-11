#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_contains(const char* haystack,
                           const char* needle,
                           const char* label) {
    if (!haystack || !needle || !strstr(haystack, needle)) {
        fprintf(stderr, "missing %s: %s\n", label, needle ? needle : "(null)");
        return 0;
    }
    return 1;
}

static int expect_true(int value, const char* label) {
    if (!value) {
        fprintf(stderr, "expected true: %s\n", label);
        return 0;
    }
    return 1;
}

static DM1_V1_StartupHoCBootProbeSummary_PC34 base_summary(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 summary;

    memset(&summary, 0, sizeof(summary));
    summary.handled = 1;
    summary.complete_support_ready = 1;
    summary.complete_source_visible_startup = 1;
    summary.complete_entrance_to_hoc = 1;
    summary.complete_hoc_render_route = 1;
    summary.complete_host_app_capture_route = 1;
    summary.complete_save_corpus_route = 1;
    summary.complete_original_save_roundtrip_route = 1;
    summary.consumed_hoc_save_capture_host_readiness = 1;
    summary.hoc_save_capture_ready = 1;
    summary.hoc_original_save_capture_ready = 1;
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
    summary.release_app_identity_hash = 0x51a7e5u;
    summary.host_capture_route_matches = 1;
    summary.release_capture_ownership_ready = 1;
    summary.host_render_consumer_ready = 1;
    summary.m11_boot_probe_consumer_ready = 1;
    summary.launch_path_ready = 1;
    summary.required_asset_capture = 1;
    summary.receipt_only_consumer_ready = 1;
    summary.host_window_capture = 1;
    summary.presented_capture = 1;
    summary.presented_capture_width = 320;
    summary.presented_capture_height = 200;
    summary.presented_capture_geometry = 1;
    summary.presented_capture_pixels = 1;
    summary.presented_capture_bytes = 320 * 200 * 4;
    summary.presented_capture_hash = 0x12345678u;
    summary.presented_capture_chain_ready = 1;
    summary.presented_capture_consumer_mask = 0x3u;
    summary.presented_capture_chain_hash = 0x87654321u;
    summary.map_width = 32;
    summary.map_height = 32;
    summary.render_command_count = 4;
    summary.user_save_corpus_pc34_ready = 1;
    summary.user_save_corpus_part_envelope_ready = 1;
    summary.user_save_corpus_roundtrip_ready = 1;
    summary.user_save_corpus_roundtrip_verified = 2;
    summary.user_save_corpus_roundtrip_hash = 0x0badc0deu;
    snprintf(summary.user_save_corpus_first_pc34_path,
             sizeof(summary.user_save_corpus_first_pc34_path),
             "fixtures/dm1/DMSAVE.DAT");
    summary.source_evidence =
        "ReDMCSB ENTRANCE.C F0797/F0441; LOADSAVE.C F0433/F0435";
    return summary;
}

int main(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 ok_summary = base_summary();
    DM1_V1_StartupHoCBootProbeSummary_PC34 failed_summary = base_summary();
    DM1_V1_StartupHoCBootProbeLogReceipt_PC34 receipt;
    int ok = 1;

    memset(&receipt, 0, sizeof(receipt));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(&ok_summary,
                                                       &receipt),
        "verified corpus log receipt");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusPC34=1",
                          "PC34 corpus field");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusPartEnvelope=1",
                          "part envelope field");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripReady=1",
                          "roundtrip ready field");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripVerified=2",
                          "roundtrip verified count");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripFailed=0",
                          "roundtrip failed count");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripHash=0badc0de",
                          "roundtrip hash");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusFirstPC34Path=fixtures/dm1/"
                          "DMSAVE.DAT",
                          "first PC34 path");

    failed_summary.complete_support_ready = 0;
    failed_summary.complete_original_save_roundtrip_route = 0;
    failed_summary.user_save_corpus_roundtrip_ready = 0;
    failed_summary.user_save_corpus_roundtrip_verified = 1;
    failed_summary.user_save_corpus_roundtrip_failed = 1;
    failed_summary.user_save_corpus_roundtrip_hash = 0x00f00badu;

    memset(&receipt, 0, sizeof(receipt));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(&failed_summary,
                                                       &receipt),
        "failed corpus log receipt");
    ok &= expect_contains(receipt.fields,
                          "dm1CompleteSupportReady=0",
                          "failed complete support");
    ok &= expect_contains(receipt.fields,
                          "dm1CompleteOriginalSaveRoundtripRoute=0",
                          "failed original-save route");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripReady=0",
                          "failed roundtrip ready");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripFailed=1",
                          "failed roundtrip count");
    ok &= expect_contains(receipt.fields,
                          "dm1UserSaveCorpusRoundtripHash=00f00bad",
                          "failed roundtrip hash");

    return ok ? 0 : 1;
}
