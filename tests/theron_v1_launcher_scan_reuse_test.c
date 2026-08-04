#define FIRESTAFF_ASSET_STATUS_TESTING 1
#include "asset_status_m12.h"
#include "theron_v1_boot.h"

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

static int write_bytes(const char* path, const void* bytes, size_t size) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0U && fwrite(bytes, 1U, size, fp) != size) {
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
    char extrasDir[512];
    char extrasJapanDir[512];
    char looseDir[512];
    char trackPath[512];
    char extrasTrackPath[512];
    char extrasCuePath[512];
    char extrasAudioPath[512];
    char isoCueDir[512];
    char isoCuePath[512];
    char isoTrackPath[512];
    char isoAudioPath[512];
    char looseTrackPath[512];
    char trackMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    M12_AssetStatus directRootStatus;
    M12_AssetStatus directCueStatus;
    M12_AssetStatus isoCueStatus;
    M12_AssetStatus specificTheronStatus;
    M12_AssetStatus looseDirStatus;
    M12_AssetStatusScanMetrics directRootMetrics;
    M12_AssetStatusScanMetrics looseDirMetrics;
    M12_AssetStatusScanMetrics firstMetrics;
    M12_AssetStatusScanMetrics refreshMetrics;
    const FirestaffTheronMediaStatus* media;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* required;
    const char* isoLaunchPath;
    Theron_V1_BootProfile isoProfile;

    check_int(make_isolated_root(root, sizeof(root)),
              "temporary Theron data root created");
    snprintf(theronDir, sizeof(theronDir), "%s/theron", root);
    check_int(make_dir_if_needed(theronDir),
              "theron fixture directory created");
    snprintf(extrasDir, sizeof(extrasDir), "%s/theron-extras", root);
    check_int(make_dir_if_needed(extrasDir),
              "theron-extras fixture directory created");
    snprintf(extrasJapanDir, sizeof(extrasJapanDir), "%s/japan", extrasDir);
    check_int(make_dir_if_needed(extrasJapanDir),
              "theron-extras/japan fixture directory created");
    snprintf(trackPath, sizeof(trackPath), "%s/track02.bin", theronDir);
    check_int(write_file(trackPath, trackPayload),
              "synthetic Theron Track 02 fixture written");
    snprintf(extrasTrackPath, sizeof(extrasTrackPath), "%s/track02.bin", extrasJapanDir);
    check_int(write_file(extrasTrackPath, trackPayload),
              "synthetic Theron raw Track 02 fixture written");
    snprintf(extrasAudioPath, sizeof(extrasAudioPath), "%s/track01.bin", extrasJapanDir);
    {
        unsigned char audio[2352] = {0};
        check_int(write_bytes(extrasAudioPath, audio, sizeof(audio)),
                  "synthetic Theron Track 01 CDDA fixture written");
    }
    snprintf(extrasCuePath, sizeof(extrasCuePath), "%s/original.cue", extrasJapanDir);
    check_int(write_file(extrasCuePath,
                         "FILE \"track01.bin\" BINARY\n"
                         "  TRACK 01 AUDIO\n"
                         "    INDEX 01 00:00:00\n"
                         "FILE \"track02.bin\" BINARY\n"
                         "  TRACK 02 MODE1/2352\n"
                         "    INDEX 01 00:00:00\n"),
              "strict paired Theron CUE fixture written");
    snprintf(isoCueDir, sizeof(isoCueDir), "%s/theron-iso", root);
    check_int(make_dir_if_needed(isoCueDir),
              "Theron ISO CUE fixture directory created");
    snprintf(isoTrackPath, sizeof(isoTrackPath), "%s/Track 02.iso", isoCueDir);
    check_int(write_file(isoTrackPath, trackPayload),
              "Theron ISO Track 02 fixture written");
    snprintf(isoAudioPath, sizeof(isoAudioPath), "%s/Track 01.bin", isoCueDir);
    {
        unsigned char audio[2352] = {0};
        check_int(write_bytes(isoAudioPath, audio, sizeof(audio)),
                  "Theron ISO CUE Track 01 fixture written");
    }
    snprintf(isoCuePath, sizeof(isoCuePath), "%s/Therons Quest.cue", isoCueDir);
    check_int(write_file(isoCuePath,
                         "FILE \"Track 01.bin\" BINARY\n"
                         "  TRACK 01 AUDIO\n"
                         "    INDEX 01 00:00:00\n"
                         "FILE \"Track 02.iso\" BINARY\n"
                         "  TRACK 02 MODE1/2048\n"
                         "    INDEX 01 00:00:00\n"),
              "strict MODE1/2048 Theron CUE fixture written");
    snprintf(looseDir, sizeof(looseDir), "%s/renamed-media", root);
    check_int(make_dir_if_needed(looseDir),
              "arbitrary renamed Theron media directory created");
    snprintf(looseTrackPath, sizeof(looseTrackPath), "%s/payload.noext", looseDir);
    check_int(write_file(looseTrackPath, trackPayload),
              "arbitrary renamed Theron Track 02 fixture written");
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
                     extrasJapanDir) == 0,
              "Theron direct-launch scan prefers raw theron-extras Track 02 runtime dir");
    version = M12_AssetStatus_GetVersion(&directRootStatus, "theron", 1U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, extrasTrackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "Theron direct-launch scan records the verified raw Track 02 child path");
    required = M12_AssetStatus_GetRequiredFile(&directRootStatus, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, extrasTrackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "Theron direct-launch scan propagates the raw Track 02 required marker");
    media = M12_AssetStatus_GetTheronMediaStatus(&directRootStatus);
    check_int(media && media->paired_track01_track02 &&
                  strcmp(media->cue_path, extrasCuePath) == 0 &&
                  strcmp(media->track01_path, extrasAudioPath) == 0 &&
                  strcmp(media->track02_path, extrasTrackPath) == 0,
              "Theron scanner records canonical CUE Track 01 and verified Track 02 paths");
    check_int(!M12_AssetStatus_GetTheronTrack02LoaderReceipt(&directRootStatus)->valid,
              "synthetic hash fixture cannot manufacture a Track02 IPL loader receipt");
    check_int(strcmp(M12_AssetStatus_GetTheronLaunchMediaPath(&directRootStatus),
                     extrasCuePath) == 0,
              "Theron launch profile receives the strict paired CUE path");
    check_int(directRootMetrics.rootCount == 0U,
              "Theron direct-launch scan skips root-wide search-root construction");
    check_int(directRootMetrics.requiredHashLookups == 0U,
              "Theron direct-launch scan skips root-wide required-file hash lookups");

    memset(&directCueStatus, 0, sizeof(directCueStatus));
    M12_AssetStatus_ScanGame(&directCueStatus, extrasCuePath, "theron");
    check_int(M12_AssetStatus_GameAvailable(&directCueStatus, "theron") == 1,
              "Theron direct CUE scan accepts a canonical paired media file");
    check_int(strcmp(M12_AssetStatus_GetTheronLaunchMediaPath(&directCueStatus),
                     extrasCuePath) == 0,
              "Theron direct CUE scan preserves CUE provenance for launch");

    /* The scanner verifies the declared ISO payload by hash, keeps the CUE
     * only as launch provenance, and the boot handoff resolves it back to
     * that ISO. A MODE1/2048 layout must not need a raw IPL receipt. */
    memset(&isoCueStatus, 0, sizeof(isoCueStatus));
    M12_AssetStatus_ScanGame(&isoCueStatus, isoCuePath, "theron");
    check_int(M12_AssetStatus_GameAvailable(&isoCueStatus, "theron") == 1,
              "Theron direct ISO CUE scan accepts MODE1/2048 Track 02");
    media = M12_AssetStatus_GetTheronMediaStatus(&isoCueStatus);
    check_int(media && media->paired_track01_track02 &&
                  media->track02_mode1_sector_bytes == 2048 &&
                  media->has_valid_track02_mode1 &&
                  strcmp(media->track02_path, isoTrackPath) == 0,
              "Theron scanner preserves the declared 2048-byte ISO Track 02 path");
    check_int(!M12_AssetStatus_GetTheronTrack02LoaderReceipt(&isoCueStatus)->valid,
              "MODE1/2048 ISO CUE does not claim a raw Track02 IPL receipt");
    isoLaunchPath = M12_AssetStatus_GetTheronLaunchMediaPath(&isoCueStatus);
    check_int(isoLaunchPath && strcmp(isoLaunchPath, isoCuePath) == 0,
              "Theron ISO CUE reaches the startup handoff as CUE provenance");
    memset(&isoProfile, 0, sizeof(isoProfile));
    check_int(theron_v1_boot_load_verified_path(&isoProfile,
                                                isoLaunchPath,
                                                THERON_TRACK02_MD5_US_ISO) == 0 &&
                  strcmp(isoProfile.graphics_path, isoTrackPath) == 0 &&
                  strcmp(isoProfile.dungeon_path, isoTrackPath) == 0,
              "Theron startup handoff resolves MODE1/2048 CUE to its verified ISO payload");

    memset(&specificTheronStatus, 0, sizeof(specificTheronStatus));
    M12_AssetStatus_ScanGame(&specificTheronStatus, theronDir, "theron");
    check_int(M12_AssetStatus_GameAvailable(&specificTheronStatus, "theron") == 1,
              "Theron specific-directory scan still accepts an explicit theron/ root");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&specificTheronStatus, "theron"),
                     theronDir) == 0,
              "Theron specific-directory scan keeps the explicit theron/ runtime dir");
    version = M12_AssetStatus_GetVersion(&specificTheronStatus, "theron", 1U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, trackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "Theron specific-directory scan records the explicit theron/ Track 02 path");

    memset(&looseDirStatus, 0, sizeof(looseDirStatus));
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_ScanGame(&looseDirStatus, looseDir, "theron");
    looseDirMetrics = M12_AssetStatus_TestGetScanMetrics();
    check_int(M12_AssetStatus_GameAvailable(&looseDirStatus, "theron") == 1,
              "Theron direct-launch scan accepts arbitrary directory by Track 02 hash");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&looseDirStatus), looseDir) == 0,
              "Theron arbitrary-directory direct scan preserves selected data dir");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&looseDirStatus, "theron"),
                     looseDir) == 0,
              "Theron arbitrary-directory direct scan derives runtime root from matched file");
    version = M12_AssetStatus_GetVersion(&looseDirStatus, "theron", 1U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, looseTrackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "Theron arbitrary-directory direct scan records renamed Track 02 path");
    required = M12_AssetStatus_GetRequiredFile(&looseDirStatus, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, looseTrackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "Theron arbitrary-directory direct scan propagates renamed required marker");
    check_int(looseDirMetrics.rootCount == 0U,
              "Theron arbitrary-directory direct scan avoids generic search roots");
    check_int(looseDirMetrics.requiredHashLookups == 0U,
              "Theron arbitrary-directory direct scan avoids required-file fallback lookups");

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    firstMetrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "initial launcher scan marks synthetic Theron Track 02 available");
    version = M12_AssetStatus_GetVersion(&status, "theron", 1U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, extrasTrackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "initial launcher scan records the verified raw Theron path and hash");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, extrasTrackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "initial launcher scan records the required raw Theron Track 02 marker");
    check_int(firstMetrics.rootCount > 0U,
              "initial launcher scan builds search roots");
    check_int(firstMetrics.versionHashLookups > 0U,
              "initial launcher scan performs version hash lookups");

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    refreshMetrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "repeat launcher refresh keeps Theron available");
    version = M12_AssetStatus_GetVersion(&status, "theron", 1U);
    check_int(version && version->matched &&
                  strcmp(version->matchedPath, extrasTrackPath) == 0 &&
                  strcmp(version->matchedMd5, trackMd5) == 0,
              "repeat launcher refresh reuses the verified raw Theron path and hash");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
    check_int(required && required->matched &&
                  strcmp(required->matchedPath, extrasTrackPath) == 0 &&
                  strcmp(required->matchedHash, trackMd5) == 0,
              "repeat launcher refresh keeps the required raw Theron Track 02 marker");
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
