#include "dm1_v1_original_save_pc34_handoff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int result;

    if (!root || !root[0]) {
        puts("DM1_PC34_CORPUS_PREFLIGHT=SKIP_ENV_UNSET");
        return 0;
    }
    memset(&report, 0, sizeof(report));
    result = dm1_v1_original_save_pc34_roundtrip_corpus_root(root, &report);
    if (result != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK || !report.scan_succeeded) {
        printf("DM1_PC34_CORPUS_PREFLIGHT=FAIL_SCAN result=%s\n",
               dm1_v1_original_save_pc34_handoff_result_name(result));
        return 1;
    }
    if (report.scanned_file_count == 0) {
        puts("DM1_PC34_CORPUS_PREFLIGHT=FAIL_NO_CANDIDATES");
        return 1;
    }
    if (report.pc34_candidate_count == 0) {
        printf("DM1_PC34_CORPUS_PREFLIGHT=FAIL_REJECTED rejected=%d truncated=%d\n",
               report.rejected_count, report.truncated_count);
        return 1;
    }
    if (report.roundtrip_failed_count != 0 ||
        report.roundtrip_succeeded_count != report.pc34_candidate_count ||
        report.core_state_match_count != report.pc34_candidate_count) {
        printf("DM1_PC34_CORPUS_PREFLIGHT=FAIL_ROUNDTRIP candidates=%d passed=%d failed=%d first=%s\n",
               report.pc34_candidate_count, report.roundtrip_succeeded_count,
               report.roundtrip_failed_count,
               dm1_v1_original_save_pc34_handoff_result_name(
                   report.first_failure_result));
        return 1;
    }
    printf("DM1_PC34_CORPUS_PREFLIGHT=VERIFIED candidates=%d fingerprint=%08x rejected=%d truncated=%d\n",
           report.pc34_candidate_count, report.provenance_fingerprint,
           report.rejected_count, report.truncated_count);
    return 0;
}
