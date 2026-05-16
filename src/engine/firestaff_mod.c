
#include "firestaff_mod.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/* Simple JSON string extraction (no full parser needed) */
static int extract_json_string(const char *json, const char *key, char *out, int max) {
    char pattern[128];
    const char *p, *start, *end;
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    p = strstr(json, pattern);
    if (!p) return 0;
    p = strchr(p + strlen(pattern), ':');
    if (!p) return 0;
    start = strchr(p, '"');
    if (!start) return 0;
    start++;
    end = strchr(start, '"');
    if (!end) return 0;
    int len = (int)(end - start);
    if (len >= max) len = max - 1;
    memcpy(out, start, len);
    out[len] = 0;
    return 1;
}

int fs_mod_scan(FS_ModManager *mgr, const char *mods_dir) {
    char path[512], json_buf[2048];
    FILE *f;

    if (!mgr || !mods_dir) return -1;
    memset(mgr, 0, sizeof(*mgr));
    mgr->active_mod = -1;

#ifdef _WIN32
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", mods_dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (fd.cFileName[0] == '.') continue;
        snprintf(path, sizeof(path), "%s\\%s\\mod.json", mods_dir, fd.cFileName);
#else
    DIR *d = opendir(mods_dir);
    struct dirent *ent;
    if (!d) return 0;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        snprintf(path, sizeof(path), "%s/%s/mod.json", mods_dir, ent->d_name);
#endif
        f = fopen(path, "r");
        if (!f) continue;
        {
            int n = (int)fread(json_buf, 1, sizeof(json_buf) - 1, f);
            json_buf[n] = 0;
            fclose(f);
        }

        if (mgr->count < MOD_MAX) {
            FS_ModInfo *m = &mgr->mods[mgr->count];
            extract_json_string(json_buf, "name", m->name, MOD_NAME_MAX);
            extract_json_string(json_buf, "author", m->author, MOD_NAME_MAX);
            extract_json_string(json_buf, "version", m->version, 16);
            extract_json_string(json_buf, "game", m->game, 16);
            extract_json_string(json_buf, "description", m->description, 256);

#ifdef _WIN32
            snprintf(m->path, MOD_PATH_MAX, "%s\\%s", mods_dir, fd.cFileName);
#else
            snprintf(m->path, MOD_PATH_MAX, "%s/%s", mods_dir, ent->d_name);
#endif

            /* Check for optional files */
            snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", m->path);
            m->has_graphics = file_exists(path);
            snprintf(path, sizeof(path), "%s/strings.po", m->path);
            m->has_strings = file_exists(path);

            if (m->name[0]) {
                printf("Mod: %s by %s (%s)\n", m->name, m->author, m->game);
                mgr->count++;
            }
        }
#ifdef _WIN32
    } while (FindNextFileA(h, &fd));
    FindClose(h);
#else
    }
    closedir(d);
#endif

    printf("Found %d mods\n", mgr->count);
    return mgr->count;
}

int fs_mod_activate(FS_ModManager *mgr, int index) {
    if (!mgr || index < 0 || index >= mgr->count) return -1;
    mgr->active_mod = index;
    printf("Activated mod: %s\n", mgr->mods[index].name);
    return 0;
}

void fs_mod_deactivate(FS_ModManager *mgr) {
    if (mgr) mgr->active_mod = -1;
}

const FS_ModInfo *fs_mod_get_active(const FS_ModManager *mgr) {
    if (!mgr || mgr->active_mod < 0) return NULL;
    return &mgr->mods[mgr->active_mod];
}

const char *fs_mod_get_data_path(const FS_ModManager *mgr, const char *filename) {
    static char buf[512];
    const FS_ModInfo *m = fs_mod_get_active(mgr);
    if (!m || !filename) return NULL;
    snprintf(buf, sizeof(buf), "%s/%s", m->path, filename);
    return file_exists(buf) ? buf : NULL;
}

