#ifndef FIRESTAFF_DM1_V1_AUTOMAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_AUTOMAP_PC34_COMPAT_H

/*
 * dm1_v1_automap_pc34_compat — automatic per-level map logging and export.
 *
 * Tracks visited cells per dungeon level for later BMP export.  The
 * minimap path already records visits via M11_GameViewState::exploredBits
 * for the current level; this module mirrors that information across
 * level changes so a complete map can be saved at any time.
 *
 * Export writes a 24-bit BMP (no external lib) to
 * ~/.firestaff/maps/dm1-level-XX.bmp.  Returns 1 on success, 0 on
 * failure.  V1 rendering path is unaffected — this is a passive logger
 * triggered only by the Ctrl+M / F8 hotkey.
 */

#include "m11_game_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Mark the current party cell as visited in the per-level visited grid. */
void DM1_AutoMap_RecordVisit(M11_GameViewState* gameView);

/* Export the auto-map for the party's current level to
 * ~/.firestaff/maps/dm1-level-XX.bmp. Returns 1 on success, 0 on error. */
int  DM1_AutoMap_ExportCurrentLevel(M11_GameViewState* gameView);

/* Underlying primitive used by the hotkey export. mapIndex 1-based not
 * required here; caller supplies the level index already in dungeon
 * coordinates. */
int  DM1_AutoMap_ExportPNG(M11_GameViewState* gameView,
                           int mapIndex,
                           const char* outputPath);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_AUTOMAP_PC34_COMPAT_H */
