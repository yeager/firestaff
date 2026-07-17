#include "theron_v1_track02_sector_record_corpus_discovery.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    Theron_V1Track02SectorRecordCorpusCandidate candidate;
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt receipt;

    if (argc == 1) {
        puts("theron Track02 sector-record corpus: SKIP (no CUE/BIN/trace supplied)");
        return 0;
    }
    if (argc != 4 || strcmp(argv[1], "--inspect") != 0) {
        return 1;
    }
    candidate.cue_path = argv[2];
    candidate.coalesced_trace_path = argv[3];
    if (!theron_v1_track02_sector_record_corpus_discover(&candidate, 1u,
                                                           &receipt)) return 1;
    if (receipt.status == THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_UNAVAILABLE) {
        puts("theron Track02 sector-record corpus: SKIP (local input unavailable)");
        return 0;
    }
    if (receipt.status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY ||
        receipt.sector_record.dungeon_draw_allowed ||
        receipt.sector_record.pixel_decode_allowed ||
        receipt.sector_record.level_object_semantics_allowed) {
        fputs("theron Track02 sector-record corpus: REJECTED\n", stderr);
        return 1;
    }
    printf("theron Track02 sector-record corpus: READY record=%x offset=%zu hash=%08x\n",
           receipt.sector_record.resolved_track02_record,
           receipt.sector_record.record_user_data_offset,
           receipt.sector_record.record_user_data_hash);
    return 0;
}
