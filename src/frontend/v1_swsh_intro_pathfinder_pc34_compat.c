/*
 * v1_swsh_intro_pathfinder_pc34_compat.c
 *
 * See v1_swsh_intro_pathfinder_pc34_compat.h for the design contract.
 *
 * ReDMCSB SWSH.C T0901006 ties the FTL logo (SWSHGDAT.C) to a
 * 320x200 bitmap that the original expands to Physbase. The canonical
 * PC 3.4 SWOOSH ships as an LZEXE-compressed MZ executable carrying
 * the PC IMG2/little-endian logo stream. This file only locates that
 * SWOOSH on disk; the actual decode and palette animation live in
 * swsh_frontend_pc34_compat.c (SWSH.C PC/F20 source-lock) and
 * main_loop_m11.c (M11 launcher handoff).
 */

#ifndef FIRESTAFF_V1_SWSH_INTRO_PATHFINDER_PC34_COMPAT_H
#include "v1_swsh_intro_pathfinder_pc34_compat.h"
#endif
#include "asset_find_by_hash.h"
#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "menu_startup_state_access_m12.h"
#include "swsh_frontend_pc34_compat.h"

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
    V1_SWSH_RECURSIVE_SCAN_MAX_DEPTH = 8,
    V1_SWSH_RECURSIVE_SCAN_MAX_FILES = 4096
};

static const char* const g_v1_swsh_known_md5s[] = {
    "28d406007e99b0ae8da0c6f7fc7f183b", /* ReDMCSB Reference/Original/A20ED/swoosh */
    "a66b607f3850e604b6703e90bbfb5189", /* ReDMCSB Reference/Original/I34E/SWOOSH */
    "a0ffbcc7ae8cecac03128ddb32887ef4", /* ReDMCSB Reference/Original/A20E/swoosh */
    "658db79c6bb87f3ab09eb7005cd91313", /* ReDMCSB Reference/Original/A21E/swoosh */
    "04adf67816f9794cc8352b41b1cb96a2", /* ReDMCSB Reference/Original/P20JB/SWOOSH */
    "4717e0f74fd326627a78ffcf3ab80127", /* ReDMCSB Reference/Original/X30J/SWOOSH */
    "1038138978975415571a878bb08f54be", /* ReDMCSB Reference/Original/A20F/swoosh */
    NULL
};

int V1_SWSH_Intro_PayloadLooksValid(const char* path) {
    FILE* f;
    long fsize;
    unsigned char* data;
    SWSH_CompatLogoPayload payload;
    int ok;
    if (!path || path[0] == '\0') return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize <= 0 || fsize > (long)(8u * 1024u * 1024u)) {
        fclose(f);
        return 0;
    }
    data = (unsigned char*)malloc((size_t)fsize);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1, (size_t)fsize, f) != (size_t)fsize) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    memset(&payload, 0, sizeof(payload));
    ok = SWSH_Compat_FindLogoImagePayloadEx(data, (unsigned int)fsize, &payload);
    SWSH_Compat_ReleaseLogoImagePayload(&payload);
    free(data);
    return ok;
}

static int v1_swsh_intro_find_known_hash(const char* dir,
                                          char* outPath,
                                          size_t outPathBytes) {
    char found[FSP_PATH_MAX];
    int matchIndex = -1;
    if (!dir || dir[0] == '\0' || !outPath || outPathBytes == 0U) {
        return 0;
    }
    found[0] = '\0';
    if (!asset_find_by_md5_list(dir,
                                g_v1_swsh_known_md5s,
                                found,
                                (int)sizeof(found),
                                &matchIndex,
                                V1_SWSH_RECURSIVE_SCAN_MAX_DEPTH)) {
        return 0;
    }
    (void)matchIndex;
    if (strstr(found, "::") != NULL || !V1_SWSH_Intro_PayloadLooksValid(found)) {
        return 0;
    }
    snprintf(outPath, outPathBytes, "%s", found);
    return 1;
}

static int v1_swsh_intro_scan_tree_for_payload(const char* dir,
                                                int depth,
                                                int* filesVisited,
                                                char* outPath,
                                                size_t outPathBytes) {
#if defined(_WIN32)
    char pattern[FSP_PATH_MAX];
    WIN32_FIND_DATAA data;
    HANDLE handle;
    if (!dir || !outPath || outPathBytes == 0U || !filesVisited ||
        depth > V1_SWSH_RECURSIVE_SCAN_MAX_DEPTH ||
        *filesVisited >= V1_SWSH_RECURSIVE_SCAN_MAX_FILES) {
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
            if (v1_swsh_intro_scan_tree_for_payload(child,
                                                     depth + 1,
                                                     filesVisited,
                                                     outPath,
                                                     outPathBytes)) {
                FindClose(handle);
                return 1;
            }
        } else {
            ++(*filesVisited);
            if (V1_SWSH_Intro_PayloadLooksValid(child)) {
                snprintf(outPath, outPathBytes, "%s", child);
                FindClose(handle);
                return 1;
            }
            if (*filesVisited >= V1_SWSH_RECURSIVE_SCAN_MAX_FILES) {
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
        depth > V1_SWSH_RECURSIVE_SCAN_MAX_DEPTH ||
        *filesVisited >= V1_SWSH_RECURSIVE_SCAN_MAX_FILES) {
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
            if (v1_swsh_intro_scan_tree_for_payload(child,
                                                     depth + 1,
                                                     filesVisited,
                                                     outPath,
                                                     outPathBytes)) {
                closedir(d);
                return 1;
            }
        } else if (S_ISREG(st.st_mode)) {
            ++(*filesVisited);
            if (V1_SWSH_Intro_PayloadLooksValid(child)) {
                snprintf(outPath, outPathBytes, "%s", child);
                closedir(d);
                return 1;
            }
            if (*filesVisited >= V1_SWSH_RECURSIVE_SCAN_MAX_FILES) {
                break;
            }
        }
    }
    closedir(d);
    return 0;
#endif
}

static int v1_swsh_intro_find_logo_path_for_suffixes(
                                const M12_StartupMenuState* menuState,
                                const char* dataDir,
                                const char* gameId,
                                const char* const* dataDirSuffixes,
                                size_t dataDirSuffixCount,
                                const char* const* homeSuffixes,
                                size_t homeSuffixCount,
                                char* outPath,
                                size_t outPathBytes) {
    char cand[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];
    char grandparent[FSP_PATH_MAX];
    size_t i;
    const char* home;
    const char* effectiveDataDir;

    if (!outPath || outPathBytes == 0U) return 0;
    outPath[0] = '\0';
    if (!gameId || gameId[0] == '\0') {
        gameId = "dm1";
    }

    /* 1. Env override. Useful for headless tests and developer overrides. */
    {
        const char* e = getenv("FIRESTAFF_SWOOSH");
        if (e && e[0] != '\0' && V1_SWSH_Intro_PayloadLooksValid(e)) {
            snprintf(outPath, outPathBytes, "%s", e);
            return 1;
        }
    }

    effectiveDataDir = (menuState &&
                        M12_StartupMenu_AssetDataDir(menuState) &&
                        M12_StartupMenu_AssetDataDir(menuState)[0] != '\0')
                           ? M12_StartupMenu_AssetDataDir(menuState)
                           : dataDir;
    if (!effectiveDataDir || effectiveDataDir[0] == '\0') {
        effectiveDataDir = ".";
    }

    /* 2. Start from the selected game's catalog path.  Do not recursively
     * hash the full shared data root during launch: it can include large ISO,
     * BIN and archive payloads for every game and stalls the title handoff.
     * The data scanner already owns full-root discovery; this runtime lookup
     * is deliberately bounded to the selected game's directory. */
    if (menuState) {
        for (i = 0U; i < M12_AssetStatus_GetVersionCount(gameId); ++i) {
            const M12_AssetVersionStatus* version =
                M12_StartupMenu_AssetVersion(menuState, gameId, i);
            if (!version || !version->matched ||
                version->matchedPath[0] == '\0') {
                continue;
            }
            if (!FSP_ParentDir(parent, sizeof(parent), version->matchedPath)) {
                continue;
            }
            if (FSP_JoinPath(cand, sizeof(cand), parent, "SWOOSH") &&
                V1_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
            if (FSP_JoinPath(cand, sizeof(cand), parent, "SWOOSH.DAT") &&
                V1_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
            if (FSP_ParentDir(grandparent, sizeof(grandparent), parent)) {
                if (FSP_JoinPath(cand, sizeof(cand), grandparent, "SWOOSH") &&
                    V1_SWSH_Intro_PayloadLooksValid(cand)) {
                    snprintf(outPath, outPathBytes, "%s", cand);
                    return 1;
                }
                if (FSP_JoinPath(cand, sizeof(cand), grandparent, "SWOOSH.DAT") &&
                    V1_SWSH_Intro_PayloadLooksValid(cand)) {
                    snprintf(outPath, outPathBytes, "%s", cand);
                    return 1;
                }
            }
        }
    }

    for (i = 0U; i < dataDirSuffixCount; ++i) {
        if (FSP_JoinPath(cand, sizeof(cand), effectiveDataDir, dataDirSuffixes[i]) &&
            V1_SWSH_Intro_PayloadLooksValid(cand)) {
            snprintf(outPath, outPathBytes, "%s", cand);
            return 1;
        }
    }
    /* Diagnostic-only escape hatch for unusual loose-file layouts.  Normal
     * launch must never walk unrelated games or disc images synchronously. */
    if (getenv("FIRESTAFF_SWOOSH_DEEP_SCAN")) {
        int filesVisited = 0;
        if (v1_swsh_intro_find_known_hash(effectiveDataDir,
                                           outPath,
                                           outPathBytes)) {
            return 1;
        }
        if (v1_swsh_intro_scan_tree_for_payload(effectiveDataDir,
                                                 0,
                                                 &filesVisited,
                                                 outPath,
                                                 outPathBytes)) {
            return 1;
        }
    }
    home = getenv("HOME");
    if (home && home[0] != '\0') {
        for (i = 0U; i < homeSuffixCount; ++i) {
            if (FSP_JoinPath(cand, sizeof(cand), home, homeSuffixes[i]) &&
                V1_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
        }
    }
    return 0;
}

int V1_SWSH_Intro_FindLogoPathForGame(const M12_StartupMenuState* menuState,
                                       const char* dataDir,
                                       const char* gameId,
                                       char* outPath,
                                       size_t outPathBytes) {
    static const char* dm1DataDirSuffixes[] = {
        "SWOOSH", "SWOOSH.DAT",
        "dm1/SWOOSH", "dm1/SWOOSH.DAT",
        "dm1-multilingual/SWOOSH", "dm1-multilingual/SWOOSH.DAT",
        "dm1-extras/legacy-dos/DungeonMasterPC34/SWOOSH",
        "dm1-extras/legacy-dos/DungeonMasterPC34/SWOOSH.DAT",
        "dm1-extras/legacy-dos/DungeonMasterPC34Multilingual/SWOOSH",
        "dm1-extras/legacy-dos/DungeonMasterPC34Multilingual/SWOOSH.DAT",
        "dm1-extras/dmfiles-dos-en-v34/SWOOSH",
        "dm1-extras/dmfiles-dos-en-v34/SWOOSH.DAT",
        "dm1-extras/dmfiles-dos-en/dungeon-master/dmaster/SWOOSH",
        "dm1-extras/dmfiles-dos-en/dungeon-master/dmaster/SWOOSH.DAT",
        "dm1-extras/pc-3.4-multi-3.5in/DATA/SWOOSH",
        "dm1-extras/pc-3.4-multi-3.5in/DATA/SWOOSH.DAT",
        "DungeonMasterPC34/SWOOSH", "DungeonMasterPC34/SWOOSH.DAT",
        "DungeonMasterPC34Multilingual/SWOOSH", "DungeonMasterPC34Multilingual/SWOOSH.DAT",
        "dm-pc34/DungeonMasterPC34/SWOOSH", "dm-pc34/DungeonMasterPC34/SWOOSH.DAT",
        "dm-pc34/DungeonMasterPC34Multilingual/SWOOSH",
        "dm-pc34/DungeonMasterPC34Multilingual/SWOOSH.DAT"
    };
    static const char* dm1HomeSuffixes[] = {
        ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/SWOOSH",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/SWOOSH",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34Multilingual/SWOOSH",
        ".firestaff/data/dm1/SWOOSH",
        ".firestaff/data/dm1-multilingual/SWOOSH"
    };
    static const char* csbDataDirSuffixes[] = {
        "csb/SWOOSH", "csb/SWOOSH.DAT",
        "ChaosStrikesBackPC34/SWOOSH",
        "ChaosStrikesBackPC34/SWOOSH.DAT",
        "csb-atari-st-2x/SWOOSH",
        "csb-atari-st-2x/SWOOSH.DAT",
        "SWOOSH", "SWOOSH.DAT",
        /* ReDMCSB SWSH.C is the shared FTL logo prelude.  CSB PC data sets
         * commonly do not carry their own SWOOSH beside GRAPHICS/DUNGEON, so
         * after CSB-specific candidates use the verified DM1 PC34 install
         * layouts as a shared FTL-logo fallback. */
        "dm1/SWOOSH", "dm1/SWOOSH.DAT",
        "dm1-multilingual/SWOOSH", "dm1-multilingual/SWOOSH.DAT",
        "dm1-extras/legacy-dos/DungeonMasterPC34/SWOOSH",
        "dm1-extras/legacy-dos/DungeonMasterPC34/SWOOSH.DAT",
        "dm1-extras/legacy-dos/DungeonMasterPC34Multilingual/SWOOSH",
        "dm1-extras/legacy-dos/DungeonMasterPC34Multilingual/SWOOSH.DAT",
        "dm1-extras/dmfiles-dos-en-v34/SWOOSH",
        "dm1-extras/dmfiles-dos-en-v34/SWOOSH.DAT",
        "dm1-extras/dmfiles-dos-en/dungeon-master/dmaster/SWOOSH",
        "dm1-extras/dmfiles-dos-en/dungeon-master/dmaster/SWOOSH.DAT",
        "dm1-extras/pc-3.4-multi-3.5in/DATA/SWOOSH",
        "dm1-extras/pc-3.4-multi-3.5in/DATA/SWOOSH.DAT"
    };
    static const char* csbHomeSuffixes[] = {
        ".firestaff/data/csb/SWOOSH",
        ".firestaff/data/csb/SWOOSH.DAT",
        ".openclaw/data/firestaff-original-games/DM/_canonical/csb/SWOOSH",
        ".openclaw/data/firestaff-original-games/DM/_extracted/csb/ChaosStrikesBackPC34/SWOOSH"
    };

    if (!gameId || strcmp(gameId, "dm1") == 0) {
        return v1_swsh_intro_find_logo_path_for_suffixes(
            menuState,
            dataDir,
            "dm1",
            dm1DataDirSuffixes,
            sizeof(dm1DataDirSuffixes) / sizeof(dm1DataDirSuffixes[0]),
            dm1HomeSuffixes,
            sizeof(dm1HomeSuffixes) / sizeof(dm1HomeSuffixes[0]),
            outPath,
            outPathBytes);
    }
    if (strcmp(gameId, "csb") == 0) {
        return v1_swsh_intro_find_logo_path_for_suffixes(
            menuState,
            dataDir,
            "csb",
            csbDataDirSuffixes,
            sizeof(csbDataDirSuffixes) / sizeof(csbDataDirSuffixes[0]),
            csbHomeSuffixes,
            sizeof(csbHomeSuffixes) / sizeof(csbHomeSuffixes[0]),
            outPath,
            outPathBytes);
    }
    if (outPath && outPathBytes > 0U) {
        outPath[0] = '\0';
    }
    return 0;
}

int V1_SWSH_Intro_FindLogoPath(const M12_StartupMenuState* menuState,
                                const char* dataDir,
                                char* outPath,
                                size_t outPathBytes) {
    return V1_SWSH_Intro_FindLogoPathForGame(menuState,
                                              dataDir,
                                              "dm1",
                                              outPath,
                                              outPathBytes);
}
