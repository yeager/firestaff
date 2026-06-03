#include "asset_status_m12.h"
#include "firestaff_nexus_v1_boot_profile.h"

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
        ++failures;
    }
}

static int make_dir_if_needed(const char* path) {
    return MKDIR(path) == 0;
}

static int make_isolated_root(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_nexus_avail_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-nexus-avail-XXXXXX";
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

static int any_nexus_version_matched(const M12_AssetStatus* status,
                                     const char* expectedMd5) {
    size_t i;
    for (i = 0U; i < M12_AssetStatus_GetVersionCount("nexus"); ++i) {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetVersion(status, "nexus", i);
        if (version && version->matched) {
            return expectedMd5 &&
                   strcmp(version->versionId, "nexus-saturn-jp") == 0 &&
                   strcmp(version->matchedMd5, expectedMd5) == 0;
        }
    }
    return 0;
}

static void check_profile_still_needs_runtime_slices(const char* root) {
    Nexus_V1_BootProfile profile;
    Nexus_V1_Diagnostic diags[NEXUS_V1_DIAG_COUNT];
    int diagCount;

    memset(diags, 0, sizeof(diags));
    check_int(Nexus_V1_BootProfile_Init(&profile, root, root, 0U) == 0,
              "Nexus boot profile initializes against fixture root");
    check_int(strstr(Nexus_V1_BootProfile_GetAssetRoot(&profile), "nexus") != NULL,
              "Nexus boot profile resolves the nexus asset subdir");
    diagCount = Nexus_V1_BootProfile_ValidateAssets(&profile,
                                                    diags,
                                                    NEXUS_V1_DIAG_COUNT);
    check_int(diagCount > 0,
              "hash-ready Nexus marker does not claim complete runtime slices");
}

int main(void) {
    static const char markerPayload[] =
        "Firestaff synthetic Nexus Saturn marker fixture v1\n";
    char root[512];
    char nexusDir[512];
    char markerPath[512];
    char markerMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus* required;

    check_int(M12_AssetStatus_GameHasCompleteHashSet("nexus") == 1,
              "Nexus exposes a complete hash gate");
    check_int(M12_AssetStatus_GameKnownHashCount("nexus") == 2U,
              "Nexus exposes the two hash-verified Saturn markers");
    check_int(M12_AssetStatus_GameRequiredFileCount("nexus") == 1U,
              "Nexus has one required launcher marker");
    check_int(M12_AssetStatus_GameVerifiedFileCount("nexus") == 1U,
              "Nexus verifies one launcher marker");
    check_int(M12_AssetStatus_FindVersionIndex("nexus", "nexus-saturn-jp") == 0,
              "Nexus Saturn JP profile is indexed");

    check_int(make_isolated_root(root, sizeof(root)),
              "temporary Nexus data root created");
    snprintf(nexusDir, sizeof(nexusDir), "%s/%s", root, "nexus");
    check_int(make_dir_if_needed(nexusDir), "nexus subdir created");
    snprintf(markerPath, sizeof(markerPath), "%s/%s", nexusDir, "DM.BIN");
    check_int(write_file(markerPath, markerPayload),
              "synthetic Nexus marker file written");
    check_int(m12_file_md5_hex(markerPath, markerMd5),
              "synthetic Nexus marker hash computed");
    check_int(test_setenv("HOME", root) &&
                  test_setenv("FIRESTAFF_DATA", root),
              "Nexus fixture environment isolated");

    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_Scan(&status, root);
    check_int(M12_AssetStatus_HasOriginalFileCandidate(&status) == 1,
              "fake Nexus marker is still original-file evidence");
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 0,
              "fake Nexus marker is not launch-ready without a known hash");
    required = M12_AssetStatus_GetRequiredFile(&status, "nexus", 0U);
    check_int(required && required->required && !required->matched,
              "fake Nexus marker leaves required hash marker unmatched");

    M12_AssetStatus_TestSetNexusSyntheticHash(markerMd5);
    M12_AssetStatus_Scan(&status, root);
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 1,
              "synthetic hash-verified Nexus marker passes launch availability");
    check_int(any_nexus_version_matched(&status, markerMd5),
              "synthetic Nexus marker selects the Saturn JP profile");
    required = M12_AssetStatus_GetRequiredFile(&status, "nexus", 0U);
    check_int(required && required->required && required->matched,
              "synthetic Nexus marker satisfies the required launcher marker");
    check_int(required && strcmp(required->matchedPath, markerPath) == 0,
              "required Nexus marker records the matched DM.BIN path");
    check_int(required && strcmp(required->matchedHash, markerMd5) == 0,
              "required Nexus marker records the matched hash");
    check_profile_still_needs_runtime_slices(root);

    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: Nexus availability gate requires a hash marker and stays separate from runtime proof");
    return 0;
}
