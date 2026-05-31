/*
 * firestaff_m12_cloud_sync_probe.c
 *
 * Headless verification probe for cloud_sync_m12.
 *
 * No game data required. Tests:
 *   1. M12_CloudSync_GetSyncDir returns a non-NULL path
 *   2. M12_CloudSync_SetSyncDir / GetSyncDir round-trip
 *   3. M12_CloudSync_LoadManifest / SaveManifest round-trip
 *   4. M12_CloudSync_TrackPath / GetLastSyncTime round-trip
 *   5. M12_CloudSync_NeedsSync returns 1 when no manifest exists
 *   6. M12_CloudSync_Run(BOTH) returns 0 with fresh sync dir
 *   7. M12_CloudSync_RescanManifest succeeds
 *   8. M12_CloudSync_GetPolicy / SetPolicy round-trip
 *   9. Cloud sync module links into firestaff_m12 library
 */

#include "cloud_sync_m12.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

int main(void) {
    printf("\n=== firestaff_m12_cloud_sync_probe ===\n\n");

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
        /* Clean up */
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
        M12_CloudSync_SetSyncDir(dir);  /* ensure dir exists */
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

        /* Track a path */
        int tracked = M12_CloudSync_TrackPath("test-saves/dm1/save1.sav", 1);
        CHECK(tracked == 1, "TrackPath returns 1 when adding path");

        /* GetLastSyncTime for tracked path */
        uint32_t t = M12_CloudSync_GetLastSyncTime("test-saves/dm1/save1.sav");
        CHECK(t == 0, "GetLastSyncTime returns 0 for newly-tracked path (never synced)");

        /* Untrack */
        int untracked = M12_CloudSync_TrackPath("test-saves/dm1/save1.sav", 0);
        CHECK(untracked == 1, "TrackPath returns 1 when removing path");
    }

    /* 5. NeedsSync when no manifest */
    {
        const char* dir = M12_CloudSync_GetSyncDir();
        char manifestPath[256];
        snprintf(manifestPath, sizeof(manifestPath), "%s/manifest.json", dir);

        /* Remove manifest if present */
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
        /* No local or sync files exist yet, so stats should be zero */
    }

    /* Summary */
    printf("\n=== Results: %d pass, %d fail ===\n", g_pass, g_fail);
    if (g_fail > 0) {
        printf("PROBE FAILED\n");
        return 1;
    }
    printf("PROBE PASSED\n");
    return 0;
}