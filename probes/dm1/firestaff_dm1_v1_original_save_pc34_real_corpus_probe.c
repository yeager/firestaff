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

static void print_receipt_coverage(const DM1OriginalSavePC34CorpusReceipt *receipt)
{
    if (!receipt) return;
    printf("DM1 original-save coverage path=%s c3_records=%u c3_bytes=%u "
           "c4_indices=%u c4_bytes=%u c13_events=%d c13_bytes=%u "
           "stage_events=%d stage_c13=%d active_groups=%d\n",
           receipt->path,
           receipt->source_c3_event_record_count,
           receipt->source_c3_event_byte_count,
           receipt->source_c4_timeline_index_count,
           receipt->source_c4_timeline_byte_count,
           receipt->source_c13_event_count,
           receipt->source_c13_event_byte_count,
           receipt->source_runtime_stage_event_count,
           receipt->source_runtime_stage_c13_event_count,
           receipt->source_runtime_stage_active_group_count);
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int result;
    int passed = 1;
    int i;

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
    for (i = 0; i < report.receipt_count; ++i) {
        print_receipt_coverage(&report.receipts[i]);
    }

    /* Tail-less original saves are valid media, but this legacy probe owns
     * only the self-contained F0435 route. The backed corpus test loads them
     * against the real DUNGEON.DAT and is the runtime proof for those bytes. */
    for (i = 0; i < report.receipt_count; ++i) {
        if (!report.receipts[i].source_runtime_stage_committed) {
            puts("SKIP DM1 original-save real corpus: candidate requires DUNGEON.DAT backing; run dm1_v1_original_save_pc34_backed_corpus_roundtrip");
            return 0;
        }
    }

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
