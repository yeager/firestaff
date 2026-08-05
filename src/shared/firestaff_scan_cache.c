#include "firestaff_scan_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

static const char *get_home_dir(void) {
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (!home) home = getenv("USERPROFILE");
#else
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
#endif
    return home;
}

static int cache_path(char *buf, size_t len) {
    const char *home = get_home_dir();
    if (!home) return -1;
    snprintf(buf, len, "%s/.firestaff/cache/asset_scan_cache.dat", home);
    return 0;
}

static void ensure_cache_dir(void) {
    char dir[512];
    const char *home = get_home_dir();
    if (!home) return;
    snprintf(dir, sizeof(dir), "%s/.firestaff", home);
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
    snprintf(dir, sizeof(dir), "%s/.firestaff/cache", home);
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0755);
#endif
}

void scan_cache_init(ScanCache *cache) {
    if (!cache) return;
    memset(cache, 0, sizeof(*cache));
}

int scan_cache_load(ScanCache *cache) {
    char path[512];
    FILE *f;

    if (!cache) return -1;
    scan_cache_init(cache);

    if (cache_path(path, sizeof(path)) != 0) return -1;
    f = fopen(path, "rb");
    if (!f) return -1;

    char line[640];
    while (cache->count < SCAN_CACHE_MAX_ENTRIES &&
           fgets(line, sizeof(line), f)) {
        ScanCacheEntry *e = &cache->entries[cache->count];
        char *tab1 = strchr(line, '\t');
        if (!tab1) continue;
        char *tab2 = strchr(tab1 + 1, '\t');
        if (!tab2) continue;
        char *tab3 = strchr(tab2 + 1, '\t');
        if (!tab3) continue;

        size_t pathlen = (size_t)(tab1 - line);
        if (pathlen >= sizeof(e->path)) continue;
        memcpy(e->path, line, pathlen);
        e->path[pathlen] = '\0';

        e->mtime = strtoll(tab1 + 1, NULL, 10);
        e->size = strtoll(tab2 + 1, NULL, 10);

        size_t md5len = strlen(tab3 + 1);
        while (md5len > 0 && (tab3[md5len] == '\n' || tab3[md5len] == '\r'))
            md5len--;
        if (md5len < 32) continue;
        memcpy(e->md5, tab3 + 1, 32);
        e->md5[32] = '\0';

        cache->count++;
    }
    fclose(f);
    return 0;
}

int scan_cache_save(const ScanCache *cache) {
    char path[512];
    FILE *f;

    if (!cache || !cache->dirty) return 0;
    if (cache_path(path, sizeof(path)) != 0) return -1;

    ensure_cache_dir();
    f = fopen(path, "wb");
    if (!f) return -1;

    for (int i = 0; i < cache->count; i++) {
        const ScanCacheEntry *e = &cache->entries[i];
        fprintf(f, "%s\t%lld\t%lld\t%s\n",
                e->path, (long long)e->mtime, (long long)e->size, e->md5);
    }
    fclose(f);
    return 0;
}

int scan_cache_lookup(const ScanCache *cache,
                      const char *path, int64_t mtime, int64_t size,
                      char out_md5[33]) {
    if (!cache || !path) return 0;
    for (int i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].path, path) == 0 &&
            cache->entries[i].mtime == mtime &&
            cache->entries[i].size == size) {
            memcpy(out_md5, cache->entries[i].md5, 33);
            return 1;
        }
    }
    return 0;
}

void scan_cache_put(ScanCache *cache,
                    const char *path, int64_t mtime, int64_t size,
                    const char *md5) {
    if (!cache || !path || !md5) return;

    for (int i = 0; i < cache->count; i++) {
        if (strcmp(cache->entries[i].path, path) == 0) {
            cache->entries[i].mtime = mtime;
            cache->entries[i].size = size;
            memcpy(cache->entries[i].md5, md5, 33);
            cache->dirty = 1;
            return;
        }
    }

    if (cache->count < SCAN_CACHE_MAX_ENTRIES) {
        ScanCacheEntry *e = &cache->entries[cache->count++];
        strncpy(e->path, path, sizeof(e->path) - 1);
        e->path[sizeof(e->path) - 1] = '\0';
        e->mtime = mtime;
        e->size = size;
        memcpy(e->md5, md5, 33);
        cache->dirty = 1;
    }
}
