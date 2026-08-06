#ifndef FIRESTAFF_DM1_V1_MINIMAP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MINIMAP_PC34_COMPAT_H

/*
 * dm1_v1_minimap_pc34_compat — togglable corner minimap overlay.
 *
 * Renders a small top-down minimap into the presentation framebuffer
 * once the viewport has finished drawing.  Fog-of-war reuses the
 * existing M11_GameViewState::exploredBits[] tracking (updated by the
 * stairs / movement paths). Only visited cells are drawn.
 *
 * V1 default is OFF.  It remains a diagnostic-world aid: authenticated DM1
 * source sessions never accept this host-drawn surface, even when a saved
 * setting or F7 requests it.
 */

#include "m11_game_view.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The minimap is procedural host UI, not a PC34 surface.  Match the
 * diagnostic-overlay boundary used by the combat log so source-owned DM1
 * viewport pixels are never covered by persisted QoL state. */
static inline int DM1_V1_Minimap_SourceAllowsDiagnosticOverlay(
    M11_GameSourceKind sourceKind) {
    return sourceKind != M11_GAME_SOURCE_BUILTIN_CATALOG &&
           sourceKind != M11_GAME_SOURCE_CUSTOM_DUNGEON &&
           sourceKind != M11_GAME_SOURCE_DIRECT_DUNGEON;
}

void DM1_V1_Minimap_RenderPc34Compat(M11_GameViewState* gameView,
                        unsigned char* framebuffer,
                        int fbWidth,
                        int fbHeight);

#define DM1_Minimap_Render \
    DM1_V1_Minimap_RenderPc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MINIMAP_PC34_COMPAT_H */
