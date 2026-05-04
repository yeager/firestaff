#ifndef FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_CEILING_ITEMS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_CEILING_ITEMS_PC34_COMPAT_H

/*
 * DM1 V1 Viewport Floor/Ceiling and Item Rendering — pc34 compat layer.
 *
 * Source reference: ReDMCSB DUNVIEW.C (8619 lines)
 *   - F0094_DUNGEONVIEW_LoadFloorSet: loads floor/ceiling bitmaps per map floor set
 *   - F0098_DUNGEONVIEW_DrawFloorAndCeiling: composites floor/ceiling into viewport
 *   - F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap: depth-correct pit/stair overlays
 *   - F0108_DUNGEONVIEW_DrawFloorOrnament: per-depth floor ornament with palette + flip
 *   - F0109_DUNGEONVIEW_DrawDoorOrnament: door ornament with shrink + palette
 *   - F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:
 *       The monolithic per-cell draw function. Draw order per cell:
 *       1. Walk thing list, defer groups/projectiles/explosions
 *       2. Draw each object (floor items) found
 *       3. Draw one creature at the cell being processed
 *       4. Restart thing list, draw only projectiles
 *       5. Restart thing list, draw only explosions (except Fluxcage)
 *       6. Draw Fluxcage if present
 *
 * Floor Set Graphics (ReDMCSB I34E DEFS.H):
 *   M644_GRAPHIC_FIRST_FLOOR_SET = 78
 *   C002_FLOOR_SET_GRAPHIC_COUNT = 2
 *   floorGraphic   = floorSet * 2 + 78
 *   ceilingGraphic = floorSet * 2 + 79
 *
 * Floor Ornament Graphics (ReDMCSB I34E DEFS.H):
 *   M616_GRAPHIC_FIRST_FLOOR_ORNAMENT = 385
 *   Each ornament has 6 variants (D3L=+0, D3C=+1, D2L=+2, D2C=+3, D1L=+4, D1C=+5)
 *   G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements[9] selects variant
 *   G0206_aaauc_Graphic558_FloorOrnamentCoordinateSets[3][9][6] positions them
 *   G0213/G0214: palette changes for D3/D2 depth darkening
 *
 * Item Drawing (F0115 object section, DUNVIEW.C:4820):
 *   Items on open squares are drawn per-cell with sub-cell offset
 *   Object position within cell uses G0219 coordinate sets
 *   Alcove items use C04_VIEW_CELL_ALCOVE special path
 *
 * Wall Ornament Graphics (ReDMCSB I34E DEFS.H):
 *   M615_GRAPHIC_FIRST_WALL_ORNAMENT = 259
 *   Each ornament has 2 variants per set
 *   Drawn at depth-scaled positions from G0207 coordinate sets
 *   Alcove ornaments (indices 1,2,3) enable item visibility on wall squares
 *
 * Viewport Draw Order (F0128 main square iteration):
 *   D3L2, D3R2 → D3L, D3R → D3C → D2L, D2R → D2C → D1L, D1R → D1C → D0L, D0R → D0C
 *   Each square: walls/doors → floor ornament → F0115 (items, creatures, projectiles)
 *   Parity flip: G0076_B_UseFlippedWallAndFootprintsBitmaps = (mapX+mapY+dir) & 1
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Floor set constants from ReDMCSB I34E DEFS.H */
#define DM1_GRAPHIC_FIRST_FLOOR_SET     78
#define DM1_FLOOR_SET_GRAPHIC_COUNT     2

/* Compute floor panel GRAPHICS.DAT index for a given floor set.
 * ReDMCSB DUNVIEW.C F0094_DUNGEONVIEW_LoadFloorSet. */
static inline unsigned int dm1_floor_set_floor_graphic(int floorSet) {
    return (unsigned int)(DM1_GRAPHIC_FIRST_FLOOR_SET +
                          floorSet * DM1_FLOOR_SET_GRAPHIC_COUNT);
}

static inline unsigned int dm1_floor_set_ceiling_graphic(int floorSet) {
    return dm1_floor_set_floor_graphic(floorSet) + 1;
}

/* Floor ornament variant indices from ReDMCSB G0191.
 * Maps VIEW_FLOOR_xxx enum to bitmap increment within the ornament set.
 *   D3L/D3R/D3L2/D3R2 = 0, D3C = 1,
 *   D2L/D2R = 2, D2C = 3,
 *   D1L/D1R = 4, D1C = 5 */
enum {
    DM1_FLOOR_ORN_D3_SIDE   = 0,
    DM1_FLOOR_ORN_D3_CENTER = 1,
    DM1_FLOOR_ORN_D2_SIDE   = 2,
    DM1_FLOOR_ORN_D2_CENTER = 3,
    DM1_FLOOR_ORN_D1_SIDE   = 4,
    DM1_FLOOR_ORN_D1_CENTER = 5,
    DM1_FLOOR_ORN_VARIANT_COUNT = 6
};

/* Floor ornament depth palette changes from ReDMCSB G0213/G0214.
 * At D3, colors are remapped for darkness; at D2, a lighter remap.
 * D1 and D0 use the identity palette (no change). */
extern const unsigned char DM1_FloorOrnPalette_D3[16];
extern const unsigned char DM1_FloorOrnPalette_D2[16];

/* ReDMCSB F0115 per-cell draw order constants.
 * P0146_ui_OrderedViewCellOrdinals encodes 4 nibbles:
 *   - Nibble 0: 0 = alcove, bit3 set = door front view pass
 *   - Nibbles 1-3: cell ordinals (1-4) for draw order
 * Common cell orders used by the DrawSquare functions: */
#define DM1_CELL_ORDER_ALCOVE                    0x0000
#define DM1_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT 0x0218
#define DM1_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT 0x0128
#define DM1_CELL_ORDER_DOORPASS1_BACKRIGHT        0x0028
#define DM1_CELL_ORDER_DOORPASS1_BACKLEFT         0x0018

/* Wall ornament alcove detection.
 * ReDMCSB DUNGEON.C F0149_DUNGEON_IsWallOrnamentAnAlcove:
 * Global ornament indices 1, 2, and 3 are alcoves. */
static inline int dm1_is_alcove_ornament(int globalIndex) {
    return globalIndex == 1 || globalIndex == 2 || globalIndex == 3;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_FLOOR_CEILING_ITEMS_PC34_COMPAT_H */
