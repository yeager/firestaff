#include "firestaff_zip_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void) {
    int fails = 0;
    fails += test_null_args();
    fails += test_nonexistent_file();
    fails += test_dm2_fmtowns_cue();
    return fails ? 1 : 0;
}
