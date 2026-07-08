/*
 * title_intro_pathfinder_m11.h
 *
 * DM1 V1 TITLE intro path finder for the M11 launcher handoff.
 * The lookup validates candidates by the canonical PC 3.4 TITLE hash
 * and manifest, so data discovery does not depend on a correct filename.
 */

#ifndef FIRESTAFF_TITLE_INTRO_PATHFINDER_M11_H
#define FIRESTAFF_TITLE_INTRO_PATHFINDER_M11_H

#include <stddef.h>

#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

int M11_TitleIntro_FindTitleDatPath(const M12_StartupMenuState* menuState,
                                    const char* dataDir,
                                    char* outPath,
                                    size_t outPathBytes);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_TITLE_INTRO_PATHFINDER_M11_H */
