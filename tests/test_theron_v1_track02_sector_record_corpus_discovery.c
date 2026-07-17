#include "theron_v1_track02_sector_record_corpus_discovery.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt receipt;
    Theron_V1Track02SectorRecordCorpusCandidate bad = {
        "/tmp/firestaff-missing-sector-record.cue",
        "/tmp/firestaff-missing-sector-record.trace"
    };

    if (!theron_v1_track02_sector_record_corpus_discover(NULL, 0u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_UNAVAILABLE ||
        receipt.direct_candidate_count || receipt.sector_record.dungeon_draw_allowed) {
        return 1;
    }
    if (!theron_v1_track02_sector_record_corpus_discover(NULL, 1u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_REJECTED) {
        return 2;
    }
    if (!theron_v1_track02_sector_record_corpus_discover(&bad, 1u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_UNAVAILABLE ||
        receipt.direct_candidate_count || receipt.unavailable_candidate_count != 1u ||
        receipt.track02_md5_verified || receipt.trace_md5_verified ||
        receipt.sector_record.status != THERON_V1_TRACK02_SECTOR_RECORD_UNAVAILABLE) {
        return 3;
    }
    puts("test_theron_v1_track02_sector_record_corpus_discovery: PASS (no local corpus)");
    return 0;
}
