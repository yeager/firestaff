#define FIRESTAFF_ASSET_STATUS_TESTING 1
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#define TEST_ACCESS(path) _access((path), 0)
#else
#include <unistd.h>
#define TEST_ACCESS(path) access((path), R_OK)
#endif

static int failures;
static int assertions;

static void check(int condition, const char *message) {
    ++assertions;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int resolve_data_root(char *out, size_t capacity) {
    const char *configured = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");
    int written;

    if (configured && configured[0]) {
        written = snprintf(out, capacity, "%s", configured);
    } else if (home && home[0]) {
        written = snprintf(out, capacity, "%s/.firestaff/data", home);
    } else {
        return 0;
    }
    return written > 0 && (size_t)written < capacity;
}

int main(void) {
    char data_root[M12_ASSET_DATA_DIR_CAPACITY];
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics initial_metrics;
    const M12_AssetVersionStatus *version;
    const M12_AssetRequiredFileStatus *required;
    char verified_md5[M12_ASSET_MD5_CAPACITY];
    char actual_md5[M12_ASSET_MD5_CAPACITY];
    char verified_path[M12_ASSET_DATA_DIR_CAPACITY];
    char required_path[M12_ASSET_DATA_DIR_CAPACITY];
    char required_hash[M12_ASSET_MD5_CAPACITY];
    int refresh;

    if (!resolve_data_root(data_root, sizeof(data_root))) {
        puts("SKIP: authentic Firestaff data root is not configured");
        return 77;
    }
    if (snprintf(verified_path, sizeof(verified_path), "%s/theron", data_root) < 0 ||
        TEST_ACCESS(verified_path) != 0) {
        puts("SKIP: authentic Theron's Quest media is not installed");
        return 77;
    }

    memset(&status, 0, sizeof(status));
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, data_root);
    initial_metrics = M12_AssetStatus_TestGetScanMetrics();

    check(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
          "authentic Theron media is available to the launcher scan");
    version = M12_AssetStatus_GetVersion(&status, "theron", 2U);
    check(version && version->matched && version->matchedPath[0] &&
              version->matchedMd5[0],
          "launcher scan records an authenticated Track 02 path and hash");
    if (!version || !version->matched || !version->matchedPath[0] ||
        !version->matchedMd5[0]) {
        return 1;
    }
    snprintf(verified_path, sizeof(verified_path), "%s", version->matchedPath);
    snprintf(verified_md5, sizeof(verified_md5), "%s", version->matchedMd5);
    check(m12_file_md5_hex(verified_path, actual_md5) &&
              strcmp(actual_md5, verified_md5) == 0,
          "launcher scan identity matches the installed authentic Track 02 bytes");

    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
    check(required && required->matched && required->matchedPath[0] &&
              required->matchedHash[0],
          "launcher scan publishes an authenticated required Track 02 marker");
    if (!required || !required->matched || !required->matchedPath[0] ||
        !required->matchedHash[0]) {
        return 1;
    }
    snprintf(required_path, sizeof(required_path), "%s", required->matchedPath);
    snprintf(required_hash, sizeof(required_hash), "%s", required->matchedHash);
    check(m12_file_md5_hex(required_path, actual_md5) &&
              strcmp(actual_md5, required_hash) == 0,
          "required Track 02 marker hash matches authentic installed bytes");
    check(initial_metrics.rootCount > 0U,
          "initial launcher scan constructs its normal data search roots");
    check(initial_metrics.versionHashLookups > 0U,
          "initial launcher scan hashes candidates against the edition catalogue");

    for (refresh = 0; refresh < 3; ++refresh) {
        M12_AssetStatusScanMetrics refresh_metrics;
        M12_AssetStatus_TestResetScanMetrics();
        M12_AssetStatus_Scan(&status, data_root);
        refresh_metrics = M12_AssetStatus_TestGetScanMetrics();
        version = M12_AssetStatus_GetVersion(&status, "theron", 2U);
        required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);

        check(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "launcher refresh preserves authenticated Theron availability");
        check(version && version->matched &&
                  strcmp(version->matchedPath, verified_path) == 0 &&
                  strcmp(version->matchedMd5, verified_md5) == 0,
              "launcher refresh reuses the same authentic Track 02 identity");
        check(required && required->matched &&
                  strcmp(required->matchedPath, required_path) == 0 &&
                  strcmp(required->matchedHash, required_hash) == 0,
              "launcher refresh preserves the authenticated Track 02 marker");
        check(refresh_metrics.reusableTheronRefreshes == 1U,
              "launcher refresh uses the verified Theron reuse gate");
        check(refresh_metrics.rootCount == 0U &&
                  refresh_metrics.versionHashLookups == 0U &&
                  refresh_metrics.requiredHashLookups == 0U,
              "launcher refresh avoids repeating full-root hash scans");
    }

    if (failures) {
        fprintf(stderr, "%d failure(s), assertions=%d\n", failures, assertions);
        return 1;
    }
    printf("ok: authentic Theron launcher scan reuse assertions=%d\n", assertions);
    return 0;
}
