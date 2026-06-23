/*
 * swsh_intro_pathfinder_m11.h
 *
 * DM1 V1 SWSH/FTL logo path finder for the M11 launcher handoff.
 *
 * ReDMCSB SWSH.C T0901006: the FTL logo is a 320x200 SWSHGDAT bitmap
 * expanded to Physbase. The canonical PC 3.4 SWOOSH file is an
 * LZEXE-compressed MZ executable that carries the PC IMG2/little-endian
 * logo stream. Firestaff has to find SWOOSH on disk before the SWSH.C
 * palette animation can run.
 *
 * This module centralises the search so the v2.7.4 release path stops
 * silently skipping the FTL intro when the file is anywhere other than
 * the very narrow path the original M11 in-tree search checked.
 *
 * Search order (mirrors m11_find_title_dat_for_intro so the FTL and
 * TITLE intros stay in lock-step on the same DM1 install layout):
 *   1. FIRESTAFF_SWOOSH env override (validated as a real SWOOSH file).
 *   2. Asset-catalog DM1 matched GRAPHICS.DAT path: parent dir of the
 *      matched file, then grandparent (PC 3.4: SWOOSH lives beside
 *      DATA/, same layout as TITLE/ANIM).
 *   3. menuState->assetStatus.dataDir, or fallback dataDir argument,
 *      joined with a small set of canonical subdirs (dm1/, dm1-multilingual/,
 *      DungeonMasterPC34/, dm-pc34/DungeonMasterPC34/...).
 *   4. $HOME canonical OpenClaw/firestaff original-games anchors.
 *
 * Every candidate is validated by M11_SWSH_Intro_PayloadLooksValid()
 * which accepts either a raw source-shaped 320x200 logo stream or an
 * MZ executable (canonical PC 3.4 SWOOSH is LZEXE-compressed). Junk files are rejected so
 * a stray "SWOOSH" of random bytes cannot break the FTL playback.
 */

#ifndef FIRESTAFF_SWSH_INTRO_PATHFINDER_M11_H
#define FIRESTAFF_SWSH_INTRO_PATHFINDER_M11_H

#include <stddef.h>

#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Returns 1 if `path` looks like a real DM1 FTL SWOOSH payload:
 * either a raw source-shaped 320x200 logo stream or an MZ executable
 * that contains one after LZEXE unpacking. Returns 0 for missing/short/junk files. */
int M11_SWSH_Intro_PayloadLooksValid(const char* path);

/* Locate the FTL SWOOSH file. On success returns 1 and writes the
 * path into outPath (NUL-terminated, truncated to outPathBytes). On
 * failure returns 0 and leaves outPath empty. Both menuState and
 * dataDir may be NULL; the search uses whichever is non-empty first.
 *
 * When the menuState carries an asset scan result, the asset-catalog
 * DM1 matched path takes priority over dataDir. The asset-catalog
 * branch is what makes the v2.7.4 release path robust: the original
 * search only looked at <dataDir>/SWOOSH and three hard-coded
 * $HOME anchors, which silently missed the typical
 * $HOME/.firestaff/data/dm1/SWOOSH layout and the canonical PC 3.4
 * install root (parent of DATA/). */
int M11_SWSH_Intro_FindLogoPath(const M12_StartupMenuState* menuState,
                                const char* dataDir,
                                char* outPath,
                                size_t outPathBytes);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SWSH_INTRO_PATHFINDER_M11_H */
