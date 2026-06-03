#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
static int test_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int failures;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

static int make_dir_if_needed(const char* path) {
    return MKDIR(path) == 0;
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_asset_status_metrics_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-asset-status-metrics-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

int main(void) {
    enum {
        KNOWN_VERSION_HASHES = 16,
        FIXED_REQUIRED_HASHES = 3
    };
    char home[512];
    char requestRoot[512];
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics metrics;

    if (!make_isolated_home(home, sizeof(home))) {
        fprintf(stderr, "fixture home setup failed\n");
        return 1;
    }
    snprintf(requestRoot, sizeof(requestRoot), "%s/custom-data", home);
    if (!make_dir_if_needed(requestRoot) ||
        !test_setenv("HOME", home) ||
        !test_setenv("FIRESTAFF_DATA", requestRoot)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, requestRoot);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(metrics.rootCount == 2U,
              "requested root and default root should be scanned once each");
    check_int(metrics.duplicateRootSkips == 1U,
              "FIRESTAFF_DATA duplicate of requested root should be skipped");
    check_int(metrics.versionHashLookups ==
                  (size_t)KNOWN_VERSION_HASHES * metrics.rootCount,
              "empty scan should show one version-hash search per known hash/root");
    check_int(metrics.requiredHashLookups ==
                  (size_t)FIXED_REQUIRED_HASHES * metrics.rootCount,
              "empty scan should show one fixed required-file search per hash/root");
    check_int(!M12_AssetStatus_HasOriginalFileCandidate(&status),
              "empty fixture should not report original asset candidates");

    (void)test_setenv("FIRESTAFF_DATA", NULL);
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: asset-status scan metrics pin duplicate-root prefilter and hash fan-out");
    return 0;
}
