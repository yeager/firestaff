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

static DM1_V1_StartupHoCBootProbeSummary_PC34 save_ready_summary(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 summary;

    memset(&summary, 0, sizeof(summary));
    summary.handled = 1;
    summary.complete_original_save_roundtrip_route = 1;
    summary.user_save_corpus_pc34_ready = 1;
    summary.user_save_corpus_part_envelope_ready = 1;
    summary.user_save_corpus_roundtrip_ready = 1;
    summary.user_save_corpus_roundtrip_verified = 3;
    summary.user_save_corpus_roundtrip_hash = 0x5a1e2026u;
    snprintf(summary.user_save_corpus_first_pc34_path,
             sizeof(summary.user_save_corpus_first_pc34_path),
             "saves/pc34/DMSAVE.DAT");
    summary.source_evidence =
        "ReDMCSB REVIVE.C F0280; LOADSAVE.C F0433/F0435";
    return summary;
}

int main(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 ready = save_ready_summary();
    DM1_V1_StartupHoCBootProbeSummary_PC34 failed = save_ready_summary();
    DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 receipt;
    int ok = 1;

    memset(&receipt, 0, sizeof(receipt));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
            &ready,
            DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_ORIGINAL_SAVE_CORPUS_PC34,
            &receipt),
        "original save corpus expectation receipt");
    ok &= expect_true(receipt.ready, "ready original save corpus expectation");
    ok &= expect_true(receipt.original_save_corpus_ready,
                      "ready original save corpus field");
    ok &= expect_contains(receipt.diagnostic,
                          "originalSave=1",
                          "original-save route diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "pc34=1",
                          "pc34 diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "partEnvelope=1",
                          "part envelope diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "roundtrip=1",
                          "roundtrip diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "verified=3",
                          "verified diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "failed=0",
                          "failed diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "hash=5a1e2026",
                          "roundtrip hash diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "firstPath=saves/pc34/DMSAVE.DAT",
                          "first path diagnostic");

    failed.user_save_corpus_roundtrip_ready = 0;
    failed.user_save_corpus_roundtrip_failed = 1;
    failed.user_save_corpus_roundtrip_hash = 0x00bad5afu;
    failed.user_save_corpus_rejected = 2;
    failed.user_save_corpus_truncated = 1;

    memset(&receipt, 0, sizeof(receipt));
    ok &= expect_true(
        dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
            &failed,
            DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_ORIGINAL_SAVE_CORPUS_PC34,
            &receipt),
        "failed original save corpus expectation receipt");
    ok &= expect_false(receipt.ready, "failed original save corpus expectation");
    ok &= expect_false(receipt.original_save_corpus_ready,
                       "failed original save corpus field");
    ok &= expect_contains(receipt.diagnostic,
                          "roundtrip=0",
                          "failed roundtrip diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "failed=1",
                          "failed count diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "hash=00bad5af",
                          "failed hash diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "rejected=2",
                          "rejected diagnostic");
    ok &= expect_contains(receipt.diagnostic,
                          "truncated=1",
                          "truncated diagnostic");

    return ok ? 0 : 1;
}
