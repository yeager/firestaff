/*
 * test_asset_status_data_dir_change_cache_invalidation.c
 *
 * Regression gate for M12 launcher cache invalidation when the
 * configured data root changes.
 *
 * The launcher keeps a single `M12_AssetStatus` struct on
 * `M12_StartupMenuState` and re-uses it across the whole session.
 * When the user picks a new folder in the data-directory picker,
 * `M12_StartupMenu_SetDataDirectory` (src/ui/menu_startup_m12.c:1291)
 * calls `M12_AssetStatus_Scan(&state->assetStatus, dataDir)` against
 * the same struct instance.
 *
 * The whole point of the scan-with-existing-struct flow is to
 * *invalidate* whatever the previous scan produced. If the new
 * directory has different (or no) game data, the previous
 * availability flags, matched paths, matched hashes, and runtime
 * cache directories must all be cleared — never re-published.
 *
 * This test exercises:
 *   1. Scan in dir A (Theron Track 02 fixture) → Theron available.
 *   2. Scan in dir B (no game data)         → Theron availability,
 *      version matched flags, required-file matched flags, matched
 *      paths, and matched hashes are all cleared; dataDir reflects B.
 *   3. Scan in dir A again                   → Theron available again,
 *      matchedPath points at the new dir A fixture (not a stale
 *      reference from before the dir-B scan).
 *   4. Scan in dir C (different Theron
 *      payload, different MD5)               → Theron available, hash
 *      reflects C, not A; reusableTheronRefreshes must NOT have fired
 *      because the data dir changed.
 *   5. Required-file + version flags are
 *      not falsely "stale-true" after dir-B scan for ANY of the
 *      five games (DM1/CSB/DM2/Nexus/Theron).
 *   6. legacyFallbackDir is refreshed, not retained from the prior
 *      scan when the env-driven resolution changes.
 *
 * Source-lock: src/shared/asset_status_m12.c (M12_AssetStatus_Scan,
 *   m12_scan_direct_theron_request, m12_scan_explicit_file_request,
 *   m12_scan_original_candidates, m12_reuse_verified_theron_refresh,
 *   m12_materialize_runtime_cache_for_game), include/asset_status_m12.h,
 *   ReDMCSB launcher/scan reference behavior (THERON.ASM / DATA.C:359-360).
 *
 * This is a synthetic data-dir invalidation contract test. It does
 * NOT load real game data, run DOSBox, capture pixels, or claim any
 * parity. The synthetic Theron Track 02 hash is registered via
 * `M12_AssetStatus_TestSetTheronSyntheticHash` (testing-only API).
 */

#define FIRESTAFF_ASSET_STATUS_TESTING 1
#include "asset_status_m12.h"
#include "config_m12.h"
#include "fs_portable_compat.h"
#include "menu_startup_m12.h"

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

static int g_failures = 0;
static int g_assertions = 0;

static void check_int(int condition, const char* message) {
    ++g_assertions;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_data_dir_change_inval_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return TEST_MKDIR(out) == 0;
#else
    char templatePath[] = "/tmp/firestaff-data-dir-change-inval-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int write_text(const char* path, const char* text) {
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

static int write_joined(const char* root,
                        const char* relative,
                        const char* text) {
    char path[M12_ASSET_DATA_DIR_CAPACITY];
    if (!FSP_JoinPath(path, sizeof(path), root, relative)) {
        return 0;
    }
    return write_text(path, text);
}

static int setup_theron_dir(const char* root,
                            const char* trackRel,
                            const char* trackPayload,
                            char* outPath, size_t outPathSize,
                            char* outMd5, size_t outMd5Size) {
    if (!FSP_JoinPath(outPath, outPathSize, root, trackRel)) {
        return 0;
    }
    if (!write_text(outPath, trackPayload ? trackPayload : "")) {
        return 0;
    }
    if (!m12_file_md5_hex(outPath, outMd5)) {
        return 0;
    }
    outMd5[outMd5Size - 1U] = '\0';
    return 1;
}

static int setup_dm1_recommended_dir(const char* root,
                                     char* outGraphicsPath,
                                     size_t outGraphicsPathSize,
                                     char* outGraphicsMd5,
                                     char* outDungeonPath,
                                     size_t outDungeonPathSize,
                                     char* outDungeonMd5) {
    char dm1Dir[M12_ASSET_DATA_DIR_CAPACITY];
    static const char graphicsPayload[] =
        "Firestaff synthetic DM1 GRAPHICS.DAT fixture for recommended layout\n";
    static const char dungeonPayload[] =
        "Firestaff synthetic DM1 DUNGEON.DAT fixture for recommended layout\n";
    if (!FSP_JoinPath(dm1Dir, sizeof(dm1Dir), root, "dm1") ||
        !FSP_CreateDirectoryRecursive(dm1Dir) ||
        !FSP_JoinPath(outGraphicsPath, outGraphicsPathSize, dm1Dir, "GRAPHICS.DAT") ||
        !FSP_JoinPath(outDungeonPath, outDungeonPathSize, dm1Dir, "DUNGEON.DAT") ||
        !write_text(outGraphicsPath, graphicsPayload) ||
        !write_text(outDungeonPath, dungeonPayload) ||
        !m12_file_md5_hex(outGraphicsPath, outGraphicsMd5) ||
        !m12_file_md5_hex(outDungeonPath, outDungeonMd5)) {
        return 0;
    }
    return 1;
}

static int setup_nexus_recommended_dir(const char* root,
                                       char* outDataPath,
                                       size_t outDataPathSize,
                                       char* outDataMd5,
                                       size_t outDataMd5Size) {
    char nexusDir[M12_ASSET_DATA_DIR_CAPACITY];
    static const char dataPayload[] =
        "Firestaff synthetic Nexus DM.BIN fixture for recommended layout\n";
    if (!FSP_JoinPath(nexusDir, sizeof(nexusDir), root, "nexus") ||
        !FSP_CreateDirectoryRecursive(nexusDir) ||
        !FSP_JoinPath(outDataPath, outDataPathSize, nexusDir, "DM.BIN") ||
        !write_text(outDataPath, dataPayload) ||
        !m12_file_md5_hex(outDataPath, outDataMd5)) {
        return 0;
    }
    outDataMd5[outDataMd5Size - 1U] = '\0';
    return 1;
}

static void check_no_game_available(const M12_AssetStatus* status) {
    static const char* const gameIds[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    size_t gi;
    size_t vi;
    size_t ri;
    for (gi = 0U; gi < sizeof(gameIds) / sizeof(gameIds[0]); ++gi) {
        const char* gameId = gameIds[gi];
        size_t versionCount = M12_AssetStatus_GetVersionCount(gameId);
        size_t requiredCount = M12_AssetStatus_GetRequiredFileCount(status, gameId);

        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s availability must be cleared after empty-dir scan",
                 gameId);
        check_int(M12_AssetStatus_GameAvailable(status, gameId) == 0, msg);

        for (vi = 0U; vi < versionCount; ++vi) {
            const M12_AssetVersionStatus* v =
                M12_AssetStatus_GetVersion(status, gameId, vi);
            snprintf(msg, sizeof(msg),
                     "%s version %lu matched must clear after empty-dir scan",
                     gameId, (unsigned long)vi);
            check_int(v && v->matched == 0, msg);
            snprintf(msg, sizeof(msg),
                     "%s version %lu matchedPath must clear after empty-dir scan",
                     gameId, (unsigned long)vi);
            check_int(v && v->matchedPath[0] == '\0', msg);
            snprintf(msg, sizeof(msg),
                     "%s version %lu matchedMd5 must clear after empty-dir scan",
                     gameId, (unsigned long)vi);
            check_int(v && v->matchedMd5[0] == '\0', msg);
        }
        for (ri = 0U; ri < requiredCount; ++ri) {
            const M12_AssetRequiredFileStatus* r =
                M12_AssetStatus_GetRequiredFile(status, gameId, ri);
            snprintf(msg, sizeof(msg),
                     "%s required %lu matched must clear after empty-dir scan",
                     gameId, (unsigned long)ri);
            check_int(r && r->matched == 0, msg);
            snprintf(msg, sizeof(msg),
                     "%s required %lu matchedPath must clear after empty-dir scan",
                     gameId, (unsigned long)ri);
            check_int(r && r->matchedPath[0] == '\0', msg);
            snprintf(msg, sizeof(msg),
                     "%s required %lu matchedHash must clear after empty-dir scan",
                     gameId, (unsigned long)ri);
            check_int(r && r->matchedHash[0] == '\0', msg);
        }
    }
}

static void check_required_metadata_preserved(const M12_AssetStatus* status) {
    /* Even on a totally empty scan the M12 layer must still expose
     * the required-file metadata catalog so the launcher can render
     * the missing-data popups. The metadata flags (required=1) and
     * counts must not be cleared by an empty scan. */
    static const char* const gameIds[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    size_t gi;
    size_t ri;
    for (gi = 0U; gi < sizeof(gameIds) / sizeof(gameIds[0]); ++gi) {
        const char* gameId = gameIds[gi];
        size_t requiredCount = M12_AssetStatus_GetRequiredFileCount(status, gameId);
        size_t expectedRequired = M12_AssetStatus_GameRequiredFileCount(gameId);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s required-file catalog must keep count %lu",
                 gameId, (unsigned long)expectedRequired);
        check_int(requiredCount == expectedRequired, msg);
        for (ri = 0U; ri < requiredCount; ++ri) {
            const M12_AssetRequiredFileStatus* r =
                M12_AssetStatus_GetRequiredFile(status, gameId, ri);
            snprintf(msg, sizeof(msg),
                     "%s required %lu metadata must still carry required=1",
                     gameId, (unsigned long)ri);
            check_int(r && r->required == 1, msg);
            snprintf(msg, sizeof(msg),
                     "%s required %lu metadata must keep roleId label",
                     gameId, (unsigned long)ri);
            check_int(r && r->roleId && r->roleId[0] != '\0', msg);
        }
    }
}

static void scan_in_isolated_env(const char* homeRoot,
                                 const char* dataRoot,
                                 M12_AssetStatus* status) {
    /* Reset env per call so the scan sees exactly the dataRoot we
     * want, regardless of what other tests may have leaked. */
    test_setenv("HOME", homeRoot);
    test_setenv("FIRESTAFF_DATA", dataRoot);
    test_setenv("XDG_DATA_HOME", homeRoot);
    test_setenv("APPDATA", homeRoot);
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(status, dataRoot);
}

typedef struct {
    size_t callbacks;
    size_t cancelAtStep;
    char lastTask[64];
    char lastGame[16];
} CancelScanProbe;

static int cancel_scan_progress_cb(const M12_AssetScanProgress* progress,
                                   void* userData) {
    CancelScanProbe* probe = (CancelScanProbe*)userData;
    if (!probe || !progress) {
        return 1;
    }
    ++probe->callbacks;
    snprintf(probe->lastTask, sizeof(probe->lastTask), "%s",
             progress->currentTask);
    snprintf(probe->lastGame, sizeof(probe->lastGame), "%s",
             progress->currentGameId);
    if (probe->cancelAtStep > 0U &&
        progress->completedSteps >= probe->cancelAtStep) {
        return 0;
    }
    return 1;
}

static void check_scan_progress_cancel_contract(const char* homeRoot,
                                                const char* dataRoot) {
    M12_AssetStatus status;
    M12_AssetStatusScanOptions options;
    const M12_AssetScanProgress* progress;
    CancelScanProbe probe;
    int rc;

    memset(&status, 0, sizeof(status));
    memset(&options, 0, sizeof(options));
    memset(&probe, 0, sizeof(probe));
    probe.cancelAtStep = 4U;
    options.progressFn = cancel_scan_progress_cb;
    options.progressUserData = &probe;

    test_setenv("HOME", homeRoot);
    test_setenv("FIRESTAFF_DATA", dataRoot);
    test_setenv("XDG_DATA_HOME", homeRoot);
    test_setenv("APPDATA", homeRoot);
    M12_AssetStatus_TestResetScanMetrics();
    rc = M12_AssetStatus_ScanWithOptions(&status, dataRoot, &options);
    progress = M12_AssetStatus_GetScanProgress(&status);

    check_int(rc == 0, "progress cancel: ScanWithOptions must report cancel");
    check_int(progress && progress->cancelled == 1,
              "progress cancel: status must publish cancelled=1");
    check_int(progress && progress->complete == 0,
              "progress cancel: cancelled scan must not publish complete=1");
    check_int(progress && progress->active == 0,
              "progress cancel: cancelled scan must not remain active");
    check_int(probe.callbacks >= 1U,
              "progress cancel: callback must be invoked before cancel");
    check_no_game_available(&status);
    check_required_metadata_preserved(&status);
}

static void check_scan_progress_complete_contract(const char* homeRoot,
                                                  const char* dataRoot) {
    M12_AssetStatus status;
    M12_AssetStatusScanOptions options;
    const M12_AssetScanProgress* progress;
    CancelScanProbe probe;
    int rc;

    memset(&status, 0, sizeof(status));
    memset(&options, 0, sizeof(options));
    memset(&probe, 0, sizeof(probe));
    options.progressFn = cancel_scan_progress_cb;
    options.progressUserData = &probe;

    test_setenv("HOME", homeRoot);
    test_setenv("FIRESTAFF_DATA", dataRoot);
    test_setenv("XDG_DATA_HOME", homeRoot);
    test_setenv("APPDATA", homeRoot);
    M12_AssetStatus_TestResetScanMetrics();
    rc = M12_AssetStatus_ScanWithOptions(&status, dataRoot, &options);
    progress = M12_AssetStatus_GetScanProgress(&status);

    check_int(rc == 1, "progress complete: ScanWithOptions must report success");
    check_int(progress && progress->complete == 1,
              "progress complete: status must publish complete=1");
    check_int(progress && progress->cancelled == 0,
              "progress complete: status must not publish cancelled=1");
    check_int(progress && progress->active == 0,
              "progress complete: scan must no longer be active");
    check_int(progress && progress->completedSteps == progress->totalSteps,
              "progress complete: completed steps must reach total steps");
    check_int(probe.callbacks >= progress->totalSteps,
              "progress complete: callback must cover coarse scan steps");
}

static void check_first_scan_then_empty_scan(const char* homeRoot,
                                             const char* dirA,
                                             const char* dirB) {
    static const char trackPayloadA[] =
        "Firestaff synthetic Theron dir-A Track 02 fixture v1\n";
    char trackA[M12_ASSET_DATA_DIR_CAPACITY];
    char trackAmd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics emptyMetrics;

    memset(&status, 0, sizeof(status));
    if (!setup_theron_dir(dirA, "track02.bin", trackPayloadA,
                           trackA, sizeof(trackA),
                           trackAmd5, sizeof(trackAmd5))) {
        fprintf(stderr, "FAIL: cannot seed dir-A Track 02 fixture\n");
        ++g_failures;
        return;
    }
    M12_AssetStatus_TestSetTheronSyntheticHash(trackAmd5);

    /* 1) Initial scan in dir A → Theron available, path = trackA. */
    scan_in_isolated_env(homeRoot, dirA, &status);
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "dir-A scan: Theron must be available");
    {
        const M12_AssetVersionStatus* version =
            M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
        check_int(version && version->matched == 1 &&
                  strcmp(version->matchedPath, trackA) == 0 &&
                  strcmp(version->matchedMd5, trackAmd5) == 0,
              "dir-A scan: Theron version matched path + hash must reflect dir A");
    }
    {
        size_t requiredCount = M12_AssetStatus_GetRequiredFileCount(&status, "theron");
        size_t ri;
        int anyMatched = 0;
        for (ri = 0U; ri < requiredCount; ++ri) {
            const M12_AssetRequiredFileStatus* r =
                M12_AssetStatus_GetRequiredFile(&status, "theron", ri);
            if (r && r->matched) {
                anyMatched = 1;
                check_int(strcmp(r->matchedPath, trackA) == 0 &&
                          strcmp(r->matchedHash, trackAmd5) == 0,
                          "dir-A scan: Theron required marker path + hash must reflect dir A");
                break;
            }
        }
        check_int(anyMatched == 1,
                  "dir-A scan: at least one Theron required file must be matched");
    }
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), dirA) == 0,
              "dir-A scan: status->dataDir must equal dir A");

    /* 2) Switch to dir B (empty). Every Theron availability signal
     *    must be invalidated; dataDir must flip to dir B. */
    scan_in_isolated_env(homeRoot, dirB, &status);
    emptyMetrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
              "dir-B scan must invalidate Theron availability");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), dirB) == 0,
              "dir-B scan must update status->dataDir to dir B");
    check_no_game_available(&status);
    check_required_metadata_preserved(&status);

    /* The dir-B scan must run a real scan (rootCount > 0,
     * versionHashLookups > 0). It must NOT use the reusable Theron
     * refresh short-circuit, because the data dir changed. */
    check_int(emptyMetrics.rootCount > 0U,
              "dir-B scan must build search roots (not short-circuit)");
    check_int(emptyMetrics.versionHashLookups > 0U,
              "dir-B scan must perform version hash lookups");
    check_int(emptyMetrics.reusableTheronRefreshes == 0U,
              "dir-B scan must NOT use the reusable Theron refresh gate "
              "because the data dir changed");
}

static void check_round_trip_resolves_new_path(const char* homeRoot,
                                               const char* dirA,
                                               const char* dirB) {
    static const char trackPayloadA[] =
        "Firestaff synthetic Theron dir-A round-trip Track 02 fixture v1\n";
    char trackA[M12_ASSET_DATA_DIR_CAPACITY];
    char trackAmd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* required;
    size_t requiredCount;
    size_t ri;
    int anyMatched = 0;

    if (!setup_theron_dir(dirA, "track02.bin", trackPayloadA,
                           trackA, sizeof(trackA),
                           trackAmd5, sizeof(trackAmd5))) {
        fprintf(stderr, "FAIL: cannot seed round-trip dir-A fixture\n");
        ++g_failures;
        return;
    }
    M12_AssetStatus_TestSetTheronSyntheticHash(trackAmd5);

    memset(&status, 0, sizeof(status));
    scan_in_isolated_env(homeRoot, dirA, &status);
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "round-trip: dir-A scan must mark Theron available");

    /* Cross to dir B (empty) — availability, paths, hashes all gone. */
    scan_in_isolated_env(homeRoot, dirB, &status);
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
              "round-trip: dir-B scan must clear Theron availability");

    /* Cross back to dir A. The matchedPath must point at the live
     * dir-A fixture, NOT a stale value from before the dir-B scan. */
    scan_in_isolated_env(homeRoot, dirA, &status);
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "round-trip: returning to dir A must re-mark Theron available");
    version = M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
    check_int(version && version->matched &&
              strcmp(version->matchedPath, trackA) == 0 &&
              strcmp(version->matchedMd5, trackAmd5) == 0,
              "round-trip: dir-A return must repopulate matched path + md5");
    requiredCount = M12_AssetStatus_GetRequiredFileCount(&status, "theron");
    for (ri = 0U; ri < requiredCount; ++ri) {
        required = M12_AssetStatus_GetRequiredFile(&status, "theron", ri);
        if (required && required->matched) {
            anyMatched = 1;
            check_int(strcmp(required->matchedPath, trackA) == 0 &&
                      strcmp(required->matchedHash, trackAmd5) == 0,
                      "round-trip: dir-A return must repopulate required path + hash");
            break;
        }
    }
    check_int(anyMatched == 1,
              "round-trip: dir-A return must repopulate a Theron required marker");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), dirA) == 0,
              "round-trip: dir-A return must update status->dataDir");
}

static void check_different_payload_different_hash(const char* homeRoot,
                                                   const char* dirA,
                                                   const char* dirC) {
    static const char trackPayloadA[] =
        "Firestaff synthetic Theron dir-A variant-A Track 02 fixture v1\n";
    static const char trackPayloadC[] =
        "Firestaff synthetic Theron dir-C variant-B Track 02 fixture v2\n";
    char trackA[M12_ASSET_DATA_DIR_CAPACITY];
    char trackAmd5[M12_ASSET_MD5_CAPACITY];
    char trackC[M12_ASSET_DATA_DIR_CAPACITY];
    char trackCmd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetVersionStatus* version;
    char msg[200];

    if (!setup_theron_dir(dirA, "track02.bin", trackPayloadA,
                           trackA, sizeof(trackA),
                           trackAmd5, sizeof(trackAmd5)) ||
        !setup_theron_dir(dirC, "track02.bin", trackPayloadC,
                           trackC, sizeof(trackC),
                           trackCmd5, sizeof(trackCmd5))) {
        fprintf(stderr, "FAIL: cannot seed variant-A/variant-C fixtures\n");
        ++g_failures;
        return;
    }
    check_int(strcmp(trackAmd5, trackCmd5) != 0,
              "variant-A and variant-C payloads must produce distinct MD5s");

    /* dir-A scan with variant-A hash registered. */
    M12_AssetStatus_TestSetTheronSyntheticHash(trackAmd5);
    memset(&status, 0, sizeof(status));
    scan_in_isolated_env(homeRoot, dirA, &status);
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "variant scan: dir-A variant-A must match");
    version = M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
    check_int(version && version->matched &&
              strcmp(version->matchedMd5, trackAmd5) == 0,
              "variant scan: dir-A must record variant-A hash");

    /* Switch to dir C and re-register the C hash. The previous
     * variant-A state must be invalidated: the new matched MD5 must
     * be variant-C, the matchedPath must point at dir C, and the
     * reusableTheronRefresh gate must NOT fire because the dir
     * changed. */
    M12_AssetStatus_TestSetTheronSyntheticHash(trackCmd5);
    {
        M12_AssetStatusScanMetrics cMetrics;
        scan_in_isolated_env(homeRoot, dirC, &status);
        cMetrics = M12_AssetStatus_TestGetScanMetrics();

        check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
                  "variant scan: dir-C variant-C must match");
        version = M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
        check_int(version && version->matched, "variant scan: dir-C version matched");
        check_int(strcmp(version->matchedMd5, trackCmd5) == 0,
                  "variant scan: dir-C matched MD5 must reflect variant-C, "
                  "not stale variant-A");
        snprintf(msg, sizeof(msg),
                 "variant scan: dir-C matchedPath must point at dir C (%s)",
                 trackC);
        check_int(strcmp(version->matchedPath, trackC) == 0, msg);
        check_int(cMetrics.reusableTheronRefreshes == 0U,
                  "variant scan: dir-C must NOT use the reusable Theron gate "
                  "because the data dir changed");
    }
}

static void check_legacy_fallback_dir_refreshes(const char* homeRoot,
                                                const char* dirA,
                                                const char* dirB) {
    static const char trackPayloadA[] =
        "Firestaff synthetic Theron legacy-refresh Track 02 fixture v1\n";
    char trackA[M12_ASSET_DATA_DIR_CAPACITY];
    char trackAmd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    const char* legacyA;

    if (!setup_theron_dir(dirA, "track02.bin", trackPayloadA,
                           trackA, sizeof(trackA),
                           trackAmd5, sizeof(trackAmd5))) {
        fprintf(stderr, "FAIL: cannot seed legacy-refresh fixture\n");
        ++g_failures;
        return;
    }
    M12_AssetStatus_TestSetTheronSyntheticHash(trackAmd5);

    memset(&status, 0, sizeof(status));
    scan_in_isolated_env(homeRoot, dirA, &status);
    legacyA = M12_AssetStatus_GetLegacyFallbackDir(&status);

    /* Cross to dir B. Even if FIRESTAFF_DATA is now dir B, the
     * legacy fallback resolution must reflect the env that the new
     * scan was invoked under — not the prior scan's snapshot. */
    scan_in_isolated_env(homeRoot, dirB, &status);
    {
        const char* legacyB = M12_AssetStatus_GetLegacyFallbackDir(&status);
        /* FIRESTAFF_DATA == dir B inside the scan, so FSP_ResolveDataDir
         * (called with NULL requestedDir) still resolves to dir B. The
         * legacy fallback must therefore equal dir B (or "." if it was
         * cleared) — it must NOT still point at dir A. */
        int legacyBIsAcceptable = (legacyB[0] == '\0') ||
                                  (strcmp(legacyB, dirB) == 0);
        check_int(legacyBIsAcceptable,
                  "legacy fallback dir must refresh to current env, "
                  "not retain the prior scan's legacy snapshot");
        if (legacyA[0] != '\0' && strcmp(legacyA, dirB) != 0) {
            check_int(strcmp(legacyB, legacyA) != 0,
                      "legacy fallback dir must not retain a stale "
                      "snapshot from the prior data root");
        }
    }
}

static void check_game_subdir_scan_promotes_to_parent(const char* homeRoot) {
    char dataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char nexusLeaf[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;

    if (!FSP_JoinPath(dataRoot, sizeof(dataRoot), homeRoot, "saved-game-leaf-data") ||
        !FSP_CreateDirectoryRecursive(dataRoot) ||
        !FSP_JoinPath(nexusLeaf, sizeof(nexusLeaf), dataRoot, "nexus") ||
        !FSP_CreateDirectoryRecursive(nexusLeaf) ||
        !setup_dm1_recommended_dir(dataRoot,
                                   graphicsPath, sizeof(graphicsPath), graphicsMd5,
                                   dungeonPath, sizeof(dungeonPath), dungeonMd5)) {
        fprintf(stderr, "FAIL: cannot seed game-subdir promotion fixture\n");
        ++g_failures;
        return;
    }

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(graphicsMd5, dungeonMd5);
    memset(&status, 0, sizeof(status));
    scan_in_isolated_env(homeRoot, nexusLeaf, &status);

    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), dataRoot) == 0,
              "saved game-leaf data_dir must promote to the parent data root");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "saved game-leaf data_dir must still find sibling DM1 data");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"),
                     dataRoot) == 0,
              "promoted game-leaf scan must launch DM1 from the parent root");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
}

#ifndef _WIN32
static void check_symlinked_recommended_dm1_layout(const char* homeRoot) {
    char targetRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char linkRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char expectedGraphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char expectedDungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char expectedDm1Dir[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    const M12_AssetRequiredFileStatus* graphicsRequired = NULL;
    const M12_AssetRequiredFileStatus* dungeonRequired = NULL;
    size_t i;
    size_t count;

    if (!FSP_JoinPath(targetRoot, sizeof(targetRoot), homeRoot, "dm1-symlink-target") ||
        !FSP_CreateDirectoryRecursive(targetRoot) ||
        !FSP_JoinPath(linkRoot, sizeof(linkRoot), homeRoot, "dm1-symlink-data") ||
        !setup_dm1_recommended_dir(targetRoot,
                                   graphicsPath, sizeof(graphicsPath), graphicsMd5,
                                   dungeonPath, sizeof(dungeonPath), dungeonMd5)) {
        fprintf(stderr, "FAIL: cannot seed symlink DM1 recommended-layout fixture\n");
        ++g_failures;
        return;
    }
    unlink(linkRoot);
    if (symlink(targetRoot, linkRoot) != 0) {
        fprintf(stderr, "FAIL: cannot create symlinked DM1 data root\n");
        ++g_failures;
        return;
    }

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(graphicsMd5, dungeonMd5);
    memset(&status, 0, sizeof(status));
    scan_in_isolated_env(homeRoot, linkRoot, &status);
    if (!FSP_JoinPath(expectedDm1Dir, sizeof(expectedDm1Dir), linkRoot, "dm1") ||
        !FSP_JoinPath(expectedGraphicsPath, sizeof(expectedGraphicsPath),
                      expectedDm1Dir, "GRAPHICS.DAT") ||
        !FSP_JoinPath(expectedDungeonPath, sizeof(expectedDungeonPath),
                      expectedDm1Dir, "DUNGEON.DAT")) {
        fprintf(stderr, "FAIL: cannot build expected symlink DM1 paths\n");
        ++g_failures;
        return;
    }

    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "symlink DM1 recommended layout must be available");
    count = M12_AssetStatus_GetRequiredFileCount(&status, "dm1");
    for (i = 0U; i < count; ++i) {
        const M12_AssetRequiredFileStatus* r =
            M12_AssetStatus_GetRequiredFile(&status, "dm1", i);
        if (r && r->roleId && strcmp(r->roleId, "graphics") == 0) {
            graphicsRequired = r;
        } else if (r && r->roleId && strcmp(r->roleId, "dungeon") == 0) {
            dungeonRequired = r;
        }
    }
    check_int(graphicsRequired && graphicsRequired->matched &&
              strcmp(graphicsRequired->matchedPath, expectedGraphicsPath) == 0,
              "symlink DM1 graphics must resolve through data/dm1/GRAPHICS.DAT");
    check_int(dungeonRequired && dungeonRequired->matched &&
              strcmp(dungeonRequired->matchedPath, expectedDungeonPath) == 0,
              "symlink DM1 dungeon must resolve through data/dm1/DUNGEON.DAT");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"), linkRoot) == 0,
              "symlink DM1 runtime root must stay on requested data root");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
}

static void check_symlinked_multi_game_root_scans_all_games(const char* homeRoot) {
    char targetRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char linkRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char nexusPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    char nexusMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;

    if (!FSP_JoinPath(targetRoot, sizeof(targetRoot), homeRoot, "multi-game-symlink-target") ||
        !FSP_CreateDirectoryRecursive(targetRoot) ||
        !FSP_JoinPath(linkRoot, sizeof(linkRoot), homeRoot, "multi-game-symlink-data") ||
        !setup_dm1_recommended_dir(targetRoot,
                                   graphicsPath, sizeof(graphicsPath), graphicsMd5,
                                   dungeonPath, sizeof(dungeonPath), dungeonMd5) ||
        !setup_nexus_recommended_dir(targetRoot,
                                     nexusPath, sizeof(nexusPath),
                                     nexusMd5, sizeof(nexusMd5))) {
        fprintf(stderr, "FAIL: cannot seed symlink multi-game data-root fixture\n");
        ++g_failures;
        return;
    }
    unlink(linkRoot);
    if (symlink(targetRoot, linkRoot) != 0) {
        fprintf(stderr, "FAIL: cannot create symlinked multi-game data root\n");
        ++g_failures;
        return;
    }

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(graphicsMd5, dungeonMd5);
    M12_AssetStatus_TestSetNexusSyntheticHash(nexusMd5);
    memset(&status, 0, sizeof(status));
    scan_in_isolated_env(homeRoot, linkRoot, &status);

    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "symlink broad data root must keep DM1 available");
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 1,
              "symlink broad data root must keep Nexus available");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), linkRoot) == 0,
              "symlink broad data root must stay on requested data root");
    check_int(strcmp(M12_AssetStatus_GetRuntimeDataDir(&status, "dm1"), linkRoot) == 0,
              "symlink broad data root must launch DM1 from requested root");

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
}
#endif

static int any_menu_game_available(const M12_StartupMenuState* menu) {
    return menu &&
           (M12_AssetStatus_GameAvailable(&menu->assetStatus, "dm1") ||
            M12_AssetStatus_GameAvailable(&menu->assetStatus, "csb") ||
            M12_AssetStatus_GameAvailable(&menu->assetStatus, "dm2") ||
            M12_AssetStatus_GameAvailable(&menu->assetStatus, "nexus") ||
            M12_AssetStatus_GameAvailable(&menu->assetStatus, "theron"));
}

static void check_start_menu_heals_stale_config_data_dir(const char* homeRoot) {
    char defaultParent[M12_ASSET_DATA_DIR_CAPACITY];
    char defaultDataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char staleDataRoot[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsPath[M12_ASSET_DATA_DIR_CAPACITY];
    char dungeonPath[M12_ASSET_DATA_DIR_CAPACITY];
    char graphicsMd5[M12_ASSET_MD5_CAPACITY];
    char dungeonMd5[M12_ASSET_MD5_CAPACITY];
    M12_Config config;
    M12_StartupMenuState menu;

    if (!FSP_JoinPath(staleDataRoot, sizeof(staleDataRoot), homeRoot, "stale-empty-data") ||
        !FSP_CreateDirectoryRecursive(staleDataRoot) ||
        !FSP_JoinPath(defaultParent, sizeof(defaultParent), homeRoot, ".firestaff") ||
        !FSP_CreateDirectoryRecursive(defaultParent) ||
        !FSP_JoinPath(defaultDataRoot, sizeof(defaultDataRoot), defaultParent, "data") ||
        !FSP_CreateDirectoryRecursive(defaultDataRoot) ||
        !setup_dm1_recommended_dir(defaultDataRoot,
                                   graphicsPath, sizeof(graphicsPath), graphicsMd5,
                                   dungeonPath, sizeof(dungeonPath), dungeonMd5)) {
        fprintf(stderr, "FAIL: cannot seed start-menu stale-config fixture\n");
        ++g_failures;
        return;
    }

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(graphicsMd5, dungeonMd5);
    (void)test_setenv("HOME", homeRoot);
    (void)test_setenv("FIRESTAFF_DATA", NULL);
#ifndef _WIN32
    (void)test_setenv("XDG_CONFIG_HOME", NULL);
#endif

    M12_Config_SetDefaults(&config);
    snprintf(config.dataDir, sizeof(config.dataDir), "%s", staleDataRoot);
    check_int(M12_Config_Save(&config) == 1,
              "start menu stale-config fixture must save startup config");

    M12_StartupMenu_InitWithDataDir(&menu, NULL, NULL);
    check_int(any_menu_game_available(&menu) == 1,
              "start menu must heal stale config data_dir when default scan finds game data");
    check_int(M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1") == 1,
              "start menu healed scan must expose DM1 availability");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&menu.assetStatus),
                     defaultDataRoot) == 0,
              "start menu healed scan must use the same default root as --scan-data");
    M12_StartupMenu_Destroy(&menu);

    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
}

int main(void) {
    char home[M12_ASSET_DATA_DIR_CAPACITY];
    char dirA[M12_ASSET_DATA_DIR_CAPACITY];
    char dirB[M12_ASSET_DATA_DIR_CAPACITY];
    char dirC[M12_ASSET_DATA_DIR_CAPACITY];

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(dirA, sizeof(dirA), home, "dataA") ||
        !FSP_CreateDirectoryRecursive(dirA) ||
        !FSP_JoinPath(dirB, sizeof(dirB), home, "dataB") ||
        !FSP_CreateDirectoryRecursive(dirB) ||
        !FSP_JoinPath(dirC, sizeof(dirC), home, "dataC") ||
        !FSP_CreateDirectoryRecursive(dirC)) {
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }

    /* Seed dir B with a non-Firestaff README to prove "empty" means
     * "no matched hashes" — not "no files at all". */
    if (!write_joined(dirB, "README.txt", "ordinary user data, not Firestaff")) {
        fprintf(stderr, "could not seed dir-B README\n");
        return 1;
    }

    check_first_scan_then_empty_scan(home, dirA, dirB);
    check_round_trip_resolves_new_path(home, dirA, dirB);
    check_different_payload_different_hash(home, dirA, dirC);
    check_legacy_fallback_dir_refreshes(home, dirA, dirB);
    check_scan_progress_cancel_contract(home, dirB);
    check_scan_progress_complete_contract(home, dirB);
    check_game_subdir_scan_promotes_to_parent(home);
#ifndef _WIN32
    check_symlinked_recommended_dm1_layout(home);
    check_symlinked_multi_game_root_scans_all_games(home);
#endif
    check_start_menu_heals_stale_config_data_dir(home);

    /* Release all test hooks. */
    M12_AssetStatus_TestSetDm1Pc34EnglishSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);
    test_setenv("FIRESTAFF_DATA", NULL);
    test_setenv("XDG_DATA_HOME", NULL);
    test_setenv("APPDATA", NULL);

    if (g_failures) {
        fprintf(stderr, "%d failure(s), assertions=%d\n",
                g_failures, g_assertions);
        return 1;
    }
    printf("ok: data-dir change invalidates stale availability/cache "
           "(%d assertions across A/B/A round-trip, A/C variant swap, "
           "legacy-fallback refresh, all-five-games metadata catalog)\n",
           g_assertions);
    return 0;
}
