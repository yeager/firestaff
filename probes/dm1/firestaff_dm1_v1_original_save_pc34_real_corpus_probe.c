#include "dm1_v1_original_save_pc34_handoff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check_int(const char *label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s: got %d want %d\n", label, got, want);
        return 0;
    }
    return 1;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int result;
    int passed = 1;

    /* This is intentionally an opt-in real-media probe. It never builds a
     * fixture or supplies a fallback directory: a configured corpus must
     * itself contain external, F0435-qualified PC34 saves. */
    if (!root || !root[0]) {
        puts("SKIP DM1 original-save real corpus: "
             "FIRESTAFF_DM1_PC34_SAVE_CORPUS unset");
        return 0;
    }

    memset(&report, 0, sizeof(report));
    result = dm1_v1_original_save_pc34_roundtrip_corpus_root(root, &report);
    printf("DM1 original-save real corpus root=%s scanned=%d candidates=%d "
           "rejected=%d truncated=%d attempted=%d passed=%d core_matches=%d failed=%d "
           "firestaff_exports=%d nonoriginal=%d first_failure=%s\n",
           root, report.scanned_file_count, report.pc34_candidate_count,
           report.rejected_count, report.truncated_count,
           report.roundtrip_attempted_count, report.roundtrip_succeeded_count,
           report.core_state_match_count, report.roundtrip_failed_count,
           report.firestaff_manifest_rejected_count,
           report.nonoriginal_envelope_rejected_count,
           dm1_v1_original_save_pc34_handoff_result_name(
               report.first_failure_result));

    passed &= check_int("corpus scan result",
                        result, DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK);
    passed &= check_int("corpus scan receipt", report.scan_succeeded, 1);
    passed &= check_int("corpus has files", report.scanned_file_count > 0, 1);
    passed &= check_int("corpus has external PC34 candidate",
                        report.pc34_candidate_count > 0, 1);
    passed &= check_int("every candidate was attempted",
                        report.roundtrip_attempted_count,
                        report.pc34_candidate_count);
    passed &= check_int("every candidate round-tripped",
                        report.roundtrip_succeeded_count,
                        report.pc34_candidate_count);
    passed &= check_int("every candidate preserved core state",
                        report.core_state_match_count,
                        report.pc34_candidate_count);
    passed &= check_int("no candidate round-trip failure",
                        report.roundtrip_failed_count, 0);
    passed &= check_int("no Firestaff export is certified as corpus media",
                        report.firestaff_manifest_rejected_count, 0);
    passed &= check_int("no malformed PC34 envelope is certified",
                        report.nonoriginal_envelope_rejected_count, 0);
    return passed ? 0 : 1;
}
