#include "asset_status_m12.h"
#include "firestaff_data_validator.h"
#include "firestaff_startup.h"
#include "theron_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define PATH_SEP "/"
#endif

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int write_file(const char *path, const char *text) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return 0;
    if (text && text[0]) {
        fwrite(text, 1, strlen(text), fp);
    }
    fclose(fp);
    return 1;
}

static int make_temp_dir(char out[512]) {
#if defined(_WIN32)
    char base[512];
    const char *tmp = getenv("TEMP");
    snprintf(base, sizeof(base), "%s\\firestaff_theron_avail_%lu",
             tmp ? tmp : ".", (unsigned long)rand());
    if (mkdir(base, 0700) != 0) return 0;
    snprintf(out, 512, "%s", base);
    return 1;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_avail_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

int main(void) {
    char temp_dir[512];
    char theron_dir[512];
    char fake_track[512];
    M12_AssetStatus status;
    FS_ValidationReport report;
    FS_GameAvailability availability;
    const char *real_data;

    expect_true(M12_AssetStatus_GameHasCompleteHashSet("theron") == 1,
                "Theron exposes a complete hash set");
    expect_true(M12_AssetStatus_GameKnownHashCount("theron") == 2U,
                "Theron exposes JP and US Track 02 hashes");
    expect_true(M12_AssetStatus_GameRequiredFileCount("theron") == 1U,
                "Theron requires one Track 02 data file");
    expect_true(M12_AssetStatus_GameVerifiedFileCount("theron") == 1U,
                "Theron verifies one Track 02 data file");
    expect_true(M12_AssetStatus_FindVersionIndex("theron", "pce-jp") == 0,
                "Theron JP version id is indexed");
    expect_true(M12_AssetStatus_FindVersionIndex("theron", "pce-en") == 1,
                "Theron US version id is indexed");

    expect_true(make_temp_dir(temp_dir), "temporary data dir created");
    snprintf(theron_dir, sizeof(theron_dir), "%s%s%s", temp_dir, PATH_SEP, "theron");
    expect_true(mkdir(theron_dir, 0700) == 0, "theron subdir created");
    snprintf(fake_track, sizeof(fake_track), "%s%s%s", theron_dir, PATH_SEP, "track02.bin");
    expect_true(write_file(fake_track, "not a known Theron's Quest data track"),
                "fake Track 02 candidate written");

    M12_AssetStatus_Scan(&status, temp_dir);
    expect_true(status.originalFileCandidateFound == 1,
                "Theron Track 02 candidate counts as original-file evidence");
    expect_true(status.theronAvailable == 0,
                "fake Track 02 is not marked available without a known MD5");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "game availability helper rejects fake Theron data");

    expect_true(fs_validate_data_dir(temp_dir, &report) >= 0,
                "validator runs on temp data dir");
    expect_true(report.theron_ready == 0,
                "validator rejects fake Theron data");

    fs_startup_check_games(temp_dir, &availability);
    expect_true(availability.theron_available == 0,
                "startup availability rejects fake Theron data");
    expect_true(theron_v1_boot_probe_available(temp_dir) == 0,
                "Theron quick boot probe rejects unverified Track 02 data");

    real_data = getenv("FIRESTAFF_THERON_TEST_DATA");
    if (real_data && real_data[0]) {
        M12_AssetStatus_Scan(&status, real_data);
        if (status.theronAvailable) {
            expect_true(fs_validate_data_dir(real_data, &report) >= 0 &&
                        report.theron_ready == 1,
                        "validator accepts hash-verified Theron data");
            fs_startup_check_games(real_data, &availability);
            expect_true(availability.theron_available == 1,
                        "startup accepts hash-verified Theron data");
            expect_true(theron_v1_boot_probe_available(real_data) == 1,
                        "Theron quick boot probe accepts hash-verified data");
        }
    }

    if (g_failures) {
        return 1;
    }
    printf("Theron V1 availability checks passed\n");
    return 0;
}
