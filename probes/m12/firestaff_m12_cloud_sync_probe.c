/*
 * firestaff_m12_cloud_sync_probe.c
 *
 * Headless verification probe for cloud_sync_m12.
 *
 * No game data required. Tests:
 *   1.  M12_CloudSync_GetSyncDir returns a non-NULL path
 *   2.  M12_CloudSync_SetSyncDir / GetSyncDir round-trip
 *   3.  M12_CloudSync_LoadManifest / SaveManifest round-trip
 *   4.  M12_CloudSync_TrackPath / GetLastSyncTime round-trip
 *   5.  M12_CloudSync_NeedsSync returns 1 when no manifest exists
 *   6.  M12_CloudSync_Run(BOTH) returns 0 with fresh sync dir
 *   7.  M12_CloudSync_RescanManifest succeeds
 *   8.  M12_CloudSync_GetPolicy / SetPolicy round-trip
 *   9.  Opt-in boundary: disabled by default
 *  10.  RunIfEnabled is a strict no-op when disabled
 *  11.  Enable + valid dir + RunIfEnabled = OK
 *  12.  Enable + missing dir + RunIfEnabled = UNSUPPORTED
 *  13.  GetBoundaryTag reports DISABLED when off
 *  14.  GetBoundaryTag reports ENABLED_OK / ENABLED_NO_ROOT correctly
 *  15.  DiscoverSaveFiles picks up *.sav from a real directory
 *  15a. ApplyConfig(NULL) leaves cloud sync disabled
 *  15b. ApplyConfig routes M12_Config.cloudSyncEnabled + dir + policy
 *  16.  Cloud sync module links into firestaff_m12 library
 */

#include "cloud_sync_m12.h"
#include "config_m12.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        printf("  PASS: %s\n", msg); \
        g_pass++; \
    } else { \
        printf("  FAIL: %s\n", msg); \
        g_fail++; \
    } \
} while (0)

/* Helper: recursively create a directory tree. */
static int m12_make_dir(const char* path) {
    if (!path || !*path) return 0;
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if (len == 0) return 0;
    if (tmp[len-1] == '/') tmp[len-1] = '\0';
    for (char* p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
#if defined(_WIN32)
            (void)_mkdir(tmp);
#else
            (void)mkdir(tmp, 0755);
#endif
            *p = '/';
        }
    }
#if defined(_WIN32)
    return (_mkdir(tmp) == 0);
#else
    return (mkdir(tmp, 0755) == 0);
#endif
}

int main(void) {
    printf("\n=== firestaff_m12_cloud_sync_probe ===\n\n");

    /* Always start with cloud sync disabled (the default).
     * Restore the user's real state at the end. */
    int savedEnabled = M12_CloudSync_IsEnabled();
    M12_CloudSync_SetEnabled(0);

    /* 1. GetSyncDir returns a non-NULL path */
    {
        const char* dir = M12_CloudSync_GetSyncDir();
        CHECK(dir != NULL && strlen(dir) > 0, "GetSyncDir returns non-empty path");
        if (dir) printf("       sync dir: %s\n", dir);
    }

    /* 2. SetSyncDir / GetSyncDir round-trip */
    {
        char testDir[256];
        snprintf(testDir, sizeof(testDir), "/tmp/firestaff-sync-test-%d", (int)time(NULL));
        int ok = M12_CloudSync_SetSyncDir(testDir);
        CHECK(ok == 1, "SetSyncDir returns 1 on success");
        const char* retrieved = M12_CloudSync_GetSyncDir();
        CHECK(strcmp(retrieved, testDir) == 0, "GetSyncDir returns what SetSyncDir set");
        (void)ok;
    }

    /* 8. Policy get/set round-trip */
    {
        M12_CloudSync_SetPolicy(M12_SYNC_POLICY_CLOUD_WINS);
        CHECK(M12_CloudSync_GetPolicy() == M12_SYNC_POLICY_CLOUD_WINS,
              "SetPolicy / GetPolicy round-trip");
        M12_CloudSync_SetPolicy(M12_SYNC_POLICY_NEWER_WINS);
    }

    /* 3. Manifest load/save round-trip */
    {
        const char* dir = M12_CloudSync_GetSyncDir();
        M12_CloudSync_SetSyncDir(dir);
        M12_SyncManifest m;
        memset(&m, 0, sizeof(m));
        m.entryCount = 1;
        snprintf(m.entries[0].relativePath, sizeof(m.entries[0].relativePath), "test-file.txt");
        m.entries[0].localTimestamp = 1234567890U;
        m.entries[0].syncTimestamp = 1234567890U;
        m.entries[0].localCrc = 0xDEADBEEFU;
        m.entries[0].syncCrc = 0xDEADBEEFU;
        m.entries[0].conflict = 0;
        m.lastFullSync = 1234567890U;

        int saved = M12_CloudSync_SaveManifest(&m);
        CHECK(saved == 1, "SaveManifest returns 1 on success");

        M12_SyncManifest m2;
        memset(&m2, 0, sizeof(m2));
        int loaded = M12_CloudSync_LoadManifest(&m2);
        CHECK(loaded == 1, "LoadManifest returns 1 on existing manifest");
        CHECK(m2.entryCount == 1, "Manifest entryCount preserved after save/load");
        CHECK(m2.entries[0].localTimestamp == 1234567890U,
              "Manifest localTimestamp preserved after save/load");
        CHECK(m2.lastFullSync == 1234567890U,
              "Manifest lastFullSync preserved after save/load");
    }

    /* 4. TrackPath / GetLastSyncTime round-trip */
    {
        const char* dir = M12_CloudSync_GetSyncDir();
        M12_CloudSync_SetSyncDir(dir);

        int tracked = M12_CloudSync_TrackPath("test-saves/dm1/save1.sav", 1);
        CHECK(tracked == 1, "TrackPath returns 1 when adding path");

        uint32_t t = M12_CloudSync_GetLastSyncTime("test-saves/dm1/save1.sav");
        CHECK(t == 0, "GetLastSyncTime returns 0 for newly-tracked path (never synced)");

        int untracked = M12_CloudSync_TrackPath("test-saves/dm1/save1.sav", 0);
        CHECK(untracked == 1, "TrackPath returns 1 when removing path");
    }

    /* 5. NeedsSync when no manifest */
    {
        const char* dir = M12_CloudSync_GetSyncDir();
        char manifestPath[256];
        snprintf(manifestPath, sizeof(manifestPath), "%s/manifest.json", dir);

        (void)remove(manifestPath);

        int needs = M12_CloudSync_NeedsSync();
        CHECK(needs == 1, "NeedsSync returns 1 when no manifest exists");
    }

    /* 7. RescanManifest succeeds */
    {
        int ok = M12_CloudSync_RescanManifest();
        CHECK(ok == 1, "RescanManifest returns 1 on success");
    }

    /* 6. Run(BOTH) with fresh sync dir */
    {
        M12_CloudSync_SetPolicy(M12_SYNC_POLICY_NEWER_WINS);
        M12_SyncStats stats;
        memset(&stats, 0, sizeof(stats));
        int rc = M12_CloudSync_Run(M12_SYNC_DIRECTION_BOTH, &stats);
        CHECK(rc == M12_SYNC_OK, "Run(BOTH) returns M12_SYNC_OK");
    }

    /* 9. Opt-in boundary defaults to disabled. */
    {
        int en = M12_CloudSync_IsEnabled();
        CHECK(en == 0, "Cloud sync defaults to disabled (opt-in boundary)");
    }

    /* 13. GetBoundaryTag reports DISABLED when off. */
    {
        char tag[64];
        M12_CloudSync_GetBoundaryTag(tag, sizeof(tag));
        CHECK(strcmp(tag, "DISABLED") == 0,
              "Boundary tag is DISABLED when opt-in is off");
    }

    /* 10. RunIfEnabled is a strict no-op when disabled. */
    {
        /* Make sure the manifest file does not exist before, then check
         * that it does not exist after. */
        const char* dir = M12_CloudSync_GetSyncDir();
        char manifestPath[256];
        snprintf(manifestPath, sizeof(manifestPath), "%s/manifest.json", dir);
        (void)remove(manifestPath);

        M12_SyncStats stats;
        memset(&stats, 0, sizeof(stats));
        /* Stuff a sentinel value into stats; RunIfEnabled must zero it. */
        stats.pushedCount = 999;
        stats.pulledCount = 999;
        stats.conflictCount = 999;
        stats.errorCount = 999;
        int rc = M12_CloudSync_RunIfEnabled(M12_SYNC_DIRECTION_BOTH, &stats);
        CHECK(rc == M12_SYNC_OK_DISABLED,
              "RunIfEnabled returns M12_SYNC_OK_DISABLED when opt-in is off");
        CHECK(stats.pushedCount == 0 && stats.pulledCount == 0 &&
              stats.conflictCount == 0 && stats.errorCount == 0,
              "RunIfEnabled zeroes stats when disabled (conflict-safe no-op)");
        struct stat st;
        int manifestExists = (stat(manifestPath, &st) == 0);
        CHECK(manifestExists == 0,
              "RunIfEnabled does not create the manifest when disabled");
    }

    /* 12. Enable + sync dir that is actually a regular file + RunIfEnabled = UNSUPPORTED. */
    {
        char regularFile[256];
        snprintf(regularFile, sizeof(regularFile),
                 "/tmp/firestaff-not-a-dir-%d", (int)time(NULL));
        FILE* fp = fopen(regularFile, "wb");
        if (fp) { fwrite("not a dir", 1, 9, fp); fclose(fp); }

        M12_CloudSync_SetSyncDir(regularFile);
        M12_CloudSync_SetEnabled(1);

        char tag[64];
        M12_CloudSync_GetBoundaryTag(tag, sizeof(tag));
        /* Sync dir is not a directory; the boundary must report
         * something other than ENABLED_OK. */
        CHECK(strcmp(tag, "ENABLED_OK") != 0,
              "Boundary tag is not ENABLED_OK when sync path is a regular file");

        M12_SyncStats stats;
        memset(&stats, 0, sizeof(stats));
        int rc = M12_CloudSync_RunIfEnabled(M12_SYNC_DIRECTION_BOTH, &stats);
        CHECK(rc == M12_SYNC_ERR_UNSUPPORTED,
              "RunIfEnabled returns M12_SYNC_ERR_UNSUPPORTED when sync path is not a directory");
        CHECK(stats.pushedCount == 0 && stats.pulledCount == 0,
              "RunIfEnabled zeroes stats when sync path is unusable");

        (void)remove(regularFile);
    }

    /* 12b. Boundary reports ENABLED_NO_ROOT when the configured sync
     * dir no longer exists at call time. We set a valid sync dir
     * (which auto-creates it), then remove the directory on disk.
     * The boundary check is a fresh stat() at call time, so it
     * must now report the root as missing.
     *
     * Note: SetSyncDir also creates saves/<game>/ subdirs, so we
     * have to recursively remove everything first. */
    {
        char noRootDir[256];
        snprintf(noRootDir, sizeof(noRootDir),
                 "/tmp/firestaff-no-root-%d", (int)time(NULL));
        M12_CloudSync_SetSyncDir(noRootDir);   /* creates it (and saves subtree) */
        M12_CloudSync_SetEnabled(1);
        /* Recursively remove the entire tree to simulate "root missing
         * at call time". We can't use system(rm -rf) in portable C,
         * so we walk the saves/ tree first. */
        {
            const char* games[] = {"dm1", "csb", "dm2", "nexus", "theron"};
            for (int gi = 0; gi < 5; ++gi) {
                char sub[512];
                snprintf(sub, sizeof(sub), "%s/saves/%s", noRootDir, games[gi]);
#if defined(_WIN32)
                (void)_rmdir(sub);
#else
                (void)rmdir(sub);
#endif
            }
            char savesDir[512];
            snprintf(savesDir, sizeof(savesDir), "%s/saves", noRootDir);
#if defined(_WIN32)
            (void)_rmdir(savesDir);
            (void)_rmdir(noRootDir);
#else
            (void)rmdir(savesDir);
            (void)rmdir(noRootDir);
#endif
        }

        char tag[64];
        M12_CloudSync_GetBoundaryTag(tag, sizeof(tag));
        CHECK(strcmp(tag, "ENABLED_NO_ROOT") == 0,
              "Boundary tag is ENABLED_NO_ROOT when sync dir is removed at call time");

        M12_SyncStats stats;
        memset(&stats, 0, sizeof(stats));
        int rc = M12_CloudSync_RunIfEnabled(M12_SYNC_DIRECTION_BOTH, &stats);
        CHECK(rc == M12_SYNC_ERR_UNSUPPORTED,
              "RunIfEnabled returns M12_SYNC_ERR_UNSUPPORTED for missing sync dir");
    }

    /* 14. GetBoundaryTag reports ENABLED_OK with a valid dir. */
    {
        char goodDir[256];
        snprintf(goodDir, sizeof(goodDir),
                 "/tmp/firestaff-good-sync-%d", (int)time(NULL));
        M12_CloudSync_SetSyncDir(goodDir);
        M12_CloudSync_SetEnabled(1);

        char tag[64];
        M12_CloudSync_GetBoundaryTag(tag, sizeof(tag));
        CHECK(strcmp(tag, "ENABLED_OK") == 0,
              "Boundary tag is ENABLED_OK when sync dir is valid and writable");
    }

    /* 11. Enable + valid dir + RunIfEnabled = OK. */
    {
        char goodDir[256];
        snprintf(goodDir, sizeof(goodDir),
                 "/tmp/firestaff-good-sync-%d", (int)time(NULL));
        M12_CloudSync_SetSyncDir(goodDir);
        M12_CloudSync_SetEnabled(1);

        M12_SyncStats stats;
        memset(&stats, 0, sizeof(stats));
        int rc = M12_CloudSync_RunIfEnabled(M12_SYNC_DIRECTION_BOTH, &stats);
        CHECK(rc == M12_SYNC_OK,
              "RunIfEnabled returns M12_SYNC_OK with valid dir and opt-in on");
    }

    /* 15a. ApplyConfig: NULL disables. */
    {
        M12_CloudSync_ApplyConfig(NULL);
        CHECK(M12_CloudSync_IsEnabled() == 0,
              "ApplyConfig(NULL) leaves cloud sync disabled");
    }

    /* 15b. ApplyConfig: enabled flag + sync dir + policy from M12_Config. */
    {
        char configDir[256];
        snprintf(configDir, sizeof(configDir),
                 "/tmp/firestaff-applyconfig-%d", (int)time(NULL));

        M12_Config cfg;
        M12_Config_SetDefaults(&cfg);
        cfg.cloudSyncEnabled = 1;
        cfg.cloudSyncPolicy = M12_SYNC_POLICY_CLOUD_WINS;
        snprintf(cfg.cloudSyncDir, sizeof(cfg.cloudSyncDir), "%s", configDir);

        M12_CloudSync_ApplyConfig(&cfg);

        CHECK(M12_CloudSync_IsEnabled() == 1,
              "ApplyConfig sets the opt-in flag from M12_Config.cloudSyncEnabled");
        CHECK(M12_CloudSync_GetPolicy() == M12_SYNC_POLICY_CLOUD_WINS,
              "ApplyConfig sets the policy from M12_Config.cloudSyncPolicy");
        const char* effective = M12_CloudSync_GetSyncDir();
        CHECK(effective && strcmp(effective, configDir) == 0,
              "ApplyConfig sets the sync dir from M12_Config.cloudSyncDir");

        char tag[64];
        M12_CloudSync_GetBoundaryTag(tag, sizeof(tag));
        CHECK(strcmp(tag, "ENABLED_OK") == 0,
              "ApplyConfig wiring produces ENABLED_OK for a fresh config");

        /* Flip the flag off; boundary must immediately revert. */
        cfg.cloudSyncEnabled = 0;
        M12_CloudSync_ApplyConfig(&cfg);
        M12_CloudSync_GetBoundaryTag(tag, sizeof(tag));
        CHECK(strcmp(tag, "DISABLED") == 0,
              "ApplyConfig respects a re-disabled flag at next boundary check");
    }

    /* 15. DiscoverSaveFiles picks up *.sav from a real directory. */
    {
        /* Build a temp saves tree with two *.sav files and one non-sav. */
        char root[256];
        snprintf(root, sizeof(root), "/tmp/firestaff-discover-%d", (int)time(NULL));
        char gameDir[512];
        snprintf(gameDir, sizeof(gameDir), "%s/saves/dm1", root);
        CHECK(m12_make_dir(gameDir), "Test setup: create saves dir");

        char fileA[512], fileB[512], fileC[512];
        snprintf(fileA, sizeof(fileA), "%s/hero-a.sav", gameDir);
        snprintf(fileB, sizeof(fileB), "%s/hero-b.sav", gameDir);
        snprintf(fileC, sizeof(fileC), "%s/not-a-save.txt", gameDir);
        FILE* fp = fopen(fileA, "wb"); if (fp) { fwrite("AAA", 1, 3, fp); fclose(fp); }
        fp = fopen(fileB, "wb"); if (fp) { fwrite("BBB", 1, 3, fp); fclose(fp); }
        fp = fopen(fileC, "wb"); if (fp) { fwrite("CCC", 1, 3, fp); fclose(fp); }

        /* Point cloud sync at a fresh sync dir to keep state isolated. */
        char syncDir[256];
        snprintf(syncDir, sizeof(syncDir),
                 "/tmp/firestaff-discover-sync-%d", (int)time(NULL));
        M12_CloudSync_SetSyncDir(syncDir);
        M12_CloudSync_SetEnabled(1);

        int added = M12_CloudSync_DiscoverSaveFiles("saves/dm1", gameDir);
        CHECK(added == 2,
              "DiscoverSaveFiles adds 2 entries (only *.sav, no .txt)");

        /* Re-run should be idempotent (no new entries). */
        int added2 = M12_CloudSync_DiscoverSaveFiles("saves/dm1", gameDir);
        CHECK(added2 == 0,
              "DiscoverSaveFiles is idempotent on second call");

        /* Manifest should now contain the two entries. */
        M12_SyncManifest m;
        memset(&m, 0, sizeof(m));
        int loaded = M12_CloudSync_LoadManifest(&m);
        CHECK(loaded == 1, "Manifest loads after discovery");
        int foundA = 0, foundB = 0, foundC = 0;
        for (int i = 0; i < m.entryCount; ++i) {
            if (strcmp(m.entries[i].relativePath, "saves/dm1/hero-a.sav") == 0) foundA = 1;
            if (strcmp(m.entries[i].relativePath, "saves/dm1/hero-b.sav") == 0) foundB = 1;
            if (strcmp(m.entries[i].relativePath, "saves/dm1/not-a-save.txt") == 0) foundC = 1;
        }
        CHECK(foundA && foundB && !foundC,
              "Manifest contains only the *.sav entries (no .txt leak)");

        /* Clean up the temp files. */
        (void)remove(fileA);
        (void)remove(fileB);
        (void)remove(fileC);
    }

    /* Restore the user's real state. */
    M12_CloudSync_SetEnabled(savedEnabled);

    /* Summary */
    printf("\n=== Results: %d pass, %d fail ===\n", g_pass, g_fail);
    if (g_fail > 0) {
        printf("PROBE FAILED\n");
        return 1;
    }
    printf("PROBE PASSED\n");
    return 0;
}
