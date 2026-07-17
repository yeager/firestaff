#ifndef THERON_V1_SRM_OPERATOR_ATTESTATION_H
#define THERON_V1_SRM_OPERATOR_ATTESTATION_H

#include "theron_v1_srm_corpus_manifest.h"

typedef enum {
    THERON_V1_SRM_ATTEST_UNAVAILABLE = 0,
    THERON_V1_SRM_ATTEST_REJECTED,
    THERON_V1_SRM_ATTEST_READY
} Theron_V1SrmOperatorAttestationStatus;

typedef struct {
    const char *srm_path;
    const char *track02_cue_path;
    const char *expected_track02_md5;
    unsigned int admission_format_version;
} Theron_V1SrmOperatorAttestationRequest;

typedef struct {
    Theron_V1SrmOperatorAttestationStatus status;
    int srm_path_canonical;
    int track02_path_canonical;
    int source_regular_file_verified;
    int track02_provenance_verified;
    int opaque_admission_verified;
    int save_mutated;
    int save_decoded;
    char canonical_srm_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char canonical_track02_cue_path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char manifest_root[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
    char manifest_name[THERON_V1_SRM_CORPUS_NAME_CAPACITY];
    char srm_md5[33];
    size_t srm_size;
    unsigned int admission_format_version;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
} Theron_V1SrmOperatorAttestationReceipt;

int theron_v1_srm_operator_attest(
    const Theron_V1SrmOperatorAttestationRequest *request,
    Theron_V1SrmOperatorAttestationReceipt *out);

#endif
