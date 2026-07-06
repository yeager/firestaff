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
        ++failures;
    }
}

static int make_dir_if_needed(const char* path) {
    return MKDIR(path) == 0;
}

static int make_isolated_root(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_nexus_bpk_meta_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-nexus-bpk-meta-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static void wb32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)((v >> 24U) & 0xffU);
    p[1] = (unsigned char)((v >> 16U) & 0xffU);
    p[2] = (unsigned char)((v >> 8U) & 0xffU);
    p[3] = (unsigned char)(v & 0xffU);
}

static void wb16(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)((v >> 8U) & 0xffU);
    p[1] = (unsigned char)(v & 0xffU);
}

static size_t make_synthetic_menu_bpk(unsigned char* buf, size_t cap) {
    const unsigned int count = 3U;
    const unsigned int off0 = 36U;
    const unsigned int off1 = 76U;
    const unsigned int off2 = 120U;
    const unsigned int size = 160U;
    unsigned char* p;

    if (!buf || cap < size) {
        return 0U;
    }
    memset(buf, 0, cap);
    memcpy(buf + 0U, "BPPK", 4U);
    wb32(buf + 4U, size);
    memcpy(buf + 12U, "BMPD", 4U);
    wb32(buf + 16U, size - 12U);
    wb32(buf + 20U, count);
    wb32(buf + 24U, off0);
    wb32(buf + 28U, off1);
    wb32(buf + 32U, off2);

    p = buf + off0;
    wb32(p + 0U, off1);
    wb32(p + 4U, off2);
    p[19] = 10U;

    p = buf + off1;
    wb16(p + 12U, 4U);
    p[15] = 5U;
    p[19] = 14U;
    memcpy(p + 20U, "PRS3", 4U);
    wb32(p + 24U, 1U);
    wb32(p + 28U, 20U);

    p = buf + off2;
    wb16(p + 12U, 3U);
    p[15] = 2U;
    p[19] = 6U;
    memcpy(p + 20U, "PRS3", 4U);
    wb32(p + 24U, 1U);
    wb32(p + 28U, 6U);
    return size;
}

static int write_bytes(const char* path, const unsigned char* data, size_t size) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (fwrite(data, 1U, size, fp) != size) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

int main(void) {
    unsigned char bpk[192];
    size_t bpkSize;
    char root[512];
    char nexusDir[512];
    char bpkPath[512];
    char directRoot[512];
    char renamedNexusDataPath[512];
    char nexusDataMd5[33];
    const unsigned char nexusDataPayload[] =
        "Firestaff synthetic Nexus launch marker with arbitrary filename\n";
    M12_AssetStatus status;
    const M12_NexusBpkTrailerMetadata* meta;

    check_int(make_isolated_root(root, sizeof(root)), "temporary root created");
    snprintf(nexusDir, sizeof(nexusDir), "%s/%s", root, "nexus");
    check_int(make_dir_if_needed(nexusDir), "nexus subdir created");
    snprintf(bpkPath, sizeof(bpkPath), "%s/%s", nexusDir, "MENU.BPK");
    bpkSize = make_synthetic_menu_bpk(bpk, sizeof(bpk));
    check_int(bpkSize == 160U, "synthetic MENU.BPK built");
    check_int(write_bytes(bpkPath, bpk, bpkSize), "synthetic MENU.BPK written");
    check_int(test_setenv("HOME", root) && test_setenv("FIRESTAFF_DATA", root),
              "environment isolated");

    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_Scan(&status, root);
    meta = M12_AssetStatus_GetNexusBpkTrailerMetadata(&status);
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 0,
              "MENU.BPK metadata alone does not make Nexus launch-ready");
    check_int(M12_AssetStatus_HasOriginalFileCandidate(&status) == 1,
              "MENU.BPK counts as original asset evidence");
    check_int(meta && meta->probed && meta->found && meta->parsed,
              "Nexus MENU.BPK metadata is parsed into launcher status");
    check_int(meta && strcmp(meta->matchedPath, bpkPath) == 0,
              "metadata records MENU.BPK path");
    check_int(meta && meta->entryCount == 3U &&
                  meta->prs3PayloadCount == 2U &&
                  meta->rawPayloadCount == 1U,
              "metadata records directory entry counts");
    check_int(meta && meta->trailerFound && meta->trailerIndex == 0U,
              "metadata records directory-trailer index");
    check_int(meta && meta->mode8bppCount == 1U &&
                  meta->mode16bppCount == 1U &&
                  meta->mode24bppCount == 0U &&
                  meta->mode32bppCount == 0U &&
                  meta->trailerModeCount == 1U,
              "metadata records BPX/BPK mode distribution");
    check_int(meta && meta->trailerFirstOffset == 76U &&
                  meta->trailerSecondOffset == 120U,
              "metadata exposes directory-trailer target offsets");

    snprintf(directRoot, sizeof(directRoot), "%s/%s", root, "renamed-nexus");
    check_int(make_dir_if_needed(directRoot),
              "direct Nexus hash fixture directory created");
    snprintf(renamedNexusDataPath, sizeof(renamedNexusDataPath),
             "%s/%s", directRoot, "disc-image.payload");
    check_int(write_bytes(renamedNexusDataPath,
                          nexusDataPayload,
                          sizeof(nexusDataPayload) - 1U),
              "renamed Nexus data marker written");
    check_int(m12_file_md5_hex(renamedNexusDataPath, nexusDataMd5),
              "renamed Nexus data marker hashed");

    M12_AssetStatus_TestSetNexusSyntheticHash(nexusDataMd5);
    M12_AssetStatus_Scan(&status, renamedNexusDataPath);
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 1,
              "direct Nexus file request should hash-scan its parent even "
              "without .cue/.bin/.iso extension");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "nexus"),
                     directRoot) == 0,
              "direct renamed Nexus file request should use the matched "
              "parent as runtime root");
    check_int(M12_AssetStatus_GetRequiredFileCount(&status, "nexus") == 1U,
              "direct renamed Nexus scan keeps the Nexus required-file row");
    {
        const M12_AssetRequiredFileStatus* required =
            M12_AssetStatus_GetRequiredFile(&status, "nexus", 0U);
        check_int(required && required->matched &&
                      strcmp(required->matchedPath, renamedNexusDataPath) == 0 &&
                      strcmp(required->matchedHash, nexusDataMd5) == 0,
                  "direct renamed Nexus required row records the hash-matched "
                  "file, not a filename-derived candidate");
    }

    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: Nexus MENU.BPK directory-trailer metadata is launcher-visible and non-blocking");
    return 0;
}
