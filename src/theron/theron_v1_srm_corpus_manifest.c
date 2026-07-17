#include <stdio.h>
#include <string.h>

#include "theron_v1_srm_corpus_manifest.h"

static int name_is_direct_srm(const char *name) {
    size_t length = name ? strlen(name) : 0;
    return length > 4 && length < THERON_V1_SRM_CORPUS_NAME_CAPACITY &&
        !strchr(name, '/') && !strchr(name, '\\') &&
        !strcmp(name + length - 4, ".srm");
}

static void trim_line(char *line) {
    size_t length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
        line[--length] = '\0';
    }
}

int theron_v1_srm_corpus_manifest_parse(const char *path,
                                        Theron_V1SrmCorpusManifest *out) {
    Theron_V1SrmCorpusManifest manifest = {0};
    FILE *file;
    char line[THERON_V1_SRM_OPAQUE_PATH_CAPACITY + 192];

    if (!out) return 0;
    *out = manifest;
    if (!path || !path[0] || !(file = fopen(path, "rb"))) return 1;
    if (!fgets(line, sizeof(line), file)) goto rejected;
    trim_line(line);
    if (strcmp(line, "format=theron_srm_opaque_corpus_v1")) goto rejected;
    while (fgets(line, sizeof(line), file)) {
        Theron_V1SrmCorpusCandidate *candidate;
        int consumed = 0;
        trim_line(line);
        if (!strncmp(line, "root=", 5)) {
            if (!line[5] || manifest.root_count >= THERON_V1_SRM_CORPUS_MAX_ROOTS) goto rejected;
            snprintf(manifest.roots[manifest.root_count],
                     sizeof(manifest.roots[manifest.root_count]), "%s", line + 5);
            ++manifest.root_count;
            continue;
        }
        if (strncmp(line, "candidate=", 10) ||
            manifest.candidate_count >= THERON_V1_SRM_CORPUS_MAX_CANDIDATES) goto rejected;
        candidate = &manifest.candidates[manifest.candidate_count];
        if (sscanf(line + 10, "%u:%63[^:]:%32[^:]:%zu:%u:%32[^:]%n",
                   &candidate->root_index, candidate->name, candidate->expected_srm_md5,
                   &candidate->expected_srm_size, &candidate->admission_format_version,
                   candidate->track02_md5, &consumed) != 6 ||
            (line + 10)[consumed] != '\0' || !name_is_direct_srm(candidate->name) ||
            candidate->root_index >= manifest.root_count || !candidate->expected_srm_size) goto rejected;
        ++manifest.candidate_count;
    }
    fclose(file);
    manifest.status = THERON_V1_SRM_CORPUS_READY;
    *out = manifest;
    return 1;
rejected:
    fclose(file);
    manifest.status = THERON_V1_SRM_CORPUS_REJECTED;
    *out = manifest;
    return 1;
}

int theron_v1_srm_corpus_manifest_scan(const Theron_V1SrmCorpusManifest *manifest,
                                       Theron_V1SrmCorpusReceipt *out) {
    Theron_V1SrmCorpusReceipt receipt = {0};
    unsigned int i;

    if (!out) return 0;
    *out = receipt;
    if (!manifest || manifest->status == THERON_V1_SRM_CORPUS_UNAVAILABLE) return 1;
    if (manifest->status != THERON_V1_SRM_CORPUS_READY || !manifest->root_count ||
        !manifest->candidate_count) { receipt.status = THERON_V1_SRM_CORPUS_REJECTED; *out = receipt; return 1; }
    for (i = 0; i < manifest->candidate_count; ++i) {
        const Theron_V1SrmCorpusCandidate *candidate = &manifest->candidates[i];
        Theron_V1SrmCorpusCandidateReceipt *candidate_receipt = &receipt.candidates[i];
        Theron_V1SrmOpaqueAdmissionRequest request = {0};
        char path[THERON_V1_SRM_OPAQUE_PATH_CAPACITY];
        unsigned int j;
        int duplicate = 0;
        int path_length;

        if (candidate->root_index >= manifest->root_count || !manifest->roots[candidate->root_index][0]) {
            candidate_receipt->reason = THERON_V1_SRM_CORPUS_REASON_BAD_ROOT;
            ++receipt.rejected_count;
            continue;
        }
        path_length = snprintf(path, sizeof(path), "%s/%s",
                               manifest->roots[candidate->root_index], candidate->name);
        if (path_length < 0 || (size_t)path_length >= sizeof(path)) {
            candidate_receipt->reason = THERON_V1_SRM_CORPUS_REASON_BAD_ROOT;
            ++receipt.rejected_count;
            continue;
        }
        for (j = 0; j < i; ++j) {
            const Theron_V1SrmCorpusCandidate *prior = &manifest->candidates[j];
            if (prior->root_index == candidate->root_index && !strcmp(prior->name, candidate->name)) {
                candidate_receipt->reason = strcmp(prior->expected_srm_md5, candidate->expected_srm_md5) ?
                    THERON_V1_SRM_CORPUS_REASON_HASH_CONFLICT : THERON_V1_SRM_CORPUS_REASON_DUPLICATE;
                if (receipt.candidates[j].admission.status == THERON_V1_SRM_OPAQUE_READY) {
                    receipt.candidates[j].admission.status = THERON_V1_SRM_OPAQUE_REJECTED;
                    receipt.candidates[j].reason = candidate_receipt->reason;
                    --receipt.admitted_count;
                    ++receipt.rejected_count;
                }
                ++receipt.rejected_count;
                duplicate = 1;
                break;
            }
        }
        if (duplicate) continue;
        request.srm_path = path;
        request.expected_srm_md5 = candidate->expected_srm_md5;
        request.expected_srm_size = candidate->expected_srm_size;
        request.admission_format_version = candidate->admission_format_version;
        request.expected_shape = THERON_V1_SRM_OPAQUE_SHAPE_GZIP_DEFLATE_V1;
        request.expected_track02_md5 = candidate->track02_md5;
        request.track02_variant = theron_v1_track02_variant_for_md5(candidate->track02_md5);
        theron_v1_srm_opaque_admit(&request, &candidate_receipt->admission);
        if (candidate_receipt->admission.status == THERON_V1_SRM_OPAQUE_READY) {
            ++receipt.admitted_count;
        } else if (candidate_receipt->admission.status == THERON_V1_SRM_OPAQUE_UNAVAILABLE) {
            candidate_receipt->reason = THERON_V1_SRM_CORPUS_REASON_SOURCE_ABSENT;
            ++receipt.absent_count;
        } else {
            candidate_receipt->reason = THERON_V1_SRM_CORPUS_REASON_SOURCE_REJECTED;
            ++receipt.rejected_count;
        }
    }
    receipt.status = receipt.rejected_count ? THERON_V1_SRM_CORPUS_REJECTED :
        (receipt.admitted_count ? THERON_V1_SRM_CORPUS_READY : THERON_V1_SRM_CORPUS_UNAVAILABLE);
    *out = receipt;
    return 1;
}
