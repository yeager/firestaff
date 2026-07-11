/*
 * title_intro_pathfinder_m11.c
 *
 * ReDMCSB TITLE fallback locator. Runtime rendering still prefers the
 * GRAPHICS.DAT C001 title graphic; this module locates the original PC 3.4
 * TITLE file when that graphic is unavailable. Every accepted file is checked
 * through V1_Title_IsCanonicalPc34Title(), which locks both bytes and manifest.
 */

#include "title_intro_pathfinder_m11.h"

#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "title_dat_loader_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

enum {
    M11_TITLE_RECURSIVE_SCAN_MAX_DEPTH = 8,
    M11_TITLE_RECURSIVE_SCAN_MAX_FILES = 4096
};

static const char* const g_m11_title_known_md5s[] = {
    "05c2ab94ce4dffe51b63985f7b0d1822", /* DM1 PC 3.4 TITLE, SHA256 in title_dat_loader_v1.h */
    NULL
};

static int m11_title_intro_candidate_is_valid(const char* path) {
    char titleErr[160];
    titleErr[0] = '\0';
    return V1_Title_IsCanonicalPc34Title(path, titleErr, sizeof(titleErr));
}

static int m11_title_intro_cache_virtual_path(const char* virtualPath,
                                              char* outPath,
                                              size_t outPathBytes) {
    char userData[FSP_PATH_MAX];
    char cacheRoot[FSP_PATH_MAX];
    char gameCache[FSP_PATH_MAX];
    char cachedTitle[FSP_PATH_MAX];
    if (!virtualPath || !outPath || outPathBytes == 0U ||
        strstr(virtualPath, "::") == NULL) {
        return 0;
    }
    if (!FSP_GetUserDataDir(userData, sizeof(userData)) ||
        !FSP_JoinPath(cacheRoot, sizeof(cacheRoot), userData, "asset-cache") ||
        !FSP_JoinPath(gameCache, sizeof(gameCache), cacheRoot, "dm1") ||
        !FSP_CreateDirectoryRecursive(gameCache) ||
        !FSP_JoinPath(cachedTitle, sizeof(cachedTitle), gameCache, "TITLE") ||
        !asset_extract_virtual_path(virtualPath, cachedTitle) ||
        !m11_title_intro_candidate_is_valid(cachedTitle)) {
        return 0;
    }
    snprintf(outPath, outPathBytes, "%s", cachedTitle);
    return 1;
}

static int m11_title_intro_find_known_hash(const char* dir,
                                           char* outPath,
                                           size_t outPathBytes) {
    char found[FSP_PATH_MAX];
    int matchIndex = -1;
    if (!dir || dir[0] == '\0' || !outPath || outPathBytes == 0U) {
        return 0;
    }
    found[0] = '\0';
    if (!asset_find_by_md5_list(dir,
                                g_m11_title_known_md5s,
                                found,
                                (int)sizeof(found),
                                &matchIndex,
                                M11_TITLE_RECURSIVE_SCAN_MAX_DEPTH)) {
        return 0;
    }
    (void)matchIndex;
    if (strstr(found, "::") != NULL) {
        return m11_title_intro_cache_virtual_path(found, outPath, outPathBytes);
    }
    if (!m11_title_intro_candidate_is_valid(found)) {
        return 0;
    }
    snprintf(outPath, outPathBytes, "%s", found);
    return 1;
}

static int m11_title_intro_scan_tree_for_title(const char* dir,
                                               int depth,
                                               int* filesVisited,
                                               char* outPath,
                                               size_t outPathBytes) {
#if defined(_WIN32)
    char pattern[FSP_PATH_MAX];
    WIN32_FIND_DATAA data;
    HANDLE handle;
    if (!dir || !outPath || outPathBytes == 0U || !filesVisited ||
        depth > M11_TITLE_RECURSIVE_SCAN_MAX_DEPTH ||
        *filesVisited >= M11_TITLE_RECURSIVE_SCAN_MAX_FILES) {
        return 0;
    }
    if (!FSP_JoinPath(pattern, sizeof(pattern), dir, "*")) {
        return 0;
    }
    handle = FindFirstFileA(pattern, &data);
    if (handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    do {
        char child[FSP_PATH_MAX];
        if (strcmp(data.cFileName, ".") == 0 ||
            strcmp(data.cFileName, "..") == 0) {
            continue;
        }
        if (!FSP_JoinPath(child, sizeof(child), dir, data.cFileName)) {
            continue;
        }
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (m11_title_intro_scan_tree_for_title(child,
                                                    depth + 1,
                                                    filesVisited,
                                                    outPath,
                                                    outPathBytes)) {
                FindClose(handle);
                return 1;
            }
        } else {
            ++(*filesVisited);
            if (m11_title_intro_candidate_is_valid(child)) {
                snprintf(outPath, outPathBytes, "%s", child);
                FindClose(handle);
                return 1;
            }
            if (*filesVisited >= M11_TITLE_RECURSIVE_SCAN_MAX_FILES) {
                break;
            }
        }
    } while (FindNextFileA(handle, &data));
    FindClose(handle);
    return 0;
#else
    DIR* d;
    struct dirent* ent;
    if (!dir || !outPath || outPathBytes == 0U || !filesVisited ||
        depth > M11_TITLE_RECURSIVE_SCAN_MAX_DEPTH ||
        *filesVisited >= M11_TITLE_RECURSIVE_SCAN_MAX_FILES) {
        return 0;
    }
    d = opendir(dir);
    if (!d) {
        return 0;
    }
    while ((ent = readdir(d)) != NULL) {
        char child[FSP_PATH_MAX];
        struct stat st;
        if (strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        if (!FSP_JoinPath(child, sizeof(child), dir, ent->d_name) ||
            stat(child, &st) != 0) {
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            if (m11_title_intro_scan_tree_for_title(child,
                                                    depth + 1,
                                                    filesVisited,
                                                    outPath,
                                                    outPathBytes)) {
                closedir(d);
                return 1;
            }
        } else if (S_ISREG(st.st_mode)) {
            ++(*filesVisited);
            if (m11_title_intro_candidate_is_valid(child)) {
                snprintf(outPath, outPathBytes, "%s", child);
                closedir(d);
                return 1;
            }
            if (*filesVisited >= M11_TITLE_RECURSIVE_SCAN_MAX_FILES) {
                break;
            }
        }
    }
    closedir(d);
    return 0;
#endif
}

int M11_TitleIntro_FindTitleDatPath(const M12_StartupMenuState* menuState,
                                    const char* dataDir,
                                    char* outPath,
                                    size_t outPathBytes) {
    const char* envPath;
    const char* effectiveDataDir;
    const char* home;
    char candidate[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];
    char grandparent[FSP_PATH_MAX];
    const M12_AssetVersionStatus* dm1v;
    size_t i;
    static const char* suffixes[] = {
        "TITLE", "TITLE.DAT",
        "dm1/TITLE", "dm1/TITLE.DAT",
        "dm1-multilingual/TITLE", "dm1-multilingual/TITLE.DAT",
        "dm1-extras/legacy-dos/DungeonMasterPC34/TITLE",
        "dm1-extras/legacy-dos/DungeonMasterPC34/TITLE.DAT",
        "dm1-extras/legacy-dos/DungeonMasterPC34Multilingual/TITLE",
        "dm1-extras/legacy-dos/DungeonMasterPC34Multilingual/TITLE.DAT",
        "dm1-extras/dmfiles-dos-en-v34/TITLE",
        "dm1-extras/dmfiles-dos-en-v34/TITLE.DAT",
        "dm1-extras/dmfiles-dos-en/dungeon-master/dmaster/TITLE",
        "dm1-extras/dmfiles-dos-en/dungeon-master/dmaster/TITLE.DAT",
        "dm1-extras/pc-3.4-multi-3.5in/DATA/TITLE",
        "dm1-extras/pc-3.4-multi-3.5in/DATA/TITLE.DAT",
        "DungeonMasterPC34/TITLE", "DungeonMasterPC34/TITLE.DAT",
        "DungeonMasterPC34Multilingual/TITLE",
        "DungeonMasterPC34Multilingual/TITLE.DAT",
        "dm-pc34/DungeonMasterPC34/TITLE",
        "dm-pc34/DungeonMasterPC34/TITLE.DAT",
        "dm-pc34/DungeonMasterPC34Multilingual/TITLE",
        "dm-pc34/DungeonMasterPC34Multilingual/TITLE.DAT"
    };
    static const char* homeSuffixes[] = {
        ".firestaff/data/TITLE",
        ".firestaff/data/dm1/TITLE",
        ".firestaff/data/dm1-multilingual/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/TITLE",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34Multilingual/TITLE"
    };

    if (!outPath || outPathBytes == 0U) {
        return 0;
    }
    outPath[0] = '\0';

    envPath = getenv("FIRESTAFF_TITLE_DAT");
    if (envPath && envPath[0] != '\0' &&
        m11_title_intro_candidate_is_valid(envPath)) {
        snprintf(outPath, outPathBytes, "%s", envPath);
        return 1;
    }

    effectiveDataDir = (menuState && menuState->assetStatus.dataDir[0] != '\0')
                           ? menuState->assetStatus.dataDir
                           : dataDir;
    if (!effectiveDataDir || effectiveDataDir[0] == '\0') {
        effectiveDataDir = ".";
    }

    if (m11_title_intro_find_known_hash(effectiveDataDir, outPath, outPathBytes)) {
        return 1;
    }

    if (menuState) {
        for (i = 0U; i < M12_AssetStatus_GetVersionCount("dm1"); ++i) {
            dm1v = M12_AssetStatus_GetVersion(&menuState->assetStatus, "dm1", i);
            if (!dm1v || !dm1v->matched || dm1v->matchedPath[0] == '\0') {
                continue;
            }
            if (!FSP_ParentDir(parent, sizeof(parent), dm1v->matchedPath)) {
                continue;
            }
            if (FSP_JoinPath(candidate, sizeof(candidate), parent, "TITLE") &&
                m11_title_intro_candidate_is_valid(candidate)) {
                snprintf(outPath, outPathBytes, "%s", candidate);
                return 1;
            }
            if (FSP_JoinPath(candidate, sizeof(candidate), parent, "TITLE.DAT") &&
                m11_title_intro_candidate_is_valid(candidate)) {
                snprintf(outPath, outPathBytes, "%s", candidate);
                return 1;
            }
            if (FSP_ParentDir(grandparent, sizeof(grandparent), parent)) {
                if (FSP_JoinPath(candidate, sizeof(candidate), grandparent, "TITLE") &&
                    m11_title_intro_candidate_is_valid(candidate)) {
                    snprintf(outPath, outPathBytes, "%s", candidate);
                    return 1;
                }
                if (FSP_JoinPath(candidate, sizeof(candidate), grandparent, "TITLE.DAT") &&
                    m11_title_intro_candidate_is_valid(candidate)) {
                    snprintf(outPath, outPathBytes, "%s", candidate);
                    return 1;
                }
            }
        }
    }

    for (i = 0U; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (FSP_JoinPath(candidate, sizeof(candidate), effectiveDataDir, suffixes[i]) &&
            m11_title_intro_candidate_is_valid(candidate)) {
            snprintf(outPath, outPathBytes, "%s", candidate);
            return 1;
        }
    }
    {
        int filesVisited = 0;
        if (m11_title_intro_scan_tree_for_title(effectiveDataDir,
                                                0,
                                                &filesVisited,
                                                outPath,
                                                outPathBytes)) {
            return 1;
        }
    }

    home = getenv("HOME");
    if (home && home[0] != '\0') {
        for (i = 0U; i < sizeof(homeSuffixes) / sizeof(homeSuffixes[0]); ++i) {
            if (FSP_JoinPath(candidate, sizeof(candidate), home, homeSuffixes[i]) &&
                m11_title_intro_candidate_is_valid(candidate)) {
                snprintf(outPath, outPathBytes, "%s", candidate);
                return 1;
            }
        }
    }
    return 0;
}
