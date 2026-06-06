/*
 * CSB V1 Viewport Rendering — pc34 compat implementation
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   DUNVIEW.C:6226-6353 F0676/F0677 (CSB back-wall D3L2/D3R2)
 *   DUNVIEW.C:6837-6896 F0678/F0679 (CSB near-wall D2L2/D2R2)
 *   DUNVIEW.C:8318-8542 F0128 (shared DM1/CSB viewport draw core)
 *
 * CSB differences from DM1:
 *   - D3L2/D3R2 back-wall positions render all 8 element types
 *     (WALL, TELEPORTER, STAIRS_FRONT, PIT, CORRIDOR, DOOR_SIDE,
 *      DOOR_FRONT, STAIRS_SIDE)
 *   - D2L2/D2R2 four-sided decoration positions render WALL only
 *     (no stairs, pits, floor ornaments, creatures, items, projectiles)
 *   - Four-sided wall decoration rules differ from DM1 corridor sides
 *   - Custom backgrounds (CSBWin/CSBCode.cpp:26 CustomBackgrounds)
 *   - CSB wall set index selection per current map
 *
 * Reference: CSBWin/Viewport.cpp (7290 lines) · CSBWin/Graphics.cpp (3186 lines)
 *   CSBWin/CSBCode.cpp:26 CustomBackgrounds · CSBWin/CSBCode.cpp:9196
 */

#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include <string.h>

enum {
    CSB_V1_ORNAMENT_SLOT_RIGHT = 1, /* M551_RIGHT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_ORNAMENT_SLOT_LEFT = 3,  /* M553_LEFT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_VIEW_WALL_D3L2_RIGHT = 0,
    CSB_V1_VIEW_WALL_D3R2_LEFT = 1,
    CSB_V1_NO_ORNAMENT_SLOT = -1,
    CSB_V1_NO_VIEW_WALL = -1,
    CSB_V1_VIEW_FLOOR_D3L2 = 0, /* C00_VIEW_FLOOR_D3L2 */
    CSB_V1_VIEW_FLOOR_D3R2 = 1, /* C01_VIEW_FLOOR_D3R2 */
    CSB_V1_CELL_ORDER_D3L2_DOORPASS1 = 0x0218,
    CSB_V1_CELL_ORDER_D3L2_DOORPASS2 = 0x0349,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS1 = 0x0128,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS2 = 0x0439,
    CSB_V1_CELL_ORDER_D3L2_CORRIDOR = 0x3421,
    CSB_V1_CELL_ORDER_D3R2_CORRIDOR = 0x4312,
    CSB_V1_CELL_ORDER_D3L2_SIDE = 0x0321,
    CSB_V1_CELL_ORDER_D3R2_SIDE = 0x0412,
    CSB_V1_ZONE_DOOR_D3L2 = 3700,
    CSB_V1_ZONE_DOOR_D3R2 = 3710,
    CSB_V1_REDMCSB_VIEW_SQUARE_D3L2 = 14, /* C14_VIEW_SQUARE_D3L2 */
    CSB_V1_REDMCSB_VIEW_SQUARE_D3R2 = 15, /* C15_VIEW_SQUARE_D3R2 */
    CSB_V1_VIEW_DEPTH_D3 = 3,
    CSB_V1_OBJECT_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_OBJECT_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_OBJECT_ZONE_BASE = 2500, /* C2500_ZONE_ */
    CSB_V1_OBJECT_ZONE_CELL_STRIDE = 4,
    CSB_V1_OBJECT_SHIFT_SET_D3_FRONT = 5, /* (viewDepth * 2) - 1 - (viewCell >> 1) for D3 back-row cells */
    CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL = 3,
    CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL = 4,
    CSB_V1_CREATURE_ROW_D3L2 = 3, /* G2033_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_CREATURE_ROW_D3R2 = 4, /* G2033_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_CREATURE_ZONE_BASE = 3200, /* C3200_ZONE_ */
    CSB_V1_CREATURE_COORDINATE_SET_STRIDE = 65,
    CSB_V1_CREATURE_ZONE_CELL_STRIDE = 5,
    CSB_V1_CREATURE_SHIFT_MASK = 0x8000, /* MASK0x8000_SHIFT_OBJECTS_AND_CREATURES */
    CSB_V1_PROJECTILE_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_PROJECTILE_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_PROJECTILE_ZONE_BASE = 2900, /* C2900_ZONE_ */
    CSB_V1_PROJECTILE_ZONE_STRIDE = 4,
    CSB_V1_EXPLOSION_ROW_D3L2 = 6, /* G2034_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_EXPLOSION_ROW_D3R2 = 7, /* G2034_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_FIELD_ASPECT_D3L2 = 0, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_FIELD_ASPECT_D3R2 = 1, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE = 3000, /* C3000_ZONE_ */
    CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE = 3007, /* C3007_ZONE_ */
    CSB_V1_EXPLOSION_CENTERED_ZONE_BASE = 3014, /* C3014_ZONE_ */
    CSB_V1_EXPLOSION_SIDE_ZONE_BASE = 3031, /* C3031_ZONE_ */
    CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE = 2,
    CSB_V1_FIELD_ZONE_D3L2 = 702, /* C702_ZONE_WALL_D3L2 */
    CSB_V1_FIELD_ZONE_D3R2 = 703, /* C703_ZONE_WALL_D3R2 */
    CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED = 1,
    CSB_V1_DOOR_PANEL_PARENT_D3L2 = 129,
    CSB_V1_DOOR_PANEL_PARENT_D3R2 = 130,
    CSB_V1_DOOR_PANEL_CLIP_D3 = 126,
    CSB_V1_DOOR_PANEL_NATIVE_W_D3 = 48,
    CSB_V1_DOOR_PANEL_NATIVE_H_D3 = 41,
    CSB_V1_DOOR_PANEL_CLIPPED_H_D3 = 40,
    CSB_V1_DOOR_PANEL_D3L2_X = 24,
    CSB_V1_DOOR_PANEL_D3R2_X = 88,
    CSB_V1_DOOR_PANEL_D3_Y = 28,
    CSB_V1_DOOR_ORNAMENT_D3LCR = 0, /* C0_VIEW_DOOR_ORNAMENT_D3LCR */
    CSB_V1_DOOR_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_WALL_ORNAMENT_ZONE_BASE = 1004, /* C1004_ZONE_WALL_ORNAMENT */
    CSB_V1_WALL_ORNAMENT_COORD_STRIDE = 15, /* MEDIA720 C15_UNKNOWN */
    CSB_V1_WALL_ORNAMENT_SCALE_X_D3 = 30, /* C30_SCALE_ */
    CSB_V1_WALL_ORNAMENT_SCALE_Y_D3 = 14, /* C14_SCALE_ */
    CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA = -1,
    CSB_V1_WALL_ORNAMENT_D3L2_BITMAP_INCREMENT = 0,
    CSB_V1_WALL_ORNAMENT_D3R2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_ZONE_BASE = 1500, /* C1500_ZONE_FLOOR_ORNAMENT */
    CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE = 11,
    CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA = -1,
    CSB_V1_FLOOR_ORNAMENT_D3L2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_D3R2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_COORD_SET = 0,
    CSB_V1_FLIP_NONE = 0, /* MASK0x0000_NO_FLIP */
    CSB_V1_FLIP_HORIZONTAL = 1 /* MASK0x0001_FLIP_HORIZONTAL */
};

static const CSB_V1_ViewportWallOrnamentRouteSpec s_wall_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        DM1_PC34_ZONE_WALL_D3L2,
        1,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6254-6263 wall panel then F0107(M551_RIGHT_WALL_ORNAMENT_ORDINAL, C00_VIEW_WALL_D3L2_RIGHT); DEFS.H:2696"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        DM1_PC34_ZONE_WALL_D3R2,
        1,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6321-6330 wall panel then F0107(M553_LEFT_WALL_ORNAMENT_ORDINAL, C01_VIEW_WALL_D3R2_LEFT); DEFS.H:2697"
    },
    {
        (int)DM1_VIEW_SQUARE_D2L2,
        DM1_PC34_ZONE_WALL_D2L2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0678_DrawD2L2",
        "DUNVIEW.C:6848-6865 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
    {
        (int)DM1_VIEW_SQUARE_D2R2,
        DM1_PC34_ZONE_WALL_D2R2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0679_DrawD2R2",
        "DUNVIEW.C:6877-6896 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3502-3590, 3817-3829, and
 * 3921-3923; F0676/F0677 lines 6263 and 6330.  The CSB/I34 far-side
 * wall-ornament views return early for ordinal 0, pre-decrement to the
 * current-map wall ornament index, build C1004 + CoordinateSet * 15 +
 * ViewWallIndex, scale through F0675 at C30/C14 with the D3 palette table,
 * flip only C01_VIEW_WALL_D3R2_LEFT, and dispatch the pixels through F0791
 * using C10 transparency. */
static const CSB_V1_ViewportWallOrnamentBlitSpec s_wall_ornament_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        1,
        CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_WALL_ORNAMENT_D3L2_BITMAP_INCREMENT,
        CSB_V1_WALL_ORNAMENT_ZONE_BASE,
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE,
        CSB_V1_WALL_ORNAMENT_SCALE_X_D3,
        CSB_V1_WALL_ORNAMENT_SCALE_Y_D3,
        CSB_V1_FLIP_NONE,
        CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR,
        1,
        1,
        "M551_RIGHT_WALL_ORNAMENT_ORDINAL",
        "F0676_DrawD3L2",
        "DUNVIEW.C:6263 F0107(M551, C00_VIEW_WALL_D3L2_RIGHT); F0107:3571 skips ordinal 0; 3575 ordinal--; 3576 reads native bitmap; 3587 zone C1004 + CoordinateSet*15 + ViewWall; 3817-3819 leaves D3L2 unflipped; 3824-3829 F0675 C30/C14 with G0198 D3 palette; 3921-3923 F0791 C10 blit. DEFS.H:2696,4222; DUNVIEW.C:805-819 G0190 MEDIA720; COORD.C:921-1025 C1000..C1107 layout records."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        1,
        CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_WALL_ORNAMENT_D3R2_BITMAP_INCREMENT,
        CSB_V1_WALL_ORNAMENT_ZONE_BASE,
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE,
        CSB_V1_WALL_ORNAMENT_SCALE_X_D3,
        CSB_V1_WALL_ORNAMENT_SCALE_Y_D3,
        CSB_V1_FLIP_HORIZONTAL,
        CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR,
        1,
        1,
        "M553_LEFT_WALL_ORNAMENT_ORDINAL",
        "F0677_DrawD3R2",
        "DUNVIEW.C:6330 F0107(M553, C01_VIEW_WALL_D3R2_LEFT); F0107:3571 skips ordinal 0; 3575 ordinal--; 3576 reads native bitmap; 3587 zone C1004 + CoordinateSet*15 + ViewWall; 3817-3819 flips C01_VIEW_WALL_D3R2_LEFT; 3824-3829 F0675 C30/C14 with G0198 D3 palette; 3921-3923 F0791 C10 blit. DEFS.H:2697,4222; DUNVIEW.C:805-819 G0190 MEDIA720; COORD.C:921-1025 C1000..C1107 layout records."
    },
};

/* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6270-6286 and 6337-6353.
 * The CSB-only D3L2/D3R2 routes call F0108 before the rear F0115 pass,
 * call F0111 for door-front panels, then finish with the front F0115 pass.
 * Pit cases fall through to the same F0108 call, preserving BUG0_64. */
static const CSB_V1_ViewportFloorOrnamentRouteSpec s_floor_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3L2,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 door-front floor ornament; 6271 F0115 pass1; 6272 F0111 C3700_ZONE_DOOR_D3L2; 6282-6286 pit/corridor F0108 then F0115"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3R2,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 door-front floor ornament; 6338 F0115 pass1; 6339 F0111 C3710_ZONE_DOOR_D3R2; 6349-6353 pit/corridor F0108 then F0115"
    },
};

/* ReDMCSB: DUNVIEW.C F0108 lines 3940-4008 and F0676/F0677 lines
 * 6270/6284 and 6337/6351.  F0108 draws nothing for ordinal 0, converts
 * ordinal to map floor-ornament index by pre-decrement, adds the per-view
 * G0191 native bitmap increment, and blits to
 * C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloorIndex using
 * F0791 with C10 transparency.  The CSB/I34 coordinate-set table is all 0,
 * and the CSB/I34 path flips D3R2 horizontally. */
static const CSB_V1_ViewportFloorOrnamentBlitSpec s_floor_ornament_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_FLOOR_ORNAMENT_D3L2_BITMAP_INCREMENT,
        CSB_V1_FLOOR_ORNAMENT_COORD_SET,
        CSB_V1_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE,
        CSB_V1_FLIP_NONE,
        CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR,
        "M552_FRONT_WALL_ORNAMENT_ORDINAL",
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 uses M552 for door-front D3L2; 6284 uses M558 for pit/corridor. F0108:3959 skips ordinal 0; 3965 ordinal-- plus G0191[0]=0; 3998 F0791 zone C1500 + CoordinateSet*11 + C00_VIEW_FLOOR_D3L2; G0195:1008-1017 gives CSB/I34 CoordinateSet 0; flip 0; C10. DEFS.H:2750, 3522-3523, 4223; COORD.C:903-913."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_FLOOR_ORNAMENT_D3R2_BITMAP_INCREMENT,
        CSB_V1_FLOOR_ORNAMENT_COORD_SET,
        CSB_V1_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE,
        CSB_V1_FLIP_HORIZONTAL,
        CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR,
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 uses M558 for door-front D3R2; 6351 uses M558 for pit/corridor. F0108:3959 skips ordinal 0; 3965 ordinal-- plus G0191[1]=0; G0195:1008-1017 gives CSB/I34 CoordinateSet 0; 3980-3983 sets MASK0x0001_FLIP_HORIZONTAL for C01_VIEW_FLOOR_D3R2; 3998 F0791 zone C1500 + CoordinateSet*11 + C01_VIEW_FLOOR_D3R2; C10. DEFS.H:2751, 3522-3523, 4223; COORD.C:903-913."
    },
};

/* ReDMCSB: DUNVIEW.C F0676 lines 6270-6286 and F0677 lines 6337-6353.
 * CSB's extra D3L2/D3R2 squares route the same F0115 stack as DM1: objects
 * first, a creature after object cells, projectiles after creatures, and
 * explosions after all cells.  Projectiles restart from the first thing for
 * the current cell and use C2900_ZONE_ + G2028[ViewSquare] * 4 + ViewCell.
 * Door-front squares split F0115 around F0111: rear cells before the door
 * panel/frame, front cells after it. */
static const CSB_V1_ViewportThingPassOrderSpec s_thing_pass_order_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3L2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3L2_SIDE,
        0,
        1,
        2,
        CSB_V1_PROJECTILE_ROW_D3L2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        3,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108; 6271 F0115 door rear cells 0x0218; 6272 F0111 door; 6284 F0108 pit/corridor BUG0_64; 6286 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5668 G2028 row, 5672 D3 front-cell skip, 5679 restart projectile list, 5681 cell match, 5683 C2900 zone, 5881-5883 blit, 5915-5933 explosions"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3R2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3R2_SIDE,
        0,
        1,
        2,
        CSB_V1_PROJECTILE_ROW_D3R2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        3,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108; 6338 F0115 door rear cells 0x0128; 6339 F0111 door; 6351 F0108 pit/corridor BUG0_64; 6353 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5668 G2028 row, 5672 D3 front-cell skip, 5679 restart projectile list, 5681 cell match, 5683 C2900 zone, 5881-5883 blit, 5915-5933 explosions"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4806-4811 and 4923.
 * For the PC34/I34E path, D3L2/D3R2 map through G2027/G2028 to depth 3
 * rows 3/4.  The object predicate accepts only weapon..junk things on the
 * processed cell; depth 3 squares suppress front-left/front-right cells by
 * requiring AL0126_i_ViewCell > C01_VIEW_CELL_FRONT_RIGHT. */
static const CSB_V1_ViewportObjectVisibilitySpec s_object_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0676_DrawD3L2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0677_DrawD3R2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4923-5110, DEFS.H lines 3517 and
 * 4228, and COORD.C lines 1129-1193.  For the PC34/I34 route,
 * visible weapon..junk objects are drawn through
 * C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES plus the G2028
 * visibility row and view cell.  The shift mask makes COORD.C apply the
 * per-pile G0223/G0217 shift before F0791 blits with C10 transparency. */
static const CSB_V1_ViewportObjectBlitSpec s_object_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        CSB_V1_OBJECT_ZONE_BASE,
        CSB_V1_OBJECT_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        CSB_V1_OBJECT_SHIFT_SET_D3_FRONT,
        1,
        10,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:4923 F0115 weapon..junk visible-cell predicate; 5030-5039 D3 object scale/shift set; 5071-5082 C2500_ZONE_ | MASK0x8000 + G2028 row*4 + ViewCell plus pile shift; 5109 F0791 C10 blit. DEFS.H:3517,4228; COORD.C:1129-1193 layout range 2500..2560."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        CSB_V1_OBJECT_ZONE_BASE,
        CSB_V1_OBJECT_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        CSB_V1_OBJECT_SHIFT_SET_D3_FRONT,
        1,
        10,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:4923 F0115 weapon..junk visible-cell predicate; 5030-5039 D3 object scale/shift set; 5071-5082 C2500_ZONE_ | MASK0x8000 + G2028 row*4 + ViewCell plus pile shift; 5109 F0791 C10 blit. DEFS.H:3517,4228; COORD.C:1129-1193 layout range 2500..2560."
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4840-4842, 5201-5214, and 5615-5627.
 * The MEDIA720 PC34/I34E path records one group thing while scanning each
 * cell, rejects view squares where G2033 maps to -1, then draws creatures
 * through C3200_ZONE_ + CreatureAspectCoordinateSet * 65 + G2033 row * 5
 * + ViewCell with MASK0x8000_SHIFT_OBJECTS_AND_CREATURES applied so COORD.C
 * adds the object/creature shifts before the F0791 blit. */
static const CSB_V1_ViewportCreatureVisibilitySpec s_creature_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_CREATURE_ROW_D3L2,
        1,
        1,
        CSB_V1_CREATURE_ZONE_BASE,
        CSB_V1_CREATURE_COORDINATE_SET_STRIDE,
        CSB_V1_CREATURE_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        "F0676_DrawD3L2",
        "DUNVIEW.C:375 G2033 row; 4840-4842 records C04_THING_TYPE_GROUP; 5201-5214 F0115 creature gate rejects G2033<0; 5615-5627 C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + G2033*5 + ViewCell then F0791; COORD.C:1248-1251 C3200 layout range 3200..3364; 2074-2075 clears shift mask"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_CREATURE_ROW_D3R2,
        1,
        1,
        CSB_V1_CREATURE_ZONE_BASE,
        CSB_V1_CREATURE_COORDINATE_SET_STRIDE,
        CSB_V1_CREATURE_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        "F0677_DrawD3R2",
        "DUNVIEW.C:375 G2033 row; 4840-4842 records C04_THING_TYPE_GROUP; 5201-5214 F0115 creature gate rejects G2033<0; 5615-5627 C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + G2033*5 + ViewCell then F0791; COORD.C:1248-1251 C3200 layout range 3200..3364; 2074-2075 clears shift mask"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 5915-6219, DEFS.H lines 4232-4235
 * and 4042-4043, COORD.C lines 1058-1123 and 1194-1238.  Explosions
 * restart from the first thing after all requested cells are processed.
 * MEDIA720 maps D3L2/D3R2 through G2034/G2035, uses C3000/C3007 for
 * rebirth, C3014 for centered explosions, C3031 + row*2 + cell for side
 * explosions, and defers fluxcage to F0113 as a field overlay. */
static const CSB_V1_ViewportExplosionBlitSpec s_explosion_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_EXPLOSION_ROW_D3L2,
        CSB_V1_FIELD_ASPECT_D3L2,
        1,
        1,
        1,
        CSB_V1_FIELD_ZONE_D3L2,
        CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE,
        CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE,
        CSB_V1_EXPLOSION_CENTERED_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE,
        10,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:5915-5933 restarts explosion pass; 5920-5924 G2034/G2035 visibility rows; 5948 rebirth requires visible row and matching cell; 5998-5999 C3000 + row; 6094-6096 C3007 + row; 6106-6107 C3014 + row; 6121-6122 C3031 + row*2 + ViewCell; 6192-6193 F0791 C10 blit; 6202-6219 fluxcage field C702 + field aspect. DEFS.H:4042,4232-4235; COORD.C:1058-1123,1194-1238."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_EXPLOSION_ROW_D3R2,
        CSB_V1_FIELD_ASPECT_D3R2,
        1,
        1,
        1,
        CSB_V1_FIELD_ZONE_D3R2,
        CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE,
        CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE,
        CSB_V1_EXPLOSION_CENTERED_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE,
        10,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:5915-5933 restarts explosion pass; 5920-5924 G2034/G2035 visibility rows; 5948 rebirth requires visible row and matching cell; 5998-5999 C3000 + row; 6094-6096 C3007 + row; 6106-6107 C3014 + row; 6121-6122 C3031 + row*2 + ViewCell; 6192-6193 F0791 C10 blit; 6202-6219 fluxcage field C702 + field aspect. DEFS.H:4043,4232-4235; COORD.C:1058-1123,1194-1238."
    },
};

/* ReDMCSB: DUNVIEW.C F0111 lines 4218-4337, F0676/F0677 lines
 * 6271-6273 and 6338-6340, DEFS.H lines 4250-4251, COORD.C lines
 * 1545-1565 and 781-807.  The CSB-only far door panels reuse the
 * D3 native 48x41 door bitmap but clip it through COORD.C record 126's
 * 48x40 viewport sub-zone.  F0111 skips state 0, shifts zone ids by
 * door state for partially-open doors, and blits with C10 transparency. */
static const CSB_V1_ViewportDoorPanelBlitSpec s_door_panel_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_ZONE_DOOR_D3L2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3L2,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6271 F0115 rear pass; 6272 F0111(... C3700_ZONE_DOOR_D3L2); 6273-6286 F0115 front pass. F0111:4248 skips C0 open, 4298-4321 shifts zone by state/horizontal halves, 4334 F0791 blits with C10. DEFS.H:4250; COORD.C:1548-1565 records 120/126/129 and 788-797 zone 3700..3709."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_ZONE_DOOR_D3R2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3R2,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6338 F0115 rear pass; 6339 F0111(... C3710_ZONE_DOOR_D3R2); 6340-6353 F0115 front pass. F0111:4248 skips C0 open, 4298-4321 shifts zone by state/horizontal halves, 4334 F0791 blits with C10. DEFS.H:4251; COORD.C:1548-1565 records 120/126/130 and 798-807 zone 3710..3719."
    },
};

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->ambient_color = 0xFF000000; /* black ambient */
    cfg->viewport_pixels = NULL;
    cfg->viewport_stride = 320;
    cfg->dungeon_grid = NULL;
    cfg->dungeon_width = 0;
    cfg->dungeon_height = 0;
}

void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set) {
    if (cfg) cfg->wall_set_index = set;
}

void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id) {
    if (cfg) cfg->custom_background = bg_id;
}

void csb_v1_viewport_set_dungeon_grid(CSB_V1_ViewportConfig *cfg,
                                       const uint8_t *grid,
                                       int width, int height) {
    if (!cfg) return;
    cfg->dungeon_grid = grid;
    cfg->dungeon_width = width;
    cfg->dungeon_height = height;
}

/* csb_v1_viewport_render_frame — integration entry point
 *
 * Renders the CSB dungeon view by delegating to the shared DM1 V1 viewport
 * engine (dm1_viewport_3d_draw_frame).  CSB config provides:
 *   - viewport_pixels + stride: the pixel buffer to draw into
 *   - dungeon_grid/width/height: square type data for element routing
 *   - wall_set_index: selects which GRAPHICS.DAT wall set to use
 *
 * When viewport_pixels is NULL, this is a no-op (allows staged integration).
 *
 * Source: CSBWin/Viewport.cpp F0128 passthrough; ReDMCSB DUNVIEW.C F0128
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y)
{
    if (!cfg) return;

    /* Guard: no viewport buffer means not yet initialised */
    if (!cfg->viewport_pixels) return;

    /* Set up a DM1 viewport state backed by our CSB pixel buffer.
     * We share the exact same pixel format (320×200 indexed, viewport
     * sub-region 224×136 at screen row 33) so the DM1 draw primitives
     * work without modification. */
    DM1_Viewport3DState vp;
    memset(&vp, 0, sizeof(vp));
    vp.viewport_pixels = cfg->viewport_pixels;
    vp.viewport_stride = cfg->viewport_stride > 0 ? cfg->viewport_stride : 320;
    vp.floor_area = cfg->viewport_pixels +
                    DM1_VIEWPORT_FLOOR_Y * vp.viewport_stride;
    vp.floor_ceiling_dirty = true;

    /* Wire dungeon grid for CSB back-wall rendering (D3L2/D3R2/D2L2/D2R2).
     * The dungeon grid enables element-specific routing for CSB four-sided
     * wall decoration: walls, doors, stairs, pits, teleporters, corridors.
     * When dungeon_grid is NULL, CSB back-wall rendering falls back to
     * the generic wall-drawing path (same as DM1).
     *
     * Grid layout: dungeon_grid[y * dungeon_width + x] = raw dungeon cell.
     * Raw cell: low 5 bits = element type (0=WALL, 1=CORRIDOR, 2=PIT,
     * 3=STAIRS, 4=DOOR, 5=TELEPORTER, 6=FAKEWALL).
     *
     * Source: ReDMCSB DUNVIEW.C:6226-6353 F0676/F0677; 6837-6896 F0678/F0679 */
    vp.dungeon_grid   = cfg->dungeon_grid;
    vp.dungeon_width  = cfg->dungeon_width;
    vp.dungeon_height = cfg->dungeon_height;

    /* Wall set index — CSB may use a different wall set than DM1.
     * ReDMCSB DUNVIEW.C F0096 loads wall set based on current map index.
     * Here we accept the index from config; 0 = default CSB wall set. */
    dm1_viewport_3d_load_wall_set(&vp, cfg->wall_set_index, 0);

    /* Main draw call — mirrors ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF.
     * Draws wall frames for D4 far objects, then D3L/R/C → D2 → D1 → D0
     * back-to-front with correct depth occlusion and parity flip.
     *
     * The CSB-specific elements (back-walls D3L2/D3R2, near-walls D2L2/D2R2)
     * are handled by dm1_viewport_3d_draw_csb_back_wall and
     * dm1_viewport_3d_draw_csb_near_wall, which are invoked from the
     * dm1_viewport_3d_draw_frame wall loop for D3L2/D3R2/D2L2/D2R2 positions.
     *
     * Custom background rendering — TODO (pass604).
     * CSBWin/CSBCode.cpp:26 CustomBackgrounds is a CSB-specific feature
     * that replaces the standard floor/ceiling rendering with a
     * per-map custom backdrop.  The background ID (cfg->custom_background)
     * indexes into the CSB custom background table.  This is distinct from
     * the DM1 floor/ceiling rendering which uses G2108/G2109 bitmap indices.
     * ReDMCSB does not have a clear reference for this in DUNVIEW.C;
     * it is specific to the CSBWin implementation.
     * Source: CSBWin/CSBCode.cpp:26 CustomBackgrounds (CSB custom feature) */

    dm1_viewport_3d_draw_frame(&vp, party_dir, party_x, party_y);
}

size_t csb_v1_viewport_wall_ornament_route_spec_count(void)
{
    return sizeof(s_wall_ornament_routes) / sizeof(s_wall_ornament_routes[0]);
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_route_spec_count()) return NULL;
    return &s_wall_ornament_routes[index];
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_route_spec_count(); ++i) {
        if (s_wall_ornament_routes[i].view_square == view_square) {
            return &s_wall_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_wall_ornament_blit_spec_count(void)
{
    return sizeof(s_wall_ornament_blits) / sizeof(s_wall_ornament_blits[0]);
}

const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_blit_spec_count()) return NULL;
    return &s_wall_ornament_blits[index];
}

const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_blit_spec_count(); ++i) {
        if (s_wall_ornament_blits[i].view_square == view_square) {
            return &s_wall_ornament_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_wall_ornament_blit_zone(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                            int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) +
           spec->view_wall_index;
}

int csb_v1_viewport_wall_ornament_native_bitmap_index(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                                      int base_native_bitmap_index)
{
    if (!spec || base_native_bitmap_index < 0) return -1;
    return base_native_bitmap_index + spec->native_bitmap_index_increment;
}

int csb_v1_viewport_wall_ornament_blit_pixels(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                              const uint8_t *source,
                                              int source_stride,
                                              uint8_t *destination,
                                              int destination_stride,
                                              int width,
                                              int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride <= 0 || destination_stride <= 0 ||
        width <= 0 || height <= 0) {
        return -1;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sx = spec->horizontal_flip ? (width - 1 - x) : x;
            uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

size_t csb_v1_viewport_floor_ornament_route_spec_count(void)
{
    return sizeof(s_floor_ornament_routes) / sizeof(s_floor_ornament_routes[0]);
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_route_spec_count()) return NULL;
    return &s_floor_ornament_routes[index];
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_route_spec_count(); ++i) {
        if (s_floor_ornament_routes[i].view_square == view_square) {
            return &s_floor_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_floor_ornament_blit_spec_count(void)
{
    return sizeof(s_floor_ornament_blits) / sizeof(s_floor_ornament_blits[0]);
}

const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_blit_spec_count()) return NULL;
    return &s_floor_ornament_blits[index];
}

const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_blit_spec_count(); ++i) {
        if (s_floor_ornament_blits[i].view_square == view_square) {
            return &s_floor_ornament_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_floor_ornament_blit_zone(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                             int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) + spec->floor_view_index;
}

int csb_v1_viewport_floor_ornament_native_bitmap_index(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                                       int base_native_bitmap_index)
{
    if (!spec || base_native_bitmap_index < 0) return -1;
    return base_native_bitmap_index + spec->native_bitmap_index_increment;
}

size_t csb_v1_viewport_thing_pass_order_spec_count(void)
{
    return sizeof(s_thing_pass_order_routes) / sizeof(s_thing_pass_order_routes[0]);
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec(size_t index)
{
    if (index >= csb_v1_viewport_thing_pass_order_spec_count()) return NULL;
    return &s_thing_pass_order_routes[index];
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_thing_pass_order_spec_count(); ++i) {
        if (s_thing_pass_order_routes[i].view_square == view_square) {
            return &s_thing_pass_order_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_object_visibility_spec_count(void)
{
    return sizeof(s_object_visibility_routes) / sizeof(s_object_visibility_routes[0]);
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_visibility_spec_count()) return NULL;
    return &s_object_visibility_routes[index];
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_visibility_spec_count(); ++i) {
        if (s_object_visibility_routes[i].view_square == view_square) {
            return &s_object_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_visibility_allows_cell(const CSB_V1_ViewportObjectVisibilitySpec *spec,
                                                  unsigned char cell_ordinal)
{
    if (!spec) return 0;
    if (cell_ordinal < spec->first_visible_cell_ordinal) return 0;
    if (cell_ordinal > spec->last_visible_cell_ordinal) return 0;
    return 1;
}

size_t csb_v1_viewport_object_blit_spec_count(void)
{
    return sizeof(s_object_blits) / sizeof(s_object_blits[0]);
}

const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_blit_spec_count()) return NULL;
    return &s_object_blits[index];
}

const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_blit_spec_count(); ++i) {
        if (s_object_blits[i].view_square == view_square) {
            return &s_object_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_blit_layout_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                            unsigned char view_cell)
{
    if (!spec || view_cell > 4 || spec->object_visibility_row < 0) return -1;
    return spec->object_zone_base +
           (spec->object_visibility_row * spec->object_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_object_blit_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                     unsigned char view_cell)
{
    int zone = csb_v1_viewport_object_blit_layout_zone(spec, view_cell);
    if (zone < 0) return -1;
    return zone | spec->shifts_objects_and_creatures;
}

size_t csb_v1_viewport_creature_visibility_spec_count(void)
{
    return sizeof(s_creature_visibility_routes) / sizeof(s_creature_visibility_routes[0]);
}

const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_creature_visibility_spec_count()) return NULL;
    return &s_creature_visibility_routes[index];
}

const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_creature_visibility_spec_count(); ++i) {
        if (s_creature_visibility_routes[i].view_square == view_square) {
            return &s_creature_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_creature_visibility_zone(const CSB_V1_ViewportCreatureVisibilitySpec *spec,
                                             int coordinate_set,
                                             unsigned char view_cell)
{
    if (!spec || coordinate_set < 0 || view_cell > 4 || spec->creature_visibility_row < 0) {
        return -1;
    }
    return spec->creature_zone_base +
           (coordinate_set * spec->creature_coordinate_set_stride) +
           (spec->creature_visibility_row * spec->creature_zone_cell_stride) +
           view_cell;
}

size_t csb_v1_viewport_explosion_blit_spec_count(void)
{
    return sizeof(s_explosion_blits) / sizeof(s_explosion_blits[0]);
}

const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_explosion_blit_spec_count()) return NULL;
    return &s_explosion_blits[index];
}

const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_explosion_blit_spec_count(); ++i) {
        if (s_explosion_blits[i].view_square == view_square) {
            return &s_explosion_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_explosion_rebirth_step1_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    if (!spec || spec->explosion_row < 0) return -1;
    return spec->rebirth_step1_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_rebirth_step2_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    if (!spec || spec->explosion_row < 0) return -1;
    return spec->rebirth_step2_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_centered_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    if (!spec || spec->explosion_row < 0) return -1;
    return spec->centered_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_side_zone(const CSB_V1_ViewportExplosionBlitSpec *spec,
                                        unsigned char view_cell)
{
    if (!spec || spec->explosion_row < 0 || view_cell > 1) return -1;
    return spec->side_zone_base +
           (spec->explosion_row * spec->side_zone_cell_stride) +
           view_cell;
}

size_t csb_v1_viewport_door_panel_blit_spec_count(void)
{
    return sizeof(s_door_panel_blits) / sizeof(s_door_panel_blits[0]);
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_door_panel_blit_spec_count()) return NULL;
    return &s_door_panel_blits[index];
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_door_panel_blit_spec_count(); ++i) {
        if (s_door_panel_blits[i].view_square == view_square) {
            return &s_door_panel_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_door_panel_blit_pixels(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                           int door_state,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < spec->native_bitmap_width ||
        destination_stride < spec->clipped_width ||
        spec->clipped_width <= 0 || spec->clipped_height <= 0 ||
        spec->clipped_width > spec->native_bitmap_width ||
        spec->clipped_height > spec->native_bitmap_height) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0111 lines 4248 and 4334 skip open doors, then
     * blit G0074 through F0791 with C10 transparency; COORD.C 1556-1560
     * clips the native D3 48x41 door bitmap to the 48x40 panel record. */
    if (spec->skips_open_state && door_state == 0) {
        return 0;
    }

    for (int y = 0; y < spec->clipped_height; ++y) {
        for (int x = 0; x < spec->clipped_width; ++x) {
            uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_source_evidence(void) {
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/DUNVIEW.C:\n"
        "  6226-6353 F0676/F0677 back-wall D3L2/D3R2 element routing\n"
        "  6254-6263 F0676 D3L2 wall panel then F0107 right-wall ornament route\n"
        "  6270-6286 F0676 D3L2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  6321-6330 F0677 D3R2 wall panel then F0107 left-wall ornament route\n"
        "  6337-6353 F0677 D3R2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  4567-4581 F0115 draws objects, then creatures, then projectiles per processed view cell\n"
        "  5668-5683 F0115 restarts for projectile things and uses C2900_ZONE_ + G2028 row * 4 + ViewCell\n"
        "  5881-5883 F0115 blits PC34/I34 projectile sprites through the computed C2900 zone\n"
        "  4806-4811 F0115 maps PC34 view square to lane/depth/object visibility rows\n"
        "  4923 F0115 filters weapon..junk objects by visible row, matching cell, and D3/D0 cell gates\n"
        "  5030-5039 F0115 selects PC34/I34 object scale and shift set from depth/cell\n"
        "  5071-5110 F0115 blits objects through C2500_ZONE_ | MASK0x8000 plus G2028 row/cell and pile shifts\n"
        "  375, 5201-5214, 5615-5627 F0115 maps creatures through G2033 and C3200_ZONE_ with MASK0x8000 shifts\n"
        "  5915-5933 F0115 restarts for explosions after all processed view cells\n"
        "  5920-6219 F0115 maps PC34/I34 explosions through G2034/G2035, C3000/C3007/C3014/C3031 zones, F0791 C10 blits, and fluxcage field deferral\n"
        "  3502-3590, 3817-3829, 3921-3923 F0107 maps CSB/I34 far wall ornaments through C1004 + CoordinateSet*15 + ViewWall, C30/C14 scaling, D3 palette changes, optional D3R2 flip, and F0791 C10 blits\n"
        "  3940-4008 F0108 floor ornament ordinal/index, G0191 native bitmap increment, C1500 zone, flip, C10 blit dispatch\n"
        "  4218-4337 F0111 door bitmap, ornament, state, zone shift, and C10 transparent blit dispatch\n"
        "  6837-6896 F0678/F0679 near-wall D2L2/D2R2 element routing\n"
        "  6848-6865 F0678 and 6877-6896 F0679 return for walls without F0107\n"
        "  8318-8542 F0128 shared viewport draw sequence\n"
        "  1008-1017 G0195 CSB/I34 floor ornament coordinate-set indices are all 0\n"
        "  DEFS.H:2750-2751 C00_VIEW_FLOOR_D3L2 / C01_VIEW_FLOOR_D3R2\n"
        "  DEFS.H:4250-4251 C3700_ZONE_DOOR_D3L2 / C3710_ZONE_DOOR_D3R2\n"
        "  DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT; COORD.C:903-913 floor ornament zone records\n"
        "  DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT; COORD.C:921-1025 wall ornament zone records\n"
        "  DEFS.H:4228 C2500_ZONE_; COORD.C:1129-1193 object zone records\n"
        "  DEFS.H:3517 MASK0x8000_SHIFT_OBJECTS_AND_CREATURES; 4236 C3200_ZONE_; COORD.C:1248-1251,2074-2075 creature zones\n"
        "  DEFS.H:4042-4043 C702/C703 field zones; 4232-4235 explosion zone bases; COORD.C:1058-1123,1194-1238 explosion zone records\n"
        "  COORD.C:1548-1565 D3 48x41 native door bitmap and 48x40 clip records; 788-807 far door zones\n"
        "  G0711/G0712 back-wall frame descriptors (lines 579-580)\n"
        "  G2107 WallSet bitmap indices (lines ~183)\n"
        "  G3048 WallSetFlipped (lines 277-295)\n"
        "ReDMCSB DEFS.H:2696-2697 C00_VIEW_WALL_D3L2_RIGHT / C01_VIEW_WALL_D3R2_LEFT\n"
        "CSBWin/Viewport.cpp: 7290 lines viewport rendering\n"
        "CSBWin/Graphics.cpp: 3186 lines asset cache\n"
        "CSBWin/CSBCode.cpp:26 CustomBackgrounds\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack (prison door)\n";
}
