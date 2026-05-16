/*
 * DM1 V1 Viewport Floor/Ceiling and Item Rendering — pc34 compat layer.
 *
 * Source reference: ReDMCSB DUNVIEW.C
 *   F0094_DUNGEONVIEW_LoadFloorSet (line 2026)
 *   F0098_DUNGEONVIEW_DrawFloorAndCeiling (line 2962)
 *   F0108_DUNGEONVIEW_DrawFloorOrnament (line 3940)
 *   F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF (line 4547)
 *
 * This module provides the constants and palette tables used by the
 * viewport rendering code in m11_game_view.c.  The actual drawing is
 * integrated into the m11_draw_viewport pipeline, but these tables are
 * the authoritative source data extracted from ReDMCSB.
 */

#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"

/* ReDMCSB DUNVIEW.C G0213_auc_Graphic558_PaletteChanges_FloorOrnament_D3
 * Palette remapping for floor ornaments at depth 3 (farthest visible).
 * Each entry maps the original color index to a darker replacement.
 * Index 0 is transparency (unchanged). */
const unsigned char DM1_FloorOrnPalette_D3[16] = {
    0, 12, 1, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 2, 14, 13
};

/* ReDMCSB DUNVIEW.C G0214_auc_Graphic558_PaletteChanges_FloorOrnament_D2
 * Palette remapping for floor ornaments at depth 2 (middle distance).
 * Lighter than D3 but still dimmed from the original colors. */
const unsigned char DM1_FloorOrnPalette_D2[16] = {
    0, 1, 2, 3, 4, 3, 6, 7, 5, 9, 10, 11, 12, 13, 14, 15
};
