#ifndef FIRESTAFF_MENU_STARTUP_STATE_ACCESS_M12_H
#define FIRESTAFF_MENU_STARTUP_STATE_ACCESS_M12_H

#include "asset_status_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct M12_StartupMenuState M12_StartupMenuState;

/* Scanner-owned runtime source. An offline remembered preference may differ;
 * this accessor never admits media or replaces per-version hash validation. */
const char* M12_StartupMenu_AssetDataDir(const M12_StartupMenuState* state);

const M12_AssetVersionStatus* M12_StartupMenu_AssetVersion(
    const M12_StartupMenuState* state,
    const char* gameId,
    int index);

#ifdef __cplusplus
}
#endif

#endif
