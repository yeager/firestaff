#include <stdio.h>

#include "theron_v1_track02_handoff_artifact_corpus.h"

int main(void)
{
    Theron_V1Track02HandoffArtifactCorpusReceipt receipt;
    Theron_V1Track02ExternalCaptureReceipt handoff = {0};
    Theron_V1Track02CaptureTargetPlan plan = {0};
    Theron_V1Track02HandoffArtifactCorpusCandidate candidate = {0};

    if (!theron_v1_track02_handoff_artifact_corpus_import(NULL, NULL, NULL, 0u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_UNAVAILABLE ||
        receipt.no_draw_only || receipt.opaque_artifact_consumed) return 1;
    candidate.bundle_path = "archive.zip::handoff.bundle";
    candidate.expected_bundle_md5 = "11111111111111111111111111111111";
    if (!theron_v1_track02_handoff_artifact_corpus_import(
            &handoff, &plan, &candidate, 1u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_HANDOFF_ARTIFACT_CORPUS_REJECTED ||
        receipt.direct_candidate_count || receipt.no_draw_only) return 2;
    puts("test_theron_v1_track02_handoff_artifact_corpus: PASS (no local corpus)");
    return 0;
}
