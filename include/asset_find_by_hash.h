#ifndef ASSET_FIND_BY_HASH_H
#define ASSET_FIND_BY_HASH_H

/*
 * Hash-based asset file discovery.
 *
 * Instead of hardcoding filenames, scan data directories recursively
 * and match files by their MD5 hash. This handles platform-specific
 * naming (DUNGEON.DAT vs Dungeon.DAT vs DM2DUNGEON.DAT) and
 * arbitrary directory layouts. Supported archive/disc containers are
 * scanned by entry hash and returned as virtual paths.
 */

#define ASSET_PATH_MAX 512

/* Verify one ordinary file against an expected lowercase MD5 hex digest.
 * Virtual archive paths are not accepted by this direct-file helper. */
int asset_file_matches_md5(const char *path, const char *expectedMd5);

/* Compute an ordinary file's lowercase MD5 identity.  This is intentionally
 * exposed alongside asset_file_matches_md5 so a user-selected package can be
 * pinned once and subsequently revalidated without trusting its filename. */
int asset_file_md5_hex(const char *path, char outHex[33]);

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

/*
 * Search for every MD5 hash in a NULL-terminated list with a single recursive
 * traversal. `outPaths` must contain `maxMatches` ASSET_PATH_MAX-sized rows.
 * `outMatched`, when non-NULL, receives 1 for each hash that was found and 0
 * otherwise. Returns the number of matched hashes.
 */
int asset_find_all_by_md5_list(const char *searchDir, const char *const *md5List,
                               char outPaths[][ASSET_PATH_MAX],
                               int *outMatched, int maxMatches,
                               int maxDepth);

/*
 * Like asset_find_all_by_md5_list(), but hashes ordinary files only.  This is
 * useful for a game-specific first pass: a verified loose-file package must
 * not cause unrelated archive containers to be opened.  Archive-only media
 * remains discoverable through asset_find_all_by_md5_list().
 */
int asset_find_all_files_by_md5_list(const char *searchDir,
                                     const char *const *md5List,
                                     char outPaths[][ASSET_PATH_MAX],
                                     int *outMatched, int maxMatches,
                                     int maxDepth);

/*
 * Materialize a virtual container path returned by asset_find_by_md5(), for
 * example "game-data.zip::dm2/GRAPHICS.DAT" or "disc.iso::DUNGEON.DAT".
 * Writes the uncompressed/extracted entry to outFilePath and returns 1 on
 * success. Returns 0 for non-virtual paths, unsupported compression methods,
 * missing entries, or write failures.
 */
int asset_extract_virtual_path(const char *virtualPath, const char *outFilePath);

/*
 * Missing-extractor diagnostics.
 *
 * When the scan meets game media that needs a host extraction tool (.7z,
 * .rar, .cab, CHD, ...) and that tool is unavailable, the media is skipped
 * and recorded here so the launcher and --scan-data output can tell the
 * user which archives were skipped and which tool would unlock them. The
 * store is bounded (16 entries), deduplicated by archive path, and
 * process-global; clear it before a fresh multi-game scan pass.
 * *_path() / *_tools() return NULL for out-of-range indices. *_tools()
 * returns a static "preferred/fallback" tool list such as "7zz/7z/bsdtar"
 * or "chdman".
 */
void asset_scan_clear_missing_extractor_diagnostics(void);
int asset_scan_missing_extractor_count(void);
const char *asset_scan_missing_extractor_path(int index);
const char *asset_scan_missing_extractor_tools(int index);

#endif /* ASSET_FIND_BY_HASH_H */
