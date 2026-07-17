#ifndef THERON_V1_SRM_CORPUS_MANIFEST_H
#define THERON_V1_SRM_CORPUS_MANIFEST_H

#include "theron_v1_srm_opaque_admission.h"

#define THERON_V1_SRM_CORPUS_MAX_ROOTS 8u
#define THERON_V1_SRM_CORPUS_MAX_CANDIDATES 32u
#define THERON_V1_SRM_CORPUS_NAME_CAPACITY 64u

typedef enum {
    THERON_V1_SRM_CORPUS_UNAVAILABLE = 0,
    THERON_V1_SRM_CORPUS_REJECTED,
    THERON_V1_SRM_CORPUS_READY
} Theron_V1SrmCorpusStatus;

typedef enum {
    THERON_V1_SRM_CORPUS_REASON_NONE = 0,
    THERON_V1_SRM_CORPUS_REASON_SOURCE_ABSENT,
    THERON_V1_SRM_CORPUS_REASON_SOURCE_REJECTED,
    THERON_V1_SRM_CORPUS_REASON_DUPLICATE,
    THERON_V1_SRM_CORPUS_REASON_HASH_CONFLICT,
    THERON_V1_SRM_CORPUS_REASON_BAD_ROOT
} Theron_V1SrmCorpusReason;

typedef struct {
    unsigned int root_index;
    char name[THERON_V1_SRM_CORPUS_NAME_CAPACITY];
    char expected_srm_md5[33];
    size_t expected_srm_size;
    unsigned int admission_format_version;
    char track02_md5[33];
} Theron_V1SrmCorpusCandidate;

typedef struct {
    Theron_V1SrmCorpusStatus status;
    unsigned int root_count;
    char roots[THERON_V1_SRM_CORPUS_MAX_ROOTS][THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    unsigned int candidate_count;
    Theron_V1SrmCorpusCandidate candidates[THERON_V1_SRM_CORPUS_MAX_CANDIDATES];
} Theron_V1SrmCorpusManifest;

typedef struct {
    Theron_V1SrmCorpusReason reason;
    Theron_V1SrmOpaqueAdmissionReceipt admission;
} Theron_V1SrmCorpusCandidateReceipt;

typedef struct {
    Theron_V1SrmCorpusStatus status;
    unsigned int admitted_count;
    unsigned int absent_count;
    unsigned int rejected_count;
    int save_semantics_decoded;
    int synthetic_fallback_used;
    Theron_V1SrmCorpusCandidateReceipt candidates[THERON_V1_SRM_CORPUS_MAX_CANDIDATES];
} Theron_V1SrmCorpusReceipt;

/* Closed text format:
 * format=theron_srm_opaque_corpus_v1
 * root=<direct-save-root>
 * candidate=<root-index>:<name.srm>:<md5>:<size>:<version>:<track02-md5>
 */
int theron_v1_srm_corpus_manifest_parse(const char *path,
                                        Theron_V1SrmCorpusManifest *out);
int theron_v1_srm_corpus_manifest_scan(const Theron_V1SrmCorpusManifest *manifest,
                                       Theron_V1SrmCorpusReceipt *out);

#endif
