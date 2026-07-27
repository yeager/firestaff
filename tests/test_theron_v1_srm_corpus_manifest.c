#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "asset_status_m12.h"
#include "theron_v1_srm_corpus_manifest.h"

static int failures;
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); ++failures; \
} } while (0)

static void write_manifest(const char *path, const char *root, const char *md5,
                           int duplicate, int conflict) {
    FILE *file = fopen(path, "wb");
    if (!file) return;
    fprintf(file, "format=theron_srm_opaque_corpus_v1\nroot=%s\n"
            "candidate=0:slot0.srm:%s:4:1:%s\n", root, md5,
            THERON_TRACK02_MD5_US_BIN);
    if (duplicate) fprintf(file, "candidate=0:slot0.srm:%s:4:1:%s\n",
                           conflict ? "00000000000000000000000000000000" : md5,
                           THERON_TRACK02_MD5_US_BIN);
    fclose(file);
}

int main(void) {
    const char *root = "/tmp/firestaff-theron-srm-corpus";
    const char *srm = "/tmp/firestaff-theron-srm-corpus/slot0.srm";
    const char *manifest_path = "/tmp/firestaff-theron-srm-corpus.manifest";
    const unsigned char gzip_header[] = {0x1f, 0x8b, 0x08, 0x00};
    Theron_V1SrmCorpusManifest manifest;
    Theron_V1SrmCorpusReceipt receipt;
    char md5[33];
    FILE *file;

    remove(srm); remove(manifest_path); rmdir(root);
    CHECK(mkdir(root, 0700) == 0);
    file = fopen(srm, "wb");
    CHECK(file != NULL);
    if (file) { fwrite(gzip_header, 1, sizeof(gzip_header), file); fclose(file); }
    CHECK(m12_file_md5_hex(srm, md5));
    write_manifest(manifest_path, root, md5, 0, 0);
    CHECK(theron_v1_srm_corpus_manifest_parse(manifest_path, &manifest));
    CHECK(manifest.status == THERON_V1_SRM_CORPUS_READY);
    CHECK(theron_v1_srm_corpus_manifest_scan(&manifest, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_CORPUS_READY && receipt.admitted_count == 1);
    CHECK(!receipt.save_semantics_decoded && !receipt.synthetic_fallback_used);
    write_manifest(manifest_path, root, md5, 1, 1);
    CHECK(theron_v1_srm_corpus_manifest_parse(manifest_path, &manifest));
    CHECK(theron_v1_srm_corpus_manifest_scan(&manifest, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_CORPUS_REJECTED);
    CHECK(receipt.candidates[0].reason == THERON_V1_SRM_CORPUS_REASON_HASH_CONFLICT &&
          receipt.candidates[1].reason == THERON_V1_SRM_CORPUS_REASON_HASH_CONFLICT);
    remove(srm);
    write_manifest(manifest_path, root, md5, 0, 0);
    CHECK(theron_v1_srm_corpus_manifest_parse(manifest_path, &manifest));
    CHECK(theron_v1_srm_corpus_manifest_scan(&manifest, &receipt));
    CHECK(receipt.status == THERON_V1_SRM_CORPUS_UNAVAILABLE && receipt.absent_count == 1);
    remove(manifest_path); rmdir(root);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
