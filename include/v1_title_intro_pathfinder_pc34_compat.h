/*
 * v1_title_intro_pathfinder_pc34_compat.h
 *
 * DM1/CSB V1 TITLE intro path finder for the runtime launcher handoff.
 * The lookup validates candidates by the canonical PC 3.4 TITLE hash
 * and manifest, so data discovery does not depend on a correct filename.
 */

#ifndef FIRESTAFF_V1_TITLE_INTRO_PATHFINDER_PC34_COMPAT_H
#define FIRESTAFF_V1_TITLE_INTRO_PATHFINDER_PC34_COMPAT_H

#include <stddef.h>

#include "menu_startup_state_access_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

int V1_TitleIntro_FindTitleDatPath(const M12_StartupMenuState* menuState,
                                    const char* dataDir,
                                    char* outPath,
                                    size_t outPathBytes);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_V1_TITLE_INTRO_PATHFINDER_PC34_COMPAT_H */
