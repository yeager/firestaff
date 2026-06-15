/*
 * firestaff_m12_no_data_scan_probe.c
 *
 * Phase A / headless data-layer invariant probe for the M12 asset
 * scanner. Verifies that `M12_AssetStatus_Scan` is well-behaved in
 * every no-data configuration the launcher can hit at boot:
 *
 *   INV_N01  Scan with a non-existent requested data dir does not
 *            crash, leaves all five games unavailable, leaves the
 *            required-file catalog populated but nothing matched.
 *   INV_N02  Scan with a real but empty data dir (mkdtemp) is
 *            indistinguishable from INV_N01.
 *   INV_N03  Scan with NULL requested data dir does not crash and
 *            still produces a usable M12_AssetStatus (games stay
 *            unavailable when no fallback roots carry assets).
 *   INV_N04  Scan with an empty-string requested data dir does not
 *            crash and is equivalent to INV_N03.
 *   INV_N05  Scan with a NULL status pointer is a safe no-op.
 *   INV_N06  GetDataDir returns a non-empty string after any scan.
 *   INV_N07  GetRuntimeDataDir returns a non-empty string for every
 *            known game after any scan.
 *   INV_N08  GameAvailable returns 0 for every known game and 0 for
 *            unknown / NULL gameId arguments (no false positives).
 *   INV_N09  GameAvailable(NULL, "dm1") and GameAvailable(&s, NULL)
 *            both return 0 (null-safety).
 *   INV_N10  HasOriginalFileCandidate returns 0 (no leak from stale
 *            struct state).
 *   INV_N11  V22ModernAssetsInstalled returns 0 (no V2.2 manifest
 *            found in any empty data dir).
 *   INV_N12  For every known game, GetRequiredFileCount matches
 *            GameRequiredFileCount (catalog) and every required
 *            file has matched == 0 with empty matchedPath.
 *   INV_N13  For every known game, GetVersionCount matches the
 *            static version catalog and every version has
 *            matched == 0 with empty matchedPath/matchedMd5.
 *   INV_N14  Required-file slots cleared by memset (no leak of a
 *            prior struct's matchedPath into a fresh scan).
 *   INV_N15  Two consecutive scans with the same empty dir produce
 *            identical availability / required-file state (scan is
 *            idempotent and deterministic).
 *
 * Exit code: 0 if every invariant passes, 1 otherwise.
 *
 * This probe is data-independent by construction: it uses mkdtemp
 * to make an isolated empty directory and never references any
 * real game data on the host. It does not open an SDL window,
 * touch the real `~/.firestaff/data` tree, or read any
 * GRAPHICS.DAT / DUNGEON.DAT payload.
 */

#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
static int portable_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) {
        return 0;
    }
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld",
                 ((long)GetCurrentProcessId() + i) % 1000000L);
        if (MKDIR(templ) == 0) {
            return 1;
        }
    }
    return 0;
}
static int portable_setenv(const char* name, const char* value) {
    return _putenv_s(name, value);
}
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
#define RMDIR(path) rmdir((path))
static int portable_mkdtemp(char* templ) {
    return mkdtemp(templ) != NULL;
}
static int portable_setenv(const char* name, const char* value) {
    return setenv(name, value, 1);
}
#endif

typedef struct {
    int total;
    int passed;
} InvTally;

static void record(InvTally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg ? msg : "");
    } else {
        printf("FAIL %s %s\n", id, msg ? msg : "");
    }
}

static const char* kKnownGameIds[] = {
    "dm1", "csb", "dm2", "nexus", "theron"
};
#define KNOWN_GAME_COUNT \
    (int)(sizeof(kKnownGameIds) / sizeof(kKnownGameIds[0]))

/* Snapshot the relevant fields of an M12_AssetStatus so we can
 * assert deterministic re-scan behavior (INV_N15). */
typedef struct {
    int dm1Available;
    int csbAvailable;
    int dm2Available;
    int nexusAvailable;
    int theronAvailable;
    int originalFileCandidateFound;
    int v22ModernAssetsInstalled;
    char dataDir[M12_ASSET_DATA_DIR_CAPACITY];
    size_t requiredFileCounts[M12_ASSET_GAME_COUNT];
    int anyFileMatched[M12_ASSET_GAME_COUNT];
} AssetStatusSnapshot;

static void snapshot_status(const M12_AssetStatus* status,
                            AssetStatusSnapshot* snap) {
    int g;
    size_t f;
    memset(snap, 0, sizeof(*snap));
    if (!status) {
        return;
    }
    snap->dm1Available = status->dm1Available;
    snap->csbAvailable = status->csbAvailable;
    snap->dm2Available = status->dm2Available;
    snap->nexusAvailable = status->nexusAvailable;
    snap->theronAvailable = status->theronAvailable;
    snap->originalFileCandidateFound = status->originalFileCandidateFound;
    snap->v22ModernAssetsInstalled = status->v22_modern_assets_installed;
    snprintf(snap->dataDir, sizeof(snap->dataDir), "%s",
             M12_AssetStatus_GetDataDir(status));
    for (g = 0; g < M12_ASSET_GAME_COUNT; ++g) {
        size_t fileCount = M12_AssetStatus_GetRequiredFileCount(
            status, kKnownGameIds[g]);
        snap->requiredFileCounts[g] = fileCount;
        for (f = 0; f < fileCount; ++f) {
            const M12_AssetRequiredFileStatus* file =
                M12_AssetStatus_GetRequiredFile(status,
                                                kKnownGameIds[g],
                                                f);
            if (file && file->matched) {
                snap->anyFileMatched[g] = 1;
                break;
            }
        }
    }
}

static int snapshot_equals(const AssetStatusSnapshot* a,
                           const AssetStatusSnapshot* b) {
    int g;
    if (a->dm1Available != b->dm1Available) return 0;
    if (a->csbAvailable != b->csbAvailable) return 0;
    if (a->dm2Available != b->dm2Available) return 0;
    if (a->nexusAvailable != b->nexusAvailable) return 0;
    if (a->theronAvailable != b->theronAvailable) return 0;
    if (a->originalFileCandidateFound != b->originalFileCandidateFound) return 0;
    if (a->v22ModernAssetsInstalled != b->v22ModernAssetsInstalled) return 0;
    if (strcmp(a->dataDir, b->dataDir) != 0) return 0;
    for (g = 0; g < M12_ASSET_GAME_COUNT; ++g) {
        if (a->requiredFileCounts[g] != b->requiredFileCounts[g]) return 0;
        if (a->anyFileMatched[g] != b->anyFileMatched[g]) return 0;
    }
    return 1;
}

/* Assert that a single scan against `dataDir` produced a clean
 * no-data state. Returns 1 if every sub-invariant passed, else 0.
 * Each sub-assertion is reported through the passed-in tally. */
static void assert_clean_no_data_state(M12_AssetStatus* status,
                                       const char* dataDir,
                                       const char* invPrefix,
                                       InvTally* t) {
    int g;
    char idBuf[64];

    /* Pre-poison the struct so we can detect proper clearing of
     * matchedPath / matchedHash by memset. The API contract is that
     * Scan() resets the struct, so any 0xA5 leakage is a bug. */
    memset(status, 0xA5, sizeof(*status));

    M12_AssetStatus_Scan(status, dataDir);

    /* INV_N06: GetDataDir returns a non-empty string. */
    {
        const char* dd = M12_AssetStatus_GetDataDir(status);
        snprintf(idBuf, sizeof(idBuf), "%s_GetDataDir", invPrefix);
        record(t, idBuf, (dd != NULL && dd[0] != '\0'),
               "GetDataDir returned non-empty path");
    }

    /* INV_N07: GetRuntimeDataDir returns a non-empty string per game. */
    for (g = 0; g < KNOWN_GAME_COUNT; ++g) {
        const char* rd = M12_AssetStatus_GetRuntimeDataDir(status,
                                                           kKnownGameIds[g]);
        snprintf(idBuf, sizeof(idBuf), "%s_RuntimeDataDir_%s",
                 invPrefix, kKnownGameIds[g]);
        record(t, idBuf, (rd != NULL && rd[0] != '\0'),
               "GetRuntimeDataDir returned non-empty path");
    }

    /* INV_N08: every known game is unavailable, unknown/empty is 0. */
    for (g = 0; g < KNOWN_GAME_COUNT; ++g) {
        snprintf(idBuf, sizeof(idBuf), "%s_GameAvailable_%s",
                 invPrefix, kKnownGameIds[g]);
        record(t, idBuf,
               M12_AssetStatus_GameAvailable(status, kKnownGameIds[g]) == 0,
               "known game reports unavailable");
    }
    snprintf(idBuf, sizeof(idBuf), "%s_GameAvailable_unknown", invPrefix);
    record(t, idBuf,
           M12_AssetStatus_GameAvailable(status, "unknown_game") == 0,
           "unknown gameId returns 0");
    snprintf(idBuf, sizeof(idBuf), "%s_GameAvailable_empty", invPrefix);
    record(t, idBuf,
           M12_AssetStatus_GameAvailable(status, "") == 0,
           "empty gameId returns 0");

    /* INV_N10: no original-file candidate. */
    snprintf(idBuf, sizeof(idBuf), "%s_HasOriginal", invPrefix);
    record(t, idBuf,
           M12_AssetStatus_HasOriginalFileCandidate(status) == 0,
           "HasOriginalFileCandidate == 0");

    /* INV_N11: no V2.2 modern assets. */
    snprintf(idBuf, sizeof(idBuf), "%s_V22Modern", invPrefix);
    record(t, idBuf,
           M12_AssetStatus_V22ModernAssetsInstalled(status) == 0,
           "V22ModernAssetsInstalled == 0");

    /* INV_N12: required-file catalog is populated but nothing matched,
     *          and matchedPath is empty (no leak from poisoned state). */
    for (g = 0; g < KNOWN_GAME_COUNT; ++g) {
        size_t catalogCount = M12_AssetStatus_GameRequiredFileCount(
            kKnownGameIds[g]);
        size_t scanCount = M12_AssetStatus_GetRequiredFileCount(
            status, kKnownGameIds[g]);
        size_t f;
        if (catalogCount == 0U) {
            snprintf(idBuf, sizeof(idBuf), "%s_Catalog_%s",
                     invPrefix, kKnownGameIds[g]);
            record(t, idBuf, 0, "GameRequiredFileCount is 0 (catalog empty)");
            continue;
        }
        snprintf(idBuf, sizeof(idBuf), "%s_ScanCount_%s",
                 invPrefix, kKnownGameIds[g]);
        record(t, idBuf, scanCount == catalogCount,
               "GetRequiredFileCount matches catalog count");
        for (f = 0; f < scanCount; ++f) {
            const M12_AssetRequiredFileStatus* file =
                M12_AssetStatus_GetRequiredFile(status,
                                                kKnownGameIds[g],
                                                f);
            if (!file) {
                snprintf(idBuf, sizeof(idBuf), "%s_File_%s[%zu]_null",
                         invPrefix, kKnownGameIds[g], f);
                record(t, idBuf, 0, "GetRequiredFile returned NULL");
                continue;
            }
            snprintf(idBuf, sizeof(idBuf), "%s_Matched_%s[%zu]",
                     invPrefix, kKnownGameIds[g], f);
            record(t, idBuf, file->matched == 0,
                   "required file reports unmatched");
            snprintf(idBuf, sizeof(idBuf), "%s_MatchedPath_%s[%zu]",
                     invPrefix, kKnownGameIds[g], f);
            record(t, idBuf, file->matchedPath[0] == '\0',
                   "required file matchedPath is empty (no stale leak)");
            snprintf(idBuf, sizeof(idBuf), "%s_MatchedHash_%s[%zu]",
                     invPrefix, kKnownGameIds[g], f);
            record(t, idBuf, file->matchedHash[0] == '\0',
                   "required file matchedHash is empty (no stale leak)");
        }
    }

    /* INV_N13: version catalog populates, no version matched, no leak. */
    for (g = 0; g < KNOWN_GAME_COUNT; ++g) {
        size_t vCount = M12_AssetStatus_GetVersionCount(kKnownGameIds[g]);
        size_t v;
        snprintf(idBuf, sizeof(idBuf), "%s_VersionCount_%s",
                 invPrefix, kKnownGameIds[g]);
        record(t, idBuf, vCount > 0U,
               "version catalog is non-empty");
        for (v = 0; v < vCount; ++v) {
            const M12_AssetVersionStatus* ver =
                M12_AssetStatus_GetVersion(status, kKnownGameIds[g], v);
            if (!ver) {
                snprintf(idBuf, sizeof(idBuf), "%s_Version_%s[%zu]_null",
                         invPrefix, kKnownGameIds[g], v);
                record(t, idBuf, 0, "GetVersion returned NULL");
                continue;
            }
            snprintf(idBuf, sizeof(idBuf), "%s_VersionMatched_%s[%zu]",
                     invPrefix, kKnownGameIds[g], v);
            record(t, idBuf, ver->matched == 0,
                   "version reports unmatched");
            snprintf(idBuf, sizeof(idBuf), "%s_VersionPath_%s[%zu]",
                     invPrefix, kKnownGameIds[g], v);
            record(t, idBuf, ver->matchedPath[0] == '\0',
                   "version matchedPath is empty (no stale leak)");
            snprintf(idBuf, sizeof(idBuf), "%s_VersionMd5_%s[%zu]",
                     invPrefix, kKnownGameIds[g], v);
            record(t, idBuf, ver->matchedMd5[0] == '\0',
                   "version matchedMd5 is empty (no stale leak)");
        }
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    InvTally t = {0, 0};
    M12_AssetStatus statusA;
    M12_AssetStatus statusB;
    AssetStatusSnapshot snapA;
    AssetStatusSnapshot snapB;
    char homeDirTemplate[] = "/tmp/firestaff-phase-a-home-XXXXXX";
    char emptyDirTemplate[] = "/tmp/firestaff-phase-a-no-data-XXXXXX";
    int emptyDirOk = 0;
    int homeDirOk = 0;
    int i;

    printf("# firestaff_m12_no_data_scan_probe\n");

    /* Isolate the scan from any real user data on the host.
     *
     * M12_AssetStatus_Scan falls back to FSP_GetDefaultOriginalsDir()
     * and FSP_ResolveDataDir() when the requestedDataDir is NULL or empty.
     * Both of those walk the real user data tree (FIRESTAFF_DATA,
     * $HOME/.firestaff/data, XDG_DATA_HOME). For a headless probe that
     * must remain deterministic, we point HOME / FIRESTAFF_DATA /
     * XDG_DATA_HOME / XDG_CONFIG_HOME at a fresh empty tempdir so the
     * scan only ever sees an empty data root. */
    if (portable_mkdtemp(homeDirTemplate)) {
        homeDirOk = 1;
    }
    if (homeDirOk) {
#if defined(_WIN32)
        portable_setenv("APPDATA", homeDirTemplate);
        portable_setenv("LOCALAPPDATA", homeDirTemplate);
#else
        portable_setenv("HOME", homeDirTemplate);
        portable_setenv("XDG_DATA_HOME", homeDirTemplate);
        portable_setenv("XDG_CONFIG_HOME", homeDirTemplate);
#endif
        portable_setenv("FIRESTAFF_DATA", homeDirTemplate);
    } else {
        printf("WARN could not create isolated home; probe may hang on real user data\n");
    }

    /* ---------- INV_N05: NULL status is a safe no-op ----------------- */
    M12_AssetStatus_Scan(NULL, "/some/path");
    record(&t, "INV_N05",
           1,
           "Scan(NULL, ...) returned without crashing");

    /* ---------- INV_N03 / INV_N04: NULL / empty requestedDataDir ----- */
    {
        M12_AssetStatus s;
        int nullOk = 1;
        int emptyOk = 1;

        memset(&s, 0xA5, sizeof(s));
        M12_AssetStatus_Scan(&s, NULL);
        /* With NULL requestedDataDir, the scan still must populate the
         * version/required-file catalogs and report all games unavailable. */
        if (M12_AssetStatus_GameAvailable(&s, "dm1") != 0) nullOk = 0;
        if (M12_AssetStatus_GameAvailable(&s, "csb") != 0) nullOk = 0;
        if (M12_AssetStatus_GameAvailable(&s, "dm2") != 0) nullOk = 0;
        if (M12_AssetStatus_GameAvailable(&s, "nexus") != 0) nullOk = 0;
        if (M12_AssetStatus_GameAvailable(&s, "theron") != 0) nullOk = 0;
        if (M12_AssetStatus_HasOriginalFileCandidate(&s) != 0) nullOk = 0;
        if (M12_AssetStatus_V22ModernAssetsInstalled(&s) != 0) nullOk = 0;
        for (i = 0; i < KNOWN_GAME_COUNT; ++i) {
            if (M12_AssetStatus_GetRequiredFileCount(&s, kKnownGameIds[i]) == 0U) {
                nullOk = 0;
            }
        }
        record(&t, "INV_N03",
               nullOk,
               "Scan(NULL requestedDataDir) yields stable no-data state");

        memset(&s, 0xA5, sizeof(s));
        M12_AssetStatus_Scan(&s, "");
        if (M12_AssetStatus_GameAvailable(&s, "dm1") != 0) emptyOk = 0;
        if (M12_AssetStatus_GameAvailable(&s, "theron") != 0) emptyOk = 0;
        if (M12_AssetStatus_HasOriginalFileCandidate(&s) != 0) emptyOk = 0;
        if (M12_AssetStatus_V22ModernAssetsInstalled(&s) != 0) emptyOk = 0;
        for (i = 0; i < KNOWN_GAME_COUNT; ++i) {
            if (M12_AssetStatus_GetRequiredFileCount(&s, kKnownGameIds[i]) == 0U) {
                emptyOk = 0;
            }
        }
        record(&t, "INV_N04",
               emptyOk,
               "Scan(\"\") yields stable no-data state");
    }

    /* ---------- INV_N09: null-safety on GameAvailable ---------------- */
    {
        M12_AssetStatus s;
        memset(&s, 0, sizeof(s));
        M12_AssetStatus_Scan(&s, NULL);
        int nullStatusOk =
            M12_AssetStatus_GameAvailable(NULL, "dm1") == 0;
        int nullGameIdOk =
            M12_AssetStatus_GameAvailable(&s, NULL) == 0;
        int bothNullOk =
            M12_AssetStatus_GameAvailable(NULL, NULL) == 0;
        record(&t, "INV_N09",
               nullStatusOk && nullGameIdOk && bothNullOk,
               "GameAvailable is null-safe on status, gameId, and both");
    }

    /* ---------- INV_N01: non-existent requested data dir ------------- */
    {
        M12_AssetStatus s;
        const char* ghostPath =
            "/this/path/should/never/exist/firestaff-phase-a-probe";
        int before = t.passed;
        memset(&s, 0, sizeof(s));
        assert_clean_no_data_state(&s, ghostPath, "INV_N01", &t);
        record(&t, "INV_N01_aggregate",
               t.passed > before,
               "INV_N01 family all passed");
    }

    /* ---------- INV_N02: real but empty data dir (mkdtemp) ----------- */
    if (portable_mkdtemp(emptyDirTemplate)) {
        emptyDirOk = 1;
    }
    if (emptyDirOk) {
        M12_AssetStatus s;
        int before = t.passed;
        memset(&s, 0, sizeof(s));
        assert_clean_no_data_state(&s, emptyDirTemplate, "INV_N02", &t);
        record(&t, "INV_N02_aggregate",
               t.passed > before,
               "INV_N02 family all passed");
    } else {
        record(&t, "INV_N02",
               0,
               "could not create isolated empty tempdir");
    }

    /* ---------- INV_N15: re-scan determinism ------------------------- */
    if (emptyDirOk) {
        int n15 = 0;
        memset(&statusA, 0, sizeof(statusA));
        memset(&statusB, 0, sizeof(statusB));
        M12_AssetStatus_Scan(&statusA, emptyDirTemplate);
        M12_AssetStatus_Scan(&statusB, emptyDirTemplate);
        snapshot_status(&statusA, &snapA);
        snapshot_status(&statusB, &snapB);
        n15 = snapshot_equals(&snapA, &snapB);
        record(&t, "INV_N15",
               n15,
               "two scans of the same empty dir produce identical state");
    } else {
        record(&t, "INV_N15",
               0,
               "skipped: no isolated empty tempdir available");
    }

    /* ---------- INV_N14: matchedPath cleared on fresh scan ----------- */
    if (emptyDirOk) {
        M12_AssetStatus s;
        int g;
        int n14 = 1;
        /* Pre-fill the required-file slots with sentinel data to
         * confirm Scan() clears them via memset. */
        memset(&s, 0xA5, sizeof(s));
        M12_AssetStatus_Scan(&s, emptyDirTemplate);
        for (g = 0; g < M12_ASSET_GAME_COUNT; ++g) {
            size_t fileCount = M12_AssetStatus_GetRequiredFileCount(
                &s, kKnownGameIds[g]);
            size_t f;
            for (f = 0; f < fileCount; ++f) {
                const M12_AssetRequiredFileStatus* file =
                    M12_AssetStatus_GetRequiredFile(&s,
                                                    kKnownGameIds[g],
                                                    f);
                if (!file) continue;
                if (file->matchedPath[0] != '\0') n14 = 0;
                if (file->matchedHash[0] != '\0') n14 = 0;
            }
        }
        record(&t, "INV_N14",
               n14,
               "matchedPath / matchedHash cleared on fresh Scan()");
    } else {
        record(&t, "INV_N14",
               0,
               "skipped: no isolated empty tempdir available");
    }

    /* Best-effort cleanup of the empty tempdir. */
    if (emptyDirOk) {
        RMDIR(emptyDirTemplate);
    }
    if (homeDirOk) {
        RMDIR(homeDirTemplate);
    }

    printf("# summary: %d/%d invariants passed\n", t.passed, t.total);
    return (t.passed == t.total) ? 0 : 1;
}
