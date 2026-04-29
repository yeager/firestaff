#ifndef FIRESTAFF_FS_PORTABLE_COMPAT_H
#define FIRESTAFF_FS_PORTABLE_COMPAT_H

/*
 * fs_portable_compat — cross-platform filesystem helpers for Firestaff.
 *
 * Replaces ad-hoc getenv/snprintf path wiring that was duplicated across
 * config_m12.c, asset_status_m12.c, card_art_m12.c, and m11_game_view.c.
 *
 * Platform support: macOS, Linux, Windows (MSVC + MinGW).
 * No dynamic allocation — all output goes into caller-owned buffers.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum path length used by all FSP buffers. */
#define FSP_PATH_MAX 1024

/* ── Path manipulation ──────────────────────────────────────────────── */

/*
 * Join two path segments with the platform path separator.
 * Avoids double separators.  Returns 1 on success, 0 on error/truncation.
 */
int FSP_JoinPath(char* out, size_t outSize,
                 const char* left, const char* right);

/*
 * Extract the parent directory of `path` into `out`.
 * Returns 1 on success, 0 if no separator found or error.
 */
int FSP_ParentDir(char* out, size_t outSize, const char* path);

/*
 * Normalize path separators in-place to the platform native separator.
 * On Windows: '/' → '\\'.  On POSIX: '\\' → '/'.
 * Returns the input pointer for convenience, or NULL on error.
 */
char* FSP_NormalizeSeparators(char* path);

/* ── Filesystem queries ─────────────────────────────────────────────── */

/*
 * Check whether `path` exists (file or directory).
 * Returns 1 if it exists, 0 otherwise.
 */
int FSP_PathExists(const char* path);

/*
 * Check whether `path` exists and is a regular file.
 * Returns 1 if it is a regular file, 0 otherwise.
 */
int FSP_FileExists(const char* path);

/*
 * Check whether `path` exists and is a directory.
 * Returns 1 if it is a directory, 0 otherwise.
 */
int FSP_DirExists(const char* path);

/* ── Directory creation ─────────────────────────────────────────────── */

/*
 * Create a single directory level.  Succeeds if the directory already exists.
 * Returns 1 on success, 0 on failure.
 */
int FSP_CreateDirectory(const char* path);

/*
 * Create nested directories (like `mkdir -p`).
 * Returns 1 on success, 0 on failure.
 */
int FSP_CreateDirectoryRecursive(const char* path);

/* ── Standard locations ─────────────────────────────────────────────── */

/*
 * Get the user data directory for Firestaff.
 *   macOS:   ~/Library/Application Support/Firestaff
 *   Linux:   $XDG_DATA_HOME/firestaff  (default ~/.local/share/firestaff)
 *   Windows: %APPDATA%\Firestaff
 * Returns 1 on success, 0 on error.
 */
int FSP_GetUserDataDir(char* out, size_t outSize);

/*
 * Get the user config directory for Firestaff.
 *   macOS:   ~/Library/Application Support/Firestaff
 *   Linux:   $XDG_CONFIG_HOME/firestaff  (default ~/.config/firestaff)
 *   Windows: %APPDATA%\Firestaff
 * Returns 1 on success, 0 on error.
 */
int FSP_GetUserConfigDir(char* out, size_t outSize);

/*
 * Get the default originals directory for retail game files.
 *   macOS/Linux: ~/.firestaff/originals
 *   Windows:     <installation-directory>\originals
 * Returns 1 on success, 0 on error.
 */
int FSP_GetDefaultOriginalsDir(char* out, size_t outSize);

/*
 * Resolve the game data directory.  Priority:
 *   1. `requestedDir` if non-NULL and non-empty
 *   2. FIRESTAFF_DATA environment variable
 *   3. Existing legacy ~/.firestaff/data on POSIX
 *   4. <user-data-dir>/data
 *   5. Current directory "."
 * Returns 1 on success, 0 on error.
 */
int FSP_ResolveDataDir(char* out, size_t outSize, const char* requestedDir);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FS_PORTABLE_COMPAT_H */
