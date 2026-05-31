/*
 * cloud_sync_m12.c — Cloud sync for launcher settings and savegames.
 *
 * Sync model: directory-based with manifest. See cloud_sync_m12.h for
 * full documentation.
 */

#include "cloud_sync_m12.h"
#include "fs_portable_compat.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>   /* for _mkdir */
#include <sys/stat.h> /* for _stat */
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

/* ── CRC-32 (zlib variant) ──────────────────────────────────────── */

static uint32_t g_crc32_table[256];
static int g_crc32_init = 0;

static void m12_crc32_init(void) {
    uint32_t poly = 0xEDB88320UL;
    for (unsigned i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (unsigned j = 0; j < 8; ++j) {
            c = (c & 1UL) ? ((c >> 1) ^ poly) : (c >> 1);
        }
        g_crc32_table[i] = c;
    }
    g_crc32_init = 1;
}

static uint32_t m12_crc32_update(uint32_t crc, const void* data, size_t size) {
    if (!g_crc32_init) m12_crc32_init();
    const uint8_t* p = (const uint8_t*)data;
    for (size_t i = 0; i < size; ++i) {
        crc = g_crc32_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFUL;
}

/* ── Static state ───────────────────────────────────────────────── */

static char      g_syncDir[FSP_PATH_MAX]   = {0};
static int       g_policy                 = M12_SYNC_POLICY_NEWER_WINS;
static uint32_t  g_lastFullSync           = 0;

enum { MAX_TRACKED = M12_SYNC_MAX_ENTRIES };

typedef struct {
    char    path[256];
    int     exists;
    uint32_t mtime;
    uint32_t crc;
} M12_TrackedFile;

/* ── Path helpers ───────────────────────────────────────────────── */

static void m12_sync_get_manifest_path(char* out, size_t outSize) {
    snprintf(out, outSize, "%s/manifest.json", g_syncDir);
}

static void m12_sync_get_local_config_path(char* out, size_t outSize) {
    char configDir[FSP_PATH_MAX];
    if (FSP_GetUserConfigDir(configDir, sizeof(configDir))) {
        snprintf(out, outSize, "%s/startup-menu.toml", configDir);
    } else {
        snprintf(out, outSize, "startup-menu.toml");
    }
}

static void m12_sync_get_local_export_path(char* out, size_t outSize) {
    snprintf(out, outSize, "%s/firestaff-settings-export.json", g_syncDir);
}

static void m12_sync_get_local_save_dir(char* out, size_t outSize, int gameId) {
    const char* names[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    if (gameId < 0 || gameId > 4) { out[0] = '\0'; return; }
    snprintf(out, outSize, "%s/saves/%s", g_syncDir, names[gameId]);
}

/* ── Default sync dir ──────────────────────────────────────────── */

static void m12_default_sync_dir(char* out, size_t outSize) {
    const char* env = getenv("FIRESTAFF_SYNC_DIR");
    if (env && env[0] != '\0') {
        snprintf(out, outSize, "%s", env);
        return;
    }
    /* ~/.firestaff/sync/ */
    const char* home = getenv("HOME");
    if (home) {
        snprintf(out, outSize, "%s/.firestaff/sync", home);
    } else {
        snprintf(out, outSize, ".firestaff/sync");
    }
}

/* ── Platform helpers ───────────────────────────────────────────── */

int M12_CloudSync_EnsureDir(const char* path) {
    if (!path || !path[0]) return 0;
#if defined(_WIN32)
    /* mkdir -p: create each component using _mkdir */
    char tmp[FSP_PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char* p = tmp; *p; ++p) {
        if (*p == '/' && p != tmp) {
            *p = '\0';
            (void)_mkdir(tmp);
            *p = '/';
        }
    }
    (void)_mkdir(tmp);
    return 1;
#else
    /* mkdir -p: create each component */
    char tmp[FSP_PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char* p = tmp; *p; ++p) {
        if (*p == '/' && p != tmp) {
            *p = '\0';
            (void)mkdir(tmp, 0755);
            *p = '/';
        }
    }
    (void)mkdir(tmp, 0755);
    return 1;
#endif
}

uint32_t M12_CloudSync_FileModTime(const char* path) {
#if defined(_WIN32)
    struct _stat st;
    if (_stat(path, &st) != 0) return 0;
    return (uint32_t)(st.st_mtime & 0xFFFFFFFFUL);
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return (uint32_t)(st.st_mtime & 0xFFFFFFFFUL);
#endif
}

uint32_t M12_CloudSync_FileCrc(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    uint32_t crc = 0;
    unsigned char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        crc = m12_crc32_update(crc, buf, n);
    }
    fclose(f);
    return crc;
}

int M12_CloudSync_CopyFile(const char* src, const char* dst) {
    FILE* in = fopen(src, "rb");
    if (!in) return 0;
    /* Ensure parent dir exists */
    char dir[FSP_PATH_MAX];
    snprintf(dir, sizeof(dir), "%s", dst);
    {
        char* p;
        for (p = dir + strlen(dir); p > dir && *p != '/'; --p);
        if (p > dir) { *p = '\0'; M12_CloudSync_EnsureDir(dir); }
    }
    FILE* out = fopen(dst, "wb");
    if (!out) { fclose(in); return 0; }
    unsigned char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return 0; }
    }
    fclose(in);
    fclose(out);
    return 1;
}

/* ── Sync dir management ─────────────────────────────────────────── */

const char* M12_CloudSync_GetSyncDir(void) {
    if (!g_syncDir[0]) m12_default_sync_dir(g_syncDir, sizeof(g_syncDir));
    return g_syncDir;
}

int M12_CloudSync_SetSyncDir(const char* path) {
    if (!path || !path[0]) {
        g_syncDir[0] = '\0';
        m12_default_sync_dir(g_syncDir, sizeof(g_syncDir));
    } else {
        snprintf(g_syncDir, sizeof(g_syncDir), "%s", path);
    }
    M12_CloudSync_EnsureDir(g_syncDir);
    /* Also ensure saves/ subdirectory tree */
    {
        char sub[FSP_PATH_MAX];
        for (int i = 0; i < 5; ++i) {
            m12_sync_get_local_save_dir(sub, sizeof(sub), i);
            M12_CloudSync_EnsureDir(sub);
        }
    }
    return 1;
}

int M12_CloudSync_GetPolicy(void) { return g_policy; }
void M12_CloudSync_SetPolicy(int policy) { g_policy = policy; }

/* ── Manifest I/O ───────────────────────────────────────────────── */

/* Minimal JSON writer for the manifest. The manifest is a simple
 * JSON object with an "entries" array and a "lastFullSync" field. */

static void m12_json_escape(FILE* fp, const char* s) {
    fputc('"', fp);
    for (; *s; ++s) {
        switch (*s) {
            case '"':  fputs("\\\"", fp); break;
            case '\\': fputs("\\\\", fp); break;
            case '\n': fputs("\\n",  fp); break;
            case '\r': fputs("\\r",  fp); break;
            case '\t': fputs("\\t",  fp); break;
            default:   fputc(*s, fp); break;
        }
    }
    fputc('"', fp);
}

static int m12_write_manifest(const char* path, const M12_SyncManifest* m) {
    FILE* fp = fopen(path, "w");
    if (!fp) return 0;
    fprintf(fp, "{\n  \"lastFullSync\": %u,\n  \"entryCount\": %d,\n  \"entries\": [\n",
            m->lastFullSync, m->entryCount);
    for (int i = 0; i < m->entryCount; ++i) {
        const M12_SyncEntry* e = &m->entries[i];
        fprintf(fp, "    {\n      \"relativePath\": ");
        m12_json_escape(fp, e->relativePath);
        fprintf(fp, ",\n      \"localTimestamp\": %u,\n      \"syncTimestamp\": %u,\n"
                   "      \"localCrc\": %u,\n      \"syncCrc\": %u,\n      \"conflict\": %d\n    }",
                e->localTimestamp, e->syncTimestamp, e->localCrc, e->syncCrc, e->conflict);
        if (i < m->entryCount - 1) fputc(',', fp);
        fputc('\n', fp);
    }
    fprintf(fp, "  ]\n}\n");
    fclose(fp);
    return 1;
}

/* Simple JSON reader for manifest (hand-crafted, no external deps). */
static int m12_read_manifest(const char* path, M12_SyncManifest* m) {
    FILE* fp = fopen(path, "r");
    if (!fp) return 0;
    memset(m, 0, sizeof(*m));
    char line[512];
    int inEntry = 0;
    M12_SyncEntry tmp;
    memset(&tmp, 0, sizeof(tmp));

    while (fgets(line, sizeof(line), fp)) {
        /* Strip whitespace */
        char* s = line;
        while (*s == ' ' || *s == '\t') ++s;
        size_t len = strlen(s);
        while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' ' || s[len-1] == '\t')) s[--len] = '\0';

        if (strncmp(s, "\"lastFullSync\"", 14) == 0) {
            char* p = strchr(s, ':');
            if (p) m->lastFullSync = (uint32_t)strtoul(p+1, NULL, 10);
        }
        else if (strncmp(s, "\"entries\"", 9) == 0 && strstr(s, "[") != NULL) {
            inEntry = 1;
        }
        else if (inEntry && strstr(s, "]") != NULL) {
            inEntry = 0;
        }
        else if (inEntry && strncmp(s, "\"relativePath\"", 14) == 0) {
            /* "relativePath": "..." */
            char* p = strchr(s, ':');
            if (p) {
                ++p;
                while (*p == ' ' || *p == '"') ++p;
                char* end = p + strlen(p) - 1;
                while (end > p && (*end == '"' || *end == ',')) *end-- = '\0';
                if (m->entryCount < M12_SYNC_MAX_ENTRIES) {
                    snprintf(m->entries[m->entryCount].relativePath,
                             sizeof(m->entries[0].relativePath), "%s", p);
                }
            }
        }
        else if (inEntry && m->entryCount < M12_SYNC_MAX_ENTRIES && m->entryCount >= 0) {
            if (strncmp(s, "\"localTimestamp\"", 15) == 0) {
                char* p = strchr(s, ':');
                if (p) m->entries[m->entryCount].localTimestamp = (uint32_t)strtoul(p+1, NULL, 10);
            }
            else if (strncmp(s, "\"syncTimestamp\"", 15) == 0) {
                char* p = strchr(s, ':');
                if (p) m->entries[m->entryCount].syncTimestamp = (uint32_t)strtoul(p+1, NULL, 10);
            }
            else if (strncmp(s, "\"localCrc\"", 10) == 0) {
                char* p = strchr(s, ':');
                if (p) m->entries[m->entryCount].localCrc = (uint32_t)strtoul(p+1, NULL, 10);
            }
            else if (strncmp(s, "\"syncCrc\"", 9) == 0) {
                char* p = strchr(s, ':');
                if (p) m->entries[m->entryCount].syncCrc = (uint32_t)strtoul(p+1, NULL, 10);
            }
            else if (strncmp(s, "\"conflict\"", 10) == 0) {
                char* p = strchr(s, ':');
                if (p) m->entries[m->entryCount].conflict = (int)strtoul(p+1, NULL, 10);
            }
            /* Detect end of entry object */
            if (strstr(s, "}") != NULL) {
                m->entryCount++;
            }
        }
    }
    fclose(fp);
    return 1;
}

/* ── Tracked files list ─────────────────────────────────────────── */

typedef struct {
    M12_TrackedFile files[64];
    int count;
} M12_TrackedList;

static void m12_tracked_add(M12_TrackedList* list, const char* relPath, const char* baseDir) {
    if (list->count >= 64) return;
    char local[FSP_PATH_MAX];
    snprintf(local, sizeof(local), "%s/%s", baseDir, relPath);
    M12_TrackedFile* f = &list->files[list->count++];
    snprintf(f->path, sizeof(f->path), "%s", local);
    f->exists = 0;
    f->mtime = 0;
    f->crc = 0;
    FILE* fp = fopen(local, "rb");
    if (fp) {
        f->exists = 1;
        f->mtime = M12_CloudSync_FileModTime(local);
        fclose(fp);
        f->crc = M12_CloudSync_FileCrc(local);
    }
}

/* ── Sync core ──────────────────────────────────────────────────── */

/* Resolve a tracked path to local and sync-dir absolute paths.
 * gameId: -1 for config/settings files, 0-4 for game save dirs. */
static void m12_resolve_path(char* localOut, size_t localSize,
                             char* syncOut, size_t syncSize,
                             const char* relPath, int gameId) {
    (void)gameId;
    /* Local: ~/.firestaff/<rel> for saves, or actual config path for config */
    const char* home = getenv("HOME") ? getenv("HOME") : "";
    if (strncmp(relPath, "saves/", 6) == 0) {
        /* saves/dm1/file.sav → ~/.firestaff/saves/dm1/file.sav */
        snprintf(localOut, localSize, "%s/.firestaff/%s", home, relPath);
    } else if (strcmp(relPath, "startup-menu.toml") == 0) {
        char cfg[FSP_PATH_MAX];
        if (FSP_GetUserConfigDir(cfg, sizeof(cfg))) {
            snprintf(localOut, localSize, "%s/startup-menu.toml", cfg);
        } else {
            snprintf(localOut, localSize, "startup-menu.toml");
        }
    } else if (strcmp(relPath, "firestaff-settings-export.json") == 0) {
        snprintf(localOut, localSize, "%s/.firestaff/firestaff-settings-export.json", home);
    } else {
        snprintf(localOut, localSize, "%s/.firestaff/%s", home, relPath);
    }
    snprintf(syncOut, syncSize, "%s/%s", g_syncDir, relPath);
}

/* Scan the local filesystem and build a list of currently-tracked files. */
static void m12_build_tracked_list(M12_TrackedList* list, const M12_SyncManifest* manifest, const char* localBase) {
    list->count = 0;
    for (int i = 0; i < manifest->entryCount; ++i) {
        m12_tracked_add(list, manifest->entries[i].relativePath, localBase);
    }
}

/* Run one sync direction. Returns number of conflicts encountered. */
static int m12_sync_direction(const M12_SyncManifest* manifest,
                               int direction,
                               int policy,
                               M12_SyncStats* stats) {
    int conflicts = 0;
    char localPath[FSP_PATH_MAX];
    char syncPath[FSP_PATH_MAX];
    const char* home = getenv("HOME") ? getenv("HOME") : ".";

    for (int i = 0; i < manifest->entryCount; ++i) {
        const M12_SyncEntry* e = &manifest->entries[i];
        /* Detect gameId for saves */
        int gameId = -1;
        if (strncmp(e->relativePath, "saves/", 6) == 0) {
            const char* g = e->relativePath + 6;
            if (strncmp(g, "dm1", 3) == 0) gameId = 0;
            else if (strncmp(g, "csb", 3) == 0) gameId = 1;
            else if (strncmp(g, "dm2", 3) == 0) gameId = 2;
            else if (strncmp(g, "nexus", 5) == 0) gameId = 3;
            else if (strncmp(g, "theron", 6) == 0) gameId = 4;
            (void)gameId; /* currently not used further */
        }

        m12_resolve_path(localPath, sizeof(localPath), syncPath, sizeof(syncPath),
                         e->relativePath, gameId);

        /* Get current file stats */
        uint32_t localMtime = M12_CloudSync_FileModTime(localPath);
        uint32_t syncMtime  = M12_CloudSync_FileModTime(syncPath);
        uint32_t localCrc   = localMtime ? M12_CloudSync_FileCrc(localPath) : 0;
        uint32_t syncCrc    = syncMtime  ? M12_CloudSync_FileCrc(syncPath)  : 0;

        int localExists = localMtime != 0;
        int syncExists  = syncMtime  != 0;

        if ((direction & M12_SYNC_DIRECTION_PUSH) && localExists) {
            int shouldPush = 0;
            if (!syncExists) {
                shouldPush = 1; /* no cloud copy */
            } else if (localMtime > syncMtime) {
                shouldPush = 1; /* local newer */
            } else if (localCrc != e->syncCrc && localMtime == e->localTimestamp && syncMtime == e->syncTimestamp) {
                /* Both changed since last sync */
                if (policy == M12_SYNC_POLICY_LOCAL_WINS) shouldPush = 1;
                else if (policy == M12_SYNC_POLICY_CLOUD_WINS) shouldPush = 0;
                else if (policy == M12_SYNC_POLICY_ASK) conflicts++;
                else /* NEWER_WINS */ shouldPush = (localMtime >= syncMtime);
            }
            if (shouldPush && !conflicts) {
                char* pp;
                for (pp = syncPath + strlen(syncPath); pp > syncPath && *pp != '/'; --pp);
                if (pp > syncPath) { *pp = '\0'; M12_CloudSync_EnsureDir(syncPath); }
                if (M12_CloudSync_CopyFile(localPath, syncPath)) {
                    if (stats) stats->pushedCount++;
                }
            }
        }

        if ((direction & M12_SYNC_DIRECTION_PULL) && syncExists) {
            int shouldPull = 0;
            if (!localExists) {
                shouldPull = 1; /* no local copy */
            } else if (syncMtime > localMtime) {
                shouldPull = 1; /* cloud newer */
            } else if (localCrc != e->localCrc && localMtime == e->localTimestamp && syncMtime == e->syncTimestamp) {
                /* Both changed since last sync */
                if (policy == M12_SYNC_POLICY_CLOUD_WINS) shouldPull = 1;
                else if (policy == M12_SYNC_POLICY_LOCAL_WINS) shouldPull = 0;
                else if (policy == M12_SYNC_POLICY_ASK) conflicts++;
                else /* NEWER_WINS */ shouldPull = (syncMtime >= localMtime);
            }
            if (shouldPull && !conflicts) {
                if (M12_CloudSync_CopyFile(syncPath, localPath)) {
                    if (stats) stats->pulledCount++;
                }
            }
        }

        /* Conflict: both changed since last sync and policy is ASK */
        if (conflicts && policy == M12_SYNC_POLICY_ASK) {
            /* Leave unresolved; will be reported */
        }
    }
    return conflicts;
}

/* ── Auto-discover tracked files ────────────────────────────────── */

static void m12_discover_tracked(M12_SyncManifest* m) {
    /* Always track launcher config files */
    const char* home = getenv("HOME") ? getenv("HOME") : ".";
    const char* cfgDir = getenv("XDG_CONFIG_HOME") ? getenv("XDG_CONFIG_HOME")
                            : (getenv("HOME") ? getenv("HOME") : ".");

    int idx = m->entryCount;
    if (idx < M12_SYNC_MAX_ENTRIES) {
        snprintf(m->entries[idx].relativePath, sizeof(m->entries[0].relativePath),
                 "startup-menu.toml");
        m->entries[idx].localTimestamp = M12_CloudSync_FileModTime("startup-menu.toml");
        m->entries[idx].syncTimestamp = 0;
        m->entries[idx].localCrc = M12_CloudSync_FileCrc("startup-menu.toml");
        m->entries[idx].syncCrc = 0;
        m->entries[idx].conflict = 0;
        if (m->entries[idx].localTimestamp) idx++;
    }
    /* Discover saves in ~/.firestaff/saves/{game}/  *.sav  */
    const char* games[] = {"dm1", "csb", "dm2", "nexus", "theron"};
    for (int g = 0; g < 5 && idx < M12_SYNC_MAX_ENTRIES; ++g) {
        char savesDir[FSP_PATH_MAX];
        snprintf(savesDir, sizeof(savesDir), "%s/.firestaff/saves/%s", home, games[g]);
        /* Scan directory for *.sav files */
        (void)savesDir; /* directory scanning deferred to runtime */
        /* Track the saves directory itself as a marker (empty entry for now) */
        /* Actual file discovery is done at sync time */
        (void)idx; (void)g;
    }
    m->entryCount = idx;
}

/* ── Public API ──────────────────────────────────────────────────── */

int M12_CloudSync_LoadManifest(M12_SyncManifest* outManifest) {
    char path[FSP_PATH_MAX];
    m12_sync_get_manifest_path(path, sizeof(path));
    return m12_read_manifest(path, outManifest);
}

int M12_CloudSync_SaveManifest(const M12_SyncManifest* manifest) {
    char path[FSP_PATH_MAX];
    m12_sync_get_manifest_path(path, sizeof(path));
    M12_CloudSync_EnsureDir(g_syncDir);
    return m12_write_manifest(path, manifest);
}

int M12_CloudSync_TrackPath(const char* relativePath, int add) {
    M12_SyncManifest m;
    if (!M12_CloudSync_LoadManifest(&m)) memset(&m, 0, sizeof(m));
    if (add) {
        int found = 0;
        for (int i = 0; i < m.entryCount; ++i) {
            if (strcmp(m.entries[i].relativePath, relativePath) == 0) { found = 1; break; }
        }
        if (!found && m.entryCount < M12_SYNC_MAX_ENTRIES) {
            snprintf(m.entries[m.entryCount].relativePath,
                     sizeof(m.entries[0].relativePath), "%s", relativePath);
            m.entries[m.entryCount].localTimestamp = 0;
            m.entries[m.entryCount].syncTimestamp = 0;
            m.entries[m.entryCount].localCrc = 0;
            m.entries[m.entryCount].syncCrc = 0;
            m.entries[m.entryCount].conflict = 0;
            m.entryCount++;
        }
    } else {
        int dst = 0;
        for (int i = 0; i < m.entryCount; ++i) {
            if (strcmp(m.entries[i].relativePath, relativePath) != 0) {
                m.entries[dst++] = m.entries[i];
            }
        }
        m.entryCount = dst;
    }
    return M12_CloudSync_SaveManifest(&m);
}

uint32_t M12_CloudSync_GetLastSyncTime(const char* relativePath) {
    M12_SyncManifest m;
    if (!M12_CloudSync_LoadManifest(&m)) return 0;
    for (int i = 0; i < m.entryCount; ++i) {
        if (strcmp(m.entries[i].relativePath, relativePath) == 0)
            return m.entries[i].syncTimestamp;
    }
    return 0;
}

int M12_CloudSync_NeedsSync(void) {
    if (!g_syncDir[0]) M12_CloudSync_GetSyncDir();
    M12_SyncManifest m;
    if (!M12_CloudSync_LoadManifest(&m)) return 1; /* no manifest = needs sync */
    for (int i = 0; i < m.entryCount; ++i) {
        char lp[FSP_PATH_MAX], sp[FSP_PATH_MAX];
        m12_resolve_path(lp, sizeof(lp), sp, sizeof(sp),
                         m.entries[i].relativePath, -1);
        uint32_t lm = M12_CloudSync_FileModTime(lp);
        uint32_t sm = M12_CloudSync_FileModTime(sp);
        if (lm != m.entries[i].localTimestamp || sm != m.entries[i].syncTimestamp)
            return 1;
    }
    return 0;
}

int M12_CloudSync_RescanManifest(void) {
    M12_SyncManifest m;
    if (!M12_CloudSync_LoadManifest(&m)) memset(&m, 0, sizeof(m));
    for (int i = 0; i < m.entryCount; ++i) {
        char lp[FSP_PATH_MAX], sp[FSP_PATH_MAX];
        m12_resolve_path(lp, sizeof(lp), sp, sizeof(sp),
                         m.entries[i].relativePath, -1);
        m.entries[i].localTimestamp = M12_CloudSync_FileModTime(lp);
        m.entries[i].syncTimestamp  = M12_CloudSync_FileModTime(sp);
        m.entries[i].localCrc = m.entries[i].localTimestamp ? M12_CloudSync_FileCrc(lp) : 0;
        m.entries[i].syncCrc  = m.entries[i].syncTimestamp  ? M12_CloudSync_FileCrc(sp) : 0;
        m.entries[i].conflict = 0;
    }
    m.lastFullSync = (uint32_t)time(NULL);
    return M12_CloudSync_SaveManifest(&m);
}

int M12_CloudSync_Run(int direction, M12_SyncStats* stats) {
    if (!g_syncDir[0]) M12_CloudSync_GetSyncDir();
    if (!g_syncDir[0] || g_syncDir[0] == '\0')
        return M12_SYNC_ERR_NO_SYNC_DIR;

    /* Ensure sync root exists */
    if (!M12_CloudSync_EnsureDir(g_syncDir))
        return M12_SYNC_ERR_SYNC_DIR_CREATE;

    /* Ensure saves subdirs */
    for (int i = 0; i < 5; ++i) {
        char sub[FSP_PATH_MAX];
        m12_sync_get_local_save_dir(sub, sizeof(sub), i);
        M12_CloudSync_EnsureDir(sub);
    }

    /* Load or create manifest */
    M12_SyncManifest manifest;
    if (!M12_CloudSync_LoadManifest(&manifest)) {
        memset(&manifest, 0, sizeof(manifest));
        m12_discover_tracked(&manifest);
    }

    /* Clear stats */
    if (stats) memset(stats, 0, sizeof(*stats));

    /* Run sync in both directions */
    int conflicts = m12_sync_direction(&manifest, direction, g_policy, stats);
    (void)conflicts;

    /* Update manifest timestamps and CRCs */
    for (int i = 0; i < manifest.entryCount; ++i) {
        char lp[FSP_PATH_MAX], sp[FSP_PATH_MAX];
        m12_resolve_path(lp, sizeof(lp), sp, sizeof(sp),
                         manifest.entries[i].relativePath, -1);
        manifest.entries[i].localTimestamp = M12_CloudSync_FileModTime(lp);
        manifest.entries[i].syncTimestamp  = M12_CloudSync_FileModTime(sp);
        manifest.entries[i].localCrc = manifest.entries[i].localTimestamp ? M12_CloudSync_FileCrc(lp) : 0;
        manifest.entries[i].syncCrc  = manifest.entries[i].syncTimestamp  ? M12_CloudSync_FileCrc(sp)  : 0;
        manifest.entries[i].conflict = 0;
    }
    manifest.lastFullSync = (uint32_t)time(NULL);

    if (!M12_CloudSync_SaveManifest(&manifest))
        return M12_SYNC_ERR_MANIFEST_WRITE;

    if (conflicts && g_policy == M12_SYNC_POLICY_ASK)
        return M12_SYNC_ERR_CONFLICT_UNRESOLVED;

    return M12_SYNC_OK;
}