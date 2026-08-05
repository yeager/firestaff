#ifndef FIRESTAFF_SCAN_CACHE_H
#define FIRESTAFF_SCAN_CACHE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Persistent file hash cache for the asset scanner.
 * Stores (path, mtime, size) → md5 so unchanged files skip re-hashing.
 * Saved to ~/.firestaff/cache/asset_scan_cache.dat */

#define SCAN_CACHE_MAX_ENTRIES 4096

typedef struct {
    char     path[512];
    int64_t  mtime;
    int64_t  size;
    char     md5[33];
} ScanCacheEntry;

typedef struct {
    ScanCacheEntry entries[SCAN_CACHE_MAX_ENTRIES];
    int count;
    int dirty;
} ScanCache;

void scan_cache_init(ScanCache *cache);

/* Load cache from ~/.firestaff/cache/asset_scan_cache.dat.
 * Returns 0 on success, -1 if file doesn't exist or is corrupt. */
int scan_cache_load(ScanCache *cache);

/* Save cache to disk. Only writes if dirty. Returns 0 on success. */
int scan_cache_save(const ScanCache *cache);

/* Look up a cached MD5 for a file. Returns 1 if found and file
 * hasn't changed (same mtime + size), 0 otherwise. */
int scan_cache_lookup(const ScanCache *cache,
                      const char *path, int64_t mtime, int64_t size,
                      char out_md5[33]);

/* Insert or update a cache entry. */
void scan_cache_put(ScanCache *cache,
                    const char *path, int64_t mtime, int64_t size,
                    const char *md5);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SCAN_CACHE_H */
