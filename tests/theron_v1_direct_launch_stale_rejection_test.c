/*
 * theron_v1_direct_launch_stale_rejection_test.c — Stale-path rejection
 * regression for the Theron direct-launch + M12 reuse-gate path.
 *
 * Verifies:
 *   1. M12_AssetStatus_Scan() records a synthetic Track 02 path/MD5
 *      on the first scan.
 *   2. A second scan with the same data_dir hits the verified-path
 *      reuse gate (reusableTheronRefreshes == 1) and does NOT bump
 *      the stale-rejection counter.
 *   3. Deleting the Track 02 file and re-scanning bumps the stale
 *      rejection counter and falls through to the full scan, so the
 *      launcher reports Theron unavailable instead of trusting a
 *      cached path to a deleted file.
 *   4. Replacing the Track 02 file with a different blob (different
 *      MD5) and re-scanning also bumps the stale rejection counter.
 *   5. Restoring the original file content lets the reuse gate fire
 *      again — the gate is path+MD5-bound, not "always refuse".
 *   6. theron_v1_boot_verified_path_is_stale() reports the same
 *      stale/missing/clean states the M12 reuse gate sees.
 *   7. theron_v1_boot_load_verified_path() refuses (-1) when the
 *      supplied Track 02 path no longer exists, with exactly one
 *      stat probe (the file_exists() check).  This makes the boot
 *      contract match the M12 reuse-gate contract on the same input.
 *
 * The test is fully synthetic: it does not touch real Track 02 BINs,
 * so CI stays deterministic on hosts without the optional Theron
 * data set.
 *
 * Source-lock: src/shared/asset_status_m12.c
 *   m12_reuse_verified_theron_refresh + m12_theron_tracked_path_is_stale,
 *   src/theron/theron_v1_boot.c
 *   theron_v1_boot_verified_path_is_stale +
 *   theron_v1_boot_load_verified_path.  See
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md sections
 *   1.3 + 4.2 for the four known Track 02 MD5s; this test does not
 *   depend on a real Track 02 file and never hashes copyrighted
 *   game data — it computes an MD5 over a fixed synthetic payload.
 */

#define FIRESTAFF_ASSET_STATUS_TESTING 1
#include "asset_status_m12.h"
#include "theron_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define PATH_SEP "\\"
#define TEST_UNLINK(path) _unlink(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define PATH_SEP "/"
#define TEST_UNLINK(path) unlink(path)
static int test_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int g_failures = 0;
static int g_assertions = 0;

static void expect_true(int condition, const char* message) {
    ++g_assertions;
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int write_file(const char* path, const char* text) {
    FILE* fp = fopen(path, "wb");
    size_t len = text ? strlen(text) : 0U;
    if (!fp) return 0;
    if (len > 0U && fwrite(text, 1U, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int make_dir_if_needed(const char* path) {
    return TEST_MKDIR(path) == 0;
}

static int make_isolated_root(char* out, size_t outSize) {
#if defined(_WIN32)
    int rc = snprintf(out, outSize, ".\\firestaff_theron_stale_%lu",
                      (unsigned long)rand());
    if (rc <= 0 || (size_t)rc >= outSize) {
        return 0;
    }
    return make_dir_if_needed(out);
#else
    char templatePath[] = "/tmp/firestaff-theron-stale-XXXXXX";
    char* made = mkdtemp(templatePath);
    if (!made) {
        return 0;
    }
    snprintf(out, outSize, "%s", made);
    return 1;
#endif
}

int main(void) {
    static const char trackPayload[] =
        "Firestaff synthetic Theron stale-rejection fixture v1\n";
    static const char trackPayloadSwapped[] =
        "Firestaff synthetic Theron stale-rejection fixture v1 -- swapped payload bytes\n";
    char root[512];
    char theronDir[512];
    char trackPath[512];
    char trackMd5[M12_ASSET_MD5_CAPACITY];
    M12_AssetStatus status;
    M12_AssetStatusScanMetrics metrics;
    const M12_AssetVersionStatus* version;
    const M12_AssetRequiredFileStatus* required;
    Theron_V1_BootProfile bootProfile;
    unsigned long rescansBefore;
    unsigned long rescansAfter;

    /* ── Isolated data root + synthetic Track 02 ──────────── */
    expect_true(make_isolated_root(root, sizeof(root)),
                "temporary Theron stale-rejection data root created");
    snprintf(theronDir, sizeof(theronDir), "%s/theron", root);
    expect_true(make_dir_if_needed(theronDir),
                "theron fixture directory created");
    snprintf(trackPath, sizeof(trackPath), "%s/track02.bin", theronDir);
    expect_true(write_file(trackPath, trackPayload),
                "synthetic Theron Track 02 fixture written");
    expect_true(m12_file_md5_hex(trackPath, trackMd5),
                "synthetic Theron Track 02 MD5 computed");
    expect_true(test_setenv("HOME", root) &&
                    test_setenv("FIRESTAFF_DATA", root),
                "Theron stale-rejection fixture environment isolated");
    M12_AssetStatus_TestSetTheronSyntheticHash(trackMd5);

    /* ── First scan — catalogue the path/MD5 ───────────────── */
    memset(&status, 0, sizeof(status));
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
                "initial scan marks synthetic Track 02 available");
    version = M12_AssetStatus_GetVersion(&status, "theron", 0U);
    expect_true(version && version->matched &&
                    strcmp(version->matchedPath, trackPath) == 0 &&
                    strcmp(version->matchedMd5, trackMd5) == 0,
                "initial scan records the verified path + MD5");
    required = M12_AssetStatus_GetRequiredFile(&status, "theron", 0U);
    expect_true(required && required->matched &&
                    strcmp(required->matchedPath, trackPath) == 0 &&
                    strcmp(required->matchedHash, trackMd5) == 0,
                "initial scan propagates the required Track 02 marker");
    expect_true(metrics.reusableTheronRefreshes == 0U,
                "initial scan does not consume a reuse-gate hit");
    expect_true(metrics.staleTheronRefreshRejections == 0U,
                "initial scan does not bump the stale-rejection counter");

    /* ── Second scan — same data_dir, reuse gate fires ─────── */
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
                "second scan keeps Theron available via the reuse gate");
    expect_true(metrics.reusableTheronRefreshes == 1U,
                "second scan fires the reuse gate exactly once");
    expect_true(metrics.staleTheronRefreshRejections == 0U,
                "second scan does not bump the stale-rejection counter");

    /* ── Delete the file — reuse gate must refuse + fall through */
    expect_true(TEST_UNLINK(trackPath) == 0,
                "deleted the cached Track 02 file");
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(metrics.staleTheronRefreshRejections == 1U,
                "deleted Track 02 bumps stale-rejection counter");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "deleted Track 02 leaves Theron unavailable");
    version = M12_AssetStatus_GetVersion(&status, "theron", 0U);
    expect_true(!version || !version->matched,
                "deleted Track 02 clears the matched version");

    /* ── Restore the original file — first scan re-discovers, second hits reuse */
    expect_true(write_file(trackPath, trackPayload),
                "restored the cached Track 02 file");
    /* m12_file_md5_hex() result should match the original trackMd5;
     * guard against the file system silently munging bytes. */
    {
        char restoredMd5[M12_ASSET_MD5_CAPACITY];
        expect_true(m12_file_md5_hex(trackPath, restoredMd5),
                    "restored Track 02 MD5 rehash succeeds");
        expect_true(strcmp(restoredMd5, trackMd5) == 0,
                    "restored Track 02 MD5 matches the recorded MD5");
    }
    /* First scan after restore: the previous deletion forced a full
     * scan that wiped status, so the status no longer has a matched
     * version.  The reuse gate returns 0 silently (version.matched
     * == 0 short-circuits before the stale check), the full scan
     * re-discovers the file, and the status now records the path +
     * MD5 again.  Counters stay at 0/0. */
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(metrics.staleTheronRefreshRejections == 0U,
                "first scan after restore does not bump stale-rejection counter");
    expect_true(metrics.reusableTheronRefreshes == 0U,
                "first scan after restore is a fresh discovery, not a reuse");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
                "restored Track 02 brings Theron back online");

    /* Second scan after restore: now status has a matched entry, so
     * the reuse gate fires exactly once. */
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(metrics.staleTheronRefreshRejections == 0U,
                "second scan after restore does not bump stale-rejection counter");
    expect_true(metrics.reusableTheronRefreshes == 1U,
                "second scan after restore fires the reuse gate again");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
                "restored Track 02 still available after second scan");

    /* ── Replace with a different blob — reuse gate must refuse */
    expect_true(write_file(trackPath, trackPayloadSwapped),
                "replaced Track 02 with a different blob");
    {
        char swappedMd5[M12_ASSET_MD5_CAPACITY];
        expect_true(m12_file_md5_hex(trackPath, swappedMd5),
                    "swapped Track 02 MD5 rehash succeeds");
        expect_true(strcmp(swappedMd5, trackMd5) != 0,
                    "swapped Track 02 MD5 differs from the recorded MD5");
    }
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(metrics.staleTheronRefreshRejections == 1U,
                "swapped Track 02 bumps stale-rejection counter");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 0,
                "swapped Track 02 leaves Theron unavailable (path no longer verified)");

    /* ── Restore original file once more — reuse gate re-fires */
    expect_true(write_file(trackPath, trackPayload),
                "re-restored the original Track 02 payload");
    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();
    /* Same as the post-deletion case: first scan re-discovers. */
    expect_true(metrics.reusableTheronRefreshes == 0U,
                "first scan after swap-restore is a fresh discovery");
    expect_true(metrics.staleTheronRefreshRejections == 0U,
                "first scan after swap-restore does not bump stale counter");

    M12_AssetStatus_TestResetScanMetrics();
    M12_AssetStatus_Scan(&status, root);
    metrics = M12_AssetStatus_TestGetScanMetrics();

    expect_true(metrics.reusableTheronRefreshes == 1U,
                "second scan after swap-restore fires the reuse gate again");
    expect_true(metrics.staleTheronRefreshRejections == 0U,
                "second scan after swap-restore does not bump stale counter");
    expect_true(M12_AssetStatus_GameAvailable(&status, "theron") == 1,
                "re-restored Track 02 brings Theron back online");

    /* ── Boot-level stale-path helper contract ───────────── */
    expect_true(theron_v1_boot_verified_path_is_stale(NULL, trackMd5) == 1,
                "boot helper reports NULL path as stale");
    expect_true(theron_v1_boot_verified_path_is_stale("", trackMd5) == 1,
                "boot helper reports empty path as stale");
    expect_true(theron_v1_boot_verified_path_is_stale(trackPath, NULL) == 1,
                "boot helper reports NULL MD5 as stale");
    expect_true(theron_v1_boot_verified_path_is_stale(trackPath, "") == 1,
                "boot helper reports empty MD5 as stale");
    expect_true(theron_v1_boot_verified_path_is_stale(
                    "/nonexistent/firestaff-theron-stale-path-zzzzz.bin",
                    trackMd5) == 1,
                "boot helper reports missing file as stale");
    expect_true(theron_v1_boot_verified_path_is_stale(trackPath, trackMd5) == 0,
                "boot helper reports clean path/MD5 as not stale");

    /* Swap bytes → boot helper reports stale */
    expect_true(write_file(trackPath, trackPayloadSwapped),
                "boot-helper swap: wrote different Track 02 bytes");
    expect_true(theron_v1_boot_verified_path_is_stale(trackPath, trackMd5) == 1,
                "boot helper reports swapped bytes as stale");
    expect_true(write_file(trackPath, trackPayload),
                "boot-helper restore: rewrote original Track 02 bytes");

    /* ── Direct-launch path rejects missing file (one stat) ── */
    /* theron_v1_boot_load_verified_path() gates on g_theron_known_md5s
     * before the file_exists() check, so the missing-file test must
     * pass a known Track 02 MD5 (otherwise the function returns -1
     * on the MD5 check without ever stat'ing the path).  We use the
     * JP Track 02 BIN MD5 because the boot module accepts it
     * regardless of the actual on-disk file content — direct launch
     * is a caller-trust contract: the caller already hashed the
     * file, and the boot module only verifies the file is still
     * reachable. */
    {
        static const char knownJpMd5[] = "b7afb338ad31be1025b53f9aff12d73a";
        theron_v1_boot_rescan_call_count_reset();
        rescansBefore = theron_v1_boot_rescan_call_count();
        theron_v1_boot_profile_init(&bootProfile);
        {
            char missingPath[512];
            int rc;
            snprintf(missingPath, sizeof(missingPath),
                     "%s%stheron%s%s",
                     root, PATH_SEP, PATH_SEP,
                     "definitely-not-here-zzzz.bin");
            /* ^ deliberately constructs "<root>/theron/definitely..." */
            rc = theron_v1_boot_load_verified_path(&bootProfile,
                                                    missingPath,
                                                    knownJpMd5);
            rescansAfter = theron_v1_boot_rescan_call_count();
            expect_true(rc == -1,
                        "direct launch refuses a Track 02 path that does not exist");
            expect_true(rescansAfter == rescansBefore + 1UL,
                        "direct launch missing-file rejection uses exactly one stat");
            expect_true(bootProfile.assets_verified == 0,
                        "refused launch leaves assets_verified unset");
        }
    }

    /* ── Direct-launch path accepts the clean file ─────────── */
    {
        static const char knownJpMd5[] = "b7afb338ad31be1025b53f9aff12d73a";
        theron_v1_boot_rescan_call_count_reset();
        rescansBefore = theron_v1_boot_rescan_call_count();
        theron_v1_boot_profile_init(&bootProfile);
        {
            int rc = theron_v1_boot_load_verified_path(&bootProfile,
                                                        trackPath,
                                                        knownJpMd5);
            rescansAfter = theron_v1_boot_rescan_call_count();
            expect_true(rc == 0,
                        "direct launch succeeds for the clean Track 02 path");
            expect_true(bootProfile.assets_verified == 1,
                        "successful launch marks assets_verified");
            expect_true(strcmp(bootProfile.graphics_path, trackPath) == 0,
                        "successful launch keeps the supplied Track 02 path");
            expect_true(strcmp(bootProfile.graphics_md5, knownJpMd5) == 0,
                        "successful launch keeps the supplied Track 02 MD5");
            expect_true(rescansAfter == rescansBefore + 1UL,
                        "direct launch clean-path fill uses exactly one stat");
        }
    }

    /* ── Direct-launch rejects a synthetic (untrusted) MD5 with no stat ── */
    {
        theron_v1_boot_rescan_call_count_reset();
        rescansBefore = theron_v1_boot_rescan_call_count();
        theron_v1_boot_profile_init(&bootProfile);
        {
            int rc = theron_v1_boot_load_verified_path(&bootProfile,
                                                        trackPath,
                                                        trackMd5);
            rescansAfter = theron_v1_boot_rescan_call_count();
            expect_true(rc == -1,
                        "direct launch refuses a synthetic (untrusted) Track 02 MD5");
            expect_true(rescansAfter == rescansBefore,
                        "rejected-MD5 path performs no stat() probes");
            expect_true(bootProfile.assets_verified == 0,
                        "rejected-MD5 launch leaves assets_verified unset");
        }
    }

    /* ── Direct-launch rejects a swapped file via the new
     *    verified_path_is_stale helper semantics ──────────── */
    expect_true(write_file(trackPath, trackPayloadSwapped),
                "direct-launch swap: wrote different Track 02 bytes");
    expect_true(theron_v1_boot_verified_path_is_stale(trackPath, trackMd5) == 1,
                "boot helper reports swapped bytes as stale (direct-launch side)");
    /* Direct launch itself trusts the caller's MD5 contract and only
     * checks file_exists(); the staleness helper is the source of
     * truth for callers that want re-hash semantics.  Confirm the
     * staleness helper agrees with the swap case so the direct-
     * launch missing-file branch (verified above) is the only
     * divergence between the two contracts. */
    expect_true(theron_v1_boot_verified_path_is_stale(trackPath, trackMd5) == 1,
                "boot helper agrees: swapped file is stale for the recorded MD5");

    /* ── Cleanup ──────────────────────────────────────────── */
    M12_AssetStatus_TestSetTheronSyntheticHash(NULL);
    (void)test_setenv("FIRESTAFF_DATA", NULL);
    (void)test_setenv("HOME", NULL);

    if (g_failures) {
        fprintf(stderr,
                "Theron V1 stale-rejection checks FAILED (%d failures, %d assertions)\n",
                g_failures, g_assertions);
        return 1;
    }
    printf("ok: Theron V1 stale-rejection assertions=%d md5=%s\n",
           g_assertions, trackMd5);
    return 0;
}
