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
 *   - Per-game savegames: saves/{game}/<name>.sav
 *
 * The sync root may be placed on a cloud-backed filesystem (Dropbox,
 * OneDrive, Google Drive, iCloud, rsync target, etc.) by pointing the
 * environment variable FIRESTAFF_SYNC_DIR at it.
 */

#ifndef FIRESTAFF_CLOUD_SYNC_M12_H
#define FIRESTAFF_CLOUD_SYNC_M12_H

#include <stddef.h>
#include <stdint.h>
#include "config_m12.h"

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
    M12_SYNC_OK_DISABLED        = 1,  /* explicit no-op: opted out via config */
    M12_SYNC_ERR_NO_SYNC_DIR    = -1,
    M12_SYNC_ERR_MANIFEST_WRITE = -2,
    M12_SYNC_ERR_FILE_COPY      = -3,
    M12_SYNC_ERR_CONFLICT_UNRESOLVED = -4,
    M12_SYNC_ERR_SYNC_DIR_CREATE = -5,
    M12_SYNC_ERR_UNSUPPORTED    = -6, /* sync root unusable / not allowed */
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

/* ── Opt-in / opt-out boundary ─────────────────────────────────────
 *
 * The launcher is the source of truth for whether cloud sync is
 * enabled. We honour an explicit config flag (`enabled` must be 1)
 * and refuse to touch the sync root if it is not writable or does
 * not look like a directory. Callers that need a one-shot run that
 * is conflict-safe and a strict no-op when disabled should use
 * `M12_CloudSync_RunIfEnabled`, which never opens files unless
 * the opt-in is on and the sync root is usable.
 *
 * M12_CloudSync_Run remains the raw, no-policy version for tooling
 * and tests. */

/* Set the explicit opt-in flag (0 = disabled, 1 = enabled).
 * Independent of any M12_Config field; the launcher wires them
 * together at startup. */
void M12_CloudSync_SetEnabled(int enabled);

/* Get the current opt-in flag. Defaults to 0 (off). */
int M12_CloudSync_IsEnabled(void);

/* Apply an M12_Config snapshot to the cloud sync runtime. Sets
 * the opt-in flag, the configured sync dir, and the policy from
 * the config struct in one call. This is the bounded wiring point
 * the launcher uses at startup and after every settings change.
 * Does not perform any sync; the sync root is not even validated
 * here. */
void M12_CloudSync_ApplyConfig(const M12_Config* config);

/* Validate the configured sync root without writing anything.
 * Returns 1 if a sync would be possible, 0 if the sync root is
 * missing, unwritable, or otherwise unusable. */
int M12_CloudSync_IsSyncRootUsable(void);

/* Describe the current sync boundary as a short stable tag
 * (e.g. "DISABLED", "ENABLED_OK", "ENABLED_NO_ROOT",
 * "ENABLED_UNWRITABLE"). Buffer must be at least 32 bytes. */
void M12_CloudSync_GetBoundaryTag(char* out, size_t outSize);

/* Run a sync cycle, but only if explicitly enabled AND the sync
 * root is usable. When disabled or unusable, returns
 * M12_SYNC_OK_DISABLED (or M12_SYNC_ERR_UNSUPPORTED) and writes
 * a zeroed stats struct; the manifest, sync dir, and tracked
 * files are never touched. This is the safe entry point the
 * launcher calls at startup / shutdown. */
int M12_CloudSync_RunIfEnabled(int direction, M12_SyncStats* stats);

/* Force a full re-scan of the sync manifest (re-hash all tracked files).
 * Useful after resolving conflicts manually. */
int M12_CloudSync_RescanManifest(void);

/* Walk a save directory and append any *.sav files as tracked
 * manifest entries. Returns the number of new entries added.
 * `relativePrefix` is the manifest-relative root (e.g. "saves/dm1");
 * `absoluteDir` is the on-disk directory to scan (e.g. the user's
 * ~/.firestaff/saves/dm1 path). This is a bounded directory walk:
 * it inspects one level of files only, no recursion, and stops
 * at M12_SYNC_MAX_ENTRIES total. Manifest-relative paths are
 * sandboxed: absolute paths, drive-qualified paths, backslashes,
 * "." segments, and ".." segments are rejected. */
int M12_CloudSync_DiscoverSaveFiles(const char* relativePrefix,
                                    const char* absoluteDir);

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
 * path: relative to sync root. Absolute paths, drive-qualified paths,
 * backslashes, "." segments, and ".." segments are rejected so sync
 * entries cannot escape the launcher-owned sync/local roots.
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
