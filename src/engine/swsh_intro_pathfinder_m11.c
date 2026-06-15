/*
 * swsh_intro_pathfinder_m11.c
 *
 * See swsh_intro_pathfinder_m11.h for the design contract.
 *
 * ReDMCSB SWSH.C T0901006 ties the FTL logo (SWSHGDAT.C) to a
 * 320x200 IMG1 bitmap that the SWSH.PRG expands to the Atari ST
 * Physbase. The canonical PC 3.4 SWOOSH ships as an MZ executable that
 * embeds the same bitmap. This file only locates that SWOOSH on disk;
 * the actual IMG1 decode and palette animation live in
 * swsh_frontend_pc34_compat.c (SWSH.C PC/F20 source-lock) and
 * main_loop_m11.c (M11 launcher handoff).
 */

#ifndef FIRESTAFF_SWSH_INTRO_PATHFINDER_M11_H
#include "swsh_intro_pathfinder_m11.h"
#endif
#include "asset_status_m12.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int M11_SWSH_Intro_PayloadLooksValid(const char* path) {
    unsigned char head[8];
    size_t got;
    FILE* f;
    unsigned int widthLo;
    unsigned int widthHi;
    unsigned int heightLo;
    unsigned int heightHi;
    if (!path || path[0] == '\0') return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    got = fread(head, 1, sizeof(head), f);
    fclose(f);
    if (got < 4u) return 0;
    widthLo = (unsigned int)head[0];
    widthHi = (unsigned int)head[1];
    heightLo = (unsigned int)head[2];
    heightHi = (unsigned int)head[3];
    if ((widthLo | (widthHi << 8)) == 320u && (heightLo | (heightHi << 8)) == 200u) {
        /* Raw IMG1 320x200 logo (Atari ST layout). */
        return 1;
    }
    if (got >= 2u && head[0] == 'M' && head[1] == 'Z') {
        /* PC 3.4 SWOOSH is wrapped in an MZ executable; the IMG1
         * header is somewhere in the payload. SWSH_Compat_FindLogoImagePayload
         * picks it up at decode time. */
        return 1;
    }
    return 0;
}

int M11_SWSH_Intro_FindLogoPath(const M12_StartupMenuState* menuState,
                                const char* dataDir,
                                char* outPath,
                                size_t outPathBytes) {
    static const char* dataDirSuffixes[] = {
        "SWOOSH", "SWOOSH.DAT",
        "dm1/SWOOSH", "dm1/SWOOSH.DAT",
        "dm1-multilingual/SWOOSH", "dm1-multilingual/SWOOSH.DAT",
        "DungeonMasterPC34/SWOOSH", "DungeonMasterPC34/SWOOSH.DAT",
        "DungeonMasterPC34Multilingual/SWOOSH", "DungeonMasterPC34Multilingual/SWOOSH.DAT",
        "dm-pc34/DungeonMasterPC34/SWOOSH", "dm-pc34/DungeonMasterPC34/SWOOSH.DAT",
        "dm-pc34/DungeonMasterPC34Multilingual/SWOOSH",
        "dm-pc34/DungeonMasterPC34Multilingual/SWOOSH.DAT"
    };
    static const char* homeSuffixes[] = {
        ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/SWOOSH",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/SWOOSH",
        ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34Multilingual/SWOOSH",
        ".firestaff/data/dm1/SWOOSH",
        ".firestaff/data/dm1-multilingual/SWOOSH"
    };
    char cand[FSP_PATH_MAX];
    char parent[FSP_PATH_MAX];
    char grandparent[FSP_PATH_MAX];
    size_t i;
    const char* home;
    const char* effectiveDataDir;

    if (!outPath || outPathBytes == 0U) return 0;
    outPath[0] = '\0';

    /* 1. Env override. Useful for headless tests and developer overrides. */
    {
        const char* e = getenv("FIRESTAFF_SWOOSH");
        if (e && e[0] != '\0' && M11_SWSH_Intro_PayloadLooksValid(e)) {
            snprintf(outPath, outPathBytes, "%s", e);
            return 1;
        }
    }

    /* 2. Asset-catalog DM1 matched path. The asset scanner sets
     *    versions[dm1][i].matchedPath to the GRAPHICS.DAT it actually
     *    found, so the parent dir is the data dir (e.g. .../dm1/DATA)
     *    and the grandparent is the DM1 PC 3.4 install root. SWOOSH
     *    lives next to TITLE/ANIM in the canonical install layout. */
    if (menuState) {
        for (i = 0U; i < M12_AssetStatus_GetVersionCount("dm1"); ++i) {
            const M12_AssetVersionStatus* dm1v = M12_AssetStatus_GetVersion(
                &menuState->assetStatus, "dm1", i);
            if (!dm1v || !dm1v->matched || dm1v->matchedPath[0] == '\0') {
                continue;
            }
            if (!FSP_ParentDir(parent, sizeof(parent), dm1v->matchedPath)) {
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

    /* 3. dataDir + canonical subdirs. Prefer the menu state dataDir
     *    because that is the resolved Firestaff data root, falling back
     *    to the caller-supplied CLI/env value. */
    effectiveDataDir = (menuState && menuState->assetStatus.dataDir[0] != '\0')
                           ? menuState->assetStatus.dataDir
                           : dataDir;
    if (!effectiveDataDir || effectiveDataDir[0] == '\0') {
        effectiveDataDir = ".";
    }
    for (i = 0U; i < sizeof(dataDirSuffixes) / sizeof(dataDirSuffixes[0]); ++i) {
        if (FSP_JoinPath(cand, sizeof(cand), effectiveDataDir, dataDirSuffixes[i]) &&
            M11_SWSH_Intro_PayloadLooksValid(cand)) {
            snprintf(outPath, outPathBytes, "%s", cand);
            return 1;
        }
    }

    /* 4. $HOME canonical anchors (last-resort fallback). */
    home = getenv("HOME");
    if (home && home[0] != '\0') {
        for (i = 0U; i < sizeof(homeSuffixes) / sizeof(homeSuffixes[0]); ++i) {
            if (FSP_JoinPath(cand, sizeof(cand), home, homeSuffixes[i]) &&
                M11_SWSH_Intro_PayloadLooksValid(cand)) {
                snprintf(outPath, outPathBytes, "%s", cand);
                return 1;
            }
        }
    }
    return 0;
}
