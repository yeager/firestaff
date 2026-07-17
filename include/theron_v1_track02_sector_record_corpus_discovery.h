#ifndef THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_DISCOVERY_H
#define THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_DISCOVERY_H

#include "theron_v1_track02_sector_record_admission.h"

typedef enum {
    THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_UNAVAILABLE = 0,
    THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_REJECTED,
    THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY
} Theron_V1Track02SectorRecordCorpusDiscoveryStatus;

typedef struct {
    const char *cue_path;
    const char *coalesced_trace_path;
} Theron_V1Track02SectorRecordCorpusCandidate;

/* Selects exactly one direct regular-file CUE/trace pair. The Track 02 and
 * trace MD5 values are calculated from the supplied files; no archive member,
 * generated trace, media copy, or record-payload interpretation is accepted. */
typedef struct {
    Theron_V1Track02SectorRecordCorpusDiscoveryStatus status;
    unsigned int supplied_candidate_count;
    unsigned int direct_candidate_count;
    unsigned int unavailable_candidate_count;
    unsigned int rejected_candidate_count;
    int direct_regular_files_verified;
    int track02_md5_verified;
    int trace_md5_verified;
    char coalesced_trace_md5[33];
    Theron_V1Track02RawMediaIntakeReceipt media;
    Theron_V1Track02SectorRecordAdmissionReceipt sector_record;
} Theron_V1Track02SectorRecordCorpusDiscoveryReceipt;

int theron_v1_track02_sector_record_corpus_discover(
    const Theron_V1Track02SectorRecordCorpusCandidate *candidates,
    unsigned int candidate_count,
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *out);

#endif
