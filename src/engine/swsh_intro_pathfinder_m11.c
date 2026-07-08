/*
 * swsh_intro_pathfinder_m11.c
 *
 * See swsh_intro_pathfinder_m11.h for the design contract.
 *
 * ReDMCSB SWSH.C T0901006 ties the FTL logo (SWSHGDAT.C) to a
 * 320x200 bitmap that the original expands to Physbase. The canonical
 * PC 3.4 SWOOSH ships as an LZEXE-compressed MZ executable carrying
 * the PC IMG2/little-endian logo stream. This file only locates that
 * SWOOSH on disk; the actual decode and palette animation live in
 * swsh_frontend_pc34_compat.c (SWSH.C PC/F20 source-lock) and
 * main_loop_m11.c (M11 launcher handoff).
 */

#ifndef FIRESTAFF_SWSH_INTRO_PATHFINDER_M11_H
#include "swsh_intro_pathfinder_m11.h"
#endif
#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "swsh_frontend_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int M11_SWSH_Intro_PayloadLooksValid(const char* path) {
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

static int m11_swsh_intro_find_logo_path_for_suffixes(
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
        if (e && e[0] != '\0' && M11_SWSH_Intro_PayloadLooksValid(e)) {
            snprintf(outPath, outPathBytes, "%s", e);
            return 1;
        }
    }

    /* 2. Asset-catalog matched path for the selected game. */
    if (menuState) {
        for (i = 0U; i < M12_AssetStatus_GetVersionCount(gameId); ++i) {
            const M12_AssetVersionStatus* version =
                M12_AssetStatus_GetVersion(&menuState->assetStatus, gameId, i);
            if (!version || !version->matched ||
                version->matchedPath[0] == '\0') {
                continue;
            }
            if (!FSP_ParentDir(parent, sizeof(parent), version->matchedPath)) {
                continue;
            }
            if (FSP_JoinPath(cand, sizeof(cand), parent, "SWOOSH") &&
                M11_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
            if (FSP_JoinPath(cand, sizeof(cand), parent, "SWOOSH.DAT") &&
                M11_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
            if (FSP_ParentDir(grandparent, sizeof(grandparent), parent)) {
                if (FSP_JoinPath(cand, sizeof(cand), grandparent, "SWOOSH") &&
                    M11_SWSH_Intro_PayloadLooksValid(cand)) {
                    snprintf(outPath, outPathBytes, "%s", cand);
                    return 1;
                }
                if (FSP_JoinPath(cand, sizeof(cand), grandparent, "SWOOSH.DAT") &&
                    M11_SWSH_Intro_PayloadLooksValid(cand)) {
                    snprintf(outPath, outPathBytes, "%s", cand);
                    return 1;
                }
            }
        }
    }

    effectiveDataDir = (menuState && menuState->assetStatus.dataDir[0] != '\0')
                           ? menuState->assetStatus.dataDir
                           : dataDir;
    if (!effectiveDataDir || effectiveDataDir[0] == '\0') {
        effectiveDataDir = ".";
    }
    for (i = 0U; i < dataDirSuffixCount; ++i) {
        if (FSP_JoinPath(cand, sizeof(cand), effectiveDataDir, dataDirSuffixes[i]) &&
            M11_SWSH_Intro_PayloadLooksValid(cand)) {
            snprintf(outPath, outPathBytes, "%s", cand);
            return 1;
        }
    }

    home = getenv("HOME");
    if (home && home[0] != '\0') {
        for (i = 0U; i < homeSuffixCount; ++i) {
            if (FSP_JoinPath(cand, sizeof(cand), home, homeSuffixes[i]) &&
                M11_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
        }
    }
    return 0;
}

int M11_SWSH_Intro_FindLogoPathForGame(const M12_StartupMenuState* menuState,
                                       const char* dataDir,
                                       const char* gameId,
                                       char* outPath,
                                       size_t outPathBytes) {
    static const char* dm1DataDirSuffixes[] = {
        "SWOOSH", "SWOOSH.DAT",
        "dm1/SWOOSH", "dm1/SWOOSH.DAT",
        "dm1-multilingual/SWOOSH", "dm1-multilingual/SWOOSH.DAT",
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
        "SWOOSH", "SWOOSH.DAT"
    };
    static const char* csbHomeSuffixes[] = {
        ".firestaff/data/csb/SWOOSH",
        ".firestaff/data/csb/SWOOSH.DAT",
        ".openclaw/data/firestaff-original-games/DM/_canonical/csb/SWOOSH",
        ".openclaw/data/firestaff-original-games/DM/_extracted/csb/ChaosStrikesBackPC34/SWOOSH"
    };

    if (!gameId || strcmp(gameId, "dm1") == 0) {
        return m11_swsh_intro_find_logo_path_for_suffixes(
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
        return m11_swsh_intro_find_logo_path_for_suffixes(
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

int M11_SWSH_Intro_FindLogoPath(const M12_StartupMenuState* menuState,
                                const char* dataDir,
                                char* outPath,
                                size_t outPathBytes) {
    return M11_SWSH_Intro_FindLogoPathForGame(menuState,
                                              dataDir,
                                              "dm1",
                                              outPath,
                                              outPathBytes);
}
