#ifndef ASSET_FIND_BY_HASH_H
#define ASSET_FIND_BY_HASH_H

/*
 * Hash-based asset file discovery.
 *
 * Instead of hardcoding filenames, scan data directories recursively
 * and match files by their MD5 hash. This handles platform-specific
 * naming (DUNGEON.DAT vs Dungeon.DAT vs DM2DUNGEON.DAT) and
 * arbitrary directory layouts.
 */

#define ASSET_PATH_MAX 512

/*
 * Search for a file matching the given MD5 hash.
 *
 * Scans `searchDir` and up to `maxDepth` levels of subdirectories.
 * On match, copies the full path to `outPath` and returns 1.
 * Returns 0 if no match found.
 *
 * Uses the same MD5 implementation as asset_status_m12.c.
 */
int asset_find_by_md5(const char *searchDir, const char *expectedMd5,
                      char *outPath, int outPathLen, int maxDepth);

/*
 * Search for a file matching ANY of the given MD5 hashes.
 *
 * `md5List` is a NULL-terminated array of MD5 hex strings.
 * On match, copies the path to `outPath` and the matched hash index
 * to `outMatchIndex` (if non-NULL). Returns 1 on match, 0 otherwise.
 */
int asset_find_by_md5_list(const char *searchDir, const char *const *md5List,
                           char *outPath, int outPathLen,
                           int *outMatchIndex, int maxDepth);

#endif /* ASSET_FIND_BY_HASH_H */
