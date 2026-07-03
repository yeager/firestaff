/*
 * firestaff_data_dir_picker_platform_matrix_probe.c
 * ==================================================
 *
 * Focused data-free regression for the data-dir picker ↔ platform
 * matrix contract (FIRESTAFF_GAP_LIST section A5 / PLATFORM_MATRIX).
 *
 * The M12 launcher lets the user pick a data root via a folder
 * dialog (m12_begin_data_dir_browse → SDL_ShowOpenFolderDialog →
 * M12_StartupMenu_SetDataDirectory). Whatever folder the user
 * picks, the contract is:
 *
 *   1. `M12_AssetStatus_Scan(status, dir)` walks the folder
 *      *recursively* and matches files by hash, not by filename
 *      or top-level subdir shape.
 *   2. Each game's availability flag
 *      (M12_AssetStatus_GameAvailable) must reflect whether the
 *      recursive scan found that game's required hashes.
 *   3. The scan metrics (rootCount, duplicateRootSkips,
 *      versionHashLookups, requiredHashLookups) must stay
 *      predictable so Tier 1 #5 `--scan-data` smoke reports stay
 *      honest across the platform-matrix sub-layouts.
 *   4. The first-matched version for a game
 *      (M12_AssetStatus_GetFirstMatchedVersion) must remain
 *      deterministic so the launch-blocking code path stays
 *      stable across the documented platform-matrix sub-layouts.
 *   5. Switching to an irrelevant root via re-Scan must clear
 *      availability flags and matched paths — never re-publish
 *      a previous scan's matches.
 *
 * These properties cover the data-dir picker flow without
 * depending on a real binary, real data, or any SDL display.
 *
 * The probe writes synthetic GRAPHICS.DAT / DUNGEON.DAT /
 * DM.BIN / track02.bin files into a tmp data root that mirrors
 * the documented tier1_strict_boot_probe sub-layouts:
 *
 *   root/dm1/...
 *   root/dm1-multilingual/...
 *   root/dm1-extras/legacy-dos/...
 *   root/csb/...
 *   root/csb-extras/legacy-amiga-dms/...
 *   root/dm2/...
 *   root/dm2-extras/dos-en/...
 *   root/dm2-extras/dos-fr/...
 *   root/dm2-extras/pc-fr/...
 *   root/dm2-extras/pc-de/...
 *   root/theron/Theron's Quest (Japan) (Track 02).iso
 *   root/theron-extras/japan/track02.bin
 *   root/theron-extras/usa/track02.bin
 *
 * That mirrors the platform-matrix sub-layout rows documented in
 * docs/PLATFORM_MATRIX.md and probed by tier1_strict_boot_probe,
 * so a regression here proves the data-dir picker preserves the
 * documented platform support surface.
 *
 * Source-lock: docs/FIRESTAFF_GAP_LIST.md section A5
 * ("--scan-data smoke reports real READY-path:er") and
 * docs/PLATFORM_MATRIX.md platform + version support matrix.
 */

#include "asset_status_m12.h"
#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
static int probe_mkdir(const char* path) {
    return _mkdir(path) == 0;
}
static int probe_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <sys/stat.h>
#include <unistd.h>
static int probe_mkdir(const char* path) {
    return mkdir(path, 0700) == 0;
}
static int probe_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int g_failures = 0;

static void check_int(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_isolated_home(char* out, size_t outSize) {
#ifdef _WIN32
    int rc = snprintf(out, outSize, ".\\firestaff_ddpm_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return FSP_CreateDirectoryRecursive(out);
#else
    char templatePath[] = "/tmp/firestaff-ddpm-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

static int write_text_file(const char* path, const char* payload) {
    FILE* fp = fopen(path, "wb");
    size_t len = payload ? strlen(payload) : 0U;
    if (!fp) {
        return 0;
    }
    if (len > 0U && fwrite(payload, 1U, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int ensure_dir(const char* path) {
    if (FSP_PathExists(path)) {
        return FSP_DirExists(path);
    }
    return probe_mkdir(path);
}

static int ensure_nested(const char* root, const char* rel) {
    char path[512];
    if (!FSP_JoinPath(path, sizeof(path), root, rel)) {
        return 0;
    }
    if (FSP_PathExists(path)) {
        return FSP_DirExists(path);
    }
    return FSP_CreateDirectoryRecursive(path);
}

/* Write `payload` to <root>/<rel> and store its MD5 hex. Returns 1 on
 * success, 0 on failure. Pass NULL for outHex if you don't need the
 * hash. */
static int seed_payload(const char* root,
                        const char* rel,
                        const char* payload,
                        char outHex[33]) {
    char path[512];
    if (!FSP_JoinPath(path, sizeof(path), root, rel)) {
        return 0;
    }
    if (!write_text_file(path, payload)) {
        return 0;
    }
    if (outHex) {
        if (!m12_file_md5_hex(path, outHex)) {
            return 0;
        }
    }
    return 1;
}

/* Build the full tier1_strict_boot_probe-style platform-matrix layout
 * inside `root`. Each leaf gets a synthetic payload that we then hash
 * with m12_file_md5_hex, so the synthetic hashes are deterministic
 * across runs but unique enough to drive the recursive scanner.
 *
 * Which rows match the M12 test setters (and therefore drive
 * M12_AssetStatus_GameAvailable):
 *
 *   - `dm1-multilingual/GRAPHICS.DAT` -> TestSetDm1Multilanguage
 *     registers against `dm1 pc34-multi`. The canonical
 *     `dm1/GRAPHICS.DAT` synthetic file is intentionally written
 *     to prove that the data-dir picker does NOT auto-promote a
 *     well-named but unknown-hash file.
 *   - `csb/GRAPHICS.DAT` -> TestSetCsb registers against
 *     `csb pc34-en`.
 *   - `dm2/GRAPHICS.DAT` -> TestSetDm2 registers against
 *     `dm2 pc-en`.
 *   - `theron/Theron's Quest (Japan) (Track 02).iso` ->
 *     TestSetTheron registers against `theron pce-jp`.
 *
 * The remaining extras leaves (`dm1-extras/legacy-dos/...`,
 * `csb-extras/legacy-amiga-dms/...`, `dm2-extras/...`,
 * `theron-extras/...`) are exercised by `asset_find_by_md5`
 * recursive-discovery tests: their hashes can be looked up by
 * hash even if no version row matches them.
 */
typedef struct {
    /* M12-test-setter-registered leaves. */
    char dm1MultiGraphics[33];
    char dm1MultiDungeon[33];
    char csbGraphics[33];
    char csbDungeon[33];
    char dm2Graphics[33];
    char dm2Dungeon[33];
    char theronTrack02[33];
    /* Recursive-discovery-only leaves. */
    char dm1CanonicalGraphics[33];
    char dm1CanonicalDungeon[33];
    char dm1DosGraphics[33];
    char dm1DosDungeon[33];
    char csbAmigaGraphics[33];
    char csbAmigaDungeon[33];
    char dm2DosEnGraphics[33];
    char dm2DosFrGraphics[33];
    char dm2PcFrGraphics[33];
    char dm2PcDeGraphics[33];
    char theronJpTrack02[33];
    char theronUsTrack02[33];
} PlatformMatrixHashes;

static int build_platform_matrix_root(const char* root,
                                      PlatformMatrixHashes* h) {
    if (!ensure_dir(root)) return 0;
    if (!ensure_nested(root, "dm1")) return 0;
    if (!ensure_nested(root, "dm1-multilingual")) return 0;
    if (!ensure_nested(root, "csb")) return 0;
    if (!ensure_nested(root, "dm2")) return 0;
    if (!ensure_nested(root, "theron")) return 0;

    /* DM1 canonical `dm1/GRAPHICS.DAT` is intentionally written
     * even though no test setter registers the `pc34-en` row: the
     * scanner must NOT auto-promote a familiar filename. The file
     * still exists for `asset_find_by_md5` recursive tests. */
    if (!seed_payload(root, "dm1/GRAPHICS.DAT",
                      "platform-matrix DM1 canonical graphics v1\n",
                      h->dm1CanonicalGraphics)) return 0;
    if (!seed_payload(root, "dm1/DUNGEON.DAT",
                      "platform-matrix DM1 canonical dungeon v1\n",
                      h->dm1CanonicalDungeon)) return 0;

    /* DM1 multilingual row — drives DM1-available under the M12
     * test setters. */
    if (!seed_payload(root, "dm1-multilingual/GRAPHICS.DAT",
                      "platform-matrix DM1 multilingual graphics v1\n",
                      h->dm1MultiGraphics)) return 0;
    if (!seed_payload(root, "dm1-multilingual/DUNGEON.DAT",
                      "platform-matrix DM1 multilingual dungeon v1\n",
                      h->dm1MultiDungeon)) return 0;

    /* DM1 DOS legacy-extras (M11 hash-fallback). */
    if (!ensure_nested(root, "dm1-extras/legacy-dos")) return 0;
    if (!seed_payload(root, "dm1-extras/legacy-dos/GRAPHICS.DAT",
                      "platform-matrix DM1 legacy-dos graphics v1\n",
                      h->dm1DosGraphics)) return 0;
    if (!seed_payload(root, "dm1-extras/legacy-dos/DUNGEON.DAT",
                      "platform-matrix DM1 legacy-dos dungeon v1\n",
                      h->dm1DosDungeon)) return 0;

    /* CSB PC 3.4 canonical. */
    if (!seed_payload(root, "csb/GRAPHICS.DAT",
                      "platform-matrix CSB graphics v1\n",
                      h->csbGraphics)) return 0;
    if (!seed_payload(root, "csb/DUNGEON.DAT",
                      "platform-matrix CSB dungeon v1\n",
                      h->csbDungeon)) return 0;

    /* CSB Amiga 3.3 Meynaf FR (legacy-amiga-dms). */
    if (!ensure_nested(root, "csb-extras/legacy-amiga-dms")) return 0;
    if (!seed_payload(root, "csb-extras/legacy-amiga-dms/GRAPHICS.DAT",
                      "platform-matrix CSB legacy-amiga graphics v1\n",
                      h->csbAmigaGraphics)) return 0;
    if (!seed_payload(root, "csb-extras/legacy-amiga-dms/DUNGEON.DAT",
                      "platform-matrix CSB legacy-amiga dungeon v1\n",
                      h->csbAmigaDungeon)) return 0;

    /* DM2 canonical. */
    if (!seed_payload(root, "dm2/GRAPHICS.DAT",
                      "platform-matrix DM2 graphics v1\n",
                      h->dm2Graphics)) return 0;
    if (!seed_payload(root, "dm2/DUNGEON.DAT",
                      "platform-matrix DM2 dungeon v1\n",
                      h->dm2Dungeon)) return 0;

    /* DM2 extras: dos-en, dos-fr, pc-fr, pc-de (mirrors
     * tier1_strict_boot_probe.c rows). */
    if (!ensure_nested(root, "dm2-extras/dos-en")) return 0;
    if (!seed_payload(root, "dm2-extras/dos-en/GRAPHICS.DAT",
                      "platform-matrix DM2 dos-en graphics v1\n",
                      h->dm2DosEnGraphics)) return 0;

    if (!ensure_nested(root, "dm2-extras/dos-fr")) return 0;
    if (!seed_payload(root, "dm2-extras/dos-fr/GRAPHICS.DAT",
                      "platform-matrix DM2 dos-fr graphics v1\n",
                      h->dm2DosFrGraphics)) return 0;

    if (!ensure_nested(root, "dm2-extras/pc-fr")) return 0;
    if (!seed_payload(root, "dm2-extras/pc-fr/GRAPHICS.DAT",
                      "platform-matrix DM2 pc-fr graphics v1\n",
                      h->dm2PcFrGraphics)) return 0;

    if (!ensure_nested(root, "dm2-extras/pc-de")) return 0;
    if (!seed_payload(root, "dm2-extras/pc-de/GRAPHICS.DAT",
                      "platform-matrix DM2 pc-de graphics v1\n",
                      h->dm2PcDeGraphics)) return 0;

    /* Theron JP canonical (Track 02.iso). */
    if (!seed_payload(root, "theron/Theron's Quest (Japan) (Track 02).iso",
                      "platform-matrix Theron JP Track02 v1\n",
                      h->theronTrack02)) return 0;

    /* Theron JP extras (Track 02.bin under theron-extras/japan). */
    if (!ensure_nested(root, "theron-extras/japan")) return 0;
    if (!seed_payload(root, "theron-extras/japan/track02.bin",
                      "platform-matrix Theron JP extras Track02 v1\n",
                      h->theronJpTrack02)) return 0;

    /* Theron US extras (Track 02.bin under theron-extras/usa). */
    if (!ensure_nested(root, "theron-extras/usa")) return 0;
    if (!seed_payload(root, "theron-extras/usa/track02.bin",
                      "platform-matrix Theron US extras Track02 v1\n",
                      h->theronUsTrack02)) return 0;

    return 1;
}

static void clear_all_synthetic_hashes(void) {
    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetDm2SyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetCsbSyntheticHashes(NULL, NULL);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);
}

static void check_default_data_dir_resolution(const char* home) {
    char expected[512];
    char resolved[512];
    char defaultOriginals[512];
    int rc;

#ifdef _WIN32
    rc = snprintf(expected, sizeof(expected), "%s\\.firestaff\\data", home);
#else
    rc = snprintf(expected, sizeof(expected), "%s/.firestaff/data", home);
#endif
    check_int(rc > 0 && (size_t)rc < sizeof(expected),
              "default data-dir expectation must fit");

    (void)probe_setenv("FIRESTAFF_DATA", NULL);
    check_int(FSP_GetDefaultOriginalsDir(defaultOriginals,
                                         sizeof(defaultOriginals)) == 1,
              "default data-dir helper must resolve");
    check_int(strcmp(defaultOriginals, expected) == 0,
              "default data-dir helper must use the canonical user data root");
    check_int(FSP_ResolveDataDir(resolved, sizeof(resolved), NULL) == 1,
              "data-dir resolver must resolve without FIRESTAFF_DATA");
    check_int(strcmp(resolved, expected) == 0,
              "data-dir resolver must default to the canonical user data root");
}

/* Part A: a fresh empty data root must report zero availability
 * and a single rootCount with zero duplicate-root skips. */
static void check_empty_root_no_availability(const char* root) {
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics metrics;
    static const char* const gameIds[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    size_t i;
    size_t gameCount = sizeof(gameIds) / sizeof(gameIds[0]);

    clear_all_synthetic_hashes();

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(metrics.rootCount == 1U,
              "empty data root must scan exactly one search root");
    check_int(metrics.duplicateRootSkips == 0U,
              "empty data root must not even attempt to dedupe fallbacks");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), root) == 0,
              "scan must echo the configured data root, not legacy fallback");

    for (i = 0U; i < gameCount; ++i) {
        char message[160];
        snprintf(message, sizeof(message),
                 "empty data root must keep %s unavailable",
                 gameIds[i]);
        check_int(M12_AssetStatus_GameAvailable(&status, gameIds[i]) == 0,
                  message);
    }
    check_int(M12_AssetStatus_HasOriginalFileCandidate(&status) == 0,
              "empty data root must not report an original asset candidate");
}

/* Part B: when the platform-matrix layout is seeded with synthetic
 * files, the M12 scanner should report DM1/CSB/DM2/Theron as
 * AVAILABLE while keeping Nexus UNAVAILABLE (no DM.BIN in the
 * fixture). Recursive discovery must find files at every nested
 * sub-layout, and the metrics must scale exactly once per game
 * per root, never per leaf file. */
static void check_full_layout_scan(const char* root,
                                   const PlatformMatrixHashes* h) {
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics metrics;
    char foundPath[ASSET_PATH_MAX];

    /* First, verify that without synthetic hashes registered,
     * availability stays 0 — proves the picker does not
     * auto-promote by filename. */
    clear_all_synthetic_hashes();

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(metrics.rootCount == 1U,
              "explicit data-dir must scan exactly one search root, "
              "no platform-fallback expansion");
    check_int(metrics.duplicateRootSkips == 0U,
              "explicit data-dir must not add platform-fallback roots");
    check_int(metrics.versionHashLookups >= 1U,
              "scan must report at least one version-hash lookup per game");
    check_int(metrics.requiredHashLookups >= 1U,
              "scan must report at least one required-hash lookup");

    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 0,
              "DM1 must stay unavailable until its known hashes are registered");
    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 0,
              "CSB must stay unavailable until its known hashes are registered");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm2") == 0,
              "DM2 must stay unavailable until its known hashes are registered");
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
              "Theron must stay unavailable until its known hashes are registered");

    /* Register the synthetic hashes and re-scan. */
    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(
        h->dm1MultiGraphics, h->dm1MultiDungeon);
    M12_AssetStatus_TestSetDm2SyntheticHashes(h->dm2Graphics, h->dm2Dungeon);
    M12_AssetStatus_TestSetCsbSyntheticHashes(h->csbGraphics, h->csbDungeon);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(h->theronTrack02);

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    check_int(metrics.rootCount == 1U,
              "explicit data-dir with platform-matrix layout must scan one root");
    check_int(metrics.duplicateRootSkips == 0U,
              "platform-matrix scan must not duplicate any root");

    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "DM1 must be available when its pc34-multi row matches");
    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 1,
              "CSB must be available when its pc34-en row matches");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
              "DM2 must be available when its pc-en row matches");
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "Theron must be available when its pce-jp row matches");
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 0,
              "Nexus must stay unavailable without a DM.BIN match");

    /* First-matched-version determinism. */
    {
        const M12_AssetVersionStatus* first =
            M12_AssetStatus_GetFirstMatchedVersion(&status, "dm1");
        check_int(first != NULL,
                  "first-matched DM1 version must not be NULL");
        check_int(first && strcmp(first->versionId, "pc34-multi") == 0,
                  "first-matched DM1 version must follow the g_dm1Versions[] order");
    }
    {
        const M12_AssetVersionStatus* first =
            M12_AssetStatus_GetFirstMatchedVersion(&status, "csb");
        check_int(first != NULL,
                  "first-matched CSB version must not be NULL");
        check_int(first && strcmp(first->versionId, "pc34-en") == 0,
                  "first-matched CSB version must be the pc34-en row");
    }
    {
        const M12_AssetVersionStatus* first =
            M12_AssetStatus_GetFirstMatchedVersion(&status, "dm2");
        check_int(first != NULL,
                  "first-matched DM2 version must not be NULL");
        check_int(first && strcmp(first->versionId, "pc-en") == 0,
                  "first-matched DM2 version must be the pc-en row");
    }
    {
        const M12_AssetVersionStatus* first =
            M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
        check_int(first != NULL,
                  "first-matched Theron version must not be NULL");
        check_int(first && strcmp(first->versionId, "pce-jp") == 0,
                  "first-matched Theron version must be the pce-jp row");
    }

    /* asset_find_by_md5 must recursively find each platform-matrix
     * leaf without depending on filename. */
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->dm1CanonicalGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm1/GRAPHICS.DAT") != NULL,
              "recursive scan must find canonical DM1 GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->dm1DosGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm1-extras/legacy-dos/GRAPHICS.DAT") != NULL,
              "recursive scan must find DM1 legacy-dos GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->csbAmigaGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "csb-extras/legacy-amiga-dms/GRAPHICS.DAT") != NULL,
              "recursive scan must find CSB legacy-amiga GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->dm2DosEnGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm2-extras/dos-en/GRAPHICS.DAT") != NULL,
              "recursive scan must find DM2 dos-en GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->dm2DosFrGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm2-extras/dos-fr/GRAPHICS.DAT") != NULL,
              "recursive scan must find DM2 dos-fr GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->dm2PcFrGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm2-extras/pc-fr/GRAPHICS.DAT") != NULL,
              "recursive scan must find DM2 pc-fr GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->dm2PcDeGraphics,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "dm2-extras/pc-de/GRAPHICS.DAT") != NULL,
              "recursive scan must find DM2 pc-de GRAPHICS by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->theronJpTrack02,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "theron-extras/japan/track02.bin") != NULL,
              "recursive scan must find Theron JP extras Track02 by hash");
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, h->theronUsTrack02,
                                foundPath, (int)sizeof(foundPath), 32) &&
                  strstr(foundPath, "theron-extras/usa/track02.bin") != NULL,
              "recursive scan must find Theron US extras Track02 by hash");

    /* Negative discovery: a hash that does not exist anywhere must
     * return 0. */
    memset(foundPath, 0, sizeof(foundPath));
    check_int(asset_find_by_md5(root, "00000000000000000000000000000000",
                                foundPath, (int)sizeof(foundPath), 32) == 0,
              "recursive scan must not match a hash that does not exist");
}

/* Part C: switching the data root via re-Scan (the contract
 * M12_StartupMenu_SetDataDirectory enforces) must clear prior
 * availability flags and matched paths. */
static void check_re_scan_clears_state(const char* goodRoot,
                                       const char* badRoot,
                                       const PlatformMatrixHashes* h) {
    M12_AssetStatus status;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(
        h->dm1MultiGraphics, h->dm1MultiDungeon);
    M12_AssetStatus_TestSetDm2SyntheticHashes(h->dm2Graphics, h->dm2Dungeon);
    M12_AssetStatus_TestSetCsbSyntheticHashes(h->csbGraphics, h->csbDungeon);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(h->theronTrack02);

    M12_AssetStatus_Scan(&status, goodRoot);
    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "good root must mark DM1 available before the bad-root re-scan");
    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 1,
              "good root must mark CSB available before the bad-root re-scan");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
              "good root must mark DM2 available before the bad-root re-scan");
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "good root must mark Theron available before the bad-root re-scan");

    /* Re-Scan against a fresh irrelevant root with no synthetic
     * hashes registered. This is the contract
     * M12_StartupMenu_SetDataDirectory relies on: the new folder
     * replaces, never merges, the previous scan. */
    clear_all_synthetic_hashes();

    M12_AssetStatus_Scan(&status, badRoot);
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), badRoot) == 0,
              "re-scan must adopt the new data root verbatim");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 0,
              "DM1 must clear when the data root re-scan finds no match");
    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 0,
              "CSB must clear when the data root re-scan finds no match");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm2") == 0,
              "DM2 must clear when the data root re-scan finds no match");
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
              "Theron must clear when the data root re-scan finds no match");
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 0,
              "Nexus must stay unavailable after the data root re-scan");
}

/* Part D: re-scanning the same good root must keep the same
 * per-game availability state (no global-state pollution, no
 * cross-game flag drift). This is what keeps `--scan-data` smoke
 * reports idempotent in CI and what keeps the data-dir picker's
 * per-folder availability grid deterministic.
 *
 * NOTE: the per-scan metric counters (`versionHashLookups`,
 * `requiredHashLookups`, `duplicateRootSkips`) are *not* expected
 * to match across scans, because `M12_AssetStatus_Scan` takes a
 * fast-path for Theron (m12_reuse_verified_theron_refresh) when
 * the previous scan already found Theron at the same data dir
 * with the same hash. That fast path correctly preserves the
 * availability state, so the behavioral contract — same flags,
 * same data-dir, same matched paths — holds even though the
 * counters differ. This is by design: see
 * `m12_reuse_verified_theron_refresh` in src/shared/asset_status_m12.c. */
static void check_rescan_idempotent(const char* root,
                                    const PlatformMatrixHashes* h) {
    M12_AssetStatus status;

    M12_AssetStatus_TestSetDm1MultilanguageSyntheticHashes(
        h->dm1MultiGraphics, h->dm1MultiDungeon);
    M12_AssetStatus_TestSetDm2SyntheticHashes(h->dm2Graphics, h->dm2Dungeon);
    M12_AssetStatus_TestSetCsbSyntheticHashes(h->csbGraphics, h->csbDungeon);
    M12_AssetStatus_TestSetNexusSyntheticHash(NULL);
    M12_AssetStatus_TestSetTheronSyntheticHash(h->theronTrack02);

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);

    /* Reset counters so the second scan starts from a clean
     * slate (mirrors how the launcher sets up its own metrics
     * before each scan). */
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);

    /* The metric counters may legitimately differ across passes
     * because of the Theron fast path: `m12_reuse_verified_theron_refresh`
     * short-circuits the second pass before `m12_build_search_roots`
     * can stamp `rootCount` and before the per-version/per-required
     * lookup counters can increment. The fast path is correct as
     * long as the post-scan *behavior* is identical: same data dir,
     * same per-game availability, same first-matched version. */
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "Theron must stay available via the fast path on re-scan");
    {
        const M12_AssetVersionStatus* first =
            M12_AssetStatus_GetFirstMatchedVersion(&status, "theron");
        check_int(first && strcmp(first->versionId, "pce-jp") == 0,
                  "Theron fast path must keep the pce-jp first-match");
    }
    check_int(M12_AssetStatus_GameAvailable(&status, "dm1") == 1,
              "DM1 must still be available after a second scan of the same root");
    check_int(M12_AssetStatus_GameAvailable(&status, "csb") == 1,
              "CSB must still be available after a second scan of the same root");
    check_int(M12_AssetStatus_GameAvailable(&status, "dm2") == 1,
              "DM2 must still be available after a second scan of the same root");
    check_int(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
              "Theron must still be available after a second scan of the same root");
    check_int(M12_AssetStatus_GameAvailable(&status, "nexus") == 0,
              "Nexus must still be unavailable after a second scan of the same root");
    check_int(strcmp(M12_AssetStatus_GetDataDir(&status), root) == 0,
              "re-scan must keep the configured data root across passes");
}

int main(void) {
    char home[512];
    char goodRoot[512];
    char badRoot[512];
    PlatformMatrixHashes hashes;

    memset(&hashes, 0, sizeof(hashes));

    if (!make_isolated_home(home, sizeof(home)) ||
        !FSP_JoinPath(goodRoot, sizeof(goodRoot), home, "data") ||
        !FSP_JoinPath(badRoot, sizeof(badRoot), home, "irrelevant") ||
        !probe_setenv("HOME", home) ||
        !probe_setenv("USERPROFILE", home) ||
        !probe_setenv("XDG_DATA_HOME", home) ||
        !probe_setenv("APPDATA", home)) {
        fprintf(stderr, "fixture environment setup failed\n");
        return 1;
    }

    /* Make sure both roots exist; badRoot is left empty. */
    if (!ensure_dir(goodRoot) || !ensure_dir(badRoot)) {
        fprintf(stderr, "fixture root setup failed\n");
        return 1;
    }

    clear_all_synthetic_hashes();

    printf("=== Firestaff data-dir picker / platform-matrix regression ===\n");

    check_default_data_dir_resolution(home);
    check_int(probe_setenv("FIRESTAFF_DATA", goodRoot),
              "fixture FIRESTAFF_DATA set after default-resolution checks");

    check_empty_root_no_availability(badRoot);
    check_empty_root_no_availability(goodRoot);

    if (!build_platform_matrix_root(goodRoot, &hashes)) {
        fprintf(stderr, "platform-matrix fixture build failed\n");
        return 1;
    }

    check_full_layout_scan(goodRoot, &hashes);
    check_re_scan_clears_state(goodRoot, badRoot, &hashes);
    check_rescan_idempotent(goodRoot, &hashes);

    /* Leave environment clean for any following test. */
    clear_all_synthetic_hashes();
    (void)probe_setenv("FIRESTAFF_DATA", NULL);

    if (g_failures) {
        fprintf(stderr, "%d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: data-dir picker / platform-matrix scan contract holds");
    return 0;
}
