#include "theron_v1_track02_level_object_descriptor_capture_intake.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Theron_V1Track02LevelObjectDescriptorCaptureIntakeReceipt receipt;
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt corpus;

    if (!theron_v1_track02_level_object_descriptor_capture_intake_admit(
            NULL, NULL, NULL, NULL, NULL, 0u, 0u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_UNAVAILABLE ||
        receipt.opaque_descriptor_only) return 1;

    memset(&corpus, 0, sizeof(corpus));
    corpus.status = THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY;
    corpus.direct_candidate_count = 1u;
    corpus.direct_regular_files_verified = 1;
    corpus.track02_md5_verified = 1;
    corpus.trace_md5_verified = 1;
    if (!theron_v1_track02_level_object_descriptor_capture_intake_admit(
            &corpus, NULL, NULL, NULL, NULL, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_LEVEL_OBJECT_DESCRIPTOR_CAPTURE_REJECTED ||
        receipt.opaque_descriptor_only) return 2;

    puts("test_theron_v1_track02_level_object_descriptor_capture_intake: PASS (no local corpus)");
    return 0;
}
