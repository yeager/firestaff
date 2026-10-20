#include "firestaff_zip_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t lastCompleted;
    uint64_t totalBytes;
    unsigned int callbacks;
    int cancelOnFirst;
} ZipProgressProbe;

static int observe_zip_progress(uint64_t completedBytes,
                                uint64_t totalBytes,
                                void *userData) {
    ZipProgressProbe *probe = (ZipProgressProbe *)userData;
    if (!probe || totalBytes == 0U || completedBytes < probe->lastCompleted ||
        completedBytes > totalBytes ||
        (probe->totalBytes && probe->totalBytes != totalBytes)) {
        return 0;
    }
    probe->lastCompleted = completedBytes;
    probe->totalBytes = totalBytes;
    ++probe->callbacks;
    return !(probe->cancelOnFirst && probe->callbacks == 1U);
}

static int test_nonexistent_file(void) {
    uint8_t *data = NULL;
    size_t size = 0;
    int rc = firestaff_zip_extract_by_suffix("/nonexistent.zip", ".txt",
                                             &data, &size);
    if (rc == 0) {
        fprintf(stderr, "FAIL: expected failure for nonexistent file\n");
        free(data);
        return 1;
    }
    return 0;
}

static int test_null_args(void) {
    int rc = firestaff_zip_extract_by_suffix(NULL, ".txt", NULL, NULL);
    if (rc == 0) {
        fprintf(stderr, "FAIL: expected failure for NULL args\n");
        return 1;
    }
    return 0;
}

static int test_dm2_fmtowns_cue(void) {
    const char *zip = getenv("FIRESTAFF_DM2_FMTOWNS_ZIP");
    if (!zip || !zip[0]) {
        printf("SKIP: FIRESTAFF_DM2_FMTOWNS_ZIP not set\n");
        return 0;
    }
    uint8_t *data = NULL;
    size_t size = 0;
    int rc = firestaff_zip_extract_by_suffix(zip, ".cue", &data, &size);
    if (rc != 0) {
        fprintf(stderr, "FAIL: could not extract .cue from %s\n", zip);
        return 1;
    }
    if (size == 0 || !data) {
        fprintf(stderr, "FAIL: .cue extracted but empty\n");
        free(data);
        return 1;
    }
    printf("OK: extracted .cue (%zu bytes)\n", size);
    free(data);
    return 0;
}

static int test_csb_fmtowns_zip_progress(void) {
    const char *zip = getenv("FIRESTAFF_CSB_FMTOWNS_ZIP");
    const char *imageName =
        "Dungeon Master Chaos Strikes Back (1990)(Victor)(Jp-En).img";
    FILE *fp = NULL;
    uint8_t *data = NULL;
    size_t size = 0U;
    ZipProgressProbe probe;
    int rc;

    if (!zip || !zip[0] || !(fp = fopen(zip, "rb"))) {
        if (fp) fclose(fp);
        printf("SKIP: authentic CSB FM Towns ZIP is not installed\n");
        return 0;
    }
    fclose(fp);
    memset(&probe, 0, sizeof(probe));
    rc = firestaff_zip_extract_by_name_with_progress(
        zip, imageName, &data, &size, observe_zip_progress, &probe);
    if (rc != 0 || !data || size == 0U || probe.callbacks < 2U ||
        probe.lastCompleted != size || probe.totalBytes != size) {
        fprintf(stderr,
                "FAIL: CSB original IMG progress receipt rc=%d size=%zu "
                "completed=%llu total=%llu callbacks=%u\n",
                rc, size, (unsigned long long)probe.lastCompleted,
                (unsigned long long)probe.totalBytes, probe.callbacks);
        free(data);
        return 1;
    }
    free(data);
    printf("OK: authentic CSB FM Towns IMG reported %u monotonic progress callbacks\n",
           probe.callbacks);

    memset(&probe, 0, sizeof(probe));
    probe.cancelOnFirst = 1;
    data = NULL;
    size = 0U;
    rc = firestaff_zip_extract_by_name_with_progress(
        zip, imageName, &data, &size, observe_zip_progress, &probe);
    if (rc != -2 || data != NULL || size != 0U || probe.callbacks != 1U) {
        fprintf(stderr,
                "FAIL: CSB ZIP cancellation rc=%d data=%p size=%zu callbacks=%u\n",
                rc, (void *)data, size, probe.callbacks);
        free(data);
        return 1;
    }
    printf("OK: authentic CSB FM Towns IMG extraction stops on progress cancellation\n");
    return 0;
}

int main(void) {
    int fails = 0;
    fails += test_null_args();
    fails += test_nonexistent_file();
    fails += test_dm2_fmtowns_cue();
    fails += test_csb_fmtowns_zip_progress();
    return fails ? 1 : 0;
}
