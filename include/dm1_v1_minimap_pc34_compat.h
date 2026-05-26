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
 * V1 default is OFF; the V1 rendering path is unaffected unless the
 * user explicitly enables the minimap from config or by pressing F7.
 */

#include "m11_game_view.h"

#ifdef __cplusplus
extern "C" {
#endif

void DM1_Minimap_Render(M11_GameViewState* gameView,
                        unsigned char* framebuffer,
                        int fbWidth,
                        int fbHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MINIMAP_PC34_COMPAT_H */
