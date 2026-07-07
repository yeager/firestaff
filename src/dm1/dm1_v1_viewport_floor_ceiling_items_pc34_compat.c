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
#include "memory_dungeon_dat_pc34_compat.h"

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

static const unsigned char kObjectInfoAspect[180] = {
    1,0,67,67,67,67,67,67,2,2,2,2,2,2,2,2,2,2,68,68,
    68,68,80,38,38,35,37,11,12,12,39,17,12,12,12,12,12,12,12,42,
    12,13,13,21,21,33,43,44,14,45,16,46,11,47,48,49,50,11,31,31,
    11,11,11,51,32,30,65,45,82,23,23,23,55,8,24,24,24,24,69,24,
    24,69,7,7,57,23,23,29,69,69,24,24,53,53,9,9,9,54,54,10,
    54,19,19,19,19,9,19,52,20,22,56,10,52,20,22,56,10,52,20,22,
    56,10,52,19,22,81,84,34,6,15,15,40,41,4,83,4,18,18,18,18,
    18,18,18,18,62,62,62,62,76,3,60,61,27,28,25,26,
    71,70,5,66,15,15,58,59,59,79,63,64,72,73,74,75,77,78,74,41
};

static const unsigned char kObjectAspectFirstNative[85] = {
     0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 82,
    84, 85, 86, 87, 88
};

int dm1_item_aspect_index(int thingType, int subtype) {
    int objectInfoIndex;
    if (subtype < 0) subtype = 0;
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (subtype > 45) subtype = 0;
            objectInfoIndex = 23 + subtype;
            break;
        case THING_TYPE_ARMOUR:
            if (subtype > 57) subtype = 0;
            objectInfoIndex = 69 + subtype;
            break;
        case THING_TYPE_SCROLL:
            objectInfoIndex = 0;
            break;
        case THING_TYPE_POTION:
            if (subtype > 20) subtype = 0;
            objectInfoIndex = 2 + subtype;
            break;
        case THING_TYPE_CONTAINER:
            if (subtype > 0) subtype = 0;
            objectInfoIndex = 1 + subtype;
            break;
        case THING_TYPE_JUNK:
            if (subtype > 52) subtype = 0;
            objectInfoIndex = 127 + subtype;
            break;
        default:
            return -1;
    }
    if (objectInfoIndex < 0 || objectInfoIndex >= 180) return -1;
    return (int)kObjectInfoAspect[objectInfoIndex];
}

unsigned int dm1_item_sprite_index(int thingType, int subtype) {
    int aspectIndex = dm1_item_aspect_index(thingType, subtype);
    if (aspectIndex < 0 || aspectIndex >= 85) return 0u;
    return DM1_GRAPHIC_FIRST_OBJECT +
           (unsigned int)kObjectAspectFirstNative[aspectIndex];
}

unsigned int dm1_object_aspect_graphic_info(int aspectIndex) {
    static const unsigned char kGraphicInfo[85] = {
        0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00
    };
    if (aspectIndex < 0 || aspectIndex >= 85) return 0u;
    return (unsigned int)kGraphicInfo[aspectIndex];
}

int dm1_object_aspect_coordinate_set(int aspectIndex) {
    static const unsigned char kCoordinateSet[85] = {
        0,1,1,1,1,1,0,0,0,1,0,1,1,0,2,1,
        1,0,1,2,2,1,2,0,0,1,1,1,1,0,1,1,
        1,0,1,1,0,0,1,1,0,0,1,1,0,2,1,1,
        1,1,1,0,0,1,0,0,0,0,1,1,1,1,1,1,
        1,0,1,1,1,0,0,0,0,1,1,1,0,1,1,1,
        1,0,1,1,0
    };
    if (aspectIndex < 0 || aspectIndex >= 85) return 0;
    return (int)kCoordinateSet[aspectIndex];
}
