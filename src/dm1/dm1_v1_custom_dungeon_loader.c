/*
 * dm1_v1_custom_dungeon_loader.c — Scan ~/.firestaff/dungeons/ (or any
 * caller-supplied directory) for community-made custom dungeons.
 *
 * A "custom dungeon" is any subdirectory that contains a dungeon.dat
 * (case-insensitive).  An optional graphics.dat alongside is recorded
 * so the engine can swap art when launching the level.
 *
 * Validation uses the same 44-byte header parser as the launcher-side
 * scanner (see custom_dungeon_m12.c), keyed off the shared
 * memory_dungeon_dat_pc34_compat.h constants.
 */

#include "dm1_v1_custom_dungeon_loader.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0755)
#endif

/* ── Helpers ────────────────────────────────────────────────────────── */

static void ensure_dir(const char* dir) {
    if (!dir || !*dir) return;
    (void)MKDIR(dir);
}

/* Case-insensitive match for "<wanted>" in a directory entry name. */
static int ieq(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

/* Look for `wanted` inside `dirPath`.  Writes the matched filename
 * back into `out` (cap `outCap`) on success.  Returns 1 if found. */
static int find_case_insensitive(const char* dirPath, const char* wanted,
                                 char* out, size_t outCap) {
    DIR* d = opendir(dirPath);
    struct dirent* e;
    if (!d) return 0;
    while ((e = readdir(d)) != NULL) {
        if (ieq(e->d_name, wanted)) {
            snprintf(out, outCap, "%s", e->d_name);
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}

/* ── Public API ─────────────────────────────────────────────────────── */

const char* DM1_V1_CustomDungeon_DefaultDirPc34Compat(void) {
    static char path[1024];
    const char* home = getenv("HOME");
    char base[1024];
    if (!home || !*home) home = ".";
    snprintf(base, sizeof(base), "%s/.firestaff", home);
    ensure_dir(base);
    snprintf(path, sizeof(path), "%s/.firestaff/dungeons", home);
    ensure_dir(path);
    return path;
}

void DM1_V1_CustomDungeon_InitPc34Compat(DM1_V1_CustomDungeonListPc34* list) {
    if (!list) return;
    memset(list, 0, sizeof(*list));
    list->count = 0;
    list->selectedIndex = 0;
}

int DM1_V1_CustomDungeon_ValidatePc34Compat(const char* dungeonDatPath, int* mapCount) {
    FILE* f;
    unsigned char hdr[DUNGEON_HEADER_SIZE];
    unsigned int sig;
    unsigned int mapCountField;
    long size;
    int ok;

    if (mapCount) *mapCount = 0;
    if (!dungeonDatPath) return 0;

    f = fopen(dungeonDatPath, "rb");
    if (!f) return 0;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    size = ftell(f);
    rewind(f);
    if (size < DUNGEON_HEADER_SIZE) { fclose(f); return 0; }

    if (fread(hdr, 1, DUNGEON_HEADER_SIZE, f) != DUNGEON_HEADER_SIZE) {
        fclose(f);
        return 0;
    }
    fclose(f);

    /* Reject compressed save signature. */
    sig = (unsigned)(hdr[0] | (hdr[1] << 8));
    if (sig == DUNGEON_COMPRESSED_SIGNATURE) return 0;

    /* mapCount byte at offset 4 must be 1..DUNGEON_MAX_MAPS. */
    mapCountField = hdr[4];
    if (mapCountField == 0 || mapCountField > (unsigned)DUNGEON_MAX_MAPS) return 0;

    ok = 1;
    if (mapCount) *mapCount = (int)mapCountField;
    return ok;
}

int DM1_V1_CustomDungeon_ScanPc34Compat(DM1_V1_CustomDungeonListPc34* list, const char* directory) {
    DIR* d;
    struct dirent* e;
    const char* baseDir;
    char fullPath[DM1_V1_CUSTOM_DUNGEON_PATH_MAX];
    struct stat st;

    if (!list) return 0;
    DM1_V1_CustomDungeon_InitPc34Compat(list);

    baseDir = directory && *directory ? directory : DM1_V1_CustomDungeon_DefaultDirPc34Compat();
    d = opendir(baseDir);
    if (!d) return 0;

    while ((e = readdir(d)) != NULL &&
           list->count < DM1_V1_CUSTOM_DUNGEON_MAX_ENTRIES) {
        char matchedDun[64];
        char matchedGfx[64];
        DM1_V1_CustomDungeonPc34* ent;

        if (e->d_name[0] == '.') continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", baseDir, e->d_name);
        if (stat(fullPath, &st) != 0) continue;
        if (!S_ISDIR(st.st_mode)) continue;

        if (!find_case_insensitive(fullPath, "dungeon.dat",
                                   matchedDun, sizeof(matchedDun))) {
            continue;
        }

        ent = &list->entries[list->count];
        memset(ent, 0, sizeof(*ent));
        snprintf(ent->name, sizeof(ent->name), "%s", e->d_name);
        snprintf(ent->path, sizeof(ent->path), "%s", fullPath);
        snprintf(ent->dungeonDatPath, sizeof(ent->dungeonDatPath),
                 "%s/%s", fullPath, matchedDun);

        if (find_case_insensitive(fullPath, "graphics.dat",
                                  matchedGfx, sizeof(matchedGfx))) {
            snprintf(ent->graphicsDatPath, sizeof(ent->graphicsDatPath),
                     "%s/%s", fullPath, matchedGfx);
        } else {
            ent->graphicsDatPath[0] = '\0';
        }

        if (stat(ent->dungeonDatPath, &st) == 0) {
            ent->fileSize = (long)st.st_size;
        } else {
            ent->fileSize = 0;
        }

        ent->valid = DM1_V1_CustomDungeon_ValidatePc34Compat(ent->dungeonDatPath,
                                                &ent->mapCount);
        list->count++;
    }
    closedir(d);

    return list->count;
}

const DM1_V1_CustomDungeonPc34* DM1_V1_CustomDungeon_GetSelectedPc34Compat(
    const DM1_V1_CustomDungeonListPc34* list) {
    if (!list || list->count == 0) return NULL;
    if (list->selectedIndex < 0 ||
        list->selectedIndex >= list->count) return NULL;
    return &list->entries[list->selectedIndex];
}
