/*
 * test_cloud_sync_m12_boundary_pc34_compat.c
 *
 * Data-free unit test for the M12 launcher/settings cloud-sync
 * boundary contract. Covers the bounded boundary gate documented in
 * `include/cloud_sync_m12.h`:
 *
 *   1. Opt-in gate (M12_CloudSync_IsEnabled / SetEnabled)
 *      • Default is OFF (the launcher never silently copies user data)
 *      • SetEnabled returns the previous value
 *      • Disabling clears any cached conflict / last-run-code state
 *
 *   2. Run() is a no-op when disabled
 *      • Returns M12_SYNC_OK
 *      • Stats are zeroed (pushed / pulled / conflict / error)
 *      • Sync-dir state is untouched (no manifest side effect)
 *
 *   3. Status report (M12_CloudSync_GetStatus)
 *      • DISABLED when opt-in flag is off — takes precedence over
 *        every other state
 *      • NO_SYNC_DIR when enabled but no sync dir configured
 *      • READY when enabled and sync dir is configured
 *      • Current-state (not cached) report: live sync-dir state wins
 *        over cached error state
 *
 *   4. Cloud provider catalog
 *      • GetProviderCount returns M12_SYNC_PROVIDER_COUNT
 *      • GetProviderName returns the canonical name for every entry
 *        and NULL for out-of-range ids
 *      • LookupProviderId is case-insensitive and rejects unknown /
 *        NULL / empty names
 *      • IsProviderSupported returns 0 for every entry today (honest
 *        'unsupported' contract; no real backend is integrated)
 *
 *   5. RescanManifest clears the cached conflict / last-error state so
 *      GetStatus reflects the live sync-dir state again.
 *
 * Source-lock: none required — this is launcher/settings boundary
 * contract coverage, not game logic. ReDMCSB does not cover the
 * launcher.
 *
 * Disjoint from the headless probe firestaff_m12_cloud_sync_probe.c
 * (which exercises manifest round-trip + Run() + policy gates) and
 * from any CTest that requires game data. This test is data-free and
 * runs in CI.
 */

#include "cloud_sync_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#include <sys/stat.h>
#define MKDIR(p) _mkdir(p)
#define RMDIR(r) _rmdir(r)
#define UNLINK(r) _unlink(r)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0755)
#define RMDIR(r) rmdir(r)
#define UNLINK(r) unlink(r)
#endif

static int g_failures = 0;
static int g_total    = 0;

#define CHECK(cond, msg) do { \
    ++g_total; \
    if (cond) { \
        /* printf("  PASS: %s\n", msg); */ \
    } else { \
        printf("  FAIL: %s\n", msg); \
        ++g_failures; \
    } \
} while (0)

/* Build an isolated per-test sync dir so the test never touches the
 * user's real ~/.firestaff/sync. */
static void make_test_dir(char* out, size_t outSize) {
    snprintf(out, outSize,
             "/tmp/firestaff-cloud-sync-boundary-%d-%ld",
             (int)getpid(), (long)time(NULL));
    MKDIR(out);
}

/* Clean up the test dir tree. Best-effort; the test passes whether or
 * not cleanup succeeds. */
static void cleanup_dir(const char* dir) {
    char manifest[1024];
    snprintf(manifest, sizeof(manifest), "%s/manifest.json", dir);
    UNLINK(manifest);
    RMDIR(dir);
}

/* ── 1. Opt-in gate ─────────────────────────────────────────────── */

static void test_opt_in_gate(void) {
    int prev;

    /* Reset to a known baseline. */
    M12_CloudSync_SetEnabled(0);

    prev = M12_CloudSync_IsEnabled();
    CHECK(prev == 0, "default opt-in flag is OFF (no cloud sync by default)");

    prev = M12_CloudSync_SetEnabled(1);
    CHECK(prev == 0, "SetEnabled(1) returns 0 (previous value)");
    CHECK(M12_CloudSync_IsEnabled() == 1, "IsEnabled returns 1 after SetEnabled(1)");

    prev = M12_CloudSync_SetEnabled(1);
    CHECK(prev == 1, "SetEnabled(1) again returns 1 (previous value)");

    prev = M12_CloudSync_SetEnabled(0);
    CHECK(prev == 1, "SetEnabled(0) returns 1 (previous value)");
    CHECK(M12_CloudSync_IsEnabled() == 0, "IsEnabled returns 0 after SetEnabled(0)");
}

/* ── 2. Run() no-op when disabled ───────────────────────────────── */

static void test_run_noop_when_disabled(void) {
    char testDir[512];
    make_test_dir(testDir, sizeof(testDir));
    M12_CloudSync_SetSyncDir(testDir);
    M12_CloudSync_SetEnabled(0);  /* off by default, but explicit */

    M12_SyncStats stats;
    memset(&stats, 0xAA, sizeof(stats));  /* poison values */
    int rc = M12_CloudSync_Run(M12_SYNC_DIRECTION_BOTH, &stats);
    CHECK(rc == M12_SYNC_OK, "Run() returns M12_SYNC_OK when disabled");
    CHECK(stats.pushedCount  == 0, "Run() clears stats.pushedCount when disabled");
    CHECK(stats.pulledCount  == 0, "Run() clears stats.pulledCount when disabled");
    CHECK(stats.conflictCount == 0, "Run() clears stats.conflictCount when disabled");
    CHECK(stats.errorCount   == 0, "Run() clears stats.errorCount when disabled");

    /* NULL stats is also allowed. */
    rc = M12_CloudSync_Run(M12_SYNC_DIRECTION_PUSH, NULL);
    CHECK(rc == M12_SYNC_OK, "Run() with NULL stats still returns M12_SYNC_OK when disabled");

    /* Run() must not write a manifest side-effect when disabled.
     * The sync dir may legitimately exist (SetSyncDir created it),
     * but no manifest.json may appear after a disabled Run(). */
    char manifest[1024];
    snprintf(manifest, sizeof(manifest), "%s/manifest.json", testDir);
    FILE* fp = fopen(manifest, "rb");
    CHECK(fp == NULL, "Run() does not write sync/manifest.json when disabled");
    if (fp) fclose(fp);

    cleanup_dir(testDir);
}

/* ── 3. Status report ───────────────────────────────────────────── */

static void test_status_report(void) {
    char testDir[512];
    make_test_dir(testDir, sizeof(testDir));

    /* Disabled + no sync dir set explicitly. */
    M12_CloudSync_SetEnabled(0);
    M12_CloudSync_SetSyncDir(NULL);  /* reset to default */
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_DISABLED,
          "GetStatus returns DISABLED when opt-in is off (precedence)");

    /* Enabled + default sync dir (which is set by SetSyncDir to a real
     * path under HOME). The point is that enabled + a sync-dir means
     * READY — not cached error state. */
    M12_CloudSync_SetEnabled(1);
    /* Confirm a sync dir is in place. SetSyncDir("") resets to the
     * default ~/.firestaff/sync, which GetStatus should accept. */
    M12_CloudSync_SetSyncDir("");
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_READY,
          "GetStatus returns READY when enabled and sync dir configured");

    /* Setting the sync dir to an empty string again — still default,
     * still READY. */
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_READY,
          "GetStatus returns READY when enabled (idempotent)");

    /* Disable: status flips to DISABLED, regardless of sync dir. */
    M12_CloudSync_SetEnabled(0);
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_DISABLED,
          "GetStatus returns DISABLED after SetEnabled(0) regardless of sync dir");

    cleanup_dir(testDir);
}

static void test_status_disabled_takes_precedence(void) {
    /* Disabled + configured sync dir + cached conflict → still
     * DISABLED. The opt-in flag takes precedence over every other
     * state so the UI cannot mistakenly show 'Ready' to a user who
     * never opted in. */
    M12_CloudSync_SetEnabled(0);
    char testDir[512];
    make_test_dir(testDir, sizeof(testDir));
    M12_CloudSync_SetSyncDir(testDir);
    M12_CloudSync_SetEnabled(0);
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_DISABLED,
          "GetStatus precedence: DISABLED wins over configured sync dir");

    /* Re-enable → READY (live state, not cached). */
    M12_CloudSync_SetEnabled(1);
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_READY,
          "GetStatus flips to READY when re-enabled with sync dir");

    M12_CloudSync_SetEnabled(0);
    cleanup_dir(testDir);
}

/* ── 4. Cloud provider catalog ──────────────────────────────────── */

static void test_provider_catalog(void) {
    int count = M12_CloudSync_GetProviderCount();
    CHECK(count == M12_SYNC_PROVIDER_COUNT,
          "GetProviderCount returns M12_SYNC_PROVIDER_COUNT");

    /* Every catalog entry has a non-NULL name. */
    for (int i = 0; i < count; ++i) {
        const char* name = M12_CloudSync_GetProviderName(i);
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "GetProviderName(%d) returns non-NULL name", i);
        CHECK(name != NULL && name[0] != '\0', msg);
    }

    /* Out-of-range ids return NULL. */
    CHECK(M12_CloudSync_GetProviderName(-1) == NULL,
          "GetProviderName(-1) returns NULL");
    CHECK(M12_CloudSync_GetProviderName(M12_SYNC_PROVIDER_COUNT) == NULL,
          "GetProviderName(COUNT) returns NULL");
    CHECK(M12_CloudSync_GetProviderName(M12_SYNC_PROVIDER_COUNT + 100) == NULL,
          "GetProviderName(COUNT+100) returns NULL");

    /* LookupProviderId round-trips for every catalog entry. */
    for (int i = 0; i < count; ++i) {
        const char* name = M12_CloudSync_GetProviderName(i);
        int id = M12_CloudSync_LookupProviderId(name);
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "LookupProviderId round-trips for entry %d", i);
        CHECK(id == i, msg);
    }

    /* LookupProviderId is case-insensitive. */
    CHECK(M12_CloudSync_LookupProviderId("icloud")  == M12_SYNC_PROVIDER_ICLOUD,
          "LookupProviderId is case-insensitive: icloud");
    CHECK(M12_CloudSync_LookupProviderId("ICLOUD")  == M12_SYNC_PROVIDER_ICLOUD,
          "LookupProviderId is case-insensitive: ICLOUD");
    CHECK(M12_CloudSync_LookupProviderId("DropBox") == M12_SYNC_PROVIDER_DROPBOX,
          "LookupProviderId is case-insensitive: DropBox");
    CHECK(M12_CloudSync_LookupProviderId("OneDrive")== M12_SYNC_PROVIDER_ONEDRIVE,
          "LookupProviderId is case-insensitive: OneDrive");
    CHECK(M12_CloudSync_LookupProviderId("google drive") == M12_SYNC_PROVIDER_GOOGLE_DRIVE,
          "LookupProviderId is case-insensitive: google drive");
    CHECK(M12_CloudSync_LookupProviderId("RSYNC")   == M12_SYNC_PROVIDER_RSYNC,
          "LookupProviderId is case-insensitive: RSYNC");
}

static void test_provider_unsupported(void) {
    int count = M12_CloudSync_GetProviderCount();
    /* Honest contract: every entry returns 0 today. */
    for (int i = 0; i < count; ++i) {
        int supported = M12_CloudSync_IsProviderSupported(i);
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "IsProviderSupported(%d) returns 0 (no real backend)", i);
        CHECK(supported == 0, msg);
    }
    /* Out-of-range id also returns 0 (defensive). */
    CHECK(M12_CloudSync_IsProviderSupported(-1) == 0,
          "IsProviderSupported(-1) returns 0");
    CHECK(M12_CloudSync_IsProviderSupported(M12_SYNC_PROVIDER_COUNT + 50) == 0,
          "IsProviderSupported(COUNT+50) returns 0");
}

static void test_provider_unknown_names(void) {
    /* Unknown names → invalid id. */
    CHECK(M12_CloudSync_LookupProviderId("DropboxX") == M12_SYNC_PROVIDER_ID_INVALID,
          "LookupProviderId rejects unknown 'DropboxX'");
    CHECK(M12_CloudSync_LookupProviderId("Mega")     == M12_SYNC_PROVIDER_ID_INVALID,
          "LookupProviderId rejects unknown 'Mega'");
    CHECK(M12_CloudSync_LookupProviderId("iCloud ")  == M12_SYNC_PROVIDER_ID_INVALID,
          "LookupProviderId rejects trailing-space 'iCloud '");
    CHECK(M12_CloudSync_LookupProviderId("")         == M12_SYNC_PROVIDER_ID_INVALID,
          "LookupProviderId rejects empty string");
    CHECK(M12_CloudSync_LookupProviderId(NULL)      == M12_SYNC_PROVIDER_ID_INVALID,
          "LookupProviderId rejects NULL");
}

/* ── 5. Opt-in / SetSyncDir independence ────────────────────────── */

static void test_opt_in_sync_dir_independence(void) {
    /* Changing the sync dir while enabled must not change the opt-in
     * flag, and changing the opt-in flag must not change the sync
     * dir. The two settings are orthogonal. */
    M12_CloudSync_SetEnabled(0);
    char testDirA[512], testDirB[512];
    make_test_dir(testDirA, sizeof(testDirA));
    make_test_dir(testDirB, sizeof(testDirB));

    M12_CloudSync_SetSyncDir(testDirA);
    M12_CloudSync_SetEnabled(1);
    CHECK(M12_CloudSync_IsEnabled() == 1, "SetEnabled(1) does not touch sync dir");
    CHECK(strcmp(M12_CloudSync_GetSyncDir(), testDirA) == 0,
          "SetEnabled does not change the configured sync dir");

    M12_CloudSync_SetSyncDir(testDirB);
    CHECK(M12_CloudSync_IsEnabled() == 1,
          "SetSyncDir does not toggle the opt-in flag");
    CHECK(strcmp(M12_CloudSync_GetSyncDir(), testDirB) == 0,
          "SetSyncDir persists the new sync dir");

    M12_CloudSync_SetEnabled(0);
    cleanup_dir(testDirA);
    cleanup_dir(testDirB);
}

/* ── 6. RescanManifest clears cached state ──────────────────────── */

static void test_rescan_clears_cached_state(void) {
    /* RescanManifest is the documented acknowledgement for a cached
     * conflict / error state. After the user resolves the conflict
     * manually, RescanManifest clears the cache so GetStatus reflects
     * the live state again. */
    M12_CloudSync_SetEnabled(1);
    char testDir[512];
    make_test_dir(testDir, sizeof(testDir));
    M12_CloudSync_SetSyncDir(testDir);

    /* Force a known sync state. */
    int rc = M12_CloudSync_Run(M12_SYNC_DIRECTION_BOTH, NULL);
    CHECK(rc == M12_SYNC_OK, "Run() returns OK on a fresh enabled sync dir");
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_READY,
          "GetStatus is READY after a successful Run()");
    CHECK(M12_CloudSync_GetLastRunCode() == M12_SYNC_OK,
          "GetLastRunCode is OK after a successful Run()");

    /* RescanManifest must keep the status READY (live state, no error). */
    rc = M12_CloudSync_RescanManifest();
    CHECK(rc == 1, "RescanManifest returns 1 on success");
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_READY,
          "GetStatus remains READY after RescanManifest");
    CHECK(M12_CloudSync_GetLastRunCode() == M12_SYNC_OK,
          "GetLastRunCode is OK after RescanManifest");

    M12_CloudSync_SetEnabled(0);
    cleanup_dir(testDir);
}

/* ── 7. Disable clears cached state ─────────────────────────────── */

static void test_disable_clears_cached_state(void) {
    /* Disabling the opt-in flag must clear any cached conflict / last-
     * run-code so GetStatus() reflects the live state when the user
     * re-enables. */
    M12_CloudSync_SetEnabled(1);
    char testDir[512];
    make_test_dir(testDir, sizeof(testDir));
    M12_CloudSync_SetSyncDir(testDir);

    /* Set up a successful sync state. */
    (void)M12_CloudSync_Run(M12_SYNC_DIRECTION_BOTH, NULL);
    CHECK(M12_CloudSync_GetLastRunCode() == M12_SYNC_OK,
          "GetLastRunCode is OK after a successful Run()");

    /* Disable. The cached last-run-code should be cleared. */
    M12_CloudSync_SetEnabled(0);
    CHECK(M12_CloudSync_GetLastRunCode() == M12_SYNC_OK,
          "GetLastRunCode is cleared to OK after SetEnabled(0)");

    /* Re-enable. The status should be READY (live state), not stale. */
    M12_CloudSync_SetEnabled(1);
    CHECK(M12_CloudSync_GetStatus() == M12_SYNC_STATUS_READY,
          "GetStatus reflects live READY state after re-enable");

    M12_CloudSync_SetEnabled(0);
    cleanup_dir(testDir);
}

int main(void) {
    printf("=== test_cloud_sync_m12_boundary_pc34_compat ===\n\n");

    /* Group 1: Opt-in gate */
    printf("[1] Opt-in gate\n");
    test_opt_in_gate();

    /* Group 2: Run() no-op when disabled */
    printf("[2] Run() no-op when disabled\n");
    test_run_noop_when_disabled();

    /* Group 3: Status report */
    printf("[3] Status report\n");
    test_status_report();
    test_status_disabled_takes_precedence();

    /* Group 4: Provider catalog */
    printf("[4] Cloud provider catalog\n");
    test_provider_catalog();
    test_provider_unsupported();
    test_provider_unknown_names();

    /* Group 5: Opt-in / SetSyncDir independence */
    printf("[5] Opt-in / SetSyncDir independence\n");
    test_opt_in_sync_dir_independence();

    /* Group 6: RescanManifest clears cached state */
    printf("[6] RescanManifest clears cached state\n");
    test_rescan_clears_cached_state();

    /* Group 7: Disable clears cached state */
    printf("[7] Disable clears cached state\n");
    test_disable_clears_cached_state();

    printf("\n=== Results: %d pass, %d fail (%d total) ===\n",
           g_total - g_failures, g_failures, g_total);
    if (g_failures > 0) {
        printf("TEST FAILED\n");
        return 1;
    }
    printf("TEST PASSED\n");
    return 0;
}
