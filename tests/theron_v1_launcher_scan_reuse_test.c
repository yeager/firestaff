#define FIRESTAFF_ASSET_STATUS_TESTING 1
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
static int test_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int failures;
static int assertions;

static void check_int(int condition, const char* message) {
    ++assertions;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int make_dir_if_needed(const char* path) {
    return TEST_MKDIR(path) == 0;
}

static int make_isolated_root(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_theron_scan_reuse_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-theron-scan-reuse-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int write_file(const char* path, const char* text) {
    FILE* fp = fopen(path, "wb");
    size_t len = text ? strlen(text) : 0U;
    if (!fp) {
        return 0;
    }
    if (len > 0U && fwrite(text, 1U, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

int main(void) {
    static const char trackPayload[] =
        "Firestaff synthetic Theron launcher scan reuse fixture v1\n";
    char root[512];
    char theronDir[512];
    char trackPath[512];
    char trackMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    M12_AssetStatus directRootStatus;
    M12_AssetStatusScanMetrics directRootMetrics;
    M12_AssetStatusScanMetrics firstMetrics;
    M12_AssetStatusScanMetrics refreshMetrics;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* required;

    check_int(make_isolated_root(root, sizeof(root)),
              "temporary Theron data root created");
    snprintf(theronDir, sizeof(theronDir), "%s/theron", root);
    check_int(make_dir_if_needed(theronDir),
              "theron fixture directory created");
    snprintf(trackPath, sizeof(trackPath), "%s/track02.bin", theronDir);
    check_int(write_file(trackPath, trackPayload),
              "synthetic Theron Track 02 fixture written");
    check_int(m12_file_md5_hex(trackPath, trackMd5),
              "synthetic Theron Track 02 MD5 computed");
    check_int(test_setenv("HOME", root) &&
                  test_setenv("FIRESTAFF_DATA", root),
              "Theron scan-reuse fixture environment isolated");

    memset(&status, 0, sizeof(status));
    M12_AssetStatus_TestSetTheronSyntheticHash(trackMd5);
    memset(&directRootStatus, 0, sizeof(directRootStatus));
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_ScanGame(&directRootStatus, root, "theron");
    directRootMetrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(M12_AssetStatus_GameAvailable(&directRootStatus, "theron") == 1,
              "Theron direct-launch scan resolves root/theron without full menu scan");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&directRootStatus), root) == 0,
              "Theron direct-launch scan preserves the configured data root");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&directRootStatus, "theron"),
                     theronDir) == 0,
              "Theron direct-launch scan keeps root/theron as the runtime dir only");
    version = M12_AssetStatus_GetVersion(&directRootStatus, "theron", 0U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, trackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "Theron direct-launch scan records the verified Track 02 child path");
    required = M12_AssetStatus_GetRequiredFile(&directRootStatus, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, trackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "Theron direct-launch scan propagates the Track 02 required marker");
    check_int(directRootMetrics.rootCount == 0U,
              "Theron direct-launch scan skips root-wide search-root construction");
    check_int(directRootMetrics.requiredHashLookups == 0U,
              "Theron direct-launch scan skips root-wide required-file hash lookups");

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    firstMetrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "initial launcher scan marks synthetic Theron Track 02 available");
    version = M12_AssetStatus_GetVersion(&status, "theron", 0U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, trackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "initial launcher scan records the verified Theron path and hash");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, trackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "initial launcher scan records the required Theron Track 02 marker");
    check_int(firstMetrics.rootCount > 0U,
              "initial launcher scan builds search roots");
    check_int(firstMetrics.versionHashLookups > 0U,
              "initial launcher scan performs version hash lookups");

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    refreshMetrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "repeat launcher refresh keeps Theron available");
    version = M12_AssetStatus_GetVersion(&status, "theron", 0U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, trackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "repeat launcher refresh reuses the verified Theron path and hash");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, trackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "repeat launcher refresh keeps the required Theron Track 02 marker");
    check_int(refreshMetrics.reusableTheronRefreshes == 1U,
              "repeat launcher refresh hits the verified Theron reuse gate once");
    check_int(refreshMetrics.rootCount == 0U,
              "repeat launcher refresh skips search-root construction");
    check_int(refreshMetrics.versionHashLookups == 0U,
              "repeat launcher refresh skips root-wide version hash lookups");
    check_int(refreshMetrics.requiredHashLookups == 0U,
              "repeat launcher refresh skips root-wide required hash lookups");

    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s), assertions=%d\n", failures, assertions);
        return 1;
    }
    printf("ok: Theron launcher scan reuse assertions=%d md5=%s\n",
           assertions,
           trackMd5);
    return 0;
}
