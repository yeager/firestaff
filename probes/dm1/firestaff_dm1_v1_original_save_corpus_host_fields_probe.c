#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(int actual, int expected, const char* label) {
    if (actual != expected) {
        fprintf(stderr, "%s: got %d expected %d\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_uint(unsigned int actual,
                       unsigned int expected,
                       const char* label) {
    if (actual != expected) {
        fprintf(stderr,
                "%s: got %08x expected %08x\n",
                label,
                actual,
                expected);
        return 0;
    }
    return 1;
}

static int expect_string(const char* actual,
                         const char* expected,
                         const char* label) {
    if (!actual || strcmp(actual, expected) != 0) {
        fprintf(stderr,
                "%s: got %s expected %s\n",
                label,
                actual ? actual : "(null)",
                expected ? expected : "(null)");
        return 0;
    }
    return 1;
}

int main(void) {
    DM1_V1_StartupHoCBootProbeSummary_PC34 summary;
    DM1_V1_StartupHoCBootProbeHostFields_PC34 fields;
    int ok = 1;

    memset(&summary, 0, sizeof(summary));
    summary.handled = 1;
    summary.complete_support_ready = 1;
    summary.complete_save_corpus_route = 1;
    summary.complete_original_save_roundtrip_route = 1;
    summary.user_save_corpus_pc34_ready = 1;
    summary.user_save_corpus_part_envelope_ready = 1;
    summary.user_save_corpus_roundtrip_ready = 1;
    summary.user_save_corpus_roundtrip_verified = 4;
    summary.user_save_corpus_roundtrip_failed = 0;
    summary.user_save_corpus_roundtrip_hash = 0x7eed5a1eu;
    summary.user_save_corpus_rejected = 2;
    summary.user_save_corpus_truncated = 1;
    snprintf(summary.user_save_corpus_first_pc34_path,
             sizeof(summary.user_save_corpus_first_pc34_path),
             "Users/test/Firestaff/DMSAVE.DAT");

    memset(&fields, 0, sizeof(fields));
    ok &= expect_int(
        dm1_v1_startup_hoc_boot_probe_host_fields_pc34(&summary, &fields),
        1,
        "host fields receipt");
    ok &= expect_int(fields.handled, 1, "handled");
    ok &= expect_int(fields.complete_save_corpus_route,
                     1,
                     "complete save corpus route");
    ok &= expect_int(fields.complete_original_save_roundtrip_route,
                     1,
                     "complete original-save route");
    ok &= expect_int(fields.user_save_corpus_pc34_ready,
                     1,
                     "PC34 ready");
    ok &= expect_int(fields.user_save_corpus_part_envelope_ready,
                     1,
                     "part envelope ready");
    ok &= expect_int(fields.user_save_corpus_roundtrip_ready,
                     1,
                     "roundtrip ready");
    ok &= expect_int(fields.user_save_corpus_roundtrip_verified,
                     4,
                     "roundtrip verified");
    ok &= expect_int(fields.user_save_corpus_roundtrip_failed,
                     0,
                     "roundtrip failed");
    ok &= expect_uint(fields.user_save_corpus_roundtrip_hash,
                      0x7eed5a1eu,
                      "roundtrip hash");
    ok &= expect_int(fields.user_save_corpus_rejected, 2, "rejected");
    ok &= expect_int(fields.user_save_corpus_truncated, 1, "truncated");
    ok &= expect_string(fields.user_save_corpus_first_pc34_path,
                        "Users/test/Firestaff/DMSAVE.DAT",
                        "first PC34 path");

    summary.user_save_corpus_roundtrip_ready = 0;
    summary.user_save_corpus_roundtrip_failed = 1;
    summary.user_save_corpus_roundtrip_hash = 0x00c0ffeeu;

    memset(&fields, 0, sizeof(fields));
    ok &= expect_int(
        dm1_v1_startup_hoc_boot_probe_host_fields_pc34(&summary, &fields),
        1,
        "failed host fields receipt");
    ok &= expect_int(fields.user_save_corpus_roundtrip_ready,
                     0,
                     "failed roundtrip ready");
    ok &= expect_int(fields.user_save_corpus_roundtrip_failed,
                     1,
                     "failed roundtrip count");
    ok &= expect_uint(fields.user_save_corpus_roundtrip_hash,
                      0x00c0ffeeu,
                      "failed roundtrip hash");

    return ok ? 0 : 1;
}
