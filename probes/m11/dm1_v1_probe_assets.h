#ifndef FIRESTAFF_PROBES_M11_DM1_V1_PROBE_ASSETS_H
#define FIRESTAFF_PROBES_M11_DM1_V1_PROBE_ASSETS_H

#include "asset_loader_m11.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"

#include <stddef.h>

/* ReDMCSB DEFS.H C026_GRAPHIC_CHAMPION_PORTRAITS and DUNVIEW.C
 * F0132 blits: champion mirrors use the DM1 C026 portrait atlas, not an
 * DM1-owned graphic id. The probe still uses the host asset loader to read
 * GRAPHICS.DAT, but the selected graphic id is owned by the DM1 contract.
 */
static inline const M11_AssetSlot*
dm1_v1_probe_load_c026_champion_portrait_atlas(M11_AssetLoader *loader)
{
    if (!loader) {
        return NULL;
    }
    return M11_AssetLoader_Load(
        loader, (unsigned int)dm1_v1_graphic_champion_portraits_pc34());
}

#endif
