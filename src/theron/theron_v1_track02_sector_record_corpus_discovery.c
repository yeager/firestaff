#include "theron_v1_track02_sector_record_corpus_discovery.h"

#include "asset_status_m12.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int direct_regular_path(const char *path)
{
    struct stat info;
    return path && path[0] && !strstr(path, "::") &&
        stat(path, &info) == 0 && S_ISREG(info.st_mode);
}

static int path_exists(const char *path)
{
    struct stat info;
    return path && path[0] && stat(path, &info) == 0;
}

int theron_v1_track02_sector_record_corpus_discover(
    const Theron_V1Track02SectorRecordCorpusCandidate *candidates,
    unsigned int candidate_count,
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt *out)
{
    Theron_V1Track02SectorRecordCorpusDiscoveryReceipt receipt = {0};
    unsigned int i;

    if (!out) return 0;
    *out = receipt;
    receipt.supplied_candidate_count = candidate_count;
    if (!candidate_count) {
        receipt.status = THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_UNAVAILABLE;
        *out = receipt;
        return 1;
    }
    if (!candidates) {
        receipt.status = THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_REJECTED;
        *out = receipt;
        return 1;
    }
    for (i = 0u; i < candidate_count; ++i) {
        Theron_V1Track02RawMediaIntakeReceipt media;
        Theron_V1Track02SectorRecordAdmissionReceipt sector;
        char trace_md5[33];

        if (!path_exists(candidates[i].cue_path) ||
            !path_exists(candidates[i].coalesced_trace_path)) {
            ++receipt.unavailable_candidate_count;
            continue;
        }
        if (!direct_regular_path(candidates[i].cue_path) ||
            !direct_regular_path(candidates[i].coalesced_trace_path) ||
            !theron_v1_track02_raw_media_intake_discover(
                candidates[i].cue_path, &media) ||
            media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY ||
            !media.cue_consumed || !media.mode1_2352 ||
            !media.raw_trace_preparation_allowed ||
            !m12_file_md5_hex(candidates[i].coalesced_trace_path, trace_md5) ||
            !theron_v1_track02_sector_record_admit_from_trace(
                &media, candidates[i].coalesced_trace_path, &sector) ||
            sector.status != THERON_V1_TRACK02_SECTOR_RECORD_READY) {
            ++receipt.rejected_candidate_count;
            continue;
        }
        ++receipt.direct_candidate_count;
        if (receipt.direct_candidate_count == 1u) {
            receipt.direct_regular_files_verified = 1;
            receipt.track02_md5_verified = 1;
            receipt.trace_md5_verified = 1;
            snprintf(receipt.coalesced_trace_md5,
                     sizeof(receipt.coalesced_trace_md5), "%s", trace_md5);
            receipt.media = media;
            receipt.sector_record = sector;
        }
    }
    receipt.status = receipt.direct_candidate_count == 1u
        ? THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY
        : (receipt.rejected_candidate_count == 0u
            ? THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_UNAVAILABLE
            : THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_REJECTED);
    if (receipt.status != THERON_V1_TRACK02_SECTOR_RECORD_CORPUS_READY) {
        receipt.direct_regular_files_verified = 0;
        receipt.track02_md5_verified = 0;
        receipt.trace_md5_verified = 0;
        receipt.coalesced_trace_md5[0] = '\0';
        memset(&receipt.media, 0, sizeof(receipt.media));
        memset(&receipt.sector_record, 0, sizeof(receipt.sector_record));
    }
    *out = receipt;
    return 1;
}
