/*
 * cloud_sync_m12.h — Cloud sync for launcher settings and savegames.
 *
 * Sync model: directory-based. All syncable state lives under a
 * configurable sync root (~/.firestaff/sync/ by default). A manifest
 * file (sync/manifest.json) records the last-sync timestamp and
 * content-hash for each tracked item. On each sync operation we:
 *   1. Scan local and sync-dir files.
 *   2. Push newer local → sync-dir (newer wins).
 *   3. Pull newer sync-dir → local.
 *   4. Detect conflicts (both changed since last sync) and resolve
 *      via configurable policy (default: newer wins, log conflict).
 *
 * Syncable items:
 *   • Launcher config: startup-menu.toml + firestaff-settings-export.json
 *   \- Per-game savegames: saves/{game}/*.sav
 *
 * The sync root may be placed on a cloud-backed filesystem (Dropbox,
 * OneDrive, Google Drive, iCloud, rsync target, etc.) by pointing the
 * environment variable FIRESTAFF_SYNC_DIR at it.
 */

#ifndef FIRESTAFF_CLOUD_SYNC_M12_H
#define FIRESTAFF_CLOUD_SYNC_M12_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Sync policy ──────────────────────────────────────────────────── */

enum {
    M12_SYNC_POLICY_NEWER_WINS   = 0,  /* default */
    M12_SYNC_POLICY_LOCAL_WINS   = 1,  /* always prefer local */
    M12_SYNC_POLICY_CLOUD_WINS   = 2,  /* always prefer cloud */
    M12_SYNC_POLICY_ASK          = 3,  /* unresolved conflicts block; caller resolves */
};

/* ── Sync direction ───────────────────────────────────────────────── */

enum {
    M12_SYNC_DIRECTION_PUSH   = 1 << 0,
    M12_SYNC_DIRECTION_PULL   = 1 << 1,
    M12_SYNC_DIRECTION_BOTH   = M12_SYNC_DIRECTION_PUSH | M12_SYNC_DIRECTION_PULL,
};

/* ── Sync result codes ─────────────────────────────────────────────── */

enum {
    M12_SYNC_OK                 = 0,
    M12_SYNC_ERR_NO_SYNC_DIR    = -1,
    M12_SYNC_ERR_MANIFEST_WRITE = -2,
    M12_SYNC_ERR_FILE_COPY      = -3,
    M12_SYNC_ERR_CONFLICT_UNRESOLVED = -4,
    M12_SYNC_ERR_SYNC_DIR_CREATE = -5,
};

/* ── Per-file entry in manifest ──────────────────────────────────── */

typedef struct {
    char  relativePath[256];   /* path relative to sync root */
    uint32_t localTimestamp;   /* Unix timestamp of local file */
    uint32_t syncTimestamp;    /* timestamp when last synced */
    uint32_t localCrc;         /* CRC-32 of local content at last sync */
    uint32_t syncCrc;          /* CRC-32 of sync-dir content at last sync */
    int    conflict;           /* 1 = conflict detected and unresolved */
} M12_SyncEntry;

/* ── Manifest (sync/manifest.json) ────────────────────────────────── */

enum {
    M12_SYNC_MAX_ENTRIES = 256,
};

typedef struct {
    M12_SyncEntry entries[M12_SYNC_MAX_ENTRIES];
    int           entryCount;
    uint32_t      lastFullSync;  /* Unix timestamp of last full sync */
} M12_SyncManifest;

/* ── Sync status summary (returned after sync run) ────────────────── */

typedef struct {
    int  pushedCount;
    int  pulledCount;
    int  conflictCount;
    int  errorCount;
} M12_SyncStats;

/* ── Public API ──────────────────────────────────────────────────── */

/* Get the sync root directory path.
 * Returns a caller-owned static buffer (do not free). */
const char* M12_CloudSync_GetSyncDir(void);

/* Set the sync root directory. Pass NULL to reset to default.
 * Creates the directory if it doesn't exist.
 * Returns 1 on success, 0 on failure. */
int M12_CloudSync_SetSyncDir(const char* path);

/* Get the current sync policy (M12_SYNC_POLICY_*). */
int M12_CloudSync_GetPolicy(void);

/* Set the sync policy. */
void M12_CloudSync_SetPolicy(int policy);

/* Run a sync cycle.
 * direction: M12_SYNC_DIRECTION_PUSH, PULL, or BOTH (default).
 * stats: optional out-param for sync statistics (may be NULL).
 * Returns M12_SYNC_OK on complete success, or a negative error code.
 * On M12_SYNC_ERR_CONFLICT_UNRESOLVED, caller must resolve conflicts
 * and retry with a tighter policy. */
int M12_CloudSync_Run(int direction, M12_SyncStats* stats);

/* Force a full re-scan of the sync manifest (re-hash all tracked files).
 * Useful after resolving conflicts manually. */
int M12_CloudSync_RescanManifest(void);

/* Check whether a sync is needed (at least one tracked file differs).
 * Returns 1 if sync would do something, 0 if everything is in sync. */
int M12_CloudSync_NeedsSync(void);

/* Load the current manifest from sync/manifest.json.
 * Returns 1 on success, 0 on file not found or parse error. */
int M12_CloudSync_LoadManifest(M12_SyncManifest* outManifest);

/* Save the manifest to sync/manifest.json.
 * Returns 1 on success, 0 on write error. */
int M12_CloudSync_SaveManifest(const M12_SyncManifest* manifest);

/* Explicitly add/remove tracked paths from the manifest.
 * path: relative to sync root.
 * action: 1 = add, 0 = remove.
 * Returns 1 on success. */
int M12_CloudSync_TrackPath(const char* relativePath, int add);

/* Get the last sync time for a tracked file, or 0 if not tracked.
 * relativePath: path relative to sync root. */
uint32_t M12_CloudSync_GetLastSyncTime(const char* relativePath);

/* CRC-32 of a file. Returns 0 on error and sets errno. */
uint32_t M12_CloudSync_FileCrc(const char* path);

/* Platform helper: create all intermediate directories for a path.
 * Returns 1 on success, 0 on failure. */
int M12_CloudSync_EnsureDir(const char* path);

/* Copy file from src to dst. Creates dst directory if needed.
 * Returns 1 on success, 0 on error. */
int M12_CloudSync_CopyFile(const char* src, const char* dst);

/* Get modification time of a file (Unix timestamp).
 * Returns 0 on error (file missing or inaccessible). */
uint32_t M12_CloudSync_FileModTime(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CLOUD_SYNC_M12_H */